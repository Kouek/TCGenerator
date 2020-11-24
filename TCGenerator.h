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
	void scanOriFile(); // ɨ�貢���Դtxt�ļ��﷨��ʽ����ȡ�������ȡֵ
	void output2Excel(); // �����Excel�ļ�

private:
	Ui::TCGeneratorClass ui;
private:
	Status stat;
	QStringList itemList; // ��˳�����Ĳ�������
	QStringList valList; // ��˳�����Ĳ���ֵ��
	QList<int> sizeList; // ������������ȡֵ��
	std::vector<std::vector<int>> TC; // ���
};
