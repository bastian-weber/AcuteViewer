#include "MainInterface.h"

namespace sv {

	MainInterface::MainInterface(QWidget *parent)
		: QMainWindow(parent) {

		setAcceptDrops(true);
		this->imageView = new hb::ImageView(this);
		this->imageView->setShowInterfaceOutline(false);
		this->imageView->setUseSmoothTransform(false);
		this->imageView->setRotation(270);
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

	void MainInterface::keyPressEvent(QKeyEvent* e) {
		if (e->key() == Qt::Key_Right || e->key() == Qt::Key_Down) {
			nextImage();
			e->accept();
		} else if (e->key() == Qt::Key_Left || e->key() == Qt::Key_Up) {
			previousImage();
			e->accept();
		} else if (e->key() == Qt::Key_Escape) {
			if (this->isFullScreen()) {
				this->exitFullscreen();
			}
		} else {
			e->ignore();
		}
	}

	void MainInterface::mouseDoubleClickEvent(QMouseEvent* e) {
		if (this->isFullScreen()) {
			this->exitFullscreen();
		} else {
			this->enterFullscreen();
		}
		e->accept();
	}

	//=============================================================================== PRIVATE ===============================================================================\\

	void MainInterface::loadImage(QString path) {
		if (path == QString()) {
			path = this->currentDirectory.absoluteFilePath(this->filesInDirectory[this->fileIndex]);
		} else {
			//find the path in the current directory listing
			QFileInfo fileInfo = QFileInfo(QDir::cleanPath(path));
			QDir directory = fileInfo.absoluteDir();
			QString filename = fileInfo.fileName();
			if (directory != this->currentDirectory) {
				this->currentDirectory = directory;
				this->filesInDirectory = directory.entryList(QDir::Files).toVector();
			}
			if (this->filesInDirectory.size() == 0 || this->fileIndex <= 0 || this->fileIndex >= this->filesInDirectory.size() || this->filesInDirectory.at(this->fileIndex) != filename) {
				this->fileIndex = this->filesInDirectory.indexOf(filename);
			}
		}
		this->image = cv::imread(path.toStdString(), CV_LOAD_IMAGE_COLOR);
		if (this->image.data) {
			cv::cvtColor(this->image, this->image, CV_BGR2RGB);
			this->imageView->setImage(this->image);
		} else {
			QMessageBox msgBox;
			msgBox.setWindowTitle(tr("Error"));
			msgBox.setText(tr("Could not read file."));
			msgBox.exec();
		}

	}

	void MainInterface::nextImage() {
		if (this->filesInDirectory.size() != 0) {
			if (this->fileIndex >= 0) {
				if (this->fileIndex != this->filesInDirectory.size() - 1) {
					++this->fileIndex;
				} else {
					this->fileIndex = 0;
				}
			} else {
				this->fileIndex = 0;
			}
			loadImage();
		}
	}

	void MainInterface::previousImage() {
		if (this->filesInDirectory.size() != 0) {
			if (this->fileIndex >= 0) {
				if (this->fileIndex != 0) {
					--this->fileIndex;
				} else {
					this->fileIndex = this->filesInDirectory.size() - 1;
				}
			} else {
				this->fileIndex = 0;
			}
			loadImage();
		}
	}

	void MainInterface::enterFullscreen() {
		this->imageView->setInterfaceBackgroundColor(Qt::black);
		this->showFullScreen();
	}

	void MainInterface::exitFullscreen() {
		QPalette palette = qApp->palette();
		this->imageView->setInterfaceBackgroundColor(palette.base().color());
		this->showNormal();
	}

}