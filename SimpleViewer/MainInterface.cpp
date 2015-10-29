#include "MainInterface.h"

namespace sv {

	Image::Image() { }

	Image::Image(cv::Mat mat, std::shared_ptr<ExifData> exifData) : matrix(mat), exifData(exifData), valid(true) { }

	cv::Mat Image::mat() const {
		return this->matrix;
	}

	std::shared_ptr<ExifData> Image::exif() const {
		return this->exifData;
	}

	bool Image::isValid() const {
		return valid;
	}

	//============================================================================ MAIN INTERFACE ============================================================================\\

	MainInterface::MainInterface(QString openWithFilename, QWidget *parent)
		: QMainWindow(parent),
		settings(new QSettings(QSettings::IniFormat, QSettings::UserScope, "Simple Viewer", "Simple Viewer")) {

		setAcceptDrops(true);
		qRegisterMetaType<Image>("Image");
		QObject::connect(this, SIGNAL(readImageFinished(Image)), this, SLOT(reactToReadImageCompletion(Image)));
		this->setWindowTitle(this->programTitle);

		this->imageView = new hb::ImageView(this);
		this->imageView->setShowInterfaceOutline(false);
		this->imageView->setUseSmoothTransform(false);
		this->imageView->installEventFilter(this);
		this->imageView->setExternalPostPaintFunction(this, &MainInterface::infoPaintFunction);
		this->imageView->setInterfaceBackgroundColor(Qt::black);
		this->imageView->setPreventMagnificationInDefaultZoom(true);
		this->imageView->setPostResizeSharpening(false, this->settings->value("sharpeningStrength", 0.5).toDouble(), this->settings->value("sharpeningRadius", 0.5).toDouble());
		setCentralWidget(this->imageView);

		this->slideshowDialog = new SlideshowDialog(settings, this);
		this->slideshowDialog->setWindowModality(Qt::WindowModal);
		QObject::connect(this->slideshowDialog, SIGNAL(dialogConfirmed()), this, SLOT(startSlideshow()));
		QObject::connect(this->slideshowDialog, SIGNAL(dialogClosed()), this, SLOT(enableAutomaticMouseHide()));

		this->sharpeningDialog = new SharpeningDialog(settings, this);
		QObject::connect(this->sharpeningDialog, SIGNAL(sharpeningParametersChanged()), this, SLOT(updateSharpening()));
		QObject::connect(this->sharpeningDialog, SIGNAL(dialogClosed()), this, SLOT(enableAutomaticMouseHide()));

		QObject::connect(this->menuBar(), SIGNAL(triggered(QAction*)), this, SLOT(hideMenuBar(QAction*)));
		this->fileMenu = this->menuBar()->addMenu(tr("&File"));
		this->viewMenu = this->menuBar()->addMenu(tr("&View"));
		this->slideshowMenu = this->menuBar()->addMenu(tr("&Slideshow"));
#ifdef Q_OS_WIN
		this->applicationMenu = this->menuBar()->addMenu(tr("&Application"));
		QObject::connect(this->applicationMenu, SIGNAL(aboutToShow()), this, SLOT(populateApplicationMenu()));
		this->installAction = new QAction(tr("&Install"), this);
		QObject::connect(this->installAction, SIGNAL(triggered()), this, SLOT(runInstaller()));
		this->uninstallAction = new QAction(tr("&Uninstall"), this);
		QObject::connect(this->uninstallAction, SIGNAL(triggered()), this, SLOT(runUninstaller()));
#endif

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
		QObject::connect(this->showInfoAction, SIGNAL(triggered(bool)), this, SLOT(reactToShowInfoToggle(bool)));
		this->viewMenu->addAction(this->showInfoAction);
		this->addAction(this->showInfoAction);

		this->zoomLevelAction = new QAction(tr("Show &Zoom Level"), this);
		this->zoomLevelAction->setCheckable(true);
		this->zoomLevelAction->setChecked(false);
		this->zoomLevelAction->setShortcut(Qt::Key_Z);
		this->zoomLevelAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->zoomLevelAction, SIGNAL(triggered(bool)), this, SLOT(reactToShowZoomLevelToggle(bool)));
		this->viewMenu->addAction(this->zoomLevelAction);
		this->addAction(this->zoomLevelAction);

		this->enlargementAction = new QAction(tr("&Enlarge Smaller Images"), this);
		this->enlargementAction->setCheckable(true);
		this->enlargementAction->setChecked(false);
		this->enlargementAction->setShortcut(Qt::Key_U);
		this->enlargementAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->enlargementAction, SIGNAL(triggered(bool)), this, SLOT(reactToEnlargementToggle(bool)));
		this->viewMenu->addAction(this->enlargementAction);
		this->addAction(this->enlargementAction);

		this->sharpeningAction = new QAction(tr("Sharpen Images After &Downsampling"), this);
		this->sharpeningAction->setCheckable(true);
		this->sharpeningAction->setChecked(false);
		this->sharpeningAction->setShortcut(Qt::Key_E);
		this->sharpeningAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->sharpeningAction, SIGNAL(triggered(bool)), this, SLOT(reactToSharpeningToggle(bool)));
		this->viewMenu->addAction(this->sharpeningAction);
		this->addAction(this->sharpeningAction);

		this->smoothingAction = new QAction(tr("Use &Smooth Interpolation when Enlarging"), this);
		this->smoothingAction->setCheckable(true);
		this->smoothingAction->setChecked(false);
		this->smoothingAction->setShortcut(Qt::Key_S);
		this->smoothingAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->smoothingAction, SIGNAL(triggered(bool)), this, SLOT(reactToSmoothingToggle(bool)));
		this->viewMenu->addAction(this->smoothingAction);
		this->addAction(this->smoothingAction);

		this->menuBarAutoHideAction = new QAction(tr("&Always Show Menu Bar"), this);
		this->menuBarAutoHideAction->setCheckable(true);
		this->menuBarAutoHideAction->setChecked(false);
		this->menuBarAutoHideAction->setShortcut(Qt::Key_M);
		this->menuBarAutoHideAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->menuBarAutoHideAction, SIGNAL(triggered(bool)), this, SLOT(reactoToAutoHideMenuBarToggle(bool)));
		this->viewMenu->addAction(this->menuBarAutoHideAction);
		this->addAction(this->menuBarAutoHideAction);

		this->viewMenu->addSeparator();

		this->sharpeningOptionsAction = new QAction(tr("Sharpening Options..."), this);
		this->sharpeningOptionsAction->setShortcut(Qt::Key_O);
		this->sharpeningOptionsAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->sharpeningOptionsAction, SIGNAL(triggered(bool)), this, SLOT(showSharpeningOptions()));
		this->viewMenu->addAction(this->sharpeningOptionsAction);
		this->addAction(this->sharpeningOptionsAction);

		this->slideshowAction = new QAction(tr("&Start Slideshow"), this);
		this->slideshowAction->setEnabled(false);
		this->slideshowAction->setShortcut(Qt::Key_P);
		this->slideshowAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->slideshowAction, SIGNAL(triggered()), this, SLOT(toggleSlideshow()));
		this->slideshowMenu->addAction(this->slideshowAction);
		this->addAction(this->slideshowAction);

		this->slideshowNoDialogAction = new QAction(this);
		this->slideshowNoDialogAction->setEnabled(false);
		this->slideshowNoDialogAction->setShortcut(Qt::CTRL + Qt::Key_P);
		this->slideshowNoDialogAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->slideshowNoDialogAction, SIGNAL(triggered()), this, SLOT(toggleSlideshowNoDialog()));
		this->addAction(this->slideshowNoDialogAction);

		//mouse hide timer in fullscreen
		this->mouseHideTimer = new QTimer(this);
		QObject::connect(this->mouseHideTimer, SIGNAL(timeout()), this, SLOT(hideMouse()));

		//timer for thread cleanup
		this->threadCleanUpTimer = new QTimer(this);
		this->threadCleanUpTimer->setSingleShot(true);
		QObject::connect(this->threadCleanUpTimer, SIGNAL(timeout()), this, SLOT(cleanUpThreads()));

		//timer for the slideshow
		this->slideshowTimer = new QTimer(this);
		QObject::connect(this->slideshowTimer, SIGNAL(timeout()), this, SLOT(nextSlide()));

		//load settings
		this->showInfoAction->setChecked(this->settings->value("showImageInfo", false).toBool());
		this->zoomLevelAction->setChecked(this->settings->value("showZoomLevel", false).toBool());
		this->enlargementAction->setChecked(this->settings->value("enlargeSmallImages", false).toBool());
		this->reactToEnlargementToggle(this->enlargementAction->isChecked());
		this->smoothingAction->setChecked(this->settings->value("useSmoothEnlargmentInterpolation", false).toBool());
		this->reactToSmoothingToggle(this->smoothingAction->isChecked());
		this->sharpeningAction->setChecked(this->settings->value("sharpenImagesAfterDownscale", false).toBool());
		this->reactToSharpeningToggle(this->sharpeningAction->isChecked());
		this->menuBarAutoHideAction->setChecked(!this->settings->value("autoHideMenuBar", true).toBool());
		this->reactoToAutoHideMenuBarToggle(this->menuBarAutoHideAction->isChecked());
		if (this->settings->value("maximized", false).toBool()) {
			this->showMaximized();
		}

		if (openWithFilename != QString()) {
			this->loadImage(openWithFilename);
		}
	}

	MainInterface::~MainInterface() {
		delete this->imageView;
		delete this->slideshowDialog;
		delete this->sharpeningDialog;
		delete this->fileMenu;
		delete this->viewMenu;
		delete this->slideshowMenu;
		delete this->applicationMenu;
		delete this->quitAction;
		delete this->openAction;
		delete this->showInfoAction;
		delete this->smoothingAction;
		delete this->enlargementAction;
		delete this->sharpeningAction;
		delete this->sharpeningOptionsAction;
		delete this->menuBarAutoHideAction;
		delete this->slideshowAction;
		delete this->slideshowNoDialogAction;
		delete this->zoomLevelAction;
		delete this->installAction;
		delete this->uninstallAction;
		delete this->mouseHideTimer;
		delete this->threadCleanUpTimer;
		delete this->slideshowTimer;
	}

	QSize MainInterface::sizeHint() const {
		return QSize(900, 600);
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
			this->enableAutomaticMouseHide();
		}
		e->ignore();
	}

	void MainInterface::changeEvent(QEvent* e) {
		if (e->type() == QEvent::WindowStateChange) {
			if (!this->isMinimized() && !this->isFullScreen()) {
				this->settings->setValue("maximized", this->isMaximized());
			} else if (this->isFullScreen()) {
				QWindowStateChangeEvent* windowStateChangeEvent = static_cast<QWindowStateChangeEvent*>(e);
				this->settings->setValue("maximized", bool(windowStateChangeEvent->oldState() & Qt::WindowMaximized));
			}
		}
	}

	//=============================================================================== PRIVATE ===============================================================================\\

	Image MainInterface::readImage(QString path, bool emitSignals) {
		cv::Mat image;
		std::shared_ptr<ExifData> exifData;
#ifdef Q_OS_WIN
		if (!utility::isASCII(path)) {
			std::vector<char> buffer = utility::readFileIntoBuffer(path);
			if (buffer.empty()) {
				if (emitSignals) emit(readImageFinished(Image()));
				return Image();
			}
			image = cv::imdecode(buffer, CV_LOAD_IMAGE_COLOR);
			exifData = std::shared_ptr<ExifData>(new ExifData(buffer));
		} else {
#endif
			image = cv::imread(path.toStdString(), CV_LOAD_IMAGE_COLOR);
			exifData = std::shared_ptr<ExifData>(new ExifData(path));
#ifdef Q_OS_WIN
		}
#endif
		Image result;
		if (image.data) {
			cv::cvtColor(image, image, CV_BGR2RGB);
			QObject::connect(exifData.get(), SIGNAL(loadingFinished(ExifData*)), this, SLOT(reactToExifLoadingCompletion(ExifData*)));
			result = Image(image, exifData);
		}
		if (emitSignals) emit(readImageFinished(result));
		return result;
	}

	void MainInterface::loadNextImage() {
		if (this->loading) return;
		std::unique_lock<std::mutex> lock(this->threadDeletionMutex);
		this->loading = true;

		if (this->filesInDirectory.size() != 0) {
			this->currentFileIndex = this->nextFileIndex();
			if (this->threads.find(this->filesInDirectory[this->currentFileIndex]) == this->threads.end()) {
				this->loading = false;
				return;
			}
			this->waitForThreadToFinish(this->threads[this->filesInDirectory[this->currentFileIndex]]);
			this->image = this->threads[this->filesInDirectory[this->currentFileIndex]].get();

			this->currentFileInfo = QFileInfo(this->getFullImagePath(this->currentFileIndex));
			//start loading next image
			if (this->threads.find(this->filesInDirectory[this->nextFileIndex()]) == this->threads.end()) {
				this->threads[this->filesInDirectory[this->nextFileIndex()]] = std::async(std::launch::async,
																						  &MainInterface::readImage,
																						  this,
																						  this->getFullImagePath(this->nextFileIndex()),
																						  false);
			}
			lock.unlock();
			this->displayImageIfOk();
		}
		this->loading = false;
		this->cleanUpThreads();
	}

	void MainInterface::loadPreviousImage() {
		if (this->loading) return;
		std::unique_lock<std::mutex> lock(this->threadDeletionMutex);
		this->loading = true;
		if (this->filesInDirectory.size() != 0) {
			this->currentFileIndex = this->previousFileIndex();
			if (this->threads.find(this->filesInDirectory[this->currentFileIndex]) == this->threads.end()) {
				this->loading = false;
				return;
			}
			this->waitForThreadToFinish(this->threads[this->filesInDirectory[this->currentFileIndex]]);
			this->image = this->threads[this->filesInDirectory[this->currentFileIndex]].get();
			this->currentFileInfo = QFileInfo(this->getFullImagePath(this->currentFileIndex));
			//start loading previous image
			if (this->threads.find(this->filesInDirectory[this->previousFileIndex()]) == this->threads.end()) {
				this->threads[this->filesInDirectory[this->previousFileIndex()]] = std::async(std::launch::async,
																							  &MainInterface::readImage,
																							  this,
																							  this->getFullImagePath(this->previousFileIndex()),
																							  false);
			}
			lock.unlock();
			this->displayImageIfOk();
		}
		this->loading = false;
		this->cleanUpThreads();
	}

	void MainInterface::clearThreads() {
		for (std::map<QString, std::shared_future<Image>>::iterator it = this->threads.begin(); it != this->threads.end(); ++it) {
			this->waitForThreadToFinish(it->second);
		}
		this->threads.clear();
	}

	void MainInterface::waitForThreadToFinish(std::shared_future<Image> const& thread) {
		if (!thread.valid()) return;
		if (thread.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
			this->setWindowTitle(this->windowTitle() + QString(tr(" - Loading...")));
		}

		while (thread.wait_for(std::chrono::milliseconds(this->eventProcessIntervalDuringWait)) != std::future_status::ready) {
			qApp->processEvents();
		}
	}

	size_t MainInterface::nextFileIndex() const {
		return (this->currentFileIndex + 1) % this->filesInDirectory.size();
	}

	size_t MainInterface::previousFileIndex() const {
		if (this->currentFileIndex == 0) return this->filesInDirectory.size() - 1;
		return (this->currentFileIndex - 1) % this->filesInDirectory.size();
	}

	QString MainInterface::getFullImagePath(size_t index) const {
		return this->currentDirectory.absoluteFilePath(this->filesInDirectory[index]);
	}

	void MainInterface::loadImage(QString path) {
		if (this->loading) return;
		std::unique_lock<std::mutex> lock(this->threadDeletionMutex);
		this->loading = true;
		//find the path in the current directory listing
		QFileInfo fileInfo = QFileInfo(QDir::cleanPath(path));
		QDir directory = fileInfo.absoluteDir();
		QString filename = fileInfo.fileName();
		if (directory != this->currentDirectory || this->noCurrentDir) {
			this->currentDirectory = directory;
			this->noCurrentDir = false;
			QStringList filters;
			filters << "*.bmp" << "*.dib" << "*.jpeg" << "*.jpg" << "*.jpe" << "*.jpeg" << "*.jp2" << "*.png" << "*.webp" << "*.pbm" << "*.pgm" << "*.ppm" << "*.sr" << "*.ras" << "*.tiff" << "*.tif";
			QStringList contents = directory.entryList(filters, QDir::Files);
			QCollator collator;
			collator.setNumericMode(true);
			std::sort(contents.begin(), contents.end(), collator);
			this->filesInDirectory = contents.toVector();
		}
		if (this->filesInDirectory.size() == 0 || this->currentFileIndex <= 0 || this->currentFileIndex >= this->filesInDirectory.size() || this->filesInDirectory.at(this->currentFileIndex) != filename) {
			this->currentFileIndex = this->filesInDirectory.indexOf(filename);
		}
		this->currentFileInfo = fileInfo;
		this->clearThreads();
		this->threads[filename] = std::async(std::launch::async, &MainInterface::readImage, this, path, true);
		this->setWindowTitle(this->windowTitle() + QString(tr(" - Loading...")));
		this->paintLoadingHint = true;
		this->imageView->update();
	}

	void MainInterface::displayImageIfOk() {
		if (this->image.isValid()) {
			this->currentImageUnreadable = false;
			this->imageView->setImage(this->image.mat());
		} else {
			this->currentImageUnreadable = true;
			this->imageView->resetImage();
		}
		this->setWindowTitle(QString("%1 - %2 - %3 of %4").arg(this->programTitle,
															   this->currentFileInfo.fileName()).arg(this->currentFileIndex + 1).arg(this->filesInDirectory.size()));
	}

	void MainInterface::enterFullscreen() {
		//this->imageView->setInterfaceBackgroundColor(Qt::black);
		this->showFullScreen();
		this->hideMenuBar();
		this->enableAutomaticMouseHide();
	}

	void MainInterface::exitFullscreen() {
		//QPalette palette = qApp->palette();
		//this->imageView->setInterfaceBackgroundColor(palette.base().color());
		if (this->settings->value("maximized", false).toBool()) {
			this->showMaximized();
		} else {
			this->showNormal();
		}
		if (this->menuBarAutoHideAction->isChecked()) showMenuBar();
		this->disableAutomaticMouseHide();
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
		double const lineSpacing = 10;
		if (this->currentImageUnreadable) {
			QString message = tr("This file could not be read:");
			canvas.drawText(QPoint((canvas.device()->width() - metrics.width(message)) / 2.0, canvas.device()->height() / 2.0 - 0.5*lineSpacing),
							message);
			canvas.drawText(QPoint((canvas.device()->width() - metrics.width(this->currentFileInfo.fileName())) / 2.0,
								   canvas.device()->height() / 2.0 + 0.5*lineSpacing + metrics.height()),
							this->currentFileInfo.fileName());
		}
		if (this->paintLoadingHint) {
			QString message = tr("Loading...");
			canvas.drawText(QPoint((canvas.device()->width() - metrics.width(message)) / 2.0, canvas.device()->height() / 2.0 + 0.5*metrics.height()),
							message);
		}
		if (this->showInfoAction->isChecked() && this->imageView->imageAssigned()) {
		//draw current filename
			canvas.drawText(QPoint(30, 30 + metrics.height()),
							this->currentFileInfo.fileName());
			canvas.drawText(QPoint(30, 30 + lineSpacing + 2 * metrics.height()),
							QString("%1 Mb").arg(this->currentFileInfo.size() / 1048576.0, 0, 'f', 2));
			if (this->image.isValid()) {
				if (this->image.exif()->isReady()) {
					if (this->image.exif()->hasExif()) {
						//get camera model, speed, aperture and ISO
						QString cameraModel = QString::fromStdString(this->image.exif()->value("Exif.Image.Model")->toString()).trimmed();
						QString aperture = "";
						if (this->image.exif()->hasValue("Exif.Photo.FNumber")) {
							Exiv2::Rational apertureValue = this->image.exif()->value("Exif.Photo.FNumber")->toRational();
							aperture = QString::number(double(apertureValue.first) / double(apertureValue.second));
						}
						QString speed = "";
						if (this->image.exif()->hasValue("Exif.Photo.ExposureTime")) {
							Exiv2::Rational speedValue = this->image.exif()->value("Exif.Photo.ExposureTime")->toRational();
							if (speedValue.first < speedValue.second) {
								speed = QString("%1/%2").arg(speedValue.first/speedValue.first).arg(speedValue.second/speedValue.first);
							} else {
								speed = QString::number(double(speedValue.first) / double(speedValue.second));
							}
						}
						QString iso = "";
						if (this->image.exif()->hasValue("Exif.Photo.ISOSpeedRatings")) {
							long isoValue = this->image.exif()->value("Exif.Photo.ISOSpeedRatings")->toLong();
							iso = QString::number(isoValue);
						}
						QString captureDate = "";
						if (this->image.exif()->hasValue("Exif.Photo.DateTimeOriginal")) {
							captureDate = QString::fromStdString(this->image.exif()->value("Exif.Photo.DateTimeOriginal")->toString());
							QString date = captureDate.section(' ', 0, 0);
							QString time = captureDate.section(' ', 1, 1);
							date.replace(':', '-');
							captureDate = QString("%1 %2").arg(date).arg(time);
						}
						//calculate the v coordinates for the lines
						int cameraModelTopOffset = 30 + 2 * lineSpacing + 3 * metrics.height();
						int apertureAndSpeedTopOffset = 30 + 3 * lineSpacing + 4 * metrics.height();
						int isoTopOffset = 30 + 4 * lineSpacing + 5 * metrics.height();
						int dateTopOffset = 30 + 5 * lineSpacing + 6 * metrics.height();
						if (cameraModel.isEmpty()) {
							apertureAndSpeedTopOffset -= lineSpacing + metrics.height();
							isoTopOffset -= lineSpacing + metrics.height();
							dateTopOffset -= lineSpacing + metrics.height();
						}
						if (aperture.isEmpty() && speed.isEmpty()) {
							isoTopOffset -= lineSpacing + metrics.height();
							dateTopOffset -= lineSpacing + metrics.height();
						}
						if (iso.isEmpty()) dateTopOffset -= lineSpacing + metrics.height();
						//draw the EXIF text
						if (!cameraModel.isEmpty()) canvas.drawText(QPoint(30, cameraModelTopOffset),
																	cameraModel);
						if (!aperture.isEmpty() && !speed.isEmpty()) {
							QString apertureAndSpeed = QString("%1s @ f%2").arg(speed).arg(aperture);
							canvas.drawText(QPoint(30, apertureAndSpeedTopOffset),
											apertureAndSpeed);
						} else if (!aperture.isEmpty()) {
							canvas.drawText(QPoint(30, apertureAndSpeedTopOffset),
											QString("f%2").arg(aperture));
						} else if (!speed.isEmpty()) {
							canvas.drawText(QPoint(30, apertureAndSpeedTopOffset),
											QString("%1s").arg(speed));
						}
						if (!iso.isEmpty()) canvas.drawText(QPoint(30, isoTopOffset),
															QString("ISO%1").arg(iso));
						if (!captureDate.isEmpty()) canvas.drawText(QPoint(30, dateTopOffset),
																	QString("%1").arg(captureDate));
					}
				} else {
					canvas.drawText(QPoint(30, 30 + 2 * lineSpacing + 3 * metrics.height()),
									tr("Loading EXIF..."));
				}
			}
		}
		if (this->zoomLevelAction->isChecked() && this->imageView->imageAssigned()) {
			QString message = QString::number(this->imageView->getCurrentPreviewScalingFactor() * 100, 'f', 1).append("%");
			canvas.drawText(QPoint(30, canvas.device()->height() - 30), message);
		}
	}

	bool MainInterface::applicationIsInstalled() {
#ifdef Q_OS_WIN
		QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE", QSettings::NativeFormat);
		return registry.contains("Microsoft/Windows/CurrentVersion/Uninstall/SimpleViewer/UninstallString");
#else
		return false;
#endif

}

//============================================================================ PRIVATE SLOTS =============================================================================\\

	void MainInterface::nextSlide() {
		this->loadNextImage();
		if (!settings->value("slideshowLoop", false).toBool() && this->currentFileIndex == (this->filesInDirectory.size() - 1)) {
			this->stopSlideshow();
		}
	}

	void MainInterface::cleanUpThreads() {
		try {
			std::unique_lock<std::mutex> lock(this->threadDeletionMutex);
		} catch (...) {
			//Couldn't lock the mutex, probably because this thread already owns it. 
			if (this->threads.size() > 3) this->threadCleanUpTimer->start(this->threadCleanUpInterval);
			return;
		}
		size_t previousIndex = this->previousFileIndex();
		size_t nextIndex = this->nextFileIndex();
		for (std::map<QString, std::shared_future<Image>>::iterator it = this->threads.begin(); it != this->threads.end();) {
			int index = this->filesInDirectory.indexOf(it->first);
			//see if the thread has finished loading
			//also the exif should have finished loading to prevent blocking, check if it's valid first to not derefence an invalid pointer
			if (index != -1
				&& index != this->currentFileIndex
				&& index != previousIndex
				&& index != nextIndex
				&& it->second.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready
				&& (!it->second.get().isValid() || it->second.get().exif()->isReady())) {
				it = this->threads.erase(it);
			} else {
				++it;
			}
		}
		if (this->threads.size() > 3) this->threadCleanUpTimer->start(this->threadCleanUpInterval);
	}

	void MainInterface::quit() {
		QCoreApplication::quit();
	}

	void MainInterface::hideMouse() const {
		qApp->setOverrideCursor(Qt::BlankCursor);
		mouseHideTimer->stop();
	}

	void MainInterface::showMouse() const {
		qApp->setOverrideCursor(Qt::ArrowCursor);
	}

	void MainInterface::enableAutomaticMouseHide() {
		if (this->isFullScreen() && !this->menuBar()->isVisible() && !this->slideshowDialog->isVisible() && !this->sharpeningDialog->isVisible()) {
			this->mouseHideTimer->start(this->mouseHideDelay);
		}
	}

	void MainInterface::disableAutomaticMouseHide() {
		this->mouseHideTimer->stop();
		this->showMouse();
	}

	void MainInterface::showMenuBar() {
		if (this->isFullScreen()) this->disableAutomaticMouseHide();
		this->menuBar()->setVisible(true);
	}

	void MainInterface::hideMenuBar(QAction* triggeringAction) {
		if (!this->menuBarAutoHideAction->isChecked() || this->isFullScreen()) {
			this->menuBar()->setVisible(false);
			this->enableAutomaticMouseHide();
		}
	}

	void MainInterface::populateApplicationMenu() {
		this->applicationMenu->removeAction(installAction);
		this->applicationMenu->removeAction(uninstallAction);
		if (applicationIsInstalled()) {
			this->applicationMenu->addAction(this->uninstallAction);
		} else {
			this->applicationMenu->addAction(this->installAction);
		}

	}

	void MainInterface::runInstaller() {
		QString installerPath = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("WinInstaller.exe");
		if (QFileInfo(installerPath).exists()) {
			ShellExecuteW(GetDesktopWindow(),
						  nullptr,
						  reinterpret_cast<LPCWSTR>(installerPath.utf16()),
						  nullptr,
						  reinterpret_cast<LPCWSTR>(QCoreApplication::applicationDirPath().utf16()),
						  SW_SHOWNORMAL);
			QCoreApplication::quit();
		} else {
			QMessageBox::critical(this,
								  tr("Installer not found"),
								  tr("The installer executable (WinInstaller.exe) could not be found. Make sure it is located in the same directory as SimpleViewer.exe."),
								  QMessageBox::Close);
		}
	}

	void MainInterface::runUninstaller() {
#ifdef Q_OS_WIN
		QString installerPath = QDir::toNativeSeparators(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("WinInstaller.exe"));
		if (QFileInfo(installerPath).exists()) {
			ShellExecuteW(GetDesktopWindow(),
						  nullptr,
						  reinterpret_cast<LPCWSTR>(installerPath.utf16()),
						  L"-uninstall",
						  reinterpret_cast<LPCWSTR>(QCoreApplication::applicationDirPath().utf16()),
						  SW_SHOWNORMAL);
			QCoreApplication::quit();
		} else {
			QMessageBox::critical(this,
								  tr("Installer not found"),
								  tr("The installer executable (WinInstaller.exe) could not be found. Make sure it is located in the same directory as SimpleViewer.exe."),
								  QMessageBox::Close);
		}
#endif
	}

	void MainInterface::toggleSlideshow() {
		if (this->slideshowTimer->isActive()) {
			this->stopSlideshow();
		} else {
			this->slideshowDialog->show();
			this->disableAutomaticMouseHide();
		}
	}

	void MainInterface::toggleSlideshowNoDialog() {
		if (this->slideshowTimer->isActive()) {
			this->stopSlideshow();
		} else {
			this->startSlideshow();
		}
	}

	void MainInterface::startSlideshow() {
		this->slideshowAction->setText(tr("&Stop Slideshow"));
		this->slideshowTimer->start(std::abs(this->settings->value("slideDelay", 3).toDouble()) * 1000);
	}

	void MainInterface::stopSlideshow() {
		this->slideshowAction->setText(tr("&Start Slideshow"));
		this->slideshowTimer->stop();
	}

	void MainInterface::reactToShowInfoToggle(bool value) {
		this->imageView->update();
		this->settings->setValue("showImageInfo", value);
	}

	void MainInterface::reactToShowZoomLevelToggle(bool value) {
		this->imageView->update();
		this->settings->setValue("showZoomLevel", value);
	}

	void MainInterface::reactToReadImageCompletion(Image image) {
		this->image = image;
		this->paintLoadingHint = false;
		this->displayImageIfOk();
		if (this->filesInDirectory.size() != 0) {
			//preload next and previous image in background
			std::lock_guard<std::mutex> lock(this->threadDeletionMutex);
			if (this->threads.find(this->filesInDirectory[this->previousFileIndex()]) == this->threads.end()) {
				this->threads[this->filesInDirectory[this->previousFileIndex()]] = std::async(std::launch::async,
																							  &MainInterface::readImage,
																							  this,
																							  this->getFullImagePath(this->previousFileIndex()),
																							  false);
			}
			if (this->threads.find(this->filesInDirectory[this->nextFileIndex()]) == this->threads.end()) {
				this->threads[this->filesInDirectory[this->nextFileIndex()]] = std::async(std::launch::async,
																						  &MainInterface::readImage,
																						  this, this->getFullImagePath(this->nextFileIndex()),
																						  false);
			}
		}
		this->slideshowAction->setEnabled(true);
		this->slideshowNoDialogAction->setEnabled(true);
		this->loading = false;
	}

	void MainInterface::reactToExifLoadingCompletion(ExifData* sender) {
		if (this->showInfoAction->isChecked()) {
		//if the sender is the currently displayed image
			if (this->threads[this->filesInDirectory[this->currentFileIndex]].get().exif().get() == sender) {
				this->update();
			}
		}
	}

	void MainInterface::openDialog() {
		QString path = QFileDialog::getOpenFileName(this,
													tr("Open Config File"),
													QDir::rootPath(),
													"All Files (*.*);; Image Files (*.bmp *.dib *.jpeg *.jpg *.jpe *.jpeg *.jp2 *.png *.webp *.pbm *.pgm *.ppm *.sr *.ras *.tiff *.tif);;");

		if (!path.isEmpty()) {
			this->loadImage(path);
		}
	}

	void MainInterface::reactToSmoothingToggle(bool value) {
		this->imageView->setUseSmoothTransform(value);
		this->settings->setValue("useSmoothEnlargmentInterpolation", value);
	}

	void MainInterface::reactToEnlargementToggle(bool value) {
		this->imageView->setPreventMagnificationInDefaultZoom(!value);
		this->settings->setValue("enlargeSmallImages", value);
	}

	void MainInterface::reactToSharpeningToggle(bool value) {
		this->imageView->setEnablePostResizeSharpening(value);
		this->settings->setValue("sharpenImagesAfterDownscale", value);
	}

	void MainInterface::reactoToAutoHideMenuBarToggle(bool value) {
		this->settings->setValue("autoHideMenuBar", !value);
		if (value) {
			this->showMenuBar();
		} else {
			this->hideMenuBar();
		}
	}

	void MainInterface::showSharpeningOptions() {
		this->disableAutomaticMouseHide();
		this->sharpeningDialog->show();
		this->sharpeningDialog->raise();
		this->sharpeningDialog->activateWindow();
	}

	void MainInterface::updateSharpening() {
		this->sharpeningAction->setChecked(this->settings->value("sharpenImagesAfterDownscale", false).toBool());
		this->imageView->setPostResizeSharpening(this->sharpeningAction->isChecked(),
												 this->settings->value("sharpeningStrength", 0.5).toDouble(),
												 this->settings->value("sharpeningRadius", 1).toDouble());
	}

}