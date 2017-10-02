//
// Created by luizorv on 9/23/17.
//

#ifndef DATA_FRAME_RECEIVER_H
#define DATA_FRAME_RECEIVER_H

#include "Socket.h"

#define FR_MAX_REQUESTS 5

namespace DataFrame
{
	class Receiver: public Socket
	{

	public:

		Receiver(std::vector<std::string> params) : Socket(DF_SOCKET_TYPE_RECEIVER)
		{
			this->params = 	params;
		}

        ~Receiver() {}

		void run()
		{
			socklen_t 	clilen;
			uint16_t 	p_number;
			int 		sock, c_socket;

			_socket_recv = socket(AF_INET, SOCK_STREAM, 0);
			if (_socket_recv < 0) {
				throw std::runtime_error("ERROR opening socket");
			}

            int size = 1;

            if (setsockopt(_socket_recv, SOL_SOCKET, SO_REUSEADDR, &size, sizeof(int)) < 0)
                printf("setsockopt(SO_REUSEADDR) failed");

			struct sockaddr_in 	serv_addr;
			serv_addr.sin_family 		= AF_INET;
			serv_addr.sin_port   		= htons(static_cast<uint16_t>(std::stoi(params[4].c_str())));
			inet_aton(params[3].c_str(), &serv_addr.sin_addr);

			while (bind(_socket_recv, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
				std::cout << "Receiver: Fail to bind (" << params[3].c_str() << ":" << params[4].c_str() << " already binded?). Trying again in 3 sec..." << std::endl;
				sleep(3);
			}

			listen(_socket_recv, FR_MAX_REQUESTS);

			struct sockaddr_in 	cli_addr;
			clilen = sizeof(cli_addr);

			c_socket = accept(_socket_recv, (struct sockaddr *)&cli_addr, &clilen);
			this->communicate(c_socket);
			pause();
		}
	};
}
#endif //DATA_FRAME_RECEIVER_H
