#ifndef SV_ABOUTDIALOG
#define SV_ABOUTDIALOG

#include <iostream>
#include <memory>

//Qt
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>
#include <QtNetwork/QtNetwork>

namespace sv {

	class AboutDialog : public QDialog {
		Q_OBJECT
	public:
		AboutDialog(std::shared_ptr<QSettings> settings, QWidget* parent = 0);
	protected:
		void showEvent(QShowEvent* e);
	private:
		//functions

		//variables
		std::shared_ptr<QSettings> settings;
		QNetworkAccessManager* network;
		QNetworkReply* infoDocumentReply;

		//widgets
		QVBoxLayout* mainLayout;
		QHBoxLayout* buttonLayout;
		QLabel* infoLabel;
		QPushButton* downloadButton;
		QPushButton* closeButton;
	private slots:
		void processInfoFile();
	};
}
#endif