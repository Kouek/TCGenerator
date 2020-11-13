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
		READ_READY,
		WRITE_READY,
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
		if (stat == WRITE_READY)
			stat = READ_READY;
		else if (stat == READ_READY)
			stat = NOT_READY;
	}
	// }
private:
	void scanOriFile();
	void output2Excel();

private:
	Ui::TCGeneratorClass ui;
private:
	Status stat;
	QList<int> sizeList;
	QStringList itemList; // 按顺序读入的测试项名
	QStringList valList; // 按顺序读入的测试值名
	std::vector<int*> TC; // 结果
};
