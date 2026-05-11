#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

void print_err(char *s) {
    while (s && *s)
        write(2, s++, 1);
    write(2, "\n", 1);
}

void *worker(void *arg) {
    int id = *(int *)arg;

    printf("Hello from thread %d\n", id);
    return NULL;
}

int main(int ac, char **av) {
    printf("Arguments amount: %d\n", ac);
    (void)av;
    if (ac < 5 || ac > 6) {
        print_err("Wrong arguments. Use it with:\n./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]");
        return 1;
    }

    pthread_t thread[5];
    int id = 0;
    while (id < 5) {
        pthread_create(&thread[id], NULL, worker, &id);
        id++;
    }

    id = 0;
    while (id < 5) {
        pthread_join(thread[id], NULL);
        id++;
    }

    return 0;
}
