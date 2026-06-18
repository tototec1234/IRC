/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstadd_front.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <torinoue@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/29 21:04:16 by torinoue          #+#    #+#             */
/*   Updated: 2023/10/04 21:51:52 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_lstadd_front(t_list **lst, t_list *new)
{
	if (lst == NULL)
		return ;
	new->next = *lst;
	*lst = new;
}

// Parameters
// 	lst: The address of a pointer to the first link of a list.
// 	new: The address of a pointer to the node to be added to the list.
//
// Returnvalue
//   None
// 
// Description
// 	 Adds the node ’new’ at the beginning of the list.
// 
// void	ft_lstadd_front(t_list **lst, t_list *new)
// {
// 	if (lst == NULL || new == NULL)   /////////////////
// 		return ;
// 	new->next = *lst;
// 	*lst = new;
// }