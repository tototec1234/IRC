/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_isdigit.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/14 18:20:31 by torinoue          #+#    #+#             */
/*   Updated: 2023/03/19 10:54:07 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_isdigit(int c)
{
	return ('0' <= c && c <= '9');
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
//	printf("ft_isdigit() = %d\n",ft_isdigit(c));
//	printf("isdigit() = %d\n",isdigit(c));
//
//	return(0);
//}