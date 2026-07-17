/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schabboe <schabboe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 18:12:41 by ksoedama          #+#    #+#             */
/*   Updated: 2026/07/17 13:37:43 by schabboe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once 

class Server {
	private:
		//serv socket
	public:
		Server();
		Server(const Server& other);
		Server& operator=(const Server& other);
		~Server();
};