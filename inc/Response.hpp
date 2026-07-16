/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: ksoedama <ksoedama@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/07/16 18:55:05 by ksoedama      #+#    #+#                 */
/*   Updated: 2026/07/16 18:56:32 by ksoedama      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once 
#include <ostream>

class Response {
	private:
		std::string	status_line;
		std::string	*body;
		//hashtable var voor headers
	public:
		//canon form
};
