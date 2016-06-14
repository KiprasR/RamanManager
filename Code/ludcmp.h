#ifndef LUDCMP_H
#define LUDCMP_H

#include "nrutil.h"
#include <QVector>

struct LUdcmp
{
    int n;
    QVector < QVector < double > > lu;
    QVector < int > indx;
    double d;
    LUdcmp(const QVector < QVector < double > > &a);
    void solve(const QVector < double > &b, QVector < double > &x);
    void solve(const QVector < QVector < double > > &b, QVector < QVector < double > > &x);
    void inverse(QVector < QVector < double > > &ainv);
    double det();
    void mprove(const QVector < double > &b, QVector < double > &x);
    const QVector < QVector < double > > &aref;
};

#endif // LUDCMP_H
