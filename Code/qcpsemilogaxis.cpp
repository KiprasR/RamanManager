#include "qcpsemilogaxis.h"

QCPSemiLogAxis::QCPSemiLogAxis(QCPAxisRect *parent, AxisType type) :
    QCPAxis(parent,type), mLogStart(1.0), mLogSubTicks(false)
{
}


void QCPSemiLogAxis::setLogStart(double logStart)
{
    mLogStart=logStart;
}

void QCPSemiLogAxis::setLogSubTicks(bool on)
{
    if (mLogSubTicks != on)
    {
        mLogSubTicks = on;
        mCachedMarginValid = false;
    }
}

void QCPSemiLogAxis::setupTickVectors()
{
    if (!mParentPlot) return;
    if ((!mTicks && !mTickLabels && !mGrid->visible()) || mRange.size() <= 0) return;

    // fill tick vectors, either by auto generating or by notifying user to fill the vectors himself
    if (mAutoTicks)
    {
      generateAutoTicks();
    } else
    {
      emit ticksRequest();
    }

    visibleTickBounds(mLowestVisibleTick, mHighestVisibleTick);
    if (mTickVector.isEmpty())
    {
      mSubTickVector.clear();
      return;
    }

    // generate subticks between ticks:
    if (!mLogSubTicks)
    {
        mSubTickVector.resize((mTickVector.size()-1)*mSubTickCount);
        if (mSubTickCount > 0)
        {
          double subTickStep = 0;
          double subTickPosition = 0;
          int subTickIndex = 0;
          bool done = false;
          int lowTick = mLowestVisibleTick > 0 ? mLowestVisibleTick-1 : mLowestVisibleTick;
          int highTick = mHighestVisibleTick < mTickVector.size()-1 ? mHighestVisibleTick+1 : mHighestVisibleTick;
          for (int i=lowTick+1; i<=highTick; ++i)
          {
            subTickStep = (mTickVector.at(i)-mTickVector.at(i-1))/(double)(mSubTickCount+1);
            for (int k=1; k<=mSubTickCount; ++k)
            {
              subTickPosition = mTickVector.at(i-1) + k*subTickStep;
              if (subTickPosition < mRange.lower)
                continue;
              if (subTickPosition > mRange.upper)
              {
                done = true;
                break;
              }
              mSubTickVector[subTickIndex] = subTickPosition;
              subTickIndex++;
            }
            if (done) break;
          }
          mSubTickVector.resize(subTickIndex);
        }
    }
    else
    {
        mSubTickVector.clear();
        if (mSubTickCount > 0)
        {
          double subTickStep = 0;
          double subTickPosition = 0;
          bool done = false;
          int lowTick = mLowestVisibleTick > 0 ? mLowestVisibleTick-1 : mLowestVisibleTick;
          int highTick = mHighestVisibleTick < mTickVector.size()-1 ? mHighestVisibleTick+1 : mHighestVisibleTick;
          for (int i=lowTick+1; i<=highTick; ++i)
          {
            if (mTickVector.at(i-1)<mLogStart)
            {
                subTickStep = (mTickVector.at(i)-mTickVector.at(i-1))/(double)(mSubTickCount+1);
                for (int k=1; k<=mSubTickCount; ++k)
                {
                  subTickPosition = mTickVector.at(i-1) + k*subTickStep;
                  if (subTickPosition < mRange.lower)
                    continue;
                  if (subTickPosition > mRange.upper)
                  {
                    done = true;
                    break;
                  }
                  mSubTickVector.append(subTickPosition);
                }
                if (done) break;
            }
            else
            {
                subTickStep = mLogStart*pow(10.0,(mTickVector.at(i-1)-mLogStart)/mLogStart);
                for (int k=2; k<=9; k++)
                {
                  subTickPosition = mLogStart+mLogStart*log10(k*subTickStep/mLogStart);
                  if (subTickPosition < mRange.lower)
                    continue;
                  if (subTickPosition > mRange.upper)
                  {
                    done = true;
                    break;
                  }
                  mSubTickVector.append(subTickPosition);
                }
                if (done) break;
            }
          }
        }
    }

    // generate tick labels according to tick positions:
    if (mAutoTickLabels)
    {
      int vecsize = mTickVector.size();
      mTickVectorLabels.resize(vecsize);
      if (mTickLabelType == ltNumber)
      {
        for (int i=mLowestVisibleTick; i<=mHighestVisibleTick; ++i)
          mTickVectorLabels[i] = mParentPlot->locale().toString(mTickVector.at(i), mNumberFormatChar.toLatin1(), mNumberPrecision);
      } else if (mTickLabelType == ltDateTime)
      {
        for (int i=mLowestVisibleTick; i<=mHighestVisibleTick; ++i)
        {
  #if QT_VERSION < QT_VERSION_CHECK(4, 7, 0) // use fromMSecsSinceEpoch function if available, to gain sub-second accuracy on tick labels (e.g. for format "hh:mm:ss:zzz")
          mTickVectorLabels[i] = mParentPlot->locale().toString(QDateTime::fromTime_t(mTickVector.at(i)).toTimeSpec(mDateTimeSpec), mDateTimeFormat);
  #else
          mTickVectorLabels[i] = mParentPlot->locale().toString(QDateTime::fromMSecsSinceEpoch(mTickVector.at(i)*1000).toTimeSpec(mDateTimeSpec), mDateTimeFormat);
  #endif
        }
      }
    } else // mAutoTickLabels == false
    {
      if (mAutoTicks) // ticks generated automatically, but not ticklabels, so emit ticksRequest here for labels
      {
        emit ticksRequest();
      }
      // make sure provided tick label vector has correct (minimal) length:
      if (mTickVectorLabels.size() < mTickVector.size())
        mTickVectorLabels.resize(mTickVector.size());
    }
}
