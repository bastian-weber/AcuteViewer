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
		QKeySequence getKeySequence1();
		QKeySequence getKeySequence2();
		int getAction1();
		int getAction2();
		QString getFolder1();
		QString getFolder2();
		bool getIncludeSidecarFiles();
		int getSidecarType();
		bool getShowConfirmation();
	public slots:
		void setHotkeysEnabled(bool value);
	private:
		//functions
		void loadSettings();
		void saveState();
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
		bool sidecarFilesOldValue;
		int sidecarTypeOldValue;
		bool confirmationOldValue;
		//widgets
		QVBoxLayout* mainLayout;
		QGroupBox* globalGroupBox;
		QVBoxLayout* globalGroupBoxLayout;
		QFormLayout* formLayout1;
		QFormLayout* formLayout2;
		QVBoxLayout* optionsLayout;
		QVBoxLayout* sidecarLayout;
		QGroupBox* groupBox1;
		QGroupBox* groupBox2;
		QGroupBox* optionsGroupBox;
		QGroupBox* sidecarFileGroupBox;
		QCheckBox* confirmationCheckBox;
		QKeySequenceEdit* keySequenceEdit1;
		QKeySequenceEdit* keySequenceEdit2;
		QButtonGroup* buttonGroup1;
		QButtonGroup* buttonGroup2;
		QButtonGroup* sidecarButtonGroup;
		QRadioButton* binRadioButton1;
		QRadioButton* binRadioButton2;
		QRadioButton* moveRadioButton1;
		QRadioButton* moveRadioButton2;
		QRadioButton* copyRadioButton1;
		QRadioButton* copyRadioButton2;
		QRadioButton* sidecarAllRadioButton;
		QRadioButton* sidecarXmpRadioButton;
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
		void resetChanges();
		void reactToCheckboxChange();
		bool verifyChanges();
		void selectFolder();
	};
}
#endif