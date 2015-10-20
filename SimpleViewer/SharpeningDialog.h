#ifndef SV_SHARPENINGDIALOG
#define SV_SHARPENINGDIALOG

#include <iostream>
#include <memory>

//Qt
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>

namespace sv {

	class SharpeningDialog : public QDialog {
		Q_OBJECT
	public:
		SharpeningDialog(std::shared_ptr<QSettings> settings, QWidget *parent = 0);
		~SharpeningDialog();
		//QSize sizeHint() const;
	protected:

	private:
		//functions

		//variables
		std::shared_ptr<QSettings> settings;
		//widgets
		QVBoxLayout* mainLayout;
		QFormLayout* formLayout;
		QHBoxLayout* buttonLayout;
		QDoubleSpinBox* strengthSpinBox;
		QDoubleSpinBox* radiusSpinBox;
		QPushButton* okButton;
		QPushButton* cancelButton;
	private slots:
		void reactToOkButtonClick();
	signals:
		void dialogClosed();
	};
}
#endif