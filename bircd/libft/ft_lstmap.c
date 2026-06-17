/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstmap.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <torinoue@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/30 16:01:10 by torinoue          #+#    #+#             */
/*   Updated: 2023/10/01 16:53:02 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

t_list	*ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *))
{
	t_list	*map;
	t_list	*current_node;

	if (lst == NULL || f == NULL || del == NULL)
		return (NULL);
	map = ft_lstnew(f(lst->content));
	if (map == NULL)
		return (NULL);
	current_node = map;
	lst = lst->next;
	while (lst)
	{
		current_node->next = ft_lstnew((*f)(lst->content));
		if (current_node->next == NULL)
		{
			ft_lstclear(&map, del);
			return (NULL);
		}
		current_node = current_node->next;
		lst = lst->next;
	}
	return (map);
}

// Parameters
// 	lst : The address of a pointer to a node.
// 	f   : The address of the function used to iterate on the list.
//  del : The address of the function used to delete the content of a node
// 		  if needed.
// 
// Return value
//  The new list.
//  NULL if the allocation fails.
// 
// External functs.
//  malloc, free
// 
// Description
// 	 Iterates the list ’lst’ and applies the function ’f’ 
// 	on the content of each node.
//  Creates a new list resulting of the successive applications
//  of the function ’f’.
//  The ’del’ function is used to delete the content of a node if needed.