#ifndef CT_InstallerInterface
#define CT_InstallerInterface

#include <iostream>

//Qt
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>

namespace wi {

	class InstallerInterface : public QMainWindow {
		Q_OBJECT
	public:
		InstallerInterface(QWidget *parent = 0);
		~InstallerInterface();
		QSize sizeHint() const;
	private:
		//functions
		static void install(QString installPath);
		static void registerProgramInRegistry(QString installPath);
		static void copyAllFilesInDirectory(QDir const& sourceDir, QDir const& destinationDir);
		static void installFiles(QDir installPath);

		//variables

		//widgets
		//menus
		//actions
		//timer
	signals:
	};
}
#endif