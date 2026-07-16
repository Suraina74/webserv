/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Poll.hpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: ksoedama <ksoedama@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/07/16 17:59:03 by ksoedama      #+#    #+#                 */
/*   Updated: 2026/07/16 18:57:06 by ksoedama      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

class Poll {
	private:
		// struct pollfd pfds;
	public:
		Poll();
		Poll(const Poll&);
		Poll&	operator=(const Poll&);
		~Poll();
};