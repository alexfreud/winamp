#ifndef NULLSOFT_WMINFORMATIONH
#define NULLSOFT_WMINFORMATIONH

#include <wmsdk.h>
#include "WMCallback.h"

class WMInformation : public WMHandler
{
public:
	WMInformation(const wchar_t *fileName, bool noBlock=false);
	WMInformation(IWMReader *reader);
	//WMInformation(IWMSyncReader *reader);
	WMInformation(IWMMetadataEditor *_editor);
	//WMInformation();
	bool ErrorOpening()
	{
		return openError;
	} // TODO: benski> this is only valid for WMInformation(const wchar_t *fileName, bool noBlock=false)!!!
	virtual ~WMInformation();

	bool MakeWritable(const wchar_t *fileName);
	bool NonWritable();
	bool MakeReadOnly(const wchar_t *fileName);
	bool Flush();
	bool IsSeekable();
	long GetLengthMilliseconds();
	long GetBitrate();
	WORD GetNumberAttributes();
	void ClearAllAttributes();
	bool IsAttribute(const wchar_t attrName[]); // false might mean "attribute not found", see IsNotAttribute
	bool IsNotAttribute(const wchar_t attrName[]); // false might mean "attribute not found", see IsAttribute
	void GetAttribute(WORD index, wchar_t *attrName, size_t attrLen, wchar_t *valueStr, size_t valueStrLen);
	void GetAttribute(const wchar_t attrName[], wchar_t *valueStr, size_t len);
	void SetAttribute(const wchar_t *attrName, wchar_t *value, WMT_ATTR_DATATYPE defaultType = WMT_TYPE_STRING);
	void DeleteAttribute(const wchar_t *attrName);
	bool GetAttributeSize(const wchar_t *attrName, size_t &size);
	void LicenseRequired()
	{
		First().OpenFailed();
	}
	void SetAttribute_BinString(const wchar_t *attrName, wchar_t *value);
	void GetAttribute_BinString(const wchar_t attrName[], wchar_t *valueStr, size_t len);

	void DeleteUserText(const wchar_t *description);
	void SetUserText(const wchar_t *description, const wchar_t *valueStr);

	bool GetCodecName(wchar_t *storage, size_t len);
	bool GetPicture(void **data, size_t *len, wchar_t **mimeType, int type);
	bool SetPicture(void *data, size_t len, const wchar_t *mimeType, int type);
	bool DeletePicture(int type);
	bool HasPicture(int type);
private:
	bool GetDataType(const wchar_t *name, WMT_ATTR_DATATYPE &type);
	long GetLongAttr(const wchar_t name[]);
	bool GetBoolAttr(const wchar_t name[]);
	DWORD GetDWORDAttr(const wchar_t name[]);
	struct IWMMetadataEditor *editor;
	struct IWMMetadataEditor2 *editor2;
	struct IWMHeaderInfo *header;
	struct IWMHeaderInfo2 *header2;
	struct IWMHeaderInfo3 *header3;
	struct IWMReader *reader;

	WMCallback callback;
	HANDLE hEvent;
	bool openError;
	void NeedsIndividualization()
	{
		First().OpenFailed();
	}
	void Opened()
	{
		openError=false;
		SetEvent(hEvent);
	}
	void OpenFailed()
	{
		openError=true;
		SetEvent(hEvent);
	}
	void Error()
	{
		openError=true;
		SetEvent(hEvent);
	}

};

#endif
