#pragma once

#include <QDialog>
#include "ui_threshold.h"

struct ThresholdSets;
class Threshold : public QDialog
{
	Q_OBJECT

public:
	Threshold(QDialog*parent = nullptr);
	~Threshold();

	ThresholdSets* Result() { return this->res_; }

private slots:
	void Save();

private:
	Ui::ThresholdClass ui;
	ThresholdSets* res_;
};
