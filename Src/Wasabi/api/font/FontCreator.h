#include <api/service/svcs/svc_font.h>
template <class T>
class FontCreator : public waServiceFactoryT<svc_font, T> 
{
public:
	FontCreator(GUID myGuid = INVALID_GUID) : waServiceFactoryT<svc_font, T>(myGuid) {}
};
