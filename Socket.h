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
			char buffer[3];
			int counter;

			_receive = std::thread(DataFrame::Socket::Receive, _socket, buffer);
			_send 	 = std::thread(DataFrame::Socket::Send, _socket, &counter);

		}

		static void Receive(int c_socket, char* buffer)
		{
			recv( c_socket, buffer, 2, MSG_WAITALL );
		}

		static void Send(int c_socket, int* counter_hs)
		{
			send( c_socket, counter_hs, 4, 0 );
		}

	};
}

#endif //DATA_FRAME_SOCKET_H
