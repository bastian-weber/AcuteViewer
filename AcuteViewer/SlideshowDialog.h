#ifndef SV_SLIDESHOWDIALOG
#define SV_SLIDESHOWDIALOG

#include <iostream>
#include <memory>

//Qt
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>

namespace sv {

	class SlideshowDialog : public QDialog {
		Q_OBJECT
	public:
		SlideshowDialog(std::shared_ptr<QSettings> settings, QWidget* parent = 0);
		~SlideshowDialog();
	protected:

	private:
		//functions

		//variables
		std::shared_ptr<QSettings> settings;
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
	};
}
#endif