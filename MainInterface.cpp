#include "MainInterface.h"

namespace sv {

	MainInterface::MainInterface(QString openWithFilename, QWidget *parent)
		: QMainWindow(parent),
		settings(QFileInfo(QCoreApplication::applicationFilePath()).absoluteDir().path() + "/SimpleViewer.ini", QSettings::IniFormat) {

		setAcceptDrops(true);
		qRegisterMetaType<cv::Mat>("cv::Mat");
		QObject::connect(this, SIGNAL(readImageFinished(cv::Mat)), this, SLOT(reactToReadImageCompletion(cv::Mat)));
		this->setWindowTitle(this->programTitle);

		this->imageView = new hb::ImageView(this);
		this->imageView->setShowInterfaceOutline(false);
		this->imageView->setUseSmoothTransform(false);
		this->imageView->installEventFilter(this);
		this->imageView->setExternalPostPaintFunction(this, &MainInterface::infoPaintFunction);
		this->imageView->setInterfaceBackgroundColor(Qt::black);
		this->imageView->setPreventMagnificationInDefaultZoom(true);
		setCentralWidget(this->imageView);

		QObject::connect(this->menuBar(), SIGNAL(triggered(QAction*)), this, SLOT(hideMenuBar(QAction*)));
		this->fileMenu = this->menuBar()->addMenu(tr("&File"));
		this->viewMenu = this->menuBar()->addMenu(tr("&View"));

		this->openAction = new QAction(tr("&Open File"), this);
		this->openAction->setShortcut(QKeySequence::Open);
		this->openAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->openAction, SIGNAL(triggered()), this, SLOT(openDialog()));
		this->fileMenu->addAction(this->openAction);
		this->addAction(this->openAction);

		this->fileMenu->addSeparator();

		this->quitAction = new QAction(tr("&Quit"), this);
		this->quitAction->setShortcut(Qt::CTRL + Qt::Key_Q);
		this->quitAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->quitAction, SIGNAL(triggered()), this, SLOT(quit()));
		this->fileMenu->addAction(this->quitAction);
		//so shortcuts also work when menu bar is not visible
		this->addAction(this->quitAction);

		this->showInfoAction = new QAction(tr("Show Image &Info"), this);
		this->showInfoAction->setCheckable(true);
		this->showInfoAction->setChecked(false);
		this->showInfoAction->setShortcut(Qt::Key_I);
		this->showInfoAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->showInfoAction, SIGNAL(triggered(bool)), this, SLOT(reactToshowInfoToggle(bool)));
		this->viewMenu->addAction(this->showInfoAction);
		this->addAction(this->showInfoAction);

		this->enlargementAction = new QAction(tr("&Enlarge Smaller Images"), this);
		this->enlargementAction->setCheckable(true);
		this->enlargementAction->setChecked(false);
		this->enlargementAction->setShortcut(Qt::Key_E);
		this->enlargementAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->enlargementAction, SIGNAL(triggered(bool)), this, SLOT(reactToEnlargementToggle(bool)));
		this->viewMenu->addAction(this->enlargementAction);
		this->addAction(this->enlargementAction);

		this->smoothingAction = new QAction(tr("Use &Smooth Interpolation when Enlarging"), this);
		this->smoothingAction->setCheckable(true);
		this->smoothingAction->setChecked(false);
		this->smoothingAction->setShortcut(Qt::Key_S);
		this->smoothingAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->smoothingAction, SIGNAL(triggered(bool)), this, SLOT(reactToSmoothingToggle(bool)));
		this->viewMenu->addAction(this->smoothingAction);
		this->addAction(this->smoothingAction);

		this->viewMenu->addSeparator();

		this->menuBarAutoHideAction = new QAction(tr("&Always Show Menu Bar"), this);
		this->menuBarAutoHideAction->setCheckable(true);
		this->menuBarAutoHideAction->setChecked(false);
		this->menuBarAutoHideAction->setShortcut(Qt::Key_M);
		this->menuBarAutoHideAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->menuBarAutoHideAction, SIGNAL(triggered(bool)), this, SLOT(reactoToAutoHideMenuBarToggle(bool)));
		this->viewMenu->addAction(this->menuBarAutoHideAction);
		this->addAction(this->menuBarAutoHideAction);

		//mouse hide timer in fullscreen
		this->mouseHideTimer = new QTimer(this);
		QObject::connect(this->mouseHideTimer, SIGNAL(timeout()), this, SLOT(hideMouse()));

		//timer for thread cleanup
		this->threadCleanUpTimer = new QTimer(this);
		this->threadCleanUpTimer->setSingleShot(true);
		QObject::connect(this->mouseHideTimer, SIGNAL(timeout()), this, SLOT(cleanUpThreads()));

		//load settings
		this->showInfoAction->setChecked(this->settings.value("showImageInfo", false).toBool());
		this->enlargementAction->setChecked(this->settings.value("enlargeSmallImages", false).toBool());
		this->smoothingAction->setChecked(this->settings.value("useSmoothEnlargmentInterpolation", false).toBool());
		this->menuBarAutoHideAction->setChecked(!this->settings.value("autoHideMenuBar", true).toBool());
		
		this->menuBar()->setVisible(this->menuBarAutoHideAction->isChecked());

		if (openWithFilename != QString()) {
			this->loadImage(openWithFilename);
		}
	}

	MainInterface::~MainInterface() {
		delete this->imageView;
		delete this->fileMenu;
		delete this->viewMenu;
		delete this->quitAction;
		delete this->openAction;
		delete this->showInfoAction;
		delete this->smoothingAction;
		delete this->enlargementAction;
		delete this->menuBarAutoHideAction;
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
				this->hideMenuBar();
			}
		} else if (e->type() == QEvent::MouseMove) {
			QMouseEvent* keyEvent = (QMouseEvent*)e;
			this->mouseMoveEvent(keyEvent);
		}
		return false;
	}

	void MainInterface::dragEnterEvent(QDragEnterEvent* e) {
		if (e->mimeData()->hasUrls()) {
			if (!e->mimeData()->urls().isEmpty() && QFileInfo(e->mimeData()->urls().first().toLocalFile()).isFile()) {
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
				this->hideMenuBar();
			} else if (this->isFullScreen()) {
				this->exitFullscreen();
			}
		} else {
			e->ignore();
		}
	}

	void MainInterface::keyReleaseEvent(QKeyEvent* e) {
		if (!this->menuBar()->isVisible() && e->key() == Qt::Key_Alt) {
			this->showMenuBar();
		} else {
			this->hideMenuBar();
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
			this->hideMenuBar();
		}
	}

	void MainInterface::mouseMoveEvent(QMouseEvent* e) {
		if (this->isFullScreen()) {
			this->showMouse();
			if (!this->menuBar()->isVisible()) this->mouseHideTimer->start(this->mouseHideDelay);
		}
		e->ignore();
	}

	//=============================================================================== PRIVATE ===============================================================================\\

	cv::Mat MainInterface::readImage(QString path, bool emitSignals) {
	#ifdef Q_OS_WIN
		cv::Mat image;
		if (!isASCII(path)) {
			//QFile file(path);
			//std::vector<char> buffer;
			//buffer.resize(file.size());
			//if (!file.open(QIODevice::ReadOnly)) {
			//	this->loading = false;
			//	if (emitSignals) emit(readImageFinished(cv::Mat()));
			//	return cv::Mat();
			//}
			//file.read(buffer.data(), file.size());
			//file.close();

			std::ifstream file(path.toStdWString(), std::iostream::binary);
			if (!file.good()) {
				if (emitSignals) emit(readImageFinished(cv::Mat()));
				return cv::Mat();
			}
			file.exceptions(std::ifstream::badbit | std::ifstream::failbit | std::ifstream::eofbit);
			file.seekg(0, std::ios::end);
			std::streampos length(file.tellg());
			std::vector<char> buffer(static_cast<std::size_t>(length));
			if (static_cast<std::size_t>(length) == 0) {
				if (emitSignals) emit(readImageFinished(cv::Mat()));
				return cv::Mat();
			}
			file.seekg(0, std::ios::beg);
			try {
				file.read(buffer.data(), static_cast<std::size_t>(length));
			} catch (...) {
				if (emitSignals) emit(readImageFinished(cv::Mat()));
				return cv::Mat();
			}
			file.close();

			image = cv::imdecode(buffer, CV_LOAD_IMAGE_COLOR);
		} else {
			image = cv::imread(path.toStdString(), CV_LOAD_IMAGE_COLOR);
		}
	#else
		cv::Mat image = cv::imread(path.toStdString(), CV_LOAD_IMAGE_COLOR);
	#endif
		if (image.data) cv::cvtColor(image, image, CV_BGR2RGB);
		if (emitSignals) emit(readImageFinished(image));
		return image;
		}

	bool MainInterface::isASCII(QString const& string) {
		bool isASCII = true;
		for (QString::ConstIterator i = string.begin(); i != string.end(); ++i) {
			isASCII = isASCII && (i->unicode() < 128);
		}
		return isASCII;
	}

	void MainInterface::clearThreads() {
		for (std::map<QString, std::shared_future<cv::Mat>>::iterator it = this->threads.begin(); it != this->threads.end(); ++it) {
			this->waitForThreadToFinish(it->second);
		}
		this->threads.clear();
	}

	void MainInterface::waitForThreadToFinish(std::shared_future<cv::Mat> const& thread) {
		if (!thread.valid()) return;
		if (thread.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
			this->setWindowTitle(this->windowTitle() + QString(tr(" - Loading...")));
		}

		while (thread.wait_for(std::chrono::milliseconds(this->eventProcessIntervalDuringWait)) != std::future_status::ready) {
			qApp->processEvents();
		}
	}

	size_t MainInterface::nextFileIndex() const {
		return (this->fileIndex + 1) % this->filesInDirectory.size();
	}

	size_t MainInterface::previousFileIndex() const {
		return (this->fileIndex - 1) % this->filesInDirectory.size();
	}

	QString MainInterface::getFullImagePath(size_t index) const {
		return this->currentDirectory.absoluteFilePath(this->filesInDirectory[index]);
	}

	void MainInterface::loadImage(QString path) {
		if (this->loading) return;
		this->loading = true;
		//find the path in the current directory listing
		QFileInfo fileInfo = QFileInfo(QDir::cleanPath(path));
		QDir directory = fileInfo.absoluteDir();
		QString filename = fileInfo.fileName();
		if (directory != this->currentDirectory) {
			this->currentDirectory = directory;
			QStringList filters;
			filters << "*.bmp" << "*.dib" << "*.jpeg" << "*.jpg" << "*.jpe" << "*.jpeg" << "*.jp2" << "*.png" << "*.webp" << "*.pbm" << "*.pgm" << "*.ppm" << "*.sr" << "*.ras" << "*.tiff" << "*.tif";
			QStringList contents = directory.entryList(filters, QDir::Files);
			QCollator collator;
			collator.setNumericMode(true);
			std::sort(contents.begin(), contents.end(), collator);
			this->filesInDirectory = contents.toVector();
		}
		if (this->filesInDirectory.size() == 0 || this->fileIndex <= 0 || this->fileIndex >= this->filesInDirectory.size() || this->filesInDirectory.at(this->fileIndex) != filename) {
			this->fileIndex = this->filesInDirectory.indexOf(filename);
		}
		this->currentFileInfo = fileInfo;
		this->clearThreads();
		this->threads[filename] = std::async(std::launch::async, &MainInterface::readImage, this, path, true);
		this->setWindowTitle(this->windowTitle() + QString(tr(" - Loading...")));
	}

	void MainInterface::displayImageIfOk() {
		if (this->image.data) {
			this->currentImageUnreadable = false;
			this->imageView->setImage(this->image);
		} else {
			this->currentImageUnreadable = true;
			this->imageView->resetImage();
		}
		this->setWindowTitle(QString("%1 - %2 - %3 of %4").arg(this->programTitle, this->currentFileInfo.fileName()).arg(this->fileIndex + 1).arg(this->filesInDirectory.size()));
	}

	void MainInterface::loadNextImage() {
		if (this->loading) return;
		this->loading = true;

		if (this->filesInDirectory.size() != 0) {
			this->fileIndex = this->nextFileIndex();
			if (this->threads.find(this->filesInDirectory[this->fileIndex]) == this->threads.end()) {
				this->loading = false;
				return;
			}
			this->waitForThreadToFinish(this->threads[this->filesInDirectory[this->fileIndex]]);
			this->image = this->threads[this->filesInDirectory[this->fileIndex]].get();

			this->currentFileInfo = QFileInfo(this->getFullImagePath(this->fileIndex));
			//start loading next image
			std::unique_lock<std::mutex> lock(this->threadDeletionMutex);
			if (this->threads.find(this->filesInDirectory[this->nextFileIndex()]) == this->threads.end()) {
				this->threads[this->filesInDirectory[this->nextFileIndex()]] = std::async(std::launch::async, &MainInterface::readImage, this, this->getFullImagePath(this->nextFileIndex()), false);
			}
			lock.unlock();
			this->displayImageIfOk();
		}
		this->loading = false;
		this->cleanUpThreads();
	}

	void MainInterface::loadPreviousImage() {
		if (this->loading) return;
		this->loading = true;
		if (this->filesInDirectory.size() != 0) {
			this->fileIndex = this->previousFileIndex();
			if (this->threads.find(this->filesInDirectory[this->fileIndex]) == this->threads.end()) {
				this->loading = false;
				return;
			}
			this->waitForThreadToFinish(this->threads[this->filesInDirectory[this->fileIndex]]);
			this->image = this->threads[this->filesInDirectory[this->fileIndex]].get();
			this->currentFileInfo = QFileInfo(this->getFullImagePath(this->fileIndex));
			//start loading previous image
			std::unique_lock<std::mutex> lock(this->threadDeletionMutex);
			if (this->threads.find(this->filesInDirectory[this->previousFileIndex()]) == this->threads.end()) {
				this->threads[this->filesInDirectory[this->previousFileIndex()]] = std::async(std::launch::async, &MainInterface::readImage, this, this->getFullImagePath(this->previousFileIndex()), false);
			}
			lock.unlock();
			this->displayImageIfOk();
		}
		this->loading = false;
		this->cleanUpThreads();
	}

	void MainInterface::enterFullscreen() {
		//this->imageView->setInterfaceBackgroundColor(Qt::black);
		this->showFullScreen();
		this->hideMenuBar();
		this->mouseHideTimer->start(this->mouseHideDelay);
	}

	void MainInterface::exitFullscreen() {
		//QPalette palette = qApp->palette();
		//this->imageView->setInterfaceBackgroundColor(palette.base().color());
		this->showNormal();
		if (this->menuBarAutoHideAction->isChecked()) showMenuBar();
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
		double lineSpacing = 30;
		if (this->currentImageUnreadable) {
			QString message = tr("This file could not be read:");
			canvas.drawText(QPoint((canvas.device()->width() - metrics.width(message)) / 2.0, canvas.device()->height() / 2.0 - 0.5*lineSpacing), message);
			canvas.drawText(QPoint((canvas.device()->width() - metrics.width(this->currentFileInfo.fileName())) / 2.0, canvas.device()->height() / 2.0 + 0.5*lineSpacing + metrics.height()), this->currentFileInfo.fileName());
		}
		if (this->showInfoAction->isChecked() && this->imageView->imageAssigned()) {
		//draw current filename
			canvas.drawText(QPoint(30, 30 + metrics.height()), this->currentFileInfo.fileName());
			canvas.drawText(QPoint(30, 30 + lineSpacing + 2 * metrics.height()), QString("%1 Mb").arg(this->currentFileInfo.size() / 1048576.0, 0, 'f', 2));
		}
	}



	//============================================================================ PRIVATE SLOTS =============================================================================\\

	void MainInterface::cleanUpThreads() {
		std::lock_guard<std::mutex> lock(this->threadDeletionMutex);
		for (std::map<QString, std::shared_future<cv::Mat>>::iterator it = this->threads.begin(); it != this->threads.end();) {
			int index = this->filesInDirectory.indexOf(it->first);
			if (index != -1 && (index < (this->fileIndex - 1) || index >(this->fileIndex + 1)) && it->second.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
				it = this->threads.erase(it);
			} else {
				++it;
			}
		}
		if (this->threads.size() > 3) this->threadCleanUpTimer->start(this->threadCleanUpInterval);
	}

	void MainInterface::quit() {
		this->close();
	}

	void MainInterface::hideMouse() const {
		qApp->setOverrideCursor(Qt::BlankCursor);
		mouseHideTimer->stop();
	}

	void MainInterface::showMouse() const {
		qApp->setOverrideCursor(Qt::ArrowCursor);
	}

	void MainInterface::showMenuBar() {
		if (this->isFullScreen()) this->mouseHideTimer->stop();
		this->menuBar()->setVisible(true);
	}

	void MainInterface::hideMenuBar(QAction* triggeringAction) {
		if (!this->menuBarAutoHideAction->isChecked() || this->isFullScreen()) {
			this->menuBar()->setVisible(false);
			if (this->isFullScreen()) this->mouseHideTimer->start(this->mouseHideDelay);
		}
	}

	void MainInterface::reactToshowInfoToggle(bool value) {
		this->imageView->update();
		this->settings.setValue("showImageInfo", value);
	}

	void MainInterface::reactToReadImageCompletion(cv::Mat image) {
		this->image = image;
		this->displayImageIfOk();
		if (this->filesInDirectory.size() != 0) {
			//preload next and previous image in background
			std::lock_guard<std::mutex> lock(this->threadDeletionMutex);
			if (this->threads.find(this->filesInDirectory[this->previousFileIndex()]) == this->threads.end()) {
				this->threads[this->filesInDirectory[this->previousFileIndex()]] = std::async(std::launch::async, &MainInterface::readImage, this, this->getFullImagePath(this->previousFileIndex()), false);
			}
			if (this->threads.find(this->filesInDirectory[this->nextFileIndex()]) == this->threads.end()) {
				this->threads[this->filesInDirectory[this->nextFileIndex()]] = std::async(std::launch::async, &MainInterface::readImage, this, this->getFullImagePath(this->nextFileIndex()), false);
			}
		}
		this->loading = false;
	}

	void MainInterface::openDialog() {
		QString path = QFileDialog::getOpenFileName(this, tr("Open Config File"), QDir::rootPath(), "All Files (*.*);; Image Files (*.bmp *.dib *.jpeg *.jpg *.jpe *.jpeg *.jp2 *.png *.webp *.pbm *.pgm *.ppm *.sr *.ras *.tiff *.tif);;");

		if (!path.isEmpty()) {
			this->loadImage(path);
		}
	}

	void MainInterface::reactToSmoothingToggle(bool value) {
		this->imageView->setUseSmoothTransform(value);
		this->settings.setValue("useSmoothEnlargmentInterpolation", value);
	}

	void MainInterface::reactToEnlargementToggle(bool value) {
		this->imageView->setPreventMagnificationInDefaultZoom(!value);
		this->settings.setValue("enlargeSmallImages", value);
	}

	void MainInterface::reactoToAutoHideMenuBarToggle(bool value) {
		this->settings.setValue("autoHideMenuBar", !value);
		if (value) {
			this->showMenuBar();
		} else {
			this->hideMenuBar();
		}
	}

}