#include "MainInterface.h"

namespace sv {

	MainInterface::MainInterface(QWidget *parent)
		: QWidget(parent) {

		setAcceptDrops(true);
	}

	MainInterface::~MainInterface() {

	}

	QSize MainInterface::sizeHint() const {
		return QSize(1053, 570);
	}

}