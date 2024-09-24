#ifndef _SENDTO_H_
#define _SENDTO_H_

class SendToMenu
{
public:
    SendToMenu();
    ~SendToMenu();
    void buildmenu(HMENU hMenu, int type, int simple, int true_type=0, int start=0x1500, int len=0x1500);
    int isourcmd(int id); 
    int handlecmd(HWND hwndParent, int id, int type, void *data); // returns 1 if handled, 0 if not

    void onAddItem(mlAddToSendToStruct *ptr);
	void onAddItem(mlAddToSendToStructW *ptr);

	void startBranch();
	void addItemToBranch(mlAddToSendToStructW *ptr);
	void endBranch(const wchar_t *name);

private:
    HMENU _hm;
	HMENU branch;
	int branch_pos;
    int _pos,_len,_start;
    int m_start, m_len;
    int m_addtolibrary;
	int activePlaylist;
    int plugin_start, plugin_len;
};

#endif//_SENDTO_H_