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

		void run()
		{
			std::ofstream 			os;
			socklen_t 				clilen;
			uint16_t 				p_number;
			int 					sock, c_socket;

			os.open(params[2].c_str(), std::ios::out | std::ios::trunc);

			sock = socket(AF_INET, SOCK_STREAM, 0);
			if (sock < 0) {
				throw std::runtime_error("ERROR opening socket");
			}

			struct sockaddr_in 	serv_addr;
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_port   = static_cast<uint16_t>(std::stoi(params[4].c_str()));
			inet_aton(params[4].c_str(), &serv_addr.sin_addr);

			if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
				throw std::runtime_error("ERROR on binding");

			listen(sock, FR_MAX_REQUESTS);

			struct sockaddr_in 	cli_addr;
			clilen = sizeof(cli_addr);

			while(1) {
				c_socket = accept(sock, (struct sockaddr *)&cli_addr, &clilen);
				this->communicate(c_socket);
				close(c_socket);
				break;
			}

			os.close();
		}

	};
}
#endif //DATA_FRAME_RECEIVER_H
