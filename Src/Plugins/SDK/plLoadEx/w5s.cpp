#include "ExComponent.h" // the component we're registering is defined here

ExComponent exComponent; // our component

// Winamp GetProcAddress()'s this after loading your w5s file
extern "C" __declspec(dllexport) ifc_wa5component *GetWinamp5SystemComponent()
{
	return &exComponent;
}
