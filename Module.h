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
		}

		~Module()
		{
			std::cout<<"killing module"<<std::endl;
		}

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
					std::unique_ptr<Receiver> instance (new Receiver(_params.params));
					instance.get()->run();
				} else {
					std::unique_ptr<Sender> instance (new Sender(_params.params));
					instance.get()->run();
				}
			} catch (std::runtime_error &ex) {
				std::cerr << ex.what();
			}
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
