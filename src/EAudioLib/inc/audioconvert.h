#ifndef __AUDIOCONVERT_H__
#define __AUDIOCONVERT_H__

#include<e32std.h>

class StereoToMonoConverter 
    {
    public:
        TInt static Convert(const TUint16* const aIn, TUint16* const aOut, TInt aCount);
    };

class BitRateConverter
    {
    public:
        TInt static Convert(const TUint16* const aIn, TUint16* const aOut, TInt aSource, TInt aTarget, TInt aCount);
        TInt static Interpolate(const TUint16* const aIn, TUint16* const aOut, TInt aSource, TInt aTarget, TInt aCount);
        TInt static Extrapolate(const TUint16* const aIn, TUint16* const aOut, TInt aSource, TInt aTarget, TInt aCount);
    };

#endif