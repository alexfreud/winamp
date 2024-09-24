#include "main.h"
#include "./deviceView.h"
#include <wincodec.h>

#include <commctrl.h>
#include <strsafe.h>
#include <vector>
#include <algorithm>

#define DEVICEVIEW_PROP		L"NullsoftDevicesViewProp"
static ATOM DEVICEVIEW_ATOM = 0;

typedef struct DeviceView
{
	Device *device;
} DeviceView;


typedef std::vector<DeviceIconInfo*> DeviceIconInfoList;

static INT_PTR
DeviceView_DialogProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam);


#define DEVICEVIEW(_hwnd) ((DeviceView*)GetPropW((_hwnd), MAKEINTATOM(DEVICEVIEW_ATOM)))
#define DEVICEVIEW_RET_VOID(_view, _hwnd) { (_view) = DEVICEVIEW((_hwnd)); if (NULL == (_view)) return; }
#define DEVICEVIEW_RET_VAL(_view, _hwnd, _error) { (_view) = DEVICEVIEW((_hwnd)); if (NULL == (_view)) return (_error); }



static HBITMAP
DeviceView_HBitmapFromWicSource(IWICBitmapSource *wicSource)
{	
	HRESULT hr;
    HBITMAP bitmap;
	BITMAPINFO bitmapInfo;
	unsigned int width, height;
	void *pixelData = NULL;
    HDC windowDC;
	unsigned int strideSize, imageSize;

	if (NULL == wicSource)
		return NULL;

	hr = wicSource->GetSize(&width, &height);
	if (FAILED(hr))
		return NULL;
    		
	ZeroMemory(&bitmapInfo, sizeof(bitmapInfo));
    bitmapInfo.bmiHeader.biSize  = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = width;
    bitmapInfo.bmiHeader.biHeight = -(LONG)height;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

	windowDC = GetDCEx(NULL, NULL, DCX_WINDOW | DCX_CACHE);
 
	bitmap = CreateDIBSection(windowDC, &bitmapInfo, DIB_RGB_COLORS, &pixelData, NULL, 0);
		
	if (NULL != windowDC)
		ReleaseDC(NULL, windowDC);

	if (NULL == bitmap)
		return NULL;

    hr = UIntMult(width, sizeof(DWORD), &strideSize);
    if (SUCCEEDED(hr))
	{
		hr = UIntMult(strideSize, height, &imageSize);
    	if (SUCCEEDED(hr))
			hr = wicSource->CopyPixels(NULL, strideSize, imageSize, (BYTE*)pixelData);
    }
    
	if (FAILED(hr))
	{
		DeleteObject(bitmap);
		bitmap = NULL;
	}
    
	return bitmap;
}

static HBITMAP
DeviceView_LoadIcon(const wchar_t *path)
{
	IWICImagingFactory *wicFactory;
	IWICBitmapDecoder *wicDecoder;
		
	HRESULT hr;
	HBITMAP bitmap;
	
	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, 
		IID_PPV_ARGS(&wicFactory));
	
	if (FAILED(hr))
		return NULL;
	
	bitmap = NULL;

	hr = wicFactory->CreateDecoderFromFilename(path, NULL, GENERIC_READ, 
						WICDecodeMetadataCacheOnDemand, &wicDecoder);

	if (SUCCEEDED(hr))
	{
		IWICBitmapFrameDecode *wicFrame;
		hr = wicDecoder->GetFrame(0, &wicFrame);
		if(SUCCEEDED(hr))
		{
			IWICBitmapSource *wicSource;
			hr = wicFrame->QueryInterface(IID_IWICBitmapSource, 
									reinterpret_cast<void **>(&wicSource));
			if (SUCCEEDED(hr))
			{
				WICPixelFormatGUID pixelFormat;
				hr = wicSource->GetPixelFormat(&pixelFormat);
				if (FAILED(hr) ||
					(GUID_WICPixelFormat32bppPBGRA != pixelFormat &&
					 GUID_WICPixelFormat32bppBGR != pixelFormat &&
					 GUID_WICPixelFormat32bppBGRA != pixelFormat &&
					 GUID_WICPixelFormat32bppRGBA != pixelFormat &&
					 GUID_WICPixelFormat32bppPRGBA != pixelFormat))
				{
					// try to convert
					IWICFormatConverter *wicConverter;
					hr = wicFactory->CreateFormatConverter(&wicConverter);
					if (SUCCEEDED(hr))
					{
						hr = wicConverter->Initialize(wicSource, GUID_WICPixelFormat32bppPBGRA, 
									WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeCustom);
						
						if (SUCCEEDED(hr))
						{
							IWICBitmapSource *wicConvertedSource;
							hr = wicConverter->QueryInterface(IID_IWICBitmapSource, 
												reinterpret_cast<void **>(&wicConvertedSource));
							if (SUCCEEDED(hr))
							{
								wicSource->Release();
								wicSource = wicConvertedSource;
							}
						}
						wicConverter->Release();
					}
				}
				
				if (SUCCEEDED(hr))
					bitmap = DeviceView_HBitmapFromWicSource(wicSource);

				wicSource->Release();
			}
			wicFrame->Release();
		}
		wicDecoder->Release();
	}


	wicFactory->Release();
	return bitmap;
	
}

HWND 
DeviceView_CreateWindow(HWND parentWindow, Device *device)
{
	HWND hwnd;
	
	if (NULL == device)
		return NULL;

	if (0 == DEVICEVIEW_ATOM)
	{
		DEVICEVIEW_ATOM = GlobalAddAtom(DEVICEVIEW_PROP);
		if (0 == DEVICEVIEW_ATOM)
			return NULL;

	}

	hwnd = WASABI_API_CREATEDIALOGPARAMW((INT_PTR)IDD_DEVICE_VIEW, parentWindow, 
					DeviceView_DialogProc, (LPARAM)device);

	return hwnd;
}

static void
DeviceView_InitCapacity(HWND hwnd)
{
	DeviceView *self;
	HWND controlWindow;
	uint64_t totalSpace, usedSpace;

	DEVICEVIEW_RET_VOID(self, hwnd);

	if (NULL != self->device)
	{
		if (FAILED(self->device->GetTotalSpace(&totalSpace)))
			totalSpace = 0;

		if (FAILED(self->device->GetUsedSpace(&usedSpace)))
			usedSpace = 0;
	}
	else
	{
		totalSpace = 0;
		usedSpace = 0;
	}

	controlWindow = GetDlgItem(hwnd, IDC_SPIN_TOTALSPACE);
	if (NULL != controlWindow)
	{
		SendMessage(controlWindow, UDM_SETRANGE32, (WPARAM)0, (LPARAM)0xFFFFFF);
		SendMessage(controlWindow, UDM_SETPOS32, (WPARAM)0, (LPARAM)totalSpace);
	}

	controlWindow = GetDlgItem(hwnd, IDC_SPIN_USEDSPACE);
	if (NULL != controlWindow)
	{
		SendMessage(controlWindow, UDM_SETRANGE32, (WPARAM)0, (LPARAM)totalSpace);
		SendMessage(controlWindow, UDM_SETPOS32, (WPARAM)0, (LPARAM)usedSpace);
	}

}

static int
DeviceView_CompareDeviceIconInfo(const void *elem1, const void *elem2)
{
	DeviceIconInfo *info1;
	DeviceIconInfo *info2;

	info1 = *((DeviceIconInfo**)elem1);
	info2 = *((DeviceIconInfo**)elem2);

	if (NULL == info1 || NULL == info2)
		return (int)(info1 - info2);
	if (info1->width != info2->width)
		return (int)(info1->width - info2->width);

	if (info1->height != info2->height)
		return (int)(info1->height - info2->height);

	if (NULL == info1->path || NULL == info2->path)
		return (int)(info1->path - info2->path);

	return CompareString(CSTR_INVARIANT, NORM_IGNORECASE, info1->path, -1, info2->path, -1) - 2;
}

static int
DeviceView_CompareDeviceIconInfo_V2(const void* elem1, const void* elem2)
{
	return DeviceView_CompareDeviceIconInfo(elem1, elem2) < 0;
}

static BOOL
DeviceView_EnumerateIcons(const wchar_t *path, unsigned int width, unsigned int height, void *user)
{
	DeviceIconInfo *info;

	DeviceIconInfoList *list = (DeviceIconInfoList*)user;
	if( NULL == list)
		return FALSE;

	info = (DeviceIconInfo*)malloc(sizeof(DeviceIconInfo));
	if (NULL != info)
	{
		info->height = height;
		info->width = width;
		info->path = String_Duplicate(path);

		list->push_back(info);
	}

	
	return TRUE;
}

static void 
DeviceView_DestroyIcons(HWND hwnd)
{
	HWND controlWindow;
	int count, index;
	DeviceIconInfo *info;


	controlWindow = GetDlgItem(hwnd, IDC_COMBO_ICONS);
	if (NULL == controlWindow)
		return;

	count = (int)SendMessage(controlWindow, CB_GETCOUNT, 0, 0L);
	for(index = 0; index < count; index++)
	{
		info = (DeviceIconInfo*)SendMessage(controlWindow, CB_GETITEMDATA, index, 0L);
		if (CB_ERR != (INT_PTR)info)
		{
			String_Free(info->path);
			free(info);
		}
	}

	SendMessage(controlWindow, CB_RESETCONTENT, 0, 0L);

	controlWindow = GetDlgItem(hwnd, IDC_STATIC_PREVIEWICON);
	if (NULL != controlWindow)
	{
		HBITMAP bitmap;
		bitmap = (HBITMAP)SendMessage(controlWindow, STM_GETIMAGE, IMAGE_BITMAP, 0L);
		if(NULL != bitmap)
			DeleteObject(bitmap);
	}

}

static void 
DeviceView_InitIcons(HWND hwnd, const wchar_t *selectPath)
{
	DeviceView *self;
	DeviceIconInfoList list;
	HWND controlWindow;
	wchar_t buffer[2048];
	
	DEVICEVIEW_RET_VOID(self, hwnd);

	DeviceView_DestroyIcons(hwnd);

	controlWindow = GetDlgItem(hwnd, IDC_COMBO_ICONS);
	if (NULL != controlWindow)
	{
		size_t index, count;
		int iItem, iSelect;
		DeviceIconInfo *info;
		
		if (NULL != self->device)
		{
			self->device->EnumerateIcons(DeviceView_EnumerateIcons, &list);
		}

		count = list.size();
		//qsort(list.begin(), count, sizeof(DeviceIconInfo**), DeviceView_CompareDeviceIconInfo);
		std::sort(list.begin(), list.end(), DeviceView_CompareDeviceIconInfo_V2);
		
		iSelect = 0;

		for(index = 0; index < count; index++)
		{			
			info = list[index];
			if (1 == info->width && 1 == info->height)
			{
				StringCchPrintf(buffer, ARRAYSIZE(buffer), L"%s", 
								info->path);
			}
			else
			{
				StringCchPrintf(buffer, ARRAYSIZE(buffer), L"[%dx%d] - %s", 
								info->width, info->height, info->path);
			}
	
			iItem = (int)SendMessage(controlWindow, CB_ADDSTRING, 0, (LPARAM)buffer);
			if (CB_ERR != iItem)
			{
				if (CB_ERR == (int)SendMessage(controlWindow, CB_SETITEMDATA, index, (LPARAM)info))
				{
					SendMessage(controlWindow, CB_DELETESTRING, index, 0L);
					iItem = CB_ERR;
				}
			}
		
			if (CB_ERR == iItem)
			{
				free(info);
			}
			else if (NULL != selectPath && 
				 		CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, 
													info->path, -1, selectPath, -1))
			{
				iSelect = iItem;
			}
		}

		iItem = (int)SendMessage(controlWindow, CB_GETCOUNT, 0, 0L);
		if (CB_ERR == iItem)
			iItem = 0;
		EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_EDITICON), (0 != iItem));
		EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_REMOVEICON),(0 != iItem));
		

		SendMessage(controlWindow, CB_SETCURSEL, iSelect, 0L);
		PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_COMBO_ICONS, CBN_SELENDOK), (LPARAM)controlWindow);
	}
}

static void
DeviceView_NewIcon(HWND hwnd)
{
	DeviceView *self;
	DeviceIconInfo info;
	INT_PTR result;
	
	DEVICEVIEW_RET_VOID(self, hwnd);

	ZeroMemory(&info, sizeof(info));

	result = DeviceIconEditor_Show(hwnd, &info);
	if (IDOK == result)
	{
		if (NULL != self->device &&
			SUCCEEDED(self->device->AddIcon(info.path, info.width, info.height)))
		{
			DeviceView_InitIcons(hwnd, info.path);
		}

		String_Free(info.path);
	}
}

static void
DeviceView_RemoveIcon(HWND hwnd)
{
	DeviceView *self;
	DeviceIconInfo *info;
	int index, count;
	HWND controlWindow;
	
	DEVICEVIEW_RET_VOID(self, hwnd);

	controlWindow = GetDlgItem(hwnd, IDC_COMBO_ICONS);
	if (NULL == controlWindow)
		return;

	index = (int)SendMessage(controlWindow, CB_GETCURSEL, 0, 0L);
	if (CB_ERR == index)
		return;
	
	info = (DeviceIconInfo*)SendMessage(controlWindow, CB_GETITEMDATA, index, 0L);
	if (CB_ERR != (INT_PTR)info)
	{
		if (NULL != self->device)
			self->device->RemoveIcon(info->width, info->height);
		
		String_Free(info->path);
		free(info);
	}
	
	SendMessage(controlWindow, CB_DELETESTRING, index, 0L);

	count = (int)SendMessage(controlWindow, CB_GETCOUNT, 0, 0L);
	if (count > 0)
	{
		if (index > count)
			index = count - 1;
		SendMessage(controlWindow, CB_SETCURSEL, index, 0L);
	}
	else
	{
		SendMessage(controlWindow, CB_SETCURSEL, -1, 0L);
		EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_EDITICON),FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_REMOVEICON),FALSE);
	}

	PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_COMBO_ICONS, CBN_SELENDOK), (LPARAM)controlWindow);

	UpdateWindow(controlWindow);
}

static void
DeviceView_EditIcon(HWND hwnd)
{
	DeviceView *self;
	DeviceIconInfo *info;
	int index;
	unsigned int width, height;
	HWND controlWindow;
	INT_PTR result;
	
	DEVICEVIEW_RET_VOID(self, hwnd);

	controlWindow = GetDlgItem(hwnd, IDC_COMBO_ICONS);
	if (NULL == controlWindow)
		return;

	index = (int)SendMessage(controlWindow, CB_GETCURSEL, 0, 0L);
	if (CB_ERR == index)
		return;
	
	info = (DeviceIconInfo*)SendMessage(controlWindow, CB_GETITEMDATA, index, 0L);
	if (CB_ERR == (INT_PTR)info)
		return;
	width = info->width;
	height = info->height;

	result = DeviceIconEditor_Show(hwnd, info);
	if (IDOK == result)
	{
		if (NULL != self->device)
		{
			self->device->RemoveIcon(width, height);
			self->device->AddIcon(info->path, info->width, info->height);
			DeviceView_InitIcons(hwnd, info->path);
		}
	}

	UpdateWindow(controlWindow);
}

static void
DeviceView_InitView(HWND hwnd)
{
	DeviceView *self;
	HWND controlWindow;
	wchar_t buffer[1024];

	DEVICEVIEW_RET_VOID(self, hwnd);

	controlWindow = GetDlgItem(hwnd, IDC_EDIT_NAME);
	if (NULL != controlWindow)
	{
		if (NULL == self->device || 
			0 == MultiByteToWideChar(CP_UTF8, 0, self->device->GetName(), -1, buffer, ARRAYSIZE(buffer)))
		{
			StringCchCopy(buffer, ARRAYSIZE(buffer), L"<unknown>");
		}
		SetWindowText(controlWindow, buffer);
	}

	controlWindow = GetDlgItem(hwnd, IDC_EDIT_TITLE);
	if (NULL != controlWindow)
	{
		if (NULL == self->device || 
			FAILED(self->device->GetDisplayName(buffer, ARRAYSIZE(buffer))))
		{
			StringCchCopy(buffer, ARRAYSIZE(buffer), L"<unknown>");
		}
		SetWindowText(controlWindow, buffer);
	}

	controlWindow = GetDlgItem(hwnd, IDC_EDIT_TYPE);
	if (NULL != controlWindow)
	{
		buffer[0] = L'\0';

		if (NULL != self->device)
		{
			const char *typeName;
			ifc_devicetype *type;

			typeName = self->device->GetType();

			if (NULL != WASABI_API_DEVICES &&
				S_OK == WASABI_API_DEVICES->TypeFind(typeName, &type))
			{
				if (FAILED(type->GetDisplayName(buffer, ARRAYSIZE(buffer))))
					buffer[0] = L'\0';

				type->Release();
			}

			if (L'\0' == *buffer)
				MultiByteToWideChar(CP_UTF8, 0, typeName, -1, buffer, ARRAYSIZE(buffer));
		}

		if (L'\0' == *buffer)
			StringCchCopy(buffer, ARRAYSIZE(buffer), L"<unknown>");
		
		SetWindowText(controlWindow, buffer);
	}

	controlWindow = GetDlgItem(hwnd, IDC_EDIT_CONNECTION);
	if (NULL != controlWindow)
	{
		buffer[0] = L'\0';

		if (NULL != self->device)
		{
			const char *connectionName;
			ifc_deviceconnection *connection;

			connectionName = self->device->GetConnection();

			if (NULL != WASABI_API_DEVICES &&
				S_OK == WASABI_API_DEVICES->ConnectionFind(connectionName, &connection))
			{
				if (FAILED(connection->GetDisplayName(buffer, ARRAYSIZE(buffer))))
					buffer[0] = L'\0';

				connection->Release();
			}

			if (L'\0' == *buffer)
				MultiByteToWideChar(CP_UTF8, 0, connectionName, -1, buffer, ARRAYSIZE(buffer));
		}

		if (L'\0' == *buffer)
			StringCchCopy(buffer, ARRAYSIZE(buffer), L"<unknown>");
		
		SetWindowText(controlWindow, buffer);
	}

	controlWindow = GetDlgItem(hwnd, IDC_COMBO_ATTACHED);
	if (NULL != controlWindow)
	{
		SendMessage(controlWindow, CB_RESETCONTENT, 0, 0L);

		if (NULL != self->device)
		{
			const wchar_t *searchString;

			SendMessage(controlWindow, CB_ADDSTRING, 0, (LPARAM)L"Yes");
			SendMessage(controlWindow, CB_ADDSTRING, 0, (LPARAM)L"No");

			if (FALSE == self->device->GetAttached())
				searchString = L"No";
			else
				searchString = L"Yes";

			SendMessage(controlWindow, CB_SELECTSTRING, -1, (LPARAM)searchString);
		}
		else
		{
			SendMessage(controlWindow, CB_ADDSTRING, 0, (LPARAM)L"<unknown>");
			SendMessage(controlWindow, CB_SETCURSEL, 0, 0L);
		}
	
	}

	controlWindow = GetDlgItem(hwnd, IDC_COMBO_VISIBLE);
	if (NULL != controlWindow)
	{
		SendMessage(controlWindow, CB_RESETCONTENT, 0, 0L);

		if (NULL != self->device)
		{
			const wchar_t *searchString;

			SendMessage(controlWindow, CB_ADDSTRING, 0, (LPARAM)L"Yes");
			SendMessage(controlWindow, CB_ADDSTRING, 0, (LPARAM)L"No");

			if (FALSE != self->device->GetHidden())
				searchString = L"No";
			else
				searchString = L"Yes";

			SendMessage(controlWindow, CB_SELECTSTRING, -1, (LPARAM)searchString);
		}
		else
		{
			SendMessage(controlWindow, CB_ADDSTRING, 0, (LPARAM)L"<unknown>");
			SendMessage(controlWindow, CB_SETCURSEL, 0, 0L);
		}
	}

	DeviceView_InitCapacity(hwnd);
	DeviceView_InitIcons(hwnd, NULL);

}

static INT_PTR 
DeviceView_OnInitDialog(HWND hwnd, HWND focusWindow, LPARAM param)
{
	DeviceView *self;
		
	self = (DeviceView*)malloc(sizeof(DeviceView));
	if (NULL != self) 
	{
		ZeroMemory(self, sizeof(DeviceView));
		if (FALSE == SetProp(hwnd, MAKEINTATOM(DEVICEVIEW_ATOM), self))
		{
			free(self);
			self = NULL;
		}
	}

	if (NULL == self)
	{
		DestroyWindow(hwnd);
		return 0;
	}

	self->device = (Device*)param;
	if (NULL != self->device)
		self->device->AddRef();

	DeviceView_InitView(hwnd);

	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, 
		SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);

	return 0;
}

static void
DeviceView_OnDestroy(HWND hwnd)
{
	DeviceView *self;
		
	DeviceView_DestroyIcons(hwnd);

	self = DEVICEVIEW(hwnd);
	RemoveProp(hwnd, MAKEINTATOM(DEVICEVIEW_ATOM));

	if (NULL == self) 
		return;
	
	if (NULL != self->device)
		self->device->Release();

	free(self);
}

static void
DeviceView_OnTitleEditChanged(HWND hwnd)
{
	DeviceView *self;
	HWND controlWindow;
	wchar_t buffer[1024];
	
	DEVICEVIEW_RET_VOID(self, hwnd);

	controlWindow = GetDlgItem(hwnd, IDC_EDIT_TITLE);
	if (NULL == controlWindow)
		return;

	if (NULL == self->device)
		return;

	GetWindowText(controlWindow, buffer, ARRAYSIZE(buffer));

	self->device->SetDisplayName(buffer);

}

static void
DeviceView_OnAttachedComboChanged(HWND hwnd)
{
	DeviceView *self;
	HWND controlWindow;
	wchar_t buffer[1024];
	int index;
	
	DEVICEVIEW_RET_VOID(self, hwnd);

	controlWindow = GetDlgItem(hwnd, IDC_COMBO_ATTACHED);
	if (NULL == controlWindow)
		return;

	if (NULL == self->device)
		return;

	index = (int)SendMessage(controlWindow, CB_GETCURSEL, 0, 0);
	if (CB_ERR != index &&
		CB_ERR != SendMessage(controlWindow, CB_GETLBTEXT, index, (LPARAM)buffer))
	{
		if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, buffer, -1, L"Yes", -1))
		{
			self->device->Attach(NULL);
		}
		else if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, buffer, -1, L"No", -1))
		{
			self->device->Detach(NULL);
		}
	}
}

static void
DeviceView_OnVisibleComboChanged(HWND hwnd)
{
	DeviceView *self;
	HWND controlWindow;
	wchar_t buffer[1024];
	int index;
	
	DEVICEVIEW_RET_VOID(self, hwnd);

	controlWindow = GetDlgItem(hwnd, IDC_COMBO_VISIBLE);
	if (NULL == controlWindow)
		return;

	if (NULL == self->device)
		return;

	index = (int)SendMessage(controlWindow, CB_GETCURSEL, 0, 0);
	if (-1 != index &&
		CB_ERR != SendMessage(controlWindow, CB_GETLBTEXT, index, (LPARAM)buffer))
	{
		if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, buffer, -1, L"Yes", -1))
		{
			self->device->SetHidden(FALSE);
		}
		else if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, buffer, -1, L"No", -1))
		{
			self->device->SetHidden(TRUE);
		}
	}
}

static void
DeviceView_OnIconsComboChanged(HWND hwnd)
{
	DeviceView *self;
	HWND controlWindow, pictureWindow;
	int index;
	HBITMAP bitmap, previousBitmap;
	
	DEVICEVIEW_RET_VOID(self, hwnd);

	controlWindow = GetDlgItem(hwnd, IDC_COMBO_ICONS);
	if (NULL == controlWindow)
		return;

	if (NULL == self->device)
		return;

	pictureWindow = GetDlgItem(hwnd, IDC_STATIC_PREVIEWICON);
	if (NULL == pictureWindow)
		return;

	bitmap = NULL;
	index = (int)SendMessage(controlWindow, CB_GETCURSEL, 0, 0);\
	if (CB_ERR != index)
	{
		DeviceIconInfo *info;
		info = (DeviceIconInfo*)SendMessage(controlWindow, CB_GETITEMDATA, index, 0L);
		if (CB_ERR != (INT_PTR)info && NULL != info->path)
		{
			bitmap = DeviceView_LoadIcon(info->path);
		}
	}
	
	previousBitmap = (HBITMAP)SendMessage(pictureWindow, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bitmap);
	if (NULL != previousBitmap)
		DeleteObject(previousBitmap);

	previousBitmap = (HBITMAP)SendMessage(pictureWindow, STM_GETIMAGE, IMAGE_BITMAP, 0L);
	if(previousBitmap != bitmap)
	{
		if (NULL != bitmap)
			DeleteObject(bitmap);
	}
	

}

static void
DeviceView_OnTotalSpaceEditChanged(HWND hwnd)
{
	DeviceView *self;
	HWND controlWindow;
	uint64_t totalSpace;
	BOOL error;
	
	DEVICEVIEW_RET_VOID(self, hwnd);

	controlWindow = GetDlgItem(hwnd, IDC_SPIN_TOTALSPACE);
	if (NULL == controlWindow)
		return;

	if (NULL == self->device)
		return;

	totalSpace = (size_t)SendMessage(controlWindow, UDM_GETPOS32, 0, (LPARAM)&error);
	if (FALSE != error)
		return;

	if (FAILED(self->device->SetTotalSpace(totalSpace)))
	{
		if (FAILED(self->device->GetTotalSpace(&totalSpace)))
			totalSpace = 0;
	}

	controlWindow = GetDlgItem(hwnd, IDC_SPIN_USEDSPACE);
	if (NULL != controlWindow)
	{
		SendMessage(controlWindow, UDM_SETRANGE32, (WPARAM)0, (LPARAM)totalSpace);
	}
}

static void
DeviceView_OnUsedSpaceEditChanged(HWND hwnd)
{
	DeviceView *self;
	HWND controlWindow;
	uint64_t usedSpace;
	BOOL error;
	
	DEVICEVIEW_RET_VOID(self, hwnd);

	controlWindow = GetDlgItem(hwnd, IDC_SPIN_USEDSPACE);
	if (NULL == controlWindow)
		return;

	if (NULL == self->device)
		return;

	usedSpace = (size_t)SendMessage(controlWindow, UDM_GETPOS32, 0, (LPARAM)&error);
	if (FALSE != error)
		return;

	if (FAILED(self->device->SetUsedSpace(usedSpace)))
	{
		if (FAILED(self->device->GetTotalSpace(&usedSpace)))
			usedSpace = 0;
	}
}

static void
DeviceView_OnCommand(HWND hwnd, INT commandId, INT eventId, HWND controlWindow)
{
	switch(commandId)
	{
		case IDC_EDIT_TITLE:
			switch(eventId)
			{
				case EN_CHANGE:	
					DeviceView_OnTitleEditChanged(hwnd);
					break;
			}
			break;
		case IDC_COMBO_ATTACHED:
			switch(eventId)
			{
				case CBN_SELENDOK:
					DeviceView_OnAttachedComboChanged(hwnd);
					break;
			}
			break;

		case IDC_COMBO_VISIBLE:
			switch(eventId)
			{
				case CBN_SELENDOK:
					DeviceView_OnVisibleComboChanged(hwnd);
					break;
			}
			break;
		case IDC_COMBO_ICONS:
			switch(eventId)
			{
				case CBN_SELENDOK:
					DeviceView_OnIconsComboChanged(hwnd);
					break;
			}
			break;
		case IDC_EDIT_TOTALSPACE:
			switch(eventId)
			{
				case EN_CHANGE:	
					DeviceView_OnTotalSpaceEditChanged(hwnd);
					break;
			}
			break;
		case IDC_EDIT_USEDSPACE:
			switch(eventId)
			{
				case EN_CHANGE:	
					DeviceView_OnUsedSpaceEditChanged(hwnd);
					break;
			}
			break;
		case IDC_BUTTON_NEWICON:
			switch(eventId)
			{
				case BN_CLICKED:
					DeviceView_NewIcon(hwnd);
					break;
			}
			break;
		case IDC_BUTTON_REMOVEICON:
			switch(eventId)
			{
				case BN_CLICKED:
					DeviceView_RemoveIcon(hwnd);
					break;
			}
			break;
		case IDC_BUTTON_EDITICON:
			switch(eventId)
			{
				case BN_CLICKED:
					DeviceView_EditIcon(hwnd);
					break;
			}
			break;
	}
}


static INT_PTR
DeviceView_DialogProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:		return DeviceView_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:		DeviceView_OnDestroy(hwnd); return TRUE;
		case WM_COMMAND:		DeviceView_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;
	}
	return 0;
}