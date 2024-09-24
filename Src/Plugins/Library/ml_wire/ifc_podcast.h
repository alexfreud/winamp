#ifndef NULLSOFT_ML_WIRE_IFC_PODCAST_H
#define NULLSOFT_ML_WIRE_IFC_PODCAST_H

#include <bfc/dispatch.h>
class ifc_podcast : public Dispatchable
{
protected:
	ifc_podcast() {}
	~ifc_podcast() {}
public:
	//int GetUrl(wchar_t *str, size_t len);
	int GetTitle(wchar_t *str, size_t len);
	//int GetLink(wchar_t *str, size_t len);
	//int GetDescription(wchar_t *str, size_t len);
	size_t GetNumArticles();
	// TODO: ifc_article *EnumArticle(size_t i);

	enum
	{
		IFC_PODCAST_GETURL = 0,
		IFC_PODCAST_GETTITLE = 1,
		IFC_PODCAST_GETLINK = 2,
		IFC_PODCAST_GETDESCRIPTION = 3,
		IFC_PODCAST_GETNUMARTICLES = 4,
	};
};

inline int ifc_podcast::GetTitle(wchar_t *str, size_t len)
{
	return _call(IFC_PODCAST_GETTITLE, (int)1, str, len);
}


inline size_t ifc_podcast::GetNumArticles()
{
	return _call(IFC_PODCAST_GETNUMARTICLES, (size_t)0);
}

#endif