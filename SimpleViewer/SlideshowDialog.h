#ifndef SV_SLIDESHOWDIALOG
#define SV_SLIDESHOWDIALOG

#include <iostream>

//Qt
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>

namespace sv {

	class SlideshowDialog : public QDialog {
		Q_OBJECT
	public:
		SlideshowDialog(QWidget *parent = 0);
		~SlideshowDialog();
		//QSize sizeHint() const;
	protected:

	private:
		//functions

		//variables

		//widgets
		QVBoxLayout* mainLayout;
		QFormLayout* formLayout;
		QHBoxLayout* buttonLayout;
		QDoubleSpinBox* timeSpinBox;
		QCheckBox* loopCheckbox;
		QPushButton* okButton;
		QPushButton* cancelButton;
	private slots:
		void reactToOkButtonClick();
	signals:
		void dialogConfirmed(double delay, bool loop);
	};
}
#endif