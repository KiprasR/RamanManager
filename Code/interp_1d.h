#ifndef INTERP_1D_H
#define INTERP_1D_H

#include "nrutil.h"
#include <QVector>
#include <QDebug>

struct Base_interp
{
    int n, mm, jsav, cor, dj;
    const double *xx, *yy;
    Base_interp(const QVector < double > &x, const double *y, int m)
        : n(x.size()), mm(m), jsav(0), cor(0), xx(&x[0]), yy(y) {
        dj = MAX(1,(int)pow((double)n,0.25));
    }

    double interp(double x) {
        int jlo = cor ? hunt(x) : locate(x);
        return rawinterp(jlo,x);
    }

    int locate(const double x);
    int hunt(const double x);

    double virtual rawinterp(int jlo, double x) = 0;

};

struct Poly_interp : Base_interp
{
    double dy;
    Poly_interp(const QVector < double > &xv, const QVector < double > &yv, int m)
        : Base_interp(xv,&yv[0],m), dy(0.) {}
    double rawinterp(int jl, double x);
};

struct Rat_interp : Base_interp
{
    double dy;
    Rat_interp(const QVector < double > &xv, const QVector < double > &yv, int m)
        : Base_interp(xv,&yv[0],m), dy(0.) {}
    double rawinterp(int jl, double x);
};

struct Spline_interp : Base_interp
{
    QVector < double > y2;

    Spline_interp(const QVector < double > &xv, const QVector < double > &yv, double yp1=1.e99, double ypn=1.e99)
    : Base_interp(xv,&yv[0],2), y2(xv.size())
    {sety2(&xv[0],&yv[0],yp1,ypn);}

    Spline_interp(const QVector < double > &xv, const double *yv, double yp1=1.e99, double ypn=1.e99)
    : Base_interp(xv,yv,2), y2(xv.size())
    {sety2(&xv[0],yv,yp1,ypn);}

    void sety2(const double *xv, const double *yv, double yp1, double ypn);
    double rawinterp(int jl, double xv);
};

struct BaryRat_interp : Base_interp
{
    QVector < double > w;
    int d;
    BaryRat_interp(const QVector < double > &xv, const QVector < double > &yv, int dd);
    double rawinterp(int jl, double x);
    double interp(double x);
};

#endif // INTERP_1D_H
