function startup(){
}

function NotImplementedYet(){
	alert("This method has not been implemented yet");
}

function tStopClicked(){
	var t_rc = window.external.Transport.Stop();
}

function tPlayClicked(){
	var t_rc = window.external.Transport.Play();
}

function tPauseClicked(){
	var t_rc = window.external.Transport.Pause();
}

function tPrevClicked(){
	var t_rc = window.external.Transport.Prev();
}

function tNextClicked(){
	var t_rc = window.external.Transport.Next();
}

function tGetMetadataClicked(){
	var metaTag = document.getElementById("t_t_mTag_in").value;
	var metadata = window.external.Transport.GetMetadata(metaTag);
	document.getElementById("t_t_mTag_Response").value = metadata;
}

function tGetShuffleClicked(){
	var t_shuffle = window.external.Transport.shuffle;
	document.getElementById("t_t_shuffle").value = t_shuffle;
}

function tSetShuffleClicked(){
	var t_shuffle = document.getElementById("t_t_shuffle_in").value;
	if (t_shuffle == "true") window.external.Transport.shuffle = true;
	else window.external.Transport.shuffle = false;
	tGetShuffleClicked();
}

function tGetRepeatClicked(){
	var t_repeat = window.external.Transport.repeat;
	document.getElementById("t_t_repeat").value = t_repeat;
}

function tSetRepeatClicked(){
	var t_repeat = document.getElementById("t_t_repeat_in").value;
	if (t_repeat == "true") window.external.Transport.repeat = true;
	else window.external.Transport.repeat = false;
	tGetRepeatClicked();
}

function tGetPositionClicked(){
	var t_position = window.external.Transport.position;
	document.getElementById("t_t_position").value = t_position;
}

function tSetPositionClicked(){
	var t_position = document.getElementById("t_t_position_in").value;
	window.external.Transport.position = parseInt(t_position);
	tGetPositionClicked();
}

function tLengthClicked(){
	var length = window.external.Transport.length;
	document.getElementById("t_t_Length").value = length;
}

function tURLClicked(){
	var url = window.external.Transport.url;
	document.getElementById("t_t_URL").value = url;
}

function tTitleClicked(){
	var title = window.external.Transport.title;
	document.getElementById("t_t_Title").value = title;
}

function tPlayingClicked(){
	var playing = window.external.Transport.playing;
	document.getElementById("t_t_Playing").value = playing;
}

function tPausedClicked(){
	var paused = window.external.Transport.paused;
	document.getElementById("t_t_Paused").value = paused;
}

function pqPlayClicked() {
	var myMusic = document.getElementById("pq_t_PlayURL_in").value;
	var title = document.getElementById("pq_t_PlayTitle_in").value
	var length = document.getElementById("pq_t_PlayLength_in").value;
	var ilength = parseInt(length);
	if (isNaN(ilength)) {ilength = 0;}
	var pq_play = window.external.PlayQueue.Play(myMusic, title, ilength);
}

function pqEnqueueClicked() {
	var myMusic = document.getElementById("pq_t_EnqueueURL_in").value;
	var title = document.getElementById("pq_t_EnqueueTitle_in").value
	var length = document.getElementById("pq_t_EnqueueLength_in").value;
	var ilength = parseInt(length);
	if (isNaN(ilength)) {ilength = 0};
	var pq_play = window.external.PlayQueue.Enqueue(myMusic, title, ilength);
}

function pqInsertClicked() {
	var position = document.getElementById("pq_t_InsertPosition_in").value;
	var iposition = parseInt(position);
	if (isNaN(iposition)) {iposition = 0;}
	var myMusic = document.getElementById("pq_t_InsertURL_in").value;
	var title = document.getElementById("pq_t_InsertTitle_in").value
	var length = document.getElementById("pq_t_InsertLength_in").value;
	var ilength = parseInt(length);
	if (isNaN(ilength)) {ilength = 0;}
	var pq_play = window.external.PlayQueue.Insert(iposition, myMusic, title, ilength);
}

function pqClearQClicked() {
	var pq_rc = window.external.PlayQueue.ClearQueue();
}

function pqGetMetadataClicked(){
	var metaPos = document.getElementById("pq_t_MetadataPosition_in").value;
	var imetaPos = parseInt(metaPos);
	var metaTag = document.getElementById("pq_t_MTag_in").value;
	var metadata = window.external.PlayQueue.GetMetadata(imetaPos, metaTag);
	document.getElementById("pq_t_MTag_Response").value = metadata;
}

function pqGetTitleClicked(){
	var position = document.getElementById("pq_t_TitlePosition_in").value;
	var iposition = parseInt(position);
	var title = window.external.PlayQueue.GetTitle(iposition);
	document.getElementById("pq_t_Title").value = title;
}

function pqGetURLClicked() {
	var position = document.getElementById("pq_t_URLPosition_in").value;
	var iposition = parseInt(position);
	var url = window.external.PlayQueue.GetURL(iposition);
	document.getElementById("pq_t_URL").value = url;
}

function pqGetLengthClicked(){
	var length = window.external.PlayQueue.length;
	document.getElementById("pq_t_Length").value = length;
}

function pqGetCursorClicked(){
	var cursor = window.external.PlayQueue.cursor;
	document.getElementById("pq_t_Cursor").value = cursor;
}

function pqSetCursorClicked(){
	var pq_cursor = document.getElementById("pq_t_Cursor_in").value;
	window.external.PlayQueue.cursor = parseInt(pq_cursor);
	pqGetCursorClicked();
}

var listPlaylists;
function plsGetPlaylists(){
	listPlaylists = window.external.Playlists.GetPlaylists();
	var numPlaylists = listPlaylists.length;
	document.getElementById("pls_t_Count").value = numPlaylists;
}

function plsViewIndexClicked(){
	var view_index = document.getElementById("pls_t_Index_in").value;
	var iview_index = parseInt(view_index);
	document.getElementById("pls_t_Filename").value = listPlaylists[iview_index].filename;
	document.getElementById("pls_t_Title").value = listPlaylists[iview_index].title;
	document.getElementById("pls_t_PlaylistId").value = listPlaylists[iview_index].playlistId;
	document.getElementById("pls_t_Length").value = listPlaylists[iview_index].length;
	document.getElementById("pls_t_NumItems").value = listPlaylists[iview_index].numitems;
	document.getElementById("pls_t_OpenPlaylistId_in").value = listPlaylists[iview_index].playlistId;
	document.getElementById("pls_t_SavePlaylistId_in").value = listPlaylists[iview_index].playlistId;
}

var currentPlaylist;
function plsOpenPlaylistClicked(){
	var id = document.getElementById("pls_t_OpenPlaylistId_in").value;
	currentPlaylist = window.external.Playlists.OpenPlaylist(id);
	document.getElementById("playlist_t_NumItems").value = currentPlaylist.numitems;
	plsDisableListMethods(false);
	showPlaylist();
}

var currentItemIndex = 0;

function plsDisablePlaylistMethods(disable){
	var playlistSection = document.getElementById("playlistmethods");
	playlistSection.disabled = disable;
	for (var i=0; i < playlistSection.childNodes.length; i++){
		if (playlistSection.childNodes[i].nodeType == 1) {
			playlistSection.childNodes[i].disabled = disable;
		}
	}
}

function plsDisableListMethods(disable){
	var listSection = document.getElementById("listmethods");
	listSection.disabled = disable;
	for (var i=0; i < listSection.childNodes.length; i++){
		if (listSection.childNodes[i].nodeType == 1) {
			listSection.childNodes[i].disabled = disable;
		}
	}
}

function plsPlaylistViewIndexClicked(){
	var view_index = document.getElementById("playlist_t_ItemIndex_in").value;
	var iview_index = parseInt(view_index);
	currentItemIndex = iview_index;
	document.getElementById("playlist_t_Filename").value = currentPlaylist.GetItemFilename(currentItemIndex);
	document.getElementById("playlist_t_Title").value = currentPlaylist.GetItemTitle(currentItemIndex);
	document.getElementById("playlist_t_Length").value = currentPlaylist.GetItemLength(currentItemIndex);
	plsDisablePlaylistMethods(false);
	showPlaylist();
}

var playlistwin;
var playlistWinShown = false;
function showPlaylist(){
	var output = "";
	for (var i = 0; i < currentPlaylist.numitems; i++) {
		var title = currentPlaylist.GetItemTitle(i);
		output += i + ":" + title + "\n";
	}
	alert(output);
}

function plsPlaylistSetItemFilenameClicked(){
	var filename = document.getElementById("playlist_t_SetItemFilename_in").value;
	currentPlaylist.SetItemFilename(currentItemIndex, filename);
	document.getElementById("playlist_t_Filename").value = filename;
}

function plsPlaylistSetItemTitleClicked(){
	var title = document.getElementById("playlist_t_SetItemTitle_in").value;
	currentPlaylist.SetItemTitle(currentItemIndex, title);
	document.getElementById("playlist_t_Title").value = title;
}

function plsPlaylistSetItemLengthClicked(){
	var length = document.getElementById("playlist_t_SetItemLength_in").value;
	var iLength = parseInt(length);
	currentPlaylist.SetItemLengthMilliseconds(currentItemIndex, iLength);
	document.getElementById("playlist_t_Length").value = length;
}

function plsPlaylistReverseClicked(){
	currentPlaylist.Reverse();
	showPlaylist();
}

function plsPlaylistSwapItemsClicked(){
	var swap1 = document.getElementById("playlist_t_Swap1_in").value;
	var swap2 = document.getElementById("playlist_t_Swap2_in").value;
	var iswap1 = parseInt(swap1);
	var iswap2 = parseInt(swap2);
	var rc = currentPlaylist.SwapItems(iswap1, iswap2);
	showPlaylist();
}

function plsPlaylistRandomizeClicked(){
	currentPlaylist.Randomize();
	showPlaylist();
}

function plsPlaylistRemoveClicked(){
	alert("Inside Remove, currentItemIndex=" + currentItemIndex);
	currentPlaylist.RemoveItem(currentItemIndex);
	showPlaylist();
}

function plsPlaylistSortByTitleClicked(){
	currentPlaylist.SortByTitle();
	showPlaylist();
}

function plsPlaylistSortByFilenameClicked(){
	currentPlaylist.SortByFilename();
	showPlaylist();
}

function plsPlaylistInsertUrlClicked(){
	var url = document.getElementById("playlist_t_InsertUrl_in").value;
	var title = document.getElementById("playlist_t_InsertTitle_in").value;
	var length = document.getElementById("playlist_t_InsertLength_in").value;
	var iLength = parseInt(length);
	var rc = currentPlaylist.InsertURL(currentItemIndex, url, title, iLength);
	showPlaylist();
}

function plsPlaylistAppendUrlClicked(){
	var url = document.getElementById("playlist_t_AppendUrl_in").value;
	var title = document.getElementById("playlist_t_AppendTitle_in").value;
	var length = document.getElementById("playlist_t_AppendLength_in").value;
	var iLength = parseInt(length);
	var rc = currentPlaylist.AppendURL(url, title, iLength);
	showPlaylist();
}

function plsPlaylistClearClicked(){
	var rc = currentPlaylist.Clear();
	showPlaylist();
}

function plsSavePlaylistClicked() {
	var playlistId = document.getElementById("pls_t_SavePlaylistId_in").value;
	var rc = window.external.Playlists.SavePlaylist(playlistId, currentPlaylist);
	document.getElementById("playlistmethods").disabled = true;
}

function BMarkAddClicked(){
	var url = document.getElementById("bmark_t_AddURL_in").value;
	var title = document.getElementById("bmark_t_AddTitle_in").value;
	var rc = window.external.Bookmarks.Add(url, title);
}

function PodcastSubscribeClicked(){
	var podcUrl = document.getElementById("podc_t_SubscribeURL_in").value;
	var rc = window.external.Podcasts.Subscribe(podcUrl);
}

function ConfigSetPropertyClicked(){
	var inParam = document.getElementById("config_t_SetPropertyName_in").value;
	var inValue = document.getElementById("config_t_SetPropertyValue_in").value;
	if (pType[0].checked) {
		inValue = "'" + inValue + "'";
	} else if (pType[2].checked){
		if (inValue != "false"){
			inValue = "true"; 
		}
	}
	var funcBody = "var rc = window.external.Config." + inParam + "=" + inValue;
	var configPropSet = new Function(funcBody);
	configPropSet();
}

function ConfigGetPropertyClicked(){
	var inParam = document.getElementById("config_t_GetPropertyName_in").value;
	var funcBody = "return window.external.Config." + inParam;
	var configPropGet = new Function(funcBody);
	var propValue = configPropGet();
	document.getElementById("config_t_GetPropertyValue").value = propValue;
}

function ApplicationLaunchURLClicked(){
	var url = document.getElementById("application_t_URL_in").value;
	var forceExternal = document.getElementById("application_t_ForceExternal_in").checked;
	var rc = window.external.Application.LaunchURL(url, forceExternal);
}

function ApplicationNumVersionClicked(){
	var numVer = window.external.Application.version;
	document.getElementById("application_t_NumVersion").value = parseInt(numVer);
}

function ApplicationStringVersionClicked(){
	var stringVer = window.external.Application.versionstring;
	document.getElementById("application_t_StringVersion").value = stringVer;
}

function ApplicationLanguageClicked(){
	var lang = window.external.Application.language;
	document.getElementById("application_t_Language").value = lang;
}

function ApplicationLanguagePackClicked(){
	var langPack = window.external.Application.languagepack;
	document.getElementById("application_t_LanguagePack").value = langPack;
}

function SkinGetClassicColorClicked(){
	var colorNum = document.getElementById("skin_t_ClassicColorNumber_in").value;
	var iColorNum = parseInt(colorNum);
	var classicColor = window.external.Skin.GetClassicColor(iColorNum);
	document.getElementById("skin_t_ClassicColor").value = classicColor;
}

function SkinGetPlaylistColorClicked(){
	var colorNum = document.getElementById("skin_t_PlaylistColorNumber_in").value;
	var iColorNum = parseInt(colorNum);
	var playlistColor = window.external.Skin.GetPlaylistColor(iColorNum);
	document.getElementById("skin_t_PlaylistColor").value = playlistColor;
}

function SkinGetSkinColorClicked(){
	var colorName = document.getElementById("skin_t_SkinColorName_in").value;
	var skinColor = window.external.Skin.GetSkinColor(colorName);
	document.getElementById("skin_t_SkinColor").value = skinColor;
}

function SkinGetNameClicked(){
	var name = window.external.Skin.name;
	document.getElementById("skin_t_Name").value = name;
}

function SkinSetNameClicked() {
	var skinName = document.getElementById("skin_t_Name_in").value;
	window.external.Skin.name = skinName;
}

function SkinGetFontClicked(){
	var font = window.external.Skin.font;
	document.getElementById("skin_t_Font").value = font;
}

function SkinGetFontSizeClicked(){
	var fontsize = window.external.Skin.fontsize;
	document.getElementById("skin_t_FontSize").value = fontsize;
}

function mcGetMetadataClicked(){
	var metaFile = document.getElementById("mc_t_MediaCoreFilename_in").value;
	var metaTag = document.getElementById("mc_t_MediaCoreTag_in").value;
	var metadata = window.external.MediaCore.GetMetadata(metaFile, metaTag);
	document.getElementById("mc_t_MediaCoreMetadata_Response").value = metadata;
}

function mcIsRegisteredExtensionClicked(){
	var extension = document.getElementById("mc_t_MediaCoreExtension_in").value;
	var supported = window.external.MediaCore.IsRegisteredExtension(extension);
	document.getElementById("mc_t_MediaCoreRegisteredExtension_Response").value = supported;
}

function mcAddMetadataHookClicked(){
	var mUrl = document.getElementById("mc_t_AddMetadataHookUrl_in").value;
	var mTag = document.getElementById("mc_t_AddMetadataHookTag_in").value;
	var mValue = document.getElementById("mc_t_AddMetadataHookValue_in").value;
	window.external.MediaCore.AddMetadataHook(mUrl,mTag,mValue);
}

function mcRemoveMetadataHookClicked(){
	var mUrl = document.getElementById("mc_t_RemoveMetadataHookUrl_in").value;
	var mTag = document.getElementById("mc_t_RemoveMetadataHookTag_in").value;
	window.external.MediaCore.RemoveMetadataHook(mUrl,mTag);
}

function teRegisterClicked(){
	var rc = window.external.Transport.RegisterForEvents(onEvents);
	document.getElementById("teRegister").disabled = true;
	document.getElementById("teUnregister").disabled = false;
}

function teUnregisterClicked(){
	var rc = window.external.Transport.UnregisterFromEvents(onEvents);
	document.getElementById("teRegister").disabled = false;
	document.getElementById("teUnregister").disabled = true;
}

var eventsArray = new Array();
var eventCount = 0;
function onEvents(event){
	eventsArray[eventCount] = event;

	// populate the select box
	var eventSelect = document.getElementById("te_s_Select");
	var newOption = new Option((eventCount + 1) + ":" + event.event, event);
	eventSelect.options[eventCount] = newOption;
	eventCount++;
}

function teEventSelected(){
	var sel = document.getElementById("te_s_Select");
	var area = document.getElementById("te_ta_Area");
	var tarea = "";
	var obj = eventsArray[sel.selectedIndex];
	for (var prop in obj){
		tarea += "property:" + prop + " value:" + obj[prop] + "\n\n";	
	}
	area.value = tarea;
}

function te_AreaCleared(){
	eventCount = 0;
	document.getElementById("te_ta_Area").value = "";
	document.getElementById("te_s_Select").options.length = 0;
}

function hisQueryClicked(){
	var query = document.getElementById("his_t_Query_in").value;
	var respArray = window.external.History.Query(query);
	alert(respArray[0].filename);
	var textOut = "";
	for (var obj in respArray){
		if (textOut != ""){
			textOut += "\n";
		}
		textOut += "==============";
		textOut += "\nTitle: " + respArray[obj].title;
		textOut += "\nLastPlay: " + respArray[obj].lastplay;
		textOut += "\nPlaycount: " + respArray[obj].playcount;
		textOut += "\nFilename: " + respArray[obj].filename;
		textOut += "\nLength: " + respArray[obj].length;
	}
	var textAreaOut = document.getElementById("his_t_Query");
	textAreaOut.value = textOut;
}

function AsyncDownloadMediaClicked(){
	var url = document.getElementById("asyncdownloader_t_URL_in").value;
	var destFile = document.getElementById("asyncdownloader_t_DestinationFile_in").value;
	document.getElementById("progressbar").firstChild.nodeValue = "";
	document.getElementById("slider").style.clip = "rect(0px 0px 16px 0px)";
	if (destFile) 
		var rc = window.external.AsyncDownloader.DownloadMedia(url, destFile);
	else
		var rc = window.external.AsyncDownloader.DownloadMedia(url);
}

function AsyncDownloadMediaClicked1(){
	var url = document.getElementById("asyncdownloader_t_URL_in_1").value;
	var destFile = document.getElementById("asyncdownloader_t_DestinationFile_in_1").value;
	document.getElementById("progressbar1").firstChild.nodeValue = "";
	document.getElementById("slider1").style.clip = "rect(0px 0px 16px 0px)";
	if (destFile) 
		var rc = window.external.AsyncDownloader.DownloadMedia(url, destFile);
	else
		var rc = window.external.AsyncDownloader.DownloadMedia(url);
}

function AsyncDownloadMultipleMediaClicked() {
	var multipleUrls = document.getElementById("multiple_urls").value;
	var urls = multipleUrls.split(";");
	for(index = 0; index < urls.length; index++){
		if (urls[index].length > 0) {
			if (urls[index].indexOf("http://") != -1) urls[index] = urls[index].substr(7);
			var rc = window.external.AsyncDownloader.DownloadMedia("http://"+escape(urls[index]));
		}
	}
}

function downloaderRegisterClicked(){
	var rc = window.external.AsyncDownloader.RegisterForEvents(onDownloaderEvents);
	document.getElementById("downloaderRegister").disabled = true;
	document.getElementById("downloaderUnregister").disabled = false;
}
  
function downloaderUnregisterClicked(){
	var rc = window.external.AsyncDownloader.UnregisterFromEvents(onDownloaderEvents);
	document.getElementById("downloaderRegister").disabled = false;
	document.getElementById("downloaderUnregister").disabled = true;
}

var downloaderEventsArray = new Array();
var downloaderEventCount = 0;
function downloader_AreaCleared(){
	downloaderEventCount = 0;
	document.getElementById("downloader_ta_Area").value = "";
	document.getElementById("downloader_s_Select").options.length = 0;
}

function onDownloaderEvents(event){
	downloaderEventsArray[downloaderEventCount] = event;

	// populate the select box
	var eventSelect = document.getElementById("downloader_s_Select");
	var newOption = new Option((downloaderEventCount + 1) + ":" + event.event, event);
	eventSelect.options[downloaderEventCount] = newOption;
	downloaderEventCount++;

	var progress, slider;
	if (event.url == document.getElementById("asyncdownloader_t_URL_in").value)
	{
		progress = document.getElementById("progressbar");
		slider = document.getElementById("slider");
	}
	if (event.url == document.getElementById("asyncdownloader_t_URL_in_1").value)
	{
		progress = document.getElementById("progressbar1");
		slider = document.getElementById("slider1");
	}

	if (progress && slider) 
	{
		if (event.event == 'OnInit')
		{
			progress.firstChild.nodeValue = 'Progress: Start downloading ...';
		}
		if (event.event == 'OnData')
		{
			progress.firstChild.nodeValue = 'Progress: Downloading ...';
			var factor = event.downloadedlen/event.totallen;
			slider.firstChild.nodeValue = Math.ceil(factor * 100) + '%';
			slider.style.clip = "rect(0px " + parseInt(factor * 417) + "px 16px 0px)";
		}
		if (event.event == 'OnFinish')
		{
			progress.firstChild.nodeValue = 'Progress: Downloading Succeed';
			slider.firstChild.nodeValue = '100%';
		}
		if (event.event == 'OnError' || event.event == 'OnCancel')
		{
			progress.firstChild.nodeValue = 'Progress: Downloading Failed';
		}
	}
}

function downloaderEventSelected(){
	var sel = document.getElementById("downloader_s_Select");
	var area = document.getElementById("downloader_ta_Area");
	var tarea = "";
	var obj = downloaderEventsArray[sel.selectedIndex];
	if (obj.url)
		tarea += "property:url value:" + obj.url + "\n\n";	
	if (obj.event == 'OnData')
	{
		tarea += "property:downloadedlen value:" + obj.downloadedlen + "\n\n";
		tarea += "property:totallen value:" + obj.totallen + "\n\n";
	}
	if (obj.event == 'OnFinish')
		tarea += "property:destfilename value:" + obj.destfilename + "\n\n";
	if (obj.event == 'OnError')
		tarea += "property:error value:" + obj.error + "\n\n";
	area.value = tarea;
}