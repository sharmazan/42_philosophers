/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssharmaz <ssharmaz@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/12 10:42:00 by ssharmaz          #+#    #+#             */
/*   Updated: 2026/03/29 18:19:34 by ssharmaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	all_philos_ate(t_sim *sim)
{
	int	i;

	if (!sim->config.must_eat_count)
		return (0);
	i = 0;
	while (i < sim->config.philo_num)
	{
		if (get_philo_meals_eaten(&sim->philos[i])
			< sim->config.must_eat_count)
			return (0);
		i++;
	}
	return (1);
}

static int	philo_died(t_sim *sim, int i)
{
	long	now;
	long	last_meal;

	now = get_time_ms();
	last_meal = get_philo_last_meal_time(&sim->philos[i]);
	if (now - last_meal > sim->config.time_to_die)
	{
		set_should_stop(sim);
		log_with_timestamp(sim, sim->philos[i].id, "died");
		return (1);
	}
	return (0);
}

static int	check_deaths(t_sim *sim)
{
	int	i;

	i = 0;
	while (!get_should_stop(sim) && i < sim->config.philo_num)
	{
		if (philo_died(sim, i))
			return (1);
		i++;
	}
	return (0);
}

void	monitor(t_sim *sim)
{
	while (!get_should_stop(sim))
	{
		if (all_philos_ate(sim))
			set_should_stop(sim);
		else
			check_deaths(sim);
		usleep(500);
	}
}
