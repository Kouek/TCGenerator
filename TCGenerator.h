#pragma once

#ifdef WIN32  
#pragma execution_character_set("utf-8")
#endif

#include <QtWidgets/QWidget>
#include "ui_TCGenerator.h"

#include <QFile>

#include <vector>

class TCGenerator : public QWidget
{
	Q_OBJECT

public:
	enum Status {
		NOT_READY,
		READY_FOR_READ,
		READY_FOR_WRITE,
		FROZEN
	};
public:
	TCGenerator(QWidget *parent = Q_NULLPTR);
	// slots
	// {
	void getOriFile();
	void getDstFilePath();
	void genTC();
	void lowStat() {
		if (stat == READY_FOR_WRITE)
			stat = READY_FOR_READ;
	}
	// }
private:
	void scanOriFile(); // 扫描并检查源txt文件语法格式，读取测试项及其取值
	void output2Excel(); // 输出到Excel文件

private:
	Ui::TCGeneratorClass ui;
private:
	Status stat;
	QStringList itemList; // 按顺序读入的测试项名
	QStringList valList; // 按顺序读入的测试值名
	QList<int> sizeList; // 保存各测试项的取值数
	std::vector<std::vector<int>> TC; // 结果
};
