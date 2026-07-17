/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schabboe <schabboe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 18:12:41 by ksoedama          #+#    #+#             */
/*   Updated: 2026/07/17 13:35:47 by schabboe         ###   ########.fr       */
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
		Request();
		Request(const Request& other);
		Request& operator=(const Request& other);
		~Request();
};
