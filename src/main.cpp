/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schabboe <schabboe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 14:58:38 by ksoedama          #+#    #+#             */
/*   Updated: 2026/07/04 16:58:16 by schabboe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

int	main(int ac, char **av)
{
	(void)av;
	if (ac != 2)
	{
		std::cout << "usage: ./webserv config" << std::endl;
		return (1);
	}

	struct addrinfo *result;
	struct addrinfo info = {0};
	info.ai_family = AF_INET; // IPV4
	// info.ai_flags = AI_PASSIVE; // Choose IP for me
	info.ai_socktype = SOCK_STREAM; // TCP communication style
	
	// Build me a list of usable socket addresses:
	// Not all can be reached. Find one that is usable.
	if (getaddrinfo("127.0.0.1", "8080", &info, &result) != 0){
		std::cout << "getaddrinfo failed\n";
		return 1;
	}

	// ai_protocol gaat op basis van socktype.
	// What kind of socket you want:
	int sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (sockfd != 0){
		std::cout << "socket failure\n";
		freeaddrinfo(result);
		return 1;
	}
	
	// Binding the socket to the host (server side).
	// Server says: I will listen on this port.
	if (bind(sockfd, result->ai_addr, result->ai_addrlen) != 0){
		std::cout << "bind failure\n";
		freeaddrinfo(result);
		return 1;
	}
	



	// Alleen result free-en met freeaddrinfo(res), info is alleen input.
	freeaddrinfo(result);
	




	return (0);
}