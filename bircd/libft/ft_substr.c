/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_substr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/02 22:47:21 by torinoue          #+#    #+#             */
/*   Updated: 2023/03/26 15:55:03 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static size_t	ft_min(size_t x, size_t y)
{
	return ((x < y) * x + (y <= x) * y);
}

char	*ft_substr(char const *s, unsigned int start, size_t len)
{
	char	*substr;
	size_t	substr_len;

	if (s == NULL)
		return (NULL);
	substr_len = (ft_strlen(s) - start) * (start < ft_strlen(s));
	substr_len = ft_min(substr_len, len);
	substr = (char *)ft_calloc(substr_len + 1, sizeof(char));
	if (substr == NULL)
		return (NULL);
	ft_strlcpy(substr, s + start, substr_len + 1);
	return (substr);
}

////gcc ft_substr.c ft_strlcpy.c ft_strlen.c ft_calloc.c ft_bzero.c ft_memset.c
//#include <string.h>
//#include <stdio.h>
//int main(void)
//{
//	char test[] = "abcdefg";
//	unsigned int start = 100000;
//	size_t len	= 2  ;
//	char *sub = ft_substr( test, start, len );
//	printf("test[%s], start : %u, len : %zu, sub[%s]\n", test, start, len, sub);
//	return 0;
//}