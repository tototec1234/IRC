/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strmapi.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/16 18:17:44 by torinoue          #+#    #+#             */
/*   Updated: 2023/04/18 20:47:22 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strmapi(char const *s, char (*f)(unsigned int, char))
{
	size_t	i;
	char	*mapped_str;

	if (s == NULL || f == NULL)
		return (NULL);
	mapped_str = ft_strdup(s);
	if (mapped_str == NULL)
		return (NULL);
	i = 0;
	while (mapped_str[i])
	{
		mapped_str[i] = f((unsigned int)i, mapped_str[i]);
		i++;
	}
	return (mapped_str);
}
