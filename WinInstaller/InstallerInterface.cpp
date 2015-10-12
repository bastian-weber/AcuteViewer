#include "InstallerInterface.h"

namespace wi {

	InstallerInterface::InstallerInterface(QWidget *parent)
		: QMainWindow(parent),
		currentlySelectedPath(QDir::toNativeSeparators(QDir::cleanPath(QString(getenv("PROGRAMFILES")) + QString("/Simple Viewer")))) {

		this->mainWidget = new QWidget(this);

		this->descriptionLabel = new QLabel(tr("This will install Simple Viewer on your system under the specified path. It will also register the application for the \"Default Programs\" selection dialog and add an uninstallation entry to \"Programs and Features\"."), this);
		this->descriptionLabel->setWordWrap(true);
		this->descriptionLabel->setMinimumHeight(40);

		this->pathInput = new QLineEdit(this->currentlySelectedPath.absolutePath(), this);

		this->browseButton = new QPushButton(tr("Browse"), this);

		this->startMenuCheckbox = new QCheckBox(tr("Create Start Menu Entry"), 0);
		this->startMenuCheckbox->setChecked(true);

		this->okButton = new QPushButton(tr("Ok"), this);
		this->okButton->setDefault(true);
		this->cancelButton = new QPushButton(tr("Cancel"), this);

		this->buttonLayout = new QHBoxLayout();
		this->buttonLayout->addStretch(1);
		this->buttonLayout->addWidget(this->okButton);
		this->buttonLayout->addWidget(this->cancelButton);

		this->mainLayout = new QVBoxLayout();
		this->mainLayout->addWidget(this->descriptionLabel);
		this->mainLayout->addSpacing(20);
		this->mainLayout->addWidget(this->pathInput);
		this->mainLayout->addWidget(this->browseButton, 0, Qt::AlignRight);
		this->mainLayout->addSpacing(20);
		this->mainLayout->addWidget(this->startMenuCheckbox);
		this->mainLayout->addSpacing(20);
		this->mainLayout->addLayout(this->buttonLayout);

		this->mainWidget->setLayout(this->mainLayout);

		this->setCentralWidget(this->mainWidget);

		//InstallerInterface::install(installPath.absolutePath());
	}

	InstallerInterface::~InstallerInterface() {
		delete this->mainWidget;
		delete this->mainLayout;
		delete this->buttonLayout;
		delete this->descriptionLabel;
		delete this->pathInput;
		delete this->browseButton;
		delete this->startMenuCheckbox;
		delete this->okButton;
		delete this->cancelButton;
	}

	QSize InstallerInterface::sizeHint() const {
		return QSize(400, -1);
	}

	//============================================================================== PROTECTED ==============================================================================\\



	//=============================================================================== PRIVATE ===============================================================================\\

	void InstallerInterface::install(QString installPath) {
		InstallerInterface::installFiles(installPath);
		InstallerInterface::registerProgramInRegistry(installPath);
	}

	void InstallerInterface::registerProgramInRegistry(QString installPath) {
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
		//uninstallation entry
		registry.setValue("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/DisplayName", "Simple Viewer");
		registry.setValue("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/UninstallString", QString("\"%1\" %2").arg(QDir::toNativeSeparators(QDir(installPath).absoluteFilePath("WinInstaller.exe"))).arg("-uninstall"));
		registry.setValue("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/DisplayIcon", QString("\"%1\"").arg(QDir::toNativeSeparators(QDir(installPath).absoluteFilePath("data/icon.ico"))));
		registry.setValue("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/EstimatedSize", 84787);
		registry.setValue("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/Publisher", "Bastian Weber");
		registry.setValue("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/DisplayVersion", "Alpha 2");
		registry.setValue("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/NoModify", "1");
		registry.setValue("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/NoRepair", "1");
	}

	void InstallerInterface::copyAllFilesInDirectory(QDir const& sourceDir, QDir const& destinationDir) {
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

	void InstallerInterface::installFiles(QDir installPath) {
		if (!installPath.exists()) installPath.mkpath(installPath.absolutePath());
		installPath.mkpath(QDir::cleanPath(installPath.absolutePath() + QString("/data")));
		installPath.mkpath(QDir::cleanPath(installPath.absolutePath() + QString("/platforms")));
		QDir currentPath(QCoreApplication::applicationDirPath());
		InstallerInterface::copyAllFilesInDirectory(currentPath, installPath);
		currentPath.cd("data");
		installPath.cd("data");
		InstallerInterface::copyAllFilesInDirectory(currentPath, installPath);
		currentPath.cd("../platforms");
		installPath.cd("../platforms");
		QStringList filesInPlatformsFolder = currentPath.entryList();
		InstallerInterface::copyAllFilesInDirectory(currentPath, installPath);
	}

	//============================================================================ PRIVATE SLOTS =============================================================================\\



}