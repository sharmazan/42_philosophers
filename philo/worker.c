/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   worker.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssharmaz <ssharmaz@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/12 10:42:00 by ssharmaz          #+#    #+#             */
/*   Updated: 2026/03/29 18:19:34 by ssharmaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static void	single_philo(t_philo *philo)
{
	pthread_mutex_lock(philo->first_fork);
	print_action(philo->sim, philo->id, "has taken a fork");
	while (!get_should_stop(philo->sim))
		usleep(1000);
	pthread_mutex_unlock(philo->first_fork);
}

static int	take_forks(t_philo *philo)
{
	pthread_mutex_lock(philo->first_fork);
	if (get_should_stop(philo->sim))
	{
		pthread_mutex_unlock(philo->first_fork);
		return (0);
	}
	print_action(philo->sim, philo->id, "has taken a fork");
	pthread_mutex_lock(philo->second_fork);
	if (get_should_stop(philo->sim))
	{
		pthread_mutex_unlock(philo->second_fork);
		pthread_mutex_unlock(philo->first_fork);
		return (0);
	}
	print_action(philo->sim, philo->id, "has taken a fork");
	return (1);
}

static void	eat(t_philo *philo)
{
	pthread_mutex_lock(&philo->meal_mutex);
	philo->last_meal_time = get_time_ms();
	pthread_mutex_unlock(&philo->meal_mutex);
	print_action(philo->sim, philo->id, "is eating");
	precise_sleep(philo->sim, philo->sim->config.time_to_eat);
	pthread_mutex_unlock(philo->first_fork);
	pthread_mutex_unlock(philo->second_fork);
	pthread_mutex_lock(&philo->meal_mutex);
	philo->meals_eaten++;
	pthread_mutex_unlock(&philo->meal_mutex);
}

static void	rest_and_think(t_philo *philo)
{
	if (get_should_stop(philo->sim))
		return ;
	print_action(philo->sim, philo->id, "is sleeping");
	precise_sleep(philo->sim, philo->sim->config.time_to_sleep);
	if (get_should_stop(philo->sim))
		return ;
	print_action(philo->sim, philo->id, "is thinking");
	wait_after_thinking(philo);
}

void	*worker(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	if (philo->sim->config.philo_num == 1)
	{
		single_philo(philo);
		return (NULL);
	}
	while (!get_should_stop(philo->sim))
	{
		if (!take_forks(philo))
			break ;
		eat(philo);
		rest_and_think(philo);
	}
	return (NULL);
}
