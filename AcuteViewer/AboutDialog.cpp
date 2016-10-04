#include "AboutDialog.h"

namespace sv {

	AboutDialog::AboutDialog(std::shared_ptr<QSettings> settings, QWidget* parent)
		: settings(settings),
		QDialog(parent) {
		this->setSizeGripEnabled(false);
		this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

		this->setFixedWidth(400);

		this->setWindowTitle(tr("About Acute Viewer"));

		this->majorVersion = AV_MAJOR_VERSION;
		this->minorVersion = AV_MINOR_VERSION;

		this->infoString = tr("<p><b>This Version: %1</b></p><b>%2</b><p><b>%3</p>").arg(QString("%1.%2").arg(this->majorVersion).arg(this->minorVersion));

		this->infoLabel = new QLabel;
		this->infoLabel->setTextFormat(Qt::RichText);
		this->infoLabel->setWordWrap(true);
		this->infoLabel->setText(this->infoString.arg(this->getInstalledVersion(), "Latest Version: Loading..."));

		this->downloadButton = new QPushButton(tr("Download Now"), this);
		this->downloadButton->setVisible(false);
		QObject::connect(this->downloadButton, SIGNAL(clicked()), this, SLOT(downloadUpdate()));
		this->closeButton = new QPushButton(tr("Close"), this);
		QObject::connect(this->closeButton, SIGNAL(clicked()), this, SLOT(close()));

		this->buttonLayout = new QHBoxLayout;
		this->buttonLayout->addStretch(1);
		this->buttonLayout->addWidget(downloadButton);
		this->buttonLayout->addWidget(closeButton);

		this->mainLayout = new QVBoxLayout;
		this->mainLayout->addWidget(infoLabel);
		this->mainLayout->addSpacing(20);
		this->mainLayout->addLayout(this->buttonLayout);

		this->setLayout(this->mainLayout);

		this->network = new QNetworkAccessManager(this);
	}

	//============================================================================== PROTECTED ==============================================================================\\

	void AboutDialog::showEvent(QShowEvent * e) {
		this->downloadButton->setVisible(false);
		this->infoLabel->setText(this->infoString.arg(this->getInstalledVersion(), "Latest Version: Loading..."));
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
			QString response = QString(this->infoDocumentReply->readAll().toStdString().c_str());
			this->infoReplyParts = response.split("\\\\\\\\\\");
			infoDocumentReply->deleteLater();
			if (this->infoReplyParts.size() >= 4) {
				//this->infoLabel->setText(this->infoString.arg(this->getInstalledVersion(), this->infoReplyParts.at(1).arg(this->infoReplyParts.at(0))));
				QStringList version = this->infoReplyParts.at(0).split('.');
				if (version.at(0).toInt() > this->majorVersion || (version.at(1).toInt() > this->minorVersion && version.at(0).toInt() >= this->majorVersion)) {
					this->downloadButton->setVisible(true);
				}
			} else {
				this->infoLabel->setText(this->infoString.arg(this->getInstalledVersion(), tr("<b>Latest Version: (error)</b>")));
			}
		} else {
			this->infoLabel->setText(this->infoString.arg(this->getInstalledVersion(), tr("<b>Latest Version: (network request failed)</b>")));
		}
	}

	void AboutDialog::downloadUpdate() {
#ifdef Q_OS_WIN
		QUrl url(this->infoReplyParts.at(2));
#else
		QUrl url(this->infoReplyParts.at(3));
#endif
		if (QDesktopServices::openUrl(url)) {
			QMessageBox::information(this,
									 QObject::tr("Download Started"),
									 QObject::tr("The download link has been opened in your default browser."),
									 QMessageBox::StandardButton::Close,
									 QMessageBox::StandardButton::Close);
		} else {
			QMessageBox::critical(this,
								  QObject::tr("File Not Moved"),
								  QObject::tr("The download link could not be opened. Please download the new version manually."),
								  QMessageBox::StandardButton::Close,
								  QMessageBox::StandardButton::Close);
		}
	}

}