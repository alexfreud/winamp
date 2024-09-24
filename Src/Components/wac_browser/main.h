#ifndef NULLSOFT_WAC_BROWSER_MAIN_H
#define NULLSOFT_WAC_BROWSER_MAIN_H

#include "api__wac_browser.h"

#include "../Agave/Component/ifc_wa5component.h"

#include <QtWebView>
#include <QUrl>

namespace wa
{
	namespace Components
	{
		class WAC_Browser : public ifc_wa5component
		{
		public:
			void RegisterServices( api_service *p_service );
			int  RegisterServicesSafeModeOk()                         { return 1; }

			void DeregisterServices( api_service *p_service );

		protected:
			RECVS_DISPATCH;
		};
	}
}

#endif // !NULLSOFT_WAC_BROWSER_MAIN_H