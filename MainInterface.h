#ifndef CT_MAININTERFACE
#define CT_MAININTERFACE

#include "ImageView.h"

#include <iostream>
#include <future>
#include <chrono>
#include <fstream>

//OpenCV
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

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
		void clearThreads();
		void waitForThreadToFinish(std::shared_future<cv::Mat> const& thread);
		size_t nextFileIndex() const;
		size_t previousFileIndex() const;
		static bool isASCII(QString const& string);
		QString getFullImagePath(size_t index) const;
		void loadImage(QString path);
		void displayImageIfOk();
		void loadNextImage();
		void loadPreviousImage();
		void enterFullscreen();
		void exitFullscreen();
		void infoPaintFunction(QPainter& canvas);

		//variables
		const QString programTitle = "Simple Viewer";
		const int mouseHideDelay = 1000;
		const int eventProcessIntervalDuringWait = 16;
		cv::Mat image;
		std::atomic<bool> loading{ false };
		QDir currentDirectory;
		QVector<QString> filesInDirectory;
		size_t fileIndex;
		QFileInfo currentFileInfo;
		bool currentImageUnreadable = false;
		QMap<QString, std::shared_future<cv::Mat>> threads;
		QSettings settings;

		//widgets
		hb::ImageView* imageView;
		//menus
		QMenu* fileMenu;
		QMenu* viewMenu;
		//actions
		QAction* quitAction;
		QAction* openAction;
		QAction* showInfoAction;
		QAction* smoothingAction;
		QAction* enlargementAction;
		QAction* menuBarAutoHideAction;
		//timer
		QTimer* mouseHideTimer;
	private slots:
		void quit();
		void hideMouse() const;
		void showMouse() const;
		void showMenuBar();
		void hideMenuBar(QAction* triggeringAction = nullptr);
		void reactToshowInfoToggle(bool value);
		void reactToReadImageCompletion(cv::Mat image);
		void openDialog();
		void reactToSmoothingToggle(bool value);
		void reactToEnlargementToggle(bool value);
		void reactoToAutoHideMenuBarToggle(bool value);
	signals:
		void readImageFinished(cv::Mat image);
	};
}
#endif