#include "threshold.h"
#include "returntype.h"

Threshold::Threshold(QDialog*parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	this->res_ = new ThresholdSets;

	connect(ui.btn_save, &QPushButton::clicked, [this] { this->Save(); });
}

Threshold::~Threshold()
{}

void Threshold::Save()
{
	this->res_->low = ui.ledit_low->text().toInt();
	this->res_->high = ui.ledit_high->text().toInt();
	this->res_->is_inside = ui.radio_inside->isChecked();
	this->hide();
}

