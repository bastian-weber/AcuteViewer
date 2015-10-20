#include "SharpeningDialog.h"

namespace sv {

	SharpeningDialog::SharpeningDialog(std::shared_ptr<QSettings> settings, QWidget *parent)
		: QDialog(parent),
		settings(settings) {
		this->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
		this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

		this->setWindowTitle(tr("Sharpening Options"));

		this->strengthSpinBox = new QDoubleSpinBox(this);
		this->strengthSpinBox->setMinimum(0);
		this->strengthSpinBox->setMaximum(1000000);
		this->strengthSpinBox->setDecimals(2);
		this->strengthSpinBox->setValue(settings->value("sharpeningStrength", 0.5).toDouble());

		this->radiusSpinBox = new QDoubleSpinBox(this);
		this->radiusSpinBox->setMinimum(0);
		this->radiusSpinBox->setMaximum(1000000);
		this->radiusSpinBox->setDecimals(1);
		this->radiusSpinBox->setSuffix("px");
		this->radiusSpinBox->setValue(settings->value("sharpeningRadius", 1).toDouble());

		this->formLayout = new QFormLayout();
		this->formLayout->addRow(tr("&Strength:"), this->strengthSpinBox);
		this->formLayout->addRow(tr("&Radius:"), this->radiusSpinBox);

		this->okButton = new QPushButton(tr("&Ok"), this);
		QObject::connect(this->okButton, SIGNAL(clicked()), this, SLOT(reactToOkButtonClick()));
		QObject::connect(this->okButton, SIGNAL(clicked()), this, SIGNAL(dialogClosed()));

		this->cancelButton = new QPushButton(tr("&Cancel"), this);
		QObject::connect(this->cancelButton, SIGNAL(clicked()), this, SLOT(close()));
		QObject::connect(this->cancelButton, SIGNAL(clicked()), this, SIGNAL(dialogClosed()));

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
		delete this->okButton;
		delete this->cancelButton;
	}

	//============================================================================== PROTECTED ==============================================================================\\


	//=============================================================================== PRIVATE ===============================================================================\\


	//============================================================================ PRIVATE SLOTS =============================================================================\\

	void SharpeningDialog::reactToOkButtonClick() {
		settings->setValue("sharpeningStrength", this->strengthSpinBox->value());
		settings->setValue("sharpeningRadius", this->radiusSpinBox->value());
		this->close();
	}

}