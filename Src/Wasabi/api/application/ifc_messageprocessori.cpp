#include "ifc_messageprocessori.h"

#define CBCLASS ifc_messageprocessorI
START_DISPATCH;
CB(IFC_MESSAGEPROCESSOR_PROCESS_MESSAGE, ProcessMessage)
END_DISPATCH;
#undef CBCLASS
