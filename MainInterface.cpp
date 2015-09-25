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
			nextImage();
			e->accept();
		} else if (e->key() == Qt::Key_Left || e->key() == Qt::Key_Up) {
			previousImage();
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
				QStringList filters;
				filters << "*.bmp" << "*.dib" << "*.jpeg" << "*.jpg" << "*.jpe" << "*.jpeg" << "*.jp2" << "*.png" << "*.webp" << "*.pbm" << "*.pgm" << "*.ppm" << "*.sr" << "*.ras" << "*.tiff" << "*.tif";
				this->filesInDirectory = directory.entryList(filters, QDir::Files).toVector();
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
		this->setWindowTitle(QString("%1 - %2 - %3 of %4").arg(this->programTitle, QFileInfo(path).fileName()).arg(this->fileIndex + 1).arg(this->filesInDirectory.size()));
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