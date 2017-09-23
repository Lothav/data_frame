#include <iostream>
#include "Module.h"


int main(int argc, char **argv) {

	DataFrame::Module* _data_frame = new DataFrame::Module();
	_data_frame->bootstrap(argc, argv);

	delete _data_frame;

    return EXIT_SUCCESS;
}