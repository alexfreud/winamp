//  The authors have released ID3Lib as Public Domain (PD) and claim no copyright,
//  patent or other intellectual property protection in this work.  This means that
//  it may be modified, redistributed and used in commercial and non-commercial
//  software and hardware without restrictions.  ID3Lib is distributed on an "AS IS"
//  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
//  
//  The ID3Lib authors encourage improvements and optimisations to be sent to the
//  ID3Lib coordinator, currently Dirk Mahoney (dirk@id3.org).  Approved
//  submissions may be altered, and will be included and released under these terms.
//  
//  Mon Nov 23 18:34:01 1998

// improved/optimized/whatever 10/30/00 JF
// improved/optimized/whatEVER jan-08-2006 benski

#include <stdlib.h>
#include "id3_field.h"
#include "id3_misc_support.h"
#include "../nu/AutoWide.h"
#include "../Plugins/Input/in_mp3/config.h" // TODO: cut


// the ::Set() function for ASCII

void ID3_Field::SetLocal(const char *newString)
{
	if	(newString)
	{
		Clear();
		AutoWide temp(newString);
		SetUnicode(temp);
		if (!(flags & ID3FF_ADJUSTENC))
			type = ID3FTY_ASCIISTRING;
	}
}

void ID3_Field::SetLatin(const char *newString)
{
	if	(newString)
	{
		Clear();
		AutoWide temp(newString, 28591); // ISO-8859-1 code page
		SetUnicode(temp);
		type = ID3FTY_ASCIISTRING;
	}

	return;
}

void ID3_Field::SetUTF8(const char *newString)
{
	if	(newString)
	{
		Clear();
		AutoWide temp(newString, CP_UTF8); 
		SetUnicode(temp);
	}

	return;
}

// the ::Get() function for ASCII

luint	ID3_Field::GetLocal(char *buffer, luint maxLength, luint itemNum)
{
	luint	bytesUsed	= 0;
	wchar_t	*temp;

	if (temp = (wchar_t*)calloc(maxLength, sizeof(wchar_t)))
	{
		if	(GetUnicode(temp, maxLength, itemNum))
		{
			bytesUsed = ID3_UnicodeToLocal(buffer, temp, maxLength);
		}

		free(temp);
	}
	else
		ID3_THROW (ID3E_NoMemory);

	return bytesUsed;
}


void ID3_Field::AddLocal(const char *newString)
{
	if	(newString)
	{
		AutoWide temp(newString);
		AddUnicode(temp);
	}

}

void ID3_Field::AddLatin(const char *newString)
{
	if	(newString)
	{
		AutoWide temp(newString, 28591);
		AddUnicode(temp);

		type = ID3FTY_ASCIISTRING;
	}
}

luint	ID3_Field::ParseASCIIString(uchar *buffer, luint posn, luint buffSize)
{
	luint	bytesUsed	= 0;

	if	(fixedLength != -1)
		bytesUsed = fixedLength;
	else
	{
		if	(flags & ID3FF_NULL)
			while	((posn + bytesUsed) < buffSize && buffer[posn + bytesUsed] != 0)
				bytesUsed++;
		else
			bytesUsed = buffSize - posn;
	}

  if (bytesUsed > 0xffff) // keep it sane, yo (64kb string max should be good)
  {
    hasChanged=false;
    return 0;
  }
	if	(bytesUsed)
	{
		char *temp = NULL;
		if (temp = (char*)calloc(bytesUsed + 1, sizeof(char)))
		{
			memcpy(temp, &buffer[posn], bytesUsed);
			temp[bytesUsed] = 0;

      if (config_read_mode == READ_LOCAL) // benski> I added a config option to deal with old tags
			  SetLocal(temp); 
      else
        SetLatin(temp);

			if (!(flags & ID3FF_ADJUSTENC))
				type = ID3FTY_ASCIISTRING;
			free(temp);
		}
		else
			ID3_THROW (ID3E_NoMemory);
	}

	if	(flags & ID3FF_NULL)
		bytesUsed++;

	hasChanged = false;

	return bytesUsed;
}

luint	ID3_Field::ParseUTF8String(uchar *buffer, luint posn, luint buffSize)
{
	luint	bytesUsed	= 0;

	if	(fixedLength != -1)
		bytesUsed = fixedLength;
	else
	{
		if	(flags & ID3FF_NULL)
			while	((posn + bytesUsed) < buffSize && buffer[posn + bytesUsed] != 0)
				bytesUsed++;
		else
			bytesUsed = buffSize - posn;
	}

  if (bytesUsed > 0xffff) // keep it sane, yo (64kb string max should be good)
  {
    hasChanged=false;
    return 0;
  }
	if	(bytesUsed)
	{
		char *temp = NULL;
		if (temp = (char*)calloc(bytesUsed + 1, sizeof(char)))
		{
			memcpy (temp, &buffer[posn], bytesUsed);
			temp[bytesUsed] = 0;

			SetUTF8(temp);

			free(temp);
		}
		else
			ID3_THROW (ID3E_NoMemory);
	}

	if	(flags & ID3FF_NULL)
		bytesUsed++;

	hasChanged = false;

	return bytesUsed;
}


luint ID3_Field::RenderLatinString(uchar *buffer)
{
	luint	bytesUsed	= 0;

	buffer[0] = 0;
	bytesUsed = BinSize();

	if	(data && size)
	{
	  luint	i;

    // benski> I added a config option for whether to use "broken" local system encoding mode
    if (config_write_mode == WRITE_LATIN)
      ID3_UnicodeToLatin( (char*)buffer, (const wchar_t *) data, bytesUsed, (int)bytesUsed);
    else
		  ID3_UnicodeToLocal( (char*)buffer, (const wchar_t *) data, bytesUsed, bytesUsed);

		for	(i = 0;i < bytesUsed; i++)
    {
			if (buffer[i] == 1)
			{
				char	sub	=	'/';

				if	(flags & ID3FF_NULLDIVIDE)
					sub = '\0';

				buffer[i] = sub;
			}
    }
	}

	if (bytesUsed == 1 && flags & ID3FF_NULL)
		buffer[0] = 0;

	hasChanged = false;

	return bytesUsed;
}


