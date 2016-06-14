#ifndef SVDFIT_H
#define SVDFIT_H

#include "nrutil.h"
#include <QVector>
#include <math.h>
#include <limits>

struct SVD {
        int m,n;
        QVector < QVector <double> > u,v;
        QVector <double> w;
        double eps, tsh;

        SVD(const QVector < QVector <double> > &a) : m(a.size()), n(a[0].size()), u(a), v(n), w(n) {
                eps = std::numeric_limits<double>::epsilon();

                for(int i=0;i<n;i++)
                    v[i].fill(0,n);
                decompose();
                reorder();

                tsh = 0.5*sqrt(m+n+1.)*w[0]*eps;
        }

        void solve(const QVector <double> &b, QVector <double> &x, double thresh);

        int rank(double thresh);
        int nullity(double thresh);
        QVector < QVector <double> > range(double thresh);
        QVector < QVector <double> > nullspace(double thresh);

        double inv_condition() {
                return (w[0] <= 0. || w[n-1] <= 0.) ? 0. : w[n-1]/w[0];
        }

        void decompose();
        void reorder();
        double pythag(const double a, const double b);
};

struct PolyFitSVD {
    int ndat, ma, n_poly;
    double tol;
    const QVector <double> *x,&y;
    QVector <double> (*funcs)(const double, const int);
    QVector <double> a;
    QVector < QVector <double> > covar;
    double chisq;

    PolyFitSVD(const QVector <double> &xx, const QVector <double> &yy, const int poly_deg,
        QVector <double> funks(const double, const int), const double TOL=1.e-12)
        : ndat(yy.size()), n_poly(poly_deg), tol(TOL), x(&xx), y(yy), funcs(funks) {}

    void fit() {
        int i,j,k;
        double thresh,sum;
        ma = funcs((*x)[0],n_poly).size();
        a.resize(ma);
        covar.resize(ma);
        for (i=0;i<ma;i++)
            covar[i].resize(ma);
        QVector < QVector <double> > aa(ndat);
        for (i=0;i<ndat;i++)
            aa[i].resize(ma);
        QVector <double> b(ndat),afunc(ma);
        for (i=0;i<ndat;i++) {
            afunc=funcs((*x)[i],n_poly);
            for (j=0;j<ma;j++) aa[i][j]=afunc[j];
            b[i]=y[i];
        }
        SVD svd(aa);
        thresh = (tol > 0. ? tol*svd.w[0] : -1.);
        svd.solve(b,a,thresh);
        chisq=0.0;
        for (i=0;i<ndat;i++) {
            sum=0.;
            for (j=0;j<ma;j++) sum += aa[i][j]*a[j];
            chisq += SQR(sum-b[i]);
        }
        for (i=0;i<ma;i++) {
            for (j=0;j<i+1;j++) {
                sum=0.0;
                for (k=0;k<ma;k++) if (svd.w[k] > svd.tsh)
                    sum += svd.v[i][k]*svd.v[j][k]/SQR(svd.w[k]);
                covar[j][i]=covar[i][j]=sum;
            }
        }

    }

    QVector <double> row(const QVector < QVector <double> > &a, const int i) {
        int j,n=a[0].size();
        QVector <double> ans(n);
        for (j=0;j<n;j++) ans[j] = a[i][j];
        return ans;
    }
};

void svdfit_wrapper(QVector<double> &x, QVector<double> &y, QVector<double> &coeffs, int poly_deg, double *chisq);
QVector<double> fpoly(const double x, const int fpoly_np);

#endif // SVDFIT_H
