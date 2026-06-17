/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strdup.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torinoue <21091003@mst.meiji-u.ac.jp>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/02 21:28:48 by torinoue          #+#    #+#             */
/*   Updated: 2023/03/26 15:50:25 by torinoue         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strdup(const char *src)
{
	size_t	size;
	char	*copied_string;

	size = ft_strlen(src) + 1;
	copied_string = (char *)malloc(size * sizeof(char));
	if (copied_string == NULL)
		return (NULL);
	ft_strlcpy(copied_string, src, size);
	return (copied_string);
}

//int main(void)
//{
//	char *str1 = "Hello,42";
//	char *str2 = strdup(str1);
//	char *str3 = ft_strdup(str1);
//	printf("Original   :%s\n", str1);
//	printf("Duplicated :%s\n", str2);
//	printf("ft_strd    :%s\n", str3);
//	free(str2);
//	free(str3);
//	return 0;
//}
