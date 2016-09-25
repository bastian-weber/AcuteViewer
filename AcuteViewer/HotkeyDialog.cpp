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
		this->binRadioButton1 = new QRadioButton(tr("Move to recycle bin"), this);
		this->moveRadioButton1 = new QRadioButton(tr("Move to folder..."), this);
		this->copyRadioButton1 = new QRadioButton(tr("Copy to folder..."), this);
		this->buttonGroup1 = new QButtonGroup(this);
		this->buttonGroup1->addButton(binRadioButton1, 0);
		this->buttonGroup1->addButton(moveRadioButton1, 1);
		this->buttonGroup1->addButton(copyRadioButton1, 2);
		QObject::connect(this->buttonGroup1, SIGNAL(buttonClicked(int)), this, SLOT(reactToCheckboxChange()));
		this->folderLineEdit1 = new QLineEdit(this);
		this->folderLineEdit1->setMinimumWidth(250);
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

		//second hotkey

		this->keySequenceEdit2 = new QKeySequenceEdit(this);
		this->binRadioButton2 = new QRadioButton(tr("Move to recycle bin"), this);
		this->moveRadioButton2 = new QRadioButton(tr("Move to folder..."), this);
		this->copyRadioButton2 = new QRadioButton(tr("Copy to folder..."), this);
		this->buttonGroup2 = new QButtonGroup(this);
		this->buttonGroup2->addButton(binRadioButton2, 0);
		this->buttonGroup2->addButton(moveRadioButton2, 1);
		this->buttonGroup2->addButton(copyRadioButton2, 2);
		QObject::connect(this->buttonGroup2, SIGNAL(buttonClicked(int)), this, SLOT(reactToCheckboxChange()));
		this->folderLineEdit2 = new QLineEdit(this);
		this->folderLineEdit2->setMinimumWidth(250);
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

		this->globalGroupBoxLayout = new QVBoxLayout;
		this->globalGroupBoxLayout->addWidget(groupBox1);
		this->globalGroupBoxLayout->addWidget(groupBox2);

		this->globalGroupBox = new QGroupBox(tr("Enable Hotkeys"), this);
		this->globalGroupBox->setCheckable(true);
		this->globalGroupBox->setLayout(this->globalGroupBoxLayout);

		this->okButton = new QPushButton(tr("Ok"), this);
		QObject::connect(this->okButton, SIGNAL(clicked()), this, SLOT(reactToOkButtonClick()));
		QObject::connect(this->okButton, SIGNAL(clicked()), this, SLOT(accept()));
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
	}

	//============================================================================== PROTECTED ==============================================================================\\

	void HotkeyDialog::showEvent(QShowEvent* event) {
		//this->strengthSpinBox->blockSignals(true);
		//this->strengthSpinBox->setValue(settings->value("sharpeningStrength", 0.5).toDouble());
		//this->strengthSpinBox->blockSignals(false);
		//sharpeningStrengthOldValue = this->strengthSpinBox->value();
		//this->radiusSpinBox->blockSignals(true);
		//this->radiusSpinBox->setValue(settings->value("sharpeningRadius", 1).toDouble());
		//this->radiusSpinBox->blockSignals(false);
		//sharpeningRadiusOldValue = this->radiusSpinBox->value();
		//this->sharpeningCheckbox->blockSignals(true);
		//this->sharpeningCheckbox->setChecked(settings->value("sharpenImagesAfterDownscale", false).toBool());
		//this->sharpeningCheckbox->blockSignals(false);
		//enableSharpeningOldValue = this->sharpeningCheckbox->isChecked();
	}

	//=============================================================================== PRIVATE ===============================================================================\\


	//============================================================================ PRIVATE SLOTS =============================================================================\\

	void HotkeyDialog::reactToOkButtonClick() {
		//settings->setValue("sharpeningStrength", this->strengthSpinBox->value());
		//settings->setValue("sharpeningRadius", this->radiusSpinBox->value());
	}

	void HotkeyDialog::updateHotkeySettings() {
		//this->settings->setValue("sharpenImagesAfterDownscale", this->sharpeningCheckbox->isChecked());
		//this->settings->setValue("sharpeningStrength", this->strengthSpinBox->value());
		//this->settings->setValue("sharpeningRadius", this->radiusSpinBox->value());
	}

	void HotkeyDialog::reactToCancelButtonClick() {
		//this->settings->setValue("sharpenImagesAfterDownscale", this->enableSharpeningOldValue);
		//this->settings->setValue("sharpeningStrength", this->sharpeningStrengthOldValue);
		//this->settings->setValue("sharpeningRadius", this->sharpeningRadiusOldValue);
	}

	void HotkeyDialog::reactToCheckboxChange() {
		if (this->buttonGroup1->checkedId() == 0) {
			this->folderLineEdit1->setEnabled(false);
			this->chooseButton1->setEnabled(false);
		} else {
			this->folderLineEdit1->setEnabled(true);
			this->chooseButton1->setEnabled(true);
		}
		if (this->buttonGroup2->checkedId() == 0) {
			this->folderLineEdit2->setEnabled(false);
			this->chooseButton2->setEnabled(false);
		} else {
			this->folderLineEdit2->setEnabled(true);
			this->chooseButton2->setEnabled(true);
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