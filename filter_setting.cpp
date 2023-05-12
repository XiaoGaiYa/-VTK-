#include "filter_setting.h"
#include "ui_filtersetting.h"
#include "returntype.h"

FilterSetting::FilterSetting(QWidget* parent)
	:QWidget(parent)
	, ui(new Ui::FilterSetting)
{
	this->ui->setupUi(this);
	this->res_ = new FiltersSets;
	connect(ui->btn_gaussion, &QPushButton::clicked, [this] { this->Save(); });
	connect(ui->btn_midvalue, &QPushButton::clicked, [this] { this->Save(); });
	connect(ui->btn_bilateral, &QPushButton::clicked, [this] { this->Save(); });
}

FilterSetting::~FilterSetting()
{
	delete ui;
}

void FilterSetting::Save()
{
	this->res_->sigma_gaussion = ui->ledit_sigma->text().toFloat();
	this->res_->kernelsize_gaussion = ui->cbox_kernelsize->currentIndex() * 2 + 1;
	this->res_->kernelsize_mid = ui->cbox_kernelsize_mid->currentIndex() * 2 + 1;
	this->res_->sigma_bilateral = ui->ledit_sigma_bilateral->text().toFloat();
	this->res_->iterations = ui->ledit_iterations->text().toInt();
	this->res_->threshold = ui->ledit_diffusion_threshold->text().toFloat();
	this->hide();
}
