#ifndef CT_MAININTERFACE
#define CT_MAININTERFACE

#include "ImageView.h"

namespace sv {

	class MainInterface : public QWidget {
		Q_OBJECT
	public:
		MainInterface(QWidget *parent = 0);
		~MainInterface();
		QSize sizeHint() const;
	};
}
#endif