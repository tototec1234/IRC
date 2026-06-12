/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/22 12:12:07 by torinoue          #+#    #+#             */
/*   Updated: 2023/03/19 18:40:44 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static size_t	get_len(int n)
{
	size_t	len;

	len = (n <= 0);
	while (n)
	{
		len++;
		n /= 10;
	}
	return (len);
}

char	*ft_itoa(int n)
{
	int		sign_check;
	size_t	len;
	char	*s;

	sign_check = n;
	len = get_len(n);
	s = (char *)ft_calloc(len + 1, sizeof(char));
	if (s == NULL)
		return (NULL);
	while (len--)
	{
		s[len] = ((n >= 0) - (n < 0)) * (n % 10) + '0';
		n /= 10;
	}
	if (sign_check < 0)
		*s = '-';
	return (s);
}
