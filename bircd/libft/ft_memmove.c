/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memmove.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/14 18:19:55 by torinoue          #+#    #+#             */
/*   Updated: 2023/03/21 19:12:15 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static void	backword_memcpy(void *dst, const void *src, size_t len)
{
	unsigned char		*uchar_dst;
	const unsigned char	*uchar_src = (const unsigned char *)src + len;

	uchar_dst = (unsigned char *)dst + len;
	while (len-- > 0)
		*--uchar_dst = *--uchar_src;
	return ;
}

void	*ft_memmove(void *dst, const void *src, size_t len)
{
	if (dst < src)
		ft_memcpy(dst, src, len);
	else if (dst > src)
		backword_memcpy(dst, src, len);
	return (dst);
}
