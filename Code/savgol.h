#ifndef SAVGOL_H
#define SAVGOL_H

#include "ludcmp.h"

void savgol(QVector < double > &c, const int np, const int nl, const int nr, const int ld, const int m);
QVector < double > savgolfilter(QVector < double > *Input, const int nl, const int nr, const int ld, const int m);

#endif // SAVGOL_H
