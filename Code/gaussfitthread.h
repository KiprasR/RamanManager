#ifndef GAUSSFITTHREAD_H
#define GAUSSFITTHREAD_H

#define A0_LIM 1.0e10
#define V0_LIM 1.0e-10


#include <QThread>
#include <QMutex>
#include <QDebug>
#include <QTime>

#include "spectrumdata.h"
#include "helpful_functions.h"
#include "fitmrq.h"

class GaussFitThread : public QThread
{
    Q_OBJECT
public:
    explicit GaussFitThread(QObject *parent=0);
    void AssignVariables(SpectrumData *mainData, QVector < QVector < double > > *mainFSRS,
                         QVector < double > *coeffs, QVector < QVector < double > > *fitFSRS,
                         QVector < double > *paramTablePTR, double toll);
    void run();
    bool Stop;

signals:
    void ThreadCancelled();
    void ProgressChanged(int);
    void SendInfo(QString);
    void SendParamsVector();
    
public slots:
    
private:
    SpectrumData *mData;
    QVector < QVector < double > > *mFSRS;
    QVector < double > cfs;
    QVector < double > *params;
    QVector < QVector < double > > *fFSRS;
    double TOL;

    inline bool CheckForINF(QVector < double > *inp);
};

#endif // GAUSSFITTHREAD_H
