#ifndef TABUPLOADFORM_H
#define TABUPLOADFORM_H

#include <QWidget>

namespace Ui {
class TabUploadForm;
}
class FiltrateTagFor6CDlg;
class FlashConfigFor6CDlg;
class UploadPramForm;
class TabUploadForm : public QWidget
{
    Q_OBJECT
    
public:
    explicit TabUploadForm(QWidget *parent = 0);
    ~TabUploadForm();
    
private:
    Ui::TabUploadForm *ui;
	FiltrateTagFor6CDlg* mFiltrateTagFor6CDlg;
	FlashConfigFor6CDlg* mFlashConfigFor6CDlg;
	UploadPramForm* mUploadPramForm;
};

#endif // TABUPLOADFORM_H
