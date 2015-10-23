#include "SlideshowDialog.h"

namespace sv {

	SlideshowDialog::SlideshowDialog(std::shared_ptr<QSettings> settings, QWidget* parent)
		: settings(settings), 
		QDialog(parent) {
		this->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
		this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

		this->setWindowTitle(tr("Start Slideshow"));

		this->timeSpinBox = new QDoubleSpinBox(this);
		this->timeSpinBox->setMinimum(0);
		this->timeSpinBox->setMaximum(1814400000);
		this->timeSpinBox->setDecimals(3);
		this->timeSpinBox->setValue(settings->value("slideDelay", 3).toDouble());
		this->timeSpinBox->setSuffix("s");

		this->loopCheckbox = new QCheckBox(tr("&Loop"), this);
		this->loopCheckbox->setChecked(settings->value("slideshowLoop", false).toBool());

		this->formLayout = new QFormLayout();
		this->formLayout->addRow(tr("&Delay:"), this->timeSpinBox);
		this->formLayout->addRow("", this->loopCheckbox);

		this->okButton = new QPushButton(tr("&Ok"), this);
		this->okButton->setDefault(true);
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
		this->layout()->setSizeConstraint(QLayout::SetFixedSize);
	}

	SlideshowDialog::~SlideshowDialog() {
		delete this->mainLayout;
		delete this->formLayout;
		delete this->buttonLayout;
		delete this->timeSpinBox;
		delete this->loopCheckbox;
		delete this->okButton;
		delete this->cancelButton;
	}

	//============================================================================== PROTECTED ==============================================================================\\


	//=============================================================================== PRIVATE ===============================================================================\\


	//============================================================================ PRIVATE SLOTS =============================================================================\\

	void SlideshowDialog::reactToOkButtonClick() {
		settings->setValue("slideshowLoop", this->loopCheckbox->isChecked());
		settings->setValue("slideDelay", this->timeSpinBox->value());
		emit(dialogConfirmed());
		this->close();
	}

}