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
		MainInterface(QWidget *parent = 0);
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
		static bool isASCII(QString const& string);
		QString getFullImagePath(size_t index) const;
		void loadImage(QString path);
		void displayImageIfOk();
		void loadNextImage();
		void loadPreviousImage();
		void enterFullscreen();
		void exitFullscreen();
		void showMenuBar();

		//variables
		const QString programTitle = "Simple Viewer";
		const int mouseHideDelay = 1000;
		cv::Mat image;
		std::atomic<bool> loading{ false };
		QDir currentDirectory;
		QVector<QString> filesInDirectory;
		size_t fileIndex;
		QFileInfo currentFileInfo;
		bool currentImageUnreadable = false;
		cv::Mat previousImage;
		cv::Mat nextImage;
		std::future<cv::Mat> previousImageThread;
		std::future<cv::Mat> nextImageThread;
		std::atomic<bool> previousImageCached{ false };
		std::atomic<bool> nextImageCached{ false };

		//widgets
		hb::ImageView* imageView;
		//menus
		QMenu* fileMenu;
		QMenu* viewMenu;
		//actions
		QAction* quitAction;
		QAction* showInfoAction;
		//timer
		QTimer* mouseHideTimer;
	private slots:
		void quit();
		void hideMouse() const;
		void showMouse() const;
		void reactToshowInfoToggle(bool value);
		void hideMenuBar(QAction* triggeringAction = nullptr);
		void infoPaintFunction(QPainter& canvas);
		void reactToReadImageCompletion(cv::Mat image);
	signals:
		void readImageFinished(cv::Mat image);
	};
}
#endif