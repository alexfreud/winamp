#pragma once

#if defined(_WIN64)
 #include "IFileTypeRegistrar_64.h"
#else
 #include "IFileTypeRegistrar_32.h"
#endif

class FileTypeRegistrar : public IFileTypeRegistrar
{
public:
	FileTypeRegistrar();
	/* IUnknown */
	HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid, 
		/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject
		);
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);

	/* Stuff we define */
	HRESULT STDMETHODCALLTYPE RegisterMIMEType(
		/* [in, string] */ LPCWSTR mimeType, 
		/* [in, string] */ LPCWSTR programName, 
		/* [in, string] */ LPCWSTR extension, 
		BOOL netscapeOnly
		);

	HRESULT STDMETHODCALLTYPE RegisterCDPlayer(
		/* [in, string] */ LPCWSTR programName
		);

	HRESULT STDMETHODCALLTYPE UnregisterCDPlayer(
		/* [in, string] */ LPCWSTR programName
		);

	HRESULT STDMETHODCALLTYPE RegisterType(
		/* [in, string] */ LPCWSTR extension,
		/* [in, string] */ LPCWSTR which_str, 
		/* [in, string] */ LPCWSTR prog_name
		);

	HRESULT STDMETHODCALLTYPE UnregisterType(
		/* [in, string] */ LPCWSTR extension, 
		/* [in, string] */ LPCWSTR which_str, 
		/* [in, string] */ LPCWSTR prog_name,
		/*[in]*/ int iconNumber
		);

	HRESULT STDMETHODCALLTYPE AddDirectoryContext(
		/* [in, string] */ LPCWSTR programName, 
		/* [in, string] */ LPCWSTR which_str, 
		/* [in, string] */ LPCWSTR description
		);

	HRESULT STDMETHODCALLTYPE RemoveDirectoryContext(
		/* [in, string] */ LPCWSTR which_str
		);

	HRESULT STDMETHODCALLTYPE AddAgent(
		/* [in, string] */ LPCWSTR agentFilename
		);

	HRESULT STDMETHODCALLTYPE RemoveAgent();

	HRESULT STDMETHODCALLTYPE RegisterMediaPlayer(
		DWORD accessEnabled, 
		/* [in, string] */ LPCWSTR programName, 
		/* [in, string] */ LPCWSTR prog_name, 
		int iconNumber
		);

	HRESULT STDMETHODCALLTYPE RegisterMediaPlayerProtocol(LPCWSTR protocol, LPCWSTR prog_name);
	HRESULT STDMETHODCALLTYPE UnregisterMediaPlayerProtocol(LPCWSTR protocol, LPCWSTR prog_name);

	HRESULT STDMETHODCALLTYPE SetupFileType(
		/* [in, string] */ LPCWSTR programName, 
		/* [in, string] */ LPCWSTR winamp_file, 
		/* [in, string] */ LPCWSTR name, 
		int iconNumber, 
		/* [in, string] */ LPCWSTR defaultShellCommand,
		/* [in, string] */ LPCWSTR iconPath
		);

	HRESULT STDMETHODCALLTYPE SetupShell(
		/* [in, string] */ LPCWSTR commandLine, 
		/* [in, string] */ LPCWSTR winamp_file, 
		/* [in, string] */ LPCWSTR description, 
		/* [in, string] */ LPCWSTR commandName,
		/* [in, string] */ LPCWSTR dragAndDropGUID
		);

	HRESULT STDMETHODCALLTYPE RemoveShell(
		/* [in, string] */ LPCWSTR winamp_file,
		/* [in, string] */ LPCWSTR commandName
		);

	HRESULT STDMETHODCALLTYPE SetupDefaultFileType(
		/* [in, string] */ LPCWSTR winamp_file,
		/* [in, string] */ LPCWSTR defaultShellCommand
		);

	HRESULT STDMETHODCALLTYPE RegisterTypeShell(
		/* [in, string] */ LPCWSTR programName, 
		/* [in, string] */ LPCWSTR which_file, 
		/* [in, string] */ LPCWSTR description, 
		int iconNumber,
		/* [in, string] */ LPCWSTR commandName
		);

	HRESULT STDMETHODCALLTYPE RegisterGUID(
		/* [in, string] */ LPCWSTR programName, 
		/* [in, string] */ LPCWSTR guidString
		);

	HRESULT STDMETHODCALLTYPE RegisterDVDPlayer(
		/* [in, string] */ LPCWSTR programName,
		int iconNumber,
		/* [in, string] */ LPCWSTR which_file,
		/* [in, string] */ LPCWSTR commandName,
		/* [in, string] */ LPCWSTR provider, 
		/* [in, string] */ LPCWSTR description
		);

	HRESULT STDMETHODCALLTYPE InstallItem(
		LPCWSTR sourceFile,
		LPCWSTR destinationFolder,
		LPCWSTR destinationFilename
		);

	HRESULT STDMETHODCALLTYPE DeleteItem(
		LPCWSTR file
		);

	HRESULT STDMETHODCALLTYPE RenameItem(
		LPCWSTR oldFile,
		LPCWSTR newFile,
		BOOL force
		);

	HRESULT STDMETHODCALLTYPE CleanupDirectory(
		LPCWSTR directory
		);

	HRESULT STDMETHODCALLTYPE MoveDirectoryContents(
		LPCWSTR oldDirectory,
		LPCWSTR newDirectory
		);

	HRESULT STDMETHODCALLTYPE WriteProKey(LPCWSTR name, LPCWSTR key);
	HRESULT STDMETHODCALLTYPE WriteClientUIDKey(LPCWSTR path, LPCWSTR uid_str);

	HRESULT STDMETHODCALLTYPE RegisterProtocol(LPCWSTR protocol, LPCWSTR command, LPCWSTR icon);
	HRESULT STDMETHODCALLTYPE RegisterCapability(const wchar_t *programName, const wchar_t *winamp_file, const wchar_t *extension);

//private:
	volatile ULONG refCount;
};