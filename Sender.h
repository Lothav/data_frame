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

		Sender(std::vector<std::string> params) : Socket(DF_SOCKET_TYPE_SENDER) {
			this->params = params;
		}

		void run(std::vector<std::string> params)
		{
			std::ifstream is;
			is.open(params[1].c_str(), std::ios::in);

			int s = socket(AF_INET, SOCK_STREAM, 0);
			if(-1 == s)
				throw std::runtime_error("Error on start socket");

			struct in_addr addr = { .s_addr = htonl(INADDR_LOOPBACK) };

			struct sockaddr_in dst = {};
			dst.sin_family 	=  AF_INET;
			dst.sin_port 	=  static_cast<uint16_t>(std::stoi(params[4].c_str()));
			dst.sin_addr 	=  addr;

			struct sockaddr *sa_dst = (struct sockaddr *)&dst;

			if(connect(s, sa_dst, sizeof(dst)))
				throw std::runtime_error("Fail to connect");

			this->communicate(s);

			close(s);

			is.close();
		}

	};
}
#endif //DATA_FRAME_SENDER_H
