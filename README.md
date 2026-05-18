# _This project has been created as part of the 42 curriculum by ssharmaz._

TODO
- arg verification
- negative or wrong arguments
- implement data structures
- main algo:
- - main process starts threads
- - each thread imitate a philosopher: eat, sleep, thinking
- - main process monitors state and set a flag to stop simulation
- verify one philo case
- add a Makefile
- add ft_atoi, remove atoi
- fix the norm
- verify with valgrind and static code analyzer
- remove the tester

## Description
The goal of this project is to learn the basics of threading a process, how to create threads and explore the use of mutexes.

## Rules
One or more philosophers sit at a round table.
There is a large bowl of spaghetti in the middle of the table.

- The philosophers take turns eating, sleeping, and thinking.
While they are eating, they are not thinking nor sleeping;
while thinking, they are not eating nor sleeping;
and, of course, while sleeping, they are not eating nor thinking.
- There are also forks on the table. There are as many forks as philosophers.
- Since eating spaghetti with just one fork is impractical, a philosopher must pick up
both the fork to their right and the fork to their left before eating.
- When a philosopher has finished eating, they put their forks back on the table and
start sleeping. Once awake, they start thinking again. The simulation stops when
a philosopher dies of starvation.
- Every philosopher needs to eat and should never starve.
- Philosophers do not communicate with each other.
- Philosophers do not know if another philosopher is about to die.
- Needless to say, philosophers should avoid dying!

- Global variables are forbidden!
- Each philosopher has a number ranging from 1 to number_of_philosophers.
- Philosopher number 1 sits next to philosopher number number_of_philosophers.
Any other philosopher, numbered N, sits between philosopher N - 1 and philosopher N + 1.

One philosopher has to die, because they could not eat with one fork.

### Mandatory part
- Each philosopher must be represented as a separate thread.
- There is one fork between each pair of philosophers. Therefore, if there are several philosophers, each philosopher has a fork on their left side and a fork on their right side. If there is only one philosopher, they will have access to just one fork.
- To prevent philosophers from duplicating forks, you should protect each fork’s state with a mutex.

### Bonus part
- All the forks are put in the middle of the table.
- They have no states in memory, but the number of available forks is represented by
a semaphore.
- Each philosopher must be represented as a separate process. However, the main
process should not act as a philosopher.
- Program Name: philo_bonus
- Makefile, *.h, *.c, in directory philo_bonus/

### Requirements

- no global variables
- no data races
- death message within 10 ms
- Makefile, *.h, *.c should be places inside philo/ for mandatory part

### Arguments
Program must take the following arguments:
number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]

- number_of_philosophers: The number of philosophers and also the number of forks.
- time_to_die (in milliseconds): If a philosopher has not started eating within time_to_die milliseconds since the start of their last meal or the start of the simulation, they die.
- time_to_eat (in milliseconds): The time it takes for a philosopher to eat.
During that time, they will need to hold two forks.
- time_to_sleep (in milliseconds): The time a philosopher will spend sleeping.
- number_of_times_each_philosopher_must_eat (optional argument): 
If all philosophers have eaten at least number_of_times_each_philosopher_must_eat
times, the simulation stops. 
If not specified, the simulation stops when a philosopher dies.

### Allowed external Functions
memset, 
malloc, free, 
printf, write, 
usleep, gettimeofday, 
pthread_create, pthread_detach, pthread_join, 
pthread_mutex_init, pthread_mutex_destroy, pthread_mutex_lock, pthread_mutex_unlock

## Instructions
To run the program:
```
cd philo
make
./philo 1 1000 100 200 
```

## Resources
man and ChatGPT to read about the functions to work with threads. 
AI was used to create a test suite, but not for the project's code itself. 

### Notes
```
void *worker(void *arg)
{
   int id;

   id = *(int *)arg;
   printf("id = %d\n", id);
   return NULL;
}


pthread_create(&thread, NULL, worker_function, &id);
pthread_join(thread, NULL);  // wait until worker finish the work


pthread_mutex_t counter_lock;
pthread_mutex_init(&counter_lock, NULL);
...
// in a thread to avoid race
pthread_mutex_lock(&counter_lock);
counter++;
pthread_mutex_unlock(&counter_lock);
...
pthread_mutex_destroy(&counter_lock);
```



## How to test

```
chmod +x tests/test_philo.sh
./tests/test_philo.sh ./philo/philo
```

For race-check
```
RUN_HELGRIND=1 ./tests/test_philo.sh ./philo/philo
```
