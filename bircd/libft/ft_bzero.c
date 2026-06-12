/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_bzero.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/14 18:19:55 by torinoue          #+#    #+#             */
/*   Updated: 2023/03/25 19:33:30 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_bzero(void *s, size_t n)
{
	ft_memset(s, 0, n);
	return ;
}

//#include <stdio.h>
//#include <ctype.h>
//
//int	main(void)
//{
//	void	*s;
//	s = "abcdef";
//	size_t	n;
//	n = 0;
//	ft_bzero(s, n);
//	return (0);
//}