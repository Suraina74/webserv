/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: ksoedama <ksoedama@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/07/16 18:12:41 by ksoedama      #+#    #+#                 */
/*   Updated: 2026/07/16 18:56:29 by ksoedama      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once 
#include <ostream>

class Request {
	private:
		std::string	request_line;
		std::string	*body;
		//hashtable var voor headers
	public:
		//canon form
};
