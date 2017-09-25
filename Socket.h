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
#include <sstream>

#define	DF_SOCKET_TYPE_RECEIVER "passivo"
#define	DF_SOCKET_TYPE_SENDER 	"ativo"

#define FR_SYNC_EVAL 	0xDCC023C2
#define FR_ST_SIZE_PAD 	112

struct Frame {
	unsigned int 	__sync_1;
	unsigned int 	__sync_2;
	short 			chksum;
	short 			length;
	char 			resvr[2];
}__attribute__((packed));

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
            if(_mode==DF_SOCKET_TYPE_SENDER) {

                _send.join();
            }else{

                _receive.join();
            }
		}

		std::vector<std::string> params;

	private:

		std::string _mode;

		std::thread _receive;
		std::thread _send;

	public:

		void communicate(int _socket)
		{
			int counter;
            if(_mode==DF_SOCKET_TYPE_SENDER){
                _send 	 = std::thread(DataFrame::Socket::Send, _socket, params[1]);
            }else{
                _receive = std::thread(DataFrame::Socket::Receive, _socket, params[2]);
            }
		}

		static void Receive(int c_socket, std::string out_path)
		{
			std::ofstream os;
			os.open(out_path.c_str(), std::ios::out | std::ios::trunc);

            char* buffer = (char *) malloc(250);

            while(recv( c_socket, buffer, 250, MSG_WAITALL) == -1) {
                std::cout << "Receive: Fail to receive data (socket " << c_socket << "). Trying again in 3 sec..." << std::endl;
                sleep(3);
            }
			std::cout << "Data:" << *buffer << std::endl;
			os.close();
		}

		static void Send(int c_socket, std::string in_path)
		{
			std::ifstream is;
			is.open(in_path.c_str(), std::ios::in);

			if(is.is_open()) {
				std::stringstream _buffer;
                std::string s_temp(_buffer.str());

				struct Frame frame = {};

				frame.__sync_1 	= FR_SYNC_EVAL;
				frame.__sync_2 	= FR_SYNC_EVAL;
				frame.length 	= sizeof(_buffer.str());
				memset(&frame.resvr, 0, sizeof(frame.resvr));

                size_t size = static_cast<size_t>(FR_ST_SIZE_PAD + frame.length);
                char* send_buffer = (char *)malloc(size*2);

                memcpy(send_buffer, &frame, FR_ST_SIZE_PAD);
                memcpy(send_buffer + size, s_temp.c_str(), sizeof(_buffer.str()));

                char a[10] = "Luiz Otav";

                uint32_t as = 234;
                uint32_t counter_hs = htonl(as);

                while (send( c_socket, &counter_hs, sizeof(uint32_t), 0 ) == -1){
                    std::cout << "Send: Fail to send data (socket " << c_socket << "). Trying again in 3 sec..." << std::endl;
                    sleep(3);
                }
                free(send_buffer);
			}

			is.close();
		}

	};
}

#endif //DATA_FRAME_SOCKET_H
