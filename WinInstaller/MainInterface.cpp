#include "MainInterface.h"

namespace sv {

	MainInterface::MainInterface(QWidget *parent)
		: QMainWindow(parent) {
		QDir installPath = QDir::toNativeSeparators(QDir::cleanPath(QString(getenv("PROGRAMFILES")) + QString("/Simple Viewer")));
		MainInterface::install(installPath.absolutePath());
	}

	MainInterface::~MainInterface() {

	}

	QSize MainInterface::sizeHint() const {
		return QSize(900, 600);
	}

	//============================================================================== PROTECTED ==============================================================================\\

	

	//=============================================================================== PRIVATE ===============================================================================\\

	void MainInterface::install(QString installPath) {
		MainInterface::installFiles(installPath);
		MainInterface::registerProgramInRegistry(installPath);
	}

	void MainInterface::registerProgramInRegistry(QString installPath) {
		installPath = QDir::toNativeSeparators(installPath);
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

	void MainInterface::copyAllFilesInDirectory(QDir const& sourceDir, QDir const& destinationDir) {
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

	void MainInterface::installFiles(QDir installPath) {
		if (!installPath.exists()) installPath.mkpath(installPath.absolutePath());
		installPath.mkpath(QDir::cleanPath(installPath.absolutePath() + QString("/data")));
		installPath.mkpath(QDir::cleanPath(installPath.absolutePath() + QString("/platforms")));
		QDir currentPath(QCoreApplication::applicationDirPath());
		MainInterface::copyAllFilesInDirectory(currentPath, installPath);
		currentPath.cd("data");
		installPath.cd("data");
		MainInterface::copyAllFilesInDirectory(currentPath, installPath);
		currentPath.cd("../platforms");
		installPath.cd("../platforms");
		QStringList filesInPlatformsFolder = currentPath.entryList();
		MainInterface::copyAllFilesInDirectory(currentPath, installPath);
	}

	//============================================================================ PRIVATE SLOTS =============================================================================\\



}