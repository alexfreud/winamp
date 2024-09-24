#if 0 //JF 10.30.00 
//  The authors have released ID3Lib as Public Domain(PD) and claim no copyright,
//  patent or other intellectual property protection in this work.  This means that
//  it may be modified, redistributed and used in commercial and non-commercial
//  software and hardware without restrictions.  ID3Lib is distributed on an "AS IS"
//  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
//
//  The ID3Lib authors encourage improvements and optimisations to be sent to the
//  ID3Lib coordinator, currently Dirk Mahoney(dirk@id3.org).  Approved
//  submissions may be altered, and will be included and released under these terms.
//
//  Mon Nov 23 18:34:01 1998


#include <string.h>
#include "id3_error.h"


/*static char *ID3_ErrorDescs[] =
    {
        "out of memory",
        "no source/dest data specified",
        "no buffer specified",
        "invalid frame id",
        "field not found",
        "unknown field type",
        "tag is already attached to a file",
        "invalid tag version",
        "file not found",
        "error in zlib compression library"
    };*/


ID3_Error::ID3_Error(ID3_Err code, char *file, luint line)
{
	error = code;
	errLine = line;
	lstrcpyn(errFile, file, 256);
}


ID3_Err ID3_Error::GetErrorID(void)
{
	return error;
}


/*char *ID3_Error::GetErrorDesc(void)
{
	return ID3_ErrorDescs[error];
}*/


char *ID3_Error::GetErrorFile(void)
{
	return errFile;
}


luint ID3_Error::GetErrorLine(void)
{
	return errLine;
}



#endif
