#include <iostream>
#include <chrono>
#include <thread>

#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>

#include "InstallerInterface.h"

namespace uninstallation {

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
							"libexiv2.dll",
							"libexpat.dll",
							"zlib1.dll",
							"opencv_world300.dll",
							"SimpleViewer.exe",
							"SimpleViewer.ini",
							"readme.txt",
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
							"data/bmp.ico",
							"data/jp2.ico",
							"data/jpg.ico",
							"data/pbm.ico",
							"data/png.ico",
							"data/sr.ico",
							"data/tif.ico",
							"data/webp.ico",
							"platforms/qminimal.dll",
							"platforms/qminimald.dll",
							"platforms/qminimald.pdb",
							"platforms/qoffscreen.dll",
							"platforms/qoffscreend.dll",
							"platforms/qoffscreend.pdb",
							"platforms/qwindows.dll",
							"platforms/qwindowsd.dll",
							"platforms/qwindowsd.pdb", });
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

	//Not used at the moment. Settings are kept when the application is uninstalled.
	void removeSettings() {
		QString settingsPath;
		{
			settingsPath = QSettings(QSettings::IniFormat, QSettings::UserScope, "Simple Viewer", "Simple Viewer").fileName();
		}
		QFile::remove(settingsPath);
		QDir settingsDir = QFileInfo(settingsPath).absoluteDir();
		settingsDir.cd("..");
		settingsDir.rmdir("Simple Viewer");
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
		QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE", QSettings::NativeFormat);
		if (registry.contains("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/UninstallString")) {
			if (QMessageBox::Yes == QMessageBox::question(nullptr,
														  QObject::tr("Uninstall"),
														  QObject::tr("Are you sure you want to remove the Simple Viewer application from your system?"),
														  QMessageBox::Yes | QMessageBox::No)) {
				QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE", QSettings::NativeFormat);
				QDir installDir(QFileInfo(registry.value("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/UninstallString").toString().section('"', 1, 1)).path());
				uninstallation::removeFiles(installDir);
				uninstallation::clearRegistryEntries();
				QMessageBox::information(nullptr,
										 QObject::tr("Uninstall"),
										 QObject::tr("The application was successfully removed."),
										 QMessageBox::Close);
				uninstallation::initiateSelfRemoval(installDir);
			}
		} else {
			//I can still try to clear registry entries that might be there
			uninstallation::clearRegistryEntries();
			//but I can't remove any files
			QMessageBox::critical(nullptr,
								  QObject::tr("Application Not Found"),
								  QObject::tr("Uninstallation is not possible because no existing Simple Viewer installation could be found."),
								  QMessageBox::Close);
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