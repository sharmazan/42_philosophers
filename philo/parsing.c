/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssharmaz <ssharmaz@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/12 10:42:00 by ssharmaz          #+#    #+#             */
/*   Updated: 2026/03/29 18:19:34 by ssharmaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	is_args_positiv_int(int ac, char **av)
{
	int	i;

	i = 1;
	while (i < ac)
	{
		if (!ft_atoi(av[i]) || ft_atoi(av[i]) == -1)
			return (0);
		i++;
	}
	return (1);
}

static void	set_config(t_sim *sim, int ac, char **av)
{
	sim->config.philo_num = ft_atoi(av[1]);
	sim->config.time_to_die = ft_atoi(av[2]);
	sim->config.time_to_eat = ft_atoi(av[3]);
	sim->config.time_to_sleep = ft_atoi(av[4]);
	if (ac == 6)
		sim->config.must_eat_count = ft_atoi(av[5]);
	else
		sim->config.must_eat_count = 0;
}

int	parse_args(t_sim *sim, int ac, char **av)
{
	if (ac < 5 || ac > 6)
	{
		print_err("Wrong arguments. Use it with:\n"
			"./philo number_of_philosophers time_to_die "
			"time_to_eat time_to_sleep "
			"[number_of_times_each_philosopher_must_eat]");
		return (0);
	}
	if (!is_args_positiv_int(ac, av))
	{
		print_err("All arguments should be positive integer");
		return (0);
	}
	if (ft_atoi(av[1]) > 1000)
	{
		print_err("Invalid number or philosophers. Maximum supported is 1000");
		return (0);
	}
	set_config(sim, ac, av);
	return (1);
}
