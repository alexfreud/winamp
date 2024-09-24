set makensis=c:\progra~1\nsis\makensis /DUSE_MUI /Dlzma

%makensis% /Dfull main
%makensis% /Dlite main
%makensis% /Dfull /Dpro main
%makensis% /Dfull /Dalkbundle /DeMusic-7plus main
%makensis% /Dfull /DeMusic-7plus main