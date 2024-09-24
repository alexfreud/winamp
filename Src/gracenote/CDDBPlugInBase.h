#ifndef CDDB_PLUGIN_BASE_H
#define CDDB_PLUGIN_BASE_H

// Version of module interface
#define	CDDBMODULE_VERSION		1

// Module Categories
// modules need to set this approprately to tell the manager what services it provides
#define CDDBMODULE_DECODER		0x10
#define CDDBMODULE_DECODERINFO	0x20
#define CDDBMODULE_ENCODER		0x40
#define CDDBMODULE_SIGNATURE	0x80
#define CDDBMODULE_FILEINFO		0x100
#define CDDBMODULE_SECURITY		0x200

//
// base module type
// all modules derive from this type
//
#ifndef CDDBMODULEINTERFACE
#define CDDBMODULEINTERFACE
typedef struct
{
	void *handle;
	char *moduleID;
	int	version;
	int categories;
	int initialized;
	int	(__stdcall *Init)(void*);
	int (__stdcall *Deinit)();
} CDDBModuleInterface;
#endif

// entry point function type
typedef CDDBModuleInterface* (__cdecl *CDDBModuleQueryInterfaceFunc)(const char* lpszInterface);


//
// internal module handle
//
typedef struct
{
	void				*handle;
	int					initialized;
	CDDBModuleInterface *baseInterface;
} CDDBModule;



#endif /* CDDB_PLUGIN_BASE_H */
