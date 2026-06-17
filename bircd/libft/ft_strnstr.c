/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strnstr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue  <torinoue@student.42tokyo.>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/30 20:45:46 by torinoue          #+#    #+#             */
/*   Updated: 2023/09/30 21:08:34 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strnstr(const char *haystack, const char *needle, size_t len)
{
	size_t	needle_len;

	needle_len = ft_strlen(needle);
	if (*needle == '\0')
		return ((char *)haystack);
	while (len)
	{
		if (*haystack == '\0')
			break ;
		if (needle_len > len)
			break ;
		if (ft_strncmp(haystack, needle, needle_len) == 0)
			return ((char *)haystack);
		len--;
		haystack++;
	}
	return (NULL);
}

// man strnstr | col -bx | fold -77 | sed "s/^/\/\\/\ /g" | pbcopy
// 
// SYNOPSIS
//      #include <string.h>
// 
//      strnstr(const char *haystack, const char *needle, size_t len);
// 
//      #include <string.h>
//      #include <xlocale.h>
//  
// DESCRIPTION
// 
//      The strnstr() function locates the first occurrence of the null-termi-
//      nated string needle in the string haystack, where not more than len char
//      acters are searched.
//       Characters that appear after a `\0' character are not searched.
//      Since the strnstr() function is a FreeBSD specific API, it
//      should only be used when portability is not a concern.
//  
// RETURN VALUES
//      If needle is an empty string, haystack is returned; if needle occurs
//      nowhere in haystack, NULL is returned; otherwise a pointer to the first
//      character of the first occurrence of needle is returned.
// 
// char	*ft_strnstr(const char *haystack, const char *needle, size_t len)
// {
// 	size_t	needle_len;

// 	needle_len = ft_strlen(needle);
// 	if (*needle == '\0')
// 		return ((char *)haystack);
// 	while (len)
// 	{
// 		if (*haystack == '\0')
// 			break ;
// 		if (needle_len > len)
// 			break ;
// 		// if (*needle == '\0')			/////////////////////
// 		// 	return ((char *)haystack);	/////////////////////
// 		if (ft_strncmp(haystack, needle, needle_len) == 0)
// 			return ((char *)haystack);
// 		len--;
// 		haystack++;
// 	}
// 	return (NULL);
// }