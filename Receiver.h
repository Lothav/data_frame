//
// Created by luizorv on 9/23/17.
//

#ifndef DATA_FRAME_RECEIVER_H
#define DATA_FRAME_RECEIVER_H

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <memory.h>

#define FR_MAX_REQUESTS 5

namespace DataFrame
{
	class Receiver
	{

	public:

		static void run(std::vector<std::string> params)
		{
			std::ofstream 			os;
			ssize_t 				n;
			socklen_t 				clilen;
			uint16_t 				p_number;
			uint32_t 				counter_hs, counter = 0;
			int 					sock, c_socket;
			char 					buffer[3], buffer_back[5];
			struct sockaddr_in 		serv_addr, cli_addr;
			struct timeval 			tout;

			os.open(params[2].c_str(), std::ios::out | std::ios::trunc);

			sock = socket(AF_INET, SOCK_STREAM, 0);
			if (sock < 0) {
				throw std::runtime_error("ERROR opening socket");
			}

			p_number = static_cast<uint16_t>(std::stoi(params[4].c_str()));

			serv_addr.sin_family            = AF_INET;
			serv_addr.sin_addr.s_addr       = static_cast<in_addr_t>(params[3].c_str());
			serv_addr.sin_port              = htons(p_number);

			if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
				throw std::runtime_error("ERROR on binding");
			}

			listen(sock, FR_MAX_REQUESTS);
			clilen = sizeof(cli_addr);

			tout.tv_sec = 1;
			tout.tv_usec = 1;
			setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(tout));

			while(1) {

				c_socket = accept(sock, (struct sockaddr *)&cli_addr, &clilen);

				if (c_socket < 0) {
					// timeout
					close(c_socket);
					continue;
				}

				n = recv( c_socket, buffer, 2, MSG_WAITALL );

				if (n < 0) {
					// timeout
					printf("T\n");
					close(c_socket);
					continue;
				}

				counter_hs = htonl(counter);

				n = send(c_socket, &counter_hs, 4, 0);

				if (n < 0) {
					close(c_socket);
					continue;
				}

				n = recv( c_socket, buffer_back, 4 , MSG_WAITALL);

				if (n < 0 || (strlen(buffer_back) != 3)) {
					// timeout
					printf("T\n");
					close(c_socket);
					continue;
				}

				if(atoi(buffer_back) == counter) {
					counter += buffer[0] == '+' ? 1 : -1;
					counter = counter > 0 ? counter % 1000 : 0;
					printf("%i\n", counter);
				}

				close(c_socket);
				break;
			}

			os.close();
			std::cout << "receiver";
		}

	};
}
#endif //DATA_FRAME_RECEIVER_H
