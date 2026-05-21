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

long get_time_ms(void) {
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000L) + (tv.tv_usec / 1000L);
}

void log_with_timestamp(t_sim *sim, int x, char *s) {
    long now;

    now = get_time_ms();
    printf("%ld %d %s\n", now - sim->start_time, x, s);
}

void *worker(void *arg) {
    t_philo *philo;

    philo = (t_philo *)arg;
    while (1) {
        // take_fork
        if (!philo->sim->should_stop) {
            pthread_mutex_lock(philo->left_fork);
            log_with_timestamp(philo->sim, philo->id, "has taken a fork");
        }
        if (!philo->sim->should_stop) {
            pthread_mutex_lock(philo->right_fork);
            log_with_timestamp(philo->sim, philo->id, "has taken a fork");
        }
        if (!philo->sim->should_stop) {
            philo->last_meal_time = get_time_ms();
            log_with_timestamp(philo->sim, philo->id, "is eating");
            usleep(philo->sim->config.time_to_eat * 1000L);
            pthread_mutex_unlock(philo->left_fork);
            pthread_mutex_unlock(philo->right_fork);
            philo->meals_eaten++;
        }
        if (!philo->sim->should_stop) {
            log_with_timestamp(philo->sim, philo->id, "is sleeping");
            usleep(philo->sim->config.time_to_sleep * 1000L);
        }
        if (!philo->sim->should_stop) {
            log_with_timestamp(philo->sim, philo->id, "is thinking");
        }
        // usleep(philo->sim->config.time_to_think * 1000L);
        // usleep(1000);
    }
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

void cleanup(t_sim sim) {
    int i;

    pthread_mutex_destroy(&sim.print_mutex);
    pthread_mutex_destroy(&sim.stop_mutex);
    i = 0;
    while (i < sim.config.philo_num)
        pthread_mutex_destroy(&sim.forks[i++]);
    free(sim.philos);
    free(sim.forks);
}


int main(int ac, char **av) {
    t_sim sim;
    int i;
    static int stop;

    if (ac < 5 || ac > 6) {
        print_err("Wrong arguments. Use it with:\n./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]");
        return 1;
    }

    // setup
    sim.config.philo_num = atoi(av[1]);
    sim.config.time_to_die = atoi(av[2]);
    sim.config.time_to_eat = atoi(av[3]);
    sim.config.time_to_sleep = atoi(av[4]);
    if (ac == 6)
        sim.config.must_eat_count = atoi(av[5]);
    else
        sim.config.must_eat_count = 0;

    sim.start_time = get_time_ms();
    sim.should_stop = 0;
    init(&sim);
    printf("Program started\n");
    // finish setup

    // start threads
    i = 0;
    while (i < sim.config.philo_num) {
        sim.philos[i].id = i;
        sim.philos[i].meals_eaten = 0;
        sim.philos[i].last_meal_time = sim.start_time;
        sim.philos[i].sim = &sim;
        printf("Left fork id: %d\n", i);
        printf("Right fork id: %d\n", (i+1) % sim.config.philo_num);
        sim.philos[i].left_fork = &sim.forks[i];
        sim.philos[i].right_fork = &sim.forks[(i+1) % sim.config.philo_num];
        pthread_create(&sim.philos[i].thread, NULL, worker, &sim.philos[i]);
        i++;
    }

    // i = 0;
    // while (i < sim.config.philo_num) {
    //     pthread_join(sim.philos[i].thread, NULL);
    //     i++;
    // }

    while (!sim.should_stop) {
        // do all philosophers eat enough?
        if (sim.config.must_eat_count) {
            stop = 1;
            i = 0;
            while (stop && i < sim.config.philo_num) {
                // log_with_timestamp(&sim, i, "ENOUGH?");
                stop = sim.philos[i].meals_eaten >= sim.config.must_eat_count;
                // if (stop)
                //     log_with_timestamp(&sim, i, "YES!");
                i++;
            }
            sim.should_stop = stop;
        }

        if (!sim.should_stop) {
            i = 0;
            // verify if philo X should die
            while (!sim.should_stop && i < sim.config.philo_num) {
                if (get_time_ms() - sim.philos[i].last_meal_time > sim.config.time_to_die) {
                    sim.should_stop = 1;
                    log_with_timestamp(&sim, i, "died");
                }
                i++;
            }
        }
    }

    // cleanup
    cleanup(sim);
    printf("Program finished\n");
    return 0;
}
