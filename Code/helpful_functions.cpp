#include "helpful_functions.h"

unsigned int GetCurveColor(int i)
{
    unsigned int ColorList [] = {
        4278190080u,4294901760u,4278255360u,4278190335u,4278255615u,4294902015u,4285098345u,4294967040u,
        4288684272u,4294944000u,4294907027u,4293821166u,4286611456u,4289014314u,4278190208u,4278215680u,
        4286513279u,4286513152u,4281290575u,4278222976u,4278222592u,4283105410u,4287299723u,4287299584u,
        4279834992u,4278225803u,4278190219u,4283788079u,4287317267u,4280453922u,4285238819u,4281240407u,
        4290283019u,4282924427u,4288696877u,4278190285u,4287889619u,4280332970u,4278243025u,4289864226u,
        4291237253u,4289736800u,4292613180u,4291979550u,4282168177u,4292519200u,4286381056u,4282811060u,
        4278254234u,4294956800u,4294937600u,4294919424u,4294902015u,4288335154u,4288230092u,4286578432u,
        4286545791u,4285563024u,4284456608u,4281519410u,4278255615u,4278255487u,4278255360u,4278239231u,
        4291659071u,4287245282u,4286023833u,4282962380u,4282441936u,4280193279u};
    return ColorList[i];
}

const QString GetCurrentDir(QString fileName)
{
    int i=0,j=0;
    while ((j=fileName.indexOf("/", j))!=-1){++j;i=j;}
    return fileName.left(i);
}

void locate(const QVector<double> *xx, double x, int *j)
{
    int ju, jm, jl;
    jl = 0;
    ju = xx->size();
    while (ju-jl > 1) {
        jm = (ju+jl) >> 1;
        if (x >= xx->at(jm))
            jl = jm;
        else
            ju = jm;
    }
    *j = jl;
}

void hunt(const QVector<double> *xx, double x, int *jlo)
{
    int jm,jhi,inc=1,n=xx->size()+1;
    if (*jlo < 0 || *jlo > n-1) {
        *jlo=0;
        jhi=n-1;
    } else {
        if (x >= xx->at(*jlo)) {
            for (;;) {
                jhi=(*jlo)+inc;
                if (jhi >= n-1) { jhi = n-1; break;}
                else if (x < xx->at(jhi)) break;
                else {
                    (*jlo) = jhi;
                    inc += inc;
                }
            }
        } else {
            jhi = (*jlo);
            for (;;) {
                (*jlo) = (*jlo) - inc;
                if ((*jlo) <= 0) { (*jlo) = 0; break;}
                else if (x >= xx->at((*jlo))) break;
                else {
                    jhi = (*jlo);
                    inc += inc;
                }
            }
        }
    }
    while (jhi-(*jlo) > 1) {
        jm=(jhi+(*jlo)) >> 1;
        if (x >= xx->at(jm))
            *jlo=jm;
        else
            jhi=jm;
    }
}

double linToLog(double linVal, double logStart)
{
    if(linVal<=logStart)
        return linVal;
    else
        return logStart+logStart*log10(linVal/logStart);
}

double logToLin(double logVal, double logStart)
{
    if(logVal<=logStart)
        return logVal;
    else
        return logStart*pow(10.0,(logVal-logStart)/logStart);
}

void interpolate(QVector<double> *x, QVector<double> *y, QVector<double> *xnew, QVector<double> *ynew)
{
    int i,loc;
    if(ynew->size()!=xnew->size()) ynew->resize( xnew->size() );
    for(i=0;i<xnew->size();i++)
    {
        locate(x,(*xnew)[i],&loc);
        if (loc==x->size()-1) loc--;
        (*ynew)[i]=((*xnew)[i]-(*x)[loc])*((*y)[loc]-(*y)[loc+1])/((*x)[loc]-(*x)[loc+1])+(*y)[loc];
    }
}

void interpolate_zero_pad(QVector<double> *x, QVector<double> *y, QVector<double> *xnew, QVector<double> *ynew)
{
    int i,loc;
    if(ynew->size()!=xnew->size()) ynew->resize( xnew->size() );
    for(i=0;i<xnew->size();i++)
    {
        if(((*xnew)[i] >= x->first()) && ((*xnew)[i] <= x->last()))
        {
            locate(x,(*xnew)[i],&loc);
            if (loc==x->size()-1) loc--;
            (*ynew)[i]=((*xnew)[i]-(*x)[loc])*((*y)[loc]-(*y)[loc+1])/((*x)[loc]-(*x)[loc+1])+(*y)[loc];
        }
        else
        {
            (*ynew)[i]=0.0;
        }
    }
}

void polynomialRecursionLin(QVector<double> *x,QVector<double> *coeffs,QVector<double> *y)
{
    int i,k;
    double temp;
    bool offset=false;
    if (coeffs->size()==1)
    {
        coeffs->push_back(0.0);
        offset = true;
    }
    if (y->size()!=x->size())
    {
        y->resize(x->size());
        y->fill(0);
    }
    for(i=0;i<x->size();i++)
    {
        temp=(*coeffs)[coeffs->size()-1]*(*x)[i]+(*coeffs)[coeffs->size()-2];
        for(k=coeffs->size()-3;k>=0;k--) temp=temp*(*x)[i]+(*coeffs)[k];
        (*y)[i]=temp;
    }
    if(offset) coeffs->pop_back();
}

void polynomialRecursionLog(QVector<double> *x,QVector<double> *coeffs,double logstart,QVector<double> *logy)
{
    int i,k;
    double temp;
    bool offset=false;
    if (coeffs->size()==1)
    {
        coeffs->push_back(0.0);
        offset = true;
    }
    if (logy->size()!=x->size())
    {
        logy->resize(x->size());
        logy->fill(0);
    }
    for(i=0;i<x->size();i++)
    {
        temp=(*coeffs)[coeffs->size()-1]*(*x)[i]+(*coeffs)[coeffs->size()-2];
        for(k=coeffs->size()-3;k>=0;k--) temp=temp*(*x)[i]+(*coeffs)[k];
        (*logy)[i]=linToLog(temp,logstart);
    }
    if(offset) coeffs->pop_back();
}

void correctDispersion(SpectrumData *inputData, QVector<double> *coeffs)
{
    int i,j;
    QVector<double> Shift(inputData->Wavelength.size()),
                    NewTime(inputData->ProbeTime.size()),
                    // All the data vectors:
                    Unshifted00(inputData->ProbeTime.size()),
                    Shifted00(inputData->ProbeTime.size()),
                    Unshifted01(inputData->ProbeTime.size()),
                    Shifted01(inputData->ProbeTime.size()),
                    Unshifted10(inputData->ProbeTime.size()),
                    Shifted10(inputData->ProbeTime.size()),
                    Unshifted11(inputData->ProbeTime.size()),
                    Shifted11(inputData->ProbeTime.size()),
                    UnshiftedAUX01(inputData->ProbeTime.size()),
                    ShiftedAUX01(inputData->ProbeTime.size()),
                    UnshiftedAUX11(inputData->ProbeTime.size()),
                    ShiftedAUX11(inputData->ProbeTime.size());

    polynomialRecursionLin(&inputData->Wavelength,coeffs,&Shift);
    for(i=0;i<inputData->Wavelength.size();i++)
    {
        for(j=0;j<inputData->ProbeTime.size();j++)
        {
            switch (inputData->Type)
            {
                case PDPRawAux:
                {
                    UnshiftedAUX01[j]=inputData->AUX01[j][i];
                    UnshiftedAUX11[j]=inputData->AUX11[j][i];
                }
                case PDPRaw:
                {
                    Unshifted10[j]=inputData->WL10[j][i];
                    Unshifted11[j]=inputData->WL11[j][i];
                }
                case PPRaw:
                {
                    Unshifted01[j]=inputData->WL01[j][i];
                    Unshifted00[j]=inputData->WL00[j][i];
                    break;
                }
                case PPRawAux:
                {
                    Unshifted01[j]=inputData->WL01[j][i];
                    Unshifted00[j]=inputData->WL00[j][i];
                    UnshiftedAUX01[j]=inputData->AUX01[j][i];
                    break;
                }
                case PP:
                {
                    Unshifted00[j]=inputData->FSRS[j][i];
                    break;
                }
                default: break;
            }
            NewTime[j]=inputData->ProbeTime[j]+Shift[i];
        }

        switch (inputData->Type)
        {
            case PDPRawAux:
            {
                interpolate(&inputData->ProbeTime,&UnshiftedAUX01,&NewTime,&ShiftedAUX01);
                interpolate(&inputData->ProbeTime,&UnshiftedAUX11,&NewTime,&ShiftedAUX11);
            }
            case PDPRaw:
            {
                interpolate(&inputData->ProbeTime,&Unshifted10,&NewTime,&Shifted10);
                interpolate(&inputData->ProbeTime,&Unshifted11,&NewTime,&Shifted11);
            }
            case PPRaw:
            {
                interpolate(&inputData->ProbeTime,&Unshifted00,&NewTime,&Shifted00);
                interpolate(&inputData->ProbeTime,&Unshifted01,&NewTime,&Shifted01);
                break;
            }
            case PPRawAux:
            {
                interpolate(&inputData->ProbeTime,&Unshifted00,&NewTime,&Shifted00);
                interpolate(&inputData->ProbeTime,&Unshifted01,&NewTime,&Shifted01);
                interpolate(&inputData->ProbeTime,&UnshiftedAUX01,&NewTime,&ShiftedAUX01);
                break;
            }
            case PP:
            {
                interpolate(&inputData->ProbeTime,&Unshifted00,&NewTime,&Shifted00);
                break;
            }
            default: break;
        }

        for(j=0;j<inputData->ProbeTime.size();j++)
        {
            switch (inputData->Type)
            {
                case PDPRawAux:
                {
                    inputData->AUX01[j][i] = ShiftedAUX01[j];
                    inputData->AUX11[j][i] = ShiftedAUX11[j];
                }
                case PDPRaw:
                {
                    inputData->WL10[j][i] = Shifted10[j];
                    inputData->WL11[j][i] = Shifted11[j];
                }
                case PPRaw:
                {
                    inputData->WL00[j][i] = Shifted00[j];
                    inputData->WL01[j][i] = Shifted01[j];
                    break;
                }
                case PPRawAux:
                {
                    inputData->WL00[j][i] = Shifted00[j];
                    inputData->WL01[j][i] = Shifted01[j];
                    inputData->AUX11[j][i] = ShiftedAUX11[j];
                    break;
                }
                case PP:
                {
                    inputData->FSRS[j][i] = Shifted00[j];
                    break;
                }
                default: break;
            }
        }
    }
    /*
    int i,j;
    QVector<double> Shift(inputData->Wavelength.size()),
                    NewTime(inputData->ProbeTime.size()),
                    Unshifted00(inputData->ProbeTime.size()),
                    Shifted00(inputData->ProbeTime.size()),
                    Unshifted01(inputData->ProbeTime.size()),
                    Shifted01(inputData->ProbeTime.size()),
                    Unshifted10(inputData->ProbeTime.size()),
                    Shifted10(inputData->ProbeTime.size()),
                    Unshifted11(inputData->ProbeTime.size()),
                    Shifted11(inputData->ProbeTime.size());

    polynomialRecursionLin(&inputData->Wavelength,coeffs,&Shift);
    for(i=0;i<inputData->Wavelength.size();i++)
    {
        for(j=0;j<inputData->ProbeTime.size();j++)
        {
            if(!inputData->WL00.isEmpty())
            {
                Unshifted00[j]=inputData->WL00[j][i];
                Unshifted01[j]=inputData->WL01[j][i];
                Unshifted10[j]=inputData->WL10[j][i];
                Unshifted11[j]=inputData->WL11[j][i];
            }
            else
            {
                Unshifted00[j]=inputData->FSRS[j][i];
            }
            NewTime[j]=inputData->ProbeTime[j]+Shift[i];
        }
        interpolate(&inputData->ProbeTime,&Unshifted00,&NewTime,&Shifted00);
        interpolate(&inputData->ProbeTime,&Unshifted01,&NewTime,&Shifted01);
        if(!inputData->WL00.isEmpty())
        {
            interpolate(&inputData->ProbeTime,&Unshifted10,&NewTime,&Shifted10);
            interpolate(&inputData->ProbeTime,&Unshifted11,&NewTime,&Shifted11);
        }

        for(j=0;j<inputData->ProbeTime.size();j++)
        {
            if(!inputData->WL00.isEmpty())
            {
                inputData->WL00[j][i] = Shifted00[j];
                inputData->WL01[j][i] = Shifted01[j];
                inputData->WL10[j][i] = Shifted10[j];
                inputData->WL11[j][i] = Shifted11[j];
            }
            else
            {
                inputData->FSRS[j][i] = Shifted00[j];
            }
        }
    }
    */
}

void shell_sort_PP(QVector < double > &a, QVector< QVector <double> > &b, const int NScans)
{
    int i,j,k,inc,n=a.size();
    double v1;
    QVector< QVector <double> > v2(NScans);

    inc=1;
    do {
        inc *= 3;
        inc++;
    } while (inc <= n);
    do {
        inc /= 3;
        for (i=inc;i<n;i++) {
            v1=a[i];
            for(k=0;k<NScans;k++)
                v2[k]=b[i+k*n];
            j=i;
            while (a[j-inc] > v1) {
                a[j]=a[j-inc];
                for(k=0;k<NScans;k++)
                    b[j+k*n]=b[j-inc+k*n];
                j -= inc;
                if (j < inc) break;
            }
            a[j]=v1;
            for(k=0;k<NScans;k++)
                b[j+k*n]=v2[k];
        }
    } while (inc > 1);
}

void shell_sort_PDP(QVector < double > &a, QVector < QVector < double > > &b, QVector < QVector < double > > &c, QVector < QVector < double > > &d, int NScans)
{
    int i,j,k,inc,n=a.size();
    double v1;
    QVector< QVector <double> > v2(NScans), v3(NScans), v4(NScans);

    inc=1;
    do {
        inc *= 3;
        inc++;
    } while (inc <= n);
    do {
        inc /= 3;
        for (i=inc;i<n;i++) {
            v1=a[i];
            for(k=0;k<NScans;k++)
            {
                v2[k]=b[i+k*n];
                v3[k]=c[i+k*n];
                v4[k]=d[i+k*n];
            }
            j=i;
            while (a[j-inc] > v1) {
                a[j]=a[j-inc];
                for(k=0;k<NScans;k++)
                {
                    b[j+k*n]=b[j-inc+k*n];
                    c[j+k*n]=c[j-inc+k*n];
                    d[j+k*n]=d[j-inc+k*n];
                }
                j -= inc;
                if (j < inc) break;
            }
            a[j]=v1;
            for(k=0;k<NScans;k++)
            {
                b[j+k*n]=v2[k];
                c[j+k*n]=v3[k];
                d[j+k*n]=v4[k];
            }
        }
    } while (inc > 1);
}

void shell_sort_Raman(QVector < double > &a, QVector < QVector < double > > &b, QVector < QVector < double > > &c, QVector < QVector < double > > &d, QVector < QVector < double > > &e, int NScans)
{
    int i,j,k,inc,n=a.size();
    double v1;
    QVector< QVector <double> > v2(NScans), v3(NScans), v4(NScans), v5(NScans);

    inc=1;
    do {
        inc *= 3;
        inc++;
    } while (inc <= n);
    do {
        inc /= 3;
        for (i=inc;i<n;i++) {
            v1=a[i];
            for(k=0;k<NScans;k++)
            {
                v2[k]=b[i+k*n];
                v3[k]=c[i+k*n];
                v4[k]=d[i+k*n];
                v5[k]=e[i+k*n];
            }
            j=i;
            while (a[j-inc] > v1) {
                a[j]=a[j-inc];
                for(k=0;k<NScans;k++)
                {
                    b[j+k*n]=b[j-inc+k*n];
                    c[j+k*n]=c[j-inc+k*n];
                    d[j+k*n]=d[j-inc+k*n];
                    e[j+k*n]=e[j-inc+k*n];
                }
                j -= inc;
                if (j < inc) break;
            }
            a[j]=v1;
            for(k=0;k<NScans;k++)
            {
                b[j+k*n]=v2[k];
                c[j+k*n]=v3[k];
                d[j+k*n]=v4[k];
                e[j+k*n]=v5[k];
            }
        }
    } while (inc > 1);
}

double min(QVector<double> *Vect)
{
    double mmn=Vect->at(0);
    for (int i=0;i<Vect->size();i++)
        if(Vect->at(i)<mmn) mmn=Vect->at(i);
    return mmn;
}

double min_index(QVector<double> *Vect, int *min_i)
{
    double mmn=Vect->at(0);
    for (int i=0;i<Vect->size();i++)
        if(Vect->at(i)<=mmn) {mmn=Vect->at(i); (*min_i)=i;}
    return mmn;
}

double below_min(QVector<double> *Vect, double mult)
{
    double mmn=min(Vect);
    if (mmn<=0)
        mmn=mmn*mult;
    else
        mmn=mmn*(2-mult);
    return mmn;
}

double below_min(QVector<double> *Vect1, QVector<double> *Vect2, double mult)
{
    double mmn=min(Vect1,Vect2);
    if (mmn<=0)
        mmn=mmn*mult;
    else
        mmn=mmn*(2-mult);
    return mmn;
}

double above_max(QVector<double> *Vect, double mult)
{
    double mmx=max(Vect);
    if (mmx>=0)
        mmx=mmx*mult;
    else
        mmx=mmx*(2-mult);
    return mmx;
}

double above_max(QVector<double> *Vect1, QVector<double> *Vect2, double mult)
{
    double mmx=max(Vect1,Vect2);
    if (mmx>=0)
        mmx=mmx*mult;
    else
        mmx=mmx*(2-mult);
    return mmx;
}

double max(QVector<double> *Vect)
{
    double mmx=Vect->at(0);
    for (int i=0;i<Vect->size();i++)
        if(Vect->at(i)>mmx) mmx=Vect->at(i);
    return mmx;
}

double max_index(QVector<double> *Vect, int *max_i)
{
    double mmx=Vect->at(0);
    for (int i=0;i<Vect->size();i++)
        if(Vect->at(i)>=mmx) {mmx=Vect->at(i); (*max_i)=i;}
    return mmx;
}

double min(QVector<double> *Vect1, QVector<double> *Vect2)
{
    double m1, m2;
    m1=min(Vect1);
    m2=min(Vect2);
    return MIN(m1,m2);
}

double max(QVector<double> *Vect1, QVector<double> *Vect2)
{
    double m1, m2;
    m1=max(Vect1);
    m2=max(Vect2);
    return MAX(m1,m2);
}

void below_min_and_above_max_proportional(QVector < double > *inp, double *b_min, double *a_max, double perc_scale)
{
    double mmn, mmx, extent;
    min_and_max(inp,&mmn,&mmx);
    extent=mmx-mmn;
    if (fabs(extent)<1e-12) extent=1e-12;
    (*b_min)=mmn-extent*perc_scale/100;
    (*a_max)=mmx+extent*perc_scale/100;
}

void below_min_and_above_max_proportional(QVector < double > *inp, double *b_min, double *a_max, int *mni, int *mxi, double perc_scale)
{
    double mmn=min_index(inp,mni),
           mmx=max_index(inp,mxi),
           extent;
    extent=mmx-mmn;
    if (fabs(extent)<1e-12) extent=1e-12;
    (*b_min)=mmn-extent*perc_scale/100;
    (*a_max)=mmx+extent*perc_scale/100;
}

void min_and_max(const QVector<double> *inp, double *mmin, double *mmax)
{
    if (!inp->isEmpty())
    {
        (*mmin)=(*mmax)=(*inp)[0];
        for (int i=1;i<inp->size();i++)
        {
            if ((*inp)[i]<(*mmin)) (*mmin)=(*inp)[i];
            if ((*inp)[i]>(*mmax)) (*mmax)=(*inp)[i];
        }
    }
}

QVector <double> fourier_abs(QVector <double> Vect)
{
    QVector <double> temp(Vect.size()/2);
    for (int i=0;i<temp.size();i++)
        temp[i]=sqrt(Vect[2*i]*Vect[2*i]+Vect[2*i+1]*Vect[2*i+1]);
    return temp;
}

QVector <double> fourier_real(QVector <double> Vect)
{
    QVector <double> temp(Vect.size()/2);
    for (int i=0;i<temp.size();i++)
        temp[i]=Vect[2*i];
    return temp;
}

void lorentzianDeriv(QVector<double> *x, double x0, double w, QVector<double> *y_out)
{
    if (y_out->size()!=x->size())
        y_out->resize(x->size());
    for(int i=0;i<x->size();i++)
        (*y_out)[i]=-2*w*w*(x->at(i)-x0)/(w*w+(x->at(i)-x0)*(x->at(i)-x0))/(w*w+(x->at(i)-x0)*(x->at(i)-x0));
}

void gaussianDeriv(QVector<double> *x, double x0, double w, QVector<double> *y_out)
{
    if (y_out->size()!=x->size())
        y_out->resize(x->size());
    for(int i=0;i<x->size();i++)
        (*y_out)[i]=-2*(x->at(i)-x0)*exp(-(x->at(i)-x0)*(x->at(i)-x0)/w/w)/w/w;
}

void normalize(QVector<double> *x)
{
    double m=max(x);
    for(int i=0;i<x->size();i++)
        (*x)[i]/=m;
}

void dummyFill(QVector<double> *input, QVector<double> *dummy)
{
    if(dummy->size()!=2*input->size())
        dummy->resize(2*input->size());
    for(int i=0;i<dummy->size();i++)
    {
        if(i%2==0)
            (*dummy)[i]=(*input)[i/2];
        else
            (*dummy)[i]=0.0;
    }
}

void exp_deriv(QVector<double> *x, double x0, double dx, QVector<double> *y)
{
    if(y->size()!=x->size())
        y->resize(x->size());
    for(int i=0;i<x->size();i++)
    {
        if((*x)[i]!=x0)
            (*y)[i]=-exp(-fabs((*x)[i]-x0)/dx)*((*x)[i]-x0)/fabs((*x)[i]-x0)/dx;
        else
            (*y)[i]=0;
    }
}

void MeanAndSTD(QVector<double> *Input, double *Mean, double *STD)
{
    int i;
    (*Mean)=(*STD)=0.0;
    for (i=0;i<Input->size();i++)
        (*Mean)+=(*Input)[i];
    (*Mean)/=double(Input->size());
    for (i=0;i<Input->size();i++)
        (*STD)+=(((*Input)[i]-(*Mean))*((*Input)[i]-(*Mean)));
    (*STD)=sqrt((*STD)/double(Input->size()));
}

void get_dispersion_rise_Raman(const SpectrumData *mainData, QVector < QVector < double > > *PPSpectra, double t_start, int every_nth, QVector < double > *x, QVector < double > *y)
{
    int stopi,i,j;
    double minval,maxval,k,thalf;

    if (!x->isEmpty()) x->clear();
    if (!y->isEmpty()) y->clear();

    locate(&(mainData->ProbeTime),t_start,&stopi);
    for(i=0;i<mainData->Wavelength.size();i+=every_nth)
    {
        minval=maxval=(*PPSpectra)[0][0];
        for(j=0;j<=stopi;j++)
        {
            if ((*PPSpectra)[j][i]>=maxval) maxval=(*PPSpectra)[j][i];
            if ((*PPSpectra)[j][i]<=minval) minval=(*PPSpectra)[j][i];
        }
        if(fabs(maxval)>=fabs(minval))
        {
            for(j=0;j<=stopi;j++)
            {
                if ((*PPSpectra)[j][i]>=(maxval*0.5))
                {
                    if(j==0) j++;
                    k=((*PPSpectra)[j][i]-(*PPSpectra)[j-1][i])/(mainData->ProbeTime[j]-mainData->ProbeTime[j-1]);
                    thalf=(maxval*0.5-(*PPSpectra)[j][i])/k+mainData->ProbeTime[j];
                    break;
                }
            }
        }
        else
        {
            for(j=0;j<=stopi;j++)
            {
                if ((*PPSpectra)[j][i]<=(minval*0.5))
                {
                    if(j==0) j++;
                    k=((*PPSpectra)[j][i]-(*PPSpectra)[j-1][i])/(mainData->ProbeTime[j]-mainData->ProbeTime[j-1]);
                    thalf=(minval*0.5-(*PPSpectra)[j][i])/k+mainData->ProbeTime[j];
                    break;
                }
            }
        }
        x->push_back(mainData->Wavelength[i]);
        y->push_back(thalf);
    }
}

inline bool InfCheck(double inp)
{
   volatile double temp = inp;
   if ((temp == inp) && ((temp - inp) != 0.0))
      return true;
   else return false;
}

void readXYFile(QString fileIn, QVector < double > *x0, QVector <double> *y0, bool norm)
{
    QFile file(fileIn);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream fileStream(&file), strStream;
    QString str;
    double number;
    if (!x0->isEmpty()) x0->clear();
    if (!y0->isEmpty()) y0->clear();
    for (str=fileStream.readLine(); !str.isNull(); str = fileStream.readLine())
    {
         strStream.setString(&str);
         strStream >> number;
         x0->push_back(number);
         strStream >> number;
         y0->push_back(number);
         strStream.flush();
    }
    if(norm)
    {
        double mmx=max(y0);
        for(int i=0;i<y0->size();i++)
            (*y0)[i]/=mmx;
    }
    fileStream.flush();
    file.close();
    shell_sort_TwoArrays(x0,y0);
}
