//
// Created by luizorv on 9/23/17.
//

#ifndef DATA_FRAME_SENDER_H
#define DATA_FRAME_SENDER_H

namespace DataFrame
{
	class Sender
	{

	public:

		static void run(std::vector<std::string> params)
		{
			std::ifstream is;
			is.open(params[1].c_str(), std::ios::in);



			is.close();
			std::cout << "sender";
		}

	};
}
#endif //DATA_FRAME_SENDER_H
