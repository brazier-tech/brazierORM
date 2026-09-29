// brazierORM.cpp: определяет точку входа для приложения.
//

#include <string>
#include "brazierORM.h"
#include "include/Logger.hpp"
#include "include/Database.hpp"

using namespace brazier;

int main()
{
	setlocale(LC_ALL, "ru");
	try {
		Database db("localhost", "5432", "test", "test", "test");
	}
	catch (const std::exception& e) {
		Logger::log(std::string("Exception: ") + e.what(), "ERROR");
	}

	return 0;
}
