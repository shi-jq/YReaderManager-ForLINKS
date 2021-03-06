﻿#ifndef GPRSCONFIGDLG_H
#define GPRSCONFIGDLG_H

#include <QWidget>

namespace Ui {
class GprsConfigDlg;
}
class QPushButton;
class QLineEdit;
class GprsConfigDlg : public QWidget
{
    Q_OBJECT
    
public:
    explicit GprsConfigDlg(QWidget *parent = 0);
    ~GprsConfigDlg();
protected:
	void showEvent(QShowEvent *event);  
private:
    Ui::GprsConfigDlg *ui;
	QPushButton* mQueryBtn;
	QPushButton* mSetBtn;
	QLineEdit* mApnEdit;
	QLineEdit* mAccoundEdit;
	QLineEdit* mPwdEdit;

	bool mFirst;
private:
	bool QueryAll();

	bool QueryApn();
	bool QueryAccount();
	bool QueryPwd();

	bool SetApn();
	bool SetAccount();
	bool SetPwd();
	
private slots:
	void slot_QueryBtnClicked();
	void slot_SetBtnClicked();
};

#endif // GPRSCONFIGDLG_H
