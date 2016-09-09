#ifndef UTILITY
#define UTILITY

#include <vector>
#include <fstream>
#include <memory>
#include <iostream>

//Qt
#include <QtCore>

//Windows
#ifdef Q_OS_WIN
#include <Windows.h>
#endif

namespace utility {

	std::shared_ptr<std::vector<char>> readFileIntoBuffer(QString const & path);

	bool isCharCompatible(QString const & string);

	QString getShortPathname(QString const & path);

}

#endif