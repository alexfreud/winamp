#include <windows.h>
#include <bfc/platform/types.h>
#include <bfc/platform/guid.h>
#include <bfc/std_mkncc.h>
#include <rpc.h>
#include <stdio.h>
#include <api/service/api_service.h>
#include "../Agave/Component/ifc_wa5component.h"
#include "WbmSvcMgr.h"

/* layout (binary)
0xdeadbeef - 32 bits
service guid - 128 bits
service fourcc - 32 bits
length of service name - 16bits
service name - see previous
length of test string - 16 bits
test string - see previous
repeat as necessary
*/

static uint32_t magic_word = 0xdeadbeefUL;
int Add(HANDLE manifest, GUID service_guid, uint32_t service_type, const char *service_name, const char *service_test_string)
{
	DWORD bytesWritten=0;
	WriteFile(manifest, &magic_word, sizeof(magic_word), &bytesWritten, NULL);
	WriteFile(manifest, &service_guid, sizeof(service_guid), &bytesWritten, NULL);
	WriteFile(manifest, &service_type, sizeof(service_type), &bytesWritten, NULL);
	uint16_t service_name_length = 0;
	if (service_name)
		service_name_length = (uint16_t)strlen(service_name);
	WriteFile(manifest, &service_name_length, sizeof(service_name_length), &bytesWritten, NULL);
	if (service_name_length)
		WriteFile(manifest, service_name, service_name_length, &bytesWritten, NULL);
	uint16_t service_test_string_length = 0;
	if (service_test_string)
		service_test_string_length = (uint16_t)strlen(service_test_string);
	WriteFile(manifest, &service_test_string_length, sizeof(service_test_string_length), &bytesWritten, NULL);
	if (service_test_string_length)
		WriteFile(manifest, service_test_string, service_test_string_length, &bytesWritten, NULL);
	return 0;
}

static GUID MakeGUID(const char *source)
{
	if (!strchr(source, '{') || !strchr(source, '}')) 
		return INVALID_GUID;

	GUID guid;
	int Data1, Data2, Data3;
	int Data4[8];

	// {1B3CA60C-DA98-4826-B4A9-D79748A5FD73}
	int n = sscanf( source, " { %08x - %04x - %04x - %02x%02x - %02x%02x%02x%02x%02x%02x } ",
		&Data1, &Data2, &Data3, Data4 + 0, Data4 + 1,
		Data4 + 2, Data4 + 3, Data4 + 4, Data4 + 5, Data4 + 6, Data4 + 7 );

	if (n != 11) return INVALID_GUID;

	// Cross assign all the values
	guid.Data1 = Data1;
	guid.Data2 = Data2;
	guid.Data3 = Data3;
	guid.Data4[0] = Data4[0];
	guid.Data4[1] = Data4[1];
	guid.Data4[2] = Data4[2];
	guid.Data4[3] = Data4[3];
	guid.Data4[4] = Data4[4];
	guid.Data4[5] = Data4[5];
	guid.Data4[6] = Data4[6];
	guid.Data4[7] = Data4[7];

	return guid;
}

static ifc_wa5component *LoadWasabiComponent(const char *filename, api_service *svcmgr)
{
	ifc_wa5component * comp = 0;
	HMODULE wacLib = LoadLibraryA(filename);
	if (wacLib)
	{
		GETCOMPONENT_FUNC f = (GETCOMPONENT_FUNC)GetProcAddress(wacLib, "GetWinamp5SystemComponent");
		if (f)
		{
			comp = f();
			if (comp)
			{
				comp->hModule = wacLib;
				comp->RegisterServices(svcmgr);
			}
		}
	}
	return comp;
}

void UnloadWasabiComponent(ifc_wa5component *comp, api_service *svcmgr)
{
	comp->DeregisterServices(svcmgr);

	FreeLibrary(comp->hModule);
}

int main(int argc, char *argv[])
{
	if (argc < 3)
		return 1; 

	const char *command=argv[1];
	const char *filename=argv[2];

	if (!_stricmp(command, "create"))
	{
		const char *guid = argv[3];
		const char *type = argv[4];
		const char *name = argv[5];
		const char *test_string = (argc>6)?argv[5]:NULL;
		GUID service_guid = MakeGUID(guid);

		FOURCC service_type = MK4CC(type[0], type[1], type[2], type[3]);


		HANDLE manifest = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		if (manifest != INVALID_HANDLE_VALUE)
		{
			Add(manifest, service_guid, service_type, name, test_string);
			CloseHandle(manifest);
		}
	}
	else if (!_stricmp(command, "add"))
	{
		const char *guid = argv[3];
		const char *type = argv[4];
		const char *name = argv[5];
		const char *test_string = (argc>6)?argv[5]:NULL;
		GUID service_guid = MakeGUID(guid);

		FOURCC service_type = MK4CC(type[0], type[1], type[2], type[3]);

		HANDLE manifest = CreateFileA(filename, GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
		if (manifest != INVALID_HANDLE_VALUE)
		{
			SetFilePointer(manifest, 0, 0, FILE_END);
			Add(manifest, service_guid, service_type, name, test_string);
			CloseHandle(manifest);
		}
	}
	else if (!_stricmp(command, "auto") && argc >= 4)
	{
		const char *w5s_filename = argv[3];
		HANDLE manifest = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		if (manifest != INVALID_HANDLE_VALUE)
		{
			WbmSvcMgr svcmgr(manifest);
			ifc_wa5component *comp = LoadWasabiComponent(w5s_filename, &svcmgr);
			if (comp)
			{
				UnloadWasabiComponent(comp, &svcmgr);
			}
			else
				printf("failed to load %s\n", w5s_filename);
			CloseHandle(manifest);
		}
		else
			printf("failed to create %s\n", filename);

	}
	return 0;
}