#include <stdio.h>
#include <unistd.h>

void print_err(char *s) {
    while (s && *s)
        write(2, s++, 1);
    write(2, "\n", 1);
}

int main(int ac, char **av) {
    (void)av;
    if (ac <= 3 || ac >= 4) {
        print_err("Wrong arguments. Use it with:\n./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]");
        return 1;
    }
    return 0;
}
