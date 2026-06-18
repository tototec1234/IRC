/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putnbr_fd.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/17 12:21:22 by torinoue          #+#    #+#             */
/*   Updated: 2023/04/18 20:46:52 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_putnbr_fd(int n, int fd)
{
	int	divisor;
	int	tmp_n;
	int	quotient;

	if (n == 0)
	{
		ft_putchar_fd('0', fd);
		return ;
	}
	if (n < 0)
		ft_putchar_fd('-', fd);
	tmp_n = n / 10;
	divisor = 1;
	while (tmp_n)
	{
		divisor *= 10;
		tmp_n /= 10;
	}
	while (divisor)
	{
		quotient = ((n >= 0) - (n < 0)) * (n / divisor) + '0';
		ft_putchar_fd(quotient, fd);
		n = n % divisor;
		divisor /= 10;
	}
}

//#include	<stdio.h>
//int	main(void)
//{
//		ft_putnbr_fd(-2147483648, 1);
//		printf("\n");
//		ft_putnbr_fd( 2147483647, 1);
//	return (0);
//}
