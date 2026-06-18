/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memchr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/22 12:12:07 by torinoue          #+#    #+#             */
/*   Updated: 2023/03/21 18:48:39 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memchr(const void *s, int c, size_t n)
{
	unsigned char	*s_ptr;

	s_ptr = (unsigned char *)s;
	while (n--)
	{
		if (*s_ptr == (unsigned char)c)
			return ((void *)s_ptr);
		s_ptr++;
	}
	return (NULL);
}

//man memchr | col -bx | fold -77 | sed -e "s/^/\/\/ /g" | pbcopy
//
// DESCRIPTION
//      The memchr() function locates the first occurrence of c (converted to an
//      unsigned char) in string s.
// 
// RETURN VALUES
//      The memchr() function returns a pointer to the byte located, or NULL if
//      no such byte exists within n bytes.
// 
// SEE ALSO
//      strchr(3), strcspn(3), strpbrk(3), strrchr(3), strsep(3), strspn(3),
//      strstr(3), strtok(3), wmemchr(3)
// 
// STANDARDS
//      The memchr() function conforms to ISO/IEC 9899:1990 (``ISO C90'').