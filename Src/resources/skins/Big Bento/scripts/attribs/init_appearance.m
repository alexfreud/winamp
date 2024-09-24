/*---------------------------------------------------
-----------------------------------------------------
Filename:	init_appearance.m
Version:	2.2

Type:		maki/attrib definitions
Date:		28. Jun. 2007 - 22:08 
Author:		Martin Poehlmann aka Deimos
E-Mail:		martin@skinconsortium.com
Internet:	www.skinconsortium.com
		www.martin.deimos.de.vu
-----------------------------------------------------
Depending Files:
		scripts/mainmenu.maki
		scripts/visualizer.maki
		scripts/beatvisualization.maki
		scripts/shadesize.maki
		scripts/mcvcore.maki
-----------------------------------------------------
---------------------------------------------------*/

#ifndef included
#error This script can only be compiled as a #include
#endif

#include "gen_pageguids.m"

Function initAttribs_Appearance();

#define CUSTOM_PAGE_APPEARANCE "{F1036C9C-3919-47ac-8494-366778CF10F9}"

Global ConfigAttribute vis_reflection_attrib;
Global ConfigAttribute timer_reflection_attrib;
Global ConfigAttribute menubar_main_attrib;
Global configAttribute beatvis_attrib;
//Global configAttribute artist_info_buttons_attrib;
Global ConfigAttribute pl_tab_attrib;
Global ConfigItem custom_page_appearance;

initAttribs_Appearance()
{

	initPages();

	custom_page_appearance = addConfigSubMenu(optionsmenu_page, "Appearance", CUSTOM_PAGE_APPEARANCE);

	menubar_main_attrib = custom_page_appearance.newAttribute("Show Main Window Menu", "1");

	addMenuSeparator(custom_page_appearance);

	vis_reflection_attrib = custom_page_appearance.newAttribute("Show Visualizer Reflection", "1");
	timer_reflection_attrib = custom_page_appearance.newAttribute("Show Timer Reflection", "1");

	addMenuSeparator(custom_page_appearance);

	//artist_info_buttons_attrib = custom_page_appearance.newAttribute("Show Artist-Information Buttons", "0");
	pl_tab_attrib = custom_page_appearance.newAttribute("Show Playlist Tab", "0");

	addMenuSeparator(custom_page_appearance);

	beatvis_attrib = custom_page_appearance.newAttribute("Enable Beat Visualization", "1");

}