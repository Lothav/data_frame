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

#define FR_SYNC_EVAL 	0xDCC023C2
#define FR_PARAMS_N 	6

struct Frame {
	unsigned int 	__sync_1;
	unsigned int 	__sync_2;
	short 			chksum;
	short 			length;
	char 			resvr[2];
};

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
					std::unique_ptr<Receiver> instance (new Receiver());
					instance.get()->run(_params.params);
				} else {
					std::unique_ptr<Sender> instance (new Sender());
					instance.get()->run(_params.params);
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
				errors |= HANDLE_ERROR_TYPE_MODE_NOT_MATCH;

			if(errors) this->presentErrors(errors);
		}

	};
}
#endif //DATA_FRAME_MODULE_H
