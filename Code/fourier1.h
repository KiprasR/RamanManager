#ifndef FOURIER1_H
#define FOURIER1_H

#define MULT1 1.06446701943
#define MULT2 3.55970733125

#include <QDebug>
#include "nrutil.h"
#include "helpful_functions.h"

void four1(double *data, const int n, const int isign);
void four1(QVector<double> &data, const int isign);
void four1(QVector<double> *data, const int isign);
void realft(QVector<double> &data, const int isign);
void sinft(QVector<double> &y);
void cosft1(QVector<double> &y, const int isign);
void cosft2(QVector<double> &y, const int isign);

void fftshift(QVector<double> *data);
void fft_wrapper(const QVector<double> *data, QVector<double> *complex_spec);
void ifft_wrapper(const QVector<double> *complex_spec, QVector<double> *complex_data);
void ifft_wrapper(const QVector<double> *complex_spec, QVector<double> *data_r, QVector<double> *data_i, bool shift=false);

void fft_gaussian_convolution(QVector<double> *x_in, QVector<double> *y_in, double g_fwhm, QVector<double> *x_out,
                              QVector<double> *y_out, bool normalize=true);

#endif // FOURIER1_H
