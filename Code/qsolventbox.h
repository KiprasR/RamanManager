#ifndef QSOLVENTBOX_H
#define QSOLVENTBOX_H

#include <QDebug>
#include <QMap>
#include <QComboBox>
#include <QSettings>
#include <QFileDialog>
#include "helpful_functions.h"

class QSolventBox : public QComboBox
{
    Q_OBJECT
public:
    explicit QSolventBox(QWidget *parent = 0);
    void setDirSettings(QSettings *actDir);
    QString getCurrentSolvent();
    QString getMapValue(QString key);

signals:
    void sendSolvent(QString file);

public slots:
    void solventIndexChanged(int arg);

private:
    QSettings *ActiveDir;
    QMap<QString, QString> Map;
};

#endif // QSOLVENTBOX_H
