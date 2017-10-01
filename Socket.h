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
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <cmath>
#include "Utils.h"

namespace DataFrame
{
	class Socket
	{

	protected:

		Socket(std::string mode)
		{
			_mode = mode;
		}

		virtual ~Socket()
		{
			_receive.detach();
			_send.detach();
			close(_socket_recv);
			close(_socket_sender);
			if(_recv_buffer != nullptr) {
				free(_recv_buffer);
				_recv_buffer = nullptr;
			}
			if(_send_buffer != nullptr){
				free(_send_buffer);
				_send_buffer = nullptr;
			}
		}

		std::vector<std::string> params;

	private:

		std::string _mode;

		std::thread _receive;
		std::thread _send;

		void * _send_buffer = nullptr;
		void * _recv_buffer = nullptr;

		const size_t __max_size = static_cast<size_t>(pow(2, 16)-2);

	protected:

		int _socket_sender;
		int _socket_recv;

	public:

		void communicate(int _socket)
		{
			_recv_buffer = malloc(__max_size+FR_ST_SIZE_PAD);
			_send_buffer = malloc(__max_size+FR_ST_SIZE_PAD);

			_receive 	 = std::thread(DataFrame::Socket::Receive, _socket, params[2], _recv_buffer, __max_size);
			_send 	 	 = std::thread(DataFrame::Socket::Send,    _socket, params[1], _send_buffer, __max_size);
		}

		static void Receive(int c_socket, std::string out_path, void* buffer, const size_t __max_size)
		{
			ssize_t rec_size;

			while(true) {

				// Clear buffer
				memset(buffer, 0, __max_size+FR_ST_SIZE_PAD);

				if((rec_size = recv(c_socket, buffer, __max_size+FR_ST_SIZE_PAD, 0)) == -1) {
					std::cout << "Receive: Fail to receive frame (socket " << c_socket << " (err: " << errno << ") is dead?)." << std::endl;
					std::cout << "Receive: Trying again in 3 sec... Hit ctrl+c to cancel" << std::endl << std::endl;
					sleep(3);
					continue;
				}

				if(rec_size == 0) {
					std::cout << "Receive: Received frame with 0B (socket " << c_socket << " is dead?)" << std::endl;
					std::cout << "Receive: Trying again in 3 sec... Hit ctrl+c to cancel" << std::endl << std::endl;
					sleep(3);
					continue;
				}

				if(rec_size <= FR_ST_SIZE_PAD) {
					std::cout << "Receive: Only header receive. There's no data." << std::endl;
					std::cout << "Receive: trying to receive more data... Hit ctrl+c to cancel" << std::endl << std::endl;
					continue;
				}

				char bytes_size[15];
				Utils::prettyBytes(bytes_size, rec_size);
				std::cout << "Receive: receiving frame with " << bytes_size << " size."<< std::endl;

				struct Frame header = {};
				memcpy(&header, buffer, FR_ST_SIZE_PAD);

				if(header.length == 0) {
					std::cout << "Receive: header field 'length' eq zero. Discarding frame." << std::endl;
					std::cout << "Receive: trying to receive more data... Hit ctrl+c to cancel." << std::endl << std::endl;
					continue;
				}

				if( !Utils::checkChecksum(buffer, header, rec_size) ){
					std::cout << "Receive: Fail checksum. Discarding frame." << std::endl;
					std::cout << "Receive: trying to receive more data... Hit ctrl+c to cancel." << std::endl << std::endl;
					continue;
				}

				// Get data from buffer
				size_t data_size = static_cast<size_t>(rec_size-FR_ST_SIZE_PAD);
				char data[ data_size+1 ];
				memcpy(data, ((uint8_t *)buffer)+FR_ST_SIZE_PAD, data_size);

				// Write on file

				std::cout << data;

				std::ofstream os (out_path.c_str(), std::ios::binary | std::ios::app);
				if(!os.is_open()) {
					std::cout << "Receive: Cant open/write file " << out_path.c_str() << std::endl;
					std::cout << "Receive: Make sure that gave me permissions to do it." << std::endl;
					return;
				}
				os << data;
				//os.write(data, strlen(data));
				os.close();
				std::cout << "Receive: successful data store in " << out_path << std::endl;
				std::cout << "Receive: trying to receive more data... Hit ctrl+c to cancel." << std::endl << std::endl;
			}
		}

		static void Send(int c_socket, std::string in_path, void* send_buffer, const size_t __max_size)
		{
			std::ifstream is(in_path.c_str(), std::ios::in|std::ios::binary|std::ios::ate);

			if(is.is_open()) {
				is.seekg (0, std::ios::beg);

				std::vector<char> _file_buffer((std::istreambuf_iterator<char>(is)), (std::istreambuf_iterator<char>()));
				is.close();

				std::cout << _file_buffer.data();

				long _buffer_pos = 0;
				while(_buffer_pos < _file_buffer.size()){

					size_t buffer_length = static_cast<size_t>( std::min(_file_buffer.size(), __max_size) );

					struct Frame frame  = {};
					frame.__sync_1 	    = htonl(FR_SYNC_EVAL);
					frame.__sync_2 	    = htonl(FR_SYNC_EVAL);
					frame.length 	    = htons(static_cast<uint16_t>(buffer_length));
					frame.chksum 	    = htons(0);
					frame.resvr 	    = htons(0);

					size_t size = static_cast<size_t>(FR_ST_SIZE_PAD + buffer_length);

					memcpy(send_buffer, &frame, FR_ST_SIZE_PAD);
					memcpy(((uint8_t *)send_buffer)+FR_ST_SIZE_PAD, _file_buffer.data()+_buffer_pos, buffer_length);

					uint16_t checksum16 = Utils::ip_checksum(send_buffer, size);
					frame.chksum = htons(checksum16);
					memcpy(send_buffer, &frame, FR_ST_SIZE_PAD);

					std::cout << (char *)send_buffer << std::endl;

					while (send( c_socket, send_buffer, size, 0 ) == -1){
						std::cout << "Send: Fail to send data (socket " << c_socket << "). Trying again in 3 sec..." << std::endl;
						sleep(3);
					}

					_buffer_pos += buffer_length;
				}
			} else {
				std::cout << "Send: cant open " << in_path.c_str() << ". Can i have permission to do it?" << std::endl;
			}
		}
	};
}

#endif //DATA_FRAME_SOCKET_H
