#include "utility.h"

namespace utility {

	std::shared_ptr<std::vector<char>> readFileIntoBuffer(QString const & path) {
		////some Qt code that also does the trick
		//QFile file(path);
		//std::vector<char> buffer;
		//buffer.resize(file.size());
		//if (!file.open(QIODevice::ReadOnly)) {
		//	return std::vector<char>();
		//}
		//file.read(buffer.data(), file.size());
		//file.close();

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

	bool moveFileToRecycleBin(QString const & filepath) {
#ifdef Q_OS_WIN
		if (!QFileInfo(filepath).exists()) return false;
		QString nativePath = QDir::toNativeSeparators(filepath);

		if (filepath.length() > MAX_PATH) return false;
		wchar_t* doubleNullTerminatedPath = new wchar_t[MAX_PATH + 2];
		wcscpy(doubleNullTerminatedPath, nativePath.toStdWString().c_str());
		memcpy(doubleNullTerminatedPath + nativePath.length() + 1, L"\0\0", 2);

		SHFILEOPSTRUCTW operation;
		operation.wFunc = FO_DELETE;
		operation.pFrom = doubleNullTerminatedPath;
		operation.fFlags = FOF_ALLOWUNDO | FOF_SILENT | FOF_NOCONFIRMATION;

		//without values below defined, operation will crash on Windows XP
		operation.hNameMappings = NULL;
		operation.lpszProgressTitle = NULL;
		operation.fAnyOperationsAborted = FALSE;
		operation.hwnd = NULL;
		operation.pTo = NULL;

		int result = SHFileOperationW(&operation);
		delete[] doubleNullTerminatedPath;
		if (result != 0) return false;

		return true;
#elif defined Q_OS_LINUX
		QString home = qgetenv("XDG_DATA_HOME").constData();
		QString user = qgetenv("HOME").constData();
		if(home.isEmpty()) home = user.append("/.local/share");
		QDir trash(home);
		if (!trash.exists()) return false;
		if (!trash.cd("Trash")) return false;
		if (!trash.cd("files")) return false;
		QFile::rename(filepath, trash.absoluteFilePath(QFileInfo(filepath).fileName()));
		return true;
#endif
		return false;
	}

	bool moveFile(QString const & oldPath, QString const & newPath, bool silent, QWidget * parent) {
		if (!QFile::exists(oldPath)) {
			if (!silent) QMessageBox::critical(parent,
											   QObject::tr("File Not Found"),
											   QObject::tr("The file \"%1\" could not be moved because it no longer exists.").arg(QFileInfo(oldPath).fileName()),
											   QMessageBox::StandardButton::Close,
											   QMessageBox::StandardButton::Close);
			return false;
		}
		if (QFile::exists(newPath)) {
			if (!silent) {
				QMessageBox msgBox;
				msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
				msgBox.setWindowTitle(QObject::tr("File Already Exists"));
				msgBox.setText(QObject::tr("There is already a file with the name \"%1\" in the destination folder. If you move the file nevertheless, the existing one will be overwritten.").arg(QFileInfo(oldPath).fileName()));
				msgBox.setButtonText(QMessageBox::Yes, QObject::tr("Overwrite"));
				msgBox.setButtonText(QMessageBox::No, QObject::tr("Cancel"));
				msgBox.setIcon(QMessageBox::Question);
				if (msgBox.exec() == QMessageBox::No) return false;
			}
			if (!QFile::remove(newPath)) {
				if (!silent) QMessageBox::critical(parent,
												   QObject::tr("File Not Moved"),
												   QObject::tr("The file \"%1\" could not be moved. Please check that you have the required permissions.").arg(QFileInfo(oldPath).fileName()),
												   QMessageBox::StandardButton::Close,
												   QMessageBox::StandardButton::Close);
				return false;
			}
		}
		if (!QFile::rename(oldPath, newPath)) {
			if (!silent) QMessageBox::critical(parent,
											   QObject::tr("File Not Moved"),
											   QObject::tr("The file \"%1\" could not be moved. Please check that you have the required permissions.").arg(QFileInfo(oldPath).fileName()),
											   QMessageBox::StandardButton::Close,
											   QMessageBox::StandardButton::Close);
			return false;
		}
		return true;
	}

	bool copyFile(QString const & oldPath, QString const & newPath, bool silent, QWidget * parent) {
		if (!QFile::exists(oldPath)) {
			if (!silent) QMessageBox::critical(parent,
											  QObject::tr("File Not Found"),
											  QObject::tr("The file \"%1\" could not be copied because it no longer exists.").arg(QFileInfo(oldPath).fileName()),
											  QMessageBox::StandardButton::Close,
											  QMessageBox::StandardButton::Close);
			return false;
		}
		if (QFile::exists(newPath)) {
			if (!silent) {
				QMessageBox msgBox;
				msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
				msgBox.setWindowTitle(QObject::tr("File Already Exists"));
				msgBox.setText(QObject::tr("There is already a file with the name \"%1\" in the destination folder. If you copy the file nevertheless, the existing one will be overwritten.").arg(QFileInfo(oldPath).fileName()));
				msgBox.setButtonText(QMessageBox::Yes, QObject::tr("Overwrite"));
				msgBox.setButtonText(QMessageBox::No, QObject::tr("Cancel"));
				msgBox.setIcon(QMessageBox::Question);
				if (msgBox.exec() == QMessageBox::No) return false;
			}
			if (!QFile::remove(newPath)) {
				if (!silent) QMessageBox::critical(parent,
												  QObject::tr("File Not Copied"),
												  QObject::tr("The file \"%1\" could not be copied. Please check that you have the required permissions.").arg(QFileInfo(oldPath).fileName()),
												  QMessageBox::StandardButton::Close,
												  QMessageBox::StandardButton::Close);
				return false;
			}
		}
		if (!QFile::copy(oldPath, newPath)) {
			if (!silent) QMessageBox::critical(parent,
											  QObject::tr("File Not Copied"),
											   QObject::tr("The file \"%1\" could not be copied. Please check that you have the required permissions.").arg(QFileInfo(oldPath).fileName()),
											  QMessageBox::StandardButton::Close,
											  QMessageBox::StandardButton::Close);
			return false;
		}
		return true;
	}

}
