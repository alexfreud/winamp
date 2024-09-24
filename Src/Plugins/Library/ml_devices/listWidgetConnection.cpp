#include "main.h"
#include "./listWidgetInternal.h"

#define LISTWIDGETCONNECTION_IMAGE_WIDTH	16
#define LISTWIDGETCONNECTION_IMAGE_HEIGHT	16

#define REQUEST_STRING   ((wchar_t*)1)
#define REQUEST_IMAGE   ((DeviceImage*)1)


typedef struct ListWidgetConnection
{
	char *name;
	wchar_t *title;
	DeviceImage *image;
} ListWidgetConnection;


ListWidgetConnection *
ListWidget_CreateConnection(const char *name)
{
	ListWidgetConnection *self;

	if (NULL == name)
		return NULL;

	self = (ListWidgetConnection*)malloc(sizeof(ListWidgetConnection));
	if (NULL == self)
		return NULL;
		
	ZeroMemory(self, sizeof(ListWidgetConnection));

	self->name = AnsiString_Duplicate(name);
	
	if (NULL == self->name)
	{
		ListWidget_DestroyConnection(self);
		return NULL;
	}
	
	self->title = REQUEST_STRING;
	self->image = REQUEST_IMAGE;

	return self;
}

void 
ListWidget_DestroyConnection(ListWidgetConnection *connection)
{
	if (NULL == connection)
		return;

	AnsiString_Free(connection->name);

	if (NULL != connection->image && REQUEST_IMAGE != connection->image)
		DeviceImage_Release(connection->image);
	
	if (REQUEST_STRING != connection->title)
		String_Free(connection->title);
	
	free(connection);
}

const wchar_t *
ListWidget_GetConnectionTitle(ListWidgetConnection *connection)
{
	if (NULL == connection || NULL == connection->title)
		return NULL;

	if (REQUEST_STRING == connection->title)
	{
		ifc_deviceconnection *info;

		connection->title = NULL;

		if (NULL != WASABI_API_DEVICES &&
			S_OK == WASABI_API_DEVICES->ConnectionFind(connection->name, &info))
		{
			wchar_t buffer[512] = {0};

			if (SUCCEEDED(info->GetDisplayName(buffer, ARRAYSIZE(buffer))))
				connection->title = String_Duplicate(buffer);

			info->Release();
		}
	}

	return connection->title;
}

HBITMAP
ListWidget_GetConnectionImage(WidgetStyle *style, ListWidgetConnection *connection, int width, int height)
{
	if (NULL == style || NULL == connection || NULL == connection->image)
		return NULL;

	if (REQUEST_IMAGE == connection->image)
	{
		ifc_deviceconnection *info;

		connection->image = NULL;

		if (NULL != WASABI_API_DEVICES &&
			S_OK == WASABI_API_DEVICES->ConnectionFind(connection->name, &info))
		{
			wchar_t buffer[MAX_PATH * 2] = {0};

			if (FAILED(info->GetIcon(buffer, ARRAYSIZE(buffer), width, height)))
				buffer[0] = L'\0';
		
			info->Release();

			if (L'\0' != buffer[0])
			{
				connection->image = DeviceImageCache_GetImage(Plugin_GetImageCache(), buffer, 
								width, height, NULL, NULL);
			}
		}
	}

	if (NULL == connection->image)
		return NULL;

	return DeviceImage_GetBitmap(connection->image, DeviceImage_Normal);
}

BOOL
ListWidget_ConnectionResetColors(WidgetStyle *style, ListWidgetConnection *connection)
{
	return FALSE;
}

void
ListWidget_ResetConnnectionsColors(ListWidget *self, WidgetStyle *style)
{
	return;
}


BOOL
ListWidget_UpdateConnectionImageSize(ListWidgetConnection *connection, int width, int height)
{
	if (NULL == connection)
		return FALSE;

	if (NULL == connection->image || REQUEST_IMAGE == connection->image)
		return TRUE;

	DeviceImage_Release(connection->image);
	connection->image = REQUEST_IMAGE;

	return TRUE;
}

ListWidgetConnection *
ListWidget_FindConnection(ListWidget *self, const char *name)
{
	if (NULL == self || FALSE != IS_STRING_EMPTY(name))
		return NULL;

	size_t index = self->connections.size();
	while(index--)
	{
		ListWidgetConnection *connection = self->connections[index];
		if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, 0, name, -1, connection->name, -1))
			return connection;
	}

	return NULL;
}

BOOL
ListWidget_AddConnection(ListWidget *self, ListWidgetConnection *connection)
{
	if (NULL == self || NULL == connection)
		return FALSE;

	self->connections.push_back(connection);
	return TRUE;
}

void
ListWidget_RemoveConnection(ListWidget *self, const char *name)
{
	size_t iCategory, iGroup, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;
	ListWidgetItem *item;


	if (NULL == self || FALSE != IS_STRING_EMPTY(name))
		return;

	size_t index = self->connections.size();
	while(index--)
	{
		ListWidgetConnection *connection = self->connections[index];
		if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, 0, name, -1, connection->name, -1))
		{
			for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
			{
				category = self->categories[iCategory];
				for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
				{
					group = category->groups[iGroup];
					for(iItem = 0; iItem < group->items.size(); iItem++)
					{
						item = group->items[iItem];
						if(item->connection == connection)
							item->connection = NULL;
					}
				}
			}

			self->connections.erase(self->connections.begin() + index);
			ListWidget_DestroyConnection(connection);
		}
	}
}

void
ListWidget_RemoveAllConnections(ListWidget *self)
{
	if (NULL == self)
		return;

	size_t index = self->connections.size();
	if (index > 0)
	{
		size_t iCategory, iGroup, iItem;
		ListWidgetCategory *category;
		ListWidgetGroup	*group;
		ListWidgetItem *item;

		for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
		{
			category = self->categories[iCategory];
			for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
			{
				group = category->groups[iGroup];
				for(iItem = 0; iItem < group->items.size(); iItem++)
				{
					item = group->items[iItem];
					if(NULL != item->connection)
					{
						item->connection = NULL;
					}
				}
			}
		}

		while(index--)
		{
			ListWidgetConnection *connection = self->connections[index];
			ListWidget_DestroyConnection(connection);
		}

		self->connections.clear();
	}


}