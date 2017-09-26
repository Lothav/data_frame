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

#define	DF_SOCKET_TYPE_RECEIVER "passivo"
#define	DF_SOCKET_TYPE_SENDER 	"ativo"

#define FR_SYNC_EVAL 	    0xDCC023C2
#define FR_ST_SIZE_PAD 	    112

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

        virtual ~Socket()
        {
            _receive.join();
            close(_socket_recv);
            _send.join();
            close(_socket_sender);
        }

        std::vector<std::string> params;

    private:

        std::string _mode;

        std::thread _receive;
        std::thread _send;

    protected:

        int _socket_sender;
        int _socket_recv;

    public:

        void communicate(int _socket)
        {
            int counter;
            _receive = std::thread(DataFrame::Socket::Receive, _socket, params[2]);
            _send 	 = std::thread(DataFrame::Socket::Send, _socket, params[1]);
        }

        static void Receive(int c_socket, std::string out_path)
        {
            size_t max_size = static_cast<size_t>(std::pow(2, 16)) - 1;
            void* buffer = malloc(max_size);
            ssize_t rec_size = recv( c_socket, buffer, max_size, 0);
            size_t data_size = static_cast<size_t>(rec_size - FR_ST_SIZE_PAD);

            while(rec_size == -1) {
                std::cout << "Receive: Fail to receive data (socket " << c_socket << "). Trying again in 3 sec..." << std::endl;
                sleep(3);
            }

            if(buffer) {
                struct Frame header = {};
                memcpy(&header, buffer, FR_ST_SIZE_PAD);

                char * data = (char *)malloc(data_size);
                memcpy(data, buffer + FR_ST_SIZE_PAD, data_size);

                std::ofstream os (out_path.c_str(), std::ios::binary | std::ios::trunc);
                if(os && os.is_open()) {
                    os.write(data, data_size);
                    os.close();
                }

                free(data);
            }
        }

        static void Send(int c_socket, std::string in_path)
        {
            std::ifstream is;
            is.open(in_path.c_str(), std::ios::binary);

            if(is.is_open()) {
                std::stringstream _buffer;
                _buffer << is.rdbuf();
                is.close();

                std::string s_temp(_buffer.str());

                struct Frame frame = {};
                frame.__sync_1 	= FR_SYNC_EVAL;
                frame.__sync_2 	= FR_SYNC_EVAL;
                frame.length 	= sizeof(_buffer.str());
                memset(&frame.resvr, 0, sizeof(frame.resvr));

                size_t size = static_cast<size_t>(FR_ST_SIZE_PAD + frame.length);
                char* send_buffer = (char *)malloc(size);

                memcpy(send_buffer, &frame, FR_ST_SIZE_PAD);
                memcpy(send_buffer+FR_ST_SIZE_PAD, s_temp.c_str(), sizeof(_buffer.str()));

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
