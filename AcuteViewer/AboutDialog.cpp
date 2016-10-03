#include "AboutDialog.h"

namespace sv {

	AboutDialog::AboutDialog(std::shared_ptr<QSettings> settings, QWidget* parent)
		: settings(settings),
		QDialog(parent) {
		this->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
		this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

		this->setWindowTitle(tr("About Acute Viewer"));

		this->infoLabel = new QLabel("Latest Version: Loading...");

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
		this->infoLabel->setText("Latest Version: Loading...");
		QNetworkRequest request;
		request.setUrl(QUrl("http://true-contrast.de/avv.txt"));
		this->infoDocumentReply = network->get(request);
		connect(infoDocumentReply, SIGNAL(finished()), this, SLOT(processInfoFile()));
	}

	//=============================================================================== PRIVATE ===============================================================================\\



	//============================================================================ PRIVATE SLOTS =============================================================================\\

	void AboutDialog::processInfoFile() {
		this->infoLabel->setText(QString("Latest Version: %1").arg(QString(this->infoDocumentReply->readAll().toStdString().c_str())));
	}

}