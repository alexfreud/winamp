#include <windows.h>
#include "c_encoder.h"

C_ENCODER::C_ENCODER(int ExtInfoSize) {
	SetName("Untyped Encoder");
	ExtendedInfoPtr = (T_EncoderIOVals *)malloc(ExtInfoSize);
	ExtendedInfoSize = ExtInfoSize;
}

C_ENCODER::~C_ENCODER() {
	Close();
	if(ExtendedInfoPtr && ExtendedInfoSize) free(ExtendedInfoPtr);
	ExtendedInfoSize = 0;
}

void C_ENCODER::Close() {
	ClearAttribs();
}

void C_ENCODER::SetName(const char *name) {
	if (name) lstrcpyn(Name, name, C_ENCODER_NameLen);
}

char *C_ENCODER::GetName() {
	return Name;
}

void C_ENCODER::Reset() {
}

void C_ENCODER::ChangeSettings(const void *Settings) {
	if(ExtendedInfoPtr && ExtendedInfoSize && Settings && Settings != ExtendedInfoPtr) {
		memcpy(ExtendedInfoPtr,Settings,ExtendedInfoSize);
		Reset();
	}
}

void C_ENCODER::Create(const T_EncoderIOVals *Settings, const char *name) {
	if(name) SetName(name);
	ChangeSettings(Settings);
}

void C_ENCODER::ClearAttribs() {
	for(int i = AttribList.size()-1; i >= 0; i--) {
	T_ATTRIB *myAttrib = AttribList[i];
		if(myAttrib->OutputVals) delete[] myAttrib->OutputVals;
	}
	
	//AttribList.deleteAll();
	for (auto attrib : AttribList)
	{
		delete attrib;
	}
	AttribList.clear();
}

void C_ENCODER::AddAttrib(const char *Text, const void *Attributes) {
	T_ATTRIB *myAttrib = new T_ATTRIB;
	if(Text!=NULL) {
		::strncpy((char *)&myAttrib->Text,Text,sizeof(myAttrib->Text));
	} else {
		::strncpy((char *)&myAttrib->Text,"<This should never appear here...>",sizeof(myAttrib->Text));
	}
	myAttrib->OutputVals = (T_EncoderIOVals *)Attributes;
	AttribList.push_back(myAttrib);
}

int C_ENCODER::Encode(const void *inputbuf, const unsigned int inputbufsize, void *outputbuf, const unsigned int outputbufsize, int *inputamtused) {
	if((inputbuf != NULL) && (outputbuf != NULL) && (inputbufsize != 0) && (outputbufsize != 0) && (inputamtused != NULL)) {
	int numitems = (inputbufsize > outputbufsize) ? outputbufsize : inputbufsize;
		memcpy(outputbuf,inputbuf,numitems);
		*inputamtused = numitems;
		return numitems;
	}
	return 0;
}

int C_ENCODER::GetNumAttribs() {
	return AttribList.size();
}

int C_ENCODER::EnumAttrib(const int val, T_ATTRIB *buf) {
	if((val < 0)||(val >= AttribList.size())||(buf==NULL)) return 0;
	T_ATTRIB *myattrib = AttribList[val];
	if(myattrib==NULL) return 0;
	::memcpy(buf,myattrib,sizeof(T_ATTRIB));
	return 1;
}

char * C_ENCODER::GetContentType() { 
	
	//if(strcmp(this->GetName(), "MP3 Encoder") == 0)
	return "audio/mpeg";

}