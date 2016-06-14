#ifndef HOLMIUMCOMPARISON_H
#define HOLMIUMCOMPARISON_H

#include <QMap>
#include <QDialog>
#include <QToolTip>
#include <QKeyEvent>
#include <QSettings>
#include <QFileDialog>
#include <QStringList>
#include <QMessageBox>

#include "nrutil.h"
#include "qcustomplot.h"
#include "helpful_functions.h"
#include "qsolventbox.h"
#include "fourier1.h"

namespace Ui {
class HolmiumComparison;
}

class HolmiumComparison : public QDialog
{
    Q_OBJECT
    
public:
    explicit HolmiumComparison(QWidget *parent=0, QSettings *DirSettings=NULL, QVector <double> *mainGSx=NULL, QVector<double> *mainGSy=NULL);
    ~HolmiumComparison();
    double get_STD(QVector <double> &z);
    
private slots:
    void on_applyButton_clicked();
    void on_closeButton_clicked();
    void on_vBox_valueChanged(double arg1);
    void on_kBox_valueChanged(double arg1);
    void on_bBox_valueChanged(double arg1);
    void on_mBox_valueChanged(double arg1);
    void showPointToolTip(QMouseEvent *event);
    void showSelectedPoints(QMouseEvent *event);
    void addSelectedPoints(QMouseEvent *event);
    void clickEvent(QMouseEvent *event);
    void on_peakBox_valueChanged(int arg1);
    void on_autoButton_clicked();
    void on_fsrsBox_currentIndexChanged(const QString &arg1);
    void on_clickButton_clicked();
    void on_convoBox_clicked(bool checked);
    void on_convoValBox_valueChanged(double arg1);
    void solventChanged(QString solvent);

signals:
    void sendCalibration(double k, double b, double v0);

private:
    Ui::HolmiumComparison *ui;
    QSettings *ActiveDir;
    QVector < QCPItemText * > peakLabels;
    QVector<double> *GSx, *GSy;

    QVector <double> tempX, tempY; // Derinimui.
    QVector <double> solvX, solvY;
    QVector <double> historyX, historyY;
    QVector <double> selX, selY;
    QVector < int >  selI;

    int selCounter;
    int nprev;
    double v0, mn, mx,
               mn_solv, mx_solv;

    void convolve(QVector < double > *in);
    void keyPressEvent(QKeyEvent *event);
    void ReadSolventSpectrum(QString solventFile, bool cmToNm, QVector<double> *x0, QVector<double> *y0);
    void ReloadSampleSpectrum(QString solventName, bool fromFile);
    void InitializeGraphs();
    void InitializeLabels();
    void replot();
};

struct Amoeba {
    HolmiumComparison *pntr;
    const double ftol;
    int nfunc;
    int mpts;
    int ndim;
    double fmin;
    QVector < double > y;
    QVector < QVector < double > > p;
    Amoeba(const double ftoll, HolmiumComparison *ptr) : pntr(ptr) ,ftol(ftoll) {}

    QVector < double > minimize(const QVector < double > &point, const double del)
    {
        QVector < double > dels(point.size(),del);
        return minimize(point,dels);
    }

    QVector < double > minimize(const QVector < double > &point, const QVector < double > &dels)
    {
        int ndim=point.size();
        QVector < QVector < double > > pp(ndim+1);
        pp.fill(QVector < double > (ndim));
        for (int i=0;i<ndim+1;i++) {
            for (int j=0;j<ndim;j++)
                pp[i][j]=point[j];
            if (i !=0 ) pp[i][i-1] += dels[i-1];
        }
        return minimize(pp);
    }

    QVector < double > minimize(const QVector < QVector < double > > &pp)
    {
        const int NMAX=5000;
        const double TINY=1.0e-10;
        int ihi,ilo,inhi;
        mpts=pp.size();
        ndim=pp[0].size();
        QVector < double > psum(ndim),pmin(ndim),x(ndim);
        p=pp;
        y.resize(mpts);
        for (int i=0;i<mpts;i++) {
            for (int j=0;j<ndim;j++)
                x[j]=p[i][j];
            y[i]=pntr->get_STD(x);
        }
        nfunc=0;
        get_psum(p,psum);
        for (;;) {
            ilo=0;
            ihi = y[0]>y[1] ? (inhi=1,0) : (inhi=0,1);
            for (int i=0;i<mpts;i++) {
                if (y[i] <= y[ilo]) ilo=i;
                if (y[i] > y[ihi]) {
                    inhi=ihi;
                    ihi=i;
                } else if (y[i] > y[inhi] && i != ihi) inhi=i;
            }
            double rtol=2.0*fabs(y[ihi]-y[ilo])/(fabs(y[ihi])+fabs(y[ilo])+TINY);
            if (rtol < ftol) {
                SWAP(y[0],y[ilo]);
                for (int i=0;i<ndim;i++) {
                    SWAP(p[0][i],p[ilo][i]);
                    pmin[i]=p[0][i];
                }
                fmin=y[0];
                return pmin;
            }
            if (nfunc >= NMAX) qDebug("NMAX exceeded");
            nfunc += 2;
            double ytry=amotry(p,y,psum,ihi,-1.0);
            if (ytry <= y[ilo])
                ytry=amotry(p,y,psum,ihi,2.0);
            else if (ytry >= y[inhi]) {
                double ysave=y[ihi];
                ytry=amotry(p,y,psum,ihi,0.5);
                if (ytry >= ysave) {
                    for (int i=0;i<mpts;i++) {
                        if (i != ilo) {
                            for (int j=0;j<ndim;j++)
                                p[i][j]=psum[j]=0.5*(p[i][j]+p[ilo][j]);
                            y[i]=pntr->get_STD(psum);
                        }
                    }
                    nfunc += ndim;
                    get_psum(p,psum);
                }
            } else --nfunc;
        }
    }

    inline void get_psum(const QVector < QVector < double > > &p, QVector < double > &psum)
    {
        for (int j=0;j<ndim;j++) {
            double sum=0.0;
            for (int i=0;i<mpts;i++)
                sum += p[i][j];
            psum[j]=sum;
        }
    }

    double amotry(QVector < QVector < double > > &p, QVector < double > &y, QVector < double > &psum,
        const int ihi, const double fac)
    {
        QVector < double > ptry(ndim);
        double fac1=(1.0-fac)/ndim;
        double fac2=fac1-fac;
        for (int j=0;j<ndim;j++)
            ptry[j]=psum[j]*fac1-p[ihi][j]*fac2;
        double ytry=pntr->get_STD(ptry);
        if (ytry < y[ihi]) {
            y[ihi]=ytry;
            for (int j=0;j<ndim;j++) {
                psum[j] += ptry[j]-p[ihi][j];
                p[ihi][j]=ptry[j];
            }
        }
        return ytry;
    }
};

#endif // HOLMIUMCOMPARISON_H
