#-------------------------------------------------
#
# Project created by QtCreator 2014-06-15T23:00:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = ProcessRamanFile
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    helpful_functions.cpp \
    comparetraces.cpp \
    qcustomplot.cpp \
    fourier1.cpp \
    svdfit.cpp \
    ludcmp.cpp \
    savgol.cpp \
    interp_1d.cpp \
    holmiumcomparison.cpp \
    wnscaledialog.cpp \
    fsrspreviewdialog.cpp \
    removedialog.cpp \
    fitmrq.cpp \
    customplot2d.cpp \
    fsrspreviewandfit.cpp \
    gaussfitthread.cpp \
    fitpreview.cpp \
    rescaleaxesdialog.cpp \
    gaussianweightdialog.cpp \
    nanreportdialog.cpp \
    spectrumdata.cpp \
    qcpsemilogaxisrect.cpp \
    qcpsemilogaxis.cpp \
    qsolventbox.cpp \
    dispersionsuite.cpp \
    dispersionplot.cpp

HEADERS  += mainwindow.h \
    read_pp_stuff.h \
    helpful_functions.h \
    nrutil.h \
    comparetraces.h \
    qcustomplot.h \
    fourier1.h \
    svdfit.h \
    savgol.h \
    ludcmp.h \
    interp_1d.h \
    holmiumcomparison.h \
    wnscaledialog.h \
    fsrspreviewdialog.h \
    removedialog.h \
    fitmrq.h \
    customplot2d.h \
    fsrspreviewandfit.h \
    gaussfitthread.h \
    fitpreview.h \
    rescaleaxesdialog.h \
    gaussianweightdialog.h \
    nanreportdialog.h \
    spectrumdata.h \
    qcpsemilogaxisrect.h \
    qcpsemilogaxis.h \
    qsolventbox.h \
    dispersionsuite.h \
    dispersionplot.h

FORMS    += mainwindow.ui \
    comparetraces.ui \
    holmiumcomparison.ui \
    wnscaledialog.ui \
    fsrspreviewdialog.ui \
    removedialog.ui \
    fsrspreviewandfit.ui \
    fitpreview.ui \
    rescaleaxesdialog.ui \
    gaussianweightdialog.ui \
    nanreportdialog.ui \
    dispersionsuite.ui

RC_FILE = my.rc

RESOURCES += \
    CalibrationSpectra.qrc
