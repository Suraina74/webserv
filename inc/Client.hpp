/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schabboe <schabboe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 18:13:40 by ksoedama          #+#    #+#             */
/*   Updated: 2026/07/17 13:36:53 by schabboe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once 

class Client {
	private:
		//client socket	
	public:
		Client();
		Client(const Client& other);
		Client& operator=(const Client& other);
		~Client();
};