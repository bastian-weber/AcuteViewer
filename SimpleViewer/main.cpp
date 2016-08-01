#include "MainInterface.h"

int init(int argc, char* argv[]) {
	QApplication app(argc, argv);



	QIcon icon;
	icon.addFile("./data/icon_16.png");
	icon.addFile("./data/icon_32.png");
	icon.addFile("./data/icon_48.png");
	icon.addFile("./data/icon_64.png");
	icon.addFile("./data/icon_96.png");
	icon.addFile("./data/icon_128.png");
	icon.addFile("./data/icon_192.png");
	icon.addFile("./data/icon_256.png");
	app.setWindowIcon(icon);

	sv::MainInterface* mainInterface;

	if (QCoreApplication::arguments().size() < 2) {
		mainInterface = new sv::MainInterface(QString());
	}else if (QCoreApplication::arguments().size() == 2) {
		mainInterface = new sv::MainInterface(QCoreApplication::arguments().at(1));
	} else if (QCoreApplication::arguments().size() > 2) {
		QStringList files = QCoreApplication::arguments();
		files.pop_front();
		mainInterface = new sv::MainInterface(files);
	}

	mainInterface->show();

	return app.exec();
}

int main(int argc, char* argv[]) {
	return init(argc, argv);
}