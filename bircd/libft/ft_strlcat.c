/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlcat.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <torinoue@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/14 18:19:55 by torinoue          #+#    #+#             */
/*   Updated: 2023/10/01 16:56:07 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

size_t	ft_strlcat(char *dst, const char *src, size_t dstsize)
{
	size_t	i;

	i = 0;
	while (i < dstsize && dst[i])
		++i;
	return (i + ft_strlcpy(dst + i, src, dstsize - i));
}

//size_t	ft_strlcat(char *dst, const char *src, size_t dstsize)
//{
//	size_t	s_len;
//	size_t	d_len;
//	size_t	cpid_len;
//	size_t	res_len;
// 
//	s_len = ft_strlen(src);
//	if (dstsize == 0 && dst == NULL)
//		return (s_len);
//	d_len = ft_strlen(dst);
//	dst += d_len;
//	cpid_len = 0;
//	while ((*src != 0) && (d_len + cpid_len++ + 1 < dstsize))
//		*dst++ = *src++;
//	*dst = '\0';
//	res_len = s_len + (dstsize <= d_len) * dstsize + (dstsize > d_len) * d_len;
//	return (res_len);
//}
