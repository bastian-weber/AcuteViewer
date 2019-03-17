#include "HotkeyDialog.h"

namespace sv {

	HotkeyDialog::HotkeyDialog(std::shared_ptr<QSettings> settings, QWidget* parent)
		: settings(settings),
		QDialog(parent) {
		setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

		setMinimumHeight(300);

		setWindowTitle(tr("Hotkey Options"));

		//first hotkey

		keySequenceEdit1 = new QKeySequenceEdit(this);
		QObject::connect(keySequenceEdit1, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(verifyChanges()));
		binRadioButton1 = new QRadioButton(tr("Move to recycle bin"), this);
		moveRadioButton1 = new QRadioButton(tr("Move to folder..."), this);
		copyRadioButton1 = new QRadioButton(tr("Copy to folder..."), this);
		buttonGroup1 = new QButtonGroup(this);
		buttonGroup1->addButton(binRadioButton1, 0);
		buttonGroup1->addButton(moveRadioButton1, 1);
		buttonGroup1->addButton(copyRadioButton1, 2);
		QObject::connect(buttonGroup1, SIGNAL(buttonClicked(int)), this, SLOT(reactToCheckboxChange()));
		QObject::connect(buttonGroup1, SIGNAL(buttonClicked(int)), this, SLOT(verifyChanges()));
		folderLineEdit1 = new QLineEdit(this);
		folderLineEdit1->setMinimumWidth(250);
		QObject::connect(folderLineEdit1, SIGNAL(textChanged(QString)), this, SLOT(verifyChanges()));
		completer1 = new QCompleter(this);
		QDirModel* dirModel1 = new QDirModel(completer1);
		dirModel1->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
		completer1->setModel(dirModel1);
		folderLineEdit1->setCompleter(completer1);
		chooseButton1 = new QPushButton(tr("Choose"), this);
		chooseButton1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
		QObject::connect(chooseButton1, SIGNAL(clicked()), this, SLOT(selectFolder()));

		formLayout1 = new QFormLayout();
		formLayout1->setFormAlignment(Qt::AlignCenter);
		formLayout1->addRow(tr("Key:"), keySequenceEdit1);
		formLayout1->addRow(tr("Action:"), binRadioButton1);
		formLayout1->addRow("", moveRadioButton1);
		formLayout1->addRow("", copyRadioButton1);
		formLayout1->addRow(tr("Folder:"), folderLineEdit1);
		formLayout1->addRow("", chooseButton1);

		groupBox1 = new QGroupBox(tr("Hotkey 1"), this);
		groupBox1->setCheckable(true);
		groupBox1->setLayout(formLayout1);
		QObject::connect(groupBox1, SIGNAL(toggled(bool)), this, SLOT(verifyChanges()));

		//second hotkey

		keySequenceEdit2 = new QKeySequenceEdit(this);
		QObject::connect(keySequenceEdit2, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(verifyChanges()));
		binRadioButton2 = new QRadioButton(tr("Move to recycle bin"), this);
		moveRadioButton2 = new QRadioButton(tr("Move to folder..."), this);
		copyRadioButton2 = new QRadioButton(tr("Copy to folder..."), this);
		buttonGroup2 = new QButtonGroup(this);
		buttonGroup2->addButton(binRadioButton2, 0);
		buttonGroup2->addButton(moveRadioButton2, 1);
		buttonGroup2->addButton(copyRadioButton2, 2);
		QObject::connect(buttonGroup2, SIGNAL(buttonClicked(int)), this, SLOT(reactToCheckboxChange()));
		QObject::connect(buttonGroup2, SIGNAL(buttonClicked(int)), this, SLOT(verifyChanges()));
		folderLineEdit2 = new QLineEdit(this);
		folderLineEdit2->setMinimumWidth(250);
		QObject::connect(folderLineEdit2, SIGNAL(textChanged(QString)), this, SLOT(verifyChanges()));
		completer2 = new QCompleter(this);
		QDirModel* dirModel2 = new QDirModel(completer2);
		dirModel2->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
		completer2->setModel(dirModel2);
		folderLineEdit2->setCompleter(completer2);
		chooseButton2 = new QPushButton(tr("Choose"), this);
		chooseButton2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
		QObject::connect(chooseButton2, SIGNAL(clicked()), this, SLOT(selectFolder()));

		formLayout2 = new QFormLayout();
		formLayout2->setFormAlignment(Qt::AlignCenter);
		formLayout2->addRow(tr("Key:"), keySequenceEdit2);
		formLayout2->addRow(tr("Action:"), binRadioButton2);
		formLayout2->addRow("", moveRadioButton2);
		formLayout2->addRow("", copyRadioButton2);
		formLayout2->addRow(tr("Folder:"), folderLineEdit2);
		formLayout2->addRow("", chooseButton2);

		groupBox2 = new QGroupBox(tr("Hotkey 2"), this);
		groupBox2->setCheckable(true);
		groupBox2->setLayout(formLayout2);
		QObject::connect(groupBox2, SIGNAL(toggled(bool)), this, SLOT(verifyChanges()));

		globalGroupBoxLayout = new QVBoxLayout;
		globalGroupBoxLayout->addWidget(groupBox1);
		globalGroupBoxLayout->addWidget(groupBox2);

		globalGroupBox = new QGroupBox(tr("Enable Hotkeys"), this);
		globalGroupBox->setCheckable(true);
		globalGroupBox->setLayout(globalGroupBoxLayout);
		QObject::connect(globalGroupBox, SIGNAL(toggled(bool)), this, SLOT(verifyChanges()));

		sidecarAllRadioButton = new QRadioButton(tr("All Sidecar Files"), this);
		sidecarAllRadioButton->setToolTip(tr("If this option is selected, only XMP sidecar files will be affected by the action."));
		sidecarXmpRadioButton = new QRadioButton(tr("XMP Sidecar Files Only"), this);
		sidecarAllRadioButton->setToolTip(tr("If this option is selected, all sidecar files regardless of the extension will be affected by the action."));
		sidecarButtonGroup = new QButtonGroup(this);
		sidecarButtonGroup->addButton(sidecarAllRadioButton, 1);
		sidecarButtonGroup->addButton(sidecarXmpRadioButton, 0);

		sidecarLayout = new QVBoxLayout;
		sidecarLayout->addWidget(sidecarXmpRadioButton);
		sidecarLayout->addWidget(sidecarAllRadioButton);
		sidecarFileGroupBox = new QGroupBox(tr("Include Sidecar Files in Action"), this);
		sidecarFileGroupBox->setCheckable(true);
		sidecarFileGroupBox->setToolTip(tr("If this is checked, sidecar files will be included in whatever action is performed. Sidecar files are files with the same name but a different extension, for example XMP files."));
		sidecarFileGroupBox->setLayout(sidecarLayout);
		
		confirmationCheckBox = new QCheckBox(tr("Ask for Confirmation Before Performing an Action"), this);

		optionsLayout = new QVBoxLayout;
		optionsLayout->addWidget(sidecarFileGroupBox);
		optionsLayout->addWidget(confirmationCheckBox);

		optionsGroupBox = new QGroupBox(tr("Options"), this);
		optionsGroupBox->setLayout(optionsLayout);

		okButton = new QPushButton(tr("Ok"), this);
		QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(reactToOkButtonClick()));
		cancelButton = new QPushButton(tr("Cancel"), this);
		QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(resetChanges()));
		QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

		buttonLayout = new QHBoxLayout();
		buttonLayout->addStretch(1);
		buttonLayout->addWidget(okButton);
		buttonLayout->addWidget(cancelButton);

		mainLayout = new QVBoxLayout();
		mainLayout->addWidget(globalGroupBox);
		mainLayout->addWidget(optionsGroupBox);
		mainLayout->addLayout(buttonLayout);

		setLayout(mainLayout);
		layout()->setSizeConstraint(QLayout::SetFixedSize);

		loadSettings();
		saveState();
	}

	//=============================================================================== PRIVATE ===============================================================================\\

	void HotkeyDialog::loadSettings() {
		keySequenceEdit1->setKeySequence(settings->value("hotkey1Shortcut", QKeySequence(Qt::Key_Delete)).value<QKeySequence>());
		buttonGroup1->button(settings->value("hotkey1Action", 0).toInt())->setChecked(true);
		folderLineEdit1->setText(settings->value("hotkey1Folder", QDir::rootPath()).toString());
		keySequenceEdit2->setKeySequence(settings->value("hotkey2Shortcut", QKeySequence(Qt::Key_Enter)).value<QKeySequence>());
		buttonGroup2->button(settings->value("hotkey2Action", 0).toInt())->setChecked(true);
		folderLineEdit2->setText(settings->value("hotkey2Folder", QDir::rootPath()).toString());
		globalGroupBox->setChecked(settings->value("enableHotkeys", true).toBool());
		groupBox1->setChecked(settings->value("enableHotkey1", true).toBool());
		groupBox2->setChecked(settings->value("enableHotkey2", false).toBool());
		sidecarFileGroupBox->setChecked(settings->value("includeSidecarFiles", false).toBool());
		sidecarButtonGroup->button(settings->value("sidecarType", 0).toInt())->setChecked(true);
		confirmationCheckBox->setChecked(settings->value("showActionConfirmation", true).toBool());
		reactToCheckboxChange();
	}

	void HotkeyDialog::saveState() {
		enableHotkeysOldValue = globalGroupBox->isChecked();
		enableHotkey1OldValue = groupBox1->isChecked();
		enableHotkey2OldValue = groupBox2->isChecked();
		keySequence1OldValue = keySequenceEdit1->keySequence();
		keySequence2OldValue = keySequenceEdit2->keySequence();
		action1OldValue = buttonGroup1->checkedId();
		action2OldValue = buttonGroup2->checkedId();
		folder1OldValue = folderLineEdit1->text();
		folder2OldValue = folderLineEdit2->text();
		sidecarFilesOldValue = sidecarFileGroupBox->isChecked();
		sidecarTypeOldValue = sidecarButtonGroup->checkedId();
		confirmationOldValue = confirmationCheckBox->isChecked();
	}

	//============================================================================ PRIVATE SLOTS =============================================================================\\

	void HotkeyDialog::reactToOkButtonClick() {
		if (!verifyChanges()) {
			if (!isVisible()) show();
			return;
		}
		settings->setValue("enableHotkeys", globalGroupBox->isChecked());
		settings->setValue("enableHotkey1", groupBox1->isChecked());
		settings->setValue("enableHotkey2", groupBox2->isChecked());
		settings->setValue("hotkey1Shortcut", keySequenceEdit1->keySequence());
		settings->setValue("hotkey1Action", buttonGroup1->checkedId());
		settings->setValue("hotkey1Folder", folderLineEdit1->text());
		settings->setValue("hotkey2Shortcut", keySequenceEdit2->keySequence());
		settings->setValue("hotkey2Action", buttonGroup2->checkedId());
		settings->setValue("hotkey2Folder", folderLineEdit2->text());
		settings->setValue("includeSidecarFiles", sidecarFileGroupBox->isChecked());
		settings->setValue("sidecarType", sidecarButtonGroup->checkedId());
		settings->setValue("showActionConfirmation", confirmationCheckBox->isChecked());
		saveState();
		accept();
	}

	void HotkeyDialog::resetChanges() {
		keySequenceEdit1->setKeySequence(keySequence1OldValue);
		keySequenceEdit2->setKeySequence(keySequence2OldValue);
		buttonGroup1->button(action1OldValue)->setChecked(true);
		buttonGroup2->button(action2OldValue)->setChecked(true);
		folderLineEdit1->setText(folder1OldValue);
		folderLineEdit2->setText(folder2OldValue);
		groupBox1->setChecked(enableHotkey1OldValue);
		groupBox2->setChecked(enableHotkey2OldValue);
		globalGroupBox->setChecked(enableHotkeysOldValue);
		sidecarFileGroupBox->setChecked(sidecarFilesOldValue);
		sidecarButtonGroup->button(sidecarTypeOldValue)->setChecked(true);
		confirmationCheckBox->setChecked(confirmationOldValue);
		reactToCheckboxChange();
	}

	void HotkeyDialog::reactToCheckboxChange() {
		if (buttonGroup1->checkedId() == 0) {
			folderLineEdit1->setEnabled(false);
			chooseButton1->setEnabled(false);
		} else if (groupBox1->isChecked()) {
			folderLineEdit1->setEnabled(true);
			chooseButton1->setEnabled(true);
		}
		if (buttonGroup2->checkedId() == 0) {
			folderLineEdit2->setEnabled(false);
			chooseButton2->setEnabled(false);
		} else if (groupBox2->isChecked()) {
			folderLineEdit2->setEnabled(true);
			chooseButton2->setEnabled(true);
		}
	}

	bool HotkeyDialog::verifyChanges() {
		QPalette redPalette;
		redPalette.setColor(QPalette::Text, Qt::red);
		bool result = true;
		okButton->setEnabled(true);
		keySequenceEdit1->setPalette(QPalette());
		keySequenceEdit2->setPalette(QPalette());
		folderLineEdit1->setPalette(QPalette());
		folderLineEdit2->setPalette(QPalette());
		if (globalGroupBox->isChecked()) {
			if (groupBox1->isChecked() && groupBox2->isChecked() && keySequenceEdit1->keySequence() == keySequenceEdit2->keySequence()) {
				keySequenceEdit1->setPalette(redPalette);
				keySequenceEdit2->setPalette(redPalette);
				okButton->setEnabled(false);
				result = false;
			}
			if (groupBox1->isChecked() && buttonGroup1->checkedId() != 0 && !QDir(folderLineEdit1->text()).exists()) {
				folderLineEdit1->setPalette(redPalette);
				okButton->setEnabled(false);
				result = false;
			}
			if (groupBox2->isChecked() && buttonGroup2->checkedId() != 0 && !QDir(folderLineEdit2->text()).exists()) {
				folderLineEdit2->setPalette(redPalette);
				okButton->setEnabled(false);
				result = false;
			}
		}
		return result;
	}

	void HotkeyDialog::selectFolder() {
		QString startDirectory = (QObject::sender() == chooseButton1) ? folderLineEdit1->text() : folderLineEdit2->text();
		QString path = QFileDialog::getExistingDirectory(this, tr("Choose Folder"), startDirectory);
		if (!path.isEmpty()) {
			(QObject::sender() == chooseButton1) ? folderLineEdit1->setText(path) : folderLineEdit2->setText(path);
		}
	}

	bool HotkeyDialog::getHotkeysEnabled() {
		return globalGroupBox->isChecked();
	}

	bool HotkeyDialog::getHotkey1Enabled() {
		return groupBox1->isChecked();
	}

	bool HotkeyDialog::getHotkey2Enabled() {
		return groupBox2->isChecked();
	}

	QKeySequence HotkeyDialog::getKeySequence1() {
		return keySequenceEdit1->keySequence();
	}

	QKeySequence HotkeyDialog::getKeySequence2() {
		return keySequenceEdit2->keySequence();
	}

	int HotkeyDialog::getAction1() {
		return buttonGroup1->checkedId();
	}

	int HotkeyDialog::getAction2() {
		return buttonGroup2->checkedId();
	}

	QString HotkeyDialog::getFolder1() {
		return folderLineEdit1->text();
	}

	QString HotkeyDialog::getFolder2() {
		return folderLineEdit2->text();
	}

	bool HotkeyDialog::getIncludeSidecarFiles() {
		return sidecarFileGroupBox->isChecked();
	}

	int HotkeyDialog::getSidecarType() {
		return sidecarButtonGroup->checkedId();
	}

	bool HotkeyDialog::getShowConfirmation() {
		return confirmationCheckBox->isChecked();
	}

	void HotkeyDialog::setHotkeysEnabled(bool value) {
		globalGroupBox->setChecked(value);
		reactToOkButtonClick();
	}

}