/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstdelone.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <torinoue@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/30 15:26:10 by torinoue          #+#    #+#             */
/*   Updated: 2023/10/01 14:27:32 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_lstdelone(t_list *lst, void (*del)(void *))
{
	if (lst == NULL || del == NULL)
		return ;
	del(lst->content);
	free(lst);
}

// Parameters
// 	lst: The node to free.
// 	del: The address of the function used to deletethe content.
//
// Returnvalue
//   None
// 
// Description
// 	 Takes as a parameter a node and frees the memory of
// 	the node’s content using the function ’del’ given
// 	as a parameter and free the node.
//  The memory of
// 	’next’ must not be freed.
// 