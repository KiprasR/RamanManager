#ifndef RESCALEAXESDIALOG_H
#define RESCALEAXESDIALOG_H

#include <QDialog>

namespace Ui {
class RescaleAxesDialog;
}

class RescaleAxesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RescaleAxesDialog(QWidget *parent = 0, double *y00 = NULL, double *y01 = NULL);
    ~RescaleAxesDialog();

private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::RescaleAxesDialog *ui;
    double  *y0_ptr, *y1_ptr;

};

#endif // RESCALEAXESDIALOG_H
