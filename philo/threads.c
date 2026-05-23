/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   threads.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssharmaz <ssharmaz@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/12 10:42:00 by ssharmaz          #+#    #+#             */
/*   Updated: 2026/03/29 18:19:34 by ssharmaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	start_threads(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.philo_num)
	{
		if (pthread_create(&sim->philos[i].thread, NULL,
				worker, &sim->philos[i]) != 0)
		{
			set_should_stop(sim);
			return (0);
		}
		i++;
	}
	return (1);
}

void	join_philos(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.philo_num)
	{
		pthread_join(sim->philos[i].thread, NULL);
		i++;
	}
}
