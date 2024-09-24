Section -HiddenExtra
	Call WMF_NeedDownload
	Pop $0
	${If} $0 == "0" #nobody needs downlod  - skip section
		goto download_end
	${EndIf}

	DetailPrint $(IDS_RUN_CHECK_IFCONNECTED)
	Call ConnectInternet
	Pop $0
	${If} $0 == "online"
		DetailPrint $(IDS_RUN_CHECK_IFINETAVAILABLE)
		Call IsInternetAvailable
		Pop $0
	${EndIf}

	${If} $0 != "online"
		DetailPrint $(IDS_RUN_NOINET)
		goto download_end
	${EndIf}

	Call WMF_Download

download_end:
	Call WMF_Install

SectionEnd