/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/08 13:50:57 by torinoue          #+#    #+#             */
/*   Updated: 2023/03/25 17:48:04 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strtrim(char const *s1, char const *set)
{
	const char		*ptr = s1;
	const char		*start = NULL;
	const char		*end = s1;
	size_t			trimd_len;

	if (s1 == NULL)
		return (NULL);
	if (set == NULL)
		return (ft_strdup(s1));
	while (*ptr)
	{
		if ((start == NULL) && (ft_strchr(set, *ptr) == NULL))
			start = ptr;
		if (ft_strchr(set, *ptr) == NULL)
			end = ptr;
		ptr++;
	}
	if (start == NULL)
	{
		start = ptr;
		end = ptr;
	}
	trimd_len = (size_t)(end - start + 1);
	return (ft_substr(start, 0, trimd_len));
}

//gcc ft_strtrim.c ft_strlcpy.c ft_strlen.c ft_calloc.c 
//ft_bzero.c ft_memset.c ft_strchr.c ft_strdup.c ft_substr.c
//#include <stdio.h>
//#include <string.h>
//int main(void)
//{
//	char *test;
//	test = ft_strtrim("1x2x345x6x7x8xx9x10x", "x");
//	printf("0[%s]\n",test);
//	test = ft_strtrim("z,A,,BC,,,,D,,", ",");
//	printf("1[%s]\n",test);
//	test = ft_strtrim("xxxxx", "x");
//	printf("2[%s]\n",test);
//	test = ft_strtrim("", "");
//	printf("3[%s]\n",test);
//	test = ft_strtrim("x", "x");
//	printf("4[%s]\n",test);
//	test = ft_strtrim("xxAxx", "x");
//	printf("5[%s]\n",test);
//	test = ft_strtrim("heloehlohelohelo world", "ehlol");
//	printf("6[%s]\n",test);
//	test = ft_strtrim("hello world", "dlrow");
//	printf("7[%s]\n",test);
//	test = ft_strtrim("hello world", "hello");
//	printf("8[%s]\n",test);
//	test = ft_strtrim("hello world", "world");
//	printf("9[%s]\n",test);
//	return (0);
//}