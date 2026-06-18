/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstadd_back.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <torinoue@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/29 21:28:57 by torinoue          #+#    #+#             */
/*   Updated: 2023/10/21 20:48:11 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_lstadd_back(t_list **lst, t_list *new)
{
	t_list	*last_node;

	if (lst == NULL)
		return ;
	if (*lst == NULL)
	{
		*lst = new;
		return ;
	}
	last_node = ft_lstlast(*lst);
	last_node->next = new;
}

// Parameters
// 	lst: The address of a pointer to the first link of a list.
// 	new: The address of a pointer to the node to be added to the list.
//
// Returnvalue
//   None
// 
// Description
// 	 Adds the node ’new’ at the end of the list.
// 
// void	ft_lstadd_back(t_list **lst, t_list *new)
// {
// 	t_list	*last_node;

// 	if (lst == NULL || new == NULL) /////////////////
// 		return ;
// 	if (*lst == NULL)
// 	{
// 		*lst = new;
// 		return ;
// 	}
// 	last_node = ft_lstlast(*lst);
// 	last_node->next = new;
// }