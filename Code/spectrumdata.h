#ifndef SPECTRUMDATA_H
#define SPECTRUMDATA_H

#include <QVector>

enum DataType { PP=1, PPRaw=2, PPRawAux=3, PDPRaw=4, PDPRawAux=6, PDPRawTShift=8 };

class SpectrumData
{
public:
    SpectrumData();
    ~SpectrumData();
    void ClearSpectra();

    int Type;

    QVector<double> ProbeTime;
    QVector<double> Wavelength;

    QVector< QVector<double> > WL00;
    QVector< QVector<double> > WL01;
    QVector< QVector<double> > WL10;
    QVector< QVector<double> > WL11;

    QVector< QVector<double> > AUX01;
    QVector< QVector<double> > AUX11;

    QVector< QVector<double> > FSRS;
};

#endif // SPECTRUMDATA_H
