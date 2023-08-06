/*
NETCLASS.HPP

Network panel plugin class header file

*/

#include "SFTPClient.h"

#ifndef __NETCLASS_HPP__
#define __NETCLASS_HPP__

#define REMOVE_FLAG 1

#include <farplug-wide.h>
#include <utils.h>

class NetPanel
{
private:
	void RemoveDups();
	void RemoveEmptyItems();
	void UpdateItems(int ShowOwners, int ShowLinks);
	int IsOwnersDisplayed(LPCTSTR ColumnTypes);
	int IsLinksDisplayed(LPCTSTR ColumnTypes);
	void ProcessRemoveKey();
	void ProcessSaveListKey();
	void ProcessPanelSwitchMenu();
	void SwitchToPanel(int NewPanelIndex);
	void FindSearchResultsPanel();
	void SaveListFile(const TCHAR *Path);
	bool IsCurrentFileCorrect(TCHAR **pCurFileName);

	PluginPanelItem *NetPanelItem;
	int NetItemsNumber;
	int LastOwnersRead;
	int LastLinksRead;
	int UpdateNotNeeded;

	bool busy = false;
	std::wstring CurDir;
    SftpClient* sc;
    bool connected = false;

public:
	NetPanel();
	~NetPanel();
	int PanelIndex;
	//    int OpenFrom;
	int GetFindData(PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode);
	void GetOpenPluginInfo(struct OpenPluginInfo *Info);
	int SetDirectory(const TCHAR *Dir, int OpMode);

	int
	PutFiles(struct PluginPanelItem *PanelItem, int ItemsNumber, int Move, const TCHAR *SrcPath, int OpMode);
	int
	GetFiles(struct PluginPanelItem *PanelItem, int ItemsNumber, int Move, const wchar_t **DestPath, int OpMode);
	HANDLE BeginPutFiles();
	void CommitPutFiles(HANDLE hRestoreScreen, int Success);
	int PutDirectoryContents(const TCHAR *Path);
	int PutOneFile(const TCHAR *SrcPath, PluginPanelItem &PanelItem);
	int PutOneFile(const TCHAR *FilePath);

	int SetFindList(const struct PluginPanelItem *PanelItem, int ItemsNumber);
	int ProcessEvent(int Event, void *Param);
	int ProcessKey(int Key, unsigned int ControlState);
	static bool GetFileInfoAndValidate(const TCHAR *FilePath, FAR_FIND_DATA *FindData, int Any);
	void IfOptCommonPanel(void);
	int DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
	int MakeDirectory(const wchar_t **Name,int OpMode);
};

#endif	/* __NETCLASS_HPP__ */
