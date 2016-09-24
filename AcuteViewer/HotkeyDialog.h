#ifndef SV_SHARPENINGDIALOG
#define SV_SHARPENINGDIALOG

#include <iostream>
#include <memory>

//Qt
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>

namespace sv {

	class HotkeyDialog : public QDialog {
		Q_OBJECT
	public:
		HotkeyDialog(std::shared_ptr<QSettings> settings, QWidget* parent = 0);
	protected:
		void showEvent(QShowEvent* event);
	private:
		//functions

		//variables
		std::shared_ptr<QSettings> settings;
		bool enableHotkeysOldValue;
		bool enableHotkey1OldValue;
		bool enableHotkey2OldValue;
		QKeySequence keySequence1OldValue;
		QKeySequence keySequence2OldValue;
		int action1OldValue;
		int action2OldValue;
		//widgets
		QVBoxLayout* mainLayout;
		QFormLayout* formLayout1;
		QFormLayout* formLayout2;
		QGroupBox* groupBox1;
		QGroupBox* groupBox2;
		QLineEdit* lineEdit1;
		QLineEdit* lineEdit2;
		QButtonGroup* buttonGroup1;
		QButtonGroup* buttonGroup2;
		QRadioButton* binRadioButton1;
		QRadioButton* binRadioButton2;
		QRadioButton* moveRadioButton1;
		QRadioButton* moveRadioButton2;
		QRadioButton* copyRadioButton1;
		QRadioButton* copyRadioButton2;
		QLineEdit* folderLineEdit1;
		QLineEdit* folderLineEdit2;
		QLineEdit* chooseButton1;
		QLineEdit* chooseButton2;
		QHBoxLayout* buttonLayout;
		QPushButton* okButton;
		QPushButton* cancelButton;
	private slots:
		void reactToOkButtonClick();
	private slots:
		void updateHotkeySettings();
		void reactToCancelButtonClick();
	};
}
#endif