#ifndef FSRSPREVIEWDIALOG_H
#define FSRSPREVIEWDIALOG_H

#include <QDialog>
#include "customplot2d.h"

namespace Ui {
class FSRSPreviewDialog;
}

class FSRSPreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FSRSPreviewDialog(QWidget *parent = 0, SpectrumData *mainData = NULL, QVector < QVector < double > > *mainFSRS = NULL,
                               QVector < QVector < double > > *auxFSRS = NULL);
    ~FSRSPreviewDialog();

private:
    Ui::FSRSPreviewDialog *ui;
};

#endif // FSRSPREVIEWDIALOG_H
