#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

long get_time_ms(void) {
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000L) + (tv.tv_usec / 1000L);
}

int main(void) {
    long start;
    long now;

    start = get_time_ms();
    usleep(500000);
    now = get_time_ms();

    printf("Elapsed: %ld ms\n", now - start);
    return 0;
}
