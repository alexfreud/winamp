#include "GammaManagerAPI.h"
#include "api.h"
#include <api/syscb/callbacks/skincb.h>

GammaManagerAPI::GammaManagerAPI()
{
	curSetSel = NULL;
	inTransaction=0;
}

void GammaManagerAPI::StartTransaction()
{
	inTransaction++;
}

	void GammaManagerAPI::EndTransaction()
	{
		inTransaction--;
		if (inTransaction == 0)
			WASABI_API_SYSCB->syscb_issueCallback(SysCallback::SKINCB, SkinCallback::COLORTHEMESLISTCHANGED);
	}

size_t GammaManagerAPI::getNumGammaSets()
{
	return gammasets.size();
}

const wchar_t *GammaManagerAPI::enumGammaSet(size_t n)
{
	return gammasets[n]->name;
}

int GammaManagerAPI::getNumGammaGroups(const wchar_t *gammaset)
{
	if (gammaset == NULL || !*gammaset) return 0;
	for (size_t i = 0;i != gammasets.size();i++)
	{
		if (IsKeyword(gammasets[i]->name, gammaset))
			return (int)gammasets[i]->gammagroups.size();

	}
	return -1;
}

const wchar_t *GammaManagerAPI::enumGammaGroup(const wchar_t *gammaset, int n)
{
	if (gammaset == NULL || !*gammaset) return 0;
	for (size_t i = 0;i != gammasets.size();i++)
	{
		if (IsKeyword(gammaset, gammasets[i]->name))
		{
			if (n < 0 || n >= (int)gammasets[i]->gammagroups.size())
			{
				return gammasets[i]->generalgroup.getName();
			}
			const wchar_t * ret = gammasets[i]->gammagroups[n]->getName();
			return ret;
		}
	}
	return NULL;
}

ColorThemeGroup *GammaManagerAPI::enumColorThemeGroup(int colorset, int colorgroup)
{
	GammaSet *set = gammasets[colorset];
	if (set != NULL)
	{
		if (colorgroup < 0 || colorgroup >= (int)set->gammagroups.size())
		{
			return &set->generalgroup;
		}
		ColorThemeGroup *grp = set->gammagroups[colorgroup];
		return grp;
	}
	return NULL;
}

ColorThemeGroup *GammaManagerAPI::getColorThemeGroup(const wchar_t *colorset, const wchar_t *colorgroup)
{
	for (size_t i = 0;i < gammasets.size();i++)
	{
		GammaSet *s = gammasets[i];
		if (IsKeyword(s->name, colorset))
		{
			if (IsKeyword(s->generalgroup.getName(), colorgroup))
			{
				ColorThemeGroupI *g = &s->generalgroup;
				return g;
			}
			for (size_t j = 0;j < s->gammagroups.size();j++)
			{
				ColorThemeGroupI *g = s->gammagroups[j];
				if (IsKeyword(g->getName(), colorgroup))
				{
					return g;
				}
			}
		}
	}
	return NULL;
}

void GammaManagerAPI::deleteAllGammaSets()
{
	//gammasets.deleteAll();
	for (auto obj : gammasets)
	{
		delete obj;
	}
	gammasets.clear();

	curSetSel = NULL;
}

void GammaManagerAPI::deleteGammaSet(const wchar_t *set)
{
	for (size_t i = 0;i < gammasets.size();i++)
	{
		if (IsKeyword(gammasets[i]->name, set))
		{
			GammaSet *s = gammasets[i];
			delete s;
			gammasets.erase(gammasets.begin() + i);
			i--;
		}
	}
	if (!inTransaction)
		WASABI_API_SYSCB->syscb_issueCallback(SysCallback::SKINCB, SkinCallback::COLORTHEMESLISTCHANGED);
}

void GammaManagerAPI::renameGammaSet( const wchar_t *set, const wchar_t *newname )
{
	for ( GammaSet *l_gammaset : gammasets )
	{
		if ( IsKeyword( l_gammaset->name, set ) )
		{
			l_gammaset->SetName( newname );
			if ( !inTransaction )
				WASABI_API_SYSCB->syscb_issueCallback( SysCallback::SKINCB, SkinCallback::COLORTHEMESLISTCHANGED );

			return;
		}
	}
}

size_t GammaManagerAPI::newGammaSet(const wchar_t *set)
{
	for (size_t i = 0;i != gammasets.size();i++)
	{
		if (IsKeyword(gammasets[i]->name, set))
			return i;
	}

	GammaSet *curset = new GammaSet(set);/*
	for (size_t i = 0;i < gammasets.size();i++)
	{
		for (size_t j = 0;j < gammasets[i]->gammagroups.size();j++)
		{
			ColorThemeGroup * g = gammasets[i]->gammagroups[j];
			if (curset->haveGroup(g->getName()))
				continue;
			curset->gammagroups.push_back(new ColorThemeGroupI(g->getName(), 0, 0, 0, 0, 0));
		}
	}*/
	size_t index = gammasets.size();
	gammasets.push_back(curset); //Martin> better add set after the loop than before
	if (!inTransaction)
		WASABI_API_SYSCB->syscb_issueCallback(SysCallback::SKINCB, SkinCallback::COLORTHEMESLISTCHANGED);
	return index;
}

void GammaManagerAPI::updateGammaSet(const wchar_t *set)
{
	for (size_t i = 0;i < gammasets.size();i++)
	{
		GammaSet *s = gammasets[i];
		if (IsKeyword(s->name, set))
		{
			for (size_t i = 0;i < gammasets.size();i++)
			{
				for (size_t j = 0;j < gammasets[i]->gammagroups.size();j++)
				{
					ColorThemeGroup * g = gammasets[i]->gammagroups[j];
					if (s->haveGroup(g->getName())) continue;
					s->gammagroups.push_back(new ColorThemeGroupI(g->getName(), 0, 0, 0, 0, 0));
				}
			}
		}
	}
}

void GammaManagerAPI::setGammaSet(const wchar_t *set)
{
	size_t i;
	for (i = 0;i != gammasets.size();i++)
	{
		if (IsKeyword(gammasets[i]->name, set))
		{
			setGammaSetInternal(gammasets[i]);
			return ;
		}
	}

	if (i) // i will still be 0 if getNumItems() was 0
		setGammaSetInternal(gammasets[0]);
}

void GammaManagerAPI::setGammaSetInternal(GammaSet *set)
{
	//if (curSetSel == set) return;
	curSetSel = set;

	//PORT
#ifdef WIN32
	SetCursor(LoadCursor(NULL, IDC_WAIT));
#endif
#ifdef LINUX
	XDefineCursor( Linux::getDisplay(), WASABI_API_WND->main_getRootWnd()->gethWnd(),
		XCreateFontCursor( Linux::getDisplay(), XC_watch ) );
#endif

	if (WASABI_API_SKIN)
		WASABI_API_SKIN->reapplySkinFilters();
	//PORT
#ifdef WIN32
	SetCursor(LoadCursor(NULL, IDC_ARROW));
#endif
#ifdef LINUX
	XUndefineCursor( Linux::getDisplay(), WASABI_API_WND->main_getRootWnd()->gethWnd() );
#endif
	WASABI_API_SYSCB->syscb_issueCallback(SysCallback::SKINCB, SkinCallback::COLORTHEMECHANGED);
}

int GammaManagerAPI::getGammaForGroup(const wchar_t *group, int *r, int *g, int *b, int *gray, int *boost)
{
	if (curSetSel)
	{
		size_t l = curSetSel->gammagroups.size();
		for (size_t i = 0;i < l;i++)
		{
			if (group && IsKeyword(group, curSetSel->gammagroups[i]->getName()))
			{
				*r = min(4095, max( -4095, curSetSel->gammagroups[i]->getRed() + curSetSel->generalgroup.getRed()));
				*g = min(4095, max( -4095, curSetSel->gammagroups[i]->getGreen() + curSetSel->generalgroup.getGreen()));
				*b = min(4095, max( -4095, curSetSel->gammagroups[i]->getBlue() + curSetSel->generalgroup.getBlue()));
				*gray = curSetSel->gammagroups[i]->getGray() | curSetSel->generalgroup.getGray();
				*boost = curSetSel->gammagroups[i]->getBoost() || curSetSel->generalgroup.getBoost();
				return 1;
			}
		}
		*r = min(4095, max( -4095, curSetSel->generalgroup.getRed()));
		*g = min(4095, max( -4095, curSetSel->generalgroup.getGreen()));
		*b = min(4095, max( -4095, curSetSel->generalgroup.getBlue()));
		*gray = curSetSel->generalgroup.getGray();
		*boost = curSetSel->generalgroup.getBoost();
		return 1;
	}
	return 0;
}

void GammaManagerAPI::resetGammaSet( const wchar_t *set )
{
	for ( GammaSet *l_gammaset : gammasets )
	{
		if ( IsKeyword( l_gammaset->name, set ) )
		{
			//gammasets[i]->gammagroups.deleteAll();
			for ( ColorThemeGroupI *obj : l_gammaset->gammagroups )
				delete obj;

			l_gammaset->gammagroups.clear();

			break;
		}
	}
}

void GammaManagerAPI::addGammaGroup( const wchar_t *set, ColorThemeGroup *group )
{
	for ( GammaSet *l_gammaset : gammasets )
	{
		if ( IsKeyword( l_gammaset->name, set ) )
		{
			for ( size_t j = 0; j < l_gammaset->gammagroups.size(); j++ )
			{
				ColorThemeGroupI *g = l_gammaset->gammagroups[ j ];
				if ( IsKeyword( g->getName(), group->getName() ) )
				{
					l_gammaset->gammagroups[ j ] = new ColorThemeGroupI( *group );
					delete g;

					return;
				}
			}

			l_gammaset->gammagroups.push_back( new ColorThemeGroupI( *group ) );
			break;
		}
	}
}

void GammaManagerAPI::addGammaGroup2(size_t i, ColorThemeGroup *group)
{
	for (size_t j = 0;j < gammasets[i]->gammagroups.size();j++)
	{
		ColorThemeGroupI * g = gammasets[i]->gammagroups[j];
		if (IsKeyword(g->getName(), group->getName()))
		{
			gammasets[i]->gammagroups[j] = new ColorThemeGroupI(*group);
			delete g;
			return;
		}
	}
	gammasets[i]->gammagroups.push_back(new ColorThemeGroupI(*group));
}

const wchar_t *GammaManagerAPI::getGammaSet()
{
	return curSetSel ? curSetSel->name : NULL; 
}

#define CBCLASS ColorThemeGroupI
START_DISPATCH;
CB(COLORTHEMEGROUPGETNAME, getName);
CB(COLORTHEMEGROUPGETRED, getRed);
CB(COLORTHEMEGROUPGETGREEN, getGreen);
CB(COLORTHEMEGROUPGETBLUE, getBlue);
CB(COLORTHEMEGROUPGETGRAY, getGray);
CB(COLORTHEMEGROUPGETBOOST, getBoost);
VCB(COLORTHEMEGROUPSETNAME, setName);
VCB(COLORTHEMEGROUPSETRED, setRed);
VCB(COLORTHEMEGROUPSETGREEN, setGreen);
VCB(COLORTHEMEGROUPSETBLUE, setBlue);
VCB(COLORTHEMEGROUPSETGRAY, setGray);
VCB(COLORTHEMEGROUPSETBOOST, setBoost);
END_DISPATCH;
#undef CBCLASS

#define CBCLASS GammaManagerAPI
START_DISPATCH;
CB(API_COLORTHEMES_GETNUMGAMMASETS, getNumGammaSets);
CB(API_COLORTHEMES_ENUMGAMMASET, enumGammaSet);
VCB(API_COLORTHEMES_DELETEGAMMASET, deleteGammaSet);
VCB(API_COLORTHEMES_DELETEALLGAMMASETS, deleteAllGammaSets);
VCB(API_COLORTHEMES_RESETGAMMASET, resetGammaSet);
VCB(API_COLORTHEMES_RENAMEGAMMASET, renameGammaSet);
CB(API_COLORTHEMES_NEWGAMMASET, newGammaSet);
VCB(API_COLORTHEMES_UPDATEGAMMASET, updateGammaSet);
CB(API_COLORTHEMES_GETNUMGAMMAGROUPS, getNumGammaGroups);
CB(API_COLORTHEMES_ENUMGAMMAGROUP, enumGammaGroup);
CB(API_COLORTHEMES_ENUMCOLORTHEMEGROUP, enumColorThemeGroup);
CB(API_COLORTHEMES_GETCOLORTHEMEGROUP, getColorThemeGroup);
CB(API_COLORTHEMES_GETGAMMAFORGROUP, getGammaForGroup);
VCB(API_COLORTHEMES_ADDGAMMAGROUP, addGammaGroup);
VCB(API_COLORTHEMES_ADDGAMMAGROUP2, addGammaGroup2);
CB(API_COLORTHEMES_GETGAMMASET, getGammaSet);
VCB(API_COLORTHEMES_SETGAMMASET, setGammaSet);
VCB(API_COLORTHEMES_STARTTRANSACTION, StartTransaction);
VCB(API_COLORTHEMES_ENDTRANSACTION, EndTransaction);
END_DISPATCH;
#undef CBCLASS