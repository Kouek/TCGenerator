#include "TCGenerator.h"

#include <QFileDialog>
#include <QTextStream>
#include <QTextCodec>

#include <QMessageBox>

#include <QDir>
#include <QAxObject>
#include "AETGcore.h"

TCGenerator::TCGenerator(QWidget *parent)
	: QWidget(parent), stat(NOT_READY)
{
	ui.setupUi(this);

	connect(ui.spinBox_T_way, QOverload<int>::of(&QSpinBox::valueChanged), this, &TCGenerator::lowStat);
	connect(ui.spinBox_rand_try, QOverload<int>::of(&QSpinBox::valueChanged), this, &TCGenerator::lowStat);

	connect(ui.pushButton_choose_ori, &QPushButton::clicked, this, &TCGenerator::getOriFile);
	connect(ui.pushButton_choose_dst, &QPushButton::clicked, this, &TCGenerator::getDstFilePath);
	connect(ui.pushButton_gen_TC, &QPushButton::clicked, this, &TCGenerator::genTC);
}

void TCGenerator::getOriFile()
{
	QString oriName = QFileDialog::getOpenFileName();
	if (oriName.isEmpty())
		return;
	lowStat();
	ui.label_ori_path->setText(oriName);
	scanOriFile();
}

void TCGenerator::getDstFilePath()
{
	QString dstPath = QFileDialog::getExistingDirectory();
	if (dstPath.isEmpty())
		return;
	ui.label_dst_path->setText(dstPath);
}

void TCGenerator::output2Excel()
{
	QString dstPath = ui.label_dst_path->text();
	dstPath = dstPath.append("/%1.xlsx").arg(ui.lineEdit_TC_name->text());
	dstPath = QDir::toNativeSeparators(dstPath);

	// Excel写入
	try
	{
		QAxObject* excel = new QAxObject("Excel.Application");
		excel->dynamicCall("SetVisible (bool Visible)", "false");
		excel->setProperty("DisplayAlerts", false);

		QAxObject* workbooks = excel->querySubObject("Workbooks");
		workbooks->dynamicCall("Add");
		QAxObject* workbook = excel->querySubObject("ActiveWorkBook");

		QAxObject* worksheets = workbook->querySubObject("Sheets");
		QAxObject* worksheet = worksheets->querySubObject("Item(int)", 1);

		QAxObject* cell = Q_NULLPTR;
		QStringList::iterator itr;
		int row, col;
		for (itr = itemList.begin(), col = 1; itr != itemList.end(); itr++, col++)
		{
			cell = worksheet->querySubObject("Cells(int, int)", 1, col);
			cell->setProperty("Value", *itr);
		}
		std::vector<int*>::iterator itr2;
		for (itr2 = TC.begin(), row = 2; itr2 != TC.end(); itr2++, row++)
			for (col = 1; col <= itemList.size(); col++)
			{
				cell = worksheet->querySubObject("Cells(int, int)", row, col);
				cell->setProperty("Value", QString(valList[TC[row - 2][col - 1]]));
			}

		workbook->dynamicCall("SaveAs(const QString&)", dstPath);
		workbook->dynamicCall("Close(Boolean)", false);
		excel->dynamicCall("Quit(void)");
		stat = WRITE_READY;
	}
	catch (...)
	{

	}
}

void TCGenerator::genTC()
{
	static QString tTooBigMsg = QString("覆盖深度过大");
	static QString oriNotReadyMsg = QString("源文件解析未完成");

	if (stat == NOT_READY)
	{
		QMessageBox msg;
		msg.resize(this->width() * 3 / 4, this->height() / 4);
		msg.setWindowTitle("Message");
		msg.setText(oriNotReadyMsg);
		msg.exec();
		return;
	}
	else if (stat == WRITE_READY)
	{
		output2Excel();
		return;
	}

	if (itemList.size() < ui.spinBox_T_way->value())
	{
		QMessageBox msg;
		msg.resize(this->width() * 3 / 4, this->height() / 4);
		msg.setWindowTitle("Message");
		msg.setText(tTooBigMsg);
		msg.exec();
		return;
	}

	// AETG
	{
		std::vector<int*>::iterator itr2;
		for (itr2 = TC.begin(); itr2 != TC.end(); itr2++)
			delete[](*itr2);
		TC.clear();

		int i;
		QList<int>::iterator itr;
		int* V = new int[itemList.size()];
		for (itr = sizeList.begin(), i = 0; itr != sizeList.end(); itr++, i++)
			V[i] = *itr;

		AETG(TC, ui.spinBox_T_way->value(), itemList.size(), V, ui.spinBox_rand_try->value());

		delete[] V;
	}

	stat = FROZEN;
	output2Excel();
}

void TCGenerator::scanOriFile()
{
	static QString defaultInfo = QString("当前有待测项%1项，覆盖深度需小于等于待测项数");
	static QString fileWrongFormInfo = QString("源文件格式不正确");
	static QString fileNotOpenInfo = QString("源文件无法打开");

	QFile oriFile(ui.label_ori_path->text());
	if (!oriFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		ui.label_ori_info->setText(fileNotOpenInfo);
		return;
	}

	int cnt, idx;
	QString line;
	QTextStream in(&oriFile);
	in.setCodec("UTF-8");

	sizeList.clear();
	itemList.clear();
	valList.clear();
	while (!in.atEnd())
	{
		line = in.readLine();

		cnt = line.count(':');
		if (cnt != 1) break;
		itemList.append(line.left(idx = line.indexOf(':')));
		
		line = line.mid(idx + 1);
		sizeList.append(line.count(',') + 1);
		valList.append(line.split(','));
	}
	
	if (!in.atEnd())
	{
		stat = NOT_READY;
		ui.label_ori_info->setText(fileWrongFormInfo);
	}
	else
	{
		stat = READ_READY;
		ui.label_ori_info->setText(defaultInfo.arg(itemList.size()));
	}

	oriFile.close();
}
