#ifndef _ENCODEDSTR_H
#define _ENCODEDSTR_H

#include <api/service/svcs/svc_stringtypes.h>

class String;

class EncodedStr {
public:
  // The EncodedStr object will automatically delete its ram, unless you 
  // specify 0 in that "delete it" parameter there, partner.
  EncodedStr(FOURCC encodingType = 0, void *encodedBuffer = NULL, int bufferSize = 0, int deleteIt = 1);
  ~EncodedStr();

  // A "reset" will ensure any previously set buffer will be deleted
  void resetBuffer(FOURCC encodingType, void *encodedBuffer, int bufferSize, int deleteIt = 1);

  // All the calls to the service level functions are through here.
  int convertToUTF8(String &output_str);
  // This method will reset this object (ie: delete RAM if necessary)
  int convertFromUTF8(FOURCC encodingType, const String &inputStr);

  // Accessor inlines
  inline FOURCC getEncodingType() { return encoding_type; }
  inline void *getEncodedBuffer() { return encoded_buffer; }
  inline int getBufferSize() { return buffer_size; }
  inline int getDeleteIt() { return delete_it; }

// This is for debugging.
  int operator ==(const EncodedStr &in_string);

private:
  FOURCC encoding_type;
  void * encoded_buffer;
  int buffer_size;
  int delete_it;
};


#endif//_ENCODEDSTR_H


