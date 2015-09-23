#include "MainInterface.h"

namespace sv {

	MainInterface::MainInterface(QWidget *parent)
		: QMainWindow(parent) {

		setAcceptDrops(true);
		this->imageView = new hb::ImageView(this);
		this->imageView->setShowInterfaceOutline(false);
		this->imageView->setUseSmoothTransform(false);
		setCentralWidget(this->imageView);
	}

	MainInterface::~MainInterface() {
		delete this->imageView;
	}

	QSize MainInterface::sizeHint() const {
		return QSize(1053, 570);
	}

	//============================================================================== PROTECTED ==============================================================================\\

	void MainInterface::dragEnterEvent(QDragEnterEvent* e) {
		if (e->mimeData()->hasUrls()) {
			if (!e->mimeData()->urls().isEmpty()) {
				e->acceptProposedAction();
			}
		}
	}

	void MainInterface::dropEvent(QDropEvent* e) {
		if (!e->mimeData()->urls().isEmpty()) {
			QString path = e->mimeData()->urls().first().toLocalFile();
			loadImage(path);
		}
	}

	//=============================================================================== PRIVATE ===============================================================================\\

	void MainInterface::loadImage(QString const& path) {
		this->image = cv::imread(path.toStdString(), CV_LOAD_IMAGE_COLOR);
		cv::cvtColor(this->image, this->image, CV_BGR2RGB);
		this->imageView->setImage(this->image);
	}

}