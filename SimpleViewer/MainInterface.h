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

namespace sv {

	class Image {
	public:
		Image();
		Image(cv::Mat mat, std::shared_ptr<ExifData> exifData);
		cv::Mat mat() const;
		std::shared_ptr<ExifData> exif() const;
		bool isValid() const;
	private:
		bool valid = false;
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
		void enterFullscreen();
		void exitFullscreen();
		void infoPaintFunction(QPainter& canvas);
		bool applicationIsInstalled();
		void changeFontSizeBy(int value);
		void changeLineSpacingBy(int value);
		void loadSettings();

		//variables
		const QString programTitle = "Simple Viewer";
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

		//widgets
		hb::ImageView* imageView;
		SlideshowDialog* slideshowDialog;
		SharpeningDialog* sharpeningDialog;
		//menus
		QMenu* fileMenu;
		QMenu* viewMenu;
		QMenu* slideshowMenu;
		QMenu* applicationMenu;
		//actions
		QAction* quitAction;
		QAction* openAction;
		QAction* refreshAction;
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
		QAction* zoomTo100Action;
		QAction* zoomToFitAction;
		QAction* slideshowAction;
		QAction* slideshowNoDialogAction;
		QAction* zoomLevelAction;
		QAction* installAction;
		QAction* uninstallAction;
		//timer
		QTimer* mouseHideTimer;
		QTimer* threadCleanUpTimer;
		QTimer* slideshowTimer;
	private slots:
		void nextSlide();
		void refresh();
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
		void showSharpeningOptions();
		void updateSharpening();
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