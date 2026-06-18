/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlen.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/14 18:19:55 by torinoue          #+#    #+#             */
/*   Updated: 2023/03/19 10:54:44 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

size_t	ft_strlen(const char *s)
{
	size_t	len;

	len = 0;
	while (*(s + len))
		++len;
	return (len);
}

//#include <stdio.h>
//#include <string.h>
//
//int	main(void)
//{
//	char	*mojiretu;
//
//	mojiretu = "ohayo";
//	printf("ft mojiretu:%zu \n", ft_strlen(mojiretu));
//	printf("   mojiretu:%lu \n",    strlen(mojiretu));
//	return (0);
//}