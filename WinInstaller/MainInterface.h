#ifndef CT_MAININTERFACE
#define CT_MAININTERFACE

#include <iostream>

//Qt
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>

namespace sv {

	class MainInterface : public QMainWindow {
		Q_OBJECT
	public:
		MainInterface(QWidget *parent = 0);
		~MainInterface();
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