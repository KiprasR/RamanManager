#include "gaussfitthread.h"

GaussFitThread::GaussFitThread(QObject *parent) :
    QThread(parent), Stop(false){
}

void GaussFitThread::AssignVariables(SpectrumData *mainData, QVector<QVector<double> > *mainFSRS, QVector<double> *coeffs, QVector<QVector<double> > *fitFSRS, QVector<double> *paramTablePTR, double toll)
{
    mData=mainData;
    mFSRS=mainFSRS;
    cfs=*coeffs;
    fFSRS=fitFSRS;
    params=paramTablePTR;
    TOL=toll;
}

void GaussFitThread::run()
{
    int i,j,k;
    double y;
    QVector < double > ssig, dummy, dummy_coeffs, high_dev, scaled_wl, dummy_coeffs_temp;

    for(i=0;i<cfs.size()/4;i++)
    {
        if(cfs[i*4+3]==1)
        {
            dummy_coeffs <<  cfs[i*4]
                         << (cfs[i*4+1]-mData->Wavelength.first())/(mData->Wavelength.last()-mData->Wavelength.first())
                         <<  cfs[i*4+2]/(mData->Wavelength.last()-mData->Wavelength.first());
        }
        else if(cfs[i*4+3]==0)
        {
            high_dev <<  cfs[i*4] << cfs[i*4+1] << cfs[i*4+2];
                     // << (cfs[i*4+1]-mData->Wavelength.first())/(mData->Wavelength.last()-mData->Wavelength.first())
                     // <<  cfs[i*4+2]/(mData->Wavelength.last()-mData->Wavelength.first());
        }
    }

    dummy.resize(dummy_coeffs.size());

    for (i=0;i<mData->Wavelength.size();i++)
        scaled_wl << (mData->Wavelength[i]-mData->Wavelength.first())/(mData->Wavelength.last()-mData->Wavelength.first());

    ssig.fill(1.0,mData->Wavelength.size());

    Stop=false;
    for(i=mData->ProbeTime.size()-1;i>=0;i--)
    {
        QVector < double > currentSpec=(*mFSRS)[i];
        if(!high_dev.isEmpty())
            for(j=0;j<mData->Wavelength.size();j++)
                for(k=0;k<high_dev.size()/3;k++)
                    if( (mData->Wavelength[j]>=(high_dev[k*3+1]-high_dev[k*3+2]/2)) && (mData->Wavelength[j]<=(high_dev[k*3+1]+high_dev[k*3+2]/2)) )
                       currentSpec[j]=0.0;

        QMutex mutex;
        mutex.lock();

        for (j=0;j<dummy_coeffs.size()/3;j++)
        {
            hunt(&scaled_wl,dummy_coeffs[j*3+1],&k);
            dummy_coeffs[j*3]=(*mFSRS)[i][k];
        }

        k=0;
        dummy_coeffs_temp=dummy_coeffs;
        bool checked=true;
        do {
            k++;

            Fitmrq mrq(scaled_wl,currentSpec,ssig,dummy_coeffs_temp,fgauss,TOL);
            mrq.fit();

            dummy_coeffs_temp=mrq.a;
            checked=CheckForINF(&dummy_coeffs_temp);

            if (checked)
            {
                emit SendInfo(QTime::currentTime().toString() + ": INF @ " + QString::number(mData->ProbeTime[i],'f',2).replace(".00","").replace(",00","") + " ps. Iteration " + QString::number(k) + ".");
                for (j=0;j<dummy_coeffs_temp.size()/3;j++)
                {
                    dummy_coeffs_temp[ 3*j ]=dummy_coeffs[ 3*j ]*(1.00+double(100-(rand()%200+1))/1000.0);
                    dummy_coeffs_temp[3*j+1]=dummy_coeffs[3*j+1]*(1.00+double(100-(rand()%200+1))/1000.0);
                    dummy_coeffs_temp[3*j+2]=dummy_coeffs[3*j+2]*(1.00+double(200-(rand()%400+1))/1000.0);
                }
            }
            if((this->Stop) || (k>100))
            {
                if(this->Stop) emit ThreadCancelled();
                dummy_coeffs_temp=dummy_coeffs;
                break;
            }
        } while(checked);

        for(j=0;j<mData->Wavelength.size();j++)
        {
            fgauss(scaled_wl[j],dummy_coeffs_temp,y,dummy);
            (*fFSRS)[i][j]=y;
        }

        for(j=0;j<dummy_coeffs_temp.size()/3;j++)
        {
            params->push_back(dummy_coeffs_temp[j*3]);
            params->push_back(dummy_coeffs_temp[j*3+1]*(mData->Wavelength.last()-mData->Wavelength.first())+mData->Wavelength.first());
            params->push_back(dummy_coeffs_temp[j*3+2]*(mData->Wavelength.last()-mData->Wavelength.first()));
        }

        if(this->Stop)
        {
            emit ThreadCancelled();
            break;
        }
        mutex.unlock();

        emit ProgressChanged(int(double(mData->ProbeTime.size()-i)/double(mData->ProbeTime.size())*100));
        emit SendInfo(QTime::currentTime().toString() + ": " + QString::number(mData->ProbeTime[i],'f',2).replace(".00","").replace(",00","") + " ps done.");
    }
    if(this->Stop)  params->clear();
    else emit SendParamsVector();
}

inline bool GaussFitThread::CheckForINF(QVector<double> *inp)
{
    for(int i=0;i<inp->size()/3;i++)
    {
        if (fabs((*inp)[i*3])>A0_LIM) return true;
        if (((*inp)[i*3+1]<-0.25) || (((*inp)[i*3+1])>1.25)) return true;     // Galima ideti globalu.
        //if ((fabs((*inp)[i*3+2])<V0_LIM) || ((*inp)[i*3+2]<=0.0)) return true;
        //if ((fabs((*inp)[i*3+2])<V0_LIM) || ((*inp)[i*3+2]>2.0) || ((*inp)[i*3+2]<=0.0)) return true;
    }
    return false;
}
