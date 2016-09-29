#ifndef SV_HOTKEYDIALOG
#define SV_HOTKEYDIALOG

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
		bool getHotkeysEnabled();
		bool getHotkey1Enabled();
		bool getHotkey2Enabled();
		QKeySequence const& getKeySequence1();
		QKeySequence const& getKeySequence2();
		int getAction1();
		int getAction2();
		QString const& getFolder1();
		QString const& getFolder2();
	protected:
		void showEvent(QShowEvent* event);
	private:
		//functions
		void loadSettings();
		//variables
		std::shared_ptr<QSettings> settings;
		bool enableHotkeysOldValue;
		bool enableHotkey1OldValue;
		bool enableHotkey2OldValue;
		QKeySequence keySequence1OldValue;
		QKeySequence keySequence2OldValue;
		int action1OldValue;
		int action2OldValue;
		QString folder1OldValue;
		QString folder2OldValue;
		//widgets
		QVBoxLayout* mainLayout;
		QGroupBox* globalGroupBox;
		QVBoxLayout* globalGroupBoxLayout;
		QFormLayout* formLayout1;
		QFormLayout* formLayout2;
		QGroupBox* groupBox1;
		QGroupBox* groupBox2;
		QKeySequenceEdit* keySequenceEdit1;
		QKeySequenceEdit* keySequenceEdit2;
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
		QCompleter* completer1;
		QCompleter* completer2;
		QPushButton* chooseButton1;
		QPushButton* chooseButton2;
		QHBoxLayout* buttonLayout;
		QPushButton* okButton;
		QPushButton* cancelButton;
	private slots:
		void reactToOkButtonClick();
	private slots:
		void resetChanges();
		void reactToCheckboxChange();
		void verifyChanges();
		void selectFolder();
	};
}
#endif