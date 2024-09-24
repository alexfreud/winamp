#include "mp4common.h"

size_t utf16len(const uint16_t *str)
{
	size_t size=0;
	while (*str++) size++;
	return size;
}

void utf16swap(uint16_t *str)
{
	while (*str)
	{
		*str = htons(*str);
		str++;
	}	
}

bool MP4File::Get3GPMetadataString(const char *atom, uint16_t **value)
{
	char atomstring[60];
	snprintf(atomstring, 60, "moov.udta.%s.metadata", atom);
	const uint8_t *str = (const uint8_t *)this->GetStringProperty(atomstring);

	if (str)
	{
		bool reverse=false;
		bool utf16=false;
		if ((str[0] == 0xFE && str[1] == 0xFF))
		{
			reverse=true;
			utf16=true;
		}
		if ((str[0] == 0xFF && str[1] == 0xFE))
		{
			utf16=true;
		}

		if (utf16)
		{
			uint16_t *utf16 = (uint16_t *)str;
			size_t len = utf16len(utf16);
			*value = (uint16_t *)malloc(len*sizeof(uint16_t));
				if (!*value)
					return false;
			memcpy(*value, utf16+1, len*sizeof(uint16_t));
		
			if (reverse)
				utf16swap(*value);
		}
		else
		{
			int len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)str, -1, 0, 0);
			*value = (uint16_t *)malloc(len * sizeof(uint16_t));
				if (*value == NULL)
		{
			return false;
		}
			MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)str, -1, (LPWSTR)*value, len);
		}



		return true;
	}
	return false;
}


bool MP4File::Set3GPMetadataString(const char *atom, const uint16_t *value)
{
  char atomstring[60];
  MP4Atom *pMetaAtom;
  MP4StringProperty *pMetadataProperty = NULL;
	MP4Integer16Property *unknown = NULL;
  snprintf(atomstring, 60, "moov.udta.%s", atom);

  pMetaAtom = m_pRootAtom->FindAtomMP4(atomstring);
  
  if (!pMetaAtom)
    {
			(void)AddDescendantAtoms("moov", atomstring+5);

      //if (!CreateMetadataAtom(atom))
	//return false;
      
      pMetaAtom = m_pRootAtom->FindAtomMP4(atomstring);
      if (pMetaAtom == NULL) return false;
    }

	snprintf(atomstring, 60, "%s.language", atom);
		ASSERT(pMetaAtom->FindProperty(atomstring, 
				 (MP4Property**)&unknown));
  ASSERT(unknown);
	unknown->SetValue(0x15C7);


	snprintf(atomstring, 60, "%s.metadata", atom);
  ASSERT(pMetaAtom->FindProperty(atomstring,
				 (MP4Property**)&pMetadataProperty));
  ASSERT(pMetadataProperty);
  
	pMetadataProperty->SetUnicode(true);
	size_t lenWithBOM = utf16len(value) + 1;
	uint16_t *newVal = (uint16_t *)malloc((lenWithBOM+1) * sizeof(uint16_t));
	newVal[0]=0xFEFF;
	memcpy(newVal+1, value, lenWithBOM*sizeof(uint16_t));
  pMetadataProperty->SetValue((char *)newVal, 0);
	free(newVal);
  
  return true;
}

bool MP4File::Get3GPMetadataInteger(const char *atom, uint64_t *value)
{
	char atomstring[60];
	snprintf(atomstring, 60, "moov.udta.%s.metadata", atom);
	
	MP4Property* pProperty;
	u_int32_t index;

	FindIntegerProperty(atomstring, &pProperty, &index);
	if (pProperty)
	{
		*value = ((MP4IntegerProperty*)pProperty)->GetValue(index);
		return true;
	}
	return false;
}

bool MP4File::Set3GPMetadataInteger(const char *atom, uint64_t value)
{
  char atomstring[60] = {0};
  MP4Atom *pMetaAtom;
  MP4IntegerProperty *pMetadataProperty = NULL;
  snprintf(atomstring, 60, "moov.udta.%s", atom);

  pMetaAtom = m_pRootAtom->FindAtomMP4(atomstring);
  
  if (!pMetaAtom)
    {
			(void)AddDescendantAtoms("moov", atomstring+5);
      //if (!CreateMetadataAtom(atom))
	//return false;
      
      pMetaAtom = m_pRootAtom->FindAtomMP4(atomstring);
      if (pMetaAtom == NULL) return false;
    }

	snprintf(atomstring, 60, "%s.metadata", atom);
  ASSERT(pMetaAtom->FindProperty(atomstring,
				 (MP4Property**)&pMetadataProperty));
  ASSERT(pMetadataProperty);
  
  pMetadataProperty->SetValue(value, 0);
	return true;
}

bool MP4File::Delete3GPMetadataAtom(const char* name)
{
  MP4Atom *pMetaAtom = NULL;
  char s[256];

    snprintf(s, 256, "moov.udta.%s", name);
    pMetaAtom = m_pRootAtom->FindAtomMP4(s);
  /* if it exists, delete it */
  if (pMetaAtom)
    {
      MP4Atom *pParent = pMetaAtom->GetParentAtom();

      pParent->DeleteChildAtom(pMetaAtom);

      delete pMetaAtom;

      return true;
    }

  return false;
}