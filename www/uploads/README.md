_This project has been created as part of the 42 curriculum by wxi._

***Description***
	The goal of the project: To be able to configure small-scale networks.
	To achieve the goal, it is necessary to understand the following topics: TCP/IP addressing, subnet masks, default gateways, routers and switches, OSI layers, etc. Details of each topic are explained under Resources. Under the root of NetPractice directory, you can see the configuration of each level of the training interface.  

***Instructions***
	To run the training interface, I go into ~/net_practice and run ./run.sh. Each level in of the trainning interface provides a button called "Get my config", by clicking it I can export the config of each level in json file.

***Submission requirement***
	Submit your assignment in your Git repository as usual. Only the work within your
	repository will be evaluated during the defense. Do not hesitate to double-check the
	names of your files to ensure they are correct.
	
	Because there are 10 levels available in the training interface, you will have to submit 10 files in your repository (one file per level). Place them at the root of your repository. Don’t forget to enter your login in the training interface. Export a file per level using the Get my config button.

***Resources***
https://github.com/caroldaniel/42sp-cursus-netpractice
https://www.youtube.com/watch?v=HQUw0CfQWAM
https://chatgpt.com/
These are the three main resources I have used to understand the following topics to complete this project. Within which the AI agent was used mainly to help me understand technical jargons. The video was referenced when I encountered difficulty on level 9.

***1. TCP/IP addressing***:
	TCP(Transmission Control Protocol) is a communication standard protocol that enables nodes to communicate with each other. TCP is responsable for "breaking" data into small pieces (or packets), sending them over the network and then "rebuilding" them all back again while ensuring there is no data lost in the process. It does so utilizing an Internet Protocol Address (or IP Adress), which is a series of numbers used to identify any device connected to a network, either public or private. TCP and IP are not the same thing, but rather two separated protocols that work together in order ensure data transfer between different devices. There are two different version of IP Addresses: IPv4 and IPv6. For the purpose of this project, only IPv4 is used.
***2. IPv4 and Subnet mask***:
	An IPv4 address is a 32-bit number divided into four 8-bit blocks. Each of theses blocks range from 0 to 255 or, in binary, 00000000 to 11111111. A Mask is also a 32-bit number divided into four 8-bit blocks.
	
	Every IP Addresscan be split into two separete pieces of information: the Network address and the Host address. In order to identify which part of the full IP Address correspond tothe Network and which correspond to the Host, we musk apply to it a Network Mask (orSubnet Mask).
	
	A Mask is also a 32-bit number divided into four 8-bit blocks. Its purpose is to identify which bits are actively part of the Network identification.  To do so, it marks with 1s the network bits. There are two ways to represent a mask: by a full IP Mask or by CIDR. 
	
	The example below shows that the submask marks three 8-bit blocks all by 1s (24bits represented by CIDR) as its network identification, which means that the first three blocks of the IP address needs to be exactly the same to be in the same net work. If the subnet mask marks 20 bits by 1s ((20bits represented by CIDR)), that'd mean that with the first two blocks being the same values, numbers from xxxx0000 to xxxx1111 in the third block of the IP addresses all belong to the same network.
	_______________________________________________________________________________________
	|								 |													  |
	|	IP Address:	153.172.250.12   | IP Address:	10011001.10101100.11111010.00001100   |
	|	Subnet Mask:255.255.255.0    | Subnet Mask:	11111111.11111111.11111111.00000000   |
	|	CIDR:		/24              | CIDR:		/24									  |
	|	IP/CIDR:    153.172.250.12/24| IP/CIDR:     10011001.10101100.11111010.12/24      |
    |_______________________________ |____________________________________________________|
	To communicate with each other, the nodes(devices) must be all in the same network, as in that they need to have the same Network Address portion of their respective IPs. 

	Another important part to understand is that, in this process of dividing a larger network into smaller ones or, subnets, we must reserve 2 IP addresses that cannot be used by any device. The first IP in the range is reserved to identify the subnet. The last IP in the range is reserved for broadcasting messages across all devices in the subnet. A fast subnet mask cheat sheet could be:

	| Last mask number | CIDR | Host bits | Total IPs | Usable hosts |
	| ---------------- | ---- | --------- | --------- | ------------ |
	| 0                | /24  | 8         | 256       | 254          |
	| 128              | /25  | 7         | 128       | 126          |
	| 192              | /26  | 6         | 64        | 62           |
	| 224              | /27  | 5         | 32        | 30           |
	| 240              | /28  | 4         | 16        | 14           |
	| 248              | /29  | 3         | 8         | 6            |
	| 252              | /30  | 2         | 4         | 2            |
	| 254              | /31  | 1         | 2         | 2            |
	| 255              | /32  | 0         | 1         | 1            |
***3. Default gateway***:
	The default gateway, is an IP address that your device uses to send traffic when the destination is outside your local network. On most home networks, this IP is the LAN-side IP of your router. It connects your Local Area Network (LAN) to Wide Wrea Networks (WAN). 
	
	Each router has two IP addresses. One IP address belongs to the LAN side, for example 192.168.1.1, and this is the address your devices use as their default gateway. The 	second IP address belongs to the WAN side and is assigned by your ISP (Internet Service Provider)—the company that gives you internet access. This WAN address is usually a public IP address that can communicate with the rest of the world.

	Every device in your network—your laptop, phone, or tablet—has an IP address for the local network, but it does not know how to reach the rest of the internet on its own. When your device wants to communicate with anything outside the local network, it sends the traffic to the default gateway (LAN IP address of your router) first. The gateway then forwards the traffic to the WAN address.

	When you open a website, your computer first checks whether the destination IP is inside your local network. If the destination is not part of the LAN, your computer automatically sends the data to the default gateway. The router then forwards that data through the WAN connection provided by the Internet Service Provider and sends it across the internet. When the reply comes back, the router sends the response back to your device on the LAN.

	In simple terms, the default gateway is the exit point of your local network, and it is what allows your devices to leave the LAN and reach the WAN—the internet.
***4. IETF and standardized IP ranges***:
	The rules for IP address ranges and their purposes were created and standardized by the Internet Engineering Task Force (IETF). It is an open international community of engineers, researchers, and network designers. Their job is to define protocols and standards that make the Internet work. They publish official rules in documents called RFCs (Request for Comments).
	___________________________________________________________________________
	| IP Range                      | Purpose               | Standardized by |
	| ----------------------------- | --------------------- | --------------- |
	| 10.0.0.0 – 10.255.255.255     | Private IPs (Class A) | IETF, RFC 1918  |
	| 127.0.0.0 – 127.255.255.255   | Loopback              | IETF, RFC 1122  |
	| 172.16.0.0 – 172.31.255.255   | Private IPs (Class B) | IETF, RFC 1918  |
	| 192.168.0.0 – 192.168.255.255 | Private IPs (Class C) | IETF, RFC 1918  |
	| 224.0.0.0 – 239.255.255.255   | Multicast             | IETF, RFC 5771  |
	| 240.0.0.0 – 255.255.255.255   | Experimental          | IETF, RFC 6890  |

***5. Routers***:
	A router is a network device that connects different networks and directs traffic between them. It operates at Layer 3 of the OSI model, the Network layer, and uses IP addresses to decide where data packets should go. When a device sends data to a destination outside its local network, the router receives the packet and forwards it toward the correct network using its routing table. In most home and small office networks, the router also acts as the default gateway, allowing devices on the local network to access the internet.

***6. Switches***:
	A switch is a device used to connect multiple devices within the same local area network. It operates at Layer 2 of the OSI model, the Data Link layer, and forwards data using MAC addresses rather than IP addresses. When a device sends data, the switch learns which device is connected to each port and ensures that the data is delivered only to the intended recipient. This improves efficiency and reduces unnecessary network traffic inside the local network.

***7. OSI layers***:
	The OSI (Open Systems Interconnection) model is a conceptual framework that explains how data travels across a network by dividing the communication process into seven layers. Starting from the bottom, the Physical layer handles cables and electrical signals, while the Data Link layer manages local device communication using MAC addresses. The Network layer is responsible for routing using IP addresses, and the Transport layer ensures reliable data delivery through protocols such as TCP or UDP. Above these, the Session layer manages connections between applications, the Presentation layer handles data formatting and encryption, and the Application layer represents the programs users interact with, such as web browsers and email clients. Together, these layers describe the complete journey of data from one device to another.
