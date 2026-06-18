/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_calloc.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <torinoue@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/01 21:51:00 by torinoue          #+#    #+#             */
/*   Updated: 2023/10/05 17:00:36 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_calloc(size_t count, size_t size)
{
	size_t	malloc_size;
	void	*mem;

	if (count == 0 || size == 0)
		return (ft_calloc(1, 1));
	if (SIZE_T_MAX / size < count)
		return (NULL);
	malloc_size = count * size;
	mem = malloc(malloc_size);
	if (mem == NULL)
		return (NULL);
	ft_bzero(mem, malloc_size);
	return (mem);
}

// void	*ft_calloc(size_t count, size_t size)
// {
// 	size_t	malloc_size;
// 	void	*mem_zero;
// 
// 	malloc_size = count * size;
// 	if (malloc_size == 0)			/////////////
// 		return (ft_calloc(1, 1));
// 	if (SSIZE_MAX / size < count)	/////////////
// 		return (NULL);
// 	mem_zero = malloc(malloc_size);
// 	if (mem_zero == NULL)
// 		return (NULL);
// 	ft_bzero(mem_zero, malloc_size);
// 	return (mem_zero);
// }

//#include <stdio.h>
//#include <stdlib.h>
//#include <strings.h>
//int main(void)	
//{
//
//	long *pMMem;
//	long *pCMem;
//	long *pFMem;
//	long count;
//
//	count = INT_MAX ;
//
//	//pMMem = (long *)malloc(sizeof(long) * 5);
//	//pCMem = (long *)calloc(5, sizeof(long));
//	//pFMem = (long *)ft_calloc(5, sizeof(long));
//
//	pMMem = (long *)malloc(sizeof(long) * count);
//	pCMem = (long *)calloc(count, sizeof(long));
//	pFMem = (long *)ft_calloc(count, sizeof(long));
//
//	printf("   malloc [0]:0x%016lx, [1]:0x%016lx,[INT_MAX-1]:0x%016lx
//,[INT_MAX]:0x%016lx\n", pMMem[0], pMMem[1], pMMem[INT_MAX-1], pMMem[INT_MAX]);
//	printf("   calloc [0]:0x%016lx, [1]:0x%016lx,[INT_MAX-1]:0x%016lx
//,[INT_MAX]:0x%016lx\n", pCMem[0], pCMem[1], pCMem[INT_MAX-1], pCMem[INT_MAX]);
//	printf("ft_calloc [0]:0x%016lx, [1]:0x%016lx,[INT_MAX-1]:0x%016lx
//,[INT_MAX]:0x%016lx\n", pFMem[0], pFMem[1], pFMem[INT_MAX-1], pFMem[INT_MAX]);
//
//	free(pMMem);
//	free(pCMem);
//	free(pFMem);
//	return 0;
//
//}
///*
//NAME
//     calloc, free, malloc, realloc, reallocf, valloc -- memory allocation
//
//SYNOPSIS
//     #include <stdlib.h>
//
//DESCRIPTION
//     The malloc(), calloc(), valloc(), realloc(), and reallocf() functions
//     allocate memory.
//   	 The allocated memory is aligned such that it can be
//     used for any data type, including AltiVec- and SSE-related types.  The
//     free() function frees allocations that were created via the preceding
//     allocation functions.
//
//     The malloc() function allocates size bytes of memory and returns a
//     pointer to the allocated memory.
//
//     The calloc() function contiguously allocates enough space for count
//     objects that are size bytes of memory each and returns a pointer to the
//     allocated memory.  The allocated memory is filled with bytes of value
//     zero.
//*/
// 
// void	*ft_calloc(size_t count, size_t size)
// {
// 	size_t	malloc_size;
// 	void	*mem_zero;0
// 	write (1,&"\n",1);
// 
// 	if (count == 0 || size == 0)
// 		{
// 			write (1,&"ft_calloc(1, 1)-",16);
// 			return (ft_calloc(1, 1));
// 		}
// 	if (count == (size_t)INT_MIN && size == (size_t)INT_MIN)
// 		{
// 			write (1,&"INT_MIN,INT_MIN-",16);
// 			return (NULL);
// 		}
// 	malloc_size = count * size;
// 	if (SIZE_T_MAX / size < count)
// 		{
// 			write (1,&"NULL-",5);
// 			return (NULL);
// 		}
// 	mem_zero = malloc(malloc_size);
// 	if (mem_zero == NULL)
// 		return (NULL);
// 	ft_bzero(mem_zero, malloc_size);
// 	return (mem_zero);
// }
// 
// 
// 
// # include <limits.h>
// #include <stdio.h>
// 
// int main(void)
// {
// 	// printf(" INT_MIN           %%d     %d\n", INT_MIN          );
// 	// printf(" INT_MIN           %%zu    %zu\n", INT_MIN          );
// 	// printf(" INT_MAX           %%d     %d\n", INT_MAX          );
// 	// printf(" SIZE_T_MAX  * SIZE_T_MAX  %%d	%d\n", SIZE_T_MAX * SIZE_T_MAX );
// 	printf(" INT_MAX  * INT_MAX  %%zu     %zu\n\n", INT_MAX  * INT_MAX );
// 
// 	printf("%%zu    INT_MAX * INT_MAX %zu\n", INT_MAX * INT_MAX);
// 	// printf("%%zu     SIZE_T_MAX%zu\n", SIZE_T_MAX);
// 	printf("%%zu  (SIZE_T_MAX / INT_MAX )   %zu\n", SIZE_T_MAX / INT_MAX );
// 	printf("%%zu  (SIZE_T_MAX / INT_MAX ) < INT_MAX 
//    %zu\n", SIZE_T_MAX  / INT_MAX  < INT_MAX );
// }