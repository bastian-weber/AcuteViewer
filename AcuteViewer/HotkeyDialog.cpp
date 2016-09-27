#include "HotkeyDialog.h"

namespace sv {

	HotkeyDialog::HotkeyDialog(std::shared_ptr<QSettings> settings, QWidget* parent)
		: settings(settings),
		QDialog(parent) {
		this->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
		this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

		this->setWindowTitle(tr("Hotkey Options"));

		//first hotkey

		this->keySequenceEdit1 = new QKeySequenceEdit(this);
		QObject::connect(this->keySequenceEdit1, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(verifyChanges()));
		this->binRadioButton1 = new QRadioButton(tr("Move to recycle bin"), this);
		this->moveRadioButton1 = new QRadioButton(tr("Move to folder..."), this);
		this->copyRadioButton1 = new QRadioButton(tr("Copy to folder..."), this);
		this->buttonGroup1 = new QButtonGroup(this);
		this->buttonGroup1->addButton(binRadioButton1, 0);
		this->buttonGroup1->addButton(moveRadioButton1, 1);
		this->buttonGroup1->addButton(copyRadioButton1, 2);
		QObject::connect(this->buttonGroup1, SIGNAL(buttonClicked(int)), this, SLOT(reactToCheckboxChange()));
		QObject::connect(this->buttonGroup1, SIGNAL(buttonClicked(int)), this, SLOT(verifyChanges()));
		this->folderLineEdit1 = new QLineEdit(this);
		this->folderLineEdit1->setMinimumWidth(250);
		QObject::connect(this->folderLineEdit1, SIGNAL(textChanged(QString)), this, SLOT(verifyChanges()));
		this->completer1 = new QCompleter(this);
		QDirModel* dirModel1 = new QDirModel(completer1);
		dirModel1->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
		completer1->setModel(dirModel1);
		this->folderLineEdit1->setCompleter(completer1);
		this->chooseButton1 = new QPushButton(tr("Choose"), this);
		this->chooseButton1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
		QObject::connect(this->chooseButton1, SIGNAL(clicked()), this, SLOT(selectFolder()));

		this->formLayout1 = new QFormLayout();
		this->formLayout1->setFormAlignment(Qt::AlignCenter);
		this->formLayout1->addRow(tr("Key:"), this->keySequenceEdit1);
		this->formLayout1->addRow(tr("Action:"), this->binRadioButton1);
		this->formLayout1->addRow("", this->moveRadioButton1);
		this->formLayout1->addRow("", this->copyRadioButton1);
		this->formLayout1->addRow(tr("Folder:"), this->folderLineEdit1);
		this->formLayout1->addRow("", this->chooseButton1);

		this->groupBox1 = new QGroupBox(tr("Hotkey 1"), this);
		this->groupBox1->setCheckable(true);
		this->groupBox1->setLayout(this->formLayout1);
		QObject::connect(this->groupBox1, SIGNAL(toggled(bool)), this, SLOT(verifyChanges()));

		//second hotkey

		this->keySequenceEdit2 = new QKeySequenceEdit(this);
		QObject::connect(this->keySequenceEdit2, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(verifyChanges()));
		this->binRadioButton2 = new QRadioButton(tr("Move to recycle bin"), this);
		this->moveRadioButton2 = new QRadioButton(tr("Move to folder..."), this);
		this->copyRadioButton2 = new QRadioButton(tr("Copy to folder..."), this);
		this->buttonGroup2 = new QButtonGroup(this);
		this->buttonGroup2->addButton(binRadioButton2, 0);
		this->buttonGroup2->addButton(moveRadioButton2, 1);
		this->buttonGroup2->addButton(copyRadioButton2, 2);
		QObject::connect(this->buttonGroup2, SIGNAL(buttonClicked(int)), this, SLOT(reactToCheckboxChange()));
		QObject::connect(this->buttonGroup2, SIGNAL(buttonClicked(int)), this, SLOT(verifyChanges()));
		this->folderLineEdit2 = new QLineEdit(this);
		this->folderLineEdit2->setMinimumWidth(250);
		QObject::connect(this->folderLineEdit2, SIGNAL(textChanged(QString)), this, SLOT(verifyChanges()));
		this->completer2 = new QCompleter(this);
		QDirModel* dirModel2 = new QDirModel(completer2);
		dirModel2->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
		this->completer2->setModel(dirModel2);
		this->folderLineEdit2->setCompleter(this->completer2);
		this->chooseButton2 = new QPushButton(tr("Choose"), this);
		this->chooseButton2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
		QObject::connect(this->chooseButton2, SIGNAL(clicked()), this, SLOT(selectFolder()));

		this->formLayout2 = new QFormLayout();
		this->formLayout2->setFormAlignment(Qt::AlignCenter);
		this->formLayout2->addRow(tr("Key:"), this->keySequenceEdit2);
		this->formLayout2->addRow(tr("Action:"), this->binRadioButton2);
		this->formLayout2->addRow("", this->moveRadioButton2);
		this->formLayout2->addRow("", this->copyRadioButton2);
		this->formLayout2->addRow(tr("Folder:"), this->folderLineEdit2);
		this->formLayout2->addRow("", this->chooseButton2);

		this->groupBox2 = new QGroupBox(tr("Hotkey 2"), this);
		this->groupBox2->setCheckable(true);
		this->groupBox2->setLayout(this->formLayout2);
		QObject::connect(this->groupBox2, SIGNAL(toggled(bool)), this, SLOT(verifyChanges()));

		this->globalGroupBoxLayout = new QVBoxLayout;
		this->globalGroupBoxLayout->addWidget(groupBox1);
		this->globalGroupBoxLayout->addWidget(groupBox2);

		this->globalGroupBox = new QGroupBox(tr("Enable Hotkeys"), this);
		this->globalGroupBox->setCheckable(true);
		this->globalGroupBox->setLayout(this->globalGroupBoxLayout);
		QObject::connect(this->globalGroupBox, SIGNAL(toggled(bool)), this, SLOT(verifyChanges()));

		this->okButton = new QPushButton(tr("Ok"), this);
		QObject::connect(this->okButton, SIGNAL(clicked()), this, SLOT(reactToOkButtonClick()));
		this->cancelButton = new QPushButton(tr("Cancel"), this);
		QObject::connect(this->cancelButton, SIGNAL(clicked()), this, SLOT(reactToCancelButtonClick()));
		QObject::connect(this->cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

		this->buttonLayout = new QHBoxLayout();
		this->buttonLayout->addStretch(1);
		this->buttonLayout->addWidget(okButton);
		this->buttonLayout->addWidget(cancelButton);

		this->mainLayout = new QVBoxLayout();
		this->mainLayout->addWidget(this->globalGroupBox);
		this->mainLayout->addLayout(this->buttonLayout);

		this->setLayout(this->mainLayout);
		this->layout()->setSizeConstraint(QLayout::SetFixedSize);

		this->loadSettings();
	}

	//============================================================================== PROTECTED ==============================================================================\\

	void HotkeyDialog::showEvent(QShowEvent* event) {
		this->enableHotkeysOldValue = this->globalGroupBox->isChecked();
		this->enableHotkey1OldValue = this->groupBox1->isChecked();
		this->enableHotkey2OldValue = this->groupBox2->isChecked();
		this->keySequence1OldValue = this->keySequenceEdit1->keySequence();
		this->keySequence2OldValue = this->keySequenceEdit2->keySequence();
		this->action1OldValue = this->buttonGroup1->checkedId();
		this->action2OldValue = this->buttonGroup2->checkedId();
		this->folder1OldValue = this->folderLineEdit1->text();
		this->folder2OldValue = this->folderLineEdit2->text();
	}

	//=============================================================================== PRIVATE ===============================================================================\\

	void HotkeyDialog::loadSettings() {
		{
			this->keySequenceEdit1->setKeySequence(this->settings->value("hotkey1Shortcut", QKeySequence(Qt::Key_Delete)).value<QKeySequence>());
			this->buttonGroup1->button(this->settings->value("hotkey1Action", 0).toInt())->setChecked(true);
			QDir dir(this->settings->value("hotkey1Folder", QDir::rootPath()).toString());
			QString path = QDir::rootPath();
			if (dir.exists()) path = QDir::cleanPath(dir.absolutePath());
			this->folderLineEdit1->setText(path);
		}

		{
			this->keySequenceEdit2->setKeySequence(this->settings->value("hotkey2Shortcut", QKeySequence(Qt::Key_Enter)).value<QKeySequence>());
			this->buttonGroup2->button(this->settings->value("hotkey2Action", 0).toInt())->setChecked(true);
			QDir dir(this->settings->value("hotkey2Folder", QDir::rootPath()).toString());
			QString path = QDir::rootPath();
			if (dir.exists()) path = QDir::cleanPath(dir.absolutePath());
			this->folderLineEdit2->setText(path);
		}
		this->globalGroupBox->setChecked(this->settings->value("enableHotkeys", true).toBool());
		this->groupBox1->setChecked(this->settings->value("enableHotkey1", true).toBool());
		this->groupBox2->setChecked(this->settings->value("enableHotkey2", false).toBool());
		this->reactToCheckboxChange();
	}

	//============================================================================ PRIVATE SLOTS =============================================================================\\

	void HotkeyDialog::reactToOkButtonClick() {
		if (this->groupBox1->isChecked() && this->groupBox2->isChecked() && this->keySequenceEdit1->keySequence() == this->keySequenceEdit2->keySequence()) {
			return;
		}
		settings->setValue("enableHotkeys", this->globalGroupBox->isChecked());
		if (this->globalGroupBox->isChecked()) {
			settings->setValue("enableHotkey1", this->groupBox1->isChecked());
			settings->setValue("enableHotkey2", this->groupBox2->isChecked());
			if (this->groupBox1->isChecked()) {
				settings->setValue("hotkey1Shortcut", this->keySequenceEdit1->keySequence());
				settings->setValue("hotkey1Action", this->buttonGroup1->checkedId());
				if (this->buttonGroup1->checkedId() != 0) {
					settings->setValue("hotkey1Folder", this->folderLineEdit1->text());
				}
			}
			if (this->groupBox2->isChecked()) {
				settings->setValue("hotkey2Shortcut", this->keySequenceEdit2->keySequence());
				settings->setValue("hotkey2Action", this->buttonGroup2->checkedId());
				if (this->buttonGroup2->checkedId() != 0) {
					settings->setValue("hotkey2Folder", this->folderLineEdit2->text());
				}
			}
		}
		this->accept();
	}

	void HotkeyDialog::reactToCancelButtonClick() {
		this->keySequenceEdit1->setKeySequence(this->keySequence1OldValue);
		this->keySequenceEdit2->setKeySequence(this->keySequence2OldValue);
		this->buttonGroup1->button(this->action1OldValue)->setChecked(true);
		this->buttonGroup2->button(this->action2OldValue)->setChecked(true);
		this->folderLineEdit1->setText(this->folder1OldValue);
		this->folderLineEdit2->setText(this->folder2OldValue);
		this->groupBox1->setChecked(this->enableHotkey1OldValue);
		this->groupBox2->setChecked(this->enableHotkey2OldValue);
		this->globalGroupBox->setChecked(this->enableHotkeysOldValue);
		this->reactToCheckboxChange();
	}

	void HotkeyDialog::reactToCheckboxChange() {
		if (this->buttonGroup1->checkedId() == 0) {
			this->folderLineEdit1->setEnabled(false);
			this->chooseButton1->setEnabled(false);
		} else if (this->groupBox1->isChecked()) {
			this->folderLineEdit1->setEnabled(true);
			this->chooseButton1->setEnabled(true);
		}
		if (this->buttonGroup2->checkedId() == 0) {
			this->folderLineEdit2->setEnabled(false);
			this->chooseButton2->setEnabled(false);
		} else if (this->groupBox2->isChecked()) {
			this->folderLineEdit2->setEnabled(true);
			this->chooseButton2->setEnabled(true);
		}
	}

	void HotkeyDialog::verifyChanges() {
		this->okButton->setEnabled(true);
		if (this->globalGroupBox->isChecked()) {
			if (this->groupBox1->isChecked() && this->groupBox2->isChecked() && this->keySequenceEdit1->keySequence() == this->keySequenceEdit2->keySequence()) {
				this->okButton->setEnabled(false);
			}
			if (this->groupBox1->isChecked() && this->buttonGroup1->checkedId() != 0 && !QDir(this->folderLineEdit1->text()).exists()) {
				this->okButton->setEnabled(false);
			}
			if (this->groupBox2->isChecked() && this->buttonGroup2->checkedId() != 0 && !QDir(this->folderLineEdit2->text()).exists()) {
				this->okButton->setEnabled(false);
			}
		}
	}

	void HotkeyDialog::selectFolder() {
		QString startDirectory = (QObject::sender() == this->chooseButton1) ? this->folderLineEdit1->text() : this->folderLineEdit2->text();
		QString path = QFileDialog::getExistingDirectory(this, tr("Choose Folder"), startDirectory);
		if (!path.isEmpty()) {
			(QObject::sender() == this->chooseButton1) ? this->folderLineEdit1->setText(path) : this->folderLineEdit2->setText(path);
		}
	}

}