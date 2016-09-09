#include "utility.h"

namespace utility {

	std::shared_ptr<std::vector<char>> readFileIntoBuffer(QString const & path) {
#ifdef Q_OS_WIN
		//wchar for utf-16
		std::ifstream file(path.toStdWString(), std::iostream::binary);
#else
		//char for utf-8
		std::ifstream file(path.toStdString(), std::iostream::binary);
#endif
		if (!file.good()) {
			return std::shared_ptr<std::vector<char>>(new std::vector<char>());
		}
		file.exceptions(std::ifstream::badbit | std::ifstream::failbit | std::ifstream::eofbit);
		file.seekg(0, std::ios::end);
		std::streampos length(file.tellg());
		std::shared_ptr<std::vector<char>> buffer(new std::vector<char>(static_cast<std::size_t>(length)));
		if (static_cast<std::size_t>(length) == 0) {
			return std::shared_ptr<std::vector<char>>(new std::vector<char>());
		}
		file.seekg(0, std::ios::beg);
		try {
			file.read(buffer->data(), static_cast<std::size_t>(length));
		} catch (...) {
			return std::shared_ptr<std::vector<char>>(new std::vector<char>());
		}
		file.close();
		return buffer;
	}

	bool isCharCompatible(QString const & string) {
#ifdef Q_OS_WIN
		bool isCharCompatible = true;
		for (QString::ConstIterator i = string.begin(); i != string.end(); ++i) {
			isCharCompatible = isCharCompatible && (i->unicode() < 128);
		}
		return isCharCompatible;
#else
		return true;
#endif
	}

	QString getShortPathname(QString const & path) {
#ifndef Q_OS_WIN
		return QString();
#else
		long length = 0;
		WCHAR* buffer = nullptr;
		length = GetShortPathNameW(path.toStdWString().c_str(), nullptr, 0);
		std::cout << length << std::endl;
		if (length == 0) return QString();
		buffer = new WCHAR[length];
		length = GetShortPathNameW(path.toStdWString().c_str(), buffer, length);
		std::wcout << buffer << std::endl;
		if (length == 0) {
			delete[] buffer;
			return QString();
		}
		QString result = QString::fromWCharArray(buffer);
		delete[] buffer;
		return result;
#endif
	}

}