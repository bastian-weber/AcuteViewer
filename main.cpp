#include "MainInterface.h"

int init(int argc, char* argv[]) {
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

	sv::MainInterface* mainInterface = new sv::MainInterface();
	mainInterface->show();

	return app.exec();
}

int main(int argc, char* argv[]) {
	//std::vector<char> vec;
	//vec.resize(1000);
	//char* a = new char[1000];

	//std::ifstream file;
	//file.exceptions(std::ifstream::badbit | std::ifstream::failbit | std::ifstream::eofbit);
	//file.open("G:/Pictures/Webcam/Snapshot_20110209.jpg", std::ifstream::in);
	//try {
	//	file.read(&vec.front(), 100);
	//} catch (std::ios_base::failure f) {
	//	std::cout << f.what() << " characters extracted: " << file.gcount() << std::endl;
	//} catch (...) {
	//	std::cout << "Some other error" << std::endl;
	//}
	//std::cout << file.gcount() << std::endl;
	//std::cout << "done" << std::endl;
	//file.close();

	//QFile fil("G:/Pictures/Webcam/Snapshot_20111005.JPG");
	//fil.open(QIODevice::ReadOnly);
	//int amount = fil.read(&vec.front(), 500);
	//std::cout << amount << std::endl;

	return init(argc, argv);
}