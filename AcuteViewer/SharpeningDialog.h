#pragma once

#include <iostream>
#include <memory>

//Qt
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>

namespace sv {

	class SharpeningDialog : public QDialog {
		Q_OBJECT
	public:
		SharpeningDialog(std::shared_ptr<QSettings> settings, QWidget* parent = 0);
		~SharpeningDialog();
	protected:
		void showEvent(QShowEvent* event);
	private:
		//functions

		//variables
		std::shared_ptr<QSettings> settings;
		bool enableSharpeningOldValue;
		double sharpeningStrengthOldValue;
		double sharpeningRadiusOldValue;
		//widgets
		QVBoxLayout* mainLayout;
		QFormLayout* formLayout;
		QHBoxLayout* buttonLayout;
		QLabel* descriptionLabel;
		QDoubleSpinBox* strengthSpinBox;
		QDoubleSpinBox* radiusSpinBox;
		QCheckBox* sharpeningCheckbox;
		QPushButton* okButton;
		QPushButton* cancelButton;
		private slots:
		void reactToOkButtonClick();
	private slots:
		void updateSharpeningSettings();
		void reactToCancelButtonClick();
	signals:
		void sharpeningParametersChanged();
	};
}
