#include "interp_1d.h"

int Base_interp::locate(const double x)
{
    int ju,jm,jl;
    if (n < 2 || mm < 2 || mm > n) qDebug("locate size error");
    bool ascnd=(xx[n-1] >= xx[0]);
    jl=0;
    ju=n-1;
    while (ju-jl > 1) {
        jm = (ju+jl) >> 1;
        if (x >= xx[jm] == ascnd)
            jl=jm;
        else
            ju=jm;
    }
    cor = abs(jl-jsav) > dj ? 0 : 1;
    jsav = jl;
    return MAX(0,MIN(n-mm,jl-((mm-2)>>1)));
}

int Base_interp::hunt(const double x)
{
    int jl=jsav, jm, ju, inc=1;
    if (n < 2 || mm < 2 || mm > n) qDebug("hunt size error");
    bool ascnd=(xx[n-1] >= xx[0]);
    if (jl < 0 || jl > n-1) {
        jl=0;
        ju=n-1;
    } else {
        if (x >= xx[jl] == ascnd) {
            for (;;) {
                ju = jl + inc;
                if (ju >= n-1) { ju = n-1; break;}
                else if (x < xx[ju] == ascnd) break;
                else {
                    jl = ju;
                    inc += inc;
                }
            }
        } else {
            ju = jl;
            for (;;) {
                jl = jl - inc;
                if (jl <= 0) { jl = 0; break;}
                else if (x >= xx[jl] == ascnd) break;
                else {
                    ju = jl;
                    inc += inc;
                }
            }
        }
    }
    while (ju-jl > 1) {
        jm = (ju+jl) >> 1;
        if (x >= xx[jm] == ascnd)
            jl=jm;
        else
            ju=jm;
    }
    cor = abs(jl-jsav) > dj ? 0 : 1;
    jsav = jl;
    return MAX(0,MIN(n-mm,jl-((mm-2)>>1)));
}

double Poly_interp::rawinterp(int jl, double x)
{
    int i,m,ns=0;
    double y,den,dif,dift,ho,hp,w;
    const double *xa = &xx[jl], *ya = &yy[jl];
    QVector < double > c(mm),d(mm);
    dif=abs(x-xa[0]);
    for (i=0;i<mm;i++) {
        if ((dift=abs(x-xa[i])) < dif) {
            ns=i;
            dif=dift;
        }
        c[i]=ya[i];
        d[i]=ya[i];
    }
    y=ya[ns--];
    for (m=1;m<mm;m++) {
        for (i=0;i<mm-m;i++) {
            ho=xa[i]-x;
            hp=xa[i+m]-x;
            w=c[i+1]-d[i];
            if ((den=ho-hp) == 0.0) qDebug("Poly_interp error");
            den=w/den;
            d[i]=hp*den;
            c[i]=ho*den;
        }
        y += (dy=(2*(ns+1) < (mm-m) ? c[ns+1] : d[ns--]));
    }
    return y;
}

double Rat_interp::rawinterp(int jl, double x)
{
    const double TINY=1.0e-99;
    int m,i,ns=0;
    double y,w,t,hh,h,dd;
    const double *xa = &xx[jl], *ya = &yy[jl];
    QVector < double > c(mm),d(mm);
    hh=abs(x-xa[0]);
    for (i=0;i<mm;i++) {
        h=abs(x-xa[i]);
        if (h == 0.0) {
            dy=0.0;
            return ya[i];
        } else if (h < hh) {
            ns=i;
            hh=h;
        }
        c[i]=ya[i];
        d[i]=ya[i]+TINY;
    }
    y=ya[ns--];
    for (m=1;m<mm;m++) {
        for (i=0;i<mm-m;i++) {
            w=c[i+1]-d[i];
            h=xa[i+m]-x;
            t=(xa[i]-x)*d[i]/h;
            dd=t-c[i+1];
            if (dd == 0.0) qDebug("Error in routine ratint");
            dd=w/dd;
            d[i]=c[i+1]*dd;
            c[i]=t*dd;
        }
        y += (dy=(2*(ns+1) < (mm-m) ? c[ns+1] : d[ns--]));
    }
    return y;
}

void Spline_interp::sety2(const double *xv, const double *yv, double yp1, double ypn)
{
    int i,k;
    double p,qn,sig,un;
    QVector < double > u(n-1);
    if (yp1 > 0.99e99)
        y2[0]=u[0]=0.0;
    else {
        y2[0] = -0.5;
        u[0]=(3.0/(xv[1]-xv[0]))*((yv[1]-yv[0])/(xv[1]-xv[0])-yp1);
    }
    for (i=1;i<n-1;i++) {
        sig=(xv[i]-xv[i-1])/(xv[i+1]-xv[i-1]);
        p=sig*y2[i-1]+2.0;
        y2[i]=(sig-1.0)/p;
        u[i]=(yv[i+1]-yv[i])/(xv[i+1]-xv[i]) - (yv[i]-yv[i-1])/(xv[i]-xv[i-1]);
        u[i]=(6.0*u[i]/(xv[i+1]-xv[i-1])-sig*u[i-1])/p;
    }
    if (ypn > 0.99e99)
        qn=un=0.0;
    else {
        qn=0.5;
        un=(3.0/(xv[n-1]-xv[n-2]))*(ypn-(yv[n-1]-yv[n-2])/(xv[n-1]-xv[n-2]));
    }
    y2[n-1]=(un-qn*u[n-2])/(qn*y2[n-2]+1.0);
    for (k=n-2;k>=0;k--)
        y2[k]=y2[k]*y2[k+1]+u[k];
}

double Spline_interp::rawinterp(int jl, double x)
{
    int klo=jl,khi=jl+1;
    double y,h,b,a;
    h=xx[khi]-xx[klo];
    if (h == 0.0) qDebug("Bad input to routine splint");
    a=(xx[khi]-x)/h;
    b=(x-xx[klo])/h;
    y=a*yy[klo]+b*yy[khi]+((a*a*a-a)*y2[klo]
        +(b*b*b-b)*y2[khi])*(h*h)/6.0;
    return y;
}

BaryRat_interp::BaryRat_interp(const QVector < double > &xv, const QVector < double > &yv, int dd)
        : Base_interp(xv,&yv[0],xv.size()), w(n), d(dd)
{
    if (n<=d) qDebug("d too large for number of points in BaryRat_interp");
    for (int k=0;k<n;k++) {
        int imin=MAX(k-d,0);
        int imax = k >= n-d ? n-d-1 : k;
        double temp = imin & 1 ? -1.0 : 1.0;
        double sum=0.0;
        for (int i=imin;i<=imax;i++) {
            int jmax=MIN(i+d,n-1);
            double term=1.0;
            for (int j=i;j<=jmax;j++) {
                if (j==k) continue;
                term *= (xx[k]-xx[j]);
            }
            term=temp/term;
            temp=-temp;
            sum += term;
        }
        w[k]=sum;
    }
}

double BaryRat_interp::rawinterp(int jl, double x)
{
    double num=0,den=0;
    for (int i=0;i<n;i++) {
        double h=x-xx[i];
        if (h == 0.0) {
            return yy[i];
        } else {
            double temp=w[i]/h;
            num += temp*yy[i];
            den += temp;
        }
    }
    return num/den;
}

double BaryRat_interp::interp(double x) {
    return rawinterp(1,x);
}
