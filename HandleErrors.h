#ifndef DATA_FRAME_HANDLEERRORS_H
#define DATA_FRAME_HANDLEERRORS_H

enum ErrorsType {
    HANDLE_ERROR_TYPE_ERR_MSG       			= 0x1 << 0,
    HANDLE_ERROR_TYPE_PARAMS_SIZE   			= 0x1 << 1,
	HANDLE_ERROR_TYPE_THROW_RUNTIME_ERROR       = 0x1 << 2,
    HANDLE_ERROR_TYPE_MODE_NOT_MATCH            = 0x1 << 3,
    HANDLE_ERRSOR_TYPE              			= 0x1 << 4,
    HANDLE_ERRSOR_FTYPE             			= 0x1 << 5
};

namespace DataFrame
{
    class HandleErrors
    {

	public:

        HandleErrors() {}
		virtual ~HandleErrors() {}

    protected:

        void presentErrors(int mask)
        {
            if(mask == (mask | HANDLE_ERROR_TYPE_ERR_MSG)) std::cout << "ERR: \n";
            if(mask == (mask | HANDLE_ERROR_TYPE_PARAMS_SIZE)) handleParamsSize();
			if(mask == (mask | HANDLE_ERROR_TYPE_MODE_NOT_MATCH)) handleMode();
			if(mask == (mask | HANDLE_ERROR_TYPE_THROW_RUNTIME_ERROR)) throw std::exception();
		}

    private:

		void handleMode()
		{
			std::cout << "Mode mismatch! Modes supported: \"ativo\" or \"passivo\"."<< std::endl;
		}

        void handleParamsSize()
        {
            std::cout << "Number incorrect of params!" << std::endl;
            std::cout << "Params expected:" << std::endl;
            std::cout << "\t<in>\tInput file name to be framed and sent" << std::endl;
            std::cout << "\t<out>\tOutput file name to store valid payload" << std::endl;
            std::cout << "\t<ip>\tIP address" << std::endl;
            std::cout << "\t<port>\tPort" << std::endl;
            std::cout << "\t<mode>\tEmulator operation mode" << std::endl;
        }

    };
}


#endif //DATA_FRAME_HANDLEERRORS_H
