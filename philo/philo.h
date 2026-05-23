#ifndef PHILO_H
# define PHILO_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/time.h>
# include <unistd.h>

typedef struct s_sim	t_sim;

typedef struct s_config
{
	int		philo_num;
	long	time_to_die;
	long	time_to_eat;
	long	time_to_sleep;
	int		must_eat_count;
}	t_config;

typedef struct s_philo
{
	int				id;
	int				meals_eaten;
	long			last_meal_time;
	pthread_t		thread;
	pthread_mutex_t	*left_fork;
	pthread_mutex_t	*right_fork;
	t_sim			*sim;
}	t_philo;

struct s_sim
{
	t_config		config;
	t_philo			*philos;
	pthread_mutex_t	*forks;
	pthread_mutex_t	print_mutex;
	pthread_mutex_t	stop_mutex;
	long			start_time;
	int				should_stop;
};

void	print_err(char *s);
void	print_string(char *s);
int		ft_atoi(const char *nptr);
long	get_time_ms(void);
int		get_should_stop(t_sim *sim);
void	log_with_timestamp(t_sim *sim, int id, char *s);
void	print_action(t_sim *sim, int id, char *s);
void	set_should_stop(t_sim *sim);
void	*worker(void *arg);
void	malloc_philo_and_forks(t_sim *sim);
void	init(t_sim *sim);
void	cleanup(t_sim *sim);
int		is_args_positiv_int(int ac, char **av);

#endif
