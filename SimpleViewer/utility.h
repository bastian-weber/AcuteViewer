#ifndef UTILITY
#define UTILITY

#include <vector>
#include <fstream>

//Qt
#include <QtCore>

namespace utility {

	std::vector<char> readFileIntoBuffer(QString const& path);

	bool isASCII(QString const& string);

}

#endif