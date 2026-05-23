/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssharmaz <ssharmaz@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/12 10:42:00 by ssharmaz          #+#    #+#             */
/*   Updated: 2026/03/29 18:19:34 by ssharmaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	malloc_philo_and_forks(t_sim *sim)
{
	sim->philos = malloc(sizeof(t_philo) * sim->config.philo_num);
	if (!sim->philos)
		return (0);
	sim->forks = malloc(sizeof(pthread_mutex_t) * sim->config.philo_num);
	if (!sim->forks)
	{
		free(sim->philos);
		return (0);
	}
	return (1);
}

static void	set_forks(t_sim *sim, int i)
{
	if (i % 2)
	{
		sim->philos[i].first_fork = &sim->forks[i];
		sim->philos[i].second_fork = &sim->forks[(i + 1)
			% sim->config.philo_num];
	}
	else
	{
		sim->philos[i].first_fork = &sim->forks[(i + 1)
			% sim->config.philo_num];
		sim->philos[i].second_fork = &sim->forks[i];
	}
}

void	init_philos(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.philo_num)
	{
		sim->philos[i].id = i + 1;
		sim->philos[i].meals_eaten = 0;
		sim->philos[i].last_meal_time = sim->start_time;
		sim->philos[i].sim = sim;
		pthread_mutex_init(&sim->philos[i].meal_mutex, NULL);
		set_forks(sim, i);
		i++;
	}
}

int	init(t_sim *sim)
{
	int	i;

	i = 0;
	sim->start_time = get_time_ms();
	sim->should_stop = 0;
	pthread_mutex_init(&sim->print_mutex, NULL);
	pthread_mutex_init(&sim->stop_mutex, NULL);
	if (!malloc_philo_and_forks(sim))
		return (0);
	while (i < sim->config.philo_num)
		pthread_mutex_init(&sim->forks[i++], NULL);
	init_philos(sim);
	return (1);
}

void	cleanup(t_sim *sim)
{
	int	i;

	i = 0;
	pthread_mutex_destroy(&sim->print_mutex);
	pthread_mutex_destroy(&sim->stop_mutex);
	while (i < sim->config.philo_num)
	{
		pthread_mutex_destroy(&sim->forks[i]);
		pthread_mutex_destroy(&sim->philos[i].meal_mutex);
		i++;
	}
	free(sim->philos);
	free(sim->forks);
}
