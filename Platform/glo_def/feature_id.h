
/***

History:
[2016-05-21 Ted]: Create

*/

#ifndef _FEATURE_ID_H
#define _FEATURE_ID_H

#ifdef __cplusplus
 extern "C" {
#endif 


#include "cmsis_os.h"



/** it's better to keep string 6 character */  

#define SystemStartup   "System"
#define FeatureCCS      "CCSDmn"
#define FeatureThread   "CCSTrd"
#define FeatureTag      "CCSTag"
#define FeatureLog      "CCSLog"
#define FeatureMem      "CCSMem"

#define FeatureSense    "SenseF"
#define FeatureOlcd     "OlcdFe"
#define FeatureTsc3200  "Tsc320"


#ifdef __cplusplus
}
#endif

#endif // _FEATURE_ID_H

// end of file


