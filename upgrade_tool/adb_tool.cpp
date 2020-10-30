#pragma execution_character_set("utf-8")
#include "adb_tool.h"
#include "ui_adb_tool.h"
#include "QDir"
#include "QDatetime"
#include"QString"
#include"QStringList"
#include "QFileDialog"
#include "QMessageBox"
#include "QProgressBar"
#include "QSettings"
#include <windows.h>


//QProgressBar *startprogressbar;

static const QString defaultApkPackage = "com.handheld.yhfr";
static const QString defaultApkActivity = "com.rfid_demo.main.MainActivity";
static const QString defaultLauncherPackage = "com.android.launcher3";
static const QString defaultLauncherActivity = "com.android.launcher3.Launcher";

adb_tool::adb_tool(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	//初始化界面
	ui->progressBar->setValue(0);
	p = new QProcess(this);
	ui->setupButton->setChecked(true);
	ui->installtypewdg->hide();
	exePath = QCoreApplication::applicationDirPath()+"/platform-tools";

	QSettings setting("./Setting.ini", QSettings::IniFormat);//保存路径信息至运行目录
	QString lastPath = setting.value("LastFilePath").toString();//获取路径
	QString PathName = setting.value("LastFilePath", "").toString();	
	QFile file(PathName);
	if (file.exists())
	{
		ui->pathtEdit->setText(PathName);//文件名称显示
	}

	apkPackage = setting.value("apkPackage", defaultApkPackage).toString();
	apkActivity = setting.value("apkActivity", defaultApkActivity).toString();
	launcherPackage = setting.value("launcherPackage", defaultLauncherPackage).toString();
	launcherActivity = setting.value("launcherActivity", defaultLauncherActivity).toString();

}

adb_tool::~adb_tool()
{
	if (p)
	{
		delete p;
		p = NULL;
	}
	delete ui;

}


//检测装置
void adb_tool::on_checkButton_clicked()
{
	//p = new QProcess(this);
	QString test = QString("%1/adb shell getprop ro.product.model").arg(exePath);

	p->start("cmd", QStringList() << "/c" << QString("%1/adb shell getprop ro.product.model").arg(exePath));
	p->waitForStarted();
	p->waitForFinished();
	//成功则提示连接成功并打印设备型号，否则打印连接失败
	QString checkTemp = QString::fromLocal8Bit(p->readAllStandardOutput());
	if (checkTemp.isNull())
	{
		QMessageBox testMassage;
		testMassage.setWindowTitle("提示！");
		testMassage.setText("连接失败！");
		testMassage.exec();
	}
	else
	{
		QMessageBox testMassage;
		testMassage.setWindowTitle("提示！");
		testMassage.setText("设备已连接!\n设备型号:" + checkTemp);
		testMassage.exec();
	}

}


//浏览功能
void adb_tool::on_choseButton_clicked()
{
	//选择安装程序功能只能显示后缀为apk的文件
	if (ui->setupButton->isChecked())
	{
		QDir dir;
		QSettings setting("./Setting.ini", QSettings::IniFormat);//保存路径信息至运行目录
		QString lastPath = setting.value("LastFilePath").toString();//获取路径
		PathName = QFileDialog::getOpenFileName(this, "请选择文件路径", lastPath, "*apk");
		if (!PathName.isEmpty())
		{
			setting.setValue("LastFilePath", PathName);//记录最后一次使用路径
		}
		//PathName.replace("/","\\");// 单斜杠转换双斜杠
		ui->pathtEdit->setText(PathName);//文件名称显示


	}
	//选择上传文件时能选择所有文件类型
	if (ui->uploadButton->isChecked())
	{
		QDir dir;
		PathName = QFileDialog::getOpenFileName(this, "请选择文件路径", "D:/");
		//PathName.replace("/","\\");// 单斜杠转换双斜杠
		ui->pathtEdit->setText(PathName);//文件名称显示
		/*
		//push不支持多个文件方式，可能需要另外做脚本，先实现单个文件
		QDir dir;
		QStringList PathName = QFileDialog::getOpenFileNames(this,"请选择文件路径","D:/");
		//PathName.replace("/","\\");  // 单斜杠转换双斜杠
		for(int i=0;i<PathName.size();i++)
		{
		ui->pathtEdit->append(PathName.at(i));//文件名称显示
		}
		*/

	}
}

//开始功能
void adb_tool::on_startButton_clicked() //点击开始按钮根据勾选操作类型执行不同的adb命令
{
	pathinfo = ui->pathtEdit->text();
	if (pathinfo.isEmpty())
	{
		QMessageBox testMassage;
		testMassage.setWindowTitle("提示！");
		testMassage.setText("请选择文件");
		testMassage.exec();
		return;
	}
	QFile file(pathinfo);
	if (!file.exists())
	{
		QMessageBox testMassage;
		testMassage.setWindowTitle("提示！");
		testMassage.setText("文件不存在");
		testMassage.exec();
		return;
	}


	//安装程序
	if (ui->setupButton->isChecked())
	{	
		p->start("cmd", QStringList() << "/c" << QString("%1/adb shell getprop ro.product.model").arg(exePath));//检查设备是否存在
		p->waitForStarted();
		p->waitForFinished();
		QString checkTemp = QString::fromUtf8(p->readAllStandardOutput());
		
		if (checkTemp.isNull())
		{
			QMessageBox testMassage;
			testMassage.setWindowTitle("提示！");
			testMassage.setText("设备未连接");
			testMassage.exec();
		}
		else
		{
			bool b = pathinfo.contains(QRegExp("[\\x4e00-\\x9fa5]+"));//检查路径中是否包含中文
			if (b == true)
			{
				QMessageBox testMassage;
				testMassage.setWindowTitle("提示！");
				testMassage.setText("路径不能包含中文字符");
				testMassage.exec();
			}
			else
			{
				p->start("cmd", QStringList() << "/c" << QString("%1/adb install").arg(exePath) << pathinfo);//调用cmd命令执行安装
																					//p->waitForStarted();
				if (!p->waitForStarted())
				{
					QMessageBox testMassage;
					testMassage.setWindowTitle("提示！");
					testMassage.setText("执行失败！");
					testMassage.exec();
				}
				for (int i = 0; i <= 70; i++)
				{
					ui->progressBar->setValue(i);//进度条，执行命令无法获取命令执行状态，先实现进度效果
				}
				//p->waitForFinished();
				if (!p->waitForFinished())
				{
					QMessageBox testMassage;
					testMassage.setText("执行失败！");
					testMassage.exec();
				}
				ui->progressBar->setValue(100);//成功后设置进度条值为100%
				QString startTemp = QString::fromUtf8(p->readAllStandardOutput());

				if (ui->bootCkb->isChecked())
				{
					p->start("cmd", QStringList() << "/c" << QString("%1/adb shell cmd package set-home-activity %2/%3").arg(exePath).arg(apkPackage).arg(apkActivity));
				}
				else
				{
					p->start("cmd", QStringList() << "/c" << QString("%1/adb shell cmd package set-home-activity %2/%3").arg(exePath).arg(launcherPackage).arg(launcherActivity));
				}

				p->waitForStarted();
				p->waitForFinished();

				p->start("cmd", QStringList() << "/c" << QString("%1/adb shell am start -n  %2/%3").arg(exePath).arg(apkPackage).arg(apkActivity));
				p->waitForStarted();
				p->waitForFinished();

				startTemp = QString::fromUtf8(p->readAllStandardOutput());

				
				//p->start("cmd", QStringList() << "/c" << "adb reboot");	//push文件至手机sdcrad目录
				//p->waitForStarted();
				//p->waitForFinished();

				QMessageBox testMassage;
				testMassage.setWindowTitle("提示！");
				testMassage.setText("执行安装成功！请检查或操作手机执行安装!\n安装信息：" + startTemp);
				testMassage.exec();
				ui->progressBar->setValue(0);//初始化进度条

			}


		}

	}


	//上传文件
	if (ui->uploadButton->isChecked())
	{
		p->start("cmd", QStringList() << "/c" << QString("%1/adb shell getprop ro.product.model").arg(exePath));	//push文件至手机sdcrad目录
		p->waitForStarted();
		p->waitForFinished();
		QString checkTemp = QString::fromUtf8(p->readAllStandardOutput());
		pathinfo = ui->pathtEdit->text();
		if (checkTemp.isNull())
		{
			QMessageBox testMassage;
			testMassage.setWindowTitle("提示！");
			testMassage.setText("设备未连接");
			testMassage.exec();
		}
		else
		{			
			bool b = pathinfo.contains(QRegExp("[\\x4e00-\\x9fa5]+"));//检查路径中是否包含中文
			if (b == true)
			{
				QMessageBox testMassage;
				testMassage.setWindowTitle("提示！");
				testMassage.setText("路径不能包含中文字符");
				testMassage.exec();
			}
			else
			{
				//QString path = ui->pathtEdit->toPlainText();
				p->start("cmd", QStringList() << "/c" << QString("%1/adb push").arg(exePath) << pathinfo << "/sdcard");//上传文件至/sdcard根目录
				//p->waitForStarted();
				if (!p->waitForStarted(-1))
				{
					QMessageBox testMassage;
					testMassage.setWindowTitle("提示！");
					testMassage.setText("上传失败！");
					testMassage.exec();
				}
				for (int i = 0; i <= 70; i++)
				{
					ui->progressBar->setValue(i);//进度条，执行命令无法获取命令执行状态，先实现进度效果
				}
				//p->waitForFinished();
				if (!p->waitForFinished(-1))
				{
					QMessageBox testMassage;
					testMassage.setWindowTitle("提示！");
					testMassage.setText("上传失败！");
					testMassage.exec();
				}
				ui->progressBar->setValue(100);//成功后设置进度条值为100%

				QString startTemp = QString::fromUtf8(p->readAllStandardOutput());//使用UTF8可暂时过滤中文文件传输丢失后续或乱码问题，直接复制失败				

				QMessageBox testMassage;
				testMassage.setWindowTitle("提示！");
				testMassage.setText("执行上传命令成功！请检查！\n下载信息：" + startTemp);
				testMassage.exec();
				ui->progressBar->setValue(0);//初始化进度条
			}

		}

	}

}

//退出功能
void adb_tool::on_cancelButton_clicked()
{
	QWidget::close(); //点击退出按钮重置界面并关闭程序，kill adb进程，先实现关闭窗口，adb命令不怎么消耗资源
}




void adb_tool::on_rebootBtn_clicked()
{
	p->start("cmd", QStringList() << "/c" << QString("%1/adb shell getprop ro.product.model").arg(exePath));//检查设备是否存在
	p->waitForStarted();
	p->waitForFinished();
	QString checkTemp = QString::fromUtf8(p->readAllStandardOutput());
	pathinfo = ui->pathtEdit->text();
	if (checkTemp.isNull())
	{
		QMessageBox testMassage;
		testMassage.setWindowTitle("提示！");
		testMassage.setText("设备未连接");
		testMassage.exec();
		return;
	}

	p->start("cmd", QStringList() << "/c" << QString("%1/adb reboot").arg(exePath));//检查设备是否存在
	p->waitForStarted();
	p->waitForFinished();

	QString startTemp = QString::fromUtf8(p->readAllStandardOutput());//使用UTF8可暂时过滤中文文件传输丢失后续或乱码问题，直接复制失败				

	QMessageBox testMassage;
	testMassage.setWindowTitle("提示！");
	testMassage.setText("重启成功! " + startTemp);
	testMassage.exec();
	ui->progressBar->setValue(0);
}



void adb_tool::on_uninstallBtn_clicked()
{
	p->start("cmd", QStringList() << "/c" << QString("%1/adb shell getprop ro.product.model").arg(exePath));//检查设备是否存在
	p->waitForStarted();
	p->waitForFinished();
	QString checkTemp = QString::fromUtf8(p->readAllStandardOutput());
	pathinfo = ui->pathtEdit->text();
	if (checkTemp.isNull())
	{
		QMessageBox testMassage;
		testMassage.setWindowTitle("提示！");
		testMassage.setText("设备未连接");
		testMassage.exec();
		return;
	}

	p->start("cmd", QStringList() << "/c" << QString("%1/adb shell pm uninstall -k %2").arg(exePath).arg(apkPackage));//检查设备是否存在
	p->waitForStarted();
	p->waitForFinished();

	QString startTemp = QString::fromUtf8(p->readAllStandardOutput());//使用UTF8可暂时过滤中文文件传输丢失后续或乱码问题，直接复制失败				

	QMessageBox testMassage;
	testMassage.setWindowTitle("提示！");
	testMassage.setText("卸载成功! " + startTemp);
	testMassage.exec();
	ui->progressBar->setValue(0);
}


void adb_tool::on_setbootBtn_clicked()
{
	p->start("cmd", QStringList() << "/c" << QString("%1/adb shell getprop ro.product.model").arg(exePath));//检查设备是否存在
	p->waitForStarted();
	p->waitForFinished();
	QString checkTemp = QString::fromUtf8(p->readAllStandardOutput());
	pathinfo = ui->pathtEdit->text();
	if (checkTemp.isNull())
	{
		QMessageBox testMassage;
		testMassage.setWindowTitle("提示！");
		testMassage.setText("设备未连接");
		testMassage.exec();
		return;
	}

	if (ui->bootCkb->isChecked())
	{
		p->start("cmd", QStringList() << "/c" << QString("%1/adb shell cmd package set-home-activity %2/%3").arg(exePath).arg(apkPackage).arg(apkActivity));
	}
	else
	{
		p->start("cmd", QStringList() << "/c" << QString("%1/adb shell cmd package set-home-activity %2/%3").arg(exePath).arg(launcherPackage).arg(launcherActivity));
	}

	p->waitForStarted();
	p->waitForFinished();

	QString startTemp = QString::fromUtf8(p->readAllStandardOutput());//使用UTF8可暂时过滤中文文件传输丢失后续或乱码问题，直接复制失败				

	QMessageBox testMassage;
	testMassage.setWindowTitle("提示！");
	testMassage.setText("设置成功! " + startTemp);
	testMassage.exec();
	ui->progressBar->setValue(0);
}