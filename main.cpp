#include "MainInterface.h"

int init(int argc, char* argv[], QString openWithFilename) {
	QApplication app(argc, argv);

	//QIcon icon;
	//icon.addFile("./data/icon_16.png");
	//icon.addFile("./data/icon_32.png");
	//icon.addFile("./data/icon_48.png");
	//icon.addFile("./data/icon_64.png");
	//icon.addFile("./data/icon_96.png");
	//icon.addFile("./data/icon_128.png");
	//icon.addFile("./data/icon_192.png");
	//icon.addFile("./data/icon_256.png");
	//app.setWindowIcon(icon);

	sv::MainInterface* mainInterface = new sv::MainInterface(openWithFilename);
	mainInterface->show();

	return app.exec();
}

//int main(int argc, char* argv[]) {
int wmain(int argc, wchar_t* argv[]) {
	QString openWithFilename;
	if (argc > 1) {
		openWithFilename = QString::fromWCharArray(argv[1], wcslen(argv[1]));
	}
	return init(0, NULL, openWithFilename);
}