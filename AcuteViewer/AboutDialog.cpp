#include "AboutDialog.h"

namespace sv {

	AboutDialog::AboutDialog(std::shared_ptr<QSettings> settings, QWidget* parent)
		: settings(settings),
		QDialog(parent) {
		this->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
		this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

		this->setWindowTitle(tr("About Acute Viewer"));

		this->majorVersion = AV_MAJOR_VERSION;
		this->minorVersion = AV_MINOR_VERSION;

		this->infoString = tr("<p>This Version: %1</p>%2<p>Latest Version: %3</p>").arg(QString("%1.%2").arg(this->majorVersion).arg(this->minorVersion));

		this->infoLabel = new QLabel;

		this->downloadButton = new QPushButton(tr("Download Now"), this);
		//QObject::connect(this->downloadButton, SIGNAL(clicked()), this, SLOT(reactToOkButtonClick()));
		this->closeButton = new QPushButton(tr("Close"), this);
		QObject::connect(this->closeButton, SIGNAL(clicked()), this, SLOT(close()));

		this->buttonLayout = new QHBoxLayout;
		this->buttonLayout->addStretch(1);
		this->buttonLayout->addWidget(downloadButton);
		this->buttonLayout->addWidget(closeButton);

		this->mainLayout = new QVBoxLayout;
		this->mainLayout->addWidget(infoLabel);
		this->mainLayout->addLayout(this->buttonLayout);

		this->setLayout(this->mainLayout);

		this->network = new QNetworkAccessManager(this);
	}

	//============================================================================== PROTECTED ==============================================================================\\

	void AboutDialog::showEvent(QShowEvent * e) {
		this->infoLabel->setText(this->infoString.arg(this->getInstalledVersion(), "Loading..."));
		QNetworkRequest request;
		request.setUrl(QUrl("http://true-contrast.de/avv.txt"));
		this->infoDocumentReply = network->get(request);
		connect(infoDocumentReply, SIGNAL(finished()), this, SLOT(processInfoFile()));
	}

	QString AboutDialog::getInstalledVersion() {
#ifdef Q_OS_WIN
		QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE", QSettings::NativeFormat);
		if (registry.contains("Microsoft/Windows/CurrentVersion/Uninstall/AcuteViewer/DisplayVersion")) {
			return tr("<p>Installed Version: %1</p>").arg(registry.value("Microsoft/Windows/CurrentVersion/Uninstall/AcuteViewer/DisplayVersion", "None").toString());
		} else {
			return tr("<p>Installed Version: None</p>");
		}
#else
		return "";
#endif
	}

	//=============================================================================== PRIVATE ===============================================================================\\



	//============================================================================ PRIVATE SLOTS =============================================================================\\

	void AboutDialog::processInfoFile() {
		if (this->infoDocumentReply->error() == QNetworkReply::NoError) {
			this->infoLabel->setText(this->infoString.arg(this->getInstalledVersion(), QString(this->infoDocumentReply->readAll().toStdString().c_str())));
		} else {
			this->infoLabel->setText(this->infoString.arg(this->getInstalledVersion(), tr("(network request failed)")));
		}
	}

}