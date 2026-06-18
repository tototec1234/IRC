/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_isascii.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/14 18:19:55 by torinoue          #+#    #+#             */
/*   Updated: 2023/02/28 01:02:03 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_isascii(int c)
{
	return (00 <= c && c <= 0177);
}
//#include <stdio.h>
//#include <ctype.h>
//
//int	main(void)
//{
//	unsigned char c;
//	scanf("%c",&c);
//
//	/* c = 0; */
//
//	printf("character    = %c\n\n",c);
//	printf("Decimal      = %d\n",c);
//	printf("HexaDecimal  = %x\n",c);
//	printf("OctaDlecimal = %o\n\n",c);
//	printf("ft_isascii() = %d\n",ft_isascii(c));
//	printf("isascii() = %d\n",isascii(c));
//
//	return(0);
//}
