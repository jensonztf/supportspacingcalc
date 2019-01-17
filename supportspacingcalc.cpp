#include "supportspacingcalc.h"
#include "elasticmodulus.h"
#include "steel.h"
#include <qmessagebox.h>
#include <qmath.h>

int waterFlag = 0;	//有无水重
float weight = 0;	//管重
float inistialArray[][2]
{
	{ 0, 0 }
};
steel inistialStell(inistialArray, 0, "inistial");
steel *currentSteel = &inistialStell;
map<string, steel> steel_map;

float caculateWeight(float od, float thickness, int waterflag, float insulationThickness, float insulationVolWeight);
float strengthCheck(float od, float thickness, float weight);
float stiffnessCheck(float od, float thickness, float weight, float em);

steel::steel(float em[][2], int num, string name)
{
	this->ElasticModulus = em;
	this->Name = name;
	this->length = num;
}

steel::~steel()
{

}

supportspacingcalc::supportspacingcalc(QWidget *parent)
: QMainWindow(parent)
{
	//定义钢材
	int num = 0;
	string name = "";
	for (EM_list_iter = EM_list.begin(); EM_list_iter != EM_list.end(); EM_list_iter++)
	{
		num = EM_list_iter->second;
		name = EM_list_iter->first;
		if (name == "A335P92")
		{
			steel A335P92(EM_A335P92, num, name);
			steel_map.insert({ "A335P92", A335P92 });
		}
		else if (name == "12Cr1MoVG")
		{
			steel GB12Cr1MoVG(EM_12Cr1MoVG, num, name);
			steel_map.insert({ "12Cr1MoVG", GB12Cr1MoVG });
		}
		else if (name == "20G")
		{
			steel GB20G(EM_20G, num, name);
			steel_map.insert({ "20G", GB20G });
		}
		else if (name == "Q235")
		{
			steel Q235(EM_Q235, num, name);
			steel_map.insert({ "Q235", Q235 });
		}
		else if (name == "15Ni1MnMoNbCu")
		{
			steel GB15Ni1MnMoNbCu(EM_15Ni1MnMoNbCu, num, name);
			steel_map.insert({ "15Ni1MnMoNbCu", GB15Ni1MnMoNbCu });
		}
		else if (name == "Q345")
		{
			steel Q345(EM_Q345, num, name);
			steel_map.insert({ "Q345", Q345 });
		}
		else if (name == "10")
		{
			steel GB10(EM_10, num, name);
			steel_map.insert({ "10", GB10 });
		}
		else if (name == "15CrMoG")
		{
			steel GB15CrMoG(EM_15CrMoG, num, name);
			steel_map.insert({ "15CrMoG", GB15CrMoG });
		}
	}

	ui.setupUi(this);
	setWindowIcon(QIcon(QStringLiteral("pipe_32x32.ico")));
}

void supportspacingcalc::on_click_btn_calcDis()
{
	QString text = "";
	QString errMsg = "";
	float od = 0;
	float thickness = 0;
	int temperature = 0;
	float ela = 0;
	//取管径、壁厚
	text = supportspacingcalc::ui.lineEdit_od->text();
	if (text.isEmpty())
	{
		errMsg += QString::fromLocal8Bit("请输入管道外径！");
	}
	else
	{
		od = text.toFloat();
	}
	text = supportspacingcalc::ui.lineEdit_thickness->text();
	if (text.isEmpty())
	{
		errMsg += '\n';
		errMsg += QString::fromLocal8Bit("请输入管道壁厚！");
	}
	else
	{
		thickness = text.toFloat();
	}
	//取温度
	text = supportspacingcalc::ui.lineEdit_t->text();
	if (text.isEmpty())
	{
		errMsg += '\n';
		errMsg += QString::fromLocal8Bit("请输入管道温度！");
	}
	else
	{
		temperature = text.toInt();
	}
	if (errMsg.length() > 0)
	{
		QMessageBox::about(NULL, "About", errMsg);
	}
	//取弹性模量,差值法
	float(*em)[2] = currentSteel->ElasticModulus;
	int len = currentSteel->length;
	for (int i = 0; i < len; i++)
	{
		if (temperature == em[i][0])
		{
			ela = em[i][1];
			break;
		}
		else if (temperature < em[i][0])
		{
			if (i == 0)
			{
				ela = (temperature / em[i][0]) * em[i][1];
			}
			else if (i > 0)
			{
				ela = ((temperature - em[i - 1][0]) / (em[i][0] - em[i - 1][0]))
					*(em[i][1] - em[i - 1][1])
					+ em[i - 1][1];
			}
			break;
		}
		else if (i == (len - 1))
		{
			float t = em[i][0];
			QString s = QString::fromLocal8Bit("温度超限！");
			s += "\n";
			s += QString::fromLocal8Bit("最高使用温度为：%1 ℃").arg(t);
			QMessageBox::about(NULL, "About", s);
		}
	}
	float dis = qMin(strengthCheck(od, thickness, weight),
		stiffnessCheck(od, thickness, weight, ela)
		);
	text = QString("%1").arg(ela);
	supportspacingcalc::ui.lineEdit_ela->setText(text);
	text = QString("%1").arg(dis);
	supportspacingcalc::ui.lineEdit_showdis->setText(text);
};

void supportspacingcalc::on_click_btn_calcWeight()
{
	QString text = "";
	QString errMsg = "";
	float od = 0;
	float thickness = 0;
	float insulationThickness = 0;
	float insulationVolWeight = 0;
	text = supportspacingcalc::ui.lineEdit_od->text();
	if (text.isEmpty())
	{
		errMsg += QString::fromLocal8Bit("请输入管道外径！");
	}
	else
	{
		od = text.toFloat();
	}
	text = supportspacingcalc::ui.lineEdit_thickness->text();
	if (text.isEmpty())
	{
		errMsg += '\n';
		errMsg += QString::fromLocal8Bit("请输入管道壁厚！");
	}
	else
	{
		thickness = text.toFloat();
	}
	text = supportspacingcalc::ui.lineEdit_insulation_thickness->text();
	if (text.isEmpty() && supportspacingcalc::ui.lineEdit_insulation_thickness->placeholderText().isEmpty())
	{
		errMsg += '\n';
		errMsg += QString::fromLocal8Bit("请输入管道保温厚！");
	}
	else
	{
		if (text.isEmpty())
		{
			insulationThickness = supportspacingcalc::ui.lineEdit_insulation_thickness->placeholderText().toFloat();
		}
		else
		{
			insulationThickness = text.toFloat();
		}

	}
	text = supportspacingcalc::ui.lineEdit_volumetric_weight->text();
	if (text.isEmpty() && supportspacingcalc::ui.lineEdit_volumetric_weight->placeholderText().isEmpty())
	{
		errMsg += '\n';
		errMsg += QString::fromLocal8Bit("请输入管道保温容重！");
	}
	else
	{
		if (text.isEmpty())
		{
			insulationVolWeight = supportspacingcalc::ui.lineEdit_volumetric_weight->placeholderText().toFloat();
		}
		else
		{
			insulationVolWeight = text.toFloat();
		}

	}
	if (errMsg.length() > 0)
	{
		QMessageBox::about(NULL, "About", errMsg);
	}
	weight = caculateWeight(od, thickness, waterFlag, insulationThickness, insulationVolWeight);
	text = QString("%1").arg(weight);
	supportspacingcalc::ui.lineEdit_weight->setText(text);

}

void supportspacingcalc::on_toggle_material_radio()
{
	QString name = supportspacingcalc::ui.buttonGroup->checkedButton()->text();
	if (name.contains("P92", Qt::CaseSensitive))
	{
		currentSteel = &steel_map.at("A335P92");
	}
	else if (name.contains("12Cr1MoVG", Qt::CaseSensitive))
	{
		currentSteel = &steel_map.at("12Cr1MoVG");
	}
	else if (name.contains("20G", Qt::CaseSensitive))
	{
		currentSteel = &steel_map.at("20G");
	}
	else if (name.contains("Q235", Qt::CaseSensitive))
	{
		currentSteel = &steel_map.at("Q235");
	}
	else if (name.contains("15Ni1MnMoNbCu", Qt::CaseSensitive))
	{
		currentSteel = &steel_map.at("15Ni1MnMoNbCu");
	}
	else if (name.contains("Q345", Qt::CaseSensitive))
	{
		currentSteel = &steel_map.at("Q345");
	}
	else if (name.contains("10", Qt::CaseSensitive))
	{
		currentSteel = &steel_map.at("10");
	}
	else if (name.contains("15CrMoG", Qt::CaseSensitive))
	{
		currentSteel = &steel_map.at("15CrMoG");
	}
}

void supportspacingcalc::on_toggle_water_radio()
{
	QString text = supportspacingcalc::ui.buttonGroup_2->checkedButton()->text();
	if (text == QString::fromLocal8Bit("有水"))
	{
		waterFlag = 1;
	}
	else if (text == QString::fromLocal8Bit("无水"))
	{
		waterFlag = 0;
	}
	else
	{
		waterFlag = -1;
	}
}

void supportspacingcalc::on_list_material_clicked()
{
	QString text = supportspacingcalc::ui.listWidget_material->currentItem()->text();
	float(*em)[2] = EM_A335P92;
	if (text.contains("P92", Qt::CaseSensitive))
	{
		em = EM_A335P92;
	}
	else if (text.contains("12Cr1MoVG", Qt::CaseSensitive))
	{
		em = EM_12Cr1MoVG;
	}
	else if (text.contains("20G", Qt::CaseSensitive))
	{
		em = EM_20G;
	}
	else if (text.contains("Q235", Qt::CaseSensitive))
	{
		em = EM_Q235;
	}
	else if (text.contains("15Ni1MnMoNbCu", Qt::CaseSensitive))
	{
		em = EM_15Ni1MnMoNbCu;
	}
	else if (text.contains("Q345", Qt::CaseSensitive))
	{
		em = EM_Q345;
	}
	else if (text.contains("10", Qt::CaseSensitive))
	{
		em = EM_10;
	}
	else if (text.contains("15Ni1MnMoNbCu", Qt::CaseSensitive))
	{
		em = EM_15CrMoG;
	}
	int len = EM_list.at(text.toStdString());
	QString text1 = QString("Tempr\t| ElasticModulus");
	text1 += '\n';
	text1 += QString("%1\t| KN/mm^2").arg(QString::fromLocal8Bit("℃"));
	text1 += '\n';
	text1 += QString("-----------\t| -------------");
	text1 += '\n';
	float temp = 0;
	float ela = 0;
	for (int i = 0; i < len; i++)
	{
		temp = em[i][0];
		ela = em[i][1];
		text1 += QString("%1\t| %2").arg(temp).arg(ela);
		text1 += '\n';
	}
	ui.textEdit_em->setText(text1);
}

float caculateWeight(float od, float thickness, int waterflag, float insulationThickness, float insulationVolWeight)
{
	//管重 kg/m
	float weight = 0;
	weight = 7.85 * M_PI * (qPow((od / 2), 2) - qPow((od - 2 * thickness) / 2, 2)) / 1000;		//每米钢管重
	weight += waterflag * M_PI * qPow((od - 2 * thickness), 2) / 4000;		//每米水重
	weight += M_PI * (qPow((od / 2 + insulationThickness), 2) - qPow(od / 2, 2)) * insulationVolWeight / 1000000;	//每米保温重
	return weight;
}

float strengthCheck(float od, float thickness, float weight)
{
	float length = 0;
	//q为管道每米自重 KN/m
	float q = weight * 9.81 / 1000;
	//W为管子截面抗弯矩,cm3
	float W = M_PI * (qPow(od, 4) - qPow((od - 2 * thickness), 4))
		/ (32 * 1000 * od);
	length = 0.3577*qPow((W / q), 0.5);
	return length;
}

float stiffnessCheck(float od, float thickness, float weight, float em)
{
	float length = 0;
	//q为管道每米自重 KN/m
	float q = weight * 9.81 / 1000;
	float alpha = (od - 2 * thickness) / od;
	// I为截面惯性矩,cm4
	float I = M_PI * qPow(od, 4)*(1 - qPow(alpha, 4)) / 640000;
	length = 0.2094 * qPow((em * I / q), 0.25);
	return length;
}