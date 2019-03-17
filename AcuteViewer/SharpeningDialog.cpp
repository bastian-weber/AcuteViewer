#include "SharpeningDialog.h"

namespace sv {

	SharpeningDialog::SharpeningDialog(std::shared_ptr<QSettings> settings, QWidget* parent)
		: settings(settings),
		QDialog(parent) {
		setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

		setWindowTitle(tr("Sharpening Options"));

		descriptionLabel = new QLabel(tr("<h3>Post-Resize Sharpening</h3>"
											   "<p>The post-resize sharpening will be applied to the image after it has been downsampled to fit the window."
											   "This can improve the impression of sharpness that might be reduced by the downscaling procedure. This is "
											   "most noticable if the original image resolution was much higher.</p><p>The sharpening will only be applied at zoom "
											   "levels below 100%. If you don't see any changes when changing the parameters your current zoom level might be equal "
											   "or above 100%.</p>"), this);
		descriptionLabel->setWordWrap(true);
		descriptionLabel->setSizePolicy(QSizePolicy(descriptionLabel->sizePolicy().horizontalPolicy(), QSizePolicy::Minimum));
		descriptionLabel->setMinimumWidth(400);

		strengthSpinBox = new QDoubleSpinBox(this);
		strengthSpinBox->setMinimum(0);
		strengthSpinBox->setMaximum(1000000);
		strengthSpinBox->setDecimals(2);
		strengthSpinBox->setSingleStep(0.25);
		strengthSpinBox->setMaximumWidth(100);
		strengthSpinBox->setMaximumWidth(strengthSpinBox->sizeHint().width());
		QObject::connect(strengthSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateSharpeningSettings()));

		radiusSpinBox = new QDoubleSpinBox(this);
		radiusSpinBox->setMinimum(0.1);
		radiusSpinBox->setMaximum(1000000);
		radiusSpinBox->setDecimals(1);
		radiusSpinBox->setSingleStep(1);
		radiusSpinBox->setSuffix("px");
		radiusSpinBox->setMaximumWidth(strengthSpinBox->sizeHint().width());
		QObject::connect(radiusSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateSharpeningSettings()));

		sharpeningCheckbox = new QCheckBox(tr("&Enable Sharpening"), this);
		sharpeningCheckbox->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, sharpeningCheckbox->sizePolicy().verticalPolicy()));
		QObject::connect(sharpeningCheckbox, SIGNAL(stateChanged(int)), this, SLOT(updateSharpeningSettings()));

		formLayout = new QFormLayout();
		formLayout->setFormAlignment(Qt::AlignCenter);
		formLayout->addRow(tr("&Strength:"), strengthSpinBox);
		formLayout->addRow(tr("&Radius:"), radiusSpinBox);
		formLayout->addRow(sharpeningCheckbox);
		okButton = new QPushButton(tr("&Ok"), this);
		QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(reactToOkButtonClick()));

		cancelButton = new QPushButton(tr("&Cancel"), this);
		QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(reactToCancelButtonClick()));
		QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

		buttonLayout = new QHBoxLayout();
		buttonLayout->addStretch(1);
		buttonLayout->addWidget(okButton);
		buttonLayout->addWidget(cancelButton);

		mainLayout = new QVBoxLayout();
		mainLayout->addWidget(descriptionLabel);
		mainLayout->addSpacing(10);
		mainLayout->addLayout(formLayout);
		mainLayout->addSpacing(10);
		mainLayout->addLayout(buttonLayout);

		setLayout(mainLayout);
		layout()->setSizeConstraint(QLayout::SetFixedSize);
	}

	SharpeningDialog::~SharpeningDialog() {
		delete mainLayout;
		delete formLayout;
		delete buttonLayout;
		delete descriptionLabel;
		delete strengthSpinBox;
		delete radiusSpinBox;
		delete sharpeningCheckbox;
		delete okButton;
		delete cancelButton;
	}

	//============================================================================== PROTECTED ==============================================================================\\

	void SharpeningDialog::showEvent(QShowEvent* event) {
		strengthSpinBox->blockSignals(true);
		strengthSpinBox->setValue(settings->value("sharpeningStrength", 0.5).toDouble());
		strengthSpinBox->blockSignals(false);
		sharpeningStrengthOldValue = strengthSpinBox->value();
		radiusSpinBox->blockSignals(true);
		radiusSpinBox->setValue(settings->value("sharpeningRadius", 1).toDouble());
		radiusSpinBox->blockSignals(false);
		sharpeningRadiusOldValue = radiusSpinBox->value();
		sharpeningCheckbox->blockSignals(true);
		sharpeningCheckbox->setChecked(settings->value("sharpenImagesAfterDownscale", false).toBool());
		sharpeningCheckbox->blockSignals(false);
		enableSharpeningOldValue = sharpeningCheckbox->isChecked();
	}

	//=============================================================================== PRIVATE ===============================================================================\\


	//============================================================================ PRIVATE SLOTS =============================================================================\\

	void SharpeningDialog::reactToOkButtonClick() {
		settings->setValue("sharpeningStrength", strengthSpinBox->value());
		settings->setValue("sharpeningRadius", radiusSpinBox->value());
		accept();
	}

	void SharpeningDialog::updateSharpeningSettings() {
		settings->setValue("sharpenImagesAfterDownscale", sharpeningCheckbox->isChecked());
		settings->setValue("sharpeningStrength", strengthSpinBox->value());
		settings->setValue("sharpeningRadius", radiusSpinBox->value());
		emit(sharpeningParametersChanged());
	}

	void SharpeningDialog::reactToCancelButtonClick() {
		settings->setValue("sharpenImagesAfterDownscale", enableSharpeningOldValue);
		settings->setValue("sharpeningStrength", sharpeningStrengthOldValue);
		settings->setValue("sharpeningRadius", sharpeningRadiusOldValue);
		emit(sharpeningParametersChanged());
	}

}