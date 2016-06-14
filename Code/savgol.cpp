#include "savgol.h"
#include <QDebug>

void savgol(QVector < double > &c, const int np, const int nl, const int nr, const int ld, const int m)
{
    int j,k,imj,ipj,kk,mm;
    double fac,sum;
    if (np < nl+nr+1 || nl < 0 || nr < 0 || ld > m || nl+nr < m)
        qDebug("bad args in savgol");
    // QVector < int > indx(m+1);
    QVector < double > filler(m+1);
    QVector < QVector < double > > a(m+1);
    a.fill( filler );
    QVector < double > b(m+1);
    for (ipj=0;ipj<=(m << 1);ipj++) {
        sum=(ipj ? 0.0 : 1.0);
        for (k=1;k<=nr;k++) sum += pow(double(k),double(ipj));
        for (k=1;k<=nl;k++) sum += pow(double(-k),double(ipj));
        mm=MIN(ipj,2*m-ipj);
        for (imj = -mm;imj<=mm;imj+=2) a[(ipj+imj)/2][(ipj-imj)/2]=sum;
    }
    LUdcmp alud(a);
    for (j=0;j<m+1;j++) b[j]=0.0;
    b[ld]=1.0;
    alud.solve(b,b);
    for (kk=0;kk<np;kk++) c[kk]=0.0;
    for (k = -nl;k<=nr;k++) {
        sum=b[0];
        fac=1.0;
        for (mm=1;mm<=m;mm++) sum += b[mm]*(fac *= k);
        kk=(np-k) % np;
        c[kk]=sum;
    }
}

QVector < double > savgolfilter(QVector<double> *Input, const int nl, const int nr, const int ld, const int m)
{
    int i,j;
    QVector < double > Output;
    QVector < double > c;
    QVector < int > index;

    index.resize(nl+nr+1);
    c.resize(nl+nr+1);
    index[0]=0;

    j=2;
    for (i=1; i<nl+1; i++) {
      index[i]=i-j;
      j += 2;
    }

    j=1;
    for (i=nl+1; i<nl+nr+1; i++) {
      index[i]=i-j;
      j += 2;
    }

    savgol(c,nl+nr+1,nl,nr,ld,m);

    Output=(*Input);

    // Mirror @ boundaries:
    /*
    QVector < double > dummy=Input;
    for(i=0;i<nl;i++)
        dummy.push_front(Input[i+1]);
    for(i=0;i<nr;i++)
        dummy.push_back(Input[Input.size()-2-i]);
    */

    // Repeat @ boundaries:
    QVector < double > dummy=(*Input);
    for(i=0;i<nl;i++)
        dummy.push_front((*Input)[0]);
    for(i=0;i<nr;i++)
        dummy.push_back((*Input)[Input->size()-1]);

    Output.resize(dummy.size());
    for (i=0; i<dummy.size()-nr; i++) {
      Output[i]=0.0;
      for (j=0; j<nl+nr+1; j++)
        if (i+index[j]>=0)
          Output[i] += c[j]*dummy[i+index[j]];
    }

    Output=Output.mid(nl,Input->size());

/*
    for (i=0; i<Input.size()-nr; i++) {
      Output[i]=0.0;
      for (j=0; j<nl+nr+1; j++)
        if (i+index[j]>=0)
          Output[i] += c[j]*Input[i+index[j]];
    }
*/

    return Output;
}
