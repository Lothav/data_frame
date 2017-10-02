//
// Created by luizorv on 9/23/17.
//

#ifndef DATA_FRAME_MODULE_H
#define DATA_FRAME_MODULE_H

#include <thread>
#include <vector>
#include <fstream>
#include "HandleErrors.h"
#include "Sender.h"
#include "Receiver.h"

#define FR_PARAMS_N 6

DataFrame::Receiver* receiver = nullptr;
DataFrame::Sender* sender = nullptr;

void handleSignal(int s)
{
	std::cout << std::endl << "Caught signal " << s << std::endl;
	if(sender != nullptr) {
		std::cout << std::endl << "Destroying Sender..." << std::endl;
		sender->_thr_receive.detach();
		sender->_thr_send.detach();

		sender->_thr_receive.join();
		sender->_thr_send.join();
	}
	if(receiver != nullptr) {
		std::cout<< std::endl << "Destroying Receiver..." << std::endl;
		receiver->_thr_receive.detach();
		receiver->_thr_send.detach();

		receiver->_thr_receive.join();
		receiver->_thr_send.join();
	}
}

namespace DataFrame
{
	class Module : public HandleErrors
	{

	public:

		Module(short size, char** params)
		{
			_params.size = size;
			for (short i = 0; i < size; i++) {
				_params.params.push_back( std::string(params[i]) );
			}
			struct sigaction sigIntHandler;

			sigIntHandler.sa_handler = handleSignal;
			sigemptyset(&sigIntHandler.sa_mask);
			sigIntHandler.sa_flags = 0;

			sigaction(SIGINT, &sigIntHandler, NULL);
		}

		~Module() {}

	private:

		struct {
			std::vector<std::string> params = {};
			short size;
		} _params = {};

	public:

		void bootstrap()
		{
			this->checkParams();
			try {
				if(_params.params[5] == DF_SOCKET_TYPE_RECEIVER){
					receiver = new Receiver(_params.params);
					receiver->run();
				} else {
					sender = new Sender(_params.params);
					sender->run();
				}
			} catch (std::runtime_error &ex) {
				std::cerr << ex.what();
			}

			delete sender;
			delete receiver;
		}

	private:

		void checkParams()
		{
			unsigned int errors = 0; //5733961

			if(_params.size != FR_PARAMS_N)
				errors |= HANDLE_ERROR_TYPE_ERR_MSG | HANDLE_ERROR_TYPE_PARAMS_SIZE | HANDLE_ERROR_TYPE_THROW_RUNTIME_ERROR;

			if(_params.params[5] != DF_SOCKET_TYPE_RECEIVER && _params.params[5] != DF_SOCKET_TYPE_SENDER)
				errors |= HANDLE_ERROR_TYPE_MODE_NOT_MATCH | HANDLE_ERROR_TYPE_THROW_RUNTIME_ERROR;

			if(errors) this->presentErrors(errors);
		}

	};
}
#endif //DATA_FRAME_MODULE_H
