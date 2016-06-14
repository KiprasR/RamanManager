#include "fitmrq.h"

void fgauss(const double x, const QVector < double > &a, double &y, QVector < double > &dyda) {
    int i,na=a.size();
    double fac,ex,arg;
    y=0.0;
    for (i=0;i<na-1;i+=3) {
        arg=(x-a[i+1])/a[i+2];
        ex=exp(-SQR(arg));
        fac=a[i]*ex*2.*arg;
        y += a[i]*ex;
        dyda[i]=ex;
        dyda[i+1]=fac/a[i+2];
        dyda[i+2]=fac*arg/a[i+2];
    }
}

void fgauss2(const double x, const QVector < double > &a, double &y, QVector < double > &dyda) {
    double arg,ex,fac;
    arg=((x-a[2])/a[3]);
    ex=exp(-FWHM*SQR(arg));
    fac=2*a[1]*arg*ex;

    y=a[0]+a[1]*ex;
    dyda[0]=1;
    dyda[1]=ex;
    dyda[2]=FWHM*fac/a[3];
    dyda[3]=FWHM*fac*arg/a[3];
}

void gaussj(QVector < QVector < double > > &a, QVector < QVector < double > > &b)
{
    int i,icol=0,irow=0,j,k,l,ll,n=a.size(),m=b[0].size();
    double big,dum,pivinv;
    QVector < int > indxc(n),indxr(n),ipiv(n);
    for (j=0;j<n;j++) ipiv[j]=0;
    for (i=0;i<n;i++) {
        big=0.0;
        for (j=0;j<n;j++)
            if (ipiv[j] != 1)
                for (k=0;k<n;k++) {
                    if (ipiv[k] == 0) {
                        if (fabs(a[j][k]) >= big) {
                            big=fabs(a[j][k]);
                            irow=j;
                            icol=k;
                        }
                    }
                }
        ++(ipiv[icol]);
        if (irow != icol) {
            for (l=0;l<n;l++) SWAP(a[irow][l],a[icol][l]);
            for (l=0;l<m;l++) SWAP(b[irow][l],b[icol][l]);
        }
        indxr[i]=irow;
        indxc[i]=icol;
        //if (a[icol][icol] == 0.0) qDebug("gaussj: Singular Matrix");
        pivinv=1.0/a[icol][icol];
        a[icol][icol]=1.0;
        for (l=0;l<n;l++) a[icol][l] *= pivinv;
        for (l=0;l<m;l++) b[icol][l] *= pivinv;
        for (ll=0;ll<n;ll++)
            if (ll != icol) {
                dum=a[ll][icol];
                a[ll][icol]=0.0;
                for (l=0;l<n;l++) a[ll][l] -= a[icol][l]*dum;
                for (l=0;l<m;l++) b[ll][l] -= b[icol][l]*dum;
            }
    }
    for (l=n-1;l>=0;l--) {
        if (indxr[l] != indxc[l])
            for (k=0;k<n;k++)
                SWAP(a[k][indxr[l]],a[k][indxc[l]]);
    }
}

