Winamp Skin Development Pack (WaSDP) ~  Version 5.9 (2022/08/22)
Copyright © 2007-2013 Martin Poehlmann / NULLSOFT
Copyright © 2013-2022 Winamp SA

~ README ~

Well, this is again one of those README files you might not want to read, but in three cases it is recommended to read it:
I	You are new to Skinning
II)	You want some information about Bento scripts
III)	You are wondering why you have no m files in Bento/Wa Modern Skin dir after reinstallig  Winamp

~ CONTENTS ~

1)	Preludium... Wasabi, the Winamp Skinning Engine
2)	MC.EXE - The 'ultimative' maki compiler
2.1)	How to compile m files to maki files
2.2)	Edit Plus 2 - a textbased editor
2.3)	Maki Overview
3)	Bento Scripts
4)	Reinstalling Winamp with WaSDP
5)	I want help - got lost within the skinning engine... or can someone make my skin
6)	...Postludium


1) Preludium... Wasabi, the Winamp Skinning Engine

Well, Wasabi is Winamp's own scripting system (WinAmp System Architecture Building Interface).
The Wasabi Skinning Engine was developed along with Winamp3 and then incorporated into gen_ff.dll of Winamp 5. But for creating Skins you need not know how to code Wasabi, you 'just' need some XML experience. If you want a real good skin, you should also be able to write maki (Maki A Killer Interface) scripts.
For beginners you should start looking at the xml code of Winamp Modern and Bento. Try to understand the structure of WasabiXML, button functions, etc.
Here are three pages that share good information about winamp skinning:

http://www.winamp.com/development/skins-modern
http://wiki.skinconsortium.com
the last one i do not remember since i have a local copy of it, hehe; but try to google: Winamp XML Object Reference :)

Last but not least there are also two good forums where you can post questions about skinning, etc.
http://forums.winamp.com
http://forums.skinconsortium.com

Now some info about me: I am Martin, Admin and Skin Dev from skinconsortium.com (known as martin.deimos). I am also the coder of Winamp's new SUI Skin called Bento.
If you have skinning related question please do NOT pm or mail me. post your question at winamp forums or skinconsortium forums!

2) MC.EXE - The 'ultimative' maki compiler
2.1)	How to compile m files to maki files

Ok, if you have played with WasabiXML you will most likely do more complex skins. With MAKI scripts you have the power to create new functions (like mute, new visualizations,...) for your skin.
Maki is also a new programming language aligned to JavaScript. The best option to start is, look at the m scripts of Winamp Modern and Bento.
Now it is the right time to configure MC.EXE, the maki compiler which coverts the m files to maki files readable by Wasabi Skinning Engine.
just call MC.EXE [filename]
but calling this everytime from the DOS prompt is really annaying. So i give now a method how you can easyly compile m scripts.

2.2) Edit Plus 2 - a textbased editor

Download Edit Plus (http://www.editplus.com), install and run it.
Goto Tools > Configure User Tools...
Click on Add Program and select Program
Then fill the editboxes:
Menu Text: Compile To Maki
Command: %WinampDir%\mc.exe (replace %WinampDir% by your Winamp Program Path!!!)
Argument: $(FileName)
Initial: $(FileDir)
uncheck close window on exit
Now click apply. you can access this tool in the tools menu. you can also add it as symbol to the titlebar (please figure out yourself if you want to have it this way).
Now we do a first test if everything is working: open from Bento Skin in scripts folder eq.m with editplus and call the maki compiler (with your preferred method).
If everything worked you will get an fine output in the dos window. if not there is either a script error or you havn't configured the user tool in the right way.
If you have installed the Edit Plus Syntax lightning from WaSDP you can enable this feature in editplus according to this procedure:
You add a new filetype under prefs > files > settings & syntax. the extensions are m/mi, now locate the stx/acp files (in your winampdir).
click ok and go to document > refresh ACP/STX. now your file should have nice colors :)
If you want to use the cliptext funstions in Editplus, you must copy maki.ctl to your editplus ini dir. (progs/editplus2) [i am not really sure about this step, since i do not use cliptext :(]

2.3) Maki Overview

Now you should at least be able to compile maki scripts. Learning how to write them is the next step. Again i encourage you to look at the scripts that are delivered with this pack!
They are always build like this:

#include <lib/std.mi> //must always be included!!

//Now you define some functions or globals.

Global String hello;
Global Text myText

//The whole script starts with this function:

System.onScriptLoaded ()
{
	//here we define our variables

	group sg = getScriptGroup(); //this is the group our script runs in

	myText = sg.findObject("the id of a text object in xml");

	hello = "Hey, this is my first script";

	myText.setText(hello);
}

Your next quetsion might be, what functions can i call?
The answer is easy: all functions that are listed in the *.mi files in WinampDir/lib.
For documentations, use again teh above links or look at other scripts.


3) Bento Scripts

I suggest if you are new in skinning and maki, start with Winamp Modern, since it uses much easier maki scripts.
Bento itself are 2 skins (Bento and Big Bento) Bento uses the same scripts as Big Bento (reads them from the Big Bento/scripts folder). So i need sometimes to submit params within my skin declaration in order to detect which skinn is currently running.
The next difference is that I included some scripts to be applied more than one time in xml (like hoverstimulate.maki). i submit all neccessary params again via the script params.
Last but not least Bento's scripts are communicating with each other via the sendAction function.

for example if you collapse the player window, the normal layout will recieve this function:

normal.onAction (String action, String param, Int x, int y, int p1, int p2, GuiObject source)
{
	if (action == "sui" && param == "tonocomp" && x == 0)
	{
		// x is 0 if the window is not collapsed, or 1 if it is already collapsed (on startup)
	}
}

here is a (incomplete) list of all calls i remember:

action		| param		| x
-normal layout:
sui		  tonocomp	  1/0
		  fromnocomp
load_comp	  pledit
pledit_posupdate
-playlist/infocomp frame
set_maxwidth
-browser
openurl		  url
search		  string
		  go (if you want to use the editbox text)
-sui group
browser_navigate  some site
browser_search	  some string
...

So you can send "browser_navigate" "http://mysite.com" to the sui group, and it will switch to the browser and open the site, hehe.
Perhaps read through the scripts to see why i added these calls, and how you can manipluate them...


4) Reinstalling Winamp with WaSDP

If you want to install a newer version of winamp and WaSDP is already installed, it might happen that the m files in the default skin dirs are deleted.
Solution: install WaSDP again, hehe.


5) I want help - got lost within the skinning engine... or can someone make my skin

If you have a great skin idea or a great design and you have big problems with coding xml/maki you have some possibilities:
- post at http://forums.winamp.com but ensure to read the "you want someone to make a skin for you"
- post at http://forums.skinconsortium.com and we will decide if we want to code this skin (atm we have lots proposals that are not done... but feel free to post)
- throw your idea/PSDs in the recycle bin
- try with more effort to learn XML/MAKI, hehe

6) ...Postludium
That's all :)


Well, i hope i could help you with this readme in some skinning issues,
-Martin
