//
// Created by luizorv on 9/23/17.
//

#ifndef DATA_FRAME_SENDER_H
#define DATA_FRAME_SENDER_H

#include "Socket.h"

namespace DataFrame
{
	class Sender : public Socket
	{

	public:

		Sender(std::vector<std::string> params) : Socket(DF_SOCKET_TYPE_SENDER)
		{
			this->params = params;
		}

		void run()
		{
			_socket_sender = socket(AF_INET, SOCK_STREAM, 0);
			if(-1 == _socket_sender) throw std::runtime_error("Sender: Error on start socket");

			struct sockaddr_in antelope;
			char *some_addr;

			struct sockaddr_in dst = {};
			dst.sin_family 		= AF_INET;
			dst.sin_port 		= htons(static_cast<uint16_t>(std::stoi(params[4].c_str())));
			inet_aton(params[3].c_str(), &dst.sin_addr);

			struct sockaddr *sa_dst = (struct sockaddr *)&dst;
			while( connect(_socket_sender, sa_dst, sizeof(dst)) ){
				std::cout << "Sender: Fail to connect (" << params[3].c_str() << ":" << params[4].c_str() << " is down?). Trying again in 3 sec..." << std::endl;
				sleep(3);
			}

			this->communicate(_socket_sender);
			sleep(50);
			std::cout <<"byeee have a great time" << std::endl;
		}

	};
}
#endif //DATA_FRAME_SENDER_H
