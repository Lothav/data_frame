//
// Created by luizorv on 9/24/17.
//

#ifndef DATA_FRAME_SOCKET_H
#define DATA_FRAME_SOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <memory.h>
#include <arpa/inet.h>

#define	DF_SOCKET_TYPE_RECEIVER "passivo"
#define	DF_SOCKET_TYPE_SENDER 	"ativo"

namespace DataFrame
{
	class Socket
	{

	protected:

		Socket(std::string mode)
		{
			_mode = mode;
		}

		~Socket()
		{
			_receive.join();
			_send.join();
		}

		std::vector<std::string> params;

	private:

		std::string _mode;

		std::thread _receive;
		std::thread _send;

	public:

		void communicate(int _socket)
		{
			char buffer[4] = "abc";
			int counter;

			_receive = std::thread(DataFrame::Socket::Receive, _socket, buffer, params[2]);
			_send 	 = std::thread(DataFrame::Socket::Send, _socket, &counter, params[1]);
		}

		static void Receive(int c_socket, char* buffer, std::string out_path)
		{
			std::ofstream os;
			os.open(out_path.c_str(), std::ios::out | std::ios::trunc);

			recv( c_socket, buffer, 4, MSG_WAITALL );
			std::cout << buffer << std::endl;
			os.close();
		}

		static void Send(int c_socket, int* counter_hs, std::string in_path)
		{
			std::ifstream is;
			is.open(in_path.c_str(), std::ios::in);

			send( c_socket, counter_hs, 4, 0 );

			is.close();
		}

	};
}

#endif //DATA_FRAME_SOCKET_H
