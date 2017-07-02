#include "MainInterface.h"

namespace sv {

	Image::Image() { }

	Image::Image(cv::Mat mat, std::shared_ptr<ExifData> exifData, bool isPreviewImage) : matrix(mat), exifData(exifData), valid(true), previewImage(isPreviewImage) { }

	cv::Mat Image::mat() const {
		return this->matrix;
	}

	std::shared_ptr<ExifData> Image::exif() const {
		return this->exifData;
	}

	bool Image::isValid() const {
		return valid;
	}

	bool Image::isPreviewImage() const {
		return this->previewImage;
	}

	//============================================================================ MAIN INTERFACE ============================================================================\\

	MainInterface::MainInterface(QString openWithFilename, QWidget *parent)
		: QMainWindow(parent),
		settings(new QSettings(QSettings::IniFormat, QSettings::UserScope, "Acute Viewer", "Acute Viewer")) {

		this->initialize();

		if (openWithFilename != QString()) {
			this->loadImage(openWithFilename);
		}
	}

	MainInterface::MainInterface(QStringList openWithFilenames, QWidget * parent) 
		: QMainWindow(parent),
		settings(new QSettings(QSettings::IniFormat, QSettings::UserScope, "Acute Viewer", "Acute Viewer")) {

		this->initialize();
		
		this->loadImages(openWithFilenames);
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
		delete this->refreshAction;
		delete this->resetSettingsAction;
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
		return this->settings->value("windowSize", QSize(900, 600)).toSize();
		//return QSize(900, 600);
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
				this->changeFontSizeBy(1);
				e->accept();
			} else if (e->key() == Qt::Key_Left) {
				this->changeFontSizeBy(-1);
				e->accept();
			} else if (e->key() == Qt::Key_Up) {
				this->changeLineSpacingBy(1);
				e->accept();
			} else if (e->key() == Qt::Key_Down) {
				this->changeLineSpacingBy(-1);
				e->accept();
			}
		} else if (e->key() == Qt::Key_Right || e->key() == Qt::Key_Down) {
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
		if (e->key() == Qt::Key_Alt && !this->skipNextAltRelease) {
			if (!this->menuBar()->isVisible()){
				this->showMenuBar();
				e->accept();
			} else {
				this->hideMenuBar();
				e->ignore();
			}
		}
		if (e->key() == Qt::Key_Alt) {
			this->skipNextAltRelease = false;
		}
	}

	void MainInterface::mouseDoubleClickEvent(QMouseEvent* e) {
		if (e->button() == Qt::LeftButton) {
			this->toggleFullscreen();
			e->accept();
		}
	}

	void MainInterface::mouseReleaseEvent(QMouseEvent* e) {
		if (e->button() == Qt::ForwardButton) this->loadNextImage();
		if (e->button() == Qt::BackButton) this->loadPreviousImage();
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
				this->saveSizeAction->setEnabled(!this->isMaximized());
			} else if (this->isFullScreen()) {
				QWindowStateChangeEvent* windowStateChangeEvent = static_cast<QWindowStateChangeEvent*>(e);
				this->settings->setValue("maximized", bool(windowStateChangeEvent->oldState() & Qt::WindowMaximized));
				this->saveSizeAction->setEnabled(false);
			}
		}
	}

	void MainInterface::wheelEvent(QWheelEvent * e) {
		if (e->modifiers() == Qt::ShiftModifier) {
			if (e->delta() > 0) {
				this->userRotation -= 10;
				this->imageView->rotateBy(-10);
			} else if (e->delta() < 0) {
				this->userRotation += 10;
				this->imageView->rotateBy(10);
			}
		}
	}

	//=============================================================================== PRIVATE ===============================================================================\\

	void MainInterface::initialize() {
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
		this->imageView->setUseGpu(true);
		this->imageView->setPostResizeSharpening(false, this->settings->value("sharpeningStrength", 0.5).toDouble(), this->settings->value("sharpeningRadius", 0.5).toDouble());
		setCentralWidget(this->imageView);

		this->slideshowDialog = new SlideshowDialog(settings, this);
		this->slideshowDialog->setWindowModality(Qt::WindowModal);
		QObject::connect(this->slideshowDialog, SIGNAL(accepted()), this, SLOT(startSlideshow()));
		QObject::connect(this->slideshowDialog, SIGNAL(finished(int)), this, SLOT(enableAutomaticMouseHide()));

		this->sharpeningDialog = new SharpeningDialog(settings, this);
		QObject::connect(this->sharpeningDialog, SIGNAL(sharpeningParametersChanged()), this, SLOT(updateSharpening()));
		QObject::connect(this->sharpeningDialog, SIGNAL(finished(int)), this, SLOT(enableAutomaticMouseHide()));

		this->hotkeyDialog = new HotkeyDialog(settings, this);
		this->hotkeyDialog->setWindowModality(Qt::WindowModal);
		QObject::connect(this->hotkeyDialog, SIGNAL(finished(int)), this, SLOT(enableAutomaticMouseHide()));
		QObject::connect(this->hotkeyDialog, SIGNAL(finished(int)), this, SLOT(updateCustomHotkeys()));

		this->aboutDialog = new AboutDialog(settings, this);
		this->aboutDialog->setWindowModality(Qt::WindowModal);

		QObject::connect(this->menuBar(), SIGNAL(triggered(QAction*)), this, SLOT(hideMenuBar(QAction*)));
		this->fileMenu = this->menuBar()->addMenu(tr("&File"));
		this->viewMenu = this->menuBar()->addMenu(tr("&View"));
		this->zoomMenu = this->menuBar()->addMenu(tr("&Zoom"));
		this->rotationMenu = this->menuBar()->addMenu(tr("&Rotation"));
		this->sharpeningMenu = this->menuBar()->addMenu(tr("&Sharpening"));
		this->slideshowMenu = this->menuBar()->addMenu(tr("S&lideshow"));

		this->applicationMenu = this->menuBar()->addMenu(tr("&Application"));
#ifdef Q_OS_WIN
		QObject::connect(this->applicationMenu, SIGNAL(aboutToShow()), this, SLOT(populateApplicationMenu()));
		this->installAction = new QAction(tr("&Install"), this);
		QObject::connect(this->installAction, SIGNAL(triggered()), this, SLOT(runInstaller()));
		this->uninstallAction = new QAction(tr("&Uninstall"), this);
		QObject::connect(this->uninstallAction, SIGNAL(triggered()), this, SLOT(runUninstaller()));
#endif

		this->aboutAction = new QAction(tr("&About"), this);
		this->aboutAction->setShortcut(Qt::Key_F1);
		this->aboutAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->aboutAction, SIGNAL(triggered()), this->aboutDialog, SLOT(show()));
		this->applicationMenu->addAction(aboutAction);
		this->addAction(this->aboutAction);

		this->openAction = new QAction(tr("&Open File"), this);
		this->openAction->setShortcut(QKeySequence::Open);
		this->openAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->openAction, SIGNAL(triggered()), this, SLOT(openDialog()));
		this->fileMenu->addAction(this->openAction);
		this->addAction(this->openAction);

		this->refreshAction = new QAction(tr("&Refresh"), this);
		this->refreshAction->setEnabled(false);
		this->refreshAction->setShortcut(Qt::Key_F5);
		this->refreshAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->refreshAction, SIGNAL(triggered()), this, SLOT(refresh()));
		this->fileMenu->addAction(this->refreshAction);
		this->addAction(this->refreshAction);

		this->fileMenu->addSeparator();

		this->fileActionAction = new QAction(tr("&Enable File Action Hotkeys"), this);
		this->fileActionAction->setCheckable(true);
		this->fileActionAction->setChecked(false);
		this->fileActionAction->setShortcut(Qt::CTRL + Qt::Key_H);
		this->fileActionAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->fileActionAction, SIGNAL(triggered(bool)), this->hotkeyDialog, SLOT(setHotkeysEnabled(bool)));
		this->fileMenu->addAction(this->fileActionAction);
		this->addAction(this->fileActionAction);

		this->hotkeyOptionsAction = new QAction(tr("&Hotkey Options..."), this);
		this->hotkeyOptionsAction->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_O);
		this->hotkeyOptionsAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->hotkeyOptionsAction, SIGNAL(triggered()), this, SLOT(showHotkeyDialog()));
		this->fileMenu->addAction(this->hotkeyOptionsAction);
		this->addAction(this->hotkeyOptionsAction);

		this->fileMenu->addSeparator();

		this->includePartiallySupportedFilesAction = new QAction(tr("&Include Preview-Only Files in Directory List"), this);
		this->includePartiallySupportedFilesAction->setCheckable(true);
		this->includePartiallySupportedFilesAction->setChecked(true);
		this->includePartiallySupportedFilesAction->setShortcut(Qt::CTRL + Qt::Key_P);
		this->includePartiallySupportedFilesAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->includePartiallySupportedFilesAction, SIGNAL(triggered(bool)), this, SLOT(togglePreviewOnlyFiles(bool)));
		this->fileMenu->addAction(this->includePartiallySupportedFilesAction);
		this->addAction(this->includePartiallySupportedFilesAction);

		this->fileMenu->addSeparator();

		this->resetSettingsAction = new QAction(tr("&Reset All Settings to Default"), this);
		QObject::connect(this->resetSettingsAction, SIGNAL(triggered()), this, SLOT(resetSettings()));
		this->fileMenu->addAction(this->resetSettingsAction);

		this->fileMenu->addSeparator();

		this->quitAction = new QAction(tr("&Quit"), this);
		this->quitAction->setShortcut(Qt::CTRL + Qt::Key_Q);
		this->quitAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->quitAction, SIGNAL(triggered()), this, SLOT(quit()));
		this->fileMenu->addAction(this->quitAction);
		//so shortcuts also work when menu bar is not visible
		this->addAction(this->quitAction);

		this->showInfoAction = new QAction(tr("Show &Info Overlay"), this);
		this->showInfoAction->setCheckable(true);
		this->showInfoAction->setChecked(false);
		this->showInfoAction->setShortcut(Qt::Key_I);
		this->showInfoAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->showInfoAction, SIGNAL(triggered(bool)), this, SLOT(toggleInfoOverlay(bool)));
		this->viewMenu->addAction(this->showInfoAction);
		this->addAction(this->showInfoAction);

		this->zoomLevelAction = new QAction(tr("Show &Zoom Level Overlay"), this);
		this->zoomLevelAction->setCheckable(true);
		this->zoomLevelAction->setChecked(false);
		this->zoomLevelAction->setShortcut(Qt::Key_Z);
		this->zoomLevelAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->zoomLevelAction, SIGNAL(triggered(bool)), this, SLOT(toggleZoomLevelOverlay(bool)));
		this->viewMenu->addAction(this->zoomLevelAction);
		this->addAction(this->zoomLevelAction);

		this->backgroundColorMenu = this->viewMenu->addMenu(tr("&Background Colour"));

		this->backgroundColorBlackAction = new QAction(tr("&Black"), this);
		this->backgroundColorBlackAction->setCheckable(true);
		this->backgroundColorBlackAction->setChecked(false);
		this->backgroundColorBlackAction->setShortcut(Qt::CTRL + Qt::Key_B);
		this->backgroundColorBlackAction->setShortcutContext(Qt::ApplicationShortcut);
		this->backgroundColorMenu->addAction(this->backgroundColorBlackAction);
		this->addAction(this->backgroundColorBlackAction);

		this->backgroundColorGrayAction = new QAction(tr("Dark &Grey"), this);
		this->backgroundColorGrayAction->setCheckable(true);
		this->backgroundColorGrayAction->setChecked(false);
		this->backgroundColorGrayAction->setShortcut(Qt::CTRL + Qt::Key_G);
		this->backgroundColorGrayAction->setShortcutContext(Qt::ApplicationShortcut);
		this->backgroundColorMenu->addAction(this->backgroundColorGrayAction);
		this->addAction(this->backgroundColorGrayAction);

		this->backgroundColorWhiteAction = new QAction(tr("&White"), this);
		this->backgroundColorWhiteAction->setCheckable(true);
		this->backgroundColorWhiteAction->setChecked(false);
		this->backgroundColorWhiteAction->setShortcut(Qt::CTRL + Qt::Key_W);
		this->backgroundColorWhiteAction->setShortcutContext(Qt::ApplicationShortcut);
		this->backgroundColorMenu->addAction(this->backgroundColorWhiteAction);
		this->addAction(this->backgroundColorWhiteAction);

		this->backgroundColorCustomAction = new QAction(tr("&Custom"), this);
		this->backgroundColorCustomAction->setCheckable(true);
		this->backgroundColorCustomAction->setChecked(false);
		this->backgroundColorCustomAction->setShortcut(Qt::CTRL + Qt::Key_C);
		this->backgroundColorCustomAction->setShortcutContext(Qt::ApplicationShortcut);
		this->backgroundColorMenu->addAction(this->backgroundColorCustomAction);
		this->addAction(this->backgroundColorCustomAction);

		this->backgroundColorActionGroup = new QActionGroup(this);
		this->backgroundColorActionGroup->addAction(this->backgroundColorBlackAction);
		this->backgroundColorActionGroup->addAction(this->backgroundColorGrayAction);
		this->backgroundColorActionGroup->addAction(this->backgroundColorWhiteAction);
		this->backgroundColorActionGroup->addAction(this->backgroundColorCustomAction);
		this->backgroundColorActionGroup->setExclusive(true);
		QObject::connect(this->backgroundColorActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeBackgroundColor(QAction*)));

		this->viewMenu->addSeparator();

		this->fullscreenAction = new QAction(tr("&Fullscreen"), this);
		this->fullscreenAction->setCheckable(true);
		this->fullscreenAction->setChecked(false);
		this->fullscreenAction->setShortcut(Qt::Key_F);
		this->fullscreenAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->fullscreenAction, SIGNAL(triggered(bool)), this, SLOT(toggleFullscreen()));
		this->viewMenu->addAction(this->fullscreenAction);
		this->addAction(this->fullscreenAction);

		this->viewMenu->addSeparator();

		this->menuBarAutoHideAction = new QAction(tr("&Always Show Menu Bar"), this);
		this->menuBarAutoHideAction->setCheckable(true);
		this->menuBarAutoHideAction->setChecked(false);
		this->menuBarAutoHideAction->setShortcut(Qt::Key_M);
		this->menuBarAutoHideAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->menuBarAutoHideAction, SIGNAL(triggered(bool)), this, SLOT(toggleMenuBarAutoHide(bool)));
		this->viewMenu->addAction(this->menuBarAutoHideAction);
		this->addAction(this->menuBarAutoHideAction);

		this->viewMenu->addSeparator();

		this->gpuAction = new QAction(tr("&Use GPU Acceleration"), this);
		this->gpuAction->setCheckable(true);
		this->gpuAction->setChecked(this->imageView->getUseGpu());
		QObject::connect(this->gpuAction, SIGNAL(triggered(bool)), this, SLOT(toggleGpu(bool)));
		this->viewMenu->addAction(this->gpuAction);

		this->viewMenu->addSeparator();

		this->saveSizeAction = new QAction(tr("&Save Current Window Size and Position as Default"), this);
		QObject::connect(this->saveSizeAction, SIGNAL(triggered(bool)), this, SLOT(saveWindowSize()));
		this->viewMenu->addAction(this->saveSizeAction);

		this->enlargementAction = new QAction(tr("&Enlarge Smaller Images to Fit Window"), this);
		this->enlargementAction->setCheckable(true);
		this->enlargementAction->setChecked(false);
		this->enlargementAction->setShortcut(Qt::CTRL + Qt::Key_U);
		this->enlargementAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->enlargementAction, SIGNAL(triggered(bool)), this, SLOT(toggleSmallImageUpscaling(bool)));
		this->zoomMenu->addAction(this->enlargementAction);
		this->addAction(this->enlargementAction);

		this->smoothingAction = new QAction(tr("Use &Smooth Interpolation when Enlarging"), this);
		this->smoothingAction->setCheckable(true);
		this->smoothingAction->setChecked(false);
		this->smoothingAction->setShortcut(Qt::CTRL + Qt::Key_S);
		this->smoothingAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->smoothingAction, SIGNAL(triggered(bool)), this, SLOT(toggleEnglargmentInterpolationMethod(bool)));
		this->zoomMenu->addAction(this->smoothingAction);
		this->addAction(this->smoothingAction);

		this->zoomMenu->addSeparator();

		this->zoomToFitAction = new QAction(tr("Zoom to &Fit"), this);
		this->zoomToFitAction->setShortcut(Qt::CTRL + Qt::Key_0);
		this->zoomToFitAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->zoomToFitAction, SIGNAL(triggered(bool)), this->imageView, SLOT(resetZoom()));
		this->zoomMenu->addAction(this->zoomToFitAction);
		this->addAction(this->zoomToFitAction);

		this->zoomTo100Action = new QAction(tr("Zoom to &100%"), this);
		this->zoomTo100Action->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_0);
		this->zoomTo100Action->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->zoomTo100Action, SIGNAL(triggered(bool)), this, SLOT(zoomTo100()));
		this->zoomMenu->addAction(this->zoomTo100Action);
		this->addAction(this->zoomTo100Action);

		this->rotateLeftAction = new QAction(tr("Rotate View &Left"), this);
		this->rotateLeftAction->setShortcut(Qt::CTRL + Qt::Key_Left);
		this->rotateLeftAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->rotateLeftAction, SIGNAL(triggered(bool)), this, SLOT(rotateLeft()));
		this->rotationMenu->addAction(this->rotateLeftAction);
		this->addAction(this->rotateLeftAction);

		this->rotateRightAction = new QAction(tr("Rotate &View Right"), this);
		this->rotateRightAction->setShortcut(Qt::CTRL + Qt::Key_Right);
		this->rotateRightAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->rotateRightAction, SIGNAL(triggered(bool)), this, SLOT(rotateRight()));
		this->rotationMenu->addAction(this->rotateRightAction);
		this->addAction(this->rotateRightAction);

		this->resetRotationAction = new QAction(tr("&Reset Rotation"), this);
		this->resetRotationAction->setShortcut(Qt::SHIFT + Qt::Key_Escape);
		this->resetRotationAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->resetRotationAction, SIGNAL(triggered(bool)), this, SLOT(resetRotation()));
		this->rotationMenu->addAction(this->resetRotationAction);
		this->addAction(this->resetRotationAction);

		this->rotationMenu->addSeparator();

		this->autoRotationAction = new QAction(tr("&Automatic EXIF-based rotation"), this);
		this->autoRotationAction->setCheckable(true);
		this->autoRotationAction->setChecked(true);
		this->autoRotationAction->setShortcut(Qt::CTRL + Qt::Key_R);
		this->autoRotationAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->autoRotationAction, SIGNAL(triggered(bool)), this, SLOT(toggleAutoRotation(bool)));
		this->rotationMenu->addAction(this->autoRotationAction);
		this->addAction(this->autoRotationAction);

		this->sharpeningAction = new QAction(tr("&Sharpen Images After Downsampling"), this);
		this->sharpeningAction->setCheckable(true);
		this->sharpeningAction->setChecked(false);
		this->sharpeningAction->setShortcut(Qt::CTRL + Qt::Key_E);
		this->sharpeningAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->sharpeningAction, SIGNAL(triggered(bool)), this, SLOT(toggleSharpening(bool)));
		this->sharpeningMenu->addAction(this->sharpeningAction);
		this->addAction(this->sharpeningAction);

		this->sharpeningOptionsAction = new QAction(tr("Sharpening &Options..."), this);
		this->sharpeningOptionsAction->setShortcut(Qt::Key_O);
		this->sharpeningOptionsAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->sharpeningOptionsAction, SIGNAL(triggered(bool)), this, SLOT(showSharpeningOptions()));
		this->sharpeningMenu->addAction(this->sharpeningOptionsAction);
		this->addAction(this->sharpeningOptionsAction);

		this->slideshowAction = new QAction(tr("&Start Slideshow"), this);
		this->slideshowAction->setEnabled(false);
		this->slideshowAction->setShortcut(Qt::CTRL + Qt::Key_Space);
		this->slideshowAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->slideshowAction, SIGNAL(triggered()), this, SLOT(toggleSlideshow()));
		this->slideshowMenu->addAction(this->slideshowAction);
		this->addAction(this->slideshowAction);

		this->slideshowNoDialogAction = new QAction(this);
		this->slideshowNoDialogAction->setEnabled(false);
		this->slideshowNoDialogAction->setShortcut(Qt::Key_Space);
		this->slideshowNoDialogAction->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->slideshowNoDialogAction, SIGNAL(triggered()), this, SLOT(toggleSlideshowNoDialog()));
		this->addAction(this->slideshowNoDialogAction);

		this->customAction1 = new QAction(this);
		this->customAction1->setEnabled(this->hotkeyDialog->getHotkey1Enabled() && this->hotkeyDialog->getHotkeysEnabled());
		this->customAction1->setShortcut(this->hotkeyDialog->getKeySequence1());
		this->customAction1->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->customAction1, SIGNAL(triggered()), this, SLOT(triggerCustomAction1()));
		this->addAction(this->customAction1);

		this->customAction2 = new QAction(this);
		this->customAction2->setEnabled(this->hotkeyDialog->getHotkey2Enabled() && this->hotkeyDialog->getHotkeysEnabled());
		this->customAction2->setShortcut(this->hotkeyDialog->getKeySequence2());
		this->customAction2->setShortcutContext(Qt::ApplicationShortcut);
		QObject::connect(this->customAction2, SIGNAL(triggered()), this, SLOT(triggerCustomAction2()));
		this->addAction(this->customAction2);

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
		this->loadSettings();
		if (this->settings->contains("windowPosition")) {
			this->move(this->settings->value("windowPosition", QPoint(10, 10)).toPoint());
		}
		if (this->settings->contains("windowSize")) {
			this->resize(this->settings->value("windowSize", QSize(900, 600)).toSize());
		}
		if (this->settings->value("maximized", false).toBool()) {
			this->showMaximized();
		}
	}

	std::shared_future<Image>& MainInterface::currentThread() {
		return this->threads[this->currentThreadName];
	}

	bool MainInterface::exifIsRequired() const {
		return this->showInfoAction->isChecked() || this->autoRotationAction->isChecked();
	}

	Image MainInterface::readImage(QString path, bool emitSignals) {
		try {
			cv::Mat image;
			bool isPreviewImage = false;
			Image result;
			std::shared_ptr<ExifData> exifData;
			//for the images we know are not supported by opencv do not attempt to read them with opencv
			bool forcePreview = this->partiallySupportedExtensions.contains(QString("*.") + QFileInfo(path).suffix().toLower());
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
				exifData = std::shared_ptr<ExifData>(new ExifData(path, !this->exifIsRequired() && image.data));
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
		if (this->loading) return;
		std::unique_lock<std::mutex> lock(this->threadDeletionMutex);
		this->loading = true;

		if (this->filesInDirectory.size() != 0) {
			this->currentFileIndex = this->nextFileIndex();
			this->currentThreadName = this->filesInDirectory[this->currentFileIndex];
			if (this->threads.find(this->filesInDirectory[this->currentFileIndex]) == this->threads.end()) {
				this->loading = false;
				return;
			}
			this->waitForThreadToFinish(this->currentThread());
			//calling this function although the exif might not be set to deferred loading is no problem (it checks internally)
			if (this->exifIsRequired() && this->currentThread().get().isValid()) this->currentThread().get().exif()->startLoading();
			this->image = this->currentThread().get();

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
		} else {
			lock.unlock();
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
			this->currentThreadName = this->filesInDirectory[this->currentFileIndex];
			if (this->threads.find(this->filesInDirectory[this->currentFileIndex]) == this->threads.end()) {
				this->loading = false;
				return;
			}
			this->waitForThreadToFinish(this->currentThread());
			//calling this function although the exif might not be set to deferred loading is no problem (it checks internally)
			if (this->exifIsRequired() && this->currentThread().get().isValid()) this->currentThread().get().exif()->startLoading();
			this->image = this->currentThread().get();
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
		} else {
			lock.unlock();
		}
		this->loading = false;
		this->cleanUpThreads();
	}

	void MainInterface::clearThreads() {
		for (std::map<QString, std::shared_future<Image>>::iterator it = this->threads.begin(); it != this->threads.end(); ++it) {
			this->waitForThreadToFinish(it->second, false);
		}
		this->threads.clear();
	}

	size_t MainInterface::nextFileIndex() const {
		if (this->filesInDirectory.size() <= 0) return -1;
		return (this->currentFileIndex + 1) % this->filesInDirectory.size();
	}

	size_t MainInterface::previousFileIndex() const {
		if (this->filesInDirectory.size() <= 0) return -1;
		if (this->currentFileIndex <= 0) return this->filesInDirectory.size() - 1;
		return (this->currentFileIndex - 1) % this->filesInDirectory.size();
	}

	void MainInterface::removeCurrentImageFromList(bool includeSidecarFiles, bool onlyXmp) {
		std::unique_lock<std::mutex> lock(this->threadDeletionMutex);

		if (this->filesInDirectory.size() != 0) {
			QFileInfo imageInfo = QFileInfo(this->filesInDirectory[this->currentFileIndex]);
			QString baseName = imageInfo.baseName();
			QString extension = imageInfo.suffix().toLower();
			//remove current file from directory list
			this->filesInDirectory.remove(this->currentFileIndex);

			//remove sidecar files from directory list
			if (includeSidecarFiles) {
				for (int i = 0; i < this->filesInDirectory.size(); ++i) {
					QFileInfo fileInfo(this->filesInDirectory[i]);
					if (fileInfo.baseName() == baseName && (!onlyXmp || (fileInfo.suffix().toLower() == "xmp" && this->supportedRawFormats.contains(extension)))) {
						this->filesInDirectory.remove(i);
						//correct the index shift
						if (i < this->currentFileIndex) --this->currentFileIndex;
						--i;
					}
				}
			}

			//if there are now images left, quit
			if (this->filesInDirectory.size() <= 0) {
				this->currentFileIndex = -1;
				lock.unlock();
				this->cleanUpThreads();
				this->reset();
				return;
			}
			//update current file index and related variables
			if (this->currentFileIndex >= this->filesInDirectory.size()) this->currentFileIndex = this->filesInDirectory.size() - 1;
			this->currentThreadName = this->filesInDirectory[this->currentFileIndex];

			//load the image that is now the current one
			if (this->threads.find(this->filesInDirectory[this->currentFileIndex]) == this->threads.end()) {
				this->threads[this->filesInDirectory[this->currentFileIndex]] = std::async(std::launch::async,
																						   &MainInterface::readImage,
																						   this,
																						   this->getFullImagePath(this->currentFileIndex),
																						   false);
			}
			this->waitForThreadToFinish(this->currentThread());
			//calling this function although the exif might not be set to deferred loading is no problem (it checks internally)
			if (this->exifIsRequired() && this->currentThread().get().isValid()) this->currentThread().get().exif()->startLoading();
			this->image = this->currentThread().get();
			this->currentFileInfo = QFileInfo(this->getFullImagePath(this->currentFileIndex));

			//start loading next and previous image
			if (this->threads.find(this->filesInDirectory[this->nextFileIndex()]) == this->threads.end()) {
				this->threads[this->filesInDirectory[this->nextFileIndex()]] = std::async(std::launch::async,
																						  &MainInterface::readImage,
																						  this,
																						  this->getFullImagePath(this->nextFileIndex()),
																						  false);
			}
			if (this->threads.find(this->filesInDirectory[this->previousFileIndex()]) == this->threads.end()) {
				this->threads[this->filesInDirectory[this->previousFileIndex()]] = std::async(std::launch::async,
																							  &MainInterface::readImage,
																							  this,
																							  this->getFullImagePath(this->previousFileIndex()),
																							  false);
			}
			lock.unlock();
			this->displayImageIfOk();
		} else {
			lock.unlock();
		}
		this->cleanUpThreads();
	}

	void MainInterface::reset() {
		this->imageView->resetImage();
		this->image = sv::Image();
		this->filesInDirectory.clear();
		this->setWindowTitle(this->programTitle);
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
		//always scan directory; uncomment to scan only if different directory
		//if (directory != this->currentDirectory || this->noCurrentDir) {
			this->currentDirectory = directory;
			this->noCurrentDir = false;
			QStringList filters = this->supportedExtensions;
			filters << "*.bmp" << "*.dib" << "*.jpeg" << "*.jpg" << "*.jpe" << "*.jpeg" << "*.jp2" << "*.png" << "*.webp" << "*.pbm" << "*.pgm" << "*.ppm" << "*.sr" << "*.ras" << "*.tiff" << "*.tif";
			if (this->includePartiallySupportedFilesAction->isChecked()) {
				filters.append(this->partiallySupportedExtensions);
			}
			QStringList contents = directory.entryList(filters, QDir::Files);
			QCollator collator;
			collator.setNumericMode(true);
			std::sort(contents.begin(), contents.end(), collator);
			this->filesInDirectory = contents.toVector();
		//}
		if (this->filesInDirectory.size() == 0 || this->currentFileIndex < 0 || this->currentFileIndex >= this->filesInDirectory.size() || this->filesInDirectory.at(this->currentFileIndex) != filename) {
			this->currentFileIndex = this->filesInDirectory.indexOf(filename);
		}
		this->currentThreadName = filename;
		this->currentFileInfo = fileInfo;
		this->clearThreads();
		this->threads[filename] = std::async(std::launch::async, &MainInterface::readImage, this, path, true);
		this->setWindowTitle(this->windowTitle() + QString(tr(" - Loading...")));
		this->statusHint = tr("Loading...");
		this->imageView->update();
	}

	void MainInterface::loadImages(QStringList paths) {
		if (this->loading) return;
		std::unique_lock<std::mutex> lock(this->threadDeletionMutex);
		this->loading = true;
		
		QString path = paths.first();
		//find the path in the current directory listing
		QFileInfo fileInfo = QFileInfo(QDir::cleanPath(path));
		QDir directory = fileInfo.absoluteDir();
		QString filename = fileInfo.fileName();
		this->currentDirectory = directory;
		this->noCurrentDir = false;
		//remove all images that are not in the same directory
		QMutableListIterator<QString> it(paths);
		while (it.hasNext()) {
			if (QFileInfo(it.next()).absoluteDir() != this->currentDirectory) {
				it.remove();
			}
		}
		this->filesInDirectory.resize(paths.size());
		for (int i = 0; i < paths.size(); ++i) {
			this->filesInDirectory[i] = QFileInfo(QDir::cleanPath(paths[i])).fileName();
		}

		if (this->filesInDirectory.size() == 0 || this->currentFileIndex < 0 || this->currentFileIndex >= this->filesInDirectory.size() || this->filesInDirectory.at(this->currentFileIndex) != filename) {
			this->currentFileIndex = this->filesInDirectory.indexOf(filename);
		}
		this->currentThreadName = filename;
		this->currentFileInfo = fileInfo;
		this->clearThreads();
		this->threads[filename] = std::async(std::launch::async, &MainInterface::readImage, this, path, true);
		this->setWindowTitle(this->windowTitle() + QString(tr(" - Loading...")));
		this->statusHint = tr("Loading...");
		this->imageView->update();
	}

	void MainInterface::displayImageIfOk() {
		if (this->image.isValid()) {
			this->currentImageUnreadable = false;
			this->imageView->setImage(this->image.mat());
			if (this->autoRotationAction->isChecked()) {
				this->autoRotateImage();
			}
		} else {
			this->currentImageUnreadable = true;
			this->imageView->resetImage();
		}
		this->setWindowTitle(QString("%1%5 - %2 - %3 of %4").arg(this->currentFileInfo.fileName(),
															   this->programTitle).arg(this->currentFileIndex + 1).arg(this->filesInDirectory.size()).arg(this->image.isPreviewImage() ? " [Preview]" : ""));
	}

	void MainInterface::autoRotateImage() {
		if (this->image.isValid()) {
			this->image.exif()->join();
			if (this->image.exif()->hasExif()) {
				long orientation = this->image.exif()->orientation();
				this->imageView->setRotation(this->userRotation);
				orientation = (orientation + 1) / 2;
				if (orientation == 3) {
					this->imageView->rotateBy(90);
				} else if (orientation == 4) {
					this->imageView->rotateBy(-90);
				} else if (orientation == 2) {
					this->imageView->rotateBy(180);
				}
			}
		}
	}

	void MainInterface::enterFullscreen() {
		//this->imageView->setInterfaceBackgroundColor(Qt::black);
		this->showFullScreen();
		this->hideMenuBar();
		this->enableAutomaticMouseHide();
		this->fullscreenAction->setChecked(true);
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
		this->fullscreenAction->setChecked(false);
	}

	void MainInterface::infoPaintFunction(QPainter& canvas) {
		canvas.setRenderHint(QPainter::Antialiasing, true);
		QPen textPen(Qt::black);
		canvas.setPen(textPen);
		canvas.setBrush(Qt::NoBrush);
		QFont font;
		font.setPointSize(this->fontSize);
		canvas.setFont(font);
		QColor base = Qt::white;
		base.setAlpha(200);
		canvas.setBackground(base);
		canvas.setBackgroundMode(Qt::OpaqueMode);
		QFontMetrics metrics(font);
		if (this->currentImageUnreadable && this->statusHint.isEmpty()) {
			QString message = tr("This file could not be read:");
			canvas.drawText(QPoint((canvas.device()->width() - metrics.width(message)) / 2.0, canvas.device()->height() / 2.0 - 0.5*this->lineSpacing),
							message);
			canvas.drawText(QPoint((canvas.device()->width() - metrics.width(this->currentFileInfo.fileName())) / 2.0,
								   canvas.device()->height() / 2.0 + 0.5*this->lineSpacing + metrics.height()),
							this->currentFileInfo.fileName());
		}
		if (!this->statusHint.isEmpty()) {
			canvas.drawText(QPoint((canvas.device()->width() - metrics.width(this->statusHint)) / 2.0, canvas.device()->height() / 2.0 + 0.5*metrics.height()),
							this->statusHint);
		}
		if (this->showInfoAction->isChecked() && this->imageView->getImageAssigned()) {
			//draw current filename
			canvas.drawText(QPoint(30, 30 + metrics.height()),
							this->currentFileInfo.fileName());
			int sizeAndResolutionTopOffset = 30 + this->lineSpacing + 2 * metrics.height();
			if (this->image.isValid()) {
				QString resolution = QString::fromWCharArray(L"%1\u2006x\u2006%2").arg(this->image.mat().cols).arg(this->image.mat().rows);
				canvas.drawText(QPoint(30, sizeAndResolutionTopOffset),
								QString::fromWCharArray(L"%1, %2\u2006Mb").arg(resolution).arg(this->currentFileInfo.size() / 1048576.0, 0, 'f', 2));
				if (this->image.exif()->isReady()) {
					if (this->image.exif()->hasExif()) {
						//get camera model, speed, aperture and ISO
						QString cameraModel = this->image.exif()->cameraModel();
						QString lensModel = this->image.exif()->lensModel();
						QString aperture = this->image.exif()->fNumber();
						QString speed = this->image.exif()->exposureTime();
						QString focalLength = this->image.exif()->focalLength();
						QString equivalentFocalLength = this->image.exif()->focalLength35mmEquivalent();
						QString exposureBias = this->image.exif()->exposureBias();
						QString iso = this->image.exif()->iso();
						QString captureDate = this->image.exif()->captureDate();
						//calculate the v coordinates for the lines
						int heightOfOneLine = this->lineSpacing + metrics.height();
						int topOffset = 30 + 2 * this->lineSpacing + 3 * metrics.height();

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
						if (this->image.isPreviewImage()) {
							canvas.drawText(QPoint(30, topOffset), "[Preview Image]");
						}
					}
				} else {
					canvas.drawText(QPoint(30, 30 + 2 * this->lineSpacing + 3 * metrics.height()),
									tr("Loading EXIF..."));
				}
			}
		}
		if (this->zoomLevelAction->isChecked() && this->imageView->getImageAssigned()) {
			QString message = QString::number(this->imageView->getCurrentPreviewScalingFactor() * 100, 'f', 1).append("%");
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
		if (this->showInfoAction->isChecked() || this->zoomLevelAction->isChecked()) {
			if (this->fontSize + value >= 1) {
				this->fontSize += value;
				this->settings->setValue("fontSize", this->fontSize);
				this->imageView->update();
			}
		}
	}

	void MainInterface::changeLineSpacingBy(int value) {
		if (this->showInfoAction->isChecked()) {
			if (int(this->lineSpacing) + value >= 0) {
				this->lineSpacing += value;
				this->settings->setValue("fontSize", this->lineSpacing);
				this->imageView->update();
			}
		}
	}

	void MainInterface::loadSettings() {
		this->fontSize = this->settings->value("fontSize", 14).toUInt();
		if (this->fontSize < 1) this->fontSize = 1;
		this->lineSpacing = this->settings->value("lineSpacing", 10).toUInt();
		this->fileActionAction->setChecked(this->settings->value("enableHotkeys", true).toBool());
		this->showInfoAction->setChecked(this->settings->value("showImageInfo", false).toBool());
		this->zoomLevelAction->setChecked(this->settings->value("showZoomLevel", false).toBool());
		this->enlargementAction->setChecked(this->settings->value("enlargeSmallImages", false).toBool());
		this->toggleSmallImageUpscaling(this->enlargementAction->isChecked());
		this->smoothingAction->setChecked(this->settings->value("useSmoothEnlargmentInterpolation", false).toBool());
		this->toggleEnglargmentInterpolationMethod(this->smoothingAction->isChecked());
		this->sharpeningAction->setChecked(this->settings->value("sharpenImagesAfterDownscale", false).toBool());
		this->toggleSharpening(this->sharpeningAction->isChecked());
		this->menuBarAutoHideAction->setChecked(!this->settings->value("autoHideMenuBar", true).toBool());
		this->toggleMenuBarAutoHide(this->menuBarAutoHideAction->isChecked());
		if (this->imageView->OpenClAvailable()) {
			this->gpuAction->setChecked(this->settings->value("useGpu", true).toBool());
		} else {
			this->gpuAction->setChecked(false);
			this->gpuAction->setEnabled(false);
		}
		this->toggleGpu(this->gpuAction->isChecked());
		QColor backgroundColor = this->settings->value("backgroundColor", QColor(Qt::black)).value<QColor>();
		this->imageView->setInterfaceBackgroundColor(backgroundColor);
		if (backgroundColor == Qt::black) {
			this->backgroundColorBlackAction->setChecked(true);
		} else if (backgroundColor == Qt::white) {
			this->backgroundColorWhiteAction->setChecked(true);
		} else if (backgroundColor == this->darkGray) {
			this->backgroundColorGrayAction->setChecked(true);
		} else {
			this->backgroundColorCustomAction->setChecked(true);
		}
		this->includePartiallySupportedFilesAction->setChecked(this->settings->value("includePreviewOnlyFiles", true).toBool());
		this->autoRotationAction->setChecked(this->settings->value("autoRotateImages", true).toBool());
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

		QString filepath = this->getFullImagePath(this->currentFileIndex);
		if (QFileInfo(filepath).exists()) {
			if (utility::moveFileToRecycleBin(filepath)) {
				if (includeSidecarFiles && this->supportedRawFormats.contains(QFileInfo(filepath).suffix().toLower())) {
					QString extension = onlyXmp ? "xmp" : "*";
					QStringList filters;
					filters << QString("%1.%2").arg(QFileInfo(filepath).baseName(), extension);
					QStringList contents = this->currentDirectory.entryList(filters, QDir::Files);
					for (QString const & file : contents) {
						if (!utility::moveFileToRecycleBin(this->currentDirectory.absoluteFilePath(file))) {
							QMessageBox::critical(this,
												  tr("Sidecar File Not Deleted"),
												  tr("The sidecar file %1 could not be deleted. Please check that you have the required permissions and that the path length does not exceed MAX_PATH.").arg(file),
												  QMessageBox::StandardButton::Close,
												  QMessageBox::StandardButton::Close);
						}
					}
				}
				this->removeCurrentImageFromList(includeSidecarFiles, onlyXmp);
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
		QString oldPath = this->getFullImagePath(this->currentFileIndex);
		QString newPath = dir.absoluteFilePath(this->filesInDirectory[this->currentFileIndex]);
		if (utility::moveFile(oldPath, newPath, false, this)) {
			if (includeSidecarFiles && this->supportedRawFormats.contains(QFileInfo(oldPath).suffix().toLower())) {
				QString extension = onlyXmp ? "xmp" : "*";
				QStringList filters;
				filters << QString("%1.%2").arg(QFileInfo(oldPath).baseName(), extension);
				QStringList contents = this->currentDirectory.entryList(filters, QDir::Files);
				for (QString const & file : contents) {
					if (!utility::moveFile(this->currentDirectory.absoluteFilePath(file), dir.absoluteFilePath(file), true, this)) {
						QMessageBox::critical(this,
											  tr("Sidecar File Not Moved"),
											  tr("The sidecar file %1 could not be deleted. Please check that you have the required permissions and that the path length does not exceed MAX_PATH.").arg(file),
											  QMessageBox::StandardButton::Close,
											  QMessageBox::StandardButton::Close);
					}
				}
			}
			this->removeCurrentImageFromList(includeSidecarFiles, onlyXmp);
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
		QString oldPath = this->getFullImagePath(this->currentFileIndex);
		QString newPath = dir.absoluteFilePath(this->filesInDirectory[this->currentFileIndex]);
		if (utility::copyFile(oldPath, newPath, false, this)) {
			if (includeSidecarFiles && this->supportedRawFormats.contains(QFileInfo(oldPath).suffix().toLower())) {
				QString extension = onlyXmp ? "xmp" : "*";
				QStringList filters;
				filters << QString("%1.%2").arg(QFileInfo(oldPath).baseName(), extension);
				QStringList contents = this->currentDirectory.entryList(filters, QDir::Files);
				for (QString const & file : contents) {
					if (!utility::copyFile(this->currentDirectory.absoluteFilePath(file), dir.absoluteFilePath(file), true, this)) {
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
		if (this->currentFileIndex >= 0 && this->filesInDirectory.size() > 0) {
			this->loadImage(this->getFullImagePath(this->currentFileIndex));
		}
	}

	void MainInterface::showHotkeyDialog() {
		this->disableAutomaticMouseHide();
		this->hotkeyDialog->show();
	}

	void MainInterface::nextSlide() {
		this->loadNextImage();
		if (!settings->value("slideshowLoop", false).toBool() && this->currentFileIndex == (this->filesInDirectory.size() - 1)) {
			this->stopSlideshow();
		}
	}

	void MainInterface::cleanUpThreads() {
		try {
			std::lock_guard<std::mutex> lock(this->threadDeletionMutex);
			size_t previousIndex = this->previousFileIndex();
			size_t nextIndex = this->nextFileIndex();
			for (std::map<QString, std::shared_future<Image>>::iterator it = this->threads.begin(); it != this->threads.end();) {
				int index = this->filesInDirectory.indexOf(it->first);
				//see if the thread has finished loading
				//also the exif should have finished loading to prevent blocking, check if it's valid first to not derefence an invalid pointer
				if (index != this->currentFileIndex
					&& index != previousIndex
					&& index != nextIndex
					&& it->second.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready
					&& (!it->second.get().isValid() || it->second.get().exif()->isReady() || it->second.get().exif()->isDeferred())) {
					it = this->threads.erase(it);
				} else {
					++it;
				}
			}
			if (this->threads.size() > 3) this->threadCleanUpTimer->start(this->threadCleanUpInterval);
		} catch (...) {
			//Probably couldn't lock the mutex (thread already owns it?)
			if (this->threads.size() > 3) this->threadCleanUpTimer->start(this->threadCleanUpInterval);
		}
	}

	void MainInterface::quit() {
		QCoreApplication::quit();
	}

	void MainInterface::saveWindowSize() {
		this->settings->setValue("windowSize", this->size());
		this->settings->setValue("windowPosition", this->pos());
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
		if (this->isFullScreen() && !this->menuBar()->isVisible() && !this->slideshowDialog->isVisible() && !this->sharpeningDialog->isVisible() && !this->hotkeyDialog->isVisible()) {
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
			this->applicationMenu->insertAction(this->aboutAction, this->uninstallAction);
		} else {
			this->applicationMenu->insertAction(this->aboutAction, this->installAction);
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
			this->settings->clear();
			this->loadSettings();
			this->imageView->update();
			QMessageBox::information(this,
									 tr("Reset Successfull"),
									 tr("The application settings have been reset to the default values."),
									 QMessageBox::Close);
		}
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

	void MainInterface::toggleFullscreen() {
		if (this->isFullScreen()) {
			this->exitFullscreen();
		} else {
			this->enterFullscreen();
		}
	}

	void MainInterface::rotateLeft() {
		this->userRotation -= 90;
		this->imageView->rotateLeft();
	}

	void MainInterface::rotateRight() {
		this->userRotation += 90;
		this->imageView->rotateRight();
	}

	void MainInterface::resetRotation() {
		this->userRotation = 0;
		this->imageView->setRotation(0);
		if (this->autoRotationAction->isChecked()) this->autoRotateImage();
	}

	void MainInterface::zoomTo100() {
		this->skipNextAltRelease = true;
		this->imageView->zoomToHundredPercent();
	}

	void MainInterface::toggleGpu(bool value) {
		this->imageView->setUseGpu(value);
		this->settings->setValue("useGpu", value);
	}

	void MainInterface::toggleInfoOverlay(bool value) {
		//if the thread of the currently displayed image is ready, start loading exif
		if (!this->currentThreadName.isEmpty()
			&& this->currentThread().valid()
			&& this->currentThread().wait_for(std::chrono::milliseconds(0)) == std::future_status::ready
			&& this->currentThread().get().isValid()) {
			this->currentThread().get().exif()->startLoading();
		}
		if (this->image.isValid()) this->image.exif()->startLoading();
		this->imageView->update();
		this->settings->setValue("showImageInfo", value);
	}

	void MainInterface::toggleZoomLevelOverlay(bool value) {
		this->imageView->update();
		this->settings->setValue("showZoomLevel", value);
	}

	void MainInterface::reactToReadImageCompletion(Image image) {
		this->image = image;
		//calling this function although the exif might not be set to deferred loading is no problem (it checks internally)
		if (this->exifIsRequired() && this->currentThread().get().isValid()) this->currentThread().get().exif()->startLoading();
		this->statusHint = QString();
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
		this->refreshAction->setEnabled(true);
		this->loading = false;
	}

	void MainInterface::reactToExifLoadingCompletion(ExifData* sender) {
		if (this->showInfoAction->isChecked()) {
		//if the sender is the currently displayed image
			if (this->image.exif().get() == sender) {
				this->update();
			}
		}
	}

	void MainInterface::openDialog() {
		QString supportedFiles = QString("Fully Supported Images (") + this->supportedExtensions.join(" ") + QString(")");
		QStringList allTypes;
		allTypes << this->supportedExtensions << this->partiallySupportedExtensions;
		allTypes.sort();
		QString partiallySupportedFiles = QString("All Supported Images (") + allTypes.join(" ") + QString(")");
		QString filters = QString("All Files (*.*);; %1;; %2;;").arg(partiallySupportedFiles).arg(supportedFiles);
		QStringList paths = QFileDialog::getOpenFileNames(this,
														  tr("Open Config File"),
														  this->settings->value("lastOpenPath", QDir::rootPath()).toString(),
														  filters);

		if (!(paths.size() < 1)) {
			this->settings->setValue("lastOpenPath", QFileInfo(paths.at(0)).path());
			if (paths.size() == 1) {
				this->loadImage(paths.at(0));
			} else {
				this->loadImages(paths);
			}
		}
	}

	void MainInterface::toggleEnglargmentInterpolationMethod(bool value) {
		this->imageView->setUseSmoothTransform(value);
		this->settings->setValue("useSmoothEnlargmentInterpolation", value);
	}

	void MainInterface::toggleSmallImageUpscaling(bool value) {
		this->imageView->setPreventMagnificationInDefaultZoom(!value);
		this->settings->setValue("enlargeSmallImages", value);
	}

	void MainInterface::toggleSharpening(bool value) {
		this->imageView->setEnablePostResizeSharpening(value);
		this->settings->setValue("sharpenImagesAfterDownscale", value);
	}

	void MainInterface::toggleMenuBarAutoHide(bool value) {
		this->settings->setValue("autoHideMenuBar", !value);
		if (value) {
			this->showMenuBar();
		} else {
			this->hideMenuBar();
		}
	}

	void MainInterface::togglePreviewOnlyFiles(bool value) {
		this->settings->setValue("includePreviewOnlyFiles", value);
		this->refresh();
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

	void MainInterface::changeBackgroundColor(QAction* action) {
		if (action == this->backgroundColorBlackAction) {
			this->imageView->setInterfaceBackgroundColor(Qt::black);
			this->settings->setValue("backgroundColor", QColor(Qt::black));
		} else if (action == this->backgroundColorGrayAction) {
			this->imageView->setInterfaceBackgroundColor(this->darkGray);
			this->settings->setValue("backgroundColor", this->darkGray);
		} else if (action == this->backgroundColorWhiteAction) {
			this->imageView->setInterfaceBackgroundColor(Qt::white);
			this->settings->setValue("backgroundColor", QColor(Qt::white));
		} else if (action == this->backgroundColorCustomAction) {
			QColor color = QColorDialog::getColor(this->settings->value("lastCustomColor", QColor(Qt::black)).value<QColor>(), this, tr("Choose Background Colour"));
			if (color.isValid()) {
				this->settings->setValue("lastCustomColor", color);
				this->settings->setValue("backgroundColor", color);
				this->imageView->setInterfaceBackgroundColor(color);
			} else {
				if (this->imageView->getInterfaceBackgroundColor() == Qt::black) {
					this->backgroundColorBlackAction->setChecked(true);
				} else if (this->imageView->getInterfaceBackgroundColor() == this->darkGray) {
					this->backgroundColorGrayAction->setChecked(true);
				} else if (this->imageView->getInterfaceBackgroundColor() == Qt::white) {
					this->backgroundColorWhiteAction->setChecked(true);
				}
			}
		}
	}

	void MainInterface::toggleAutoRotation(bool value) {
		this->settings->setValue("autoRotateImages", value);
		if (value) {
			this->autoRotateImage();
		} else {
			this->imageView->setRotation(userRotation);
		}
	}

	void MainInterface::triggerCustomAction1() {
		if (this->filesInDirectory.size() > 0 && !this->loading) {
			this->loading = true;
			if (this->hotkeyDialog->getAction1() == 0) {
				this->statusHint = tr("Deleting...");
				this->update();
				this->deleteCurrentImage(this->hotkeyDialog->getShowConfirmation(), this->hotkeyDialog->getIncludeSidecarFiles());
			} else if (this->hotkeyDialog->getAction1() == 1) {
				this->statusHint = tr("Moving...");
				this->update();
				this->moveCurrentImage(this->hotkeyDialog->getFolder1(), this->hotkeyDialog->getShowConfirmation(), this->hotkeyDialog->getIncludeSidecarFiles());
			} else if (this->hotkeyDialog->getAction1() == 2) {
				this->statusHint = tr("Copying...");
				this->update();
				this->copyCurrentImage(this->hotkeyDialog->getFolder1(), this->hotkeyDialog->getShowConfirmation(), this->hotkeyDialog->getIncludeSidecarFiles());
			}
			this->statusHint = QString();
			this->loading = false;
			this->update();
		}
	}

	void MainInterface::triggerCustomAction2() { 
		if (this->filesInDirectory.size() > 0 && !this->loading) {
			this->loading = true;
			if (this->hotkeyDialog->getAction2() == 0) {
				this->statusHint = tr("Deleting...");
				this->update();
				this->deleteCurrentImage(this->hotkeyDialog->getShowConfirmation(), this->hotkeyDialog->getIncludeSidecarFiles(), this->hotkeyDialog->getSidecarType());
			} else if (this->hotkeyDialog->getAction2() == 1) {
				this->statusHint = tr("Moving...");
				this->update();
				this->moveCurrentImage(this->hotkeyDialog->getFolder2(), this->hotkeyDialog->getShowConfirmation(), this->hotkeyDialog->getIncludeSidecarFiles(), this->hotkeyDialog->getSidecarType());
			} else if (this->hotkeyDialog->getAction2() == 2) {
				this->statusHint = tr("Copying...");
				this->update();
				this->copyCurrentImage(this->hotkeyDialog->getFolder2(), this->hotkeyDialog->getShowConfirmation(), this->hotkeyDialog->getIncludeSidecarFiles(), this->hotkeyDialog->getSidecarType());
			}
			this->statusHint = QString();
			this->loading = false;
			this->update();
		}
	}

	void MainInterface::updateCustomHotkeys() {
		this->fileActionAction->setChecked(this->hotkeyDialog->getHotkeysEnabled());
		this->customAction1->setEnabled(this->hotkeyDialog->getHotkeysEnabled() && this->hotkeyDialog->getHotkey1Enabled());
		this->customAction2->setEnabled(this->hotkeyDialog->getHotkeysEnabled() && this->hotkeyDialog->getHotkey2Enabled());
		this->customAction1->setShortcut(this->hotkeyDialog->getKeySequence1());
		this->customAction2->setShortcut(this->hotkeyDialog->getKeySequence2());
	}

}