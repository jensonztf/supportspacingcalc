#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_supportspacingcalc.h"

class supportspacingcalc : public QMainWindow
{
	Q_OBJECT

public:
	supportspacingcalc(QWidget *parent = Q_NULLPTR);

private slots:
	void on_click_btn_calcDis();
	void on_click_btn_calcWeight();
	void on_toggle_material_radio();
	void on_toggle_water_radio();
	void on_list_material_clicked();

private:
	Ui::supportspacingcalcClass ui;
	
};
