#include "MainInterface.h"

int init(int argc, char* argv[]) {
	QApplication app(argc, argv);

	QString openWithFilename;
	if (QCoreApplication::arguments().size() > 1) {
		openWithFilename = QCoreApplication::arguments().at(1);
	}

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

	sv::MainInterface* mainInterface = new sv::MainInterface(openWithFilename);

	return app.exec();
}

int main(int argc, char* argv[]) {
	return init(argc, argv);
}