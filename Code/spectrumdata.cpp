#include "spectrumdata.h"

SpectrumData::SpectrumData()
{
    Type=0;
}

SpectrumData::~SpectrumData()
{
    ClearSpectra();
}

void SpectrumData::ClearSpectra()
{
    Type=0;

    Wavelength.clear();
    ProbeTime.clear();

    WL00.clear();
    WL01.clear();
    WL10.clear();
    WL11.clear();

    AUX01.clear();
    AUX11.clear();

    FSRS.clear();
}
