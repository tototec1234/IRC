/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_isalpha.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/14 18:19:18 by torinoue          #+#    #+#             */
/*   Updated: 2023/02/28 01:01:49 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	ft_isupper(int c)
{
	return (c >= 0141 && c <= 0172);
}

static int	ft_islower(int c)
{
	return (c >= 0101 && c <= 0132);
}

int	ft_isalpha(int c)
{
	return (ft_isupper (c) || ft_islower (c));
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
//	printf("ft_isalpha() = %d\n",ft_isalpha(c));
//	printf("isalpha() = %d\n",isalpha(c));
//
//	return(0);
//}
