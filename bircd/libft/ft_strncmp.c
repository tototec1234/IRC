/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strncmp.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/26 17:46:52 by torinoue          #+#    #+#             */
/*   Updated: 2023/03/25 15:27:32 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_strncmp(const char *s1, const char *s2, size_t n)
{
	size_t	i;

	i = 0;
	if (n == 0)
		return (0);
	while (i + 1 < n && s1[i] && s2[i] && s1[i] == s2[i])
		i++;
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

// man strncmp | col -bx | fold -77 | sed "s/^/\/\\/\ /g" | pbcopy
//
// DESCRIPTION
//      The strcmp() and strncmp() functions lexicographically compare the null-
//      terminated strings s1 and s2.
// 
//      The strncmp() function compares not more than n characters.  Because
//      strncmp() is designed for comparing strings rather than binary data,
//      characters that appear after a `\0' character are not compared.
// 
// RETURN VALUES
//      The strcmp() and strncmp() functions return an integer greater than,
//      equal to, or less than 0, according as the string s1 is greater than,
//      equal to, or less than the string s2.  The comparison is done using
//      unsigned characters, so that `\200' is greater than `\0'.
// 
//#include<stdio.h>
//#include<string.h>
//int main(void)
//{
// printf("   strncmp(\"\\0\", \"\\0\",0)) = %d\n", strncmp("\0", "\0",0));
// printf("   strncmp(\"\\0\", \"\\0\",1)) = %d\n", strncmp("\0", "\0",1));
// printf("   strncmp(\"1\\0\",\"\\0\",1)) = %d\n", strncmp("1\0", "\0",1));
// printf("   strncmp(\"\\0\", \"1\\0\",1) = %d\n", strncmp("\0", "1\0",1));
// printf("------------------------------\n");
// printf("ft_strncmp(\"\\0\", \"\\0\",0)) = %d\n", ft_strncmp("\0", "\0",0));
// printf("ft_strncmp(\"\\0\", \"\\0\",1)) = %d\n", ft_strncmp("\0", "\0",1));
// printf("ft_strncmp(\"1\\0\",\"\\0\",1)) = %d\n", ft_strncmp("1\0", "\0",1));
// printf("ft_strncmp(\"\\0\", \"1\\0\",1)) = %d\n", ft_strncmp("\0", "1\0",1));
// printf("------------------------------\n");
// printf("   strncmp(\"1234\", \"12\",0)) = %d\n", strncmp("1234","12",0));
// printf("   strncmp(\"1234\", \"11\",1)) = %d\n", strncmp("1234","11",1));
// printf("   strncmp(\"1234\", \"11\",2)) = %d\n", strncmp("1234","11",2));
// printf("   strncmp(\"1234\", \"13\",2)) = %d\n", strncmp("1234","13",2));
// printf("   strncmp(\"1234\", \"\\09\",2)) = %d\n", strncmp("1234","\09",2));
// printf("   strncmp(\"1234\", NULL,0)) = %d\n", strncmp("1234",NULL,0));
// printf("------------------------------\n");
// printf("ft_strncmp(\"1234\", \"12\",0)) = %d\n", ft_strncmp("1234","12",0));
// printf("ft_strncmp(\"1234\", \"11\",1)) = %d\n", ft_strncmp("1234","11",1));
// printf("ft_strncmp(\"1234\", \"11\",2)) = %d\n", ft_strncmp("1234","11",2));
// printf("ft_strncmp(\"1234\", \"13\",2)) = %d\n", ft_strncmp("1234","13",2));
// printf("ft_strncmp(\"1234\",\"\\09\",2)) = %d\n",ft_strncmp("1234","\09",2));
// printf("ft_strncmp(\"1234\", NULL,0)) = %d\n",ft_strncmp("1234",NULL,0));
//}