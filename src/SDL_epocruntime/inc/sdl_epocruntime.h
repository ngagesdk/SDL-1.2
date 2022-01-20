#ifndef __EPOCRUNTIME_H__
#define __EPOCRUNTIME_H__

/*
*All SOS versions are not compatible, this class
*containts variation software. There are different
*versions of this DLL in SIS file and options
*are used to select a right one when installing 
*/

#include<e32std.h>
#include<e32svr.h>


class Epoc_Runtime
 {
 public:
    IMPORT_C static void GetScreenInfo(TScreenInfoV01& screenInfo2);
 };

#endif