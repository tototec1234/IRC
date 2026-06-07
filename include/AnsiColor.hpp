/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AnsiColor.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: toruinoue <toruinoue@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 19:48:18 by torinoue          #+#    #+#             */
/*   Updated: 2026/06/07 23:20:34 by toruinoue        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ANSICOLOR_HPP
#define ANSICOLOR_HPP

// ANSI Color Codes for Console Output
#define RESET_COLOR     "\033[0m"
#define RED_COLOR       "\033[31m"
#define GREEN_COLOR     "\033[32m"
#define YELLOW_COLOR    "\033[33m"
#define BLUE_COLOR      "\033[34m"
#define MAGENTA_COLOR   "\033[35m"
#define CYAN_COLOR      "\033[36m"
#define WHITE_COLOR     "\033[37m"

// Background Colors
#define BG_RED_COLOR    "\033[41m"
#define BG_GREEN_COLOR  "\033[42m"
#define BG_YELLOW_COLOR "\033[43m"
#define BG_BLUE_COLOR   "\033[44m"

// Bold Colors
#define BOLD_RED_COLOR    "\033[1;31m"
#define BOLD_GREEN_COLOR  "\033[1;32m"
#define BOLD_YELLOW_COLOR "\033[1;33m"
#define BOLD_BLUE_COLOR   "\033[1;34m"

#endif
