/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memcpy.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/14 18:19:55 by torinoue          #+#    #+#             */
/*   Updated: 2023/03/26 15:36:21 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memcpy(void *dst, const void *src, size_t n)
{
	unsigned char		*uchr_dst;
	const unsigned char	*uchr_src = (const unsigned char *)src;

	if (dst == src)
		return (dst);
	uchr_dst = (unsigned char *)dst;
	while (n-- > 0)
			*uchr_dst++ = *uchr_src++;
	return (dst);
}

//void	*ft_memcpy(void *dst, const void *src, size_t n)
//{
//	unsigned char		*uchr_dst;
//	const unsigned char	*uchr_src = (const unsigned char *)src;

//	if (dst == NULL && src == NULL)
//		return (NULL);
//	uchr_dst = (unsigned char *)dst;
//	while (n-- > 0)
//			*uchr_dst++ = *uchr_src++;
//	return (dst);
//}
//#include <string.h>
//#include <stdio.h>
//int main()
//{
//	printf("%s",memcpy(     ((void *)0)      , ((void *)0)          , 3)  );
//	printf("%s",ft_memcpy(     ((void *)0)      , ((void *)0)          , 3)  );
//			return 0;
//}
//while (n > 0)
//{
//	*d = *s;
//	++d;
//	++s;
//	--n;
//}