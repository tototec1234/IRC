/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_isalnum.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/14 18:17:49 by torinoue          #+#    #+#             */
/*   Updated: 2023/02/13 20:00:28 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	ft_isupper(int c)
{
	return ('A' <= c && c <= 'Z');
}

static int	ft_islower(int c)
{
	return ('a' <= c && c <= 'z');
}

int	ft_isalnum(int c)
{
	return (ft_isupper (c) || ft_islower (c) || ft_isdigit (c));
}
//#include <stdio.h>
//#include <ctype.h>
//
//int	main(void)
//{
//	unsigned char c;
//	scanf("%c",&c);
//
//	printf("character    = %c\n\n",c);
//	printf("Decimal      = %d\n",c);
//	printf("HexaDecimal  = %x\n",c);
//	printf("OctaDlecimal = %o\n\n",c);
//	printf("ft_isalnum() = %d\n",ft_isalnum(c));
//	printf("isalnum() = %d\n",isalnum(c));
//
//	return(0);
//}
