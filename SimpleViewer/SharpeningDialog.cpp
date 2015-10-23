#include "SharpeningDialog.h"

namespace sv {

	SharpeningDialog::SharpeningDialog(std::shared_ptr<QSettings> settings, QWidget* parent)
		: settings(settings),
		QDialog(parent) {
		this->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
		this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

		this->setWindowTitle(tr("Sharpening Options"));

		this->descriptionLabel = new QLabel(tr("<h3>Post-Resize Sharpening</h3>"
											   "<p>The post-resize sharpening will be applied to the image after it has been downsampled to fit the window."
											   "This can improve the impression of sharpness that might be reduced by the downscaling procedure. This is "
											   "most noticable if the original image resolution was much higher.</p><p>The sharpening will only be applied at zoom "
											   "levels below 100%. If you don't see any changes when changing the parameters your current zoom level might be equal "
											   "or above 100%.</p>"), this);
		this->descriptionLabel->setWordWrap(true);
		this->descriptionLabel->setSizePolicy(QSizePolicy(this->descriptionLabel->sizePolicy().horizontalPolicy(), QSizePolicy::Minimum));
		this->descriptionLabel->setMinimumWidth(400);

		this->strengthSpinBox = new QDoubleSpinBox(this);
		this->strengthSpinBox->setMinimum(0);
		this->strengthSpinBox->setMaximum(1000000);
		this->strengthSpinBox->setDecimals(2);
		this->strengthSpinBox->setSingleStep(0.25);
		this->strengthSpinBox->setMaximumWidth(100);
		this->strengthSpinBox->setMaximumWidth(this->strengthSpinBox->sizeHint().width());
		QObject::connect(this->strengthSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateSharpeningSettings()));

		this->radiusSpinBox = new QDoubleSpinBox(this);
		this->radiusSpinBox->setMinimum(0.1);
		this->radiusSpinBox->setMaximum(1000000);
		this->radiusSpinBox->setDecimals(1);
		this->strengthSpinBox->setSingleStep(1);
		this->radiusSpinBox->setSuffix("px");
		this->radiusSpinBox->setMaximumWidth(this->strengthSpinBox->sizeHint().width());
		QObject::connect(this->radiusSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateSharpeningSettings()));

		this->sharpeningCheckbox = new QCheckBox(tr("&Enable Sharpening"), this);
		this->sharpeningCheckbox->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, this->sharpeningCheckbox->sizePolicy().verticalPolicy()));
		QObject::connect(this->sharpeningCheckbox, SIGNAL(stateChanged(int)), this, SLOT(updateSharpeningSettings()));

		this->formLayout = new QFormLayout();
		this->formLayout->setFormAlignment(Qt::AlignCenter);
		this->formLayout->addRow(tr("&Strength:"), this->strengthSpinBox);
		this->formLayout->addRow(tr("&Radius:"), this->radiusSpinBox);
		this->formLayout->addRow(this->sharpeningCheckbox);
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
		this->mainLayout->addWidget(this->descriptionLabel);
		this->mainLayout->addSpacing(10);
		this->mainLayout->addLayout(this->formLayout);
		this->mainLayout->addSpacing(10);
		this->mainLayout->addLayout(this->buttonLayout);

		this->setLayout(this->mainLayout);
		this->layout()->setSizeConstraint(QLayout::SetFixedSize);
	}

	SharpeningDialog::~SharpeningDialog() {
		delete this->mainLayout;
		delete this->formLayout;
		delete this->buttonLayout;
		delete this->descriptionLabel;
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