/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/09 23:32:45 by torinoue          #+#    #+#             */
/*   Updated: 2023/03/29 22:53:28 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

//Returns the length of a sep or '\0' terminated str.
static size_t	get_token_len(const char *str, char sep)
{
	size_t	token_len;

	token_len = 0;
	while (*str != '\0' && *str != sep)
	{
		token_len++;
		str++;
	}
	return (token_len);
}

//Rerurns the size of array needed to split s by sep.
static size_t	get_array_size(const char *s, char sep)
{
	size_t	array_size;

	array_size = 1;
	while (*s)
	{
		if (get_token_len(s, sep) > 0)
		{
			array_size++;
			s += get_token_len(s, sep);
		}
		else
			s++;
	}
	return (array_size);
}

static	char	**ft_all_free(char **arr)
{
	size_t	i;

	i = 0;
	while (arr[i] != NULL)
	{
		free(arr[i]);
		i++;
	}
	free(arr);
	return (NULL);
}

//ft_split splits the string s by c, terminating each split 
//segment with a null character."
char	**ft_split(char const *s, char c)
{
	char	**septed_arr;
	size_t	i;

	if (s == NULL)
		return (NULL);
	septed_arr = ft_calloc(get_array_size(s, c), sizeof(char *));
	if (septed_arr == NULL)
		return (NULL);
	i = 0;
	while (*s)
	{
		if (get_token_len(s, c) > 0)
		{
			septed_arr[i] = ft_substr (s, 0, get_token_len(s, c));
			if (septed_arr[i] == NULL)
				return (ft_all_free(septed_arr));
			s += get_token_len(s, c);
			i++;
		}
		else
			s++;
	}
	return (septed_arr);
}
