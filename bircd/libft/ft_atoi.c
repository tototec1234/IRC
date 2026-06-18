/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoi.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <torinoue@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/22 12:12:07 by torinoue          #+#    #+#             */
/*   Updated: 2023/10/21 20:49:55 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_atoi(const char *str)
{
	int			sign;
	long		value;
	size_t		i;

	value = 0;
	i = 0;
	while (ft_strchr("\t\n\v\f\r ", str[i]) != NULL)
		i++;
	sign = (str[i] != '-') - (str[i] == '-');
	i += (str[i] == '-' || str[i] == '+');
	while (ft_isdigit(str[i]))
	{
		if ((value > LONG_MAX / 10)
			|| (value == (LONG_MAX / 10) && ((str[i] - '0') > (LONG_MAX % 10))))
			return ((sign == 1) * ((int)LONG_MAX)
				+ (sign == -1) * ((int)LONG_MIN));
		value = value * 10 + (str[i++] - '0');
	}
	return ((int)(value * sign));
}

// git clone https://github.com/usatie/libft-tester-tokyo.git
// cd libft-tester-tokyo ; make
// 
// git clone https://github.com/Tripouille/libftTester.git
// cd libftTester ; make 
// 
// git clone https://github.com/alelievr/libft-unit-test.git
// cd libft-unit-test ; make f ; make
// 
// 
// #include	<stdio.h>
// int	main(int argc, char **argv)
// {
// 	if (argc == 2)
// 		printf("%d\n", ft_atoi(argv[1]));
// 	return (0);
// }