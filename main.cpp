#include <iostream>
#include <signal.h>

#include "./include/server.h"

void inthand(int signum)
{
	throw std::runtime_error(" Interrupt signal received.");
}

int main(int argc, char *argv[])
{
	try
	{
		signal(SIGINT, inthand);

		server();
	}

	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	std::cout << "Application exiting." << std::endl;
	return 0;
}
