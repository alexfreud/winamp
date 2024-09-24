#pragma once
#include "foundation/dispatch.h"

class ifc_equalizer : public Wasabi2::Dispatchable
{
protected:
	ifc_equalizer() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_equalizer() {}

public:

	int SetPreamp(double dB) { return Equalizer_SetPreamp(dB); }
	int SetBand(unsigned int band, double dB) { return Equalizer_SetBand(band, dB); }
	int Enable() { return Equalizer_Enable(); }
	int Disable() { return Equalizer_Disable(); }
private:
	virtual int WASABICALL Equalizer_SetPreamp(double dB)=0;
	virtual int WASABICALL Equalizer_SetBand(unsigned int band, double dB)=0;
	virtual int WASABICALL Equalizer_Enable()=0;
	virtual int WASABICALL Equalizer_Disable()=0;

	enum
	{
		DISPATCHABLE_VERSION,
	};

};
