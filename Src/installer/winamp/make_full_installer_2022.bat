set makensis=%ProgramFiles%\NSIS\Unicode\makensis /DUSE_MUI /Dlzma

%makensis% /Dfull main

pause

echo Press any key to continue . . .

exit
