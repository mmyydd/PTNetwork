#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "db_intr.hpp"

class db_sample : public db_intr
{
public:
	db_sample(bool pipe, std::string host, uint16_t port): 
		db_intr(pipe, host, port)
	{

	}

	virtual ~db_sample(){
	}
	

	virtual void on_connect_completed(){
		std::cout << "on_connect_completed" << std::endl;
	}

	virtual void on_connect_failed(){
		std::cout << "on_connect_failed" << std::endl;
	}

	virtual void on_lost_connection(){
		std::cout << "on_lost_connection" << std::endl;
	}
};

int main(int argc, char *argv[])
{
	db_sample mysample(false, "127.0.0.1", 27015);
	return 0;
}
