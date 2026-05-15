#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#define THREADS 4
#define ITERATIONS 1000000

int counter = 0;
pthread_mutex_t counter_lock;

typedef struct s_task  {
    int id;
    char *message;
} t_task;

void print_err(char *s) {
    while (s && *s)
        write(2, s++, 1);
    write(2, "\n", 1);
}

void *worker(void *arg) {
    t_task *task;

    task = (t_task *)arg;
    printf("Hello from thread %d\n", task->id);
    sleep(1);
    printf("thread %d message: %s\n", task->id, task->message);
    printf("Bye from thread %d\n", task->id);
    return NULL;
}

void *counter_worker(void *arg) {
    (void)arg;
    int i=0;
    while (i++ < ITERATIONS) {
        pthread_mutex_lock(&counter_lock);
        counter++;
        pthread_mutex_unlock(&counter_lock);
    }
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
    pthread_mutex_init(&counter_lock, NULL);
    int ids[THREADS];
    int i = 0;
    struct s_task task;
    task.id = 1;
    task.message = "Just hi";

    printf("Program started\n");
    (void)ids;
    for (i = 0; i < THREADS; i++) {
        pthread_create(&thread[i], NULL, counter_worker, &task);
    }

    i = 0;
    while (i < THREADS) {
        pthread_join(thread[i], NULL);
        // pthread_detach(thread[id]);
        i++;
    }
    // sleep(2);
    printf("Expected: %d\n", THREADS * ITERATIONS);
    printf("Actual: %d\n", counter);

    pthread_mutex_destroy(&counter_lock);
    printf("Program finished\n");

    return 0;
}
