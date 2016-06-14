#ifndef READ_PP_STUFF_H
#define READ_PP_STUFF_H

#include <QVector>
#include <QString>

struct PPTempData
{
    QVector<double> smp00,smp01;
    QString NextType;
    int NextScanNo;
    double NextDelayPump;
    bool FileEnd;
};

struct PDPTempData : PPTempData
{
    QVector<double> smp10,smp11,
                    aux01,aux11;
    double NextDelayDump;
};

#endif // READ_PP_STUFF_H
