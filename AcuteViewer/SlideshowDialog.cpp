#include "SlideshowDialog.h"

namespace sv {

	SlideshowDialog::SlideshowDialog(std::shared_ptr<QSettings> settings, QWidget* parent)
		: settings(settings), 
		QDialog(parent) {
		setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

		setWindowTitle(tr("Start Slideshow"));

		timeSpinBox = new QDoubleSpinBox(this);
		timeSpinBox->setMinimum(0);
		timeSpinBox->setMaximum(1814400000);
		timeSpinBox->setDecimals(3);
		timeSpinBox->setValue(settings->value("slideDelay", 3).toDouble());
		timeSpinBox->setSuffix("s");

		loopCheckbox = new QCheckBox(tr("&Loop"), this);
		loopCheckbox->setChecked(settings->value("slideshowLoop", false).toBool());

		formLayout = new QFormLayout();
		formLayout->addRow(tr("&Delay:"), timeSpinBox);
		formLayout->addRow("", loopCheckbox);

		okButton = new QPushButton(tr("&Ok"), this);
		okButton->setDefault(true);
		QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(reactToOkButtonClick()));

		cancelButton = new QPushButton(tr("&Cancel"), this);
		QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

		buttonLayout = new QHBoxLayout();
		buttonLayout->addStretch(1);
		buttonLayout->addWidget(okButton);
		buttonLayout->addWidget(cancelButton);

		mainLayout = new QVBoxLayout();
		mainLayout->addLayout(formLayout);
		mainLayout->addLayout(buttonLayout);

		setLayout(mainLayout);
		layout()->setSizeConstraint(QLayout::SetFixedSize);
	}

	SlideshowDialog::~SlideshowDialog() {
		delete mainLayout;
		delete formLayout;
		delete buttonLayout;
		delete timeSpinBox;
		delete loopCheckbox;
		delete okButton;
		delete cancelButton;
	}

	//============================================================================== PROTECTED ==============================================================================\\


	//=============================================================================== PRIVATE ===============================================================================\\


	//============================================================================ PRIVATE SLOTS =============================================================================\\

	void SlideshowDialog::reactToOkButtonClick() {
		settings->setValue("slideshowLoop", loopCheckbox->isChecked());
		settings->setValue("slideDelay", timeSpinBox->value());
		accept();
		close();
	}

}