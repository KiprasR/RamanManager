#include "ludcmp.h"

LUdcmp::LUdcmp(const QVector < QVector < double > > &a) : n(a.size()), lu(a), indx(n), aref(a) {
    const double TINY=1.0e-40;
    int i,imax,j,k;
    double big,temp;
    QVector < double > vv(n);
    d=1.0;
    for (i=0;i<n;i++) {
        big=0.0;
        for (j=0;j<n;j++)
            if ((temp=abs(lu[i][j])) > big) big=temp;
        if (big == 0.0) qDebug("Singular matrix in LUdcmp");
        vv[i]=1.0/big;
    }
    for (k=0;k<n;k++) {
        big=0.0;
        for (i=k;i<n;i++) {
            temp=vv[i]*abs(lu[i][k]);
            if (temp > big) {
                big=temp;
                imax=i;
            }
        }
        if (k != imax) {
            for (j=0;j<n;j++) {
                temp=lu[imax][j];
                lu[imax][j]=lu[k][j];
                lu[k][j]=temp;
            }
            d = -d;
            vv[imax]=vv[k];
        }
        indx[k]=imax;
        if (lu[k][k] == 0.0) lu[k][k]=TINY;
        for (i=k+1;i<n;i++) {
            temp=lu[i][k] /= lu[k][k];
            for (j=k+1;j<n;j++)
                lu[i][j] -= temp*lu[k][j];
        }
    }
}

void LUdcmp::solve(const QVector < double > &b, QVector < double > &x)
{
    int i,ii=0,ip,j;
    double sum;
    if (b.size() != n || x.size() != n)
        qDebug("LUdcmp::solve bad sizes");
    for (i=0;i<n;i++) x[i] = b[i];
    for (i=0;i<n;i++) {
        ip=indx[i];
        sum=x[ip];
        x[ip]=x[i];
        if (ii != 0)
            for (j=ii-1;j<i;j++) sum -= lu[i][j]*x[j];
        else if (sum != 0.0)
            ii=i+1;
        x[i]=sum;
    }
    for (i=n-1;i>=0;i--) {
        sum=x[i];
        for (j=i+1;j<n;j++) sum -= lu[i][j]*x[j];
        x[i]=sum/lu[i][i];
    }
}

void LUdcmp::solve(const QVector < QVector < double > > &b, QVector < QVector < double > > &x)
{
    int i,j,m=b[0].size();
    if (b.size() != n || x.size() != n || b[0].size() != x[0].size())
        qDebug("LUdcmp::solve bad sizes");
    QVector < double > xx(n);
    for (j=0;j<m;j++) {
        for (i=0;i<n;i++) xx[i] = b[i][j];
        solve(xx,xx);
        for (i=0;i<n;i++) x[i][j] = xx[i];
    }
}

void LUdcmp::inverse(QVector < QVector < double > > &ainv)
{
    int i,j;
    ainv.resize(n);
    for (i=0;i<n;i++)
        ainv[i].resize(n);
    for (i=0;i<n;i++) {
        for (j=0;j<n;j++) ainv[i][j] = 0.;
        ainv[i][i] = 1.;
    }
    solve(ainv,ainv);
}

double LUdcmp::det()
{
    double dd = d;
    for (int i=0;i<n;i++) dd *= lu[i][i];
    return dd;
}

void LUdcmp::mprove(const QVector < double > &b, QVector < double > &x)
{
    int i,j;
    QVector < double > r(n);
    for (i=0;i<n;i++) {
        long double sdp = -b[i];
        for (j=0;j<n;j++)
            sdp += (long double)aref[i][j] * (long double)x[j];
        r[i]=sdp;
    }
    solve(r,r);
    for (i=0;i<n;i++) x[i] -= r[i];
}
