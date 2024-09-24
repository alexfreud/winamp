#include "precomp_wasabi_bfc.h"
// someday, there will be a file here.
#include <bfc/wasabi_std.h>
#include "encodedstr.h"
#include <bfc/string/bfcstring.h>
#include <api/service/svcs/svc_stringconverter.h>
#include <api/memmgr/api_memmgr.h>
EncodedStr::EncodedStr(FOURCC encodingType, void *encodedBuffer, int bufferSize, int deleteIt) {
  encoding_type = encodingType;
  encoded_buffer = encodedBuffer;
  buffer_size = bufferSize;
  delete_it = deleteIt;
}

EncodedStr::~EncodedStr() {
  if (delete_it && (encoded_buffer != NULL)) {
#ifdef WASABI_COMPILE_MEMMGR
    WASABI_API_MEMMGR->sysFree(encoded_buffer);
#else
    free(encoded_buffer);
#endif
  }
}

void EncodedStr::resetBuffer(FOURCC encodingType, void *encodedBuffer, int bufferSize, int deleteIt) {
  // if there's someone already there, toss them.
  if (delete_it && (encoded_buffer != NULL)) {
#ifdef WASABI_COMPILE_MEMMGR
    WASABI_API_MEMMGR->sysFree(encoded_buffer);
#else
    free(encoded_buffer);
#endif
  }
  encoding_type = encodingType;
  encoded_buffer = encodedBuffer;
  buffer_size = bufferSize;
  delete_it = deleteIt;
}

int EncodedStr::convertToUTF8(String &output_str) {
  int retval = 0;
  StringConverterEnum myServiceEnum(encoding_type);
  svc_stringConverter *myConv = myServiceEnum.getFirst();
  if (myConv != NULL) {

    void *in_buffer = encoded_buffer;
    int size_in_bytes = buffer_size; 

    if (encoded_buffer != NULL) {
      // Preflight
      int u8size = myConv->preflightToUTF8(encoding_type, in_buffer, size_in_bytes);
      // Alloc
#ifdef WASABI_COMPILE_MEMMGR
      char *u8str = reinterpret_cast<char *>(WASABI_API_MEMMGR->sysMalloc(u8size));
#else
      char *u8str = reinterpret_cast<char *>(MALLOC(u8size));
#endif
      if (u8str != NULL) {
        // Convert
        retval = myConv->convertToUTF8(encoding_type, in_buffer, size_in_bytes, u8str, u8size);
        if (retval < 0) {
          // Clear on error.
#ifdef WASABI_COMPILE_MEMMGR
          WASABI_API_MEMMGR->sysFree(u8str);
#else
          free(encoded_buffer);
#endif
          u8str = NULL;
        }
      } else {
        ASSERTPR(u8str != NULL, "Malloc failed in string converter\n");
      }
      // And call the method to inject the pointer into our string (cleared on error).
      output_str.setValue(u8str);
    }

    // Once we use our service, release our locked instance of it.
    myServiceEnum.release(myConv);
  } else {
    // Clear the string on error.
    retval = SvcStrCnv::ERROR_UNAVAILABLE;
    output_str.setValue(NULL);
  }
  return retval;
}

int EncodedStr::convertFromUTF8(FOURCC encodingType, const String &inputStr) {
  int retval = 0;
  int written = 0;
  void *buffer = NULL;
  StringConverterEnum myServiceEnum(encodingType);
  svc_stringConverter *myConv = myServiceEnum.getFirst();

  if (myConv != NULL) {
    if (inputStr != NULL) {
      const char *val = inputStr.getValue();
      int valsize = STRLEN(val) + 1; // count the null char in your size-in-bytes!
      // Preflight
      int size = myConv->preflightFromUTF8(encodingType, val, valsize);
      if (size > 0) {
        // Alloc
#ifdef WASABI_COMPILE_MEMMGR
        buffer = WASABI_API_MEMMGR->sysMalloc(size);
#else
        buffer = MALLOC(size);
#endif
        if (buffer != NULL) {
          // Convert
          written = myConv->convertFromUTF8(encodingType, val, valsize, buffer, size);
          if (written > 0) {
            retval = written;
          } else {
            // Clear on error.
#ifdef WASABI_COMPILE_MEMMGR
            WASABI_API_MEMMGR->sysFree(buffer);
#else
            free(buffer);
#endif
            buffer = NULL;
            retval = written;
            written = 0;
          }
        } else {
          ASSERTPR(buffer != NULL, "Malloc failed in string converter\n");
        }
      } else {
        // Clear on error.
        buffer = NULL;
        retval = size;
        written = 0;
      }
    }
    // Once we use our service, release our locked instance of it.
    myServiceEnum.release(myConv);
  } else {
    // On error locking down a service, all the default values are errors and called through resetBuffer.
    retval = SvcStrCnv::ERROR_UNAVAILABLE;
  }
  resetBuffer(encodingType, buffer, written);
  return retval;
}

// This is for debugging.
int EncodedStr::operator ==(const EncodedStr &in_string) {
  if (encoding_type == in_string.encoding_type) {
    switch (encoding_type) {
      case SvcStrCnv::OSNATIVE:
        return (STRCMP(reinterpret_cast<char *>(encoded_buffer), reinterpret_cast<char *>(in_string.encoded_buffer)) == 0);
      break;
      default:
        return 0;
      break;
    }
  }
  return 0;
}
