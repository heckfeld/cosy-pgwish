#include <tcl.h>

void Bin_DStringAppend(Tcl_DString *s, void *data, int len);
void Bin_DStringAppendElement(Tcl_DString *s, void *data, int len);
void Bin_SetResult(Tcl_Interp *interp, char *data, int len);
int Bin_EncodedLength(char *src, int len);
void Bin_EncodeData(char *dst, char *src, int len);
int Bin_DecodeData(char *dst, char *src, int len);	    
    
    
