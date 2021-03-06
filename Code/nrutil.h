#ifndef NRUTIL_H
#define NRUTIL_H

#include <math.h>

template<class T>
inline T SQR(const T a) {return a*a;}

template<class T>
inline const T &MAX(const T &a, const T &b)
        {return b > a ? (b) : (a);}

template<class T>
inline const T &MIN(const T &a, const T &b)
        {return b < a ? (b) : (a);}

template<class T>
inline T SIGN(const T &a, const T &b)
    {return b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a);}

template<class T>
inline void SWAP(T &a, T &b)
    {T dum=a; a=b; b=dum;}

#endif // NRUTIL_H
