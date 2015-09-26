#ifndef CT_MAININTERFACE
#define CT_MAININTERFACE

#include "ImageView.h"

#include <iostream>
#include <future>
#include <chrono>

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
		cv::Mat readImage(QString path) const;
		QString getFullImagePath(size_t index) const;
		void loadImage(QString path);
		void displayImageIfOk();
		void loadNextImage();
		void loadPreviousImage();
		void enterFullscreen();
		void exitFullscreen();
		void infoPaintFunction(QPainter& canvas);

		//variables
		QString programTitle = "Simple Viewer";
		cv::Mat image;
		QDir currentDirectory;
		QVector<QString> filesInDirectory;
		size_t fileIndex;
		QString nameOfCurrentFile;
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
		//actions
		QAction* quitAction;
		//timer
		QTimer* mouseHideTimer;
	private slots:
		void quit();
		void hideMouse() const;
		void showMouse() const;
	};
}
#endif