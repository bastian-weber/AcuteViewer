#include "InstallerInterface.h"

namespace wi {

	InstallerInterface::InstallerInterface(QWidget *parent)
		: QMainWindow(parent),
		currentlySelectedPath(QDir::toNativeSeparators(QDir::cleanPath(QString(getenv("PROGRAMFILES")) + QString("/Acute Viewer")))) {

		QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE", QSettings::NativeFormat);
		if (registry.contains("Microsoft/Windows/CurrentVersion/Uninstall/AcuteViewer/UninstallString")) {
			currentlySelectedPath = QDir(QFileInfo(registry.value("Microsoft/Windows/CurrentVersion/Uninstall/AcuteViewer/UninstallString").toString().section('"', 1, 1)).path());
		}
		setWindowTitle("Acute Viewer Installer");

		mainWidget = new QWidget(this);

		descriptionLabel = new QLabel(tr("This will install Acute Viewer on your system under the specified path (a subdirectory will be created automatically). The application will also be registered for the \"Default Programs\" selection dialog and an uninstallation entry will be added to \"Programs and Features\"."), this);
		descriptionLabel->setWordWrap(true);
		descriptionLabel->setMinimumHeight(descriptionLabel->sizeHint().height());

		pathInput = new QLineEdit(currentlySelectedPath.absolutePath(), this);
		pathInput->setReadOnly(true);

		browseButton = new QPushButton(tr("&Browse"), this);
		QObject::connect(browseButton, SIGNAL(clicked()), this, SLOT(reactToBrowseButtonClick()));

		startMenuCheckbox = new QCheckBox(tr("Create Start Menu Entry"), 0);
		startMenuCheckbox->setChecked(true);

		okButton = new QPushButton(tr("&Ok"), this);
		okButton->setDefault(true);
		QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(install()));
		cancelButton = new QPushButton(tr("&Cancel"), this);
		QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));

		buttonLayout = new QHBoxLayout();
		buttonLayout->addStretch(1);
		buttonLayout->addWidget(okButton);
		buttonLayout->addWidget(cancelButton);

		mainLayout = new QVBoxLayout();
		mainLayout->addWidget(descriptionLabel);
		mainLayout->addSpacing(20);
		mainLayout->addWidget(pathInput);
		mainLayout->addWidget(browseButton, 0, Qt::AlignRight);
		mainLayout->addSpacing(20);
		mainLayout->addWidget(startMenuCheckbox);
		mainLayout->addSpacing(20);
		mainLayout->addLayout(buttonLayout);

		mainWidget->setLayout(mainLayout);

		setCentralWidget(mainWidget);
	}

	InstallerInterface::~InstallerInterface() {
		delete mainWidget;
		delete mainLayout;
		delete buttonLayout;
		delete descriptionLabel;
		delete pathInput;
		delete browseButton;
		delete startMenuCheckbox;
		delete okButton;
		delete cancelButton;
	}

	QSize InstallerInterface::sizeHint() const {
		return QSize(400, -1);
	}

	//============================================================================== PROTECTED ==============================================================================\\



	//=============================================================================== PRIVATE ===============================================================================\\

	void InstallerInterface::registerProgramInRegistry(QDir installPath) {
		QDir dataPath = installPath;
		dataPath.cd("data");
		QString openCommand = QString("\"%1\\AcuteViewer.exe\" \"%2\"").arg(QDir::toNativeSeparators(installPath.absolutePath())).arg("%1");
		//filetypes
		QString icoPath = QString("\"%1\\%2.ico\"").arg(QDir::toNativeSeparators(dataPath.absolutePath()));
		QSettings registryHklm("HKEY_LOCAL_MACHINE\\SOFTWARE", QSettings::NativeFormat);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.TIF/.", "Tif Image File");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.TIF/DefaultIcon/.", icoPath.arg("tif"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.TIF/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.BMP/.", "Bitmap ImageFile");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.BMP/DefaultIcon/.", icoPath.arg("bmp"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.BMP/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.JPG/.", "Jpeg Image File");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.JPG/DefaultIcon/.", icoPath.arg("jpg"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.JPG/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.JP2/.", "Jpeg 2000 Image File");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.JP2/DefaultIcon/.", icoPath.arg("jp2"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.JP2/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.PNG/.", "Portable Network Graphics");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.PNG/DefaultIcon/.", icoPath.arg("png"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.PNG/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.WEBP/.", "WebP Image File");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.WEBP/DefaultIcon/.", icoPath.arg("webp"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.WEBP/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.PBM/.", "Portable Image Format");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.PBM/DefaultIcon/.", icoPath.arg("pbm"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.PBM/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.SR/.", "Sun Raster");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.SR/DefaultIcon/.", icoPath.arg("sr"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.SR/shell/open/command/.", openCommand);
		//partially supported filetypes
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.ARW/.", "Sony Alpha Raw");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.ARW/DefaultIcon/.", icoPath.arg("arw"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.ARW/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.CR2/.", "Canon Raw Version 2");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.CR2/DefaultIcon/.", icoPath.arg("cr2"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.CR2/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.CRW/.", "Canon Raw");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.CRW/DefaultIcon/.", icoPath.arg("crw"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.CRW/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.DNG/.", "Digital Negative");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.DNG/DefaultIcon/.", icoPath.arg("dng"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.DNG/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.MRW/.", "Minolta Raw");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.MRW/DefaultIcon/.", icoPath.arg("mrw"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.MRW/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.NEF/.", "Nikon Raw");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.NEF/DefaultIcon/.", icoPath.arg("nef"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.NEF/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.ORF/.", "Olympus Raw");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.ORF/DefaultIcon/.", icoPath.arg("orf"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.ORF/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.PEF/.", "Pentax Raw");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.PEF/DefaultIcon/.", icoPath.arg("pef"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.PEF/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.PSD/.", "Photoshop Document");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.PSD/DefaultIcon/.", icoPath.arg("psd"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.PSD/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.RAF/.", "Fuji Raw");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.RAF/DefaultIcon/.", icoPath.arg("raf"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.RAF/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.RW2/.", "Panasonic Raw");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.RW2/DefaultIcon/.", icoPath.arg("rw2"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.RW2/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.SR2/.", "Sony Raw");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.SR2/DefaultIcon/.", icoPath.arg("sr2"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.SR2/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.SRF/.", "Sony Raw");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.SRF/DefaultIcon/.", icoPath.arg("srf"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.SRF/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.SRW/.", "Samsung Raw");
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.SRW/DefaultIcon/.", icoPath.arg("srw"));
		registryHklm.setValue("Classes/AcuteViewer.AssocFile.SRW/shell/open/command/.", openCommand);
		//capabilities
		registryHklm.setValue("Acute Viewer/Capabilities/ApplicationName", "Acute Viewer");
		registryHklm.setValue("Acute Viewer/Capabilities/ApplicationDescription", "An image viewer featuring a minimal interface design and high-quality image display.");
		//tif
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.tif", "AcuteViewer.AssocFile.TIF");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.tiff", "AcuteViewer.AssocFile.TIF");
		//bmp, dib
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.bmp", "AcuteViewer.AssocFile.BMP");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.dib", "AcuteViewer.AssocFile.BMP");
		//jpeg, jpg, jpe
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.jpeg", "AcuteViewer.AssocFile.JPG");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.jpg", "AcuteViewer.AssocFile.JPG");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.jpe", "AcuteViewer.AssocFile.JPG");
		//jp2
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.jp2", "AcuteViewer.AssocFile.JP2");
		//png
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.png", "AcuteViewer.AssocFile.PNG");
		//webp
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.webp", "AcuteViewer.AssocFile.WEBP");
		//pbm, pgm, ppm
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.pbm", "AcuteViewer.AssocFile.PBM");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.pgm", "AcuteViewer.AssocFile.PBM");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.ppm", "AcuteViewer.AssocFile.PBM");
		//sr, ras
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.sr", "AcuteViewer.AssocFile.SR");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.ras", "AcuteViewer.AssocFile.SR");
		//partially supported files
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.arw", "AcuteViewer.AssocFile.ARW");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.cr2", "AcuteViewer.AssocFile.CR2");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.crw", "AcuteViewer.AssocFile.CRW");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.dng", "AcuteViewer.AssocFile.DNG");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.mrw", "AcuteViewer.AssocFile.MRW");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.nef", "AcuteViewer.AssocFile.NEF");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.orf", "AcuteViewer.AssocFile.ORF");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.pef", "AcuteViewer.AssocFile.PEF");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.psd", "AcuteViewer.AssocFile.PSD");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.raf", "AcuteViewer.AssocFile.RAF");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.rw2", "AcuteViewer.AssocFile.RW2");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.sr2", "AcuteViewer.AssocFile.SR2");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.srf", "AcuteViewer.AssocFile.SRF");
		registryHklm.setValue("Acute Viewer/Capabilities/FileAssociations/.srw", "AcuteViewer.AssocFile.SRW");
		//register application
		registryHklm.setValue("RegisteredApplications/Acute Viewer", "SOFTWARE\\Acute Viewer\\Capabilities");
		//uninstallation entry
		registryHklm.setValue("Microsoft/Windows/CurrentVersion/Uninstall/AcuteViewer/DisplayName", "Acute Viewer");
		registryHklm.setValue("Microsoft/Windows/CurrentVersion/Uninstall/AcuteViewer/UninstallString", QString("\"%1\" %2").arg(QDir::toNativeSeparators(QDir(installPath).absoluteFilePath("WinInstaller.exe"))).arg("-uninstall"));
		registryHklm.setValue("Microsoft/Windows/CurrentVersion/Uninstall/AcuteViewer/DisplayIcon", QString("\"%1\"").arg(QDir::toNativeSeparators(QDir(installPath).absoluteFilePath("data/icon.ico"))));
		registryHklm.setValue("Microsoft/Windows/CurrentVersion/Uninstall/AcuteViewer/EstimatedSize", 108544);
		registryHklm.setValue("Microsoft/Windows/CurrentVersion/Uninstall/AcuteViewer/Publisher", "Bastian Weber");
		registryHklm.setValue("Microsoft/Windows/CurrentVersion/Uninstall/AcuteViewer/DisplayVersion", QString("%1.%2").arg(AV_MAJOR_VERSION).arg(AV_MINOR_VERSION));
		registryHklm.setValue("Microsoft/Windows/CurrentVersion/Uninstall/AcuteViewer/NoModify", "1");
		registryHklm.setValue("Microsoft/Windows/CurrentVersion/Uninstall/AcuteViewer/NoRepair", "1");
		//set friendly app name
		QSettings registryHkcr("HKEY_CLASSES_ROOT\\Applications", QSettings::NativeFormat);
		registryHkcr.setValue("AcuteViewer.exe/shell/open/FriendlyAppName", "Acute Viewer");
	}

	bool InstallerInterface::copyAllFilesInDirectory(QDir const& sourceDir, QDir const& destinationDir) {
		bool allSuccessful = true;
		if (sourceDir != destinationDir) {
			QStringList filesInFolder = sourceDir.entryList(QDir::NoDotAndDotDot | QDir::Files);
			for (QString const& entry : filesInFolder) {
				QString oldPath = sourceDir.absoluteFilePath(entry);
				QString newPath = destinationDir.absoluteFilePath(entry);
				if (QFile::exists(newPath)) {
					QFile::remove(newPath);
				}
				allSuccessful = QFile::copy(oldPath, newPath) && allSuccessful;
				QCoreApplication::processEvents();
			}
		}
		return allSuccessful;
	}

	bool InstallerInterface::installFiles(QDir installPath) {
		bool allSuccessful = true;
		QDir currentPath(QCoreApplication::applicationDirPath());
		if (currentPath != installPath) {
			if (!installPath.exists()) installPath.mkpath(installPath.absolutePath());
			installPath.mkpath(QDir::cleanPath(installPath.absolutePath() + QString("/data")));
			installPath.mkpath(QDir::cleanPath(installPath.absolutePath() + QString("/platforms")));
			allSuccessful = InstallerInterface::copyAllFilesInDirectory(currentPath, installPath) && allSuccessful;
			currentPath.cd("data");
			installPath.cd("data");
			allSuccessful = InstallerInterface::copyAllFilesInDirectory(currentPath, installPath);
			currentPath.cd("../platforms");
			installPath.cd("../platforms");
			QStringList filesInPlatformsFolder = currentPath.entryList();
			allSuccessful = InstallerInterface::copyAllFilesInDirectory(currentPath, installPath);
		}
		return allSuccessful;
	}

	bool InstallerInterface::createStartMenuEntry(QString targetPath) {
		targetPath = QDir::toNativeSeparators(targetPath);

		WCHAR startMenuPath[MAX_PATH];
		HRESULT result = SHGetFolderPathW(NULL, CSIDL_COMMON_PROGRAMS, NULL, 0, startMenuPath);

		if (SUCCEEDED(result)) {
			QString linkPath = QDir(QString::fromWCharArray(startMenuPath)).absoluteFilePath("Acute Viewer.lnk");

			CoInitialize(NULL);
			IShellLinkW* shellLink = NULL;
			result = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_ALL, IID_IShellLinkW, (void**)&shellLink);
			if (SUCCEEDED(result)) {
				shellLink->SetPath(targetPath.toStdWString().c_str());
				shellLink->SetDescription(L"Acute Viewer image viewer");
				shellLink->SetIconLocation(targetPath.toStdWString().c_str(), 0);
				IPersistFile* persistFile;
				result = shellLink->QueryInterface(IID_IPersistFile, (void**)&persistFile);

				if (SUCCEEDED(result)) {
					result = persistFile->Save(linkPath.toStdWString().c_str(), TRUE);

					persistFile->Release();
				} else {
					return false;
				}
				shellLink->Release();
			} else {
				return false;
			}
		} else {
			return false;
		}
		return true;
	}

	void InstallerInterface::disableControls() {
		browseButton->setEnabled(false);
		startMenuCheckbox->setEnabled(false);
		okButton->setEnabled(false);
		cancelButton->setEnabled(false);
	}

	//============================================================================ PRIVATE SLOTS =============================================================================\\

	void InstallerInterface::install() {
		disableControls();
		setWindowTitle(windowTitle() + QString("- Installing..."));
		QCoreApplication::processEvents();
		bool allFilesCopied = InstallerInterface::installFiles(currentlySelectedPath);
		InstallerInterface::registerProgramInRegistry(currentlySelectedPath);
		if (startMenuCheckbox->isChecked()) createStartMenuEntry(currentlySelectedPath.absoluteFilePath("AcuteViewer.exe"));
		if (allFilesCopied) {
			QMessageBox::information(this,
									 tr("Installation Successful"),
									 tr("The installation was successful. The installer will now quit."),
									 QMessageBox::Close);
		} else {
			QMessageBox::information(this,
									 tr("Installation Completed"),
									 tr("The installation completed, but not all files could be copied. If you are trying to overwrite an older Acute Viewer installation, make sure it's not running and try again. The installer will now quit."),
									 QMessageBox::Close);
		}
		QCoreApplication::quit();
	}

	void InstallerInterface::reactToBrowseButtonClick() {
		QString path = QFileDialog::getExistingDirectory(this, tr("Select Installation Directory"), currentlySelectedPath.absolutePath());

		if (!path.isEmpty()) {
			currentlySelectedPath = QDir(path).absoluteFilePath("Acute Viewer");
			pathInput->setText(currentlySelectedPath.absolutePath());
		}
	}

}