//
// Created by luizorv on 9/23/17.
//

#ifndef DATA_FRAME_RECEIVER_H
#define DATA_FRAME_RECEIVER_H

namespace DataFrame
{
	class Receiver
	{

	public:

		static void run(std::vector<std::string> params)
		{
			std::ofstream os;
			os.open(params[2].c_str(), std::ios::out | std::ios::trunc);



			os.close();
			std::cout << "receiver";
		}

	};
}
#endif //DATA_FRAME_RECEIVER_H
