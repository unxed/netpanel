/*
NETCLASS.CPP

Network panel plugin class implementation

*/

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "NetPanel.hpp"


std::wstring host = L"test.rebex.net";
std::wstring user = L"demo";
std::wstring pass = L"password";
std::wstring newdir;


LONG_PTR WINAPI ShowDialogProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	fprintf(stderr, "DIALOG PROC %i\n", Msg);

	switch (Msg) {
		case 9: // Key pressed, update vars
//		case DN_BTNCLICK:

			int len;

			len = (int)Info.SendDlgMessage(hDlg, DM_GETTEXTPTR, 1, 0);
			host.clear();
			host.resize(len);
			Info.SendDlgMessage(hDlg, DM_GETTEXTPTR, 1, (LONG_PTR)host.data());

			len = (int)Info.SendDlgMessage(hDlg, DM_GETTEXTPTR, 3, 0);
			user.clear();
			user.resize(len);
			Info.SendDlgMessage(hDlg, DM_GETTEXTPTR, 3, (LONG_PTR)user.data());

			len = (int)Info.SendDlgMessage(hDlg, DM_GETTEXTPTR, 5, 0);
			pass.clear();
			pass.resize(len);
			Info.SendDlgMessage(hDlg, DM_GETTEXTPTR, 5, (LONG_PTR)pass.data());

			fprintf(stderr, "DIALOG: host: %ls\n", host.c_str());
			fprintf(stderr, "DIALOG: user: %ls\n", user.c_str());
			fprintf(stderr, "DIALOG: pass: %ls\n", pass.c_str());

			break;
	}

	return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}

LONG_PTR WINAPI ShowDialogProcMkDir(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	fprintf(stderr, "DIALOG PROC %i\n", Msg);

	switch (Msg) {
		case 9: // Key pressed, update vars
//		case DN_BTNCLICK:

			int len;

			len = (int)Info.SendDlgMessage(hDlg, DM_GETTEXTPTR, 1, 0);
			newdir.clear();
			newdir.resize(len);
			Info.SendDlgMessage(hDlg, DM_GETTEXTPTR, 1, (LONG_PTR)newdir.data());

			fprintf(stderr, "DIALOG 2: newdir: %ls\n", newdir.c_str());

			break;
	}

	return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}

NetPanel::NetPanel()
{
	
	fprintf(stderr, "NetPanel\n");

    FarDialogItem DialogItems[] = {
        {DI_TEXT, 5, 1, 0, 0, 0, 0, 0, 0, L"Host:"},
        {DI_EDIT, 5, 2, 45, 0, 0, 0, 0, 0, host.c_str()},
        {DI_TEXT, 5, 3, 0, 0, 0, 0, 0, 0, L"Login:"},
        {DI_EDIT, 5, 4, 45, 0, 0, 0, 0, 0, user.c_str()},
        {DI_TEXT, 5, 5, 0, 0, 0, 0, 0, 0, L"Password:"},
        {DI_PSWEDIT, 5, 6, 45, 0, 0, 0, 0, 0, pass.c_str()},
        {DI_BUTTON, 0, 8, 0, 0, 0, 0, DIF_CENTERGROUP, 1, L"OK"},
        {DI_BUTTON, 0, 8, 0, 0, 0, 0, DIF_CENTERGROUP, 0, L"Cancel"}
    };
    size_t DlgData = 0;
    HANDLE hDlg = Info.DialogInit(-1, -1, -1, 50, 10, L"Dialog", DialogItems,
    	sizeof(DialogItems) / sizeof(DialogItems[0]), 0, 0, ShowDialogProc, DlgData);

	fprintf(stderr, "dialog init\n");

    if (hDlg != INVALID_HANDLE_VALUE)
    {
		fprintf(stderr, "dialog init ok, try to run\n");
        Info.DialogRun(hDlg);

		//fprintf(stderr, "checking ok button\n");

		//if (Info.SendDlgMessage(hDlg, DM_GETCHECK, 6, 0) == BSTATE_CHECKED) // OK button
		{

			//fprintf(stderr, "ok button detected\n");

			/*
			int Size = Info.Control(PANEL_PASSIVE, DM_GETTEXTPTR, 0, (LONG_PTR)0);
			wchar_t* temp = new wchar_t[Size];
			Info.Control(PANEL_PASSIVE, DM_GETTEXTPTR, Size, (LONG_PTR)temp);
			user = temp;
			fprintf(stderr, "user set as %ls\n", u);
			

			Info.SendDlgMessage(hDlg, DM_GETTEXTPTR, 1, (LONG_PTR)host.c_str());
			*/
		}

        Info.DialogFree(hDlg);
    }

	LastOwnersRead = FALSE;
	LastLinksRead = FALSE;
	UpdateNotNeeded = FALSE;
	NetPanelItem = NULL;
	NetItemsNumber = 0;
	PanelIndex = CurrentCommonPanel;
	IfOptCommonPanel();

	// #DEBUG

	fprintf(stderr, "&&&2 %s\n", Wide2MB(host.c_str()).c_str());

    sc = new SftpClient();

	fprintf(stderr, "&1 %s\n", Wide2MB((std::wstring(user) + L"\@" + host).c_str()).c_str());

    if (!sc->openApp("sftp", Wide2MB((std::wstring(user) + L"\@" + host).c_str()).c_str())) {
        std::cerr << "SFTPWRAP: Failed to connect (1)" << std::endl;
		fprintf(stderr, "&3\n");
		Info.Control(this, FCTL_CLOSEPLUGIN, 0, 0);
		fprintf(stderr, "&4\n");
    } else {
		if (!pass.empty()) {
								   
			fprintf(stderr, "&4.10\n");

		    if (sc->waitFor("assword:", {"This key is not known by any other names"}).empty()) {

				fprintf(stderr, "&4.11\n");

			    if (!sc->sendCommand("yes\n")) {
			        std::cerr << "SFTPWRAP: Failed to connect (2)" << std::endl;
					Info.Control(this, FCTL_CLOSEPLUGIN, 0, 0);
				} else {
					fprintf(stderr, "&4.13\n");

				    if (sc->waitFor("Connected to", {
				    	"assword:"
			   		}).empty()) {
						// proceed to sending password
						fprintf(stderr, "&4.14\n");
					} else {
						// no need to send password
						connected = true;
						fprintf(stderr, "&4.15\n");
					}
				}
		    }

			fprintf(stderr, "&4.20 %i\n", connected);

			if (!connected) {
				fprintf(stderr, "&4.21 %i\n", connected);
			    if (!sc->sendCommand(Wide2MB((pass + L"\n").c_str()).c_str())) {
					fprintf(stderr, "&4.22 %i\n", connected);
			        std::cerr << "SFTPWRAP: Failed to connect (4)" << std::endl;
					Info.Control(this, FCTL_CLOSEPLUGIN, 0, 0);
			    } else {
					fprintf(stderr, "&4.23 %i\n", connected);

				    if (sc->waitFor("Connected to", {
						"assword:",
				    	"Permission denied, please try again."
			   		}).empty()) {
				        std::cerr << "SFTPWRAP: Failed to connect (5)" << std::endl;
						Info.Control(this, FCTL_CLOSEPLUGIN, 0, 0);
					} else {
						fprintf(stderr, "&4.24 %i\n", connected);
						connected = true;
					}
				}
			}

			fprintf(stderr, "&4.40\n");

		} else {

			fprintf(stderr, "&4.50\n");

		    if (sc->waitFor("Connected to", {
				"assword:", // key known, password asked (and we have none)
		    	"This key is not known by any other names" // key not known
	   		}).empty()) {

				fprintf(stderr, "&4.60 %i\n", connected);

	            // if key not known
			    if (!sc->sendCommand("yes\n")) {
			        std::cerr << "SFTPWRAP: Failed to connect (6)" << std::endl;
					Info.Control(this, FCTL_CLOSEPLUGIN, 0, 0);
				} else {
				    if (sc->waitFor("Connected to", {"assword:"}).empty()) {
						// password asked, but we have no one
				        std::cerr << "SFTPWRAP: Failed to connect (7)" << std::endl;
						Info.Control(this, FCTL_CLOSEPLUGIN, 0, 0);
					} else {
						connected = true;
					}
				}
		    } else {
				connected = true;
			}
		}    	

		fprintf(stderr, "&4.90 %i\n", connected);
    }
}

NetPanel::~NetPanel()
{
	fprintf(stderr, "~NetPanel\n");
	if (!StartupOptCommonPanel)
		FreePanelItems(NetPanelItem, NetItemsNumber);
}

int NetPanel::GetFindData(PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode)
{
	fprintf(stderr, "GetFindData\n");
	IfOptCommonPanel();
	int Size = Info.Control(this, FCTL_GETCOLUMNTYPES, 0, 0);
	wchar_t *ColumnTypes = new wchar_t[Size];
	Info.Control(this, FCTL_GETCOLUMNTYPES, Size, (LONG_PTR)ColumnTypes);
	UpdateItems(IsOwnersDisplayed(ColumnTypes), IsLinksDisplayed(ColumnTypes));
	fprintf(stderr, "*18.2\n");
	delete[] ColumnTypes;
	*pPanelItem = NetPanelItem;
	*pItemsNumber = NetItemsNumber;

	fprintf(stderr, "*18.2.1\n");

	return (TRUE);
}

void NetPanel::GetOpenPluginInfo(struct OpenPluginInfo *Info)
{
//	fprintf(stderr, "GetOpenPluginInfo\n");
	Info->StructSize = sizeof(*Info);
//	Info->Flags =
//			OPIF_USEFILTER | OPIF_USESORTGROUPS | OPIF_USEHIGHLIGHTING | OPIF_ADDDOTS | OPIF_SHOWNAMESONLY;

//	Info->Flags =
//			OPIF_USEFILTER | OPIF_USESORTGROUPS | OPIF_USEHIGHLIGHTING | OPIF_REALNAMES;

	Info->Flags = OPIF_SHOWPRESERVECASE | OPIF_USEHIGHLIGHTING;

	fprintf(stderr, "*21\n");

	if (!Opt.SafeModePanel)
		Info->Flags|= OPIF_REALNAMES;

	Info->HostFile = NULL;
	Info->CurDir = L".";

	Info->Format = (TCHAR *)GetMsg(MTempPanel);

	static TCHAR Title[100] = {};
#define PANEL_MODE (Opt.SafeModePanel ? _T("(R) ") : _T(""))
	if (StartupOptCommonPanel)
		FSF.snprintf(Title, ARRAYSIZE(Title) - 1, GetMsg(MTempPanelTitleNum), PANEL_MODE, PanelIndex);
	else
		FSF.snprintf(Title, ARRAYSIZE(Title) - 1, _T(" %s%s "), PANEL_MODE, GetMsg(MTempPanel));
#undef PANEL_MODE

	Info->PanelTitle = Title;

	fprintf(stderr, "*22\n");

	static struct PanelMode PanelModesArray[10];
	PanelModesArray[4].FullScreen =
			(StartupOpenFrom == OPEN_COMMANDLINE) ? Opt.FullScreenPanel : StartupOptFullScreenPanel;
	PanelModesArray[4].ColumnTypes = Opt.ColumnTypes;
	PanelModesArray[4].ColumnWidths = Opt.ColumnWidths;
	PanelModesArray[4].StatusColumnTypes = Opt.StatusColumnTypes;
	PanelModesArray[4].StatusColumnWidths = Opt.StatusColumnWidths;
	PanelModesArray[4].CaseConversion = TRUE;

	fprintf(stderr, "*23\n");

	Info->PanelModesArray = PanelModesArray;
	Info->PanelModesNumber = ARRAYSIZE(PanelModesArray);
	Info->StartPanelMode = _T('4');
	static struct KeyBarTitles KeyBar;
	memset(&KeyBar, 0, sizeof(KeyBar));
	KeyBar.Titles[7 - 1] = (TCHAR *)GetMsg(MF7);
	if (StartupOptCommonPanel)
		KeyBar.AltShiftTitles[12 - 1] = (TCHAR *)GetMsg(MAltShiftF12);
	KeyBar.AltShiftTitles[2 - 1] = (TCHAR *)GetMsg(MAltShiftF2);
	KeyBar.AltShiftTitles[3 - 1] = (TCHAR *)GetMsg(MAltShiftF3);
	Info->KeyBar = &KeyBar;

	fprintf(stderr, "*24\n");
}

int NetPanel::SetDirectory(const TCHAR *Dir, int OpMode)
{
	if (!connected) {
		Info.Control(this, FCTL_CLOSEPLUGIN, 0, 0);
		return 0;
	}

	fprintf(stderr, "SetDirectory %ls, mode: %i\n", Dir, OpMode);
	sc->cd(EscapeQuotes(Wide2MB(Dir)));

	CurDir = CurDir + L"/" + Dir;
	fprintf(stderr, "CurDir: %ls\n", CurDir.c_str());

	if (CurDir == L"/..") {
        std::wcout << L"Error: too many \"..\" (1)\n";
		Info.Control(this, FCTL_CLOSEPLUGIN, 0, 0);
		return (FALSE);
	}

    std::vector<std::wstring> folders;
    std::wstringstream ss(CurDir);
    std::wstring token;

    while (std::getline(ss, token, L'/')) {
        if (token == L"..") {
            if (folders.empty()) {
                // Если мы пытаемся выйти за пределы папки, в которую попали при установке соединения,
                // завершим работу плагина.
                std::wcout << L"Error: too many \"..\" (2)\n";
				Info.Control(this, FCTL_CLOSEPLUGIN, 0, 0);
				return (FALSE);
            }
            folders.pop_back();
        } else {
            folders.push_back(token);
        }
    }

    std::wstringstream result;
    for (size_t i = 0; i < folders.size(); i++) {
        result << folders[i];
        if (i != folders.size() - 1) {
            result << L"/";
        }
    }

    CurDir = result.str();

	fprintf(stderr, "Optimized CurDir: %ls\n", CurDir.c_str());

	/*
	IfOptCommonPanel();
	int Size = Info.Control(this, FCTL_GETCOLUMNTYPES, 0, 0);
	wchar_t *ColumnTypes = new wchar_t[Size];
	Info.Control(this, FCTL_GETCOLUMNTYPES, Size, (LONG_PTR)ColumnTypes);
	UpdateItems(IsOwnersDisplayed(ColumnTypes), IsLinksDisplayed(ColumnTypes));
	*/
	UpdateItems(0, 0);

	fprintf(stderr, "*18.3\n");

	//PanelRedrawInfo ri = {};
	//Info.Control(this, FCTL_REDRAWPANEL, 0, (LONG_PTR)&ri);

	struct PanelInfo PInfo;
	Info.Control(this, FCTL_GETPANELINFO, 0, (LONG_PTR)&PInfo);
	Info.Control(this, FCTL_UPDATEPANEL, 0, 0);
	Info.Control(this, FCTL_REDRAWPANEL, 0, 0);
	Info.Control(PANEL_PASSIVE, FCTL_GETPANELINFO, 0, (LONG_PTR)&PInfo);
	if (PInfo.PanelType == PTYPE_QVIEWPANEL) {
		Info.Control(PANEL_PASSIVE, FCTL_UPDATEPANEL, 0, 0);
		Info.Control(PANEL_PASSIVE, FCTL_REDRAWPANEL, 0, 0);
	}

#if 0
	if ((OpMode & OPM_FIND) /* || lstrcmp(Dir,_T("\\"))==0*/)
		return (FALSE);
	if (lstrcmp(Dir, WGOOD_SLASH) == 0)
		Info.Control(this, FCTL_CLOSEPLUGIN, 0, 0);
	else
		Info.Control(this, FCTL_CLOSEPLUGIN, 0, (LONG_PTR)Dir);
#endif

	return (TRUE);
}

int NetPanel::PutFiles(struct PluginPanelItem *PanelItem, int ItemsNumber, int, const TCHAR *SrcPath, int)
{
	fprintf(stderr, "PutFiles internal / %ls / %ls\n", SrcPath, PanelItem->FindData.lpwszFileName);

	/*
	int Size = Info.Control(PANEL_PASSIVE, FCTL_GETPANELDIR, 0, (LONG_PTR)0);
	wchar_t* lpwszCurDir = new wchar_t[Size];
	Info.Control(PANEL_PASSIVE, FCTL_GETPANELDIR, Size, (LONG_PTR)lpwszCurDir);

	size_t len = Info.Control(PANEL_ACTIVE, FCTL_GETPANELITEM, 1, 0); // вместо 1 нужно число выделенных, если их много
	PluginPanelItem *pi;
	if (len >= sizeof(PluginPanelItem)) {
		pi = (PluginPanelItem *)calloc(1, len + 0x20);
		if (pi == nullptr) {
			fprintf(stderr, "GetItems: no memory\n");
			exit(-10);
		}

		Info.Control(PANEL_ACTIVE, FCTL_GETPANELITEM, 1, (LONG_PTR)(void *)pi); // вместо 1 тоже число выделенных тут
		//_items_to_free.push_back(pi);
	}

	fprintf(stderr, "passive panel dir: %ls\n", lpwszCurDir);
	*/

    //std::wstring path = (std::wstring(lpwszCurDir) + L"/" + pi->FindData.lpwszFileName);
	std::wstring path = (std::wstring(SrcPath) + L"/" + PanelItem->FindData.lpwszFileName);

	fprintf(stderr, "mp: %ls\n", path.c_str());

	// some systems require mkdir first
    std::filesystem::path mypath(path);
	fprintf(stderr, "*05\n");
    if (std::filesystem::is_directory(mypath)) {
		fprintf(stderr, "*10\n");
	    if (!sc->sendCommand((std::string("mkdir \"") + EscapeQuotes(Wide2MB(PanelItem->FindData.lpwszFileName)) + "\"\n").c_str())) {
			fprintf(stderr, "*20\n");
	        return false;
	    }
	    sc->waitFor("sftp>", {});
		// #debug #fixme а что если ошибка?
    }

    std::wstring command =
        // a = append, f = flush, p = copy permissions, R = recursive
    	std::wstring(L"put -afpR \"") +
        EscapeQuotes(path.c_str()) +
    	L"\"\n";

    fprintf(stderr, "running command: [%ls]\n", command.c_str());

    if (!sc->sendCommand(Wide2MB(command.c_str()).c_str())) {
        return false;
    }

    // Проверим, что команда выполнена успешно, ожидая приглашения "sftp>"
    // stat remote: No such file or directory
    // realpath /home/u27646/tmp/test: No such file
    int res = !sc->waitFor("sftp>", {"stat remote: No such file or directory", "No such file"}).empty();
    if (!res) {

    	// не можем сделать resume. ну ладно, не надо! см. !!!

	    std::wstring command =
	    	std::wstring(L"put -fpR \"") + // !!! разница в этой строке
	        EscapeQuotes(path.c_str()) +
	    	L"\"\n";

	    fprintf(stderr, "running command: [%ls]\n", command.c_str());

	    if (!sc->sendCommand(Wide2MB(command.c_str()).c_str())) {
	        return false;
	    }

		res = !sc->waitFor("sftp>", {}).empty();
	}

    return res;

	Info.Control(PANEL_PASSIVE, FCTL_UPDATEPANEL, 0, 0);
	Info.Control(PANEL_PASSIVE, FCTL_REDRAWPANEL, 0, 0);

//    delete[] lpwszCurDir; // #DEBUG #fixme: crashes


	/*
	UpdateNotNeeded = FALSE;

	HANDLE hScreen = BeginPutFiles();
	for (int i = 0; i < ItemsNumber; i++) {
		if (!PutOneFile(SrcPath, PanelItem[i])) {
			CommitPutFiles(hScreen, FALSE);
			return FALSE;
		}
	}
	CommitPutFiles(hScreen, TRUE);
	*/

	return (1);
}

int NetPanel::GetFiles(struct PluginPanelItem *PanelItem, int ItemsNumber, int, const wchar_t **DestPath, int)
{
	fprintf(stderr, "GetFiles internal / %ls / %ls\n", *DestPath, PanelItem->FindData.lpwszFileName);

	int Size = Info.Control(PANEL_PASSIVE, FCTL_GETPANELDIR, 0, (LONG_PTR)0);
	wchar_t* lpwszCurDir = new wchar_t[Size];
	Info.Control(PANEL_PASSIVE, FCTL_GETPANELDIR, Size, (LONG_PTR)lpwszCurDir);

	fprintf(stderr, "passive panel dir: %ls\n", lpwszCurDir);

    std::wstring command =
    	std::wstring(L"get -afpR \"") +
    	EscapeQuotes(PanelItem->FindData.lpwszFileName) +
    	L"\" \"" +
    	EscapeQuotes(lpwszCurDir) +
    	L"\"\n";

    if (!sc->sendCommand(Wide2MB(command.c_str()).c_str())) {
        return false;
    }
    // Проверим, что команда выполнена успешно, ожидая приглашения "sftp>"
    return !sc->waitFor("sftp>", {}).empty();

	Info.Control(PANEL_PASSIVE, FCTL_UPDATEPANEL, 0, 0);
	Info.Control(PANEL_PASSIVE, FCTL_REDRAWPANEL, 0, 0);

//    delete[] lpwszCurDir; // #DEBUG #fixme: crashes

}

HANDLE NetPanel::BeginPutFiles()
{
	fprintf(stderr, "BeginPutFiles\n");
	IfOptCommonPanel();
	Opt.SelectedCopyContents = Opt.CopyContents;

	HANDLE hScreen = Info.SaveScreen(0, 0, -1, -1);
	const TCHAR *MsgItems[] = {GetMsg(MTempPanel), GetMsg(MTempSendFiles)};
	Info.Message(Info.ModuleNumber, 0, NULL, MsgItems, ARRAYSIZE(MsgItems), 0);
	return hScreen;
}

static inline int cmp_names(const WIN32_FIND_DATA &wfd, const FAR_FIND_DATA &ffd)
{
#define FILE_NAME lpwszFileName
	return lstrcmp(wfd.cFileName, FSF.PointToName(ffd.FILE_NAME));
}

int NetPanel::PutDirectoryContents(const TCHAR *Path)
{
	fprintf(stderr, "PutDirectoryContents\n");
	if (Opt.SelectedCopyContents == 2) {
		const TCHAR *MsgItems[] = {GetMsg(MWarning), GetMsg(MCopyContensMsg)};
		Opt.SelectedCopyContents = !Info.Message(Info.ModuleNumber, FMSG_MB_YESNO, _T("Config"), MsgItems,
				ARRAYSIZE(MsgItems), 0);
	}
	if (Opt.SelectedCopyContents) {
		FAR_FIND_DATA *DirItems;
		int DirItemsNumber;
		if (!Info.GetDirList(Path, &DirItems, &DirItemsNumber)) {
			FreePanelItems(NetPanelItem, NetItemsNumber);
			NetItemsNumber = 0;
			return FALSE;
		}
		struct PluginPanelItem *NewPanelItem = (struct PluginPanelItem *)realloc(NetPanelItem,
				sizeof(*NetPanelItem) * (NetItemsNumber + DirItemsNumber));
		if (NewPanelItem == NULL)
			return FALSE;
		NetPanelItem = NewPanelItem;
		memset(&NetPanelItem[NetItemsNumber], 0, sizeof(*NetPanelItem) * DirItemsNumber);

		for (int i = 0; i < DirItemsNumber; i++) {
			struct PluginPanelItem *CurPanelItem = &NetPanelItem[NetItemsNumber];
			CurPanelItem->UserData = NetItemsNumber;
			NetItemsNumber++;

			CurPanelItem->FindData = DirItems[i];
			CurPanelItem->FindData.lpwszFileName = wcsdup(DirItems[i].lpwszFileName);
		}
		Info.FreeDirList(DirItems
				,
				DirItemsNumber
		);
	}
	return TRUE;
}

int NetPanel::PutOneFile(const TCHAR *SrcPath, PluginPanelItem &PanelItem)
{
	fprintf(stderr, "PutOneFile\n");
	struct PluginPanelItem *NewPanelItem =
			(struct PluginPanelItem *)realloc(NetPanelItem, sizeof(*NetPanelItem) * (NetItemsNumber + 1));
	if (NewPanelItem == NULL)
		return FALSE;
	NetPanelItem = NewPanelItem;
	struct PluginPanelItem *CurPanelItem = &NetPanelItem[NetItemsNumber];
	memset(CurPanelItem, 0, sizeof(*CurPanelItem));
	CurPanelItem->FindData = PanelItem.FindData;
	CurPanelItem->UserData = NetItemsNumber;

	CurPanelItem->FindData.lpwszFileName = reinterpret_cast<wchar_t *>(
			malloc((lstrlen(SrcPath) + 1 + lstrlen(PanelItem.FindData.lpwszFileName) + 1) * sizeof(wchar_t)));
	if (CurPanelItem->FindData.lpwszFileName == NULL)
		return FALSE;

	lstrcpy((TCHAR *)CurPanelItem->FindData.FILE_NAME, SrcPath);
	if (*SrcPath) {
		FSF.AddEndSlash((TCHAR *)CurPanelItem->FindData.FILE_NAME);
	}
	lstrcat((TCHAR *)CurPanelItem->FindData.FILE_NAME, PanelItem.FindData.FILE_NAME);
	NetItemsNumber++;
	if (Opt.SelectedCopyContents && (CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return PutDirectoryContents(CurPanelItem->FindData.FILE_NAME);
	return TRUE;
}

int NetPanel::PutOneFile(const TCHAR *FilePath)
{
	fprintf(stderr, "PutOneFile - 2\n");
	struct PluginPanelItem *NewPanelItem =
			(struct PluginPanelItem *)realloc(NetPanelItem, sizeof(*NetPanelItem) * (NetItemsNumber + 1));
	if (NewPanelItem == NULL)
		return FALSE;
	NetPanelItem = NewPanelItem;
	struct PluginPanelItem *CurPanelItem = &NetPanelItem[NetItemsNumber];
	memset(CurPanelItem, 0, sizeof(*CurPanelItem));
	CurPanelItem->UserData = NetItemsNumber;
	if (GetFileInfoAndValidate(FilePath, &CurPanelItem->FindData, Opt.AnyInPanel)) {
		NetItemsNumber++;
		if (Opt.SelectedCopyContents && (CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			return PutDirectoryContents(CurPanelItem->FindData.FILE_NAME);
	}
	return TRUE;
}

void NetPanel::CommitPutFiles(HANDLE hRestoreScreen, int Success)
{
	fprintf(stderr, "CommitPutFiles\n");
	if (Success)
		RemoveDups();
	Info.RestoreScreen(hRestoreScreen);
}

/*
int NetPanel::SetFindList(const struct PluginPanelItem *PanelItem, int ItemsNumber)
{
	fprintf(stderr, "SetFindList\n");
	HANDLE hScreen = BeginPutFiles();
	FindSearchResultsPanel();
	FreePanelItems(NetPanelItem, NetItemsNumber);
	NetItemsNumber = 0;
	NetPanelItem = (PluginPanelItem *)malloc(sizeof(PluginPanelItem) * ItemsNumber);
	if (NetPanelItem) {
		NetItemsNumber = ItemsNumber;
		memset(NetPanelItem, 0, NetItemsNumber * sizeof(*NetPanelItem));
		for (int i = 0; i < ItemsNumber; ++i) {
			NetPanelItem[i].UserData = i;
			NetPanelItem[i].FindData = PanelItem[i].FindData;

			if (NetPanelItem[i].FindData.lpwszFileName)
				NetPanelItem[i].FindData.lpwszFileName = wcsdup(NetPanelItem[i].FindData.lpwszFileName);
		}
	}
	CommitPutFiles(hScreen, TRUE);
	UpdateNotNeeded = TRUE;
	return (TRUE);
}
*/

void NetPanel::FindSearchResultsPanel()
{
	fprintf(stderr, "FindSearchResultsPanel\n");
	if (StartupOptCommonPanel) {
		if (!Opt.NewPanelForSearchResults)
			IfOptCommonPanel();
		else {
			int SearchResultsPanel = -1;
			for (int i = 0; i < COMMONPANELSNUMBER; i++) {
				if (CommonPanels[i].ItemsNumber == 0) {
					SearchResultsPanel = i;
					break;
				}
			}
			if (SearchResultsPanel < 0) {
				// all panels are full - use least recently used panel
				SearchResultsPanel = Opt.LastSearchResultsPanel++;
				if (Opt.LastSearchResultsPanel >= COMMONPANELSNUMBER)
					Opt.LastSearchResultsPanel = 0;
			}
			if (PanelIndex != SearchResultsPanel) {
				CommonPanels[PanelIndex].Items = NetPanelItem;
				CommonPanels[PanelIndex].ItemsNumber = NetItemsNumber;
				PanelIndex = SearchResultsPanel;
				NetPanelItem = CommonPanels[PanelIndex].Items;
				NetItemsNumber = CommonPanels[PanelIndex].ItemsNumber;
			}
			CurrentCommonPanel = PanelIndex;
		}
	}
}

int _cdecl SortListCmp(const void *el1, const void *el2, void *userparam)
{
	PluginPanelItem *NetPanelItem = reinterpret_cast<PluginPanelItem *>(userparam);
	int idx1 = *reinterpret_cast<const int *>(el1);
	int idx2 = *reinterpret_cast<const int *>(el2);
	int res = lstrcmp(NetPanelItem[idx1].FindData.FILE_NAME, NetPanelItem[idx2].FindData.FILE_NAME);
	if (res == 0) {
		if (idx1 < idx2)
			return -1;
		else if (idx1 == idx2)
			return 0;
		else
			return 1;
	} else
		return res;
}

void NetPanel::RemoveDups()
{
	fprintf(stderr, "RemoveDups\n");
	int *indices = reinterpret_cast<int *>(malloc(NetItemsNumber * sizeof(int)));
	if (indices == NULL)
		return;
	for (int i = 0; i < NetItemsNumber; i++)
		indices[i] = i;
	FSF.qsortex(indices, NetItemsNumber, sizeof(int), SortListCmp, NetPanelItem);
	for (int i = 0; i + 1 < NetItemsNumber; i++)
		if (lstrcmp(NetPanelItem[indices[i]].FindData.FILE_NAME,
					NetPanelItem[indices[i + 1]].FindData.FILE_NAME)
				== 0)
			NetPanelItem[indices[i + 1]].Flags|= REMOVE_FLAG;
	free(indices);
	RemoveEmptyItems();
}

void NetPanel::RemoveEmptyItems()
{
	fprintf(stderr, "RemoveEmptyItems\n");
	int EmptyCount = 0;
	struct PluginPanelItem *CurItem = NetPanelItem;
	for (int i = 0; i < NetItemsNumber; i++, CurItem++)
		if (CurItem->Flags & REMOVE_FLAG) {
			if (CurItem->Owner) {
				free((void *)CurItem->Owner);
				CurItem->Owner = NULL;
			}

			if (CurItem->FindData.lpwszFileName) {
				free((wchar_t *)CurItem->FindData.lpwszFileName);
				CurItem->FindData.lpwszFileName = NULL;
			}

			EmptyCount++;
		} else if (EmptyCount) {
			CurItem->UserData-= EmptyCount;
			*(CurItem - EmptyCount) = *CurItem;
			memset(CurItem, 0, sizeof(*CurItem));
		}

	NetItemsNumber-= EmptyCount;
	if (EmptyCount > 1)
		NetPanelItem =
				(struct PluginPanelItem *)realloc(NetPanelItem, sizeof(*NetPanelItem) * (NetItemsNumber + 1));

	if (StartupOptCommonPanel) {
		CommonPanels[PanelIndex].Items = NetPanelItem;
		CommonPanels[PanelIndex].ItemsNumber = NetItemsNumber;
	}
}

//bool first_call = true;

void NetPanel::UpdateItems(int ShowOwners, int ShowLinks)
{

//	if (!first_call) { return; }
//	first_call = false;

	if (!connected) {
		Info.Control(this, FCTL_CLOSEPLUGIN, 0, 0);
		return;
	}

	fprintf(stderr, "UpdateItems\n");
	HANDLE hScreen = Info.SaveScreen(0, 0, -1, -1);

	// #DEBUG
	struct PluginPanelItem *NewPanelItem;
	struct PluginPanelItem *CurPanelItem;

	fprintf(stderr, "*1\n");

	if (NetItemsNumber) {
		fprintf(stderr, "*2\n");
		FreePanelItems(NetPanelItem, NetItemsNumber);
		NetPanelItem = NULL;
		NetItemsNumber = 0;
		fprintf(stderr, "*3\n");
	}

	fprintf(stderr, "*4\n");

	busy = true;

//	for (int i=0; i<3; i++) {

    std::vector<FileInfo> files = sc->ls();

	fprintf(stderr, "*4.5\n");

    for (const auto& file : files) {

		fprintf(stderr, "*5 %s %li\n", file.path.c_str(), sizeof(*NetPanelItem) * (NetItemsNumber + 1));

//		NewPanelItem = (struct PluginPanelItem *)realloc(NetPanelItem,
//				sizeof(*NetPanelItem) * (NetItemsNumber + 1));

PluginPanelItem* newPanelItem = static_cast<PluginPanelItem*>(realloc(NetPanelItem, sizeof(*NetPanelItem) * (NetItemsNumber + 1)));
if (newPanelItem == NULL) {
    if (NetPanelItem == NULL) { free(NetPanelItem); }

	fprintf(stderr, "*5.5\n");

    // обработка ошибки выделения памяти
} else {
    NetPanelItem = newPanelItem;
}

		fprintf(stderr, "*5.10\n");

		fprintf(stderr, "*5.15\n");
		memset(&NetPanelItem[NetItemsNumber], 0, sizeof(*NetPanelItem) * 1);

		fprintf(stderr, "*5.20\n");

		CurPanelItem = &NetPanelItem[NetItemsNumber];
		CurPanelItem->UserData = NetItemsNumber;
		NetItemsNumber++;

		fprintf(stderr, "*5.30\n");

		CurPanelItem->FindData.lpwszFileName = wcsdup(file.path.wstring().c_str());

		fprintf(stderr, "*5.40\n");

		CurPanelItem->FindData.dwFileAttributes = 0;
		if (file.is_directory) {
			CurPanelItem->FindData.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
		}

		// fprintf(stderr, "+*+ [%ls]\n", file.path.wstring().c_str());
		// works ok
	}

	fprintf(stderr, "*17\n");

	busy = false;

	RemoveEmptyItems(); // !!! #DEBUG отсутствие этой штуки давало вылеты при попытке смены папки

	// /DEBUG
	
	Info.RestoreScreen(hScreen);

	fprintf(stderr, "*18\n");
}

int NetPanel::ProcessEvent(int Event, void * Param)
{
	fprintf(stderr, "ProcessEvent %i\n", Event);

	fprintf(stderr, "FE_CHANGEVIEWMODE =0,	FE_REDRAW         =1,	FE_IDLE           =2,	FE_CLOSE          =3,\n");
	fprintf(stderr, "FE_BREAK          =4,	FE_COMMAND        =5,	FE_GOTFOCUS       =6,	FE_KILLFOCUS      =7 \n");

	if (Event == FE_CHANGEVIEWMODE) {
		fprintf(stderr, "*100\n");
		IfOptCommonPanel();

		int Size = Info.Control(this, FCTL_GETCOLUMNTYPES, 0, 0);
		wchar_t *ColumnTypes = new wchar_t[Size];
		Info.Control(this, FCTL_GETCOLUMNTYPES, Size, (LONG_PTR)ColumnTypes);
		int UpdateOwners = IsOwnersDisplayed(ColumnTypes) && !LastOwnersRead;
		int UpdateLinks = IsLinksDisplayed(ColumnTypes) && !LastLinksRead;
		delete[] ColumnTypes;

		if (UpdateOwners || UpdateLinks) {
			fprintf(stderr, "*101\n");
			UpdateItems(UpdateOwners, UpdateLinks);
			fprintf(stderr, "*18.4\n");

			Info.Control(this, FCTL_UPDATEPANEL, TRUE, 0);
			Info.Control(this, FCTL_REDRAWPANEL, 0, 0);
		}
	}
	fprintf(stderr, "*102\n");

	/*
    if (Event == FE_CLOSE)
    {
    	fprintf(stderr, "Param: %s\n", (char*)Param);

        SetDirectory(L"..", 0);
        return TRUE;
	}
	*/

	return (FALSE);
}

bool NetPanel::IsCurrentFileCorrect(TCHAR **pCurFileName)
{
	return true;
	
	fprintf(stderr, "IsCurrentFileCorrect\n");
	struct PanelInfo PInfo;
	const TCHAR *CurFileName = NULL;

	if (pCurFileName)
		*pCurFileName = NULL;

	Info.Control(this, FCTL_GETPANELINFO, 0, (LONG_PTR)&PInfo);
	PluginPanelItem *PPI =
			(PluginPanelItem *)malloc(Info.Control(this, FCTL_GETPANELITEM, PInfo.CurrentItem, 0));
	if (PPI) {
		Info.Control(this, FCTL_GETPANELITEM, PInfo.CurrentItem, (LONG_PTR)PPI);
		CurFileName = PPI->FindData.lpwszFileName;
	} else {
		return false;
	}

	bool IsCorrectFile = false;
	if (lstrcmp(CurFileName, _T("..")) == 0) {
		IsCorrectFile = true;
	} else {
		FAR_FIND_DATA TempFindData;
		IsCorrectFile = GetFileInfoAndValidate(CurFileName, &TempFindData, FALSE);
	}

	if (pCurFileName) {
		*pCurFileName = (TCHAR *)malloc((lstrlen(CurFileName) + 1) * sizeof(TCHAR));
		lstrcpy(*pCurFileName, CurFileName);
	}

	free(PPI);

	return IsCorrectFile;
}

int NetPanel::ProcessKey(int Key, unsigned int ControlState)
{
	// pressing F5 generates:
	// ProcessKey (Key = 116, VK_F5 = 116, equal: 1)
	fprintf(stderr, "ProcessKey (Key = %i, VK_F5 = %i, equal: %i)\n", Key, VK_F5, Key == VK_F5);

	if (busy) {
		fprintf(stderr, "!!! !!! !!! busy, ignoring key\n");

		return false;
	}
	fprintf(stderr, "not busy, processing key\n");

	if (!ControlState && Key == VK_F1) {
		Info.ShowHelp(Info.ModuleName, NULL, FHELP_USECONTENTS | FHELP_NOSHOWERROR);
		return TRUE;
	}

	if (ControlState == (PKF_SHIFT | PKF_ALT) && Key == VK_F9) {
		EXP_NAME(Configure)(0);
		return TRUE;
	}

	if (ControlState == (PKF_SHIFT | PKF_ALT) && Key == VK_F3) {
		PtrGuard CurFileName;
		if (IsCurrentFileCorrect(CurFileName.PtrPtr())) {
			struct PanelInfo PInfo;

			Info.Control(this, FCTL_GETPANELINFO, 0, (LONG_PTR)&PInfo);

			if (lstrcmp(CurFileName, _T("..")) != 0) {

				PluginPanelItem *PPI = (PluginPanelItem *)malloc(
						Info.Control(this, FCTL_GETPANELITEM, PInfo.CurrentItem, 0));
				DWORD attributes = 0;
				if (PPI) {
					Info.Control(this, FCTL_GETPANELITEM, PInfo.CurrentItem, (LONG_PTR)PPI);
					attributes = PPI->FindData.dwFileAttributes;
					free(PPI);
				}

				if (attributes & FILE_ATTRIBUTE_DIRECTORY) {

					Info.Control(PANEL_PASSIVE, FCTL_SETPANELDIR, 0, (LONG_PTR)CurFileName.Ptr());
				} else {
					GoToFile(CurFileName, true);
				}

				Info.Control(PANEL_PASSIVE, FCTL_REDRAWPANEL, 0, 0);

				return (TRUE);
			}
		}
	}

	if (ControlState != PKF_CONTROL && Key >= VK_F3 && Key <= VK_F8 && Key != VK_F7) {
		if (!IsCurrentFileCorrect(NULL))
			return (TRUE);
	}

	fprintf(stderr, "KEY: %i\n", Key);

	if (ControlState == 0 && Key == VK_RETURN && Opt.AnyInPanel) {
		PtrGuard CurFileName;

		fprintf(stderr, "ENTER\n");

		if (!IsCurrentFileCorrect(CurFileName.PtrPtr())) {

			fprintf(stderr, "=== %ls\n", CurFileName.Ptr());

			Info.Control(this, FCTL_SETCMDLINE, 0, (LONG_PTR)CurFileName.Ptr());

			return (TRUE);
		}

		fprintf(stderr, "/===\n");
	}

	if (Opt.SafeModePanel && ControlState == PKF_CONTROL && Key == VK_PRIOR) {
		PtrGuard CurFileName;
		if (IsCurrentFileCorrect(CurFileName.PtrPtr())) {
			if (lstrcmp(CurFileName, _T(".."))) {
				GoToFile(CurFileName, false);
				return TRUE;
			}
		}

		if (CurFileName.Ptr() && !lstrcmp(CurFileName, _T(".."))) {
			fprintf(stderr, "setdir1\n");
			SetDirectory(_T("."), 0);
			return TRUE;
		}
	}

	/*
	if (ControlState == 0 && Key == VK_F7) {
		ProcessRemoveKey();
		return TRUE;
	} else if (ControlState == (PKF_SHIFT | PKF_ALT) && Key == VK_F2) {
		ProcessSaveListKey();
		return TRUE;
	} else {
		if (StartupOptCommonPanel && ControlState == (PKF_SHIFT | PKF_ALT)) {
			if (Key == VK_F12) {
				ProcessPanelSwitchMenu();
				return (TRUE);
			} else if (Key >= _T('0') && Key <= _T('9')) {
				SwitchToPanel(Key - _T('0'));
				return TRUE;
			}
		}
	}
	*/

	return (FALSE);
}

int NetPanel::DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
	fprintf(stderr, "DeleteFiles internal\n");
    if (PanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		return sc->rmdir(EscapeQuotes(Wide2MB(PanelItem->FindData.lpwszFileName).c_str()));
    } else {
		return sc->rm(EscapeQuotes(Wide2MB(PanelItem->FindData.lpwszFileName).c_str()));
    }
}

int NetPanel::MakeDirectory(const wchar_t **Name,int OpMode)
{
	newdir.clear();

    FarDialogItem DialogItems[] = {
        {DI_TEXT, 5, 1, 0, 0, 0, 0, 0, 0, L"Name:"},
        {DI_EDIT, 5, 2, 45, 0, 0, 0, 0, 0, newdir.c_str()},
        {DI_BUTTON, 0, 8, 0, 0, 0, 0, DIF_CENTERGROUP, 1, L"OK"},
        {DI_BUTTON, 0, 8, 0, 0, 0, 0, DIF_CENTERGROUP, 0, L"Cancel"}
    };
    size_t DlgData = 0;
    HANDLE hDlg = Info.DialogInit(-1, -1, -1, 50, 10, L"Dialog", DialogItems,
    	sizeof(DialogItems) / sizeof(DialogItems[0]), 0, 0, ShowDialogProcMkDir, DlgData);

	fprintf(stderr, "dialog 2 init\n");

    if (hDlg != INVALID_HANDLE_VALUE)
    {
		fprintf(stderr, "dialog 2 init ok, try to run\n");
        Info.DialogRun(hDlg);
        Info.DialogFree(hDlg);

		//if (Dlg.GetExitCode() == MKDIR_OK) {
			return sc->mkdir(EscapeQuotes(Wide2MB(newdir.c_str()).c_str()));
		//} else {
			//return false;
		//}

    } else { return false; }
}

void NetPanel::ProcessRemoveKey()
{
	fprintf(stderr, "ProcessRemoveKey\n");
	IfOptCommonPanel();

	struct PanelInfo PInfo;

	Info.Control(this, FCTL_GETPANELINFO, 0, (LONG_PTR)&PInfo);

	for (int i = 0; i < PInfo.SelectedItemsNumber; i++) {

		struct PluginPanelItem *RemovedItem = NULL;
		PluginPanelItem *PPI = (PluginPanelItem *)malloc(Info.Control(this, FCTL_GETSELECTEDPANELITEM, i, 0));
		if (PPI) {
			Info.Control(this, FCTL_GETSELECTEDPANELITEM, i, (LONG_PTR)PPI);
			RemovedItem = NetPanelItem + PPI->UserData;
		}

		if (RemovedItem != NULL)
			RemovedItem->Flags|= REMOVE_FLAG;

		free(PPI);
	}
	RemoveEmptyItems();

	Info.Control(this, FCTL_UPDATEPANEL, 0, 0);
	Info.Control(this, FCTL_REDRAWPANEL, 0, 0);

	Info.Control(PANEL_PASSIVE, FCTL_GETPANELINFO, 0, (LONG_PTR)&PInfo);

	if (PInfo.PanelType == PTYPE_QVIEWPANEL) {

		Info.Control(PANEL_PASSIVE, FCTL_UPDATEPANEL, 0, 0);
		Info.Control(PANEL_PASSIVE, FCTL_REDRAWPANEL, 0, 0);
	}
}

void NetPanel::ProcessSaveListKey()
{
	fprintf(stderr, "ProcessSaveListKey\n");
	IfOptCommonPanel();
	if (NetItemsNumber == 0)
		return;

	// default path: opposite panel directory\panel<index>.<mask extension>
	StrBuf ListPath(NT_MAX_PATH + 20 + 512);

	Info.Control(PANEL_PASSIVE, FCTL_GETPANELDIR, NT_MAX_PATH, (LONG_PTR)ListPath.Ptr());

	FSF.AddEndSlash(ListPath);
	lstrcat(ListPath, _T("panel"));
	if (Opt.CommonPanel)
		FSF.itoa(PanelIndex, ListPath.Ptr() + lstrlen(ListPath), 10);

	TCHAR ExtBuf[512];
	lstrcpy(ExtBuf, Opt.Mask);
	TCHAR *comma = _tcschr(ExtBuf, _T(','));
	if (comma)
		*comma = _T('\0');
	TCHAR *ext = _tcschr(ExtBuf, _T('.'));
	if (ext && !_tcschr(ext, _T('*')) && !_tcschr(ext, _T('?')))
		lstrcat(ListPath, ext);

	if (Info.InputBox(GetMsg(MTempPanel), GetMsg(MListFilePath), _T("NetPanel.SaveList"), ListPath, ListPath,
				ListPath.Size() - 1, NULL, FIB_BUTTONS)) {
		SaveListFile(ListPath);

		Info.Control(PANEL_PASSIVE, FCTL_UPDATEPANEL, 0, 0);
		Info.Control(PANEL_PASSIVE, FCTL_REDRAWPANEL, 0, 0);
	}
#undef _HANDLE
#undef _UPDATE
#undef _REDRAW
#undef _GET
}

void NetPanel::SaveListFile(const TCHAR *Path)
{
	fprintf(stderr, "SaveListFile\n");
	IfOptCommonPanel();

	if (!NetItemsNumber)
		return;

	StrBuf FullPath;
	GetFullPath(Path, FullPath);
	StrBuf NtPath;
	FormNtPath(FullPath, NtPath);

	HANDLE hFile = CreateFile(NtPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		const TCHAR *Items[] = {GetMsg(MError)};
		Info.Message(Info.ModuleNumber, FMSG_WARNING | FMSG_ERRORTYPE | FMSG_MB_OK, NULL, Items, 1, 0);
		return;
	}

	DWORD BytesWritten;

	static const unsigned short bom = BOM_UCS2;
	WriteFile(hFile, &bom, sizeof(bom), &BytesWritten, NULL);

	int i = 0;
	do {
		static const TCHAR *CRLF = _T("\r\n");
		const TCHAR *FName = NetPanelItem[i].FindData.FILE_NAME;
		WriteFile(hFile, FName, sizeof(TCHAR) * lstrlen(FName), &BytesWritten, NULL);
		WriteFile(hFile, CRLF, 2 * sizeof(TCHAR), &BytesWritten, NULL);
	} while (++i < NetItemsNumber);
	CloseHandle(hFile);
}

void NetPanel::SwitchToPanel(int NewPanelIndex)
{
	fprintf(stderr, "SwitchToPanel\n");
	if ((unsigned)NewPanelIndex < COMMONPANELSNUMBER && NewPanelIndex != (int)PanelIndex) {
		CommonPanels[PanelIndex].Items = NetPanelItem;
		CommonPanels[PanelIndex].ItemsNumber = NetItemsNumber;
		if (!CommonPanels[NewPanelIndex].Items) {
			CommonPanels[NewPanelIndex].ItemsNumber = 0;
			CommonPanels[NewPanelIndex].Items = (PluginPanelItem *)calloc(1, sizeof(PluginPanelItem));
		}
		if (CommonPanels[NewPanelIndex].Items) {
			CurrentCommonPanel = PanelIndex = NewPanelIndex;

			Info.Control(this, FCTL_UPDATEPANEL, 0, 0);
			Info.Control(this, FCTL_REDRAWPANEL, 0, 0);
		}
	}
}

void NetPanel::ProcessPanelSwitchMenu()
{
	fprintf(stderr, "ProcessPanelSwitchMenu\n");
	FarMenuItem fmi[COMMONPANELSNUMBER];
	memset(&fmi, 0, sizeof(FarMenuItem) * COMMONPANELSNUMBER);
	const TCHAR *txt = GetMsg(MSwitchMenuTxt);

	wchar_t tmpstr[COMMONPANELSNUMBER][128];
	static const TCHAR fmt1[] = _T("&%c. %ls %d");

	for (unsigned int i = 0; i < COMMONPANELSNUMBER; ++i) {
#define _OUT tmpstr[i]
		fmi[i].Text = tmpstr[i];

		if (i < 10)
			FSF.snprintf(_OUT, sizeof(_OUT) - 1, fmt1, _T('0') + i, txt, CommonPanels[i].ItemsNumber);
		else if (i < 36)
			FSF.snprintf(_OUT, sizeof(_OUT) - 1, fmt1, _T('A') - 10 + i, txt, CommonPanels[i].ItemsNumber);
		else
			FSF.snprintf(_OUT, sizeof(_OUT) - 1, _T("   %s %d"), txt, CommonPanels[i].ItemsNumber);
#undef _OUT
	}
	fmi[PanelIndex].Selected = TRUE;
	int ExitCode = Info.Menu(Info.ModuleNumber, -1, -1, 0, FMENU_AUTOHIGHLIGHT | FMENU_WRAPMODE,
			GetMsg(MSwitchMenuTitle), NULL, NULL, NULL, NULL, fmi, COMMONPANELSNUMBER);
	SwitchToPanel(ExitCode);
}

int NetPanel::IsOwnersDisplayed(LPCTSTR ColumnTypes)
{
	fprintf(stderr, "IsOwnersDisplayed\n");
	for (int i = 0; ColumnTypes[i]; i++)
		if (ColumnTypes[i] == _T('O') && (i == 0 || ColumnTypes[i - 1] == _T(','))
				&& (ColumnTypes[i + 1] == _T(',') || ColumnTypes[i + 1] == 0))
			return (TRUE);
	return (FALSE);
}

int NetPanel::IsLinksDisplayed(LPCTSTR ColumnTypes)
{
	fprintf(stderr, "IsLinksDisplayed\n");
	for (int i = 0; ColumnTypes[i]; i++)
		if (ColumnTypes[i] == _T('L') && ColumnTypes[i + 1] == _T('N')
				&& (i == 0 || ColumnTypes[i - 1] == _T(','))
				&& (ColumnTypes[i + 2] == _T(',') || ColumnTypes[i + 2] == 0))
			return (TRUE);
	return (FALSE);
}

inline bool isDevice(const TCHAR *FileName, const TCHAR *dev_begin)
{
	const int len = (int)lstrlen(dev_begin);
	if (FSF.LStrnicmp(FileName, dev_begin, len))
		return false;
	FileName+= len;
	if (!*FileName)
		return false;
	while (*FileName >= _T('0') && *FileName <= _T('9'))
		FileName++;
	return !*FileName;
}

bool NetPanel::GetFileInfoAndValidate(const TCHAR *FilePath, FAR_FIND_DATA *FindData, int Any)
{
	fprintf(stderr, "GetFileInfoAndValidate\n");
	StrBuf ExpFilePath;
	ExpandEnvStrs(FilePath, ExpFilePath);

	TCHAR *FileName = ExpFilePath;
	ParseParam(FileName);

	StrBuf FullPath;
	GetFullPath(FileName, FullPath);
	StrBuf NtPath;
	FormNtPath(FullPath, NtPath);

	if (!wcscmp(FileName, L"/")) {
	copy_name_set_attr:
		FindData->dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE;
	copy_name:

		if (FindData->lpwszFileName)
			free((void *)FindData->lpwszFileName);
		FindData->lpwszFileName = wcsdup(FileName);

		return (TRUE);
	}

	if (lstrlen(FileName)) {
		DWORD dwAttr = GetFileAttributes(NtPath);
		if (dwAttr != INVALID_FILE_ATTRIBUTES) {
			WIN32_FIND_DATA wfd;
			HANDLE fff = FindFirstFile(NtPath, &wfd);
			if (fff != INVALID_HANDLE_VALUE) {
				WFD2FFD(wfd, *FindData);
				FindClose(fff);
				FileName = FullPath;
				goto copy_name;
			} else {
				memset(&wfd, 0, sizeof(wfd));
				wfd.dwFileAttributes = dwAttr;
				HANDLE hFile = CreateFile(NtPath, FILE_READ_ATTRIBUTES,
						FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
						FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_POSIX_SEMANTICS, NULL);
				if (hFile != INVALID_HANDLE_VALUE) {
					GetFileTime(hFile, &wfd.ftCreationTime, &wfd.ftLastAccessTime, &wfd.ftLastWriteTime);
					wfd.nPhysicalSize = wfd.nFileSize = GetFileSize64(hFile);
					CloseHandle(hFile);
				}
				WFD2FFD(wfd, *FindData);
				FileName = FullPath;
				goto copy_name;
			}
		}
		if (Any)
			goto copy_name_set_attr;
	}
	return (FALSE);
}

void NetPanel::IfOptCommonPanel(void)
{
	fprintf(stderr, "IfOptCommonPanel\n");
	if (StartupOptCommonPanel) {
		NetPanelItem = CommonPanels[PanelIndex].Items;
		NetItemsNumber = CommonPanels[PanelIndex].ItemsNumber;
	}
}
