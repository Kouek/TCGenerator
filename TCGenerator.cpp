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
	static QString excelErrMsg = QString("Excel读写错误");

	QString dstPath = ui.label_dst_path->text();
	dstPath = dstPath.append("/%1.xlsx").arg(ui.lineEdit_TC_name->text());
	dstPath = QDir::toNativeSeparators(dstPath);

	// Excel写入
	try
	{
		// 获取Excel应用
		QAxObject* excel = new QAxObject("Excel.Application");
		excel->dynamicCall("SetVisible (bool Visible)", "false");
		excel->setProperty("DisplayAlerts", false);

		// 获取工作簿
		QAxObject* workbooks = excel->querySubObject("Workbooks");
		workbooks->dynamicCall("Add");
		QAxObject* workbook = excel->querySubObject("ActiveWorkBook");

		// 获取工作表
		QAxObject* worksheets = workbook->querySubObject("Sheets");
		QAxObject* worksheet = worksheets->querySubObject("Item(int)", 1);

		// 进入单元格操作
		QAxObject* cell = Q_NULLPTR;
		QStringList::iterator itr;
		int row, col;

		// 表头
		for (itr = itemList.begin(), col = 1; itr != itemList.end(); itr++, col++)
		{
			cell = worksheet->querySubObject("Cells(int, int)", 1, col);
			cell->setProperty("Value", *itr);
		}
		cell = worksheet->querySubObject("Cells(int, int)", 1, col++);
		cell->setProperty("Value", QString("cover_num")); // 表示覆盖数
		cell = worksheet->querySubObject("Cells(int, int)", 1, col);
		cell->setProperty("Value", QString("%1_way_cover").arg(ui.spinBox_T_way->value())); // 表示覆盖数

		// 表体
		std::vector<std::vector<int>>::iterator itr2;
		for (itr2 = TC.begin(), row = 2; itr2 != TC.end(); itr2++, row++)
		{
			for (col = 1; col <= itemList.size(); col++)
			{
				cell = worksheet->querySubObject("Cells(int, int)", row, col);
				cell->setProperty("Value", QString(valList[TC[row - 2][col - 1]]));
			}
			cell = worksheet->querySubObject("Cells(int, int)", row, col);
			cell->setProperty("Value", QString("%1").arg(TC[row - 2][col - 1]));
		}

		// 保存，释放所有资源
		workbook->dynamicCall("SaveAs(const QString&)", dstPath);
		workbook->dynamicCall("Close(Boolean)", false);
		excel->dynamicCall("Quit(void)");
	}
	catch (...)
	{
		QMessageBox msg;
		msg.resize(this->width() * 3 / 4, this->height() / 4);
		msg.setWindowTitle("Message");
		msg.setText(excelErrMsg);
		msg.exec();
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
	else if (stat == READY_FOR_WRITE)
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
		if (!TC.empty())
		{
			std::vector<std::vector<int>>::iterator itr2;
			for (itr2 = TC.begin(); itr2 != TC.end(); itr2++)
				(*itr2).clear();
			TC.clear();
		}

		int i;
		QList<int>::iterator itr;
		int* V = new int[itemList.size()];
		for (itr = sizeList.begin(), i = 0; itr != sizeList.end(); itr++, i++)
			V[i] = *itr;

		AETG(TC, ui.spinBox_T_way->value(), itemList.size(), V, ui.spinBox_rand_try->value());

		delete[] V;
	}

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
		stat = READY_FOR_READ;
		ui.label_ori_info->setText(defaultInfo.arg(itemList.size()));
	}

	oriFile.close();
}
