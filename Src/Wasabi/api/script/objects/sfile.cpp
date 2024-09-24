#include <precomp.h>
#include "SFile.h"

#include <api/script/script.h>
#include <api/script/scriptmgr.h>
#include <api/script/objecttable.h>


// {836F8B2E-E0D1-4db4-937F-0D0A04C8DCD1}
static const GUID fileGuid = 
{ 0x836f8b2e, 0xe0d1, 0x4db4, { 0x93, 0x7f, 0xd, 0xa, 0x4, 0xc8, 0xdc, 0xd1 } };


fileScriptController _fileController;
fileScriptController *fileController=&_fileController;

// -- Functions table -------------------------------------
function_descriptor_struct fileScriptController::exportedFunction[] = {
	{L"load",			1, (void*)SFile::script_vcpu_load },
	{L"getSize",		0, (void*)SFile::script_vcpu_getSize },
	{L"exists",			0, (void*)SFile::script_vcpu_exists },
};
// --------------------------------------------------------

const wchar_t *fileScriptController::getClassName() {
	return L"File";
}

const wchar_t *fileScriptController::getAncestorClassName() {
  return L"Object";
}

ScriptObjectController *fileScriptController::getAncestorController() { return rootScriptObjectController; }

ScriptObject *fileScriptController::instantiate() {
  SFile *xd = new SFile;
  ASSERT(xd != NULL);
  return xd->getScriptObject();
}

void fileScriptController::destroy(ScriptObject *o) {
  SFile *xd = static_cast<SFile *>(o->vcpu_getInterface(fileGuid));
  ASSERT(xd != NULL);
  delete xd;
}

void *fileScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for files for now
}

void fileScriptController::deencapsulate(void *o) {
}

int fileScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *fileScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID fileScriptController::getClassGuid() {
	return fileGuid;
}



SFile::SFile() {
  getScriptObject()->vcpu_setInterface(fileGuid, (void *)static_cast<SFile *>(this));
  getScriptObject()->vcpu_setClassName(L"File");
  getScriptObject()->vcpu_setController(fileController);
  filename = NULL;
}

SFile::~SFile() {
  //if (bmp) delete bmp;
}

void SFile::loadfile(const wchar_t *b)
{
  filename = b;
}

// VCPU

scriptVar SFile::script_vcpu_load(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar fn) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(fn.type == SCRIPT_STRING);
  SFile *m = static_cast<SFile *>(o->vcpu_getInterface(fileGuid));
  if (m) m->loadfile(fn.data.sdata);
  RETURN_SCRIPT_VOID;  
}

scriptVar SFile::script_vcpu_getSize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SFile *m = static_cast<SFile *>(o->vcpu_getInterface(fileGuid));
	if (m)
	{
		OSFILETYPE in = WFOPEN(m->filename, WF_READONLY_BINARY);
		if (in == OPEN_FAILED) RETURN_SCRIPT_ZERO;
		int size = (int)FGETSIZE(in);
		FCLOSE(in);
		return MAKE_SCRIPT_INT(size);
	}
	RETURN_SCRIPT_ZERO;
}

scriptVar SFile::script_vcpu_exists(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SFile *m = static_cast<SFile *>(o->vcpu_getInterface(fileGuid));
	if (m)
	{
		OSFILETYPE in = WFOPEN(m->filename, WF_READONLY_BINARY);
		if (in == OPEN_FAILED) return MAKE_SCRIPT_BOOLEAN(0);
		FCLOSE(in);
		return MAKE_SCRIPT_BOOLEAN(1);
	}
	return MAKE_SCRIPT_BOOLEAN(0);
}