#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#define THREADS 5

void print_err(char *s) {
    while (s && *s)
        write(2, s++, 1);
    write(2, "\n", 1);
}

void *worker(void *arg) {
    int id = *(int *)arg;

    printf("Hello from thread %d\n", id);
    sleep(1);
    printf("Bye from thread %d\n", id);
    return NULL;
}

int main(int ac, char **av) {
    printf("Arguments amount: %d\n", ac);
    (void)av;
    if (ac < 5 || ac > 6) {
        print_err("Wrong arguments. Use it with:\n./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]");
        return 1;
    }

    pthread_t thread[THREADS];
    int ids[THREADS];
    int i = 0;

    printf("Program started\n");
    for (i = 0; i < THREADS; i++) {
        ids[i] = i;
        pthread_create(&thread[i], NULL, worker, &ids[i]);
    }

    i = 0;
    while (i < THREADS) {
        pthread_join(thread[i], NULL);
        // pthread_detach(thread[id]);
        i++;
    }
    // sleep(2);
    printf("Program finished\n");

    return 0;
}
