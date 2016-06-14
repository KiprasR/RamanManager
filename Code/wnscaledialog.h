#ifndef WNSCALEDIALOG_H
#define WNSCALEDIALOG_H

#include <QDialog>

namespace Ui {
class wnScaleDialog;
}

class wnScaleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit wnScaleDialog(QWidget *parent = 0);
    ~wnScaleDialog();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::wnScaleDialog *ui;

signals:
    void sendCalibration(double d1, double d2, double d3);
};

#endif // WNSCALEDIALOG_H
