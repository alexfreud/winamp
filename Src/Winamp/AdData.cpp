#include "main.h"
#include "AdData.h"

ad_data::ad_data()
:strCurtain(0), cbCurtain(0)/*, strBrowser(0), cbBrowser(0)*/
{
}
 
ad_data::~ad_data()
{
	delete[] strCurtain;  cbCurtain=0;
	/*delete[] strBrowser;  cbBrowser=0;*/
}
