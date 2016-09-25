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
		this->buttonGroup1->addButton(binRadioButton1);
		this->buttonGroup1->addButton(moveRadioButton1);
		this->buttonGroup1->addButton(copyRadioButton1);
		this->folderLineEdit1 = new QLineEdit(this);
		this->folderLineEdit1->setMinimumWidth(250);
		this->chooseButton1 = new QPushButton(tr("Choose"), this);
		this->chooseButton1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

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

		this->globalGroupBoxLayout = new QVBoxLayout;
		this->globalGroupBoxLayout->addWidget(groupBox1);

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

}