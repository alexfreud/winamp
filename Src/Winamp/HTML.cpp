/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description: HTML generation from playlist
 ** Author:
 ** Created:
 **/

#include "main.h"
#include <stdio.h>
#include "../nu/AutoChar.h"
#include "resource.h"

void doHtmlPlaylist(void)
{
	FILE *fp = 0;
	wchar_t filename[MAX_PATH] = {0}, tp[MAX_PATH] = {0};
	if (!GetTempPathW(MAX_PATH,tp)) StringCchCopyW(tp, MAX_PATH, L".");
	if (GetTempFileNameW(tp, L"WHT", 0, filename)){
		DeleteFileW(filename);
		StringCchCatW(filename, MAX_PATH, L".html");
	}
	else StringCchCopyW(filename, MAX_PATH, L"wahtml_tmp.html");

	fp = _wfopen(filename, L"wt");
	if (!fp)
	{
		LPMessageBox(hPLWindow, IDS_HTML_ERROR_WRITE, IDS_ERROR, MB_OK | MB_ICONWARNING);
		return;
	}

	fprintf(fp, "<!DOCTYPE html>\n"
				"<html lang=\"en\"><head>\n"
				"<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n"
				"<meta name=\"generator\" content=\"Winamp 5.9\">\n"
				"<style>body{background:#000040;font-family:arial,helvetica;font-size:9pt;font-weight:normal;}"
				".name{margin-top:-1em;margin-left:15px;font-size:40pt;color:#004080;text-align:left;font-weight:900;}"
				".name-small{margin-top:-3em;margin-left:140px;font-size:22pt;color:#E1E1E1;text-align:left;}"
				"table{font-size:9pt;color:#004080;text-align:left;border-width:0px;padding:0px;letter-spacing:normal;}"
				"hr{border:0;background-color:#FFBF00;height:1px;}"
				"ol{color:#FFFFFF;font-size:11pt;}"
				"table{margin-left:15px;color:#409FFF;border-width:0px;}"
				".val{color:#FFBF00;}"
				".header{color:#FFBF00;font-size:14pt;}"
				"</style>\n"
				"<title>Winamp Generated PlayList</title></head>\n"
				"<body>"
				"<div>"
				"<div class=\"name\"><p>WINAMP</p></div>"
				"<div class=\"name-small\"><p>playlist</p></div>"
				"</div>"
				"<hr><div>\n"
				"<table><tr><td>\n");

	int x, t = PlayList_getlength(), t_in_pl = 0, n_un = 0;
	for (x = 0; x < t; x ++) 
	{
		int a = PlayList_getsonglength(x);
		if (a >= 0) t_in_pl += a;
		else n_un++;
	}
	if (t != n_un)
	{
		int old_t_in_pl=t_in_pl;
		t_in_pl += (n_un * t_in_pl) / (t - n_un);

		fprintf(fp, "<span class=\"val\">%d</span> track%s in playlist, ", t, t == 1 ? "" : "s");
		fprintf(fp, "average track length: <span class=\"val\">%d:%02d",
				   old_t_in_pl / (t-n_un) / 60, (old_t_in_pl / (t - n_un)) % 60);

		fprintf(fp, "</span><br>%slaylist length: ",
				n_un ? "Estimated p" : "P");

		if (t_in_pl / 3600)
		{
			fprintf(fp, "<span class=\"val\">%d</span> hour%s ",
					t_in_pl / 3600, t_in_pl / 3600 == 1 ? "" : "s");
			t_in_pl %= 3600;
		}

		if (t_in_pl / 60)
		{
			fprintf(fp, "<span class=\"val\">%d</span> minute%s ",
					t_in_pl / 60, t_in_pl / 60 == 1 ? "" : "s");
			t_in_pl %= 60;
		}
		fprintf(fp, "<span class=\"val\">%d</span> second%s %s",
				t_in_pl, t_in_pl == 1 ? "" : "s", n_un ? "<br>(" : "");
		if (n_un) fprintf(fp, "<span class=\"val\">%d</span> track%s of unknown length)",
						  n_un, n_un == 1 ? "" : "s");

		fprintf(fp,
				"<br>Right-click <a href=\"file://%s\">here</a> to save this HTML file."
				"</td></tr>",
				(char *)AutoChar(filename, CP_UTF8));
	}
	else
	{
		fprintf(fp, "There are no tracks in the current playlist.<br>");
	}

	fprintf(fp, "</table></div>\n");

	if (t > 0)
	{
		fprintf(fp,
				"<blockquote><span class=\"header\">Playlist files:</span><ol>\n");

		for (x = 0; x < t; x++)
		{
			wchar_t fn[FILENAME_SIZE] = {0}, ft[FILETITLE_SIZE] = {0};
			int l;
			PlayList_getitem2W(x, fn, ft);
			AutoChar narrowFt(ft, CP_UTF8);
			char *p = narrowFt;
			l = PlayList_getsonglength(x);
			fprintf(fp, "<li>");
			while (p && *p)
			{
				if (*p == '&') fprintf(fp, "&amp;");
				else if (*p == '<') fprintf(fp, "&lt;");
				else if (*p == '>') fprintf(fp, "&gt;");
				else if (*p == '\'') fprintf(fp, "&#39;");
				else if (*p == '"') fprintf(fp, "&quot;");
				else fputc(*p, fp);
				p++;
			}

			if(l > 0) fprintf(fp, " (%d:%02d) \n", l / 60, l % 60);
			else fprintf(fp, " \n");
		}

		fprintf(fp, "</ol></blockquote>\n");
	}
	fprintf(fp, "<hr><br></body></html>");
	fclose(fp);

	myOpenURL(hPLWindow, filename);
}