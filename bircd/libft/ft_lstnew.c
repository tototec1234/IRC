/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstnew.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <torinoue@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/29 21:13:55 by torinoue          #+#    #+#             */
/*   Updated: 2023/10/01 14:27:58 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

t_list	*ft_lstnew(void *content)
{
	t_list	*new;

	new = (t_list *)malloc(sizeof(t_list));
	if (new == NULL)
		return (NULL);
	new->content = content;
	new->next = NULL;
	return (new);
}

// Parameters
// 	content: The content to create the node with.
// 
// Return value
//  The new node.
// 
// External functs.
//  malloc
// 
// Description
// 	 Allocates (with malloc(3)) and returns a new node.
// 	The member variable ’content’ is initialized with
// 	the value of the parameter ’content’.
//  The variable ’next’ is initialized to NULL.
// 