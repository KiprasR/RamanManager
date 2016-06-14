#ifndef REMOVEDIALOG_H
#define REMOVEDIALOG_H

#include <QDialog>
#include "spectrumdata.h"

namespace Ui {
class RemoveDialog;
}

class RemoveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoveDialog(QWidget *parent = 0, SpectrumData *mainData=NULL);
    ~RemoveDialog();

signals:
    void SendRemovableRegion(bool wl,double w0,double w1,bool pt,double p0,double p1);

private slots:
    void on_waveBox_toggled(bool checked);
    void on_probeBox_toggled(bool checked);
    void on_applyButton_clicked();
    void on_cancelButton_clicked();

    void on_waveStart_editingFinished();
    void on_waveEnd_editingFinished();
    void on_probeStart_editingFinished();
    void on_probeEnd_editingFinished();

private:
    SpectrumData *mData;
    Ui::RemoveDialog *ui;
};

#endif // REMOVEDIALOG_H
