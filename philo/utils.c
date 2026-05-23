/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssharmaz <ssharmaz@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/12 10:42:00 by ssharmaz          #+#    #+#             */
/*   Updated: 2026/03/29 18:19:34 by ssharmaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	print_err(char *s)
{
	while (s && *s)
		write(2, s++, 1);
	write(2, "\n", 1);
}

long	get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000L) + (tv.tv_usec / 1000L));
}

int	ft_atoi(const char *nptr)
{
	long	i;
	int		digit;

	i = 0;
	if (!nptr || !*nptr)
		return (-1);
	if (*nptr == '+')
		nptr++;
	if (!*nptr || *nptr == '-')
		return (-1);
	while (*nptr == '0')
		nptr++;
	while (*nptr >= '0' && *nptr <= '9')
	{
		digit = *nptr - '0';
		i = i * 10 + digit;
		nptr++;
	}
	if (*nptr || i > 2147483647)
		return (-1);
	return (i);
}
