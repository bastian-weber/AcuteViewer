#include "MainInterface.h"

namespace sv {

	Image::Image() { }

	Image::Image(cv::Mat mat, std::shared_ptr<ExifData> exifData, bool isPreviewImage) : matrix(mat), exifData(exifData), valid(true), previewImage(isPreviewImage) { }

	cv::Mat Image::mat() const {
		return matrix;
	}

	std::shared_ptr<ExifData> Image::exif() const {
		return exifData;
	}

	bool Image::isValid() const {
		return valid;
	}

	bool Image::isPreviewImage() const {
		return previewImage;
	}

	//============================================================================ MAIN INTERFACE ============================================================================\\

	MainInterface::MainInterface(QString openWithFilename, QWidget *parent)
		: QMainWindow(parent),
		settings(new QSettings(QSettings::IniFormat, QSettings::UserScope, "Acute Viewer", "Acute Viewer")) {

		initialize();

		if (openWithFilename != QString()) {
			loadImage(openWithFilename);
		}
	}

	MainInterface::MainInterface(QStringList openWithFilenames, QWidget * parent) 
		: QMainWindow(parent),
		settings(new QSettings(QSettings::IniFormat, QSettings::UserScope, "Acute Viewer", "Acute Viewer")) {

		initialize();
		
		loadImages(openWithFilenames);
	}

	MainInterface::~MainInterface() {
		delete imageView;
		delete slideshowDialog;
		delete sharpeningDialog;
		delete fileMenu;
		delete viewMenu;
		delete slideshowMenu;
		delete applicationMenu;
		delete quitAction;
		delete openAction;
		delete refreshAction;
		delete resetSettingsAction;
		delete showInfoAction;
		delete smoothingAction;
		delete enlargementAction;
		delete sharpeningAction;
		delete sharpeningOptionsAction;
		delete menuBarAutoHideAction;
		delete slideshowAction;
		delete slideshowNoDialogAction;
		delete zoomLevelAction;
		delete installAction;
		delete uninstallAction;
		delete mouseHideTimer;
		delete threadCleanUpTimer;
		delete slideshowTimer;
	}

	QSize MainInterface::sizeHint() const {
		return settings->value("windowSize", QSize(900, 600)).toSize();
		//return QSize(900, 600);
	}

	//============================================================================== PROTECTED ==============================================================================\\

	bool MainInterface::eventFilter(QObject* object, QEvent* e) {
		if (e->type() == QEvent::MouseButtonRelease) {
			QMouseEvent* keyEvent = (QMouseEvent*)e;
			mouseReleaseEvent(keyEvent);
		} else if (e->type() == QEvent::Wheel) {
			if (menuBar()->isVisible()) {
				hideMenuBar();
			}
		} else if (e->type() == QEvent::MouseMove) {
			QMouseEvent* keyEvent = (QMouseEvent*)e;
			mouseMoveEvent(keyEvent);
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
			if (e->mimeData()->urls().size() == 1) {
				QString path = e->mimeData()->urls().first().toLocalFile();
				loadImage(path);
			} else {
				QStringList paths;
				for (QUrl& url : e->mimeData()->urls()) {
					paths.append(url.toLocalFile());
				}
				loadImages(paths);
			}
		}
	}

	void MainInterface::keyPressEvent(QKeyEvent* e) {
		if (e->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) {
			if (e->key() == Qt::Key_Right) {
				changeFontSizeBy(1);
				e->accept();
			} else if (e->key() == Qt::Key_Left) {
				changeFontSizeBy(-1);
				e->accept();
			} else if (e->key() == Qt::Key_Up) {
				changeLineSpacingBy(1);
				e->accept();
			} else if (e->key() == Qt::Key_Down) {
				changeLineSpacingBy(-1);
				e->accept();
			}
		} else if (e->key() == Qt::Key_Right || e->key() == Qt::Key_Down) {
			loadNextImage();
			e->accept();
		} else if (e->key() == Qt::Key_Left || e->key() == Qt::Key_Up) {
			loadPreviousImage();
			e->accept();
		} else if (e->key() == Qt::Key_Escape) {
			if (menuBar()->isVisible()) {
				hideMenuBar();
			} else if (isFullScreen()) {
				exitFullscreen();
			}
		} else {
			e->ignore();
		}
	}

	void MainInterface::keyReleaseEvent(QKeyEvent* e) {
		if (e->key() == Qt::Key_Alt && !skipNextAltRelease) {
			if (!menuBar()->isVisible()){
				showMenuBar();
				e->accept();
			} else {
				hideMenuBar();
				e->ignore();
			}
		}
		if (e->key() == Qt::Key_Alt) {
			skipNextAltRelease = false;
		}
	}

	void MainInterface::mouseDoubleClickEvent(QMouseEvent* e) {
		if (e->button() == Qt::LeftButton) {
			toggleFullscreen();
			e->accept();
		}
	}

	void MainInterface::mouseReleaseEvent(QMouseEvent* e) {
		if (e->button() == Qt::ForwardButton) loadNextImage();
		if (e->button() == Qt::BackButton) loadPreviousImage();
		if (menuBar()->isVisible()) {
			hideMenuBar();
		}
	}

	void MainInterface::mouseMoveEvent(QMouseEvent* e) {
		if (isFullScreen()) {
			showMouse();
			enableAutomaticMouseHide();
		}
		e->ignore();
	}

	void MainInterface::changeEvent(QEvent* e) {
		if (e->type() == QEvent::WindowStateChange) {
			if (!isMinimized() && !isFullScreen()) {
				settings->setValue("maximized", isMaximized());
				saveSizeAction->setEnabled(!isMaximized());
			} else if (isFullScreen()) {
				QWindowStateChangeEvent* windowStateChangeEvent = static_cast<QWindowStateChangeEvent*>(e);
				settings->setValue("maximized", bool(windowStateChangeEvent->oldState() & Qt::WindowMaximized));
				saveSizeAction->setEnabled(false);
			}
		}
	}

	void MainInterface::wheelEvent(QWheelEvent * e) {
		if (e->modifiers() == Qt::ShiftModifier) {
			if (e->delta() > 0) {
				userRotation -= 10;
				imageView->rotateBy(-10);
			} else if (e->delta() < 0) {
				userRotation += 10;
				imageView->rotateBy(10);
			}
		}
	}

	//=============================================================================== PRIVATE ===============================================================================\\

	void MainInterface::initialize() {
		setAcceptDrops(true);
		qRegisterMetaType<Image>("Image");
		QObject::connect(this, SIGNAL(readImageFinished(Image)), this, SLOT(reactToReadImageCompletion(Image)));
		setWindowTitle(programTitle);

		imageView = new hb::ImageView(this);
		imageView->setShowInterfaceOutline(false);
		imageView->setUseSmoothTransform(false);
		imageView->installEventFilter(this);
		imageView->setExternalPostPaintFunction(this, &MainInterface::infoPaintFunction);
		imageView->setInterfaceBackgroundColor(Qt::black);
		imageView->setPreventMagnificationInDefaultZoom(true);
		imageView->setUseGpu(true);
		imageView->setPostResizeSharpening(false, settings->value("sharpeningStrength", 0.5).toDouble(), settings->value("sharpeningRadius", 0.5).toDouble());
		setCentralWidget(imageView);

		slideshowDialog = new SlideshowDialog(settings, this);
		slideshowDialog->setWindowModality(Qt::WindowModal);
		QObject::connect(slideshowDialog, &SlideshowDialog::accepted, this, &MainInterface::startSlideshow);
		QObject::connect(slideshowDialog, SIGNAL(finished(int)), this, SLOT(enableAutomaticMouseHide()));

		sharpeningDialog = new SharpeningDialog(settings, this);
		QObject::connect(sharpeningDialog, SIGNAL(sharpeningParametersChanged()), this, SLOT(updateSharpening()));
		QObject::connect(sharpeningDialog, SIGNAL(finished(int)), this, SLOT(enableAutomaticMouseHide()));

		hotkeyDialog = new HotkeyDialog(settings, this);
		hotkeyDialog->setWindowModality(Qt::WindowModal);
		QObject::connect(hotkeyDialog, SIGNAL(finished(int)), this, SLOT(enableAutomaticMouseHide()));
		QObject::connect(hotkeyDialog, SIGNAL(finished(int)), this, SLOT(updateCustomHotkeys()));

		aboutDialog = new AboutDialog(settings, this);
		aboutDialog->setWindowModality(Qt::WindowModal);

		QObject::connect(menuBar(), SIGNAL(triggered(QAction*)), this, SLOT(hideMenuBar(QAction*)));
		fileMenu = menuBar()->addMenu(tr("&File"));
		viewMenu = menuBar()->addMenu(tr("&View"));
		zoomMenu = menuBar()->addMenu(tr("&Zoom"));
		rotationMenu = menuBar()->addMenu(tr("&Rotation"));
		sharpeningMenu = menuBar()->addMenu(tr("&Sharpening"));
		slideshowMenu = menuBar()->addMenu(tr("S&lideshow"));

		applicationMenu = menuBar()->addMenu(tr("&Application"));
#ifdef Q_OS_WIN
		QObject::connect(applicationMenu, SIGNAL(aboutToShow()), this, SLOT(populateApplicationMenu()));
		installAction = new QAction(tr("&Install"), this);
		QObject::connect(installAction, SIGNAL(triggered()), this, SLOT(runInstaller()));
		uninstallAction = new QAction(tr("&Uninstall"), this);
		QObject::connect(uninstallAction, SIGNAL(triggered()), this, SLOT(runUninstaller()));
#endif

		aboutAction = new QAction(tr("&About"), this);
		aboutAction->setShortcut(Qt::Key_F1);
		aboutAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(aboutAction, SIGNAL(triggered()), aboutDialog, SLOT(show()));
		applicationMenu->addAction(aboutAction);
		addAction(aboutAction);

		openAction = new QAction(tr("&Open File"), this);
		openAction->setShortcut(QKeySequence::Open);
		openAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(openAction, SIGNAL(triggered()), this, SLOT(openDialog()));
		fileMenu->addAction(openAction);
		addAction(openAction);

		refreshAction = new QAction(tr("&Refresh"), this);
		refreshAction->setEnabled(false);
		refreshAction->setShortcut(Qt::Key_F5);
		refreshAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(refreshAction, SIGNAL(triggered()), this, SLOT(refresh()));
		fileMenu->addAction(refreshAction);
		addAction(refreshAction);

		fileMenu->addSeparator();

		fileActionAction = new QAction(tr("&Enable File Action Hotkeys"), this);
		fileActionAction->setCheckable(true);
		fileActionAction->setChecked(false);
		fileActionAction->setShortcut(Qt::CTRL + Qt::Key_H);
		fileActionAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(fileActionAction, SIGNAL(triggered(bool)), hotkeyDialog, SLOT(setHotkeysEnabled(bool)));
		fileMenu->addAction(fileActionAction);
		addAction(fileActionAction);

		hotkeyOptionsAction = new QAction(tr("&Hotkey Options..."), this);
		hotkeyOptionsAction->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_O);
		hotkeyOptionsAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(hotkeyOptionsAction, SIGNAL(triggered()), this, SLOT(showHotkeyDialog()));
		fileMenu->addAction(hotkeyOptionsAction);
		addAction(hotkeyOptionsAction);

		fileMenu->addSeparator();

		includePartiallySupportedFilesAction = new QAction(tr("&Include Preview-Only Files in Directory List"), this);
		includePartiallySupportedFilesAction->setCheckable(true);
		includePartiallySupportedFilesAction->setChecked(true);
		includePartiallySupportedFilesAction->setShortcut(Qt::CTRL + Qt::Key_P);
		includePartiallySupportedFilesAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(includePartiallySupportedFilesAction, SIGNAL(triggered(bool)), this, SLOT(togglePreviewOnlyFiles(bool)));
		fileMenu->addAction(includePartiallySupportedFilesAction);
		addAction(includePartiallySupportedFilesAction);

		fileMenu->addSeparator();

		resetSettingsAction = new QAction(tr("&Reset All Settings to Default"), this);
		QObject::connect(resetSettingsAction, SIGNAL(triggered()), this, SLOT(resetSettings()));
		fileMenu->addAction(resetSettingsAction);

		fileMenu->addSeparator();

		quitAction = new QAction(tr("&Quit"), this);
		quitAction->setShortcut(Qt::CTRL + Qt::Key_Q);
		quitAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(quitAction, SIGNAL(triggered()), this, SLOT(quit()));
		fileMenu->addAction(quitAction);
		//so shortcuts also work when menu bar is not visible
		addAction(quitAction);

		showInfoAction = new QAction(tr("Show &Info Overlay"), this);
		showInfoAction->setCheckable(true);
		showInfoAction->setChecked(false);
		showInfoAction->setShortcut(Qt::Key_I);
		showInfoAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(showInfoAction, SIGNAL(triggered(bool)), this, SLOT(toggleInfoOverlay(bool)));
		viewMenu->addAction(showInfoAction);
		addAction(showInfoAction);

		zoomLevelAction = new QAction(tr("Show &Zoom Level Overlay"), this);
		zoomLevelAction->setCheckable(true);
		zoomLevelAction->setChecked(false);
		zoomLevelAction->setShortcut(Qt::Key_Z);
		zoomLevelAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(zoomLevelAction, SIGNAL(triggered(bool)), this, SLOT(toggleZoomLevelOverlay(bool)));
		viewMenu->addAction(zoomLevelAction);
		addAction(zoomLevelAction);

		backgroundColorMenu = viewMenu->addMenu(tr("&Background Colour"));

		backgroundColorBlackAction = new QAction(tr("&Black"), this);
		backgroundColorBlackAction->setCheckable(true);
		backgroundColorBlackAction->setChecked(false);
		backgroundColorBlackAction->setShortcut(Qt::CTRL + Qt::Key_B);
		backgroundColorBlackAction->setShortcutContext(Qt::ApplicationShortcut);
		backgroundColorMenu->addAction(backgroundColorBlackAction);
		addAction(backgroundColorBlackAction);

		backgroundColorGrayAction = new QAction(tr("Dark &Grey"), this);
		backgroundColorGrayAction->setCheckable(true);
		backgroundColorGrayAction->setChecked(false);
		backgroundColorGrayAction->setShortcut(Qt::CTRL + Qt::Key_G);
		backgroundColorGrayAction->setShortcutContext(Qt::ApplicationShortcut);
		backgroundColorMenu->addAction(backgroundColorGrayAction);
		addAction(backgroundColorGrayAction);

		backgroundColorWhiteAction = new QAction(tr("&White"), this);
		backgroundColorWhiteAction->setCheckable(true);
		backgroundColorWhiteAction->setChecked(false);
		backgroundColorWhiteAction->setShortcut(Qt::CTRL + Qt::Key_W);
		backgroundColorWhiteAction->setShortcutContext(Qt::ApplicationShortcut);
		backgroundColorMenu->addAction(backgroundColorWhiteAction);
		addAction(backgroundColorWhiteAction);

		backgroundColorCustomAction = new QAction(tr("&Custom"), this);
		backgroundColorCustomAction->setCheckable(true);
		backgroundColorCustomAction->setChecked(false);
		backgroundColorCustomAction->setShortcut(Qt::CTRL + Qt::Key_C);
		backgroundColorCustomAction->setShortcutContext(Qt::ApplicationShortcut);
		backgroundColorMenu->addAction(backgroundColorCustomAction);
		addAction(backgroundColorCustomAction);

		backgroundColorActionGroup = new QActionGroup(this);
		backgroundColorActionGroup->addAction(backgroundColorBlackAction);
		backgroundColorActionGroup->addAction(backgroundColorGrayAction);
		backgroundColorActionGroup->addAction(backgroundColorWhiteAction);
		backgroundColorActionGroup->addAction(backgroundColorCustomAction);
		backgroundColorActionGroup->setExclusive(true);
		QObject::connect(backgroundColorActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeBackgroundColor(QAction*)));

		viewMenu->addSeparator();

		fullscreenAction = new QAction(tr("&Fullscreen"), this);
		fullscreenAction->setCheckable(true);
		fullscreenAction->setChecked(false);
		fullscreenAction->setShortcut(Qt::Key_F);
		fullscreenAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(fullscreenAction, SIGNAL(triggered(bool)), this, SLOT(toggleFullscreen()));
		viewMenu->addAction(fullscreenAction);
		addAction(fullscreenAction);

		viewMenu->addSeparator();

		menuBarAutoHideAction = new QAction(tr("&Always Show Menu Bar"), this);
		menuBarAutoHideAction->setCheckable(true);
		menuBarAutoHideAction->setChecked(false);
		menuBarAutoHideAction->setShortcut(Qt::Key_M);
		menuBarAutoHideAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(menuBarAutoHideAction, SIGNAL(triggered(bool)), this, SLOT(toggleMenuBarAutoHide(bool)));
		viewMenu->addAction(menuBarAutoHideAction);
		addAction(menuBarAutoHideAction);

		viewMenu->addSeparator();

		gpuAction = new QAction(tr("&Use GPU Acceleration"), this);
		gpuAction->setCheckable(true);
		gpuAction->setChecked(imageView->getUseGpu());
		QObject::connect(gpuAction, SIGNAL(triggered(bool)), this, SLOT(toggleGpu(bool)));
		viewMenu->addAction(gpuAction);

		viewMenu->addSeparator();

		saveSizeAction = new QAction(tr("&Save Current Window Size and Position as Default"), this);
		QObject::connect(saveSizeAction, SIGNAL(triggered(bool)), this, SLOT(saveWindowSize()));
		viewMenu->addAction(saveSizeAction);

		enlargementAction = new QAction(tr("&Enlarge Smaller Images to Fit Window"), this);
		enlargementAction->setCheckable(true);
		enlargementAction->setChecked(false);
		enlargementAction->setShortcut(Qt::CTRL + Qt::Key_U);
		enlargementAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(enlargementAction, SIGNAL(triggered(bool)), this, SLOT(toggleSmallImageUpscaling(bool)));
		zoomMenu->addAction(enlargementAction);
		addAction(enlargementAction);

		smoothingAction = new QAction(tr("Use &Smooth Interpolation when Enlarging"), this);
		smoothingAction->setCheckable(true);
		smoothingAction->setChecked(false);
		smoothingAction->setShortcut(Qt::CTRL + Qt::Key_S);
		smoothingAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(smoothingAction, SIGNAL(triggered(bool)), this, SLOT(toggleEnglargmentInterpolationMethod(bool)));
		zoomMenu->addAction(smoothingAction);
		addAction(smoothingAction);

		zoomMenu->addSeparator();

		zoomToFitAction = new QAction(tr("Zoom to &Fit"), this);
		zoomToFitAction->setShortcut(Qt::CTRL + Qt::Key_0);
		zoomToFitAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(zoomToFitAction, SIGNAL(triggered(bool)), imageView, SLOT(resetZoom()));
		zoomMenu->addAction(zoomToFitAction);
		addAction(zoomToFitAction);

		zoomTo100Action = new QAction(tr("Zoom to &100%"), this);
		zoomTo100Action->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_0);
		zoomTo100Action->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(zoomTo100Action, SIGNAL(triggered(bool)), this, SLOT(zoomTo100()));
		zoomMenu->addAction(zoomTo100Action);
		addAction(zoomTo100Action);

		rotateLeftAction = new QAction(tr("Rotate View &Left"), this);
		rotateLeftAction->setShortcut(Qt::CTRL + Qt::Key_Left);
		rotateLeftAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(rotateLeftAction, SIGNAL(triggered(bool)), this, SLOT(rotateLeft()));
		rotationMenu->addAction(rotateLeftAction);
		addAction(rotateLeftAction);

		rotateRightAction = new QAction(tr("Rotate &View Right"), this);
		rotateRightAction->setShortcut(Qt::CTRL + Qt::Key_Right);
		rotateRightAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(rotateRightAction, SIGNAL(triggered(bool)), this, SLOT(rotateRight()));
		rotationMenu->addAction(rotateRightAction);
		addAction(rotateRightAction);

		resetRotationAction = new QAction(tr("&Reset Rotation"), this);
		resetRotationAction->setShortcut(Qt::SHIFT + Qt::Key_Escape);
		resetRotationAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(resetRotationAction, SIGNAL(triggered(bool)), this, SLOT(resetRotation()));
		rotationMenu->addAction(resetRotationAction);
		addAction(resetRotationAction);

		rotationMenu->addSeparator();

		autoRotationAction = new QAction(tr("&Automatic EXIF-based rotation"), this);
		autoRotationAction->setCheckable(true);
		autoRotationAction->setChecked(true);
		autoRotationAction->setShortcut(Qt::CTRL + Qt::Key_R);
		autoRotationAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(autoRotationAction, SIGNAL(triggered(bool)), this, SLOT(toggleAutoRotation(bool)));
		rotationMenu->addAction(autoRotationAction);
		addAction(autoRotationAction);

		sharpeningAction = new QAction(tr("&Sharpen Images After Downsampling"), this);
		sharpeningAction->setCheckable(true);
		sharpeningAction->setChecked(false);
		sharpeningAction->setShortcut(Qt::CTRL + Qt::Key_E);
		sharpeningAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(sharpeningAction, SIGNAL(triggered(bool)), this, SLOT(toggleSharpening(bool)));
		sharpeningMenu->addAction(sharpeningAction);
		addAction(sharpeningAction);

		sharpeningOptionsAction = new QAction(tr("Sharpening &Options..."), this);
		sharpeningOptionsAction->setShortcut(Qt::Key_O);
		sharpeningOptionsAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(sharpeningOptionsAction, SIGNAL(triggered(bool)), this, SLOT(showSharpeningOptions()));
		sharpeningMenu->addAction(sharpeningOptionsAction);
		addAction(sharpeningOptionsAction);

		slideshowAction = new QAction(tr("&Start Slideshow"), this);
		slideshowAction->setEnabled(false);
		slideshowAction->setShortcut(Qt::CTRL + Qt::Key_Space);
		slideshowAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(slideshowAction, SIGNAL(triggered()), this, SLOT(toggleSlideshow()));
		slideshowMenu->addAction(slideshowAction);
		addAction(slideshowAction);

		slideshowNoDialogAction = new QAction(this);
		slideshowNoDialogAction->setEnabled(false);
		slideshowNoDialogAction->setShortcut(Qt::Key_Space);
		slideshowNoDialogAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(slideshowNoDialogAction, SIGNAL(triggered()), this, SLOT(toggleSlideshowNoDialog()));
		addAction(slideshowNoDialogAction);

		customAction1 = new QAction(this);
		customAction1->setEnabled(hotkeyDialog->getHotkey1Enabled() && hotkeyDialog->getHotkeysEnabled());
		customAction1->setShortcut(hotkeyDialog->getKeySequence1());
		customAction1->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(customAction1, SIGNAL(triggered()), this, SLOT(triggerCustomAction1()));
		addAction(customAction1);

		customAction2 = new QAction(this);
		customAction2->setEnabled(hotkeyDialog->getHotkey2Enabled() && hotkeyDialog->getHotkeysEnabled());
		customAction2->setShortcut(hotkeyDialog->getKeySequence2());
		customAction2->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(customAction2, SIGNAL(triggered()), this, SLOT(triggerCustomAction2()));
		addAction(customAction2);

		//mouse hide timer in fullscreen
		mouseHideTimer = new QTimer(this);
		QObject::connect(mouseHideTimer, SIGNAL(timeout()), this, SLOT(hideMouse()));

		//timer for thread cleanup
		threadCleanUpTimer = new QTimer(this);
		threadCleanUpTimer->setSingleShot(true);
		QObject::connect(threadCleanUpTimer, SIGNAL(timeout()), this, SLOT(cleanUpThreads()));

		//timer for the slideshow
		slideshowTimer = new QTimer(this);
		QObject::connect(slideshowTimer, SIGNAL(timeout()), this, SLOT(nextSlide()));

		//load settings
		loadSettings();
		if (settings->contains("windowPosition")) {
			move(settings->value("windowPosition", QPoint(10, 10)).toPoint());
		}
		if (settings->contains("windowSize")) {
			resize(settings->value("windowSize", QSize(900, 600)).toSize());
		}
		if (settings->value("maximized", false).toBool()) {
			showMaximized();
		}
	}

	std::shared_future<Image>& MainInterface::currentThread() {
		return threads[currentThreadName];
	}

	bool MainInterface::exifIsRequired() const {
		return showInfoAction->isChecked() || autoRotationAction->isChecked();
	}

	Image MainInterface::readImage(QString path, bool emitSignals) {
		try {
			cv::Mat image;
			bool isPreviewImage = false;
			Image result;
			std::shared_ptr<ExifData> exifData;
			//for the images we know are not supported by opencv do not attempt to read them with opencv
			bool forcePreview = partiallySupportedExtensions.contains(QString("*.") + QFileInfo(path).suffix().toLower());
			if (!utility::isCharCompatible(path)) {
				std::shared_ptr<std::vector<char>> buffer = utility::readFileIntoBuffer(path);
				if (buffer->empty()) {
					if (emitSignals) emit(readImageFinished(Image()));
					return Image();
				}
				if(!forcePreview) image = cv::imdecode(*buffer, cv::IMREAD_UNCHANGED);
				exifData = std::shared_ptr<ExifData>(new ExifData(buffer));
			} else {
				if (!forcePreview) image = cv::imread(path.toStdString(), cv::IMREAD_UNCHANGED);
				exifData = std::shared_ptr<ExifData>(new ExifData(path, !exifIsRequired() && image.data));
			}
			if (!image.data) {  
				exifData->join();
				if (exifData->hasPreviewImage()) {
					image = exifData->largestReadablePreviewImage();
					isPreviewImage = true;
				}
			}
			if (image.data) {
				QObject::connect(exifData.get(), SIGNAL(loadingFinished(ExifData*)), this, SLOT(reactToExifLoadingCompletion(ExifData*)));
				//convert format
				if (image.channels() == 3) {
					cv::cvtColor(image, image, CV_BGR2RGB);
				}
				if (image.depth() == CV_16U) {
					image.convertTo(image, CV_8U, 1.0 / 256.0);
				} else if (image.depth() == CV_32F) {
					image.convertTo(image, CV_8U, 256.0);
				}
				result = Image(image, exifData, isPreviewImage);
			}
			if (emitSignals) emit(readImageFinished(result));
			return result;
		} catch (...) {
			return Image();
		}
	}

	void MainInterface::loadNextImage() {
		if (loading) return;
		std::unique_lock<std::mutex> lock(threadDeletionMutex);
		loading = true;

		if (filesInDirectory.size() != 0) {
			currentFileIndex = nextFileIndex();
			currentThreadName = filesInDirectory[currentFileIndex];
			if (threads.find(filesInDirectory[currentFileIndex]) == threads.end()) {
				loading = false;
				return;
			}
			waitForThreadToFinish(currentThread());
			//calling this function although the exif might not be set to deferred loading is no problem (it checks internally)
			if (exifIsRequired() && currentThread().get().isValid()) currentThread().get().exif()->startLoading();
			image = currentThread().get();

			currentFileInfo = QFileInfo(getFullImagePath(currentFileIndex));
			//start loading next image
			if (threads.find(filesInDirectory[nextFileIndex()]) == threads.end()) {
				threads[filesInDirectory[nextFileIndex()]] = std::async(std::launch::async,
																						  &MainInterface::readImage,
																						  this,
																						  getFullImagePath(nextFileIndex()),
																						  false);
			}
			lock.unlock();
			displayImageIfOk();
		} else {
			lock.unlock();
		}
		loading = false;
		cleanUpThreads();
	}

	void MainInterface::loadPreviousImage() {
		if (loading) return;
		std::unique_lock<std::mutex> lock(threadDeletionMutex);
		loading = true;
		if (filesInDirectory.size() != 0) {
			currentFileIndex = previousFileIndex();
			currentThreadName = filesInDirectory[currentFileIndex];
			if (threads.find(filesInDirectory[currentFileIndex]) == threads.end()) {
				loading = false;
				return;
			}
			waitForThreadToFinish(currentThread());
			//calling this function although the exif might not be set to deferred loading is no problem (it checks internally)
			if (exifIsRequired() && currentThread().get().isValid()) currentThread().get().exif()->startLoading();
			image = currentThread().get();
			currentFileInfo = QFileInfo(getFullImagePath(currentFileIndex));
			//start loading previous image
			if (threads.find(filesInDirectory[previousFileIndex()]) == threads.end()) {
				threads[filesInDirectory[previousFileIndex()]] = std::async(std::launch::async,
																							  &MainInterface::readImage,
																							  this,
																							  getFullImagePath(previousFileIndex()),
																							  false);
			}
			lock.unlock();
			displayImageIfOk();
		} else {
			lock.unlock();
		}
		loading = false;
		cleanUpThreads();
	}

	void MainInterface::clearThreads() {
		for (std::map<QString, std::shared_future<Image>>::iterator it = threads.begin(); it != threads.end(); ++it) {
			waitForThreadToFinish(it->second, false);
		}
		threads.clear();
	}

	size_t MainInterface::nextFileIndex() const {
		if (filesInDirectory.size() <= 0) return -1;
		return (currentFileIndex + 1) % filesInDirectory.size();
	}

	size_t MainInterface::previousFileIndex() const {
		if (filesInDirectory.size() <= 0) return -1;
		if (currentFileIndex <= 0) return filesInDirectory.size() - 1;
		return (currentFileIndex - 1) % filesInDirectory.size();
	}

	void MainInterface::removeCurrentImageFromList(bool includeSidecarFiles, bool onlyXmp) {
		std::unique_lock<std::mutex> lock(threadDeletionMutex);

		if (filesInDirectory.size() != 0) {
			QFileInfo imageInfo = QFileInfo(filesInDirectory[currentFileIndex]);
			QString baseName = imageInfo.baseName();
			QString extension = imageInfo.suffix().toLower();
			//remove current file from directory list
			filesInDirectory.remove(currentFileIndex);

			//remove sidecar files from directory list
			if (includeSidecarFiles) {
				for (int i = 0; i < filesInDirectory.size(); ++i) {
					QFileInfo fileInfo(filesInDirectory[i]);
					if (fileInfo.baseName() == baseName && (!onlyXmp || (fileInfo.suffix().toLower() == "xmp" && supportedRawFormats.contains(extension)))) {
						filesInDirectory.remove(i);
						//correct the index shift
						if (i < currentFileIndex) --currentFileIndex;
						--i;
					}
				}
			}

			//if there are no images left, quit
			if (filesInDirectory.size() <= 0) {
				currentFileIndex = -1;
				lock.unlock();
				cleanUpThreads();
				reset();
				return;
			}
			//update current file index and related variables
			if (currentFileIndex >= filesInDirectory.size()) currentFileIndex = filesInDirectory.size() - 1;
			currentThreadName = filesInDirectory[currentFileIndex];

			//load the image that is now the current one
			if (threads.find(filesInDirectory[currentFileIndex]) == threads.end()) {
				threads[filesInDirectory[currentFileIndex]] = std::async(std::launch::async,
																						   &MainInterface::readImage,
																						   this,
																						   getFullImagePath(currentFileIndex),
																						   false);
			}
			waitForThreadToFinish(currentThread());
			//calling this function although the exif might not be set to deferred loading is no problem (it checks internally)
			if (exifIsRequired() && currentThread().get().isValid()) currentThread().get().exif()->startLoading();
			image = currentThread().get();
			currentFileInfo = QFileInfo(getFullImagePath(currentFileIndex));

			//start loading next and previous image
			if (threads.find(filesInDirectory[nextFileIndex()]) == threads.end()) {
				threads[filesInDirectory[nextFileIndex()]] = std::async(std::launch::async,
																						  &MainInterface::readImage,
																						  this,
																						  getFullImagePath(nextFileIndex()),
																						  false);
			}
			if (threads.find(filesInDirectory[previousFileIndex()]) == threads.end()) {
				threads[filesInDirectory[previousFileIndex()]] = std::async(std::launch::async,
																							  &MainInterface::readImage,
																							  this,
																							  getFullImagePath(previousFileIndex()),
																							  false);
			}
			lock.unlock();
			displayImageIfOk();
		} else {
			lock.unlock();
		}
		cleanUpThreads();
	}

	void MainInterface::reset() {
		imageView->resetImage();
		image = sv::Image();
		filesInDirectory.clear();
		setWindowTitle(programTitle);
	}

	QString MainInterface::getFullImagePath(size_t index) const {
		return currentDirectory.absoluteFilePath(filesInDirectory[index]);
	}

	void MainInterface::loadImage(QString path) {
		if (loading) return;
		std::unique_lock<std::mutex> lock(threadDeletionMutex);
		loading = true;
		//find the path in the current directory listing
		QFileInfo fileInfo = QFileInfo(QDir::cleanPath(path));
		QDir directory = fileInfo.absoluteDir();
		QString filename = fileInfo.fileName();
		//always scan directory; uncomment to scan only if different directory
		//if (directory != currentDirectory || noCurrentDir) {
			currentDirectory = directory;
			noCurrentDir = false;
			QStringList filters = supportedExtensions;
			filters << "*.bmp" << "*.dib" << "*.jpeg" << "*.jpg" << "*.jpe" << "*.jpeg" << "*.jp2" << "*.png" << "*.webp" << "*.pbm" << "*.pgm" << "*.ppm" << "*.sr" << "*.ras" << "*.tiff" << "*.tif";
			if (includePartiallySupportedFilesAction->isChecked()) {
				filters.append(partiallySupportedExtensions);
			}
			QStringList contents = directory.entryList(filters, QDir::Files);
			QCollator collator;
			collator.setNumericMode(true);
			std::sort(contents.begin(), contents.end(), collator);
			filesInDirectory = contents.toVector();
		//}
		if (filesInDirectory.size() == 0 || currentFileIndex < 0 || currentFileIndex >= filesInDirectory.size() || filesInDirectory.at(currentFileIndex) != filename) {
			currentFileIndex = filesInDirectory.indexOf(filename);
		}
		currentThreadName = filename;
		currentFileInfo = fileInfo;
		clearThreads();
		threads[filename] = std::async(std::launch::async, &MainInterface::readImage, this, path, true);
		setWindowTitle(windowTitle() + QString(tr(" - Loading...")));
		statusHint = tr("Loading...");
		imageView->update();
	}

	void MainInterface::loadImages(QStringList paths) {
		if (loading) return;
		std::unique_lock<std::mutex> lock(threadDeletionMutex);
		loading = true;
		
		QString path = paths.first();
		//find the path in the current directory listing
		QFileInfo fileInfo = QFileInfo(QDir::cleanPath(path));
		QDir directory = fileInfo.absoluteDir();
		QString filename = fileInfo.fileName();
		currentDirectory = directory;
		noCurrentDir = false;
		//remove all images that are not in the same directory
		QMutableListIterator<QString> it(paths);
		while (it.hasNext()) {
			if (QFileInfo(it.next()).absoluteDir() != currentDirectory) {
				it.remove();
			}
		}
		filesInDirectory.resize(paths.size());
		for (int i = 0; i < paths.size(); ++i) {
			filesInDirectory[i] = QFileInfo(QDir::cleanPath(paths[i])).fileName();
		}

		if (filesInDirectory.size() == 0 || currentFileIndex < 0 || currentFileIndex >= filesInDirectory.size() || filesInDirectory.at(currentFileIndex) != filename) {
			currentFileIndex = filesInDirectory.indexOf(filename);
		}
		currentThreadName = filename;
		currentFileInfo = fileInfo;
		clearThreads();
		threads[filename] = std::async(std::launch::async, &MainInterface::readImage, this, path, true);
		setWindowTitle(windowTitle() + QString(tr(" - Loading...")));
		statusHint = tr("Loading...");
		imageView->update();
	}

	void MainInterface::displayImageIfOk() {
		if (image.isValid()) {
			currentImageUnreadable = false;
			imageView->setImage(image.mat());
			if (autoRotationAction->isChecked()) {
				autoRotateImage();
			}
		} else {
			currentImageUnreadable = true;
			imageView->resetImage();
		}
		setWindowTitle(QString("%1%5 - %2 - %3 of %4").arg(currentFileInfo.fileName(),
															   programTitle).arg(currentFileIndex + 1).arg(filesInDirectory.size()).arg(image.isPreviewImage() ? " [Preview]" : ""));
	}

	void MainInterface::autoRotateImage() {
		if (image.isValid()) {
			image.exif()->join();
			if (image.exif()->hasExif()) {
				long orientation = image.exif()->orientation();
				imageView->setRotation(userRotation);
				orientation = (orientation + 1) / 2;
				if (orientation == 3) {
					imageView->rotateBy(90);
				} else if (orientation == 4) {
					imageView->rotateBy(-90);
				} else if (orientation == 2) {
					imageView->rotateBy(180);
				}
			}
		}
	}

	void MainInterface::enterFullscreen() {
		//imageView->setInterfaceBackgroundColor(Qt::black);
		showFullScreen();
		hideMenuBar();
		enableAutomaticMouseHide();
		fullscreenAction->setChecked(true);
	}

	void MainInterface::exitFullscreen() {
		//QPalette palette = qApp->palette();
		//imageView->setInterfaceBackgroundColor(palette.base().color());
		if (settings->value("maximized", false).toBool()) {
			showMaximized();
		} else {
			showNormal();
		}
		if (menuBarAutoHideAction->isChecked()) showMenuBar();
		disableAutomaticMouseHide();
		fullscreenAction->setChecked(false);
	}

	void MainInterface::infoPaintFunction(QPainter& canvas) {
		canvas.setRenderHint(QPainter::Antialiasing, true);
		QPen textPen(Qt::black);
		canvas.setPen(textPen);
		canvas.setBrush(Qt::NoBrush);
		QFont font;
		font.setPointSize(fontSize);
		canvas.setFont(font);
		QColor base = Qt::white;
		base.setAlpha(200);
		canvas.setBackground(base);
		canvas.setBackgroundMode(Qt::OpaqueMode);
		QFontMetrics metrics(font);
		if (currentImageUnreadable && statusHint.isEmpty()) {
			QString message = tr("This file could not be read:");
			canvas.drawText(QPoint((canvas.device()->width() - metrics.width(message)) / 2.0, canvas.device()->height() / 2.0 - 0.5*lineSpacing),
							message);
			canvas.drawText(QPoint((canvas.device()->width() - metrics.width(currentFileInfo.fileName())) / 2.0,
								   canvas.device()->height() / 2.0 + 0.5*lineSpacing + metrics.height()),
							currentFileInfo.fileName());
		}
		if (!statusHint.isEmpty()) {
			canvas.drawText(QPoint((canvas.device()->width() - metrics.width(statusHint)) / 2.0, canvas.device()->height() / 2.0 + 0.5*metrics.height()),
							statusHint);
		}
		if (showInfoAction->isChecked() && imageView->getImageAssigned()) {
			//draw current filename
			canvas.drawText(QPoint(30, 30 + metrics.height()),
							currentFileInfo.fileName());
			int sizeAndResolutionTopOffset = 30 + lineSpacing + 2 * metrics.height();
			if (image.isValid()) {
				QString resolution = QString::fromWCharArray(L"%1\u2006x\u2006%2").arg(image.mat().cols).arg(image.mat().rows);
				canvas.drawText(QPoint(30, sizeAndResolutionTopOffset),
								QString::fromWCharArray(L"%1, %2\u2006Mb").arg(resolution).arg(currentFileInfo.size() / 1048576.0, 0, 'f', 2));
				if (image.exif()->isReady()) {
					if (image.exif()->hasExif()) {
						//get camera model, speed, aperture and ISO
						QString cameraModel = image.exif()->cameraModel();
						QString lensModel = image.exif()->lensModel();
						QString aperture = image.exif()->fNumber();
						QString speed = image.exif()->exposureTime();
						QString focalLength = image.exif()->focalLength();
						QString equivalentFocalLength = image.exif()->focalLength35mmEquivalent();
						QString exposureBias = image.exif()->exposureBias();
						QString iso = image.exif()->iso();
						QString captureDate = image.exif()->captureDate();
						//calculate the v coordinates for the lines
						int heightOfOneLine = lineSpacing + metrics.height();
						int topOffset = 30 + 2 * lineSpacing + 3 * metrics.height();

						//draw the EXIF text (note \u2005 is a sixth of a quad)
						if (!cameraModel.isEmpty()) {
							canvas.drawText(QPoint(30, topOffset),
											cameraModel);
							topOffset += heightOfOneLine;
						}
						if (!lensModel.isEmpty()) {
							canvas.drawText(QPoint(30, topOffset),
											lensModel);
							topOffset += heightOfOneLine;
						}
						if (!focalLength.isEmpty() && !equivalentFocalLength.isEmpty()) {
							canvas.drawText(QPoint(30, topOffset),
											QString::fromWCharArray(L"%1\u2006mm (\u2261 %2\u2006mm)").arg(focalLength).arg(equivalentFocalLength));
							topOffset += heightOfOneLine;
						} else if (!focalLength.isEmpty()) {
							canvas.drawText(QPoint(30, topOffset),
											QString::fromWCharArray(L"%1\u2006mm").arg(focalLength));
							topOffset += heightOfOneLine;
						} else if (!equivalentFocalLength.isEmpty()) {
							canvas.drawText(QPoint(30, topOffset),
											QString::fromWCharArray(L"%1\u2006mm (35\u2006mm equivalent)").arg(equivalentFocalLength));
							topOffset += heightOfOneLine;
						}
						if (!speed.isEmpty() && !aperture.isEmpty()) {
							canvas.drawText(QPoint(30, topOffset),
											QString::fromWCharArray(L"%1\u2006s @ f/%2").arg(speed).arg(aperture));
							topOffset += heightOfOneLine;
						} else if (!speed.isEmpty()) {
							canvas.drawText(QPoint(30, topOffset),
											QString::fromWCharArray(L"%1\u2006s").arg(speed));
							topOffset += heightOfOneLine;
						} else if (!aperture.isEmpty()) {
							canvas.drawText(QPoint(30, topOffset),
											QString("f/%1").arg(aperture));
							topOffset += heightOfOneLine;
						}
						if (!iso.isEmpty() && !exposureBias.isEmpty()) {
							canvas.drawText(QPoint(30, topOffset),
											QString::fromWCharArray(L"ISO\u2006%1, %2\u2006EV").arg(iso).arg(exposureBias));
							topOffset += heightOfOneLine;
						} else if (!iso.isEmpty()) {
							canvas.drawText(QPoint(30, topOffset),
											QString::fromWCharArray(L"ISO\u2006%1").arg(iso));
							topOffset += heightOfOneLine;
						} else if (!exposureBias.isEmpty()) {
							canvas.drawText(QPoint(30, topOffset),
											QString::fromWCharArray(L"%1\u2006EV").arg(exposureBias));
							topOffset += heightOfOneLine;
						}
						if (!captureDate.isEmpty()) {
							canvas.drawText(QPoint(30, topOffset),
											QString("%1").arg(captureDate));
							topOffset += heightOfOneLine;
						}
						if (image.isPreviewImage()) {
							canvas.drawText(QPoint(30, topOffset), "[Preview Image]");
						}
					}
				} else {
					canvas.drawText(QPoint(30, 30 + 2 * lineSpacing + 3 * metrics.height()),
									tr("Loading EXIF..."));
				}
			}
		}
		if (zoomLevelAction->isChecked() && imageView->getImageAssigned()) {
			QString message = QString::number(imageView->getCurrentPreviewScalingFactor() * 100, 'f', 1).append("%");
			canvas.drawText(QPoint(30, canvas.device()->height() - 30), message);
		}
	}

	bool MainInterface::applicationIsInstalled() {
#ifdef Q_OS_WIN
		QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE", QSettings::NativeFormat);
		return registry.contains("Microsoft/Windows/CurrentVersion/Uninstall/AcuteViewer/UninstallString");
#else
		return false;
#endif

	}

	void MainInterface::changeFontSizeBy(int value) {
		if (showInfoAction->isChecked() || zoomLevelAction->isChecked()) {
			if (fontSize + value >= 1) {
				fontSize += value;
				settings->setValue("fontSize", fontSize);
				imageView->update();
			}
		}
	}

	void MainInterface::changeLineSpacingBy(int value) {
		if (showInfoAction->isChecked()) {
			if (int(lineSpacing) + value >= 0) {
				lineSpacing += value;
				settings->setValue("fontSize", lineSpacing);
				imageView->update();
			}
		}
	}

	void MainInterface::loadSettings() {
		fontSize = settings->value("fontSize", 14).toUInt();
		if (fontSize < 1) fontSize = 1;
		lineSpacing = settings->value("lineSpacing", 10).toUInt();
		fileActionAction->setChecked(settings->value("enableHotkeys", true).toBool());
		showInfoAction->setChecked(settings->value("showImageInfo", false).toBool());
		zoomLevelAction->setChecked(settings->value("showZoomLevel", false).toBool());
		enlargementAction->setChecked(settings->value("enlargeSmallImages", false).toBool());
		toggleSmallImageUpscaling(enlargementAction->isChecked());
		smoothingAction->setChecked(settings->value("useSmoothEnlargmentInterpolation", false).toBool());
		toggleEnglargmentInterpolationMethod(smoothingAction->isChecked());
		sharpeningAction->setChecked(settings->value("sharpenImagesAfterDownscale", false).toBool());
		toggleSharpening(sharpeningAction->isChecked());
		menuBarAutoHideAction->setChecked(!settings->value("autoHideMenuBar", true).toBool());
		toggleMenuBarAutoHide(menuBarAutoHideAction->isChecked());
		if (imageView->OpenClAvailable()) {
			gpuAction->setChecked(settings->value("useGpu", true).toBool());
		} else {
			gpuAction->setChecked(false);
			gpuAction->setEnabled(false);
		}
		toggleGpu(gpuAction->isChecked());
		QColor backgroundColor = settings->value("backgroundColor", QColor(Qt::black)).value<QColor>();
		imageView->setInterfaceBackgroundColor(backgroundColor);
		if (backgroundColor == Qt::black) {
			backgroundColorBlackAction->setChecked(true);
		} else if (backgroundColor == Qt::white) {
			backgroundColorWhiteAction->setChecked(true);
		} else if (backgroundColor == darkGray) {
			backgroundColorGrayAction->setChecked(true);
		} else {
			backgroundColorCustomAction->setChecked(true);
		}
		includePartiallySupportedFilesAction->setChecked(settings->value("includePreviewOnlyFiles", true).toBool());
		autoRotationAction->setChecked(settings->value("autoRotateImages", true).toBool());
	}

	void MainInterface::deleteCurrentImage(bool askForConfirmation, bool includeSidecarFiles, bool onlyXmp) {
		if (askForConfirmation) {
			QMessageBox msgBox;
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::No);
			msgBox.setWindowTitle(tr("Delete File"));
			msgBox.setText(tr("Please confirm that you want to delete this file."));
			msgBox.setButtonText(QMessageBox::Yes, tr("Delete"));
			msgBox.setButtonText(QMessageBox::No, tr("Do not Delete"));
			msgBox.setIcon(QMessageBox::Question);
			if (msgBox.exec() == QMessageBox::No) return;
		}

		QString filepath = getFullImagePath(currentFileIndex);
		if (QFileInfo(filepath).exists()) {
			if (utility::moveFileToRecycleBin(filepath)) {
				if (includeSidecarFiles && supportedRawFormats.contains(QFileInfo(filepath).suffix().toLower())) {
					QString extension = onlyXmp ? "xmp" : "*";
					QStringList filters;
					filters << QString("%1.%2").arg(QFileInfo(filepath).baseName(), extension);
					QStringList contents = currentDirectory.entryList(filters, QDir::Files);
					for (QString const & file : contents) {
						if (!utility::moveFileToRecycleBin(currentDirectory.absoluteFilePath(file))) {
							QMessageBox::critical(this,
												  tr("Sidecar File Not Deleted"),
												  tr("The sidecar file %1 could not be deleted. Please check that you have the required permissions and that the path length does not exceed MAX_PATH.").arg(file),
												  QMessageBox::StandardButton::Close,
												  QMessageBox::StandardButton::Close);
						}
					}
				}
				removeCurrentImageFromList(includeSidecarFiles, onlyXmp);
			} else {
#ifdef Q_OS_WIN
				QMessageBox::critical(this,
									  tr("File Not Deleted"),
									  tr("The file could not be deleted. Please check that you have the required permissions and that the path length does not exceed MAX_PATH."),
									  QMessageBox::StandardButton::Close,
									  QMessageBox::StandardButton::Close);
#else
				QMessageBox::critical(this,
									  tr("File Not Deleted"),
									  tr("The file could not be deleted. Please check that you have the required permissions. It might also be that your trash folder is not at the default location."),
									  QMessageBox::StandardButton::Close,
									  QMessageBox::StandardButton::Close);
#endif
			}

		} else {
			QMessageBox::critical(this,
								  tr("File Not Found"),
								  tr("The file could not be deleted because it no longer exists."),
								  QMessageBox::StandardButton::Close,
								  QMessageBox::StandardButton::Close);
		}
	}

	void MainInterface::moveCurrentImage(QString const & newFolder, bool askForConfirmation, bool includeSidecarFiles, bool onlyXmp) {
		if (askForConfirmation) {
			QMessageBox msgBox;
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::No);
			msgBox.setWindowTitle(tr("Move File"));
			msgBox.setText(tr("Please confirm that you want to move this file to the folder you specified."));
			msgBox.setButtonText(QMessageBox::Yes, tr("Move"));
			msgBox.setButtonText(QMessageBox::No, tr("Do not Move"));
			msgBox.setIcon(QMessageBox::Question);
			if (msgBox.exec() == QMessageBox::No) return;
		}
		QDir dir = QDir(newFolder);
		QString oldPath = getFullImagePath(currentFileIndex);
		QString newPath = dir.absoluteFilePath(filesInDirectory[currentFileIndex]);
		if (utility::moveFile(oldPath, newPath, false, this)) {
			if (includeSidecarFiles && supportedRawFormats.contains(QFileInfo(oldPath).suffix().toLower())) {
				QString extension = onlyXmp ? "xmp" : "*";
				QStringList filters;
				filters << QString("%1.%2").arg(QFileInfo(oldPath).baseName(), extension);
				QStringList contents = currentDirectory.entryList(filters, QDir::Files);
				for (QString const & file : contents) {
					if (!utility::moveFile(currentDirectory.absoluteFilePath(file), dir.absoluteFilePath(file), true, this)) {
						QMessageBox::critical(this,
											  tr("Sidecar File Not Moved"),
											  tr("The sidecar file %1 could not be deleted. Please check that you have the required permissions and that the path length does not exceed MAX_PATH.").arg(file),
											  QMessageBox::StandardButton::Close,
											  QMessageBox::StandardButton::Close);
					}
				}
			}
			removeCurrentImageFromList(includeSidecarFiles, onlyXmp);
		}
	}

	void MainInterface::copyCurrentImage(QString const & newFolder, bool askForConfirmation, bool includeSidecarFiles, bool onlyXmp) {
		if (askForConfirmation) {
			QMessageBox msgBox;
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::No);
			msgBox.setWindowTitle(tr("Copy File"));
			msgBox.setText(tr("Please confirm that you want to copy this file to the folder you specified."));
			msgBox.setButtonText(QMessageBox::Yes, tr("Copy"));
			msgBox.setButtonText(QMessageBox::No, tr("Do not Copy"));
			msgBox.setIcon(QMessageBox::Question);
			if (msgBox.exec() == QMessageBox::No) return;
		}
		QDir dir = QDir(newFolder);
		QString oldPath = getFullImagePath(currentFileIndex);
		QString newPath = dir.absoluteFilePath(filesInDirectory[currentFileIndex]);
		if (utility::copyFile(oldPath, newPath, false, this)) {
			if (includeSidecarFiles && supportedRawFormats.contains(QFileInfo(oldPath).suffix().toLower())) {
				QString extension = onlyXmp ? "xmp" : "*";
				QStringList filters;
				filters << QString("%1.%2").arg(QFileInfo(oldPath).baseName(), extension);
				QStringList contents = currentDirectory.entryList(filters, QDir::Files);
				for (QString const & file : contents) {
					if (!utility::copyFile(currentDirectory.absoluteFilePath(file), dir.absoluteFilePath(file), true, this)) {
						QMessageBox::critical(this,
											  tr("Sidecar File Not Copied"),
											  tr("The sidecar file %1 could not be copied. Please check that you have the required permissions and that the path length does not exceed MAX_PATH.").arg(file),
											  QMessageBox::StandardButton::Close,
											  QMessageBox::StandardButton::Close);
					}
				}
			}
		}

	}

	//============================================================================ PRIVATE SLOTS =============================================================================\\

	void MainInterface::refresh() {
		if (currentFileIndex >= 0 && filesInDirectory.size() > 0) {
			loadImage(getFullImagePath(currentFileIndex));
		}
	}

	void MainInterface::showHotkeyDialog() {
		disableAutomaticMouseHide();
		hotkeyDialog->show();
	}

	void MainInterface::nextSlide() {
		loadNextImage();
		if (!settings->value("slideshowLoop", false).toBool() && currentFileIndex == (filesInDirectory.size() - 1)) {
			stopSlideshow();
		}
	}

	void MainInterface::cleanUpThreads() {
		try {
			std::lock_guard<std::mutex> lock(threadDeletionMutex);
			size_t previousIndex = previousFileIndex();
			size_t nextIndex = nextFileIndex();
			for (std::map<QString, std::shared_future<Image>>::iterator it = threads.begin(); it != threads.end();) {
				int index = filesInDirectory.indexOf(it->first);
				//see if the thread has finished loading
				//also the exif should have finished loading to prevent blocking, check if it's valid first to not derefence an invalid pointer
				if (index != currentFileIndex
					&& index != previousIndex
					&& index != nextIndex
					&& it->second.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready
					&& (!it->second.get().isValid() || it->second.get().exif()->isReady() || it->second.get().exif()->isDeferred())) {
					it = threads.erase(it);
				} else {
					++it;
				}
			}
			if (threads.size() > 3) threadCleanUpTimer->start(threadCleanUpInterval);
		} catch (...) {
			//Probably couldn't lock the mutex (thread already owns it?)
			if (threads.size() > 3) threadCleanUpTimer->start(threadCleanUpInterval);
		}
	}

	void MainInterface::quit() {
		QCoreApplication::quit();
	}

	void MainInterface::saveWindowSize() {
		settings->setValue("windowSize", size());
		settings->setValue("windowPosition", pos());
		QMessageBox::information(this,
								 tr("Successfull"),
								 tr("The current size and position of the window have been saved as the new default."),
								 QMessageBox::Close);
	}

	void MainInterface::hideMouse() const {
		qApp->setOverrideCursor(Qt::BlankCursor);
		mouseHideTimer->stop();
	}

	void MainInterface::showMouse() const {
		qApp->setOverrideCursor(Qt::ArrowCursor);
	}

	void MainInterface::enableAutomaticMouseHide() {
		if (isFullScreen() && !menuBar()->isVisible() && !slideshowDialog->isVisible() && !sharpeningDialog->isVisible() && !hotkeyDialog->isVisible()) {
			mouseHideTimer->start(mouseHideDelay);
		}
	}

	void MainInterface::disableAutomaticMouseHide() {
		mouseHideTimer->stop();
		showMouse();
	}

	void MainInterface::showMenuBar() {
		if (isFullScreen()) disableAutomaticMouseHide();
		menuBar()->setVisible(true);
	}

	void MainInterface::hideMenuBar(QAction* triggeringAction) {
		if (!menuBarAutoHideAction->isChecked() || isFullScreen()) {
			menuBar()->setVisible(false);
			enableAutomaticMouseHide();
		}
	}

	void MainInterface::populateApplicationMenu() {
		applicationMenu->removeAction(installAction);
		applicationMenu->removeAction(uninstallAction);
		if (applicationIsInstalled()) {
			applicationMenu->insertAction(aboutAction, uninstallAction);
		} else {
			applicationMenu->insertAction(aboutAction, installAction);
		}

	}

	void MainInterface::runInstaller() {
#ifdef Q_OS_WIN
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
								  tr("The installer executable (WinInstaller.exe) could not be found. Make sure it is located in the same directory as AcuteViewer.exe."),
								  QMessageBox::Close);
		}
#endif
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
								  tr("The installer executable (WinInstaller.exe) could not be found. Make sure it is located in the same directory as AcuteViewer.exe."),
								  QMessageBox::Close);
		}
#endif
	}

	void MainInterface::resetSettings() {
		if (QMessageBox::Yes == QMessageBox::question(this,
													  tr("Reset Application Settings"),
													  tr("Are you sure you want to reset all application settings to the default values?"),
													  QMessageBox::Yes | QMessageBox::No)) {
			settings->clear();
			loadSettings();
			imageView->update();
			QMessageBox::information(this,
									 tr("Reset Successfull"),
									 tr("The application settings have been reset to the default values."),
									 QMessageBox::Close);
		}
	}

	void MainInterface::toggleSlideshow() {
		if (slideshowTimer->isActive()) {
			stopSlideshow();
		} else {
			slideshowDialog->show();
			disableAutomaticMouseHide();
		}
	}

	void MainInterface::toggleSlideshowNoDialog() {
		if (slideshowTimer->isActive()) {
			stopSlideshow();
		} else {
			startSlideshow();
		}
	}

	void MainInterface::startSlideshow() {
		slideshowAction->setText(tr("&Stop Slideshow"));
		slideshowTimer->start(std::abs(settings->value("slideDelay", 3).toDouble()) * 1000);
	}

	void MainInterface::stopSlideshow() {
		slideshowAction->setText(tr("&Start Slideshow"));
		slideshowTimer->stop();
	}

	void MainInterface::toggleFullscreen() {
		if (isFullScreen()) {
			exitFullscreen();
		} else {
			enterFullscreen();
		}
	}

	void MainInterface::rotateLeft() {
		userRotation -= 90;
		imageView->rotateLeft();
	}

	void MainInterface::rotateRight() {
		userRotation += 90;
		imageView->rotateRight();
	}

	void MainInterface::resetRotation() {
		userRotation = 0;
		imageView->setRotation(0);
		if (autoRotationAction->isChecked()) autoRotateImage();
	}

	void MainInterface::zoomTo100() {
		skipNextAltRelease = true;
		imageView->zoomToHundredPercent();
	}

	void MainInterface::toggleGpu(bool value) {
		imageView->setUseGpu(value);
		settings->setValue("useGpu", value);
	}

	void MainInterface::toggleInfoOverlay(bool value) {
		//if the thread of the currently displayed image is ready, start loading exif
		if (!currentThreadName.isEmpty()
			&& currentThread().valid()
			&& currentThread().wait_for(std::chrono::milliseconds(0)) == std::future_status::ready
			&& currentThread().get().isValid()) {
			currentThread().get().exif()->startLoading();
		}
		if (image.isValid()) image.exif()->startLoading();
		imageView->update();
		settings->setValue("showImageInfo", value);
	}

	void MainInterface::toggleZoomLevelOverlay(bool value) {
		imageView->update();
		settings->setValue("showZoomLevel", value);
	}

	void MainInterface::reactToReadImageCompletion(Image image) {
		this->image = image;
		//calling this function although the exif might not be set to deferred loading is no problem (it checks internally)
		if (exifIsRequired() && currentThread().get().isValid()) currentThread().get().exif()->startLoading();
		statusHint = QString();
		displayImageIfOk();
		if (filesInDirectory.size() != 0) {
			//preload next and previous image in background
			std::lock_guard<std::mutex> lock(threadDeletionMutex);
			if (threads.find(filesInDirectory[previousFileIndex()]) == threads.end()) {
				threads[filesInDirectory[previousFileIndex()]] = std::async(std::launch::async,
																							  &MainInterface::readImage,
																							  this,
																							  getFullImagePath(previousFileIndex()),
																							  false);
			}
			if (threads.find(filesInDirectory[nextFileIndex()]) == threads.end()) {
				threads[filesInDirectory[nextFileIndex()]] = std::async(std::launch::async,
																						  &MainInterface::readImage,
																						  this, getFullImagePath(nextFileIndex()),
																						  false);
			}
		}
		slideshowAction->setEnabled(true);
		slideshowNoDialogAction->setEnabled(true);
		refreshAction->setEnabled(true);
		loading = false;
	}

	void MainInterface::reactToExifLoadingCompletion(ExifData* sender) {
		if (showInfoAction->isChecked()) {
		//if the sender is the currently displayed image
			if (image.exif().get() == sender) {
				update();
			}
		}
	}

	void MainInterface::openDialog() {
		QString supportedFiles = QString("Fully Supported Images (") + supportedExtensions.join(" ") + QString(")");
		QStringList allTypes;
		allTypes << supportedExtensions << partiallySupportedExtensions;
		allTypes.sort();
		QString partiallySupportedFiles = QString("All Supported Images (") + allTypes.join(" ") + QString(")");
		QString filters = QString("All Files (*.*);; %1;; %2;;").arg(partiallySupportedFiles).arg(supportedFiles);
		QStringList paths = QFileDialog::getOpenFileNames(this,
														  tr("Open Config File"),
														  settings->value("lastOpenPath", QDir::rootPath()).toString(),
														  filters);

		if (!(paths.size() < 1)) {
			settings->setValue("lastOpenPath", QFileInfo(paths.at(0)).path());
			if (paths.size() == 1) {
				loadImage(paths.at(0));
			} else {
				loadImages(paths);
			}
		}
	}

	void MainInterface::toggleEnglargmentInterpolationMethod(bool value) {
		imageView->setUseSmoothTransform(value);
		settings->setValue("useSmoothEnlargmentInterpolation", value);
	}

	void MainInterface::toggleSmallImageUpscaling(bool value) {
		imageView->setPreventMagnificationInDefaultZoom(!value);
		settings->setValue("enlargeSmallImages", value);
	}

	void MainInterface::toggleSharpening(bool value) {
		imageView->setEnablePostResizeSharpening(value);
		settings->setValue("sharpenImagesAfterDownscale", value);
	}

	void MainInterface::toggleMenuBarAutoHide(bool value) {
		settings->setValue("autoHideMenuBar", !value);
		if (value) {
			showMenuBar();
		} else {
			hideMenuBar();
		}
	}

	void MainInterface::togglePreviewOnlyFiles(bool value) {
		settings->setValue("includePreviewOnlyFiles", value);
		refresh();
	}

	void MainInterface::showSharpeningOptions() {
		disableAutomaticMouseHide();
		sharpeningDialog->show();
		sharpeningDialog->raise();
		sharpeningDialog->activateWindow();
	}

	void MainInterface::updateSharpening() {
		sharpeningAction->setChecked(settings->value("sharpenImagesAfterDownscale", false).toBool());
		imageView->setPostResizeSharpening(sharpeningAction->isChecked(),
												 settings->value("sharpeningStrength", 0.5).toDouble(),
												 settings->value("sharpeningRadius", 1).toDouble());
	}

	void MainInterface::changeBackgroundColor(QAction* action) {
		if (action == backgroundColorBlackAction) {
			imageView->setInterfaceBackgroundColor(Qt::black);
			settings->setValue("backgroundColor", QColor(Qt::black));
		} else if (action == backgroundColorGrayAction) {
			imageView->setInterfaceBackgroundColor(darkGray);
			settings->setValue("backgroundColor", darkGray);
		} else if (action == backgroundColorWhiteAction) {
			imageView->setInterfaceBackgroundColor(Qt::white);
			settings->setValue("backgroundColor", QColor(Qt::white));
		} else if (action == backgroundColorCustomAction) {
			QColor color = QColorDialog::getColor(settings->value("lastCustomColor", QColor(Qt::black)).value<QColor>(), this, tr("Choose Background Colour"));
			if (color.isValid()) {
				settings->setValue("lastCustomColor", color);
				settings->setValue("backgroundColor", color);
				imageView->setInterfaceBackgroundColor(color);
			} else {
				if (imageView->getInterfaceBackgroundColor() == Qt::black) {
					backgroundColorBlackAction->setChecked(true);
				} else if (imageView->getInterfaceBackgroundColor() == darkGray) {
					backgroundColorGrayAction->setChecked(true);
				} else if (imageView->getInterfaceBackgroundColor() == Qt::white) {
					backgroundColorWhiteAction->setChecked(true);
				}
			}
		}
	}

	void MainInterface::toggleAutoRotation(bool value) {
		settings->setValue("autoRotateImages", value);
		if (value) {
			autoRotateImage();
		} else {
			imageView->setRotation(userRotation);
		}
	}

	void MainInterface::triggerCustomAction1() {
		if (filesInDirectory.size() > 0 && !loading) {
			loading = true;
			if (hotkeyDialog->getAction1() == 0) {
				statusHint = tr("Deleting...");
				update();
				deleteCurrentImage(hotkeyDialog->getShowConfirmation(), hotkeyDialog->getIncludeSidecarFiles());
			} else if (hotkeyDialog->getAction1() == 1) {
				statusHint = tr("Moving...");
				update();
				moveCurrentImage(hotkeyDialog->getFolder1(), hotkeyDialog->getShowConfirmation(), hotkeyDialog->getIncludeSidecarFiles());
			} else if (hotkeyDialog->getAction1() == 2) {
				statusHint = tr("Copying...");
				update();
				copyCurrentImage(hotkeyDialog->getFolder1(), hotkeyDialog->getShowConfirmation(), hotkeyDialog->getIncludeSidecarFiles());
			}
			statusHint = QString();
			loading = false;
			update();
		}
	}

	void MainInterface::triggerCustomAction2() { 
		if (filesInDirectory.size() > 0 && !loading) {
			loading = true;
			if (hotkeyDialog->getAction2() == 0) {
				statusHint = tr("Deleting...");
				update();
				deleteCurrentImage(hotkeyDialog->getShowConfirmation(), hotkeyDialog->getIncludeSidecarFiles(), hotkeyDialog->getSidecarType());
			} else if (hotkeyDialog->getAction2() == 1) {
				statusHint = tr("Moving...");
				update();
				moveCurrentImage(hotkeyDialog->getFolder2(), hotkeyDialog->getShowConfirmation(), hotkeyDialog->getIncludeSidecarFiles(), hotkeyDialog->getSidecarType());
			} else if (hotkeyDialog->getAction2() == 2) {
				statusHint = tr("Copying...");
				update();
				copyCurrentImage(hotkeyDialog->getFolder2(), hotkeyDialog->getShowConfirmation(), hotkeyDialog->getIncludeSidecarFiles(), hotkeyDialog->getSidecarType());
			}
			statusHint = QString();
			loading = false;
			update();
		}
	}

	void MainInterface::updateCustomHotkeys() {
		fileActionAction->setChecked(hotkeyDialog->getHotkeysEnabled());
		customAction1->setEnabled(hotkeyDialog->getHotkeysEnabled() && hotkeyDialog->getHotkey1Enabled());
		customAction2->setEnabled(hotkeyDialog->getHotkeysEnabled() && hotkeyDialog->getHotkey2Enabled());
		customAction1->setShortcut(hotkeyDialog->getKeySequence1());
		customAction2->setShortcut(hotkeyDialog->getKeySequence2());
	}

}