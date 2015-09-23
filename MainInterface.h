#ifndef CT_MAININTERFACE
#define CT_MAININTERFACE

#include "ImageView.h"

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
	private:
		//functions
		void loadImage(QString const& path);

		//variables
		hb::ImageView* imageView;
		cv::Mat image;
	};
}
#endif