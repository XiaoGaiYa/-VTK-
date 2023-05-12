#pragma once
#include <qwidget.h>


namespace Ui {
class FilterSetting;
}

struct FiltersSets;
class FilterSetting : public QWidget
{
	Q_OBJECT
	friend class VisualDicom;
public:
	FilterSetting(QWidget* parent = nullptr);
	~FilterSetting();

	FiltersSets* Result() const { return res_; }

private slots:
	void Save();

private:
	Ui::FilterSetting* ui;
	FiltersSets* res_;
};

