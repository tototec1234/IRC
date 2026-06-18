/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstiter.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <torinoue@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/30 15:54:26 by torinoue          #+#    #+#             */
/*   Updated: 2023/10/01 14:27:41 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_lstiter(t_list *lst, void (*f)(void *))
{
	while (lst)
	{
		f(lst->content);
		lst = lst->next;
	}
}

// Parameters
// 	lst: The address of a pointer to a node.
// 	f: The address of the function used to iterate on the list.
//
// Return value
//   None
// 
// Description
// 	 Iterates the list ’lst’ and applies the function ’f’ 
// 	on the content of each node.
// 