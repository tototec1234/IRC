/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strchr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/23 23:03:01 by torinoue          #+#    #+#             */
/*   Updated: 2023/03/25 17:49:31 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strchr(const char *s, int c)
{
	while (1)
	{
		if (*s == (char)c)
			return ((char *)s);
		if (*s == '\0')
			return (NULL);
		s++;
	}
}

//#include <stdio.h>
//int main(void)
//{
//	char *s1 = "a";
//	printf("%p\n", s1);
//	printf("%p\n", ft_strchr(s1, '\0'));
//}

// The strchr() function locates the first occurrence of c (converted
// to a char) in the string pointed to by s.
//  The terminating null character is considered to be part of the string;
// therefore if c is`\0', the functions locate the terminating `\0'.
//
//	 RETURN VALUES
//     The functions strchr() and strrchr() return a pointer to the located
//     character, or NULL if the character does not appear in the string.
//#include<stdio.h>
//#include<strings.h>
//#include<unistd.h>
//
//int main(int argc, char *argv[])
//{
//	int i;
//
//	
//	if ( argc == 3)
//		{
//		i = (int)(*argv[2]);	
//		printf("%c...%s in %s\n", i, strchr( argv[1], i), argv[1]);
//		printf("%c...%s in %s\n", i, ft_strchr( argv[1], i), argv[1]);
//	write(1, "\n", 1);
//	write(1, "\n", 1);
//		}	
//	char source_a[] = "abc";
//	int  cha = 'a';
//	printf("%c...%s in %s\n", cha, strchr( source_a, cha), source_a);
//	printf("%c...%s in %s\n", cha, ft_strchr( source_a, cha), source_a);
//
//	cha = 'b';
//	printf("%c...%s in %s\n", cha, strchr( source_a, cha), source_a);
//	printf("%c...%s in %s\n", cha, ft_strchr( source_a, cha), source_a);
//
//	cha = 'x';
//	printf("%c...%s in %s\n", cha, strchr( source_a, cha), source_a);
//	printf("%c...%s in %s\n", cha, ft_strchr( source_a, cha), source_a);
//
//
//	cha = 0;
//	printf("%c...%s in %s\n", cha, strchr( source_a, cha), source_a);
//	printf("%c...%s in %s\n", cha, ft_strchr( source_a, cha), source_a);
//
//	write(1, "\n", 1);
//
//	char source_b[] = "";
//	cha = 'a';
//	printf("%c...%s in %s\n", cha, strchr( source_b, cha), source_b);
//	printf("%c...%s in %s\n", cha, ft_strchr( source_b, cha), source_b);
//
//
//	cha = 0;
//	printf("%c...%s in %s\n", cha, strchr( source_b, cha), source_b);
//	printf("%c...%s in %s\n", cha, ft_strchr( source_b, cha), source_b);
//
//	write(1, "\n", 1);
//
//	char *source_c;
//	cha = 0;
//	strchr( source_c, cha);
//	//printf("%c...%s in %s\n", cha, strchr( source_c, cha), source_c);
//	//printf("%c...%s in %s\n", cha, ft_strchr( source_c, cha), source_c);
//	//cha = 'a';
//	//printf("%c...%s in %s\n", cha, strchr( source_c, cha), source_c);
//	//printf("%c...%s in %s\n", cha, ft_strchr( source_c, cha), source_c);
//	return 0;
//}
//