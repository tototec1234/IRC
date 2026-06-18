/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstclear.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <torinoue@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/30 15:36:50 by torinoue          #+#    #+#             */
/*   Updated: 2023/10/01 21:26:41 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_lstclear(t_list **lst, void (*del)(void *))
{
	t_list	*current_lst;

	if (lst == NULL || del == NULL)
		return ;
	while (*lst)
	{
		current_lst = *lst;
		*lst = (*lst)->next;
		ft_lstdelone(current_lst, del);
	}
}

// void	ft_lstclear(t_list **lst, void (*del)(void *))
// {
// 	t_list	*current_lst;
// 
// 	if (lst == NULL)   /////////////////////////////////
// 		return ;
// 	while (*lst)
// 	{
// 		current_lst = *lst;
// 		*lst = (*lst)->next;
// 		ft_lstdelone(current_lst, del);
// 	}
// }
// 
// Parameters
// 	lst: The address of a pointer to a node.
// 	del: The address of the function used to delete the content of the node.
// 
// Return value
// 	None
// 
// Description
// 	Deletes and frees the given node and every successor of that node,
// 	using the function ’del’ and free(3).
// 	Finally, the pointer to the list must be set to NULL.
