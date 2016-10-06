#ifndef SV_ABOUTDIALOG
#define SV_ABOUTDIALOG

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
		void loadSettings();
		QString getInstalledVersion();
		void autoUpdate();
		void checkForUpdates(bool isAutoUpdate = false);
		//variables
		std::shared_ptr<QSettings> settings;
		QNetworkAccessManager* network;
		std::shared_ptr<QNetworkReply> infoDocumentReply;
		QStringList infoReplyParts;
		QString infoString;

		int majorVersion;
		int minorVersion;
		int latestMajorVersion;
		int latestMinorVersion;

		//widgets
		QVBoxLayout* mainLayout;
		QHBoxLayout* buttonLayout;
		QLabel* infoLabel;
		QPushButton* downloadButton;
		QPushButton* closeButton;
		QCheckBox* autoUpdateCheckbox;
	private slots:
		void processInfoFile();
		void downloadUpdate();
		void reactToCheckboxChange();
		void processAutoUpdate();
	};
}
#endif