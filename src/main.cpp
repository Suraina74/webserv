/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schabboe <schabboe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 14:58:38 by ksoedama          #+#    #+#             */
/*   Updated: 2026/07/04 18:59:19 by schabboe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#define QUEUE 10 // how many incoming connections can be in queue for listen. 
// Incoming connections are going to wait in this queue until you accept them.

int	main(int ac, char **av)
{
	(void)av;
	if (ac != 2)
	{
		std::cout << "usage: ./webserv config" << std::endl;
		return (1);
	}

	struct addrinfo *result;
	struct addrinfo info;
	memset(&info, 0, sizeof(info));
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
	struct addrinfo *ptr;
	int sockfd = 0;

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (sockfd == -1){
			std::cout << "socket failure\n";
			// freeaddrinfo(result);
			return 1;
		}
		// if (setsockopt())
		// Binding the socket to the host (server side).
		// Server says: I will listen on this port.
		std::cout << "connected to the host!\n";
		if (bind(sockfd, result->ai_addr, result->ai_addrlen) == -1){
			std::cout << "bind failure\n";
			// freeaddrinfo(result);
			return 1;
		}
		break ;
	}
	
	freeaddrinfo(result);
	if (listen(sockfd, QUEUE) != 0){
		std::cout << "listen failure\n";
		return 1;
	}
	// Accept incoming connections.
	while (1)
	{
		struct sockaddr_storage client_address;
		socklen_t address_size;
		address_size = sizeof(client_address);
		std::cout << "LOOP\n";
		int new_fd = accept(sockfd, (struct sockaddr *)&client_address, &address_size);
		if (new_fd == -1){
			std::cout << "accept failure\n";
			continue;
		}
	}

	// Alleen result free-en met freeaddrinfo(res), info is alleen input.

	




	return (0);
}