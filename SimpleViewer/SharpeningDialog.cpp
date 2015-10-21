#include "SharpeningDialog.h"

namespace sv {

	SharpeningDialog::SharpeningDialog(std::shared_ptr<QSettings> settings, QWidget* parent)
		: settings(settings),
		QDialog(parent) {
		this->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
		this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

		this->setWindowTitle(tr("Sharpening Options"));

		this->strengthSpinBox = new QDoubleSpinBox(this);
		this->strengthSpinBox->setMinimum(0);
		this->strengthSpinBox->setMaximum(1000000);
		this->strengthSpinBox->setDecimals(2);
		this->strengthSpinBox->setSingleStep(0.25);
		QObject::connect(this->strengthSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateSharpeningSettings()));

		this->radiusSpinBox = new QDoubleSpinBox(this);
		this->radiusSpinBox->setMinimum(0.1);
		this->radiusSpinBox->setMaximum(1000000);
		this->radiusSpinBox->setDecimals(1);
		this->strengthSpinBox->setSingleStep(1);
		this->radiusSpinBox->setSuffix("px");
		QObject::connect(this->radiusSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateSharpeningSettings()));

		this->sharpeningCheckbox = new QCheckBox(tr("&Enable Sharpening"), this);
		QObject::connect(this->sharpeningCheckbox, SIGNAL(stateChanged(int)), this, SLOT(updateSharpeningSettings()));

		this->formLayout = new QFormLayout();
		this->formLayout->addRow(tr("&Strength:"), this->strengthSpinBox);
		this->formLayout->addRow(tr("&Radius:"), this->radiusSpinBox);
		this->formLayout->addRow("", this->sharpeningCheckbox);
		this->okButton = new QPushButton(tr("&Ok"), this);
		QObject::connect(this->okButton, SIGNAL(clicked()), this, SLOT(reactToOkButtonClick()));
		QObject::connect(this->okButton, SIGNAL(clicked()), this, SIGNAL(dialogClosed()));

		this->cancelButton = new QPushButton(tr("&Cancel"), this);
		QObject::connect(this->cancelButton, SIGNAL(clicked()), this, SLOT(reactToCancelButtonClick()));
		QObject::connect(this->cancelButton, SIGNAL(clicked()), this, SIGNAL(dialogClosed()));
		QObject::connect(this->cancelButton, SIGNAL(clicked()), this, SLOT(close()));

		this->buttonLayout = new QHBoxLayout();
		this->buttonLayout->addStretch(1);
		this->buttonLayout->addWidget(okButton);
		this->buttonLayout->addWidget(cancelButton);

		this->mainLayout = new QVBoxLayout();
		this->mainLayout->addLayout(this->formLayout);
		this->mainLayout->addLayout(this->buttonLayout);

		this->setLayout(this->mainLayout);
	}

	SharpeningDialog::~SharpeningDialog() {
		delete this->mainLayout;
		delete this->formLayout;
		delete this->buttonLayout;
		delete this->strengthSpinBox;
		delete this->radiusSpinBox;
		delete this->sharpeningCheckbox;
		delete this->okButton;
		delete this->cancelButton;
	}

	//============================================================================== PROTECTED ==============================================================================\\

	void SharpeningDialog::showEvent(QShowEvent* event) {
		this->strengthSpinBox->blockSignals(true);
		this->strengthSpinBox->setValue(settings->value("sharpeningStrength", 0.5).toDouble());
		this->strengthSpinBox->blockSignals(false);
		sharpeningStrengthOldValue = this->strengthSpinBox->value();
		this->radiusSpinBox->blockSignals(true);
		this->radiusSpinBox->setValue(settings->value("sharpeningRadius", 1).toDouble());
		this->radiusSpinBox->blockSignals(false);
		sharpeningRadiusOldValue = this->radiusSpinBox->value();
		this->sharpeningCheckbox->blockSignals(true);
		this->sharpeningCheckbox->setChecked(settings->value("sharpenImagesAfterDownscale", false).toBool());
		this->sharpeningCheckbox->blockSignals(false);
		enableSharpeningOldValue = this->sharpeningCheckbox->isChecked();
	}

	//=============================================================================== PRIVATE ===============================================================================\\


	//============================================================================ PRIVATE SLOTS =============================================================================\\

	void SharpeningDialog::reactToOkButtonClick() {
		settings->setValue("sharpeningStrength", this->strengthSpinBox->value());
		settings->setValue("sharpeningRadius", this->radiusSpinBox->value());
		this->close();
	}

	void SharpeningDialog::updateSharpeningSettings() {
		this->settings->setValue("sharpenImagesAfterDownscale", this->sharpeningCheckbox->isChecked());
		this->settings->setValue("sharpeningStrength", this->strengthSpinBox->value());
		this->settings->setValue("sharpeningRadius", this->radiusSpinBox->value());
		emit(sharpeningParametersChanged());
	}

	void SharpeningDialog::reactToCancelButtonClick() {
		this->settings->setValue("sharpenImagesAfterDownscale", this->enableSharpeningOldValue);
		this->settings->setValue("sharpeningStrength", this->sharpeningStrengthOldValue);
		this->settings->setValue("sharpeningRadius", this->sharpeningRadiusOldValue);
		emit(sharpeningParametersChanged());
	}

}