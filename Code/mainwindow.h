#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define REPLOT_PERC 0.015
#define BoolToBinary(x) (x ? 1 : 0)

#include <QDebug>
#include <QString>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <stdio.h>
#include <math.h>

#include "qcustomplot.h"
#include "spectrumdata.h"
#include "read_pp_stuff.h"
#include "helpful_functions.h"
#include "comparetraces.h"
#include "fourier1.h"
#include "svdfit.h"
#include "savgol.h"
#include "holmiumcomparison.h"
#include "wnscaledialog.h"
#include "interp_1d.h"
#include "fsrspreviewdialog.h"
#include "removedialog.h"
#include "fitmrq.h"
#include "fsrspreviewandfit.h"
#include "gaussianweightdialog.h"
#include "nanreportdialog.h"
#include "qsolventbox.h"
#include "dispersionsuite.h"

#define NanCheck(a) (a==a)

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_actionOpen_PDP_Dataset_triggered();
    void on_actionOpen_Raw_WL_File_triggered();
    void on_actionSave_Raw_WL_File_triggered();
    void on_actionOpen_PP_Dataset_triggered();
    void on_delaySlider_valueChanged(int value);
    void showPointToolTip(QMouseEvent *event);
    void clickedGraph(QMouseEvent* event);
    void on_clearButton_clicked();
    void on_actionSave_Difference_File_triggered();
    void on_polyBox_valueChanged(int arg1);
    void on_zeroBox_toggled(bool checked);
    void on_linButton_toggled(bool checked);
    void on_smoothBox_toggled(bool checked);
    void on_windowSpinBox_valueChanged(int arg1);
    void on_degSpinBox_valueChanged(int arg1);
    void on_subGSBox_toggled(bool checked);
    void on_actionInvert_Signals_triggered();
    void on_actionReverse_Wavenumber_triggered();
    void on_clearGSButton_clicked();
    void on_actionOpen_Common_PP_File_triggered();
    void on_actionCalibrate_wavenumber_triggered();
    void getScale(double k,double b,double v0);
    void ReceiveRemovableRegion(bool wl_b, double w0, double w1, bool pt_b, double p0, double p1);
    void on_actionScale_Wavenumber_triggered();
    void on_actionSave_Averaged_GS_Spectrum_triggered();
    void on_isoBox_valueChanged(int arg1);
    void on_actionCorrect_Dispersion_triggered();
    void on_actionPreview_FSRS_Spectrum_triggered();
    void on_isolateFSRSBox_toggled(bool checked);
    void on_isolateGSBox_toggled(bool checked);
    void on_splineFSRSBox_toggled(bool checked);
    void on_splineGSBox_toggled(bool checked);
    void on_actionRemove_Region_triggered();
    void on_actionPreview_GS_FSRS_Spectrum_triggered();
    void on_actionInvert_Probetime_triggered();
    void on_actionPreview_PP_Spectrum_triggered();
    void on_actionPreview_DP_Spectrum_triggered();
    void on_actionPreview_PDP_Spectrum_triggered();
    void on_actionSave_Configs_triggered();
    void on_actionLoad_Configs_triggered();
    void on_actionFit_FSRS_Spectrum_triggered();
    void on_actionAdd_Gaussian_Weights_triggered();
    void on_actionSave_On_Screen_Data_triggered();
    void on_zeroSpinBox_valueChanged(double arg1);
    void LoadReferenceSolvent(QString solvFile);
    void on_refBox_toggled(bool checked);
    void on_actionOpen_Temporally_Shifted_FSRS_triggered();
    void on_actionOpen_Spectrally_Shifted_FSRS_triggered();
    void on_convoBox_toggled(bool checked);
    void on_convoSpinBox_valueChanged(double arg1);
    void on_actionPreview_Difference_Spectrum_triggered();

private:
    Ui::MainWindow *ui;
    QSettings DirSettings;
    SpectrumData *Spectrum;
    QCPAxis *x1 ,*y1,
            *x2, *y2,
            *x3, *y3;
    QVector <int> zero_indexes;
    QVector <int> gs_zero_indexes;
    QVector <double> Solvent;
    QVector <double> GaussianMults;

    // The baseline-corrected spectra are stored here forconvenience:
    QVector <double> GroundState;
    QVector <double> AuxGroundState;
    QVector <double> ExcState;
    QVector <double> AuxExcState;
    int max_ind;

    inline int my_isinf(double x);
    void ReadPDPPoint(FILE* pFile, PDPTempData *Temp, QVector < QString > *NaNs, int npix, bool Reference);
    void ReadPPPoint(FILE* pFile, PPTempData *Temp, QVector < QString > *NaNs, int npix, bool Reference);
    void replot_some(bool rscFSRS=true, bool rscCFSRS=true, bool rscGS=true);
    void replot_gs(bool rscGS=true);
    void get_FSRS(bool plot, int delay, QVector<double> *FSRS, QVector<double> *AUX=NULL);
    void get_GS_FSRS(bool plot, int delay, QVector<double> *GS_FSRS, QVector<double> *AUX_GS_FSRS=NULL);
    void reverse_wavenumber();
    void InitializeGraphs();
    void PreviewSpectrum(QVector < QVector < double > > *spec, QVector<QVector<double> > *aux=NULL);

    // Experiments with shifting:
    void Open_PDP_Type_Spectrally_Shifted_File(QString fileName);
    void Open_PP_Type_Spectrally_Shifted_File(QString fileName);
    void ReadFSRSPoint(FILE* pFile, PDPTempData *Temp, QVector < QString > *NaNs, int npix);
    void ReadGSFSRSPoint(FILE* pFile, PDPTempData *Temp, QVector < QString > *NaNs, int npix);

    // Segmented functions:
    inline void get_FSRS_spectrum(int delay, QVector < double > *FSRS, QVector < double > *AUX);
    inline void get_zero_spectrum(QVector < double > *FSRS, QVector < double > *AUX);
    inline void get_poly(QVector < double > *FSRS, QVector < double > *c_FSRS,
                         bool plot=false, bool excited=true, bool AUX=false);
    inline void get_GS_FSRS_spectrum(int delay, QVector < double > *FSRS, QVector < double > *AUX);
};

#endif // MAINWINDOW_H
