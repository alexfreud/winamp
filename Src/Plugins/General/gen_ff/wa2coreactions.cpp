#include <precomp.h>
#include "wa2frontend.h"
#include "wa2core.h"
#include "wa2coreactions.h"
#include "main.h"
#include <api/core/api_core.h>
#include <api/locales/xlatstr.h>

//-----------------------------------------------------------------------------------------------
CoreActions::CoreActions()
{
	registerAction( L"prev",              ACTION_PREV );
	registerAction( L"play",              ACTION_PLAY );
	registerAction( L"pause",             ACTION_PAUSE );
	registerAction( L"stop",              ACTION_STOP );
	registerAction( L"next",              ACTION_NEXT );
	registerAction( L"eject",             ACTION_EJECT );
	registerAction( L"eject_url",         ACTION_EJECTURL );
	registerAction( L"eject_dir",         ACTION_EJECTDIR );
	registerAction( L"seek",              ACTION_SEEK );
	registerAction( L"volume",            ACTION_VOLUME );
	registerAction( L"pan",               ACTION_PAN );
	registerAction( L"volume_up",         ACTION_VOLUME_UP );
	registerAction( L"volume_down",       ACTION_VOLUME_DOWN );
	registerAction( L"rewind_5s",         ACTION_REWIND_5S );
	registerAction( L"ffwd_5s",           ACTION_FFWD_5S );
	registerAction( L"toggle_repeat",     ACTION_TOGGLE_REPEAT );
	registerAction( L"toggle_shuffle",    ACTION_TOGGLE_SHUFFLE );
	registerAction( L"toggle_crossfader", ACTION_TOGGLE_CROSSFADER );
	registerAction( L"mute",              ACTION_MUTE );
	registerAction( L"eq_toggle",         ACTION_EQ_TOGGLE );
	registerAction( L"eq_preamp",         ACTION_EQ_PREAMP );
	registerAction( L"eq_band",           ACTION_EQ_BAND );
	registerAction( L"eq_auto",           ACTION_EQ_AUTO );
	registerAction( L"eq_reset",          ACTION_EQ_RESET );
	// these were duped?
	// yes, for wa3alpha skins compatibility
	registerAction( L"toggle_repeat",     ACTION_TOGGLE_REPEAT );
	registerAction( L"toggle_shuffle",    ACTION_TOGGLE_SHUFFLE );
	registerAction( L"toggle_crossfader", ACTION_TOGGLE_CROSSFADER );
	for ( int i = 0; i < 4; i++ )
		registerAction( StringPrintfW( L"play_cd%d", i + 1 ), ACTION_PLAY_CD + i );
	// Martin> Moved from menuaction.cpp
	registerAction( L"WA5:Prefs", ACTION_PREFS );
}

//-----------------------------------------------------------------------------------------------
CoreActions::~CoreActions()
{}


//-----------------------------------------------------------------------------------------------
int CoreActions::onActionId(int pvtid, const wchar_t *action, const wchar_t *param /* =NULL */, int p1 /* =0 */, int p2 /* =0 */, void *data /* =NULL */, int datalen /* =0 */, ifc_window *source /* =NULL */)
{
	if (g_Core == NULL) return 0;
	int a = WTOI(param);
	switch (pvtid)
	{
	case ACTION_PREV:
		if (Std::keyDown(VK_SHIFT))
		{
			wa2.rewind5s();
		}
		else if (Std::keyDown(VK_CONTROL))
		{
			wa2.startOfPlaylist();
		}
		else
		{
			if (a)
				wa2.previousPopupMenu();
			else
				g_Core->userButton(UserButton::PREV);
		}
		break;
	case ACTION_PLAY:
		if (Std::keyDown(VK_SHIFT))
		{
			wa2.openFileDialog(source ? source->gethWnd() : WASABI_API_WND->main_getRootWnd()->gethWnd());
		}
		else if (Std::keyDown(VK_CONTROL))
		{
			wa2.openUrlDialog(source ? source->gethWnd() : WASABI_API_WND->main_getRootWnd()->gethWnd());
		}
		else
		{
			if (a)
				wa2.playPopupMenu();
			else
				g_Core->userButton(UserButton::PLAY);
		}
		break;
	case ACTION_PAUSE:
		if (a)
			wa2.pausePopupMenu();
		else
			g_Core->userButton(UserButton::PAUSE);
		break;
	case ACTION_STOP:
		if (Std::keyDown(VK_SHIFT))
		{
			wa2.stopWithFade();
		}
		else if (Std::keyDown(VK_CONTROL))
		{
			wa2.stopAfterCurrent();
		}
		else
		{
			if (a)
				wa2.stopPopupMenu();
			else
				g_Core->userButton(UserButton::STOP);
		}
		break;
	case ACTION_NEXT:
		if (Std::keyDown(VK_SHIFT))
		{
			wa2.forward5s();
		}
		else if (Std::keyDown(VK_CONTROL))
		{
			wa2.endOfPlaylist();
		}
		else
		{
			if (a)
				wa2.nextPopupMenu();
			else	if (a == 0)
				g_Core->userButton(UserButton::NEXT);
		}
		break;
	case ACTION_EJECT:
		if (Std::keyDown(VK_SHIFT))
		{
			wa2.openDirectoryDialog(source ? source->gethWnd() : WASABI_API_WND->main_getRootWnd()->gethWnd());
		}
		else if (Std::keyDown(VK_CONTROL))
		{
			wa2.openUrlDialog(source ? source->gethWnd() : WASABI_API_WND->main_getRootWnd()->gethWnd());
		}
		else
		{
			if (a)
				wa2.ejectPopupMenu();
			else
				wa2.openFileDialog(source ? source->gethWnd() : wa2.getMainWindow());
		}
		break;
	case ACTION_EJECTURL:
		if (a == 0)
		{
			wa2.openUrlDialog(source ? source->gethWnd() : wa2.getMainWindow()); break;
		}
		else return 0;
	case ACTION_EJECTDIR:
		if (a == 0)
		{
			wa2.openDirectoryDialog(source ? source->gethWnd() : wa2.getMainWindow()); break;
		}
		else return 0;
	case ACTION_VOLUME_UP:
		if (a == 0)
		{
			g_Core->setVolume(MIN(g_Core->getVolume() + 5, 255)); break;
		}
		else return 0;
	case ACTION_VOLUME_DOWN:
		if (a == 0)
		{
			g_Core->setVolume(MIN(g_Core->getVolume() - 5, 255)); break;
		}
		else return 0;
	case ACTION_REWIND_5S:
		if (a == 0)
		{
			g_Core->setPosition(MAX(g_Core->getPosition() - 5000, 0)); break;
		}
		else return 0;
	case ACTION_FFWD_5S:
		if (a == 0)
		{
			g_Core->setPosition(MIN(g_Core->getPosition() + 5000, g_Core->getLength())); break;
		}
		else return 0;
	case ACTION_EQ_AUTO:
		if (a == 0)
		{
			g_Core->setEQAuto(!g_Core->getEQAuto()); break;
		}
		else return 0;
	case ACTION_EQ_TOGGLE:
		if (a == 0)
		{
			g_Core->setEQStatus(!g_Core->getEQStatus()); break;
		}
		else return 0;
	case ACTION_EQ_RESET:
		if (a == 0)
		{
			{
				for (int i = 0;i < 10;i++) g_Core->setEqBand(i, 0);
			}
			break;
		}
		else return 0;
		//case ACTION_MUTE: g_Core->setMute(!g_Core->getMute()); break;
	case ACTION_TOGGLE_REPEAT:
		if (a == 0)
		{
			wa2.setRepeat(!wa2.getRepeat());
		}
		else return 0;
		break;
	case ACTION_TOGGLE_SHUFFLE:
		if (a == 0)
		{
			wa2.setShuffle(!wa2.getShuffle());
		}
		else
			return 0;
		break;
	case ACTION_TOGGLE_CROSSFADER:
		break; // todo
	// Martin> Moved from menuactions.cpp
	case ACTION_PREFS:
		{
			// check if param is set, otherwise we will get a crash
			if (param != NULL)
			{
				int prefsPage = _wtoi(param);
				SendMessageW(wa2.getMainWindow(),WM_WA_IPC,prefsPage,IPC_OPENPREFSTOPAGE);
			}
			break;
		}
	}
	if (pvtid >= ACTION_PLAY_CD && pvtid < ACTION_PLAY_CD + 16)
	{
		if (a == 0)
		{
			wa2.playAudioCD(pvtid - ACTION_PLAY_CD);
		}
		else return 0;
	}
	return 1;
}

const wchar_t *CoreActions::getHelp(int action)
{
	static StringW name;
	switch (action)
	{
		// TODO: benski> move to win32 resources and let them get translated via gen_ff.lng
	case ACTION_PREV: name = _(L"Previous track"); break;
	case ACTION_PAUSE: name = _(L"Pause/Resume playback"); break;
	case ACTION_STOP: name = _(L"Stop playback"); break;
	case ACTION_NEXT: name = _(L"Next track"); break;
	case ACTION_EJECT: name = _(L"Load file"); break;
	case ACTION_EJECTURL: name = _(L"Load URL"); break;
	case ACTION_EJECTDIR: name = _(L"Load directory"); break;
	case ACTION_SEEK: name = _(L"Seek"); break;
	case ACTION_VOLUME: name = _(L"Volume"); break;
	case ACTION_PAN: name = _(L"Panning"); break;
	case ACTION_EQ_TOGGLE: name = _(L"Toggle equalizer"); break;
	case ACTION_EQ_PREAMP: name = _(L"Toggle preamplifier"); break;
	case ACTION_EQ_BAND: name = _(L"Change equalizer band"); break;
	case ACTION_EQ_AUTO: name = _(L"Auto equalizer"); break;
	case ACTION_EQ_RESET: name = _(L"Reset equalizer"); break;
	case ACTION_VOLUME_UP: name = _(L"Volume up"); break;
	case ACTION_VOLUME_DOWN: name = _(L"Volume down"); break;
	case ACTION_REWIND_5S: name = _(L"Rewind 5 seconds"); break;
	case ACTION_FFWD_5S: name = _(L"Fast forward 5 seconds"); break;
	case ACTION_MUTE: name = _(L"Mute/Unmute sound"); break;
	case ACTION_TOGGLE_REPEAT: name = _(L"Toggle repeat"); break;
	case ACTION_TOGGLE_SHUFFLE: name = _(L"Toggle shuffle"); break;
	case ACTION_TOGGLE_CROSSFADER: name = _(L"Toggle crossfader"); break;
	}
	return name;
}
