#include <iostream>

#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>

void registerProgramInRegistry(QString installPath) {
	installPath.replace("/", "\\");
	QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE", QSettings::NativeFormat);
	//filetypes
	QString openCommand = QString("\"%1\\SimpleViewer.exe\" \"%2\"").arg(installPath).arg("%1");
	registry.setValue("Classes/SimpleViewer.AssocFile.TIF/.", "Tif Image File");
	registry.setValue("Classes/SimpleViewer.AssocFile.TIF/shell/open/command/.", openCommand);
	registry.setValue("Classes/SimpleViewer.AssocFile.BMP/.", "Bitmap ImageFile");
	registry.setValue("Classes/SimpleViewer.AssocFile.BMP/shell/open/command/.", openCommand);
	registry.setValue("Classes/SimpleViewer.AssocFile.JPG/.", "Jpeg Image File");
	registry.setValue("Classes/SimpleViewer.AssocFile.JPG/shell/open/command/.", openCommand);
	registry.setValue("Classes/SimpleViewer.AssocFile.JP2/.", "Jpeg 2000 Image File");
	registry.setValue("Classes/SimpleViewer.AssocFile.JP2/shell/open/command/.", openCommand);
	registry.setValue("Classes/SimpleViewer.AssocFile.PNG/.", "Portable Network Graphics");
	registry.setValue("Classes/SimpleViewer.AssocFile.PNG/shell/open/command/.", openCommand);
	registry.setValue("Classes/SimpleViewer.AssocFile.WEBP/.", "WebP Image File");
	registry.setValue("Classes/SimpleViewer.AssocFile.WEBP/shell/open/command/.", openCommand);
	registry.setValue("Classes/SimpleViewer.AssocFile.PBM/.", "Portable Image Format");
	registry.setValue("Classes/SimpleViewer.AssocFile.PBM/shell/open/command/.", openCommand);
	registry.setValue("Classes/SimpleViewer.AssocFile.SR/.", "Sun Raster");
	registry.setValue("Classes/SimpleViewer.AssocFile.SR/shell/open/command/.", openCommand);
	//capabilities
	registry.setValue("Simple Viewer/Capabilities/ApplicationName", "Simple Viewer");
	registry.setValue("Simple Viewer/Capabilities/ApplicationDescription", "An image viewer featuring a minimal interface design and high-quality image display.");
	//tif
	registry.setValue("Simple Viewer/Capabilities/FileAssociations/.tif", "SimpleViewer.AssocFile.TIF");
	registry.setValue("Simple Viewer/Capabilities/FileAssociations/.tiff", "SimpleViewer.AssocFile.TIF");
	//bmp, dib
	registry.setValue("Simple Viewer/Capabilities/FileAssociations/.bmp", "SimpleViewer.AssocFile.BMP");
	registry.setValue("Simple Viewer/Capabilities/FileAssociations/.dib", "SimpleViewer.AssocFile.BMP");
	//jpeg, jpg, jpe
	registry.setValue("Simple Viewer/Capabilities/FileAssociations/.jpeg", "SimpleViewer.AssocFile.JPG");
	registry.setValue("Simple Viewer/Capabilities/FileAssociations/.jpg", "SimpleViewer.AssocFile.JPG");
	registry.setValue("Simple Viewer/Capabilities/FileAssociations/.jpe", "SimpleViewer.AssocFile.JPG");
	//jp2
	registry.setValue("Simple Viewer/Capabilities/FileAssociations/.jp2", "SimpleViewer.AssocFile.JP2");
	//png
	registry.setValue("Simple Viewer/Capabilities/FileAssociations/.png", "SimpleViewer.AssocFile.PNG");
	//webp
	registry.setValue("Simple Viewer/Capabilities/FileAssociations/.webp", "SimpleViewer.AssocFile.WEBP");
	//pbm, pgm, ppm
	registry.setValue("Simple Viewer/Capabilities/FileAssociations/.pbm", "SimpleViewer.AssocFile.PBM");
	registry.setValue("Simple Viewer/Capabilities/FileAssociations/.pgm", "SimpleViewer.AssocFile.PBM");
	registry.setValue("Simple Viewer/Capabilities/FileAssociations/.ppm", "SimpleViewer.AssocFile.PBM");
	//sr, ras
	registry.setValue("Simple Viewer/Capabilities/FileAssociations/.sr", "SimpleViewer.AssocFile.SR");
	registry.setValue("Simple Viewer/Capabilities/FileAssociations/.ras", "SimpleViewer.AssocFile.SR");
	//register application
	registry.setValue("RegisteredApplications/Simple Viewer", "SOFTWARE\\Simple Viewer\\Capabilities");
}

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
}

void copyAllFilesInDirectory(QDir const& sourceDir, QDir const& destinationDir) {
	if (sourceDir != destinationDir) {
		QStringList filesInFolder = sourceDir.entryList();
		for (QString const& entry : filesInFolder) {
			QString oldPath = sourceDir.absoluteFilePath(entry);
			QString newPath = destinationDir.absoluteFilePath(entry);
			if (QFile::exists(newPath)) {
				QFile::remove(newPath);
			}
			QFile::copy(oldPath, newPath);
		}
	}
}

void installFiles(QDir installPath) {
	if (!installPath.exists()) installPath.mkpath(installPath.absolutePath());
	installPath.mkpath(QDir::cleanPath(installPath.absolutePath() + QString("/data")));
	installPath.mkpath(QDir::cleanPath(installPath.absolutePath() + QString("/platforms")));
	QDir currentPath(QCoreApplication::applicationDirPath());
	copyAllFilesInDirectory(currentPath, installPath);
	currentPath.cd("data");
	installPath.cd("data");
	copyAllFilesInDirectory(currentPath, installPath);
	currentPath.cd("../platforms");
	installPath.cd("../platforms");
	QStringList filesInPlatformsFolder = currentPath.entryList();
	copyAllFilesInDirectory(currentPath, installPath);
}

void removeFiles() {
	QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE", QSettings::NativeFormat);
	QString programPath = registry.value("Classes/SimpleViewer.AssocFile.TIF/shell/open/command/.").toString();
	programPath = programPath.section('"', 1, 1);
	std::cout << programPath.toStdString() << std::endl;
	if (QFileInfo(programPath).exists()) {
		std::cout << "exists" << std::endl;
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

	//sv::MainInterface* mainInterface = new sv::MainInterface(openWithFilename);
	//mainInterface->show();

	if (QCoreApplication::arguments().contains("-uninstall", Qt::CaseInsensitive)) {
		removeFiles();
	} else {
		QDir installPath = QDir::cleanPath(QString(getenv("PROGRAMFILES")) + QString("/Simple Viewer"));
		installFiles(installPath);
		registerProgramInRegistry(installPath.absolutePath().replace("/", "\\"));
	}
	return app.exec();
}

int main(int argc, char* argv[]) {
	return init(argc, argv);
}