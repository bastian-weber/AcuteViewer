#pragma once

#include <vector>
#include <fstream>
#include <memory>

//Qt
#include <QtCore>
#include <QtGui>
#include <QtWidgets>

//Windows
#ifdef Q_OS_WIN
#include <Windows.h>
#endif

namespace utility {

	std::shared_ptr<std::vector<char>> readFileIntoBuffer(QString const& path);

	bool isCharCompatible(QString const& string);

	bool moveFileToRecycleBin(QString const & filepath);

	bool moveFile(QString const & oldPath, QString const & newPath, bool silent = false, QWidget * parent = 0);

	bool copyFile(QString const & oldPath, QString const & newPath, bool silent = false, QWidget * parent = 0);

}