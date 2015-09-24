#ifndef CT_MAININTERFACE
#define CT_MAININTERFACE

#include "ImageView.h"

#include <iostream>

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
		void dragEnterEvent(QDragEnterEvent* e);
		void dropEvent(QDropEvent* e);
		void keyPressEvent(QKeyEvent* e);
	private:
		//functions
		void loadImage(QString path = QString());
		void nextImage();
		void previousImage();

		//variables
		cv::Mat image;
		QDir currentDirectory;
		QVector<QString> filesInDirectory;
		size_t fileIndex;
		//widgets
		hb::ImageView* imageView;

	};
}
#endif