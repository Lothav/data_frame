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

#define FR_CHECKSUM_OFFSET  8
#define FR_CHECKSUM_SIZE    2

struct Frame {
    uint32_t 	__sync_1;
    uint32_t 	__sync_2;
    uint16_t 	chksum;
    uint16_t 	length;
    uint16_t    resvr;
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
            size_t  max_size  = static_cast<size_t>(std::pow(2, 16)) - 1;
            void*   buffer    = malloc(max_size);

            ssize_t rec_size;
            if((rec_size = recv(c_socket, buffer, max_size, 0)) == -1) {
                std::cout << "Receive: Fail to receive data (socket " << c_socket << "). Trying again in 3 sec..." << std::endl;
                sleep(3);
            }

            struct Frame header = {};
            memcpy(&header, buffer, FR_ST_SIZE_PAD);

            if(header.length == 0) {
                std::cout << "Field length eq zero" << std::endl;
                return;
            }

            // clear checksum field
            memset(((uint8_t *)buffer)+FR_CHECKSUM_OFFSET, 0, FR_CHECKSUM_SIZE);

            uint16_t checksum16 = Socket::ip_checksum(buffer, header.length+FR_ST_SIZE_PAD);

            if(checksum16 != header.chksum){
                std::cout << "fail checksum" << std::endl;
                return;
            }

            size_t data_size = static_cast<size_t>(rec_size-FR_ST_SIZE_PAD);
            char data[ data_size+1 ];
            memcpy(data, ((uint8_t *)buffer)+FR_ST_SIZE_PAD, data_size);

            std::ofstream os (out_path.c_str(), std::ios::binary | std::ios::trunc);
            if(os.is_open()) {
                os.write(data, strlen(data));
                os.close();
            }

            free(buffer);
        }

        static void Send(int c_socket, std::string in_path)
        {
            std::ifstream is;
            is.open(in_path.c_str(), std::ios::binary);

            if(is.is_open()) {
                std::stringstream _buffer;
                _buffer << is.rdbuf();
                is.close();

                std::string s_temp( _buffer.str() );

                struct Frame frame  = {};
                frame.__sync_1 	    = htonl(FR_SYNC_EVAL);
                frame.__sync_2 	    = htonl(FR_SYNC_EVAL);
                frame.length 	    = htons(sizeof(_buffer.str()));
                frame.chksum 	    = htons(0);
                frame.resvr 	    = htons(0);

                size_t size         = static_cast<size_t>(FR_ST_SIZE_PAD + frame.length);
                void* send_buffer   = malloc(size);

                memcpy(send_buffer, &frame, FR_ST_SIZE_PAD);
                memcpy(((uint8_t *)send_buffer)+FR_ST_SIZE_PAD, s_temp.c_str(), sizeof(_buffer.str()));

                uint16_t checksum16 = Socket::ip_checksum(send_buffer, size);
                frame.chksum = checksum16;
                memcpy(send_buffer, &frame, FR_ST_SIZE_PAD);

                while (send( c_socket, send_buffer, size, 0 ) == -1){
                    std::cout << "Send: Fail to send data (socket " << c_socket << "). Trying again in 3 sec..." << std::endl;
                    sleep(3);
                }
                free(send_buffer);
            }
        }


        static uint16_t ip_checksum(void* vdata, size_t length) {
            // Cast the data pointer to one that can be indexed.
            char* data=(char*)vdata;

            // Initialise the accumulator.
            uint32_t acc=0xffff;

            // Handle complete 16-bit blocks.
            for (size_t i=0;i+1<length;i+=2) {
                uint16_t word;
                memcpy(&word,data+i,2);
                acc+=ntohs(word);
                if (acc>0xffff) {
                    acc-=0xffff;
                }
            }

            // Handle any partial block at the end of the data.
            if (length&1) {
                uint16_t word=0;
                memcpy(&word,data+length-1,1);
                acc+=ntohs(word);
                if (acc>0xffff) {
                    acc-=0xffff;
                }
            }

            // Return the checksum in network byte order.
            return htons(~acc);
        }
    };
}

#endif //DATA_FRAME_SOCKET_H
