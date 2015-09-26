#include "MainInterface.h"

namespace sv {

	MainInterface::MainInterface(QWidget *parent)
		: QMainWindow(parent) {

		setAcceptDrops(true);
		this->setWindowTitle(this->programTitle);

		this->imageView = new hb::ImageView(this);
		this->imageView->setShowInterfaceOutline(false);
		this->imageView->setUseSmoothTransform(false);
		this->imageView->installEventFilter(this);
		this->imageView->setExternalPostPaintFunction(this, &MainInterface::infoPaintFunction);
		this->imageView->setInterfaceBackgroundColor(Qt::black);
		setCentralWidget(this->imageView);

		this->fileMenu = this->menuBar()->addMenu(tr("&File"));
		this->menuBar()->setVisible(false);

		this->quitAction = new QAction(tr("&Quit"), this);
		this->quitAction->setShortcut(Qt::CTRL + Qt::Key_Q);
		this->quitAction->setShortcutContext(Qt::ApplicationShortcut);
		this->fileMenu->addAction(this->quitAction);
		//so shortcuts also work when menu bar is not visible
		this->addAction(this->quitAction);
		QObject::connect(this->quitAction, SIGNAL(triggered()), this, SLOT(quit()));

		//mouse hide timer in fullscreen
		this->mouseHideTimer = new QTimer(this);
		QObject::connect(this->mouseHideTimer, SIGNAL(timeout()), this, SLOT(hideMouse()));
	}

	MainInterface::~MainInterface() {
		delete this->imageView;
		delete this->fileMenu;
		delete this->quitAction;
		delete this->mouseHideTimer;
	}

	QSize MainInterface::sizeHint() const {
		return QSize(1053, 570);
	}

	//============================================================================== PROTECTED ==============================================================================\\

	bool MainInterface::eventFilter(QObject* object, QEvent* e) {
		if (e->type() == QEvent::MouseButtonRelease) {
			QMouseEvent* keyEvent = (QMouseEvent*)e;
			this->mouseReleaseEvent(keyEvent);
		} else if (e->type() == QEvent::Wheel) {
			if (this->menuBar()->isVisible()) {
				this->menuBar()->setVisible(false);
			}
		} else if (e->type() == QEvent::MouseMove) {
			QMouseEvent* keyEvent = (QMouseEvent*)e;
			this->mouseMoveEvent(keyEvent);
		}
		return false;
	}

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
			loadNextImage();
			e->accept();
		} else if (e->key() == Qt::Key_Left || e->key() == Qt::Key_Up) {
			loadPreviousImage();
			e->accept();
		} else if (e->key() == Qt::Key_Escape) {
			if (this->menuBar()->isVisible()) {
				this->menuBar()->setVisible(false);
			}else if (this->isFullScreen()) {
				this->exitFullscreen();
			}
		} else {
			e->ignore();
		}
	}

	void MainInterface::keyReleaseEvent(QKeyEvent* e) {
		if (!this->menuBar()->isVisible() && e->key() == Qt::Key_Alt) {
			this->menuBar()->setVisible(true);
		} else {
			this->menuBar()->setVisible(false);
		}
		e->ignore();
	}

	void MainInterface::mouseDoubleClickEvent(QMouseEvent* e) {
		if (e->button() == Qt::LeftButton) {
			if (this->isFullScreen()) {
				this->exitFullscreen();
			} else {
				this->enterFullscreen();
			}
			e->accept();
		}
	}

	void MainInterface::mouseReleaseEvent(QMouseEvent* e) {
		if (this->menuBar()->isVisible()) {
			this->menuBar()->setVisible(false);
		}
	}

	void MainInterface::mouseMoveEvent(QMouseEvent* e) {
		if (this->isFullScreen()) {
			this->showMouse();
			this->mouseHideTimer->start(1000);
		}
		e->ignore();
	}

	//=============================================================================== PRIVATE ===============================================================================\\

	cv::Mat MainInterface::readImage(QString path) const {
		cv::Mat image = cv::imread(path.toStdString(), CV_LOAD_IMAGE_COLOR);
		if(image.data) cv::cvtColor(image, image, CV_BGR2RGB);
		return image;
	}

	QString MainInterface::getFullImagePath(size_t index) const {
		return this->currentDirectory.absoluteFilePath(this->filesInDirectory[index]);
	}

	void MainInterface::loadImage(QString path) {
		//find the path in the current directory listing
		QFileInfo fileInfo = QFileInfo(QDir::cleanPath(path));
		QDir directory = fileInfo.absoluteDir();
		QString filename = fileInfo.fileName();
		if (directory != this->currentDirectory) {
			this->currentDirectory = directory;
			QStringList filters;
			filters << "*.bmp" << "*.dib" << "*.jpeg" << "*.jpg" << "*.jpe" << "*.jpeg" << "*.jp2" << "*.png" << "*.webp" << "*.pbm" << "*.pgm" << "*.ppm" << "*.sr" << "*.ras" << "*.tiff" << "*.tif";
			this->filesInDirectory = directory.entryList(filters, QDir::Files).toVector();
		}
		if (this->filesInDirectory.size() == 0 || this->fileIndex <= 0 || this->fileIndex >= this->filesInDirectory.size() || this->filesInDirectory.at(this->fileIndex) != filename) {
			this->fileIndex = this->filesInDirectory.indexOf(filename);
		}
		this->image = readImage(path);
		this->nameOfCurrentFile = filename;
		this->displayImageIfOk();
		this->previousImageCached = false;
		this->nextImageCached = false;
		if (this->filesInDirectory.size() != 0) {
			//preload next and previous image in background
			this->previousImageThread = std::async(std::launch::async, &MainInterface::readImage, this, this->getFullImagePath((this->fileIndex - 1) % this->filesInDirectory.size()));
			this->nextImageThread = std::async(std::launch::async, &MainInterface::readImage, this, this->getFullImagePath((this->fileIndex + 1) % this->filesInDirectory.size()));
		}
	}

	void MainInterface::displayImageIfOk() {
		if (this->image.data) {
			this->currentImageUnreadable = false;
			this->imageView->setImage(this->image);
		} else {
			this->currentImageUnreadable = true;
			this->imageView->resetImage();
		}
		this->setWindowTitle(QString("%1 - %2 - %3 of %4").arg(this->programTitle, this->nameOfCurrentFile).arg(this->fileIndex + 1).arg(this->filesInDirectory.size()));
	}

	void MainInterface::loadNextImage() {
		if (this->filesInDirectory.size() != 0) {
			this->fileIndex = (this->fileIndex + 1) % this->filesInDirectory.size();
			if (this->image.data) {
				this->previousImage = this->image;
				this->previousImageCached = true;
			} else {
				this->previousImageThread = std::async(std::launch::async, &MainInterface::readImage, this, this->getFullImagePath((this->fileIndex - 1) % this->filesInDirectory.size()));
				this->previousImageCached = false;
			}
			if (this->nextImageCached) {
				this->image = this->nextImage;
			} else {
				if (this->nextImageThread.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready) {
					this->setWindowTitle(this->windowTitle() + QString(tr(" - Loading...")));
				}
				this->image = this->nextImageThread.get();
			}
			this->nextImageThread = std::async(std::launch::async, &MainInterface::readImage, this, this->getFullImagePath((this->fileIndex + 1) % this->filesInDirectory.size()));
			this->nextImageCached = false;
			this->nameOfCurrentFile = this->filesInDirectory[this->fileIndex];
			this->displayImageIfOk();
		}
	}

	void MainInterface::loadPreviousImage() {
		if (this->filesInDirectory.size() != 0) {
			this->fileIndex = (this->fileIndex - 1) % this->filesInDirectory.size();
			if (this->image.data) {
				this->nextImage = this->image;
				this->nextImageCached = true;
			} else {
				this->nextImageThread = std::async(std::launch::async, &MainInterface::readImage, this, this->getFullImagePath((this->fileIndex + 1) % this->filesInDirectory.size()));
				this->nextImageCached = false;
			}
			if (this->previousImageCached) {
				this->image = this->previousImage;
			} else {
				if (this->previousImageThread.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready) {
					this->setWindowTitle(this->windowTitle() + QString(tr(" - Loading...")));
				}
				this->image = this->previousImageThread.get();
			}
			this->previousImageThread = std::async(std::launch::async, &MainInterface::readImage, this, this->getFullImagePath((this->fileIndex - 1) % this->filesInDirectory.size()));
			this->previousImageCached = false;
			this->nameOfCurrentFile = this->filesInDirectory[this->fileIndex];
			this->displayImageIfOk();
		}
	}

	void MainInterface::enterFullscreen() {
		//this->imageView->setInterfaceBackgroundColor(Qt::black);
		this->showFullScreen();
		this->mouseHideTimer->start(1000);
	}

	void MainInterface::exitFullscreen() {
		//QPalette palette = qApp->palette();
		//this->imageView->setInterfaceBackgroundColor(palette.base().color());
		this->showNormal();
		this->mouseHideTimer->stop();
		this->showMouse();
	}

	void MainInterface::infoPaintFunction(QPainter& canvas) {
		canvas.setRenderHint(QPainter::Antialiasing, true);
		QPen textPen(Qt::black);
		canvas.setPen(textPen);
		canvas.setBrush(Qt::NoBrush);
		QFont font;
		font.setPointSize(15);
		canvas.setFont(font);
		QColor base = Qt::white;
		base.setAlpha(200);
		canvas.setBackground(base);
		canvas.setBackgroundMode(Qt::OpaqueMode);
		QFontMetrics metrics(font);
		if (this->currentImageUnreadable) {
			QString message = tr("This file could not be read:");
			double lineSpacing = 30;
			canvas.drawText(QPoint((canvas.device()->width() - metrics.width(message)) / 2.0, canvas.device()->height() / 2.0 - 0.5*lineSpacing), message);
			canvas.drawText(QPoint((canvas.device()->width() - metrics.width(this->nameOfCurrentFile)) / 2.0, canvas.device()->height() / 2.0 + 0.5*lineSpacing + metrics.height()), this->nameOfCurrentFile);
		}
	}

	//============================================================================ PRIVATE SLOTS =============================================================================\\

	void MainInterface::quit() {
		this->close();
	}

	void MainInterface::hideMouse() const{
		qApp->setOverrideCursor(Qt::BlankCursor);
		mouseHideTimer->stop();
	}

	void MainInterface::showMouse() const {
		qApp->setOverrideCursor(Qt::ArrowCursor);
	}

}