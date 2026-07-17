/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Poll.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schabboe <schabboe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 17:59:03 by ksoedama          #+#    #+#             */
/*   Updated: 2026/07/17 13:34:17 by schabboe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

class Poll {
	private:
		// struct pollfd pfds;
	public:
		Poll();
		Poll(const Poll&);
		Poll& operator=(const Poll&);
		~Poll();
};