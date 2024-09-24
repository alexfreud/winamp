/*

PlaylistPro build 005
Originally done by pjn123 (www.skinconsortium.com) for the Classic Pro plugin

*/

#include "lib/std.mi"
#include "lib/pldir.mi"
#include "attribs/init_Playlist.m"

Function resizeResults(int items);
Function doSearch(String input);
Function setSearchBox(boolean onOff);
Function clearSearchBox();
Function int getPlEntry(int search_item);
Function String replaceString(string baseString, string toreplace, string replacedby);

Global Group frameGroup, topbar;
Global Edit searchBox;
Global Text helpSearch, searchNews;// fakeText;
Global GuiObject fakeSB, searchXUI, searchButtonXui;
Global Button searchButton, clearButton, clearButtonText;
Global GuiList searchResults;
Global Boolean foundsomething;
Global int tn, h_tune;
Global String temptoken;
Global Windowholder plwh;
GLobal Timer refreshActiveCheck;
Global PopupMenu search_rc;

Global Container results_container;
Global Layout results_layout, main_layout;

System.onScriptLoaded() {
	frameGroup = getScriptGroup();
	topbar = frameGroup.findObject("PlaylistPro.topbar");
	plwh = frameGroup.findObject("wdh.playlist");
	
	searchBox = frameGroup.findObject("wasabi.edit.box");
	searchXUI = frameGroup.findObject("pl.search.edit");
	searchButton = frameGroup.findObject("pl.search.go");
	searchButtonXui = frameGroup.findObject("pl.search.go");
	fakeSB = frameGroup.findObject("pl.search.edit.rect");
	clearButton = frameGroup.findObject("pl.search.edit.clear");
	clearButtonText = frameGroup.findObject("pl.search.edit.clear.text");
	helpSearch = frameGroup.findObject("pl.search.edit.searchhelp");
	//fakeText = frameGroup.findObject("pl.search.go.text.fake");
	
	results_container = newDynamicContainer("searchresults");
	results_layout = results_container.getLayout("normal");
	searchResults = results_layout.findObject("PlaylistPro.list");
	searchResults.setFontSize(16);
	searchNews = results_layout.findObject("PlaylistPro.list.news");
	
	main_layout = getContainer("main").getLayout("normal");
	
	initAttribs_Playlist();
	
	//translation workaround
	/*searchButtonXui.setXmlParam("text", System.translate("Search"));
	searchButtonXui.setXmlParam("x", integerToString(-1*fakeText.getAutoWidth()-4));
	searchButtonXui.setXmlParam("w", integerToString(fakeText.getAutoWidth()+2));
	searchXUI.setXmlParam("w", integerToString(-1*fakeText.getAutoWidth()-9));*/

	refreshActiveCheck = new Timer;
	refreshActiveCheck.setDelay(100);
}
System.onScriptUnLoading() {
		delete refreshActiveCheck;
}

refreshActiveCheck.onTimer(){
	if((!System.isAppActive() || System.isMinimized()) && results_layout.isVisible() ) results_layout.hide();
}

results_layout.onSetVisible(boolean onOff){
	if(onOff){
		refreshActiveCheck.start();
	}
	else{
		refreshActiveCheck.stop();
	}
}

System.onShowLayout(Layout _layout){
	if(main_layout.isVisible()) results_layout.setXmlParam("ontop", "1");
}

frameGroup.onSetVisible(boolean onOff){
	if(!onOff) clearSearchBox();
}

searchButton.onLeftClick(){
	doSearch(searchBox.getText());
}

resizeResults(int items){
	/*results_layout.setTargetX(results_layout.getLeft());
	results_layout.setTargetY(results_layout.getTop());
	results_layout.setTargetW(results_layout.getWidth());
	results_layout.setTargetH(500);
	results_layout.setTargetSpeed(1);
	results_layout.gotoTarget();*/

	//items++; //temp add one extra for info... xx items found
	if(items>20) items=20;
	
	
	if(items>=1) h_tune=29;
	else h_tune=25;

	//results_layout.setXmlParam("h", integerToString(20+items*18));

	/*results_layout.setTargetX(results_layout.getLeft());
	results_layout.setTargetY(results_layout.getTop());
	results_layout.setTargetW(results_layout.getWidth());
	results_layout.setTargetH(h_tune+items*16);
	results_layout.setTargetSpeed(0.3);
	results_layout.gotoTarget();*/
	results_layout.resize(results_layout.getLeft(), results_layout.getTop(), results_layout.getWidth(), h_tune+items*16);
}

searchBox.onEnter(){
	doSearch(searchBox.getText());
}

doSearch(String input){
	if(input==""){
		clearSearchBox();
		return; 
	}
	
	//search history stuff
	String history = getPublicString("cPro.PlaylistPro.history", "");
	history = replaceString(history, input, "");
	history = replaceString(history, ";;", ";");
	if(strleft(history, 1)==";") history = strright(history, strlen(history)-1);
	if(strright(history, 1)==";") history = strleft(history, strlen(history)-1);
	if(history=="") history= input;
	else history= input+";"+history;
	
	String output = getToken(history, ";", 0);
	for(int i = 1; i<15; i++){
		if(getToken(history, ";", i)=="") break;
		output+=";"+getToken(history, ";", i);
	}
	setPublicString("cPro.PlaylistPro.history", history);


	int itemsfound = 0;
	input = strlower(input);
	
	results_layout.setXmlParam("x", integerToString(fakeSB.clientToScreenX(fakeSB.getLeft()-2)));
	results_layout.setXmlParam("y", integerToString(fakeSB.clientToScreenY(fakeSB.getTop() + fakeSB.getHeight())));
	results_layout.setXmlParam("w", integerToString(frameGroup.getWidth()-19));
	
	if(!results_layout.isVisible()) results_layout.setXmlParam("h", "1");

	searchResults.deleteAllItems();
	searchResults.scrollToItem(0);
	
	for(int i = 0; i<PlEdit.getNumTracks(); i++){
		foundsomething=false;

		for(tn = 0; tn<10; tn++){
			if(getToken(input, " ", tn)==""){
				break;
			}
		
			temptoken = getToken(input, " ", tn);
			if(strsearch(strlower(PlEdit.getTitle(i) + " " + PlEdit.getFileName(i)), temptoken)>=0){
				foundsomething=true;
			}
			else{
				foundsomething=false;
			}
			
			if(!foundsomething){
				break;
			}
		}
		
		if(foundsomething){
			itemsfound++;
			searchResults.addItem(integerToString(i+1)+". " + PlEdit.getTitle(i));
			if(itemsfound>500){
				searchNews.setText("Search result limited to 500 items");
				break;
			}
		}
	}

	if(itemsfound==0){
		searchNews.setText("Nothing was found");
	}
	else if(itemsfound<=500){
		searchNews.setText(System.translate("Items found: ") +integerToString(itemsfound));
	}

	if(!results_layout.isVisible()) results_layout.show();

	// Fix if always on top is enabled.. it just refresh the ontop ;)
	results_layout.setXmlParam("ontop", "1");

	resizeResults(itemsfound);
}

searchResults.onDoubleClick(Int itemnum){
	PlEdit.playTrack (getPlEntry(itemnum));
	setSearchBox(false);
}

searchResults.onRightClick(Int itemnum){
	search_rc = new PopupMenu;
	search_rc.addCommand("Move selected to top", 1, 0, 0);
	search_rc.addCommand("Move selected to bottom", 2, 0, 0);
	search_rc.addCommand("Move selected after current", 3, 0, 0);
	search_rc.addCommand("Move selected together", 4, 0, 0);
	search_rc.addSeparator();
	search_rc.addCommand("Remove selected from playlist", 5, 0, 0);
	int result = search_rc.popAtMouse();
	delete search_rc;
	
	int lastselected = searchResults.getFirstItemSelected();
	int itemcounter = 1;

	if(result==1){
		PlEdit.moveTo (getPlEntry(lastselected), 0);
		while(searchResults.getNextItemSelected(lastselected) != -1){
			lastselected = searchResults.getNextItemSelected(lastselected);
			PlEdit.moveTo (getPlEntry(lastselected), itemcounter);
			itemcounter++;
		}
		PlEdit.showTrack(0);
	}
	else if(result==2){
		PlEdit.moveTo (getPlEntry(lastselected), PlEdit.getNumTracks ()-1);
		while(searchResults.getNextItemSelected(lastselected) != -1){
			lastselected = searchResults.getNextItemSelected(lastselected);
			PlEdit.moveTo (getPlEntry(lastselected)-itemcounter, PlEdit.getNumTracks ()-1);
			itemcounter++;
		}
		PlEdit.showTrack(PlEdit.getNumTracks ()-1);
	}
	else if(result==3){
		int align = 0;
		int orignalPos = PlEdit.getCurrentIndex();
		int temp = getPlEntry(lastselected);
		if(PlEdit.getCurrentIndex() > temp) align++;

		if(orignalPos > getPlEntry(lastselected)) PlEdit.moveTo (getPlEntry(lastselected), PlEdit.getCurrentIndex());
		else  PlEdit.moveTo (getPlEntry(lastselected), PlEdit.getCurrentIndex()+1);
		
		while(searchResults.getNextItemSelected(lastselected) != -1){
			lastselected = searchResults.getNextItemSelected(lastselected);
			
			if(orignalPos != getPlEntry(lastselected)){
				if(orignalPos > getPlEntry(lastselected)){
					PlEdit.moveTo (getPlEntry(lastselected)-align, PlEdit.getCurrentIndex()+itemcounter);
				}
				else{
					PlEdit.moveTo (getPlEntry(lastselected), PlEdit.getCurrentIndex()+itemcounter+1);
				}

				temp = getPlEntry(lastselected);
				if(orignalPos > temp) align++;
				itemcounter++;
			}
		}
		PlEdit.showCurrentlyPlayingTrack();
	}
	else if(result==4){
		int startpos = getPlEntry(lastselected);
		while(searchResults.getNextItemSelected(lastselected) != -1){
			lastselected = searchResults.getNextItemSelected(lastselected);
			PlEdit.moveTo (getPlEntry(lastselected), startpos+itemcounter);
			itemcounter++;
		}
		PlEdit.showTrack(startpos);
	}
	else if(result==5){
		PlEdit.removeTrack (getPlEntry(lastselected));
		while(searchResults.getNextItemSelected(lastselected) != -1){
			lastselected = searchResults.getNextItemSelected(lastselected);
			PlEdit.removeTrack (getPlEntry(lastselected)-itemcounter);	
			itemcounter++;
		}
	}
	
	else return;
	
	//hides the search
	setSearchBox(false);
}

int getPlEntry(int search_item){
	return stringToInteger(getToken(searchResults.getItemLabel(search_item,0), ". ", 0))-1;
}

setSearchBox(boolean onOff){
	if(onOff){
		searchBox.show();
		clearButton.show();
		clearButtonText.show();
		helpSearch.hide();
		searchBox.setFocus();
	}
	else{
		searchBox.hide();
		clearButton.hide();
		clearButtonText.hide();
		helpSearch.show();
		results_layout.hide();
	}
}

fakeSB.onLeftButtonDown(int x, int y){
	setSearchBox(true);
}
fakeSB.onRightButtonDown(int x, int y){
	search_rc = new PopupMenu;
	search_rc.addCommand("** Search history **", 0, 0, 1);
	search_rc.addSeparator();
	
	String history = getPublicString("cPro.PlaylistPro.history", "");
	boolean historyfound=false;
	for(int i=0;i<15;i++){
		String historyitem = getToken(history, ";", i);
		if(historyitem==""){
			if(i>0) historyfound=true;
			break;
		}
		search_rc.addCommand(historyitem, i+1, 0, 0);
	}
	if(historyfound) search_rc.addSeparator();
	search_rc.addCommand("Clear History", 100, 0, 0);

	int result = search_rc.popAtMouse();
	delete search_rc;

	if(result>0 && result<100){
		setSearchBox(true);
		searchBox.setText(getToken(history, ";", result-1));
		searchButton.leftClick();
	}
	else if(result==100){
		setPublicString("cPro.PlaylistPro.history", "");
	}
}

clearSearchBox(){
	searchBox.setText("");
	setSearchBox(false);
}

main_layout.onMove(){
	if(results_layout.isVisible()){
		results_layout.setXmlParam("x", integerToString(fakeSB.clientToScreenX(fakeSB.getLeft()-2)));
		results_layout.setXmlParam("y", integerToString(fakeSB.clientToScreenY(fakeSB.getTop() + fakeSB.getHeight())));
		results_layout.setXmlParam("ontop", "1");
	}
}
main_layout.onResize(int x, int y, int w, int h){
	if(results_layout.isVisible()) clearSearchBox();
}

frameGroup.onResize(int x, int y, int w, int h){
	if(results_layout.isVisible()) clearSearchBox();
	
	//if(playlist_search_attib.getData()=="0") return;
	
	if(h<102 || playlist_search_attrib.getData()=="0"){
	
		if(!topbar.isVisible()) return; //Don't do the same code over and over

		topbar.hide();
		plwh.setXmlParam("y", "0");
		plwh.setXmlParam("h", "0");
	}
	else{
		if(topbar.isVisible()) return; //Don't do the same code over and over

		topbar.show();
		plwh.setXmlParam("y", "30");
		plwh.setXmlParam("h", "-30");
	}
}

clearButton.onLeftClick(){
	clearSearchBox();
}

playlist_search_attrib.onDataChanged(){
	frameGroup.onResize(frameGroup.getLeft(), frameGroup.getTop(), frameGroup.getWidth(), frameGroup.getHeight());
	//debug("abc " + getData());

	if (getData() == "0"){
		topbar.hide();
		plwh.setXMLParam("y", "0");
		plwh.setXMLParam("h", "0");
	}
	else if(getData() == "1"){
	
		topbar.show();
		plwh.setXMLParam("y", "30");
		plwh.setXMLParam("h", "-30");
		
	}
}



String replaceString(string baseString, string toreplace, string replacedby){
	if (toreplace == "") return baseString;
	string sf1 = strupper(baseString);
	string sf2 = strupper(toreplace);
	int i = strsearch(sf1, sf2);
	if (i == -1) return baseString;
	string left = "", right = "";
	if (i != 0) left = strleft(baseString, i);
	if (strlen(basestring) - i - strlen(toreplace) != 0) {
		right = strright(basestring, strlen(basestring) - i - strlen(toreplace));
	}
	return left + replacedby + right;
}