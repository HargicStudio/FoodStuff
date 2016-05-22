
/***

History:
[2016-05-22 Ted]: Create

*/

#ifndef _AATAG_H
#define _AATAG_H

#ifdef __cplusplus
 extern "C" {
#endif 



u8 AaTagCEInit();
u8 AaTagCreateDeamon();
u8 AaTagCreate(char* name, u32 value);
u8 AaTagDelete(char* name);
u8 AaTagRegister(char* name, void(*function)(u32));
u8 AaTagSetValue(char* name, u32 value);




#ifdef __cplusplus
}
#endif

#endif // _AATAG_H

// end of file
