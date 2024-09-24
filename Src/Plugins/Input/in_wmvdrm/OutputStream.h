#ifndef NULLSOFT_OUTPUTSTREAMH
#define NULLSOFT_OUTPUTSTREAMH

#include <wmsdk.h>
#define NULLSOFT_INTERFACE_BEGIN(RIID, OBJ) void **&NULLSOFT_interfaceHolder = OBJ; REFIID NULLSOFT_IID = RIID;
#define NULLSOFT_VALID_INTERFACE(a) if (NULLSOFT_IID == IID_ ## a) { *NULLSOFT_interfaceHolder = static_cast<a *>(this); return S_OK; }
#define NULLSOFT_INTERFACE_END()  *NULLSOFT_interfaceHolder = 0; return E_NOINTERFACE;

class OutputStream : public IWMOutputMediaProps
{
public:
	OutputStream(IWMMediaProps *props) : mediaType(0)
	{
		DWORD mediaTypeSize;
		props->GetMediaType(0, &mediaTypeSize);
		if (mediaTypeSize)
		{
			mediaType = (WM_MEDIA_TYPE *)new unsigned char[mediaTypeSize];
			props->GetMediaType(mediaType, &mediaTypeSize);
		}
	}

	~OutputStream()
	{
		if (mediaType)
		{
			delete mediaType;
			mediaType = 0;
		}
	}

	GUID &GetSubType() const
	{
		return mediaType->subtype;
	}

	WM_MEDIA_TYPE *mediaType;

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
	{
		NULLSOFT_INTERFACE_BEGIN(riid, ppvObject)
		NULLSOFT_VALID_INTERFACE(IWMOutputMediaProps);
		NULLSOFT_VALID_INTERFACE(IWMMediaProps);
		NULLSOFT_INTERFACE_END()
	}

	ULONG STDMETHODCALLTYPE AddRef()
	{
		return 0;
	}
	ULONG STDMETHODCALLTYPE Release()
		{
		return 0;
	}
	HRESULT STDMETHODCALLTYPE GetType(GUID *pguidType)
	{
		if (!mediaType) return E_FAIL;
		*pguidType = mediaType->majortype;
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE GetMediaType(WM_MEDIA_TYPE *pType, DWORD *pcbType)
		{
			if (!mediaType) return E_FAIL;
			if (!pType)
			{
				if (!pcbType)	return E_INVALIDARG;
				*pcbType = sizeof(WM_MEDIA_TYPE);
			}
			else
			{
				if (*pcbType < sizeof(WM_MEDIA_TYPE)) ASF_E_BUFFERTOOSMALL;
				memcpy(pType, mediaType, sizeof(WM_MEDIA_TYPE));
			}
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE SetMediaType(WM_MEDIA_TYPE *pType)
	{
		return E_NOTIMPL;
	}
	HRESULT STDMETHODCALLTYPE GetStreamGroupName(WCHAR *pwszName, WORD *pcchName)
	{
		return E_NOTIMPL;
	}
	HRESULT STDMETHODCALLTYPE GetConnectionName(WCHAR *pwszName, WORD *pcchName)
	{
		return E_NOTIMPL;
	}
};
#include <uuids.h>
class VideoOutputStream : public OutputStream
{
public:

	VideoOutputStream(IWMMediaProps *props) : OutputStream(props)
	{}

	WMVIDEOINFOHEADER *VideoInfo() const
	{
		return (WMVIDEOINFOHEADER *)mediaType->pbFormat;
	}
int SourceWidth() const
{
		return VideoInfo()->rcSource.right - VideoInfo()->rcSource.left;
	}
	int DestinationWidth() const
	{
		return VideoInfo()->rcTarget.right - VideoInfo()->rcTarget.left;
	}

	int DestinationHeight() const
	{
		return VideoInfo()->rcTarget.bottom - VideoInfo()->rcTarget.top;
	}

	bool Flipped() const
	{
		BITMAPINFOHEADER &info = VideoInfo()->bmiHeader;
		if (info.biHeight < 0			|| info.biCompression == 0)
			return true;
		else
			return false;

	}
	int bmiHeight()
	{
	return	VideoInfo()->bmiHeader.biYPelsPerMeter;
	}
	int bmiWidth()
	{
	return	VideoInfo()->bmiHeader.biXPelsPerMeter;
	}
	RGBQUAD *CreatePalette()
	{
		RGBQUAD *palette = (RGBQUAD *)calloc(1, 1024);
		BITMAPINFOHEADER &info = VideoInfo()->bmiHeader;
		memcpy(palette, (char *)(&info) + 40, info.biClrUsed * 4);
		return palette;
	}
	int FourCC() const
	{
		BITMAPINFOHEADER &info = VideoInfo()->bmiHeader;
		int fourcc = info.biCompression;
		if (fourcc == BI_RGB)
		{			
			switch(info.biBitCount)
			{
			case 32:
				fourcc='23GR'; // RG32
				break;
			case 24:
				fourcc='42GR'; // RG24
				break;
			case 8:
				fourcc='8BGR'; // RGB8
				break;
			}
		} else 		 if (fourcc == BI_BITFIELDS)
			fourcc = 0; // TODO: calc a CC that winamp likes
		return fourcc;
	}

	bool IsVideo() const
	{
		return !!(mediaType->formattype == WMFORMAT_VideoInfo);
	}
};

#endif
