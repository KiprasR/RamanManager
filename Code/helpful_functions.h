#ifndef HELPFUL_FUNCTIONS_H
#define HELPFUL_FUNCTIONS_H

#include <QFile>
#include <QString>
#include <QTextStream>

#include "nrutil.h"
#include "spectrumdata.h"

#define GAUSS_MULT 2.77258872224
#define RANDOMCOMPARETEMPLATE -123456
#define MAXLINELENGTH 64000
#define NanCheck(a) (a==a)

unsigned int GetCurveColor(int i);
const QString GetCurrentDir(QString fileName);
void locate(const QVector<double> *xx, double x, int *j);
void hunt(const QVector<double> *xx, double x, int *jlo);
void interpolate(QVector<double> *x,QVector<double> *y,QVector<double> *xnew,QVector<double> *ynew);
void interpolate_zero_pad(QVector<double> *x, QVector<double> *y, QVector<double> *xnew, QVector<double> *ynew);
void polynomialRecursionLog(QVector<double> *x,QVector<double> *coeffs,double logstart,QVector<double> *logy);
void polynomialRecursionLin(QVector<double> *x,QVector<double> *coeffs,QVector<double> *y);
void correctDispersion(SpectrumData *inputData, QVector<double> *coeffs);
double linToLog(double linVal, double logStart);
double logToLin(double logVal, double logStart);

void shell_sort_PP(QVector < double > &a, QVector < QVector < double > > &b, const int NScans);
void shell_sort_PDP(QVector < double > &a, QVector < QVector < double > > &b, QVector < QVector < double > > &c, QVector < QVector < double > > &d, int NScans);
void shell_sort_Raman(QVector < double > &a, QVector < QVector < double > > &b, QVector < QVector < double > > &c, QVector < QVector < double > > &d, QVector < QVector < double > > &e, int NScans);
void get_dispersion_rise_Raman(const SpectrumData *mainData, QVector<QVector<double> > *PPSpectra, double t_start, int every_nth, QVector < double > *x, QVector < double > *y);

template<class T1, class T2>
void shell_sort_TwoArrays(QVector< T1 > *a, QVector< T2 > *b)
{
    int i,j,inc,n=a->size();
    T1 v1;
    T2 v2;
    inc=1;
    do {
        inc *= 3;
        inc++;
    } while (inc <= n);
    do {
        inc /= 3;
        for (i=inc;i<n;i++) {
            v1=(*a)[i];
            v2=(*b)[i];
            j=i;
            while ((*a)[j-inc] > v1) {
                (*a)[j]=(*a)[j-inc];
                (*b)[j]=(*b)[j-inc];
                j -= inc;
                if (j < inc) break;
            }
            (*a)[j]=v1;
            (*b)[j]=v2;
        }
    } while (inc > 1);
}

double min(QVector<double> *Vect);
double min_index(QVector<double> *Vect, int *min_i);
double below_min(QVector<double> *Vect, double mult=1.0001);
double below_min(QVector<double> *Vect1, QVector<double> *Vect2, double mult=1.0001);
double max(QVector<double> *Vect);
double max_index(QVector<double> *Vect, int *max_i);
double above_max(QVector<double> *Vect, double mult=1.0001);
double above_max(QVector<double> *Vect1, QVector<double> *Vect2, double mult=1.0001);
double min(QVector<double> *Vect1,QVector<double> *Vect2);
double max(QVector<double> *Vect1,QVector<double> *Vect2);
void below_min_and_above_max_proportional(QVector<double> *inp, double *b_min, double *a_max, double perc_scale=5.0);
void below_min_and_above_max_proportional(QVector<double> *inp, double *b_min, double *a_max, int *mni, int *mxi, double perc_scale=5.0);
void min_and_max(const QVector<double> *inp, double *mmin, double *mmax);

void readXYFile(QString fileIn, QVector < double > *x0, QVector <double> *y0, bool norm=true);
void lorentzianDeriv(QVector<double> *x, double x0, double w, QVector<double> *y_out);
void gaussianDeriv(QVector<double> *x, double x0, double w, QVector<double> *y_out);
void normalize(QVector<double> *x);
void dummyFill(QVector<double> *input, QVector<double> *dummy);
QVector <double> fourier_abs(QVector <double> Vect);
QVector <double> fourier_real(QVector <double> Vect);

void exp_deriv(QVector<double> *x, double x0, double dx, QVector<double> *y);
void MeanAndSTD(QVector<double> *Input, double *Mean, double *STD);

inline bool InfCheck(double inp);

#endif // HELPFUL_FUNCTIONS_H
