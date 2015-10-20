#ifndef SV_MAININTERFACE
#define SV_MAININTERFACE

#include <iostream>
#include <future>
#include <chrono>
#include <fstream>

//OpenCV
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "ImageView.h"
#include "SlideshowDialog.h"
#include "SharpeningDialog.h"

namespace sv {

	class MainInterface : public QMainWindow {
		Q_OBJECT
	public:
		MainInterface(QString openWithFilename, QWidget *parent = 0);
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
	private:
		//functions
		cv::Mat readImage(QString path, bool emitSignals = false);
		void loadNextImage();
		void loadPreviousImage();
		void clearThreads();
		void waitForThreadToFinish(std::shared_future<cv::Mat> const& thread);
		size_t nextFileIndex() const;
		size_t previousFileIndex() const;
		static bool isASCII(QString const& string);
		QString getFullImagePath(size_t index) const;
		void loadImage(QString path);
		void displayImageIfOk();
		void enterFullscreen();
		void exitFullscreen();
		void infoPaintFunction(QPainter& canvas);
		bool applicationIsInstalled();

		//variables
		const QString programTitle = "Simple Viewer";
		const int mouseHideDelay = 1000;
		const int threadCleanUpInterval = 500;
		const int eventProcessIntervalDuringWait = 16;
		cv::Mat image;
		std::atomic<bool> loading{ false };
		std::mutex threadDeletionMutex;
		QDir currentDirectory;
		bool noCurrentDir = true;
		QVector<QString> filesInDirectory;
		size_t currentFileIndex;
		QFileInfo currentFileInfo;
		std::atomic<bool> currentImageUnreadable{ false };
		std::atomic<bool> paintLoadingHint{ false };
		std::map<QString, std::shared_future<cv::Mat>> threads;
		std::shared_ptr<QSettings> settings;

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
		QAction* showInfoAction;
		QAction* smoothingAction;
		QAction* enlargementAction;
		QAction* sharpeningAction;
		QAction* sharpeningOptionsAction;
		QAction* menuBarAutoHideAction;
		QAction* slideshowAction;
		QAction* installAction;
		QAction* uninstallAction;
		//timer
		QTimer* mouseHideTimer;
		QTimer* threadCleanUpTimer;
		QTimer* slideshowTimer;
		private slots:
		void nextSlide();
		void cleanUpThreads();
		void quit();
		void hideMouse() const;
		void showMouse() const;
		void enableAutomaticMouseHide();
		void disableAutomaticMouseHide();
		void showMenuBar();
		void hideMenuBar(QAction* triggeringAction = nullptr);
		void populateApplicationMenu();
		void runInstaller();
		void runUninstaller();
		void toggleSlideshow();
		void startSlideshow(double delay, bool loop);
		void stopSlideshow();
		void reactToshowInfoToggle(bool value);
		void reactToReadImageCompletion(cv::Mat image);
		void openDialog();
		void reactToSmoothingToggle(bool value);
		void reactToEnlargementToggle(bool value);
		void reactToSharpeningToggle(bool value);
		void reactoToAutoHideMenuBarToggle(bool value);
	signals:
		void readImageFinished(cv::Mat image);
	};
}
#endif