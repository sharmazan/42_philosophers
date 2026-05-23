/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   state.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssharmaz <ssharmaz@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/12 10:42:00 by ssharmaz          #+#    #+#             */
/*   Updated: 2026/03/29 18:19:34 by ssharmaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	get_should_stop(t_sim *sim)
{
	int	stop;

	pthread_mutex_lock(&sim->stop_mutex);
	stop = sim->should_stop;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (stop);
}

void	set_should_stop(t_sim *sim)
{
	pthread_mutex_lock(&sim->stop_mutex);
	sim->should_stop = 1;
	pthread_mutex_unlock(&sim->stop_mutex);
}

int	get_philo_meals_eaten(t_philo *philo)
{
	int	value;

	pthread_mutex_lock(&philo->meal_mutex);
	value = philo->meals_eaten;
	pthread_mutex_unlock(&philo->meal_mutex);
	return (value);
}

long	get_philo_last_meal_time(t_philo *philo)
{
	long	value;

	pthread_mutex_lock(&philo->meal_mutex);
	value = philo->last_meal_time;
	pthread_mutex_unlock(&philo->meal_mutex);
	return (value);
}

void	precise_sleep(t_sim *sim, long duration_ms)
{
	long	end_time;

	end_time = get_time_ms() + duration_ms;
	while (!get_should_stop(sim) && get_time_ms() < end_time)
		usleep(500);
}
