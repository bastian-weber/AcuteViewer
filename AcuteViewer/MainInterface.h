#ifndef SV_MAININTERFACE
#define SV_MAININTERFACE

#include <iostream>
#include <future>
#include <chrono>
#include <memory>

//OpenCV
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "utility.h"
#include "ExifData.h"
#include "ImageView.h"
#include "SlideshowDialog.h"
#include "SharpeningDialog.h"
#include "HotkeyDialog.h"

namespace sv {

	class Image {
	public:
		Image();
		Image(cv::Mat mat, std::shared_ptr<ExifData> exifData, bool isPreviewImage = false);
		cv::Mat mat() const;
		std::shared_ptr<ExifData> exif() const;
		bool isValid() const;
		bool isPreviewImage() const;
	private:
		bool valid = false;
		bool previewImage = false;
		cv::Mat matrix;
		std::shared_ptr<ExifData> exifData;
	};

	class MainInterface : public QMainWindow {
		Q_OBJECT
	public:
		MainInterface(QString openWithFilename, QWidget *parent = 0);
		MainInterface(QStringList openWithFilenames, QWidget *parent = 0);
		~MainInterface();
		QSize sizeHint() const;
	protected:
		bool eventFilter(QObject* object, QEvent* e);
		void dragEnterEvent(QDragEnterEvent* e);
		void dropEvent(QDropEvent* e);
		void keyPressEvent(QKeyEvent* e);
		void keyReleaseEvent(QKeyEvent* e);
		void mouseDoubleClickEvent(QMouseEvent* e);
		void mouseReleaseEvent(QMouseEvent* e);
		void mouseMoveEvent(QMouseEvent* e);
		void changeEvent(QEvent* e);
		void wheelEvent(QWheelEvent* e);
	private:
		//functions
		void initialize();
		std::shared_future<Image>& currentThread();
		bool exifIsRequired() const;
		Image readImage(QString path, bool emitSignals = false);
		void loadNextImage();
		void loadPreviousImage();
		void clearThreads();
		template <typename T> void waitForThreadToFinish(std::shared_future<T> const& thread, bool indicateLoading = true);
		size_t nextFileIndex() const;
		size_t previousFileIndex() const;
		QString getFullImagePath(size_t index) const;
		void loadImage(QString path);
		void loadImages(QStringList paths);
		void displayImageIfOk();
		void autoRotateImage();
		void enterFullscreen();
		void exitFullscreen();
		void infoPaintFunction(QPainter& canvas);
		bool applicationIsInstalled();
		void changeFontSizeBy(int value);
		void changeLineSpacingBy(int value);
		void loadSettings();

		//variables
		const QString programTitle = "Acute Viewer";
		const QStringList supportedExtensions = { "*.bmp", "*.dib", "*.jpeg", "*.jpg", "*.jpe", "*.jpeg", "*.jp2", "*.png", "*.webp", "*.pbm", "*.pgm", "*.ppm", "*.sr", "*.ras", "*.tiff", "*.tif" };
		const QStringList partiallySupportedExtensions = { "*.arw", "*.dng", "*.psd", "*.nef", "*.cr2", "*.crw", "*.mrw", "*.pef", "*.rw2", "*.sr2", "*.srf", "*.srw", "*.orf", "*.pgf", "*.raf"};
		const int mouseHideDelay = 1000;
		const int threadCleanUpInterval = 500;
		const int eventProcessIntervalDuringWait = 16;
		Image image;
		std::atomic<bool> loading{ false };
		std::mutex threadDeletionMutex;
		QDir currentDirectory;
		bool noCurrentDir = true;
		QVector<QString> filesInDirectory;
		long currentFileIndex = -1;
		QString currentThreadName;
		QFileInfo currentFileInfo;
		std::atomic<bool> currentImageUnreadable{ false };
		std::atomic<bool> paintLoadingHint{ false };
		std::map<QString, std::shared_future<Image>> threads;
		std::shared_ptr<QSettings> settings;
		bool skipNextAltRelease = false;
		unsigned int fontSize;
		unsigned int lineSpacing;
		QColor const darkGray = QColor(30, 30, 30);
		double userRotation = 0;

		//widgets
		hb::ImageView* imageView;
		SlideshowDialog* slideshowDialog;
		SharpeningDialog* sharpeningDialog;
		HotkeyDialog* hotkeyDialog;
		//menus
		QMenu* fileMenu;
		QMenu* viewMenu;
		QMenu* zoomMenu;
		QMenu* rotationMenu;
		QMenu* sharpeningMenu;
		QMenu* backgroundColorMenu;
		QMenu* slideshowMenu;
		QMenu* applicationMenu;
		//actions
		QAction* quitAction;
		QAction* openAction;
		QAction* refreshAction;
		QAction* hotkeyOptionsAction;
		QAction* resetSettingsAction;
		QAction* showInfoAction;
		QAction* smoothingAction;
		QAction* enlargementAction;
		QAction* sharpeningAction;
		QAction* sharpeningOptionsAction;
		QAction* menuBarAutoHideAction;
		QAction* saveSizeAction;
		QAction* gpuAction;
		QAction* fullscreenAction;
		QAction* rotateLeftAction;
		QAction* rotateRightAction;
		QAction* resetRotationAction;
		QAction* autoRotationAction;
		QAction* zoomTo100Action;
		QAction* zoomToFitAction;
		QAction* slideshowAction;
		QAction* slideshowNoDialogAction;
		QAction* zoomLevelAction;
		QAction* backgroundColorCustomAction;
		QAction* backgroundColorWhiteAction;
		QAction* backgroundColorBlackAction;
		QAction* backgroundColorGrayAction;
		QAction* includePartiallySupportedFilesAction;
		QAction* installAction;
		QAction* uninstallAction;
		QAction* customAction1;
		QAction* customAction2;
		QActionGroup* backgroundColorActionGroup;
		//timer
		QTimer* mouseHideTimer;
		QTimer* threadCleanUpTimer;
		QTimer* slideshowTimer;
	private slots:
		void nextSlide();
		void refresh();
		void showHotkeyDialog();
		void cleanUpThreads();
		void quit();
		void saveWindowSize();
		void hideMouse() const;
		void showMouse() const;
		void enableAutomaticMouseHide();
		void disableAutomaticMouseHide();
		void showMenuBar();
		void hideMenuBar(QAction* triggeringAction = nullptr);
		void populateApplicationMenu();
		void runInstaller();
		void runUninstaller();
		void resetSettings();
		void toggleSlideshow();
		void toggleSlideshowNoDialog();
		void startSlideshow();
		void stopSlideshow();
		void toggleFullscreen();
		void rotateLeft();
		void rotateRight();
		void resetRotation();
		void zoomTo100();
		void toggleGpu(bool value);
		void toggleInfoOverlay(bool value);
		void toggleZoomLevelOverlay(bool value);
		void reactToReadImageCompletion(Image image);
		void reactToExifLoadingCompletion(ExifData* sender);
		void openDialog();
		void toggleEnglargmentInterpolationMethod(bool value);
		void toggleSmallImageUpscaling(bool value);
		void toggleSharpening(bool value);
		void toggleMenuBarAutoHide(bool value);
		void togglePreviewOnlyFiles(bool value);
		void showSharpeningOptions();
		void updateSharpening();
		void changeBackgroundColor(QAction* action);
		void toggleAutoRotation(bool value);
		void triggerCustomAction1();
		void triggerCustomAction2();
		void updateCustomHotkeys();
	signals:
		void readImageFinished(Image image);
	};

	//================================= Implementation of Template Functions =================================\\

	template <typename T> void MainInterface::waitForThreadToFinish(std::shared_future<T> const& thread, bool indicateLoading) {
		if (!thread.valid()) return;
		if (thread.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
			if(indicateLoading) this->setWindowTitle(this->windowTitle() + QString(tr(" - Loading...")));
		}

		while (thread.wait_for(std::chrono::milliseconds(this->eventProcessIntervalDuringWait)) != std::future_status::ready) {
			qApp->processEvents();
		}
	}

}

#endif