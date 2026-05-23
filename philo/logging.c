/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logging.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssharmaz <ssharmaz@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/12 10:42:00 by ssharmaz          #+#    #+#             */
/*   Updated: 2026/03/29 18:19:34 by ssharmaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	log_with_timestamp(t_sim *sim, int id, char *s)
{
	long	now;

	pthread_mutex_lock(&sim->print_mutex);
	now = get_time_ms();
	printf("%ld %d %s\n", now - sim->start_time, id, s);
	pthread_mutex_unlock(&sim->print_mutex);
}

void	print_action(t_sim *sim, int id, char *s)
{
	long	now;

	pthread_mutex_lock(&sim->print_mutex);
	if (!get_should_stop(sim))
	{
		now = get_time_ms();
		printf("%ld %d %s\n", now - sim->start_time, id, s);
	}
	pthread_mutex_unlock(&sim->print_mutex);
}

void	wait_after_thinking(t_philo *philo)
{
	long	delay;

	if (philo->sim->config.philo_num % 2 == 0)
		return ;
	delay = philo->sim->config.time_to_eat * 2;
	delay = delay - philo->sim->config.time_to_sleep;
	if (delay < 1)
		delay = 1;
	precise_sleep(philo->sim, delay);
}
