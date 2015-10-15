#include <iostream>
#include <chrono>
#include <thread>

#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>

#include "InstallerInterface.h"

void clearRegistryEntries() {
	QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE", QSettings::NativeFormat);
	//file types
	registry.remove("Classes/SimpleViewer.AssocFile.TIF");
	registry.remove("Classes/SimpleViewer.AssocFile.BMP");
	registry.remove("Classes/SimpleViewer.AssocFile.JPG");
	registry.remove("Classes/SimpleViewer.AssocFile.JP2");
	registry.remove("Classes/SimpleViewer.AssocFile.PNG");
	registry.remove("Classes/SimpleViewer.AssocFile.WEBP");
	registry.remove("Classes/SimpleViewer.AssocFile.PBM");
	registry.remove("Classes/SimpleViewer.AssocFile.SR");
	//capabilities
	registry.remove("Simple Viewer");
	//application registration
	registry.remove("RegisteredApplications/Simple Viewer");
	//uninstallation entries
	registry.remove("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer");
}

void removeFiles(QDir const& installDir) {
	QStringList files({ "icudt54.dll",
						"icuin54.dll",
						"icuuc54.dll",
						"opencv_world300.dll",
						"SimpleViewer.exe",
						"SimpleViewer.ini",
						"data/icon.ico",
						"data/icon_16.png",
						"data/icon_16_installer.png",
						"data/icon_24.png",
						"data/icon_24_installer.png",
						"data/icon_32.png",
						"data/icon_32_installer.png",
						"data/icon_48.png",
						"data/icon_48_installer.png",
						"data/icon_64.png",
						"data/icon_64_installer.png",
						"data/icon_96.png",
						"data/icon_96_installer.png",
						"data/icon_128.png",
						"data/icon_128_installer.png",
						"data/icon_192.png",
						"data/icon_192_installer.png",
						"data/icon_256.png",
						"data/icon_256_installer.png",
						"data/icon_installer.ico",
						"platforms/qminimal.dll",
						"platforms/qminimald.dll",
						"platforms/qoffscreen.dll",
						"platforms/qoffscreend.dll",
						"platforms/qwindows.dll",
						"platforms/qwindowsd.dll" });
	for (QString const& entry : files) {
		QString path = installDir.absoluteFilePath(entry);
		QFile::remove(path);
	}
	installDir.rmdir("data");

	//remove start menu entry
	WCHAR startMenuPath[MAX_PATH];
	HRESULT result = SHGetFolderPathW(NULL, CSIDL_COMMON_PROGRAMS, NULL, 0, startMenuPath);

	if (SUCCEEDED(result)) {
		QFile::remove(QDir(QString::fromWCharArray(startMenuPath)).absoluteFilePath("Simple Viewer.lnk"));
	}

}

void initiateSelfRemoval(QDir installDir) {
	//remove files that can only be removed after installer terminated
	QString file1 = QDir::toNativeSeparators(installDir.absoluteFilePath("WinInstaller.exe"));
	QString file2 = QDir::toNativeSeparators(installDir.absoluteFilePath("Qt5Core.dll"));
	QString file3 = QDir::toNativeSeparators(installDir.absoluteFilePath("Qt5Gui.dll"));
	QString file4 = QDir::toNativeSeparators(installDir.absoluteFilePath("Qt5Widgets.dll"));
	QString file5 = QDir::toNativeSeparators(installDir.absoluteFilePath("platforms/qwindows.dll"));
	QString path1 = QDir::toNativeSeparators(installDir.absoluteFilePath("platforms"));
	QString path2 = QDir::toNativeSeparators(installDir.absolutePath());
	QString parameters = QString("/C choice /C Y /N /D Y /T 3 & del \"%1\" & del \"%2\" & del \"%3\" & del \"%4\" & del \"%5\" & rmdir \"%6\" & rmdir \"%7\" ").arg(file1).arg(file2).arg(file3).arg(file4).arg(file5).arg(path1).arg(path2);
	QString cmdPath = getenv("ComSpec");
	ShellExecute(0, 0, QString("\"%1\"").arg(cmdPath).toStdString().c_str(), parameters.toStdString().c_str(), QDir::rootPath().toStdString().c_str(), SW_HIDE);
}

int init(int argc, char* argv[]) {
	QApplication app(argc, argv);

	QIcon icon;
	icon.addFile("./data/icon_16_installer.png");
	icon.addFile("./data/icon_32_installer.png");
	icon.addFile("./data/icon_48_installer.png");
	icon.addFile("./data/icon_64_installer.png");
	icon.addFile("./data/icon_96_installer.png");
	icon.addFile("./data/icon_128_installer.png");
	icon.addFile("./data/icon_192_installer.png");
	icon.addFile("./data/icon_256_installer.png");
	app.setWindowIcon(icon);

	if (QCoreApplication::arguments().contains("-uninstall", Qt::CaseInsensitive)) {
		QMessageBox msgBox;
		QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE", QSettings::NativeFormat);
		if (registry.contains("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/UninstallString")) {
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox.setIcon(QMessageBox::Question);
			msgBox.setWindowTitle(QObject::tr("Uninstall"));
			msgBox.setText(QObject::tr("Are you sure you want to remove the Simple Viewer application from your system?"));
			if (QMessageBox::Yes == msgBox.exec()) {
				QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE", QSettings::NativeFormat);
				QDir installDir(QFileInfo(registry.value("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/UninstallString").toString().section('"', 1, 1)).path());
				removeFiles(installDir);
				clearRegistryEntries();
				msgBox.setWindowTitle(QObject::tr("Uninstall"));
				msgBox.setText(QObject::tr("The application was successfully removed."));
				msgBox.setIcon(QMessageBox::Information);
				msgBox.setStandardButtons(QMessageBox::Ok);
				msgBox.exec();
				initiateSelfRemoval(installDir);
			}
		} else {
			clearRegistryEntries();
			msgBox.setWindowTitle(QObject::tr("Application Not Found"));
			msgBox.setText(QObject::tr("Uninstallation is not possible because no existing Simple Viewer installation could be found."));
			msgBox.setIcon(QMessageBox::Critical);
			msgBox.setStandardButtons(QMessageBox::Close);
			msgBox.exec();
		}
	} else {
		wi::InstallerInterface* InstallerInterface = new wi::InstallerInterface();
		InstallerInterface->show();
		return app.exec();
	}
	return 0;
}

int main(int argc, char* argv[]) {
	return init(argc, argv);
}