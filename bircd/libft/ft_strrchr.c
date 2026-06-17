/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strrchr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/23 23:03:01 by torinoue          #+#    #+#             */
/*   Updated: 2023/03/19 10:55:00 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strrchr(const char *s, int c)
{
	const char	*s_last;

	s_last = s + ft_strlen(s);
	while (1)
	{
		if (*s_last == (char)c)
			return ((char *)s_last);
		if (s_last == s)
			return (NULL);
		s_last--;
	}
}

//#include<stdio.h>
//#include<strings.h>
//#include<unistd.h>
//int main(int argc, char *argv[])
//{
//	int i;
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
//	cha = 'b';
//	printf("%c...%s in %s\n", cha, strchr( source_a, cha), source_a);
//	printf("%c...%s in %s\n", cha, ft_strchr( source_a, cha), source_a);
//	cha = 'x';
//	printf("%c...%s in %s\n", cha, strchr( source_a, cha), source_a);
//	printf("%c...%s in %s\n", cha, ft_strchr( source_a, cha), source_a);
//	cha = 0;
//	printf("%c...%s in %s\n", cha, strchr( source_a, cha), source_a);
//	printf("%c...%s in %s\n", cha, ft_strchr( source_a, cha), source_a);
//	write(1, "\n", 1);
//	char source_b[] = "";
//	cha = 'a';
//	printf("%c...%s in %s\n", cha, strchr( source_b, cha), source_b);
//	printf("%c...%s in %s\n", cha, ft_strchr( source_b, cha), source_b);
//	cha = 0;
//	printf("%c...%s in %s\n", cha, strchr( source_b, cha), source_b);
//	printf("%c...%s in %s\n", cha, ft_strchr( source_b, cha), source_b);
//	write(1, "\n", 1);
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
// The strchr() function locates the first occurrence of c (converted
// to a char) in the string pointed to by s.
//       ------->return ((char *)s_last);
//  The terminating null character is considered to be part of the string;
// therefore if c is`\0', the functions locate the terminating `\0'.
//
//	The strrchr() function is identical to strchr(), except it locates
// the last occurrence of c.
//	
//
//	 RETURN VALUES
//     The functions strchr() and strrchr() return a pointer to the located
//     character, or NULL if the character does not appear in the string.
//
//	s_last == s   means that  s_last is decremented and reached the first
// address of s without encountering c. In this case the return value is NULL.