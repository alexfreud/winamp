#ifndef _SVC_STRINGCONVERTER_H
#define _SVC_STRINGCONVERTER_H

#include <bfc/dispatch.h>
#include <api/service/services.h>
#include <api/service/svcs/svc_stringtypes.h>

// Porting Rule: A new service to respond to at least OSNATIVE and UTF16 shall be
// provided for every new platform.  This service should provide transcoding
// to and from the platform's native internationalized string encoding format
// (ie: MBCS under Win9x, WorldScript on the Mac, etc etc etc).and to and from
// UTF16.  If the OSNATIVE string _IS_ UTF16, just respond that you can convert
// both.

class NOVTABLE svc_stringConverter : public Dispatchable {
public:
  static FOURCC getServiceType() { return WaSvc::STRINGCONVERTER; }

  // test the type.
  int canConvert(FOURCC encoding_type);

  // The return code is the number of bytes written to the output buffer, or the error.
  // A return code of 0 is not an error, other than you passing NULL or a size 0 buffer won't do much interesting.
  int convertToUTF8(FOURCC encoding_type, const void *in_buffer, int size_in_bytes, char *out_buffer, int size_out_bytes);
  int preflightToUTF8(FOURCC encoding_type, const void *in_buffer, int size_in_bytes);
  int convertFromUTF8(FOURCC encoding_type, const char *in_buffer, int size_in_bytes, void *out_buffer, int size_out_bytes);
  int preflightFromUTF8(FOURCC encoding_type, const char *in_buffer, int size_in_bytes);

protected:
  enum {
    CANCONVERT,
    CONVERTTOUTF8,
    PREFLIGHTTOUTF8,
    CONVERTFROMUTF8,
    PREFLIGHTFROMUTF8
  };
};


inline
int svc_stringConverter::canConvert(FOURCC encoding_type) {
  return _call(CANCONVERT, (int)0, encoding_type);
}

inline
int svc_stringConverter::convertToUTF8(FOURCC encoding_type, const void *in_buffer, int size_in_bytes, char *out_buffer, int size_out_bytes) {
  return _call(CONVERTTOUTF8, (int)0, encoding_type, in_buffer, size_in_bytes, out_buffer, size_out_bytes);
}

inline
int svc_stringConverter::preflightToUTF8(FOURCC encoding_type, const void *in_buffer, int size_in_bytes) {
  return _call(PREFLIGHTTOUTF8, (int)0, encoding_type, in_buffer, size_in_bytes);
}

inline
int svc_stringConverter::convertFromUTF8(FOURCC encoding_type, const char *in_buffer, int size_in_bytes, void *out_buffer, int size_out_bytes) {
  return _call(CONVERTFROMUTF8, (int)0, encoding_type, in_buffer, size_in_bytes, out_buffer, size_out_bytes);
}

inline
int svc_stringConverter::preflightFromUTF8(FOURCC encoding_type, const char *in_buffer, int size_in_bytes) {
  return _call(PREFLIGHTFROMUTF8, (int)0, encoding_type, in_buffer, size_in_bytes);
}

// implementor derives from this one
class NOVTABLE svc_stringConverterI : public svc_stringConverter {
public:

  // test the type
  virtual int canConvert(FOURCC encoding_type) = 0;

  virtual int convertToUTF8(FOURCC encoding_type, const void *in_buffer, int size_in_bytes, char *out_buffer, int size_out_bytes) = 0;
  virtual int preflightToUTF8(FOURCC encoding_type, const void *in_buffer, int size_in_bytes) = 0;
  virtual int convertFromUTF8(FOURCC encoding_type, const char *in_buffer, int size_in_bytes, void *out_buffer, int size_out_bytes) = 0;
  virtual int preflightFromUTF8(FOURCC encoding_type, const char *in_buffer, int size_in_bytes) = 0;

protected:
  RECVS_DISPATCH;
};

#include <api/service/svc_enum.h>

class StringConverterEnum : public SvcEnumT<svc_stringConverter> {
public:
  StringConverterEnum(FOURCC enc_type) : encoding_type(enc_type) {}
protected:
  virtual int testService(svc_stringConverter *svc) {
    return (svc->canConvert(encoding_type));
  }
private:
  FOURCC encoding_type;
};

#endif // _SVC_STRINGCONVERTER_H
