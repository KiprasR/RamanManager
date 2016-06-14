#ifndef NANREPORTDIALOG_H
#define NANREPORTDIALOG_H

#include <QDialog>

namespace Ui {
class NaNReportDialog;
}

class NaNReportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NaNReportDialog(QWidget *parent = 0, QVector<QString> *NaNs = NULL);
    ~NaNReportDialog();

private slots:
    void on_OKButton_clicked();

private:
    Ui::NaNReportDialog *ui;
};

#endif // NANREPORTDIALOG_H
