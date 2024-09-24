#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifdef CONFIG_IMPL
#define CONFIG_EXT
#define DEF_VAL(x) =( x )
#else
#define CONFIG_EXT extern
#define DEF_VAL(x)
#endif   // CONFIG_IMPL

/* --- This lets us change the registry strings at compile-time, for special builds --- */

#ifndef WINAMP_FILE
#define REGISTRY_PREFIX "Winamp"
#endif

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)

#define WINAMP_FILE REGISTRY_PREFIX ".File"
#define WINAMP_FILEW WIDEN(REGISTRY_PREFIX) L".File"
#define WINAMP_PLAYLIST REGISTRY_PREFIX ".PlayList"
#define WINAMP_PLAYLISTW WIDEN(REGISTRY_PREFIX) L".PlayList"
#define WINAMP_SKINZIP REGISTRY_PREFIX ".SkinZip"
#define WINAMP_SKINZIPW WIDEN(REGISTRY_PREFIX) L".SkinZip"
#define WINAMP_LANGZIP REGISTRY_PREFIX ".LangZip"
#define WINAMP_LANGZIPW WIDEN(REGISTRY_PREFIX) L".LangZip"
#define WINAMP_PLAY REGISTRY_PREFIX ".Play"
#define WINAMP_PLAYW WIDEN(REGISTRY_PREFIX) L".Play"
#define WINAMP_ENQUEUE REGISTRY_PREFIX ".Enqueue"
#define WINAMP_ENQUEUEW WIDEN(REGISTRY_PREFIX) L".Enqueue"
#define WINAMP_BOOKMARK REGISTRY_PREFIX ".Bookmark"
#define WINAMP_BOOKMARKW WIDEN(REGISTRY_PREFIX) L".Bookmark"

/* -------------------------------------------------------------------------------- --- */

extern void _r_s(char *name,char *data, int mlen);
extern void _w_s(char *name, char *data);
void _w_sW(const char *name, const wchar_t *data);
void _r_sW(const char *name, wchar_t *data, int mlen);
extern int _r_i(char *name, int def);
extern void _w_i(char *name, intptr_t d);

CONFIG_EXT int config_utf8 DEF_VAL(1);
CONFIG_EXT wchar_t M3UDIR[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t M3UBASE[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t CONFIGDIR[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t SHAREDDIR[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t PROGDIR[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t ML_INI_FILE[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t INI_FILE[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT char INI_FILEA[MAX_PATH] DEF_VAL("");
CONFIG_EXT wchar_t M3U_FILE[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t OLD_M3U_FILE[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t EQDIR1[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t EQDIR2[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t TEMP_FILE[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t PLUGINDIR[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t SYSPLUGINDIR[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t VISDIR[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t DSPDIR[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t SKINDIR[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t DEMOMP3[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t SKINTEMPDIR[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t LANGTEMPDIR[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t BOOKMARKFILE[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t BOOKMARKFILE8[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t LANGDIR[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT wchar_t JSAPI2_INIFILE[MAX_PATH] DEF_VAL(L"");

#define PE_FONTSIZE 12
CONFIG_EXT int config_pe_fontsize DEF_VAL(PE_FONTSIZE);
#define PE_DIRECTION_AUTO 0
#define PE_DIRECTION_LTR 1
#define PE_DIRECTION_RTL 2
CONFIG_EXT int config_pe_direction DEF_VAL(PE_DIRECTION_LTR);
CONFIG_EXT wchar_t config_langpack[MAX_PATH] DEF_VAL(L""), config_langpack2[MAX_PATH] DEF_VAL(L""), lang_directory[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT unsigned char config_wlz_menu DEF_VAL(0);
CONFIG_EXT int config_skin_prompt DEF_VAL(1);
CONFIG_EXT int config_wlz_prompt DEF_VAL(1);
CONFIG_EXT char config_proxy[256], config_inet_mode DEF_VAL(0);
CONFIG_EXT char config_defext[32] DEF_VAL("mp3");
CONFIG_EXT wchar_t config_titlefmt[1024] DEF_VAL(L"[%artist% - ]$if2(%title%,$filepart(%filename%))");
CONFIG_EXT int config_useexttitles DEF_VAL(1);
CONFIG_EXT wchar_t config_visplugin_name[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT char config_visplugin_num, config_visplugin_priority DEF_VAL(2), config_visplugin_autoexec;
CONFIG_EXT unsigned char config_shuffle_morph_rate DEF_VAL(50);
CONFIG_EXT unsigned char config_playlist_recyclebin DEF_VAL(1);
CONFIG_EXT unsigned char config_ospb DEF_VAL(0), config_eqdsize DEF_VAL(1);
CONFIG_EXT char config_outname[128] DEF_VAL("out_ds.dll");
CONFIG_EXT wchar_t config_dspplugin_name[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT unsigned char config_dspplugin_num;
CONFIG_EXT unsigned char config_sticon,config_usecursors DEF_VAL(1);
CONFIG_EXT unsigned char config_splash DEF_VAL(0) ,config_minst;
CONFIG_EXT unsigned char config_taskbar,config_fixtitles DEF_VAL(3);
CONFIG_EXT unsigned char config_ascb_new DEF_VAL(1),config_ttips DEF_VAL(1), config_priority DEF_VAL(1);
CONFIG_EXT unsigned char config_riol DEF_VAL(4);
CONFIG_EXT unsigned char config_whichicon DEF_VAL(1),config_whichicon2 DEF_VAL(1),config_addtolist;
CONFIG_EXT unsigned char config_snap DEF_VAL(1), config_snaplen DEF_VAL(10), config_parent DEF_VAL(1);
CONFIG_EXT unsigned char config_hilite DEF_VAL(1);
CONFIG_EXT unsigned char config_disvis DEF_VAL(1), config_pladv DEF_VAL(1);
CONFIG_EXT unsigned char config_shownumsinpl DEF_VAL(1),config_zeropadplnum DEF_VAL(0),config_keeponscreen DEF_VAL(1);
CONFIG_EXT unsigned char config_dropaotfs DEF_VAL(1);
CONFIG_EXT unsigned char config_nomwheel DEF_VAL(0);
CONFIG_EXT unsigned char config_autoload_eq;
CONFIG_EXT unsigned char config_use_eq;
CONFIG_EXT int config_wx DEF_VAL(26), config_wy DEF_VAL(29);
CONFIG_EXT unsigned char config_minimized;
CONFIG_EXT unsigned char config_aot;
CONFIG_EXT unsigned char config_shuffle, config_repeat;
CONFIG_EXT unsigned char config_volume DEF_VAL(200), config_easymove DEF_VAL(1);
CONFIG_EXT char config_pan;
CONFIG_EXT unsigned char config_dsize;
CONFIG_EXT unsigned char config_timeleftmode;
CONFIG_EXT unsigned char config_autoscrollname DEF_VAL(1), config_sa DEF_VAL(1), config_safire DEF_VAL(4), 
						 config_saref DEF_VAL(2), config_safalloff DEF_VAL(2),config_sa_peaks DEF_VAL(1),
						 config_sa_peak_falloff DEF_VAL(1);
CONFIG_EXT int config_eq_wx DEF_VAL(26), config_eq_wy DEF_VAL(145), config_eq_open DEF_VAL(1), config_mw_open DEF_VAL(1);
CONFIG_EXT int config_pe_wx DEF_VAL(26), config_pe_wy DEF_VAL(261), config_pe_open DEF_VAL(1), config_pe_width DEF_VAL(275), 
			   config_pe_height DEF_VAL(116), config_pe_height_ws;
/*
CONFIG_EXT int config_si_wx DEF_VAL(0), config_si_wy DEF_VAL(0), config_si_width DEF_VAL(100), config_si_height DEF_VAL(100),
				config_si_autoshow DEF_VAL(1), config_si_autosize DEF_VAL(1), config_si_autohide DEF_VAL(1), config_si_open DEF_VAL(0);
				*/
CONFIG_EXT int config_plscrollsize DEF_VAL(1), config_plmw2xscroll DEF_VAL(1);
CONFIG_EXT unsigned char config_windowshade,config_rofiob,config_eq_ws;
CONFIG_EXT unsigned char config_preamp DEF_VAL(31);
CONFIG_EXT int config_pilp;
CONFIG_EXT unsigned char config_randskin;
CONFIG_EXT wchar_t config_cwd[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT int config_newverchk DEF_VAL(1);
CONFIG_EXT int config_newverchk_rc DEF_VAL(0);
CONFIG_EXT int config_user_consent_join_channels DEF_VAL(-1);
CONFIG_EXT int config_newverchk2 DEF_VAL(1);
CONFIG_EXT int config_newverchk3 DEF_VAL(0);
CONFIG_EXT int config_embedwnd_freesize;
CONFIG_EXT wchar_t config_skin[MAX_PATH] DEF_VAL(L""), skin_directory[MAX_PATH] DEF_VAL(L"");
CONFIG_EXT int config_dotitlenum DEF_VAL(1);
CONFIG_EXT int config_dotasknum DEF_VAL(1);
CONFIG_EXT char config_check_ft_startup DEF_VAL(0), config_updated_ft_startup DEF_VAL(0), config_uid_ft DEF_VAL(0);
CONFIG_EXT char config_bifont DEF_VAL(0); // bitmapped font (off by default)
CONFIG_EXT char config_bifont_alt DEF_VAL(0); // non-bitmapped font alternative option (off by default)
CONFIG_EXT char config_browserbrand[16] DEF_VAL("");

/* --- video --- */
CONFIG_EXT int config_video_wx DEF_VAL(26), config_video_wy DEF_VAL(145),
config_video_open DEF_VAL(0),
config_video_width DEF_VAL(275), config_video_height DEF_VAL(232),
config_video_ratio1 DEF_VAL(4), config_video_ratio2 DEF_VAL(3),
config_video_useratio;

CONFIG_EXT int config_video_aspectadj DEF_VAL(1);
CONFIG_EXT int config_video_updsize DEF_VAL(0);
CONFIG_EXT int config_video_noss DEF_VAL(1);
CONFIG_EXT int config_video_logo DEF_VAL(1);
CONFIG_EXT int config_video_osd DEF_VAL(1);
CONFIG_EXT int config_video_stopclose DEF_VAL(1);
//CONFIG_EXT int config_video_auto_fs DEF_VAL(0); // plague> moved to WinampAttributes.h

//#define EXPERIMENTAL_CONTRAST
#ifdef EXPERIMENTAL_CONTRAST
CONFIG_EXT int config_video_contrast DEF_VAL(128);
CONFIG_EXT int config_video_brightness DEF_VAL(128);
#endif

CONFIG_EXT int config_video_remove_fs_on_stop DEF_VAL(1);
CONFIG_EXT int config_video_fliprgb DEF_VAL(0);

CONFIG_EXT int config_wav_do_header DEF_VAL(1);
CONFIG_EXT int config_wav_convert DEF_VAL(0);
CONFIG_EXT char config_wav_ext[8] DEF_VAL("WAV");
CONFIG_EXT int config_no_visseh;
CONFIG_EXT int config_no_registry DEF_VAL(0);

CONFIG_EXT int config_last_classic_skin_page DEF_VAL(0);
CONFIG_EXT int config_last_playback_page DEF_VAL(0);
CONFIG_EXT int config_last_fileinfo_page DEF_VAL(0);
CONFIG_EXT int config_upd_mode DEF_VAL(0);
CONFIG_EXT wchar_t config_artwork_filter[64] DEF_VAL(L"cover");

CONFIG_EXT int config_noml_ratings_prompt DEF_VAL(1);
CONFIG_EXT int config_noml_ratings DEF_VAL(0);

CONFIG_EXT int config_jtf_check DEF_VAL(0);
CONFIG_EXT int config_block_img DEF_VAL(1);

#endif // config.h