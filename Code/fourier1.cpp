#include "fourier1.h"

void four1(double *data, const int n, const int isign) {
    int nn,mmax,m,j,istep,i;
    double wtemp,wr,wpr,wpi,wi,theta,tempr,tempi;
    nn = n << 1;
    j = 1;
    for (i=1;i<nn;i+=2) {
        if (j > i) {
            SWAP(data[j-1],data[i-1]);
            SWAP(data[j],data[i]);
        }
        m=n;
        while (m >= 2 && j > m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    }
    mmax=2;
    while (nn > mmax) {
        istep=mmax << 1;
        theta=isign*(6.28318530717959/mmax);
        wtemp=sin(0.5*theta);
        wpr = -2.0*wtemp*wtemp;
        wpi=sin(theta);
        wr=1.0;
        wi=0.0;
        for (m=1;m<mmax;m+=2) {
            for (i=m;i<=nn;i+=istep) {
                j=i+mmax;
                tempr=wr*data[j-1]-wi*data[j];
                tempi=wr*data[j]+wi*data[j-1];
                data[j-1]=data[i-1]-tempr;
                data[j]=data[i]-tempi;
                data[i-1] += tempr;
                data[i] += tempi;
            }
            wr=(wtemp=wr)*wpr-wi*wpi+wr;
            wi=wi*wpr+wtemp*wpi+wi;
        }
        mmax=istep;
    }
}

void four1(QVector<double> &data, const int isign) {
    four1(&data[0],data.size()/2,isign);
}

void four1(QVector<double> *data, const int isign) {
    four1(&(*data)[0],data->size()/2,isign);
}

void realft(QVector<double> &data, const int isign) {
    int i,i1,i2,i3,i4,n=data.size();
    double c1=0.5,c2,h1r,h1i,h2r,h2i,wr,wi,wpr,wpi,wtemp;
    double theta=3.141592653589793238/double(n>>1);
    if (isign == 1) {
        c2 = -0.5;
        four1(data,1);
    } else {
        c2=0.5;
        theta = -theta;
    }
    wtemp=sin(0.5*theta);
    wpr = -2.0*wtemp*wtemp;
    wpi=sin(theta);
    wr=1.0+wpr;
    wi=wpi;
    for (i=1;i<(n>>2);i++) {
        i2=1+(i1=i+i);
        i4=1+(i3=n-i1);
        h1r=c1*(data[i1]+data[i3]);
        h1i=c1*(data[i2]-data[i4]);
        h2r= -c2*(data[i2]+data[i4]);
        h2i=c2*(data[i1]-data[i3]);
        data[i1]=h1r+wr*h2r-wi*h2i;
        data[i2]=h1i+wr*h2i+wi*h2r;
        data[i3]=h1r-wr*h2r+wi*h2i;
        data[i4]= -h1i+wr*h2i+wi*h2r;
        wr=(wtemp=wr)*wpr-wi*wpi+wr;
        wi=wi*wpr+wtemp*wpi+wi;
    }
    if (isign == 1) {
        data[0] = (h1r=data[0])+data[1];
        data[1] = h1r-data[1];
    } else {
        data[0]=c1*((h1r=data[0])+data[1]);
        data[1]=c1*(h1r-data[1]);
        four1(data,-1);
    }
}
void sinft(QVector<double> &y) {
    int j,n=y.size();
    double sum,y1,y2,theta,wi=0.0,wr=1.0,wpi,wpr,wtemp;
    theta=3.141592653589793238/double(n);
    wtemp=sin(0.5*theta);
    wpr= -2.0*wtemp*wtemp;
    wpi=sin(theta);
    y[0]=0.0;
    for (j=1;j<(n>>1)+1;j++) {
        wr=(wtemp=wr)*wpr-wi*wpi+wr;
        wi=wi*wpr+wtemp*wpi+wi;
        y1=wi*(y[j]+y[n-j]);
        y2=0.5*(y[j]-y[n-j]);
        y[j]=y1+y2;
        y[n-j]=y1-y2;
    }
    realft(y,1);
    y[0]*=0.5;
    sum=y[1]=0.0;
    for (j=0;j<n-1;j+=2) {
        sum += y[j];
        y[j]=y[j+1];
        y[j+1]=sum;
    }
}

void cosft1(QVector<double> &y) {
    const double PI=3.141592653589793238;
    int j,n=y.size()-1;
    double sum,y1,y2,theta,wi=0.0,wpi,wpr,wr=1.0,wtemp;
    QVector<double> yy(n);
    theta=PI/n;
    wtemp=sin(0.5*theta);
    wpr = -2.0*wtemp*wtemp;
    wpi=sin(theta);
    sum=0.5*(y[0]-y[n]);
    yy[0]=0.5*(y[0]+y[n]);
    for (j=1;j<n/2;j++) {
        wr=(wtemp=wr)*wpr-wi*wpi+wr;
        wi=wi*wpr+wtemp*wpi+wi;
        y1=0.5*(y[j]+y[n-j]);
        y2=(y[j]-y[n-j]);
        yy[j]=y1-wi*y2;
        yy[n-j]=y1+wi*y2;
        sum += wr*y2;
    }
    yy[n/2]=y[n/2];
    realft(yy,1);
    for (j=0;j<n;j++) y[j]=yy[j];
    y[n]=y[1];
    y[1]=sum;
    for (j=3;j<n;j+=2) {
        sum += y[j];
        y[j]=sum;
    }
}
void cosft2(QVector<double> &y, const int isign) {
    const double PI=3.141592653589793238;
    int i,n=y.size();
    double sum,sum1,y1,y2,ytemp,theta,wi=0.0,wi1,wpi,wpr,wr=1.0,wr1,wtemp;
    theta=0.5*PI/n;
    wr1=cos(theta);
    wi1=sin(theta);
    wpr = -2.0*wi1*wi1;
    wpi=sin(2.0*theta);
    if (isign == 1) {
        for (i=0;i<n/2;i++) {
            y1=0.5*(y[i]+y[n-1-i]);
            y2=wi1*(y[i]-y[n-1-i]);
            y[i]=y1+y2;
            y[n-1-i]=y1-y2;
            wr1=(wtemp=wr1)*wpr-wi1*wpi+wr1;
            wi1=wi1*wpr+wtemp*wpi+wi1;
        }
        realft(y,1);
        for (i=2;i<n;i+=2) {
            wr=(wtemp=wr)*wpr-wi*wpi+wr;
            wi=wi*wpr+wtemp*wpi+wi;
            y1=y[i]*wr-y[i+1]*wi;
            y2=y[i+1]*wr+y[i]*wi;
            y[i]=y1;
            y[i+1]=y2;
        }
        sum=0.5*y[1];
        for (i=n-1;i>0;i-=2) {
            sum1=sum;
            sum += y[i];
            y[i]=sum1;
        }
    } else if (isign == -1) {
        ytemp=y[n-1];
        for (i=n-1;i>2;i-=2)
            y[i]=y[i-2]-y[i];
        y[1]=2.0*ytemp;
        for (i=2;i<n;i+=2) {
            wr=(wtemp=wr)*wpr-wi*wpi+wr;
            wi=wi*wpr+wtemp*wpi+wi;
            y1=y[i]*wr+y[i+1]*wi;
            y2=y[i+1]*wr-y[i]*wi;
            y[i]=y1;
            y[i+1]=y2;
        }
        realft(y,-1);
        for (i=0;i<n/2;i++) {
            y1=y[i]+y[n-1-i];
            y2=(0.5/wi1)*(y[i]-y[n-1-i]);
            y[i]=0.5*(y1+y2);
            y[n-1-i]=0.5*(y1-y2);
            wr1=(wtemp=wr1)*wpr-wi1*wpi+wr1;
            wi1=wi1*wpr+wtemp*wpi+wi1;
        }
    }
}

void fftshift(QVector<double> *data)
{
    int i, j=data->size()/2;
    double temp;
    for (i=0;i<j;i++)
        //SWAP(data[i],data[i+j]);
    {
        temp=(*data)[i+j];
        (*data)[i+j]=(*data)[i];
        (*data)[i]=temp;
    }
}

void fft_wrapper(const QVector<double> *data,QVector<double> *complex_spec)
{
    if (complex_spec->size()!=(2*data->size()))
        complex_spec->resize(2*data->size());
    for (int i=0;i<data->size();i++)
    {
        (*complex_spec)[2*i]   = (*data)[i];
        (*complex_spec)[2*i+1] = 0.0;
    }
    four1(complex_spec,1);
}

void ifft_wrapper(const QVector<double> *complex_spec,QVector<double> *complex_data)
{
    (*complex_data)=(*complex_spec);
    four1(complex_data,-1);
}

void ifft_wrapper(const QVector<double> *complex_spec, QVector<double> *data_r, QVector<double> *data_i, bool shift)
{
    double temp;
    QVector<double> dummy;
    int i,j=complex_spec->size()/2;

    ifft_wrapper(complex_spec,&dummy);
    if(data_r->size()!=j) data_r->resize(j);
    if(data_i->size()!=j) data_i->resize(j);

    for (i=0;i<j;i++)
    {
        (*data_r)[i] = dummy[2*i]   *(2.0/complex_spec->size());
        (*data_i)[i] = dummy[2*i+1] *(2.0/complex_spec->size());
    }

    if (shift)
    {
        j/=2;
        for (i=0;i<j;i++)
        {
            /* Real: */
            temp=(*data_r)[i+j];
            (*data_r)[i+j]=(*data_r)[i];
            (*data_r)[i]=temp;
            /* Imag: */
            temp=(*data_i)[i+j];
            (*data_i)[i+j]=(*data_i)[i];
            (*data_i)[i]=temp;
        }
    }
}

void fft_gaussian_convolution(QVector<double> *x_in, QVector<double> *y_in, double g_fwhm,
                              QVector<double> *x_out,QVector<double> *y_out,bool normalize)
{
    int i, n_pow;
    double d_nu, f_s, nu_0, kern;
    QVector < double > new_v, new_amp,
                       freq,  solv_spec,
                       convo,convo_r,convo_i;

    /* Find the nearest 2^n: */
    n_pow = 1 << int(ceil(log(double(x_in->size())*1.15)/log(2)));

    /* Resample the data: */
    d_nu  = fabs(x_in->last()-x_in->first())/(x_in->size()-1);
    f_s   = 1/d_nu;
    nu_0  = (x_in->last()+x_in->first())/2.00-d_nu*n_pow/2.0;

    /* Resize the vectors: */
    convo.resize(2*n_pow);
    new_v.resize(n_pow);
    freq.resize(n_pow);

    /* Create a linear nu space and a fftshifted inverse nu (i.e., frequency): */
    for (i=0;i<n_pow;i++)
    {
        new_v[i] = nu_0+i*d_nu;
        freq[i]  = -f_s/2.0+i*f_s/n_pow;
    }
    fftshift(&freq);

    /* Interpolate the original data to a linear expanded scale: */
    interpolate_zero_pad(x_in,y_in,&new_v,&new_amp);

    /* Calculate the FFT: */
    fft_wrapper(&new_amp,&solv_spec);

    /* Convolve with the fftshifted kernel: */
    for (i=0;i<n_pow;i++)
    {
        kern = g_fwhm*MULT1*exp(-MULT2*SQR(freq[i]*g_fwhm));
        convo[2*i]   = solv_spec[2*i]*kern;
        convo[2*i+1] = solv_spec[2*i+1]*kern;
    }

    /* Return to v domain, re-interpolate. */
    ifft_wrapper(&convo,&convo_r,&convo_i);
    interpolate(&new_v,&convo_r,x_out,y_out);

    /* Normalize */
    if (normalize)
    {
        double mmx=max(y_out);
        for (i=0;i<y_out->size();i++) (*y_out)[i]/=mmx;
    }
}
