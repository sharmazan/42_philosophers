#include "philo.h"

void print_err(char *s) {
    while (s && *s)
        write(2, s++, 1);
    write(2, "\n", 1);
}

void print_string(char *s) {
    while (s && *s)
        write(1, s++, 1);
    write(1, "\n", 1);
}

int	ft_atoi(const char *nptr)
{
	long	i;
	int	digit;

	i = 0;
    if (!nptr || !*nptr) {
        return -1;
    }
    if (*nptr == '+')
		nptr++;
    if (!*nptr || *nptr == '-')
    	return -1;
    while (*nptr == '0')
		nptr++;
    while (*nptr >= '0' && *nptr <= '9')
	{
		digit = *nptr - '0';
		i = i * 10 + digit;
		nptr++;
	}
	if (*nptr)
	    return -1;
	if (i > 2147483647)
	    return -1;
	return i;
}

long get_time_ms(void) {
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000L) + (tv.tv_usec / 1000L);
}

int get_should_stop(t_sim *sim) {
    int stop;

    pthread_mutex_lock(&sim->stop_mutex);
    stop = sim->should_stop;
    pthread_mutex_unlock(&sim->stop_mutex);
    return stop;
}

void log_with_timestamp(t_sim *sim, int id, char *s) {
    long now;

    pthread_mutex_lock(&sim->print_mutex);
    now = get_time_ms();
    printf("%ld %d %s\n", now - sim->start_time, id, s);
    pthread_mutex_unlock(&sim->print_mutex);
}

void print_action(t_sim *sim, int id, char *s) {
    long now;

    pthread_mutex_lock(&sim->print_mutex);
    if (!get_should_stop(sim))
    {
        now = get_time_ms();
        printf("%ld %d %s\n", now - sim->start_time, id, s);
    }
    pthread_mutex_unlock(&sim->print_mutex);
}

void set_should_stop(t_sim *sim) {
    pthread_mutex_lock(&sim->stop_mutex);
    sim->should_stop = 1;
    pthread_mutex_unlock(&sim->stop_mutex);
}

void precise_sleep(t_sim *sim, long duration_ms) {
    long end_time;

    end_time = get_time_ms() + duration_ms;
    while (!get_should_stop(sim) && get_time_ms() < end_time)
        usleep(500);
}

void wait_after_thinking(t_philo *philo) {
    long delay;

    if (philo->sim->config.philo_num % 2 == 0)
        return ;
    delay = philo->sim->config.time_to_eat * 2
        - philo->sim->config.time_to_sleep;
    if (delay < 1)
        delay = 1;
    precise_sleep(philo->sim, delay);
}

void *worker(void *arg) {
    t_philo *philo;

    philo = (t_philo *)arg;
    if (philo->sim->config.philo_num == 1)
    {
        pthread_mutex_lock(philo->first_fork);
        print_action(philo->sim, philo->id, "has taken a fork");
        while (!get_should_stop(philo->sim))
            usleep(1000);
        pthread_mutex_unlock(philo->first_fork);
        return NULL;
    }
    while (!get_should_stop(philo->sim)) {
        // take_fork
        pthread_mutex_lock(philo->first_fork);
        if (get_should_stop(philo->sim))
        {
            pthread_mutex_unlock(philo->first_fork);
            break;
        }
        print_action(philo->sim, philo->id, "has taken a fork");
        pthread_mutex_lock(philo->second_fork);
        if (get_should_stop(philo->sim))
        {
            pthread_mutex_unlock(philo->second_fork);
            pthread_mutex_unlock(philo->first_fork);
            break;
        }
        print_action(philo->sim, philo->id, "has taken a fork");
        if (get_should_stop(philo->sim))
        {
            pthread_mutex_unlock(philo->first_fork);
            pthread_mutex_unlock(philo->second_fork);
            break ;
        }
        pthread_mutex_lock(&philo->meal_mutex);
        philo->last_meal_time = get_time_ms();
        pthread_mutex_unlock(&philo->meal_mutex);
        print_action(philo->sim, philo->id, "is eating");
        precise_sleep(philo->sim, philo->sim->config.time_to_eat);
        pthread_mutex_unlock(philo->first_fork);
        pthread_mutex_unlock(philo->second_fork);
        pthread_mutex_lock(&philo->meal_mutex);
        philo->meals_eaten++;
        pthread_mutex_unlock(&philo->meal_mutex);
        if (get_should_stop(philo->sim))
            break ;
        print_action(philo->sim, philo->id, "is sleeping");
        precise_sleep(philo->sim, philo->sim->config.time_to_sleep);
        if (get_should_stop(philo->sim))
            break ;
        print_action(philo->sim, philo->id, "is thinking");
        wait_after_thinking(philo);
    }
    return NULL;
}

void malloc_philo_and_forks(t_sim *sim) {
    sim->philos = malloc(sizeof(t_philo) * sim->config.philo_num);
    sim->forks = malloc(sizeof(pthread_mutex_t) * sim->config.philo_num);
}

void init(t_sim *sim) {
    int i;

    pthread_mutex_init(&sim->print_mutex, NULL);
    pthread_mutex_init(&sim->stop_mutex, NULL);
    malloc_philo_and_forks(sim);
    i = 0;
    while (i < sim->config.philo_num)
        pthread_mutex_init(&sim->forks[i++], NULL);
}

void cleanup(t_sim *sim) {
    int i;

    pthread_mutex_destroy(&sim->print_mutex);
    pthread_mutex_destroy(&sim->stop_mutex);
    i = 0;
    while (i < sim->config.philo_num) {
        pthread_mutex_destroy(&sim->forks[i]);
        pthread_mutex_destroy(&sim->philos[i].meal_mutex);
        i++;
    }
    free(sim->philos);
    free(sim->forks);
}

int is_args_positiv_int(int ac, char **av) {
    int i;

    i = 1;
    while (i < ac) {
        if (!ft_atoi(av[i]) || ft_atoi(av[i]) == -1)
            return 0;
        i++;
    }
    return 1;
}

int get_philo_meals_eaten(t_philo *philo) {
    int value;

    pthread_mutex_lock(&philo->meal_mutex);
    value = philo->meals_eaten;
    pthread_mutex_unlock(&philo->meal_mutex);
    return value;
}


long get_philo_last_meal_time(t_philo *philo) {
    long value;

    pthread_mutex_lock(&philo->meal_mutex);
    value = philo->last_meal_time;
    pthread_mutex_unlock(&philo->meal_mutex);
    return value;
}

int main(int ac, char **av) {
    t_sim sim;
    int i;
    static int stop;

    if (ac < 5 || ac > 6) {
        print_err("Wrong arguments. Use it with:\n./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]");
        return 1;
    }

    if (!is_args_positiv_int(ac, av)) {
        print_err("All arguments should be positive integer");
        return 1;
    }

    if (ft_atoi(av[1]) > 1000) {
        print_err("Invalid number or philosophers. Maximum supported amount is 1000");
        return 1;
    }

    // setup
    sim.config.philo_num = ft_atoi(av[1]);
    sim.config.time_to_die = ft_atoi(av[2]);
    sim.config.time_to_eat = ft_atoi(av[3]);
    sim.config.time_to_sleep = ft_atoi(av[4]);
    if (ac == 6)
        sim.config.must_eat_count = ft_atoi(av[5]);
    else
        sim.config.must_eat_count = 0;

    sim.start_time = get_time_ms();
    sim.should_stop = 0;
    init(&sim);
    // printf("Program started\n");
    // finish setup

    // start threads
    i = 0;
    while (i < sim.config.philo_num) {
        sim.philos[i].id = i + 1;
        sim.philos[i].meals_eaten = 0;
        sim.philos[i].last_meal_time = sim.start_time;
        sim.philos[i].sim = &sim;
        pthread_mutex_init(&sim.philos[i].meal_mutex, NULL);
        if (i % 2) {
            sim.philos[i].first_fork = &sim.forks[i];
            sim.philos[i].second_fork = &sim.forks[(i+1) % sim.config.philo_num];
        }
        else {
            sim.philos[i].first_fork = &sim.forks[(i+1) % sim.config.philo_num];
            sim.philos[i].second_fork = &sim.forks[i];
        }

        pthread_create(&sim.philos[i].thread, NULL, worker, &sim.philos[i]);
        i++;
    }

    while (!get_should_stop(&sim)) {
        // do all philosophers eat enough?
        if (sim.config.must_eat_count) {
            stop = 1;
            i = 0;
            while (stop && i < sim.config.philo_num) {
                stop = get_philo_meals_eaten(&sim.philos[i]) >= sim.config.must_eat_count;
                i++;
            }
            if (stop)
                set_should_stop(&sim);
        }

        if (!get_should_stop(&sim)) {
            i = 0;
            // verify if philo X should die
            while (!get_should_stop(&sim) && i < sim.config.philo_num) {
                if (get_time_ms() - get_philo_last_meal_time(&sim.philos[i]) > sim.config.time_to_die) {
                    set_should_stop(&sim);
                    log_with_timestamp(&sim, sim.philos[i].id, "died");
                }
                i++;
            }

        }
        usleep(500);
    }

    i = 0;
    while (i < sim.config.philo_num)
    {
        pthread_join(sim.philos[i].thread, NULL);
        i++;
    }
    // cleanup
    cleanup(&sim);
    // printf("Program finished\n");
    return 0;
}
