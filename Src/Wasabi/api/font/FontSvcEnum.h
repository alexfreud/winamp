#include <bfc/string/StringW.h>
#include <api/service/svc_enum.h>

class FontSvcEnum : public SvcEnumT<svc_font> {
public:
  FontSvcEnum(const wchar_t *_svc_name = NULL) : svc_name(_svc_name) {}
protected:
  virtual int testService(svc_font *svc) 
	{
    if (!svc_name.len())
			return 1; // blank name returns all services.
    return (!WCSICMP(svc->getFontSvcName(),svc_name));
  }
private:
  StringW svc_name;
};


