//
// Created by luizorv on 9/23/17.
//

#ifndef DATA_FRAME_MODULE_H
#define DATA_FRAME_MODULE_H

#include "HandleErrors.h"

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

		Module() {}

		void bootstrap(int argc, char **argv)
		{
			if(argc != FR_PARAMS_N) {
				this->presentErrors(HANDLE_ERROR_TYPE_ERR_MSG | HANDLE_ERROR_TYPE_PARAMS_SIZE);
			}

		}



	};
}
#endif //DATA_FRAME_MODULE_H
