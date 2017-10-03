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
            close(_socket_recv);
            close(_socket_sender);
            std::cout << "\tSockets closed..."<< std::endl;

            if(_thr_receive.joinable()) {
                _thr_receive.join();
            }
            if(_thr_send.joinable()) {
                _thr_send.join();
            }
            std::cout << "\tThreads joined..."<< std::endl;

            if(_recv_buffer != nullptr) {
                free(_recv_buffer);
                _recv_buffer = nullptr;
            }
            if(_send_buffer != nullptr){
                free(_send_buffer);
                _send_buffer = nullptr;
            }
            std::cout << "\tMemory has been cleaned..."<< std::endl;

            _output_file.close();
            std::cout << "\t" << params[2] << " closed."<< std::endl;
        }

        std::vector<std::string> params;

    private:

        std::string 	_mode;

        void * 			_send_buffer = nullptr;
        void * 			_recv_buffer = nullptr;

        const size_t 	__max_size = static_cast<size_t>(pow(2, 16)-1);

        std::ofstream 	_output_file;

    protected:

        int _socket_sender;
        int _socket_recv;

    public:

        std::thread 	_thr_receive;
        std::thread 	_thr_send;

        void communicate(int _socket)
        {
            std::vector<char> _recv_buffer (__max_size+FR_ST_SIZE_PAD);
            std::vector<char> _send_buffer (__max_size+FR_ST_SIZE_PAD);
            _output_file.open (params[2], std::ios::binary);

            if(!_output_file.is_open()) {
                std::cout << "Receive: Cant open/write file " << params[2] << std::endl;
                std::cout << "Receive: Make sure that gave me permissions to do it." << std::endl;
                return;
            }

            _thr_receive 	 = std::thread(DataFrame::Socket::Receive, _socket, std::ref(_output_file), _recv_buffer, __max_size);
            _thr_send 	 	 = std::thread(DataFrame::Socket::Send,    _socket, params[1], _send_buffer, __max_size);
        }

        static void Receive(int c_socket, std::ofstream& os, std::vector<char> buffer, const size_t __max_size)
        {
            ssize_t rec_size;

            while(true) {

                // Clear buffer
                memset(buffer.data(), 0, __max_size+FR_ST_SIZE_PAD);
                // Receive data
                rec_size = recv(c_socket, buffer.data(), __max_size+FR_ST_SIZE_PAD, 0);
                // Check receive size is valid
                if(!Utils::checkReceiveSize(rec_size, c_socket)) continue;
                // Check if frame has valid header
                int _pad_data_found = Utils::findValidHeader(buffer, rec_size);
                if(_pad_data_found != -1) {
                    // if it does, write on file
                    char bytes_size[15];
                    Utils::prettyBytes(bytes_size, rec_size-_pad_data_found);
                    std::cout << "Receive: receiving valid frame with " << bytes_size << " size."<< std::endl;

                    // Get data from buffer
                    size_t data_size = static_cast<size_t>(rec_size-(_pad_data_found+FR_ST_SIZE_PAD));
                    char data[ data_size+1 ];
                    memcpy(data, buffer.data()+FR_ST_SIZE_PAD+_pad_data_found, data_size);

                    os.write(data, data_size);
                    std::cout << "Receive: successful data store in output buffer." << std::endl;
                    std::cout << "Receive: trying to receive more data... Hit ctrl+c to cancel." << std::endl << std::endl;
                } else {
                    std::cout << "Receive: header is not valid." << std::endl;
                    std::cout << "Receive: trying to receive more data... Hit ctrl+c to cancel." << std::endl << std::endl;
                    break;
                }
            }
        }

        static void Send(int c_socket, std::string in_path, std::vector<char> send_buffer, const size_t __max_size)
        {
            std::ifstream is(in_path.c_str(), std::ios::binary);

            if(is.is_open()) {

                is.seekg (0, std::ios::end);
                size_t size_file = static_cast<size_t >(is.tellg());
                is.seekg (0, std::ios::beg);

                std::vector<char> _file_buf (size_file);
                is.read(_file_buf.data(), size_file);
                is.close();

                long _buffer_pos = 0;
                while(_buffer_pos < size_file){

                    size_t buffer_length = static_cast<size_t>( std::min(size_file, __max_size) );
                    buffer_length = ((_buffer_pos+buffer_length) > size_file ? size_file - _buffer_pos : buffer_length);

                    struct Frame frame  = {};
                    frame.__sync_1 	    = htonl(FR_SYNC_EVAL);
                    frame.__sync_2 	    = htonl(FR_SYNC_EVAL);
                    frame.length 	    = htons(static_cast<uint16_t>(buffer_length));
                    frame.chksum 	    = htons(0);
                    frame.resvr 	    = htons(0);

                    size_t size = static_cast<size_t>(FR_ST_SIZE_PAD + buffer_length);

                    memset(send_buffer.data(), 0, __max_size+FR_ST_SIZE_PAD);
                    memcpy(send_buffer.data(), &frame, FR_ST_SIZE_PAD);
                    memcpy(send_buffer.data()+FR_ST_SIZE_PAD, &_file_buf[_buffer_pos], buffer_length);
                    frame.chksum = Utils::ip_checksum(send_buffer.data(), size);
                    memcpy(send_buffer.data(), &frame, FR_ST_SIZE_PAD);

                    while (send( c_socket, send_buffer.data(), size, 0 ) == -1){
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
