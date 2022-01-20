#ifndef __LINEREADER__
#define __LINEREADER__

#include<f32file.h>

class CLineReader;

typedef TBool (*TReadFunc)(CLineReader&);


class CLineReader : public CBase
    {
    public:
        IMPORT_C static CLineReader* NewLC(RFs& aFs, const TDesC& aFileName);
        IMPORT_C static CLineReader* NewL(RFs& aFs, const TDesC& aFileName);
        IMPORT_C TBool NextL();
        IMPORT_C TPtrC Current() const;
        IMPORT_C ~CLineReader();
    private:
        static TBool ReadUTF16LEL(CLineReader& aThis);
        static TBool ReadAsciiL(CLineReader& aThis);
    private:
        TReadFunc ReaderL;
        HBufC* iBuffer;
        RFile  iFile;
        TInt iCursor;
        TInt iSize;
    };
    
#endif