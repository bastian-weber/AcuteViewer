#ifndef CT_InstallerInterface
#define CT_InstallerInterface

#include <iostream>

//Qt
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>

//for creating windows shortcuts
#include <shlobj.h>

namespace wi {

	class InstallerInterface : public QMainWindow {
		Q_OBJECT
	public:
		InstallerInterface(QWidget *parent = 0);
		~InstallerInterface();
		QSize sizeHint() const;
	private:
		//functions
		static void registerProgramInRegistry(QDir installPath);
		static void copyAllFilesInDirectory(QDir const& sourceDir, QDir const& destinationDir);
		static void installFiles(QDir installPath);
		static bool createStartMenuEntry(QString targetPath);
		void disableControls();

		//variables
		QDir currentlySelectedPath;

		//widgets
		QWidget* mainWidget;
		QVBoxLayout* mainLayout;
		QHBoxLayout* buttonLayout;
		QLabel* descriptionLabel;
		QLineEdit* pathInput;
		QPushButton* browseButton;
		QCheckBox* startMenuCheckbox;
		QPushButton* okButton;
		QPushButton* cancelButton;
	private slots:
		void install();
		void reactToBrowseButtonClick();
	signals:
	};
}
#endif