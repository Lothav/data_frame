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

enum {
	DF_ALL_DATA_RECEIVED,
	DF_ERROR_RECEIVE_DATA,
	DF_TRYING_AGAIN_FEW_SECS
};

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
            _receive.join();
            close(_socket_recv);
            _send.join();
            close(_socket_sender);
			if(_buffer != nullptr) free(_buffer);
        }

        std::vector<std::string> params;

    private:

        std::string _mode;

        std::thread _receive;
        std::thread _send;

		void * _buffer = nullptr;

    protected:

        int _socket_sender;
        int _socket_recv;

    public:

        void communicate(int _socket)
        {
            int counter;

			size_t  max_size  = static_cast<size_t>(std::pow(2, 16)) - 1;
			_buffer = malloc(max_size);

			_receive = std::thread(DataFrame::Socket::Receive, _socket, params[2], _buffer, max_size);
            _send 	 = std::thread(DataFrame::Socket::Send, _socket, params[1]);
        }

        static void Receive(int c_socket, std::string out_path, void* buffer, size_t max_size)
        {
            ssize_t rec_size;

			while(true) {

				// Clear buffer
				memset(buffer, 0, max_size);

				if((rec_size = recv(c_socket, buffer, max_size, 0)) == -1) {
					std::cout << "Receive: Fail to receive data (socket " << c_socket << " (err: " << errno << ") is dead?)." << std::endl;
					std::cout << "Receiver: Trying again in 3 sec... Hit ctrl+c to cancel" << std::endl << std::endl;
					sleep(3);
					continue;
				}

				if(rec_size == 0) {
					std::cout << "Receive: Received data with 0B (socket " << c_socket << " is dead?)." << std::endl;
					std::cout << "Receiver: Trying again in 3 sec... Hit ctrl+c to cancel" << std::endl << std::endl;
					sleep(3);
					continue;
				}

				char bytes_size[15];
				Utils::prettyBytes(bytes_size, rec_size);
				std::cout << "Receive: receiving data with " << bytes_size << " size."<< std::endl;

				struct Frame header = {};
				memcpy(&header, buffer, FR_ST_SIZE_PAD);

				if(header.length == 0) {
					std::cout << "Field length eq zero" << std::endl;
					continue;
				}

				if( !Utils::checkChecksum(buffer, header) ){
					std::cout << "fail checksum" << std::endl;
					continue;
				}

				// Get data from buffer
				size_t data_size = static_cast<size_t>(rec_size-FR_ST_SIZE_PAD);
				char data[ data_size+1 ];
				memcpy(data, ((uint8_t *)buffer)+FR_ST_SIZE_PAD, data_size);

				// Write on file
				std::ofstream os (out_path.c_str(), std::ios::binary);
				if(!os.is_open()) {
					std::cout << "Receive: Cant open/write file " << out_path.c_str() << std::endl;
					std::cout << "Receive: Make sure that gave me permissions to do it." << std::endl;
					return;
				}
				os.write(data, strlen(data));
				os.close();
				std::cout << "Receive: successful data store in " << out_path << std::endl;
				std::cout << "Receive: trying to receive more data... Hit ctrl+c to cancel." << std::endl;
			}
        }

        static void Send(int c_socket, std::string in_path)
        {
            std::ifstream is;
            is.open(in_path.c_str(), std::ios::binary);

            if(is.is_open()) {
                size_t __max_size = 5;//static_cast<size_t>( pow(2, 16)-1 );

                char* _file_buffer = (char *) malloc(__max_size);
                is.read(_file_buffer, __max_size);
                size_t buffer_length = strlen(_file_buffer);

                is.close();

                struct Frame frame  = {};
                frame.__sync_1 	    = htonl(FR_SYNC_EVAL);
                frame.__sync_2 	    = htonl(FR_SYNC_EVAL);
                frame.length 	    = htons(buffer_length);
                frame.chksum 	    = htons(0);
                frame.resvr 	    = htons(0);

                size_t size         = static_cast<size_t>(FR_ST_SIZE_PAD + frame.length);
                void* send_buffer   = malloc(size);

                memcpy(send_buffer, &frame, FR_ST_SIZE_PAD);
                memcpy(((uint8_t *)send_buffer)+FR_ST_SIZE_PAD, _file_buffer, buffer_length);

                uint16_t checksum16 = Utils::ip_checksum(send_buffer, size);
                frame.chksum = checksum16;
                memcpy(send_buffer, &frame, FR_ST_SIZE_PAD);

                while (send( c_socket, send_buffer, size, 0 ) == -1){
                    std::cout << "Send: Fail to send data (socket " << c_socket << "). Trying again in 3 sec..." << std::endl;
                    sleep(3);
                }
                free(send_buffer);
            }
        }

    };
}

#endif //DATA_FRAME_SOCKET_H
