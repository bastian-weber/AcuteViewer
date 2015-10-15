#include "InstallerInterface.h"

namespace wi {

	InstallerInterface::InstallerInterface(QWidget *parent)
		: QMainWindow(parent),
		currentlySelectedPath(QDir::toNativeSeparators(QDir::cleanPath(QString(getenv("PROGRAMFILES")) + QString("/Simple Viewer")))) {
		
		QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE", QSettings::NativeFormat);
		if (registry.contains("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/UninstallString")) {
			currentlySelectedPath = QDir(QFileInfo(registry.value("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/UninstallString").toString().section('"', 1, 1)).path());
		}
		this->setWindowTitle("Simple Viewer Installer");

		this->mainWidget = new QWidget(this);

		this->descriptionLabel = new QLabel(tr("This will install Simple Viewer on your system under the specified path (a subdirectory will be created automatically). The application will also be registered for the \"Default Programs\" selection dialog and an uninstallation entry will be added to \"Programs and Features\"."), this);
		this->descriptionLabel->setWordWrap(true);
		this->descriptionLabel->setMinimumHeight(this->descriptionLabel->sizeHint().height());

		this->pathInput = new QLineEdit(this->currentlySelectedPath.absolutePath(), this);
		this->pathInput->setReadOnly(true);

		this->browseButton = new QPushButton(tr("&Browse"), this);
		QObject::connect(this->browseButton, SIGNAL(clicked()), this, SLOT(reactToBrowseButtonClick()));

		this->startMenuCheckbox = new QCheckBox(tr("Create Start Menu Entry"), 0);
		this->startMenuCheckbox->setChecked(true);

		this->okButton = new QPushButton(tr("&Ok"), this);
		this->okButton->setDefault(true);
		QObject::connect(this->okButton, SIGNAL(clicked()), this, SLOT(install()));
		this->cancelButton = new QPushButton(tr("&Cancel"), this);
		QObject::connect(this->cancelButton, SIGNAL(clicked()), this, SLOT(close()));

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

	void InstallerInterface::registerProgramInRegistry(QDir installPath) {
		QDir dataPath = installPath;
		dataPath.cd("data");
		QString openCommand = QString("\"%1\\SimpleViewer.exe\" \"%2\"").arg(QDir::toNativeSeparators(installPath.absolutePath())).arg("%1");
		//filetypes
		QString icoPath = QString("\"%1\\%2.ico\"").arg(QDir::toNativeSeparators(dataPath.absolutePath()));
		QSettings registryHklm("HKEY_LOCAL_MACHINE\\SOFTWARE", QSettings::NativeFormat);
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.TIF/.", "Tif Image File");
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.TIF/DefaultIcon/.", icoPath.arg("tif"));
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.TIF/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.BMP/.", "Bitmap ImageFile");
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.BMP/DefaultIcon/.", icoPath.arg("bmp"));
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.BMP/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.JPG/.", "Jpeg Image File");
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.JPG/DefaultIcon/.", icoPath.arg("jpg"));
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.JPG/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.JP2/.", "Jpeg 2000 Image File");
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.JP2/DefaultIcon/.", icoPath.arg("jp2"));
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.JP2/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.PNG/.", "Portable Network Graphics");
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.PNG/DefaultIcon/.", icoPath.arg("png"));
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.PNG/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.WEBP/.", "WebP Image File");
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.WEBP/DefaultIcon/.", icoPath.arg("webp"));
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.WEBP/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.PBM/.", "Portable Image Format");
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.PBM/DefaultIcon/.", icoPath.arg("pbm"));
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.PBM/shell/open/command/.", openCommand);
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.SR/.", "Sun Raster");
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.SR/DefaultIcon/.", icoPath.arg("sr"));
		registryHklm.setValue("Classes/SimpleViewer.AssocFile.SR/shell/open/command/.", openCommand);
		//capabilities
		registryHklm.setValue("Simple Viewer/Capabilities/ApplicationName", "Simple Viewer");
		registryHklm.setValue("Simple Viewer/Capabilities/ApplicationDescription", "An image viewer featuring a minimal interface design and high-quality image display.");
		//tif
		registryHklm.setValue("Simple Viewer/Capabilities/FileAssociations/.tif", "SimpleViewer.AssocFile.TIF");
		registryHklm.setValue("Simple Viewer/Capabilities/FileAssociations/.tiff", "SimpleViewer.AssocFile.TIF");
		//bmp, dib
		registryHklm.setValue("Simple Viewer/Capabilities/FileAssociations/.bmp", "SimpleViewer.AssocFile.BMP");
		registryHklm.setValue("Simple Viewer/Capabilities/FileAssociations/.dib", "SimpleViewer.AssocFile.BMP");
		//jpeg, jpg, jpe
		registryHklm.setValue("Simple Viewer/Capabilities/FileAssociations/.jpeg", "SimpleViewer.AssocFile.JPG");
		registryHklm.setValue("Simple Viewer/Capabilities/FileAssociations/.jpg", "SimpleViewer.AssocFile.JPG");
		registryHklm.setValue("Simple Viewer/Capabilities/FileAssociations/.jpe", "SimpleViewer.AssocFile.JPG");
		//jp2
		registryHklm.setValue("Simple Viewer/Capabilities/FileAssociations/.jp2", "SimpleViewer.AssocFile.JP2");
		//png
		registryHklm.setValue("Simple Viewer/Capabilities/FileAssociations/.png", "SimpleViewer.AssocFile.PNG");
		//webp
		registryHklm.setValue("Simple Viewer/Capabilities/FileAssociations/.webp", "SimpleViewer.AssocFile.WEBP");
		//pbm, pgm, ppm
		registryHklm.setValue("Simple Viewer/Capabilities/FileAssociations/.pbm", "SimpleViewer.AssocFile.PBM");
		registryHklm.setValue("Simple Viewer/Capabilities/FileAssociations/.pgm", "SimpleViewer.AssocFile.PBM");
		registryHklm.setValue("Simple Viewer/Capabilities/FileAssociations/.ppm", "SimpleViewer.AssocFile.PBM");
		//sr, ras
		registryHklm.setValue("Simple Viewer/Capabilities/FileAssociations/.sr", "SimpleViewer.AssocFile.SR");
		registryHklm.setValue("Simple Viewer/Capabilities/FileAssociations/.ras", "SimpleViewer.AssocFile.SR");
		//register application
		registryHklm.setValue("RegisteredApplications/Simple Viewer", "SOFTWARE\\Simple Viewer\\Capabilities");
		//uninstallation entry
		registryHklm.setValue("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/DisplayName", "Simple Viewer");
		registryHklm.setValue("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/UninstallString", QString("\"%1\" %2").arg(QDir::toNativeSeparators(QDir(installPath).absoluteFilePath("WinInstaller.exe"))).arg("-uninstall"));
		registryHklm.setValue("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/DisplayIcon", QString("\"%1\"").arg(QDir::toNativeSeparators(QDir(installPath).absoluteFilePath("data/icon.ico"))));
		registryHklm.setValue("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/EstimatedSize", 84787);
		registryHklm.setValue("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/Publisher", "Bastian Weber");
		registryHklm.setValue("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/DisplayVersion", "1.0 Beta 1");
		registryHklm.setValue("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/NoModify", "1");
		registryHklm.setValue("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/NoRepair", "1");
		//set friendly app name
		QSettings registryHkcr("HKEY_CLASSES_ROOT\\Applications", QSettings::NativeFormat);
		registryHkcr.setValue("SimpleViewer.exe/shell/open/FriendlyAppName", "Simple Viewer");
	}

	void InstallerInterface::copyAllFilesInDirectory(QDir const& sourceDir, QDir const& destinationDir) {
		if (sourceDir != destinationDir) {
			QStringList filesInFolder = sourceDir.entryList(QDir::NoDotAndDotDot | QDir::Files);
			for (QString const& entry : filesInFolder) {
				QString oldPath = sourceDir.absoluteFilePath(entry);
				QString newPath = destinationDir.absoluteFilePath(entry);
				if (QFile::exists(newPath)) {
					QFile::remove(newPath);
				}
				QFile::copy(oldPath, newPath);
				QCoreApplication::processEvents();
			}
		}
	}

	void InstallerInterface::installFiles(QDir installPath) {
		QDir currentPath(QCoreApplication::applicationDirPath());
		if (currentPath != installPath) {
			if (!installPath.exists()) installPath.mkpath(installPath.absolutePath());
			installPath.mkpath(QDir::cleanPath(installPath.absolutePath() + QString("/data")));
			installPath.mkpath(QDir::cleanPath(installPath.absolutePath() + QString("/platforms")));
			InstallerInterface::copyAllFilesInDirectory(currentPath, installPath);
			currentPath.cd("data");
			installPath.cd("data");
			InstallerInterface::copyAllFilesInDirectory(currentPath, installPath);
			currentPath.cd("../platforms");
			installPath.cd("../platforms");
			QStringList filesInPlatformsFolder = currentPath.entryList();
			InstallerInterface::copyAllFilesInDirectory(currentPath, installPath);
		}
	}

	bool InstallerInterface::createStartMenuEntry(QString targetPath) {
		targetPath = QDir::toNativeSeparators(targetPath);

		WCHAR startMenuPath[MAX_PATH];
		HRESULT result = SHGetFolderPathW(NULL, CSIDL_COMMON_PROGRAMS, NULL, 0, startMenuPath);

		if (SUCCEEDED(result)) {
			QString linkPath = QDir(QString::fromWCharArray(startMenuPath)).absoluteFilePath("Simple Viewer.lnk");

			CoInitialize(NULL);
			IShellLinkW* shellLink = NULL;
			result = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_ALL, IID_IShellLinkW, (void**)&shellLink);
			if (SUCCEEDED(result)) {
				shellLink->SetPath(reinterpret_cast<LPCWSTR>(targetPath.utf16()));
				shellLink->SetDescription(L"Simple Viewer image viewer");
				shellLink->SetIconLocation(reinterpret_cast<LPCWSTR>(targetPath.utf16()), 0);

				IPersistFile* persistFile;
				result = shellLink->QueryInterface(IID_IPersistFile, (void**)&persistFile);

				if (SUCCEEDED(result)) {
					result = persistFile->Save(reinterpret_cast<LPCOLESTR>(linkPath.utf16()), TRUE);

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
		this->browseButton->setEnabled(false);
		this->startMenuCheckbox->setEnabled(false);
		this->okButton->setEnabled(false);
		this->cancelButton->setEnabled(false);
	}

	//============================================================================ PRIVATE SLOTS =============================================================================\\

	void InstallerInterface::install() {
		this->disableControls();
		this->setWindowTitle(this->windowTitle() + QString("- Installing..."));
		QCoreApplication::processEvents();
		InstallerInterface::installFiles(this->currentlySelectedPath);
		InstallerInterface::registerProgramInRegistry(this->currentlySelectedPath);
		if (this->startMenuCheckbox->isChecked()) this->createStartMenuEntry(this->currentlySelectedPath.absoluteFilePath("SimpleViewer.exe"));
		QMessageBox msgBox;
		msgBox.setWindowTitle(QObject::tr("Installation Successful"));
		msgBox.setText(QObject::tr("The installation was successful. The installer will now quit."));
		msgBox.setIcon(QMessageBox::Information);
		msgBox.setStandardButtons(QMessageBox::Close);
		msgBox.exec();
		QCoreApplication::quit();
	}

	void InstallerInterface::reactToBrowseButtonClick() {
		QString path = QFileDialog::getExistingDirectory(this, tr("Select Installation Directory"), this->currentlySelectedPath.absolutePath());

		if (!path.isEmpty()) {
			this->currentlySelectedPath = QDir(path).absoluteFilePath("Simple Viewer");
			this->pathInput->setText(this->currentlySelectedPath.absolutePath());
		}
	}

}