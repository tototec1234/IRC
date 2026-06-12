/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_isprint.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/14 18:19:55 by torinoue          #+#    #+#             */
/*   Updated: 2023/03/19 10:54:11 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_isprint(int c)
{
	return (040 <= c && c <= 0176);
}

//#include <stdio.h>
//#include <ctype.h>
//int	main(void)
//{
//	unsigned char c;
//	scanf("%c",&c);
//	/* c = 0; */
//	printf("character    = %c\n\n",c);
//	printf("Decimal      = %d\n",c);
//	printf("HexaDecimal  = %x\n",c);
//	printf("OctaDlecimal = %o\n\n",c);
//	printf("ft_isprint() = %d\n",ft_isprint(c));
//	printf("isprint() = %d\n",isprint(c));
//	return(0);
//}