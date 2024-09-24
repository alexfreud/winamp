Winamp Installer (NSIS)

Author: Eddy Richman
Date: 9 June 2022
Revised: 26 June 2022
Copyright: Winamp SA, c. 1997-2022


Compilation instructions
========================

The easiest way to achieve success is to ensure all required Winamp files/folders are in the same parent directory.


1. Copy the required files from Winamp-Desktop

Copy the following folders/files to the Output Dir, which in the case of the Winamp 32-bit release package is:

..\..\..\..\Winamp_x86_Release


All the files must be in the correct subfolders!

(maybe we can create a batch file or add this process to e.g. winampAll PostBuildEvent?)


- installer\winamp
- installer\shared
- resources\data
- resources\license
- resources\skins
		- Note: "Nokia" subfolder is not required
- resources\libraries
		- All files are required, EXCEPT:
		- atl90.dll, msvcp90.dll, msvcr90.dll, Microsoft.VC90.ATL.manifest, Microsoft.VC90.CRT.manifest
- resources\plugins\dsp_sps.dll
- resources\media\demo.mp3
- resources\media\winamp.m3u

_________________________________________________________________________

Note:
=====

resources\languages dir is only required for running specific international builds.

With a couple of exceptions (en-us & pl-pl), all the lang packs need updating.
So we are currently only providing the en-us version (for v5.8 and v5.9).

To create e.g. Winamp59_final_full_all.exe or Winamp59_final_full_fr-fr.exe,
the scripts in the Mastering folder need updating accordingly first...
which is something I'm currently working on... *gulp*

_________________________________________________________________________


2. Make sure the required NSIS plugins are present

Extract the dll files from: %GitHub%\Radionomy\Nullsoft\nsis\nsis_plugins.zip
into the NSIS plugins folder, which by default will be found at:
%ProgramFiles%\NSIS\Unicode\Plugins\x86-unicode

_________________________________________________________________________


3. Digital Signing

winamp.exe, winampa.exe, Elevator.exe & reporter.exe
all need to be digitally signed before we create the final public distribution package.

The scripts in Mastering\Winamp would normally do this, but they need updating first.
Note that the Mastering scripts were written by previous devs (iirc, Justin, Francis, Benski, Maksim), and not by me.

Codesign\sign.cmd can be used (and would be used by any mastering script),
but for manual signing, it's much easier to use DigiCertUtil.exe

If you've already run DigiCertUtil.exe from Winamp-Desktop\codesign, then (unless you hack the registry), afaict, that's where it will always run from!

a) Copy winamp.exe, winampa.exe, Elevator.exe & reporter.exe from the "Winamp_x86_Release" root folder to the "Winamp-Desktop\codesign" folder
b) Open Winamp-Desktop\Codesign\DigiCertUtil.exe
c) Select: "Code Signing" on the left - and the valid cert (Winamp SA, 13-Jun-2024, 08d4bf5a529c725997e0f6c526495d2f, DigiCert)
d) Note: If not already done so, you might need to install the cert first, so click the Install Cert button
e) Click the "Sign Files" button
f) Checkmark "Add a timestamp to the sig" if not already checked.
g) Click "Add Files" and use ctrl+click to add the 4 x *.exe files in a)
h) Click "Sign"
i) When done, close DigiCertUtil
j) cut+paste the 4 signed .exe files back into the "Winamp_x86_Release" root folder

k) Repeat the above process for the installer file after step 5.

So this is a bit of a shemozzle, but again, the updated scripts in Mastering will (eventually) automate the process.

_________________________________________________________________________


4. Create .wbm files

   Copy "Mastering\Winamp\build_wbm_2022.cmd" to the root of Winamp_x86_Release
   Run "build_wbm_2022.cmd" to generate the System\*.wbm files

   - Note: .wbm = Lazy-load Wasabi Manifests
   - This makes the respective .w5s files only load when required and not at Winamp startup.

   IMPORTANT
   =========

   The .wbm files need to be recreated every time new .w5s files are built!

_________________________________________________________________________


5. Create the installer

   - Open in Notepad or similar: Winamp_x86_Release\installer\winamp\make_full_installer_2022.bat
   - Make sure the path to NSIS\makensis is correct
   - Run: make_full_installer_2022.bat


Notes
=====

The generated filename will be: Winamp_x86_Release\installer\winamp\$%INSTALL_NAME%_full.exe
Rename to e.g. Winamp59_9999_beta_rc1_en-us.exe or Winamp59_9999_final_full_en-us.exe, etc.

Again, this is something the updated scripts in Mastering will do automatically...

_________________________________________________________________________

END
