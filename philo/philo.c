#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

typedef struct s_config {
    int philo_num;
    long last_meal;
    long time_to_die;
    long time_to_eat;
    long time_to_sleep;
    int must_eat_count;
} t_config;

typedef struct s_sim t_sim;

typedef struct s_philo  {
    int id;
    pthread_t thread;
    pthread_mutex_t *left_fork;
    pthread_mutex_t *right_fork;
    int meals_eaten;
    pthread_mutex_t *meals_mutex;

    t_sim *sim;
} t_philo;

typedef struct s_sim {
    t_config config;
    t_philo *philos;
    pthread_mutex_t *forks;
    pthread_mutex_t print_mutex;
    pthread_mutex_t stop_mutex;
    long start_time;
} t_sim;

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

void log_with_timestamp(int x, char *s, long starttime) {
    long now;
    static long start;

    if (starttime) {
        start = get_time_ms();
        printf("log_with_timestamp inited\n");
        return;
    }

    now = get_time_ms();
    printf("%ld: %d %s\n", now - start, x, s);
}

void *worker(void *arg) {
    t_philo *philo;

    philo = (t_philo *)arg;
    while (1) {
        printf("%d eating\n", philo->id);
        usleep(philo->sim->config.time_to_eat * 1000L);
        philo->meals_eaten++;
        log_with_timestamp(philo->id, "is sleeping", 0);
        usleep(philo->sim->config.time_to_sleep * 1000L);
        printf("%d thinking\n", philo->id);
        // usleep(philo->sim->config.time_to_think * 1000L);
        usleep(1000);
        if (philo->sim->config.must_eat_count && philo->meals_eaten == philo->sim->config.must_eat_count) {
            printf("thread %d finished\n", philo->id);
            return NULL;
        }
    }
}

void malloc_philo_and_forks(t_sim *sim) {
    sim->philos = malloc(sizeof(t_philo) * sim->config.philo_num);
    sim->forks = malloc(sizeof(pthread_mutex_t) * sim->config.philo_num);
}

void cleanup(t_sim sim) {
    free(sim.philos);
    free(sim.forks);
}


int main(int ac, char **av) {
    t_sim sim;
    int i;

    if (ac < 5 || ac > 6) {
        print_err("Wrong arguments. Use it with:\n./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]");
        return 1;
    }

    log_with_timestamp(0, "", 1);
    sim.config.philo_num = atoi(av[1]);
    sim.config.time_to_die = atoi(av[2]);
    sim.config.time_to_eat = atoi(av[3]);
    sim.config.time_to_sleep = atoi(av[4]);
    if (ac == 6)
        sim.config.must_eat_count = atoi(av[5]);
    else
        sim.config.must_eat_count = 0;

    malloc_philo_and_forks(&sim);
    printf("Program started\n");
    i = 0;
    while (i < sim.config.philo_num) {
        sim.philos[i].id = i;
        sim.philos[i].meals_eaten = 0;
        sim.philos[i].sim = &sim;
        pthread_create(&sim.philos[i].thread, NULL, worker, &sim.philos[i]);
        i++;
    }

    i = 0;
    while (i < sim.config.philo_num) {
        pthread_join(sim.philos[i].thread, NULL);
        i++;
    }
    cleanup(sim);
    printf("Program finished\n");
    return 0;
}
