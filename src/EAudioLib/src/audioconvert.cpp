#include"audioconvert.h"

TInt StereoToMonoConverter:: Convert(const TUint16* const aIn, TUint16* const aOut, TInt aCount)
    {
    const TInt count = aCount >> 2;
    for(TInt i = 0; i < count; i++)
        {
         const TInt j = i << 1;
         const TUint32 data1  = aIn[j];
         const TUint32 data2  = aIn[j + 1];  
         aOut[i] = (TUint16) ((data1 + data2) >> 1); 
        }
    return count << 1;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

TInt BitRateConverter::Convert(const TUint16* const aIn, TUint16* const aOut, TInt aSource, TInt aTarget, TInt aCount)
    {
    if(aTarget < aSource)
        return Interpolate(aIn, aOut, aSource, aTarget, aCount);
    if(aTarget > aSource)
        return Extrapolate(aIn, aOut, aSource, aTarget, aCount);
    Mem::Copy(aOut, aIn, aCount);
    return aCount;
    }
/*****************************************************************************************************
Linear interpolation
http://delivery.acm.org/10.1145/10000/3976/p1-field.pdf?key1=3976&key2=8477427011&coll=GUIDE&dl=GUIDE&CFID=37194872&CFTOKEN=8173716
 
n values from [a,b]

    int i, a, b, n
    int x, dx
1   x <- ((((a < 1) + 1) < Z) - 1) >> 1
2   dx <- (((b - a) << (Z + 1)) + n)div(n << 1)
3   for
        i=0 to n
    do
3.1 output(i, x >> Z)
3.2 x <- x + dx
    od
********************************************************************************************************/


const TInt KFix(18); //up to 16k samples

TInt BitRateConverter::Interpolate(const TUint16* const aIn, TUint16* const aOut, TInt aSource, TInt aTarget, TInt aCount)
    {
    const TUint* const in  = (const TUint* const) aIn; //streo samples, thus both channels are copied at once
    TUint* const out = (TUint* const) aOut;
    const TInt sourcecount = aCount >> 2; // / sizeof(uint32);
    TInt bytecount = (aTarget * aCount) / aSource;
    bytecount += (4 - bytecount & 0x3); //just for alignment
    const TInt targetcount = bytecount >> 2; // / sizeof(uint32);
    //here starts the alg
    TInt x = ((1 << KFix) - 1) >> 1;
    const TInt dx = ((sourcecount << (KFix + 1)) + targetcount) / (targetcount << 1);
    for(TInt i = 0; i < targetcount; i++)
        {
        out[i] = in[x >> KFix];
        x += dx;
        }
    return bytecount;
    }

TInt BitRateConverter::Extrapolate(const TUint16* const aIn, TUint16* const aOut, TInt aSource, TInt aTarget, TInt aCount)
    {
    const TUint* const in  = (const TUint* const) aIn; //stereo samples, thus both channels are copied at once
    TUint* const  out = (TUint* const) aOut;
    const TInt sourcecount = aCount >> 2; // / sizeof(uint32);
    TInt bytecount = (aTarget * aCount) / aSource;
    bytecount += (4 - bytecount & 0x3); //just for alignment
    const TInt targetcount = bytecount >> 2;

    TInt last = 0;
    TInt x = ((1 << KFix) - 1) >> 1;
    const TInt dx = ((targetcount << (KFix + 1)) + sourcecount) / (sourcecount << 1);
    TInt i;
    for(i = 0; i < sourcecount; i++)
        {
        const TInt newpos = x >> KFix;
        for(TInt j = last; j < newpos; j++)
            out[j] = in[i - 1]; //fill missing
        last = newpos;
        x += dx;
        }
    for(TInt j = last; j < targetcount; j++)
        out[j] = in[i - 1]; //fill missing
    return bytecount;
    }
