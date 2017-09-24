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
			_th_recv.join();
			_th_sender.join();
		}

	private:

		std::thread _th_recv;
		std::thread _th_sender;

		std::unique_ptr<std::filebuf> file_in = nullptr;
		std::unique_ptr<std::filebuf> file_out = nullptr;

		struct {
			std::vector<std::string> params = {};
			short size;
		} _params = {};

	public:

		void bootstrap()
		{
			this->checkParams();

			std::ifstream is;
			file_in.reset(is.rdbuf());
			file_in.get()->open(_params.params[1].c_str(), std::ios::in);

			std::ifstream os;
			file_out.reset(os.rdbuf());
			file_out.get()->open(_params.params[2].c_str(), std::ios::out | std::ios::trunc);

			this->_th_recv   = std::thread(Receiver::run);
			this->_th_sender = std::thread(Sender::run);
		}

	private:

		void checkParams()
		{
			if(_params.size != FR_PARAMS_N)
				this->presentErrors(HANDLE_ERROR_TYPE_ERR_MSG | HANDLE_ERROR_TYPE_PARAMS_SIZE | HANDLE_ERROR_TYPE_THROW_RUNTIME_ERROR);
		}

	};
}
#endif //DATA_FRAME_MODULE_H
