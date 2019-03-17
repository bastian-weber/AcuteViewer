#include "AboutDialog.h"

namespace sv {

	AboutDialog::AboutDialog(std::shared_ptr<QSettings> settings, QWidget* parent)
		: settings(settings),
		QDialog(parent) {
		setSizeGripEnabled(false);
		setWindowFlags((windowFlags() & ~Qt::WindowContextHelpButtonHint) | Qt::MSWindowsFixedSizeDialogHint);

		setFixedWidth(400);

		setWindowTitle(tr("About Acute Viewer"));

		majorVersion = AV_MAJOR_VERSION;
		minorVersion = AV_MINOR_VERSION;

		infoString = tr("<h2>Acute Viewer</h2><p><b>This Version: %1</b></p><b>%2</b><p><b>%3</p>").arg(QString("%1.%2").arg(majorVersion).arg(minorVersion));

		infoLabel = new QLabel;
		infoLabel->setTextFormat(Qt::RichText);
		infoLabel->setWordWrap(true);
		infoLabel->setText(infoString.arg(getInstalledVersion(), "Latest Version: Loading..."));

		autoUpdateCheckbox = new QCheckBox(tr("Notify me when there is a new version available"), this);
		QObject::connect(autoUpdateCheckbox, SIGNAL(clicked()), this, SLOT(reactToCheckboxChange()));

		downloadButton = new QPushButton(tr("Download Now"), this);
		downloadButton->setVisible(false);
		QObject::connect(downloadButton, SIGNAL(clicked()), this, SLOT(downloadUpdate()));
		closeButton = new QPushButton(tr("Close"), this);
		QObject::connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

		buttonLayout = new QHBoxLayout;
		buttonLayout->addStretch(1);
		buttonLayout->addWidget(downloadButton);
		buttonLayout->addWidget(closeButton);

		mainLayout = new QVBoxLayout;
		mainLayout->addWidget(infoLabel);
		mainLayout->addSpacing(20);
		mainLayout->addWidget(autoUpdateCheckbox);
		mainLayout->addSpacing(20);
		mainLayout->addLayout(buttonLayout);

		setLayout(mainLayout);

		network = new QNetworkAccessManager(this);

		loadSettings();
		autoUpdate();
	}

	//============================================================================== PROTECTED ==============================================================================\\

	void AboutDialog::showEvent(QShowEvent * e) {
		downloadButton->setVisible(false);
		infoLabel->setText(infoString.arg(getInstalledVersion(), "Latest Version: Loading..."));
		checkForUpdates();
	}

	void AboutDialog::loadSettings() {
		autoUpdateCheckbox->setChecked(settings->value("updateNotifications", false).toBool());
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

	void AboutDialog::autoUpdate() {
		if (autoUpdateCheckbox->isChecked()) {
			checkForUpdates(true);
		}
	}

	void AboutDialog::checkForUpdates(bool isAutoUpdate) {
		QNetworkRequest request;
		request.setUrl(QUrl("http://true-contrast.de/avv"));
		infoDocumentReply = std::shared_ptr<QNetworkReply>(network->get(request), [](QNetworkReply* reply) { reply->deleteLater(); });
		connect(infoDocumentReply.get(), SIGNAL(finished()), this, SLOT(processInfoFile()));
		QDateTime local(QDateTime::currentDateTime());
		QDateTime Utc(local.toUTC());
		//auto update is only conducted every 6 hours
		if (isAutoUpdate && (Utc.toTime_t() - settings->value("lastUpdateCheck", 0).toUInt() > 21600)) {
			connect(infoDocumentReply.get(), SIGNAL(finished()), this, SLOT(processAutoUpdate()));
		}
	}

	//=============================================================================== PRIVATE ===============================================================================\\



	//============================================================================ PRIVATE SLOTS =============================================================================\\

	void AboutDialog::processInfoFile() {
		if (infoDocumentReply->error() == QNetworkReply::NoError) {
			QString response = QString(infoDocumentReply->readAll().toStdString().c_str());
			infoReplyParts = response.split("\\\\\\\\\\");
			if (infoReplyParts.size() >= 4) {
				infoLabel->setText(infoString.arg(getInstalledVersion(), infoReplyParts.at(1).arg(infoReplyParts.at(0))));
				QStringList version = infoReplyParts.at(0).split('.');
				latestMajorVersion = version.at(0).toInt();
				latestMinorVersion = version.at(1).toInt();
				if (latestMajorVersion > majorVersion || (latestMinorVersion > minorVersion && latestMajorVersion >= majorVersion)) {
					downloadButton->setVisible(true);
				}
			} else {
				infoLabel->setText(infoString.arg(getInstalledVersion(), tr("<b>Latest Version: (error)</b>")));
			}
		} else {
			infoLabel->setText(infoString.arg(getInstalledVersion(), tr("<b>Latest Version: (network request failed)</b>")));
		}
	}

	void AboutDialog::downloadUpdate() {
#ifdef Q_OS_WIN
		QUrl url(infoReplyParts.at(2));
#else
		QUrl url(infoReplyParts.at(3));
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

	void AboutDialog::reactToCheckboxChange() {
		settings->setValue("updateNotifications", autoUpdateCheckbox->isChecked());
		settings->setValue("noRemindMinorVersion", 0);
		settings->setValue("noRemindMajorVersion", 0);
	}

	void AboutDialog::processAutoUpdate() {
		QDateTime local(QDateTime::currentDateTime());
		QDateTime Utc(local.toUTC());
		settings->setValue("lastUpdateCheck", Utc.toTime_t());
		if (latestMajorVersion > majorVersion || (latestMinorVersion > minorVersion && latestMajorVersion >= majorVersion)) {
			int noRemindMajorVersion = settings->value("noRemindMajorVersion", 0).toInt();
			int noRemindMinorVersion = settings->value("noRemindMinorVersion", 0).toInt();
			if (latestMajorVersion > noRemindMajorVersion || (latestMinorVersion > noRemindMinorVersion && latestMajorVersion >= noRemindMajorVersion)) {
				QWidget* parent;
				if (!(parent = dynamic_cast<QWidget*>(this->parent()))) {
					parent = this;
				}
				QMessageBox msgBox(parent);
				msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Ok);
				msgBox.setDefaultButton(QMessageBox::Ok);
				msgBox.setWindowTitle(tr("New Version"));
				msgBox.setText(tr("There is a newer version of Acute Viewer (%1.%2) available.").arg(latestMajorVersion).arg(latestMinorVersion));
				msgBox.setButtonText(QMessageBox::Yes, QObject::tr("Don't remind me about this version anymore"));
				msgBox.setIcon(QMessageBox::Information);
				if (msgBox.exec() == QMessageBox::Yes) {
					settings->setValue("noRemindMinorVersion", latestMinorVersion);
					settings->setValue("noRemindMajorVersion", latestMajorVersion);
				} else {
					show();
				}
			}
		}
	}

}