
/*

Alacn
alacn.uhahaa@gmail.com
http://alacn.dnsalias.org:8080/

*/

#include "std.h"
#include "popedt.h"
#include "commctrl.h"
#include "pop.h"
#include "engine.h"
#include "dialogs.h"
#include "log.h"
#include "script.h"
#include "user_storage.h"
#include "network.h"


std::string strMacro;
BYTE nSelectPriority = 0;

HWND hDlgDevice = 0,
	 hDlgDeviceConfirm = 0,
	 hDlgMenuBar = 0,
	 hDlgBrush = 0,
	 hDlgFlagsBrush = 0,
	 hDlgObject = 0,
	 hDlgFilter = 0,
	 hDlgDiscovery = 0,
	 hDlgTrigger = 0,
	 hDlgLink = 0,
	 hDlgList = 0,
	 hDlgAbout = 0,
	 hDlgAllies = 0,
	 hDlgSpellsBuildings = 0,
	 hDlgScript2 = 0,
	 hDlgSpellsNotCharging = 0,
	 hDlgHeader = 0,
	 hDlgObjBank = 0,
	 hDlgMapType = 0,
	 hDlgInfo = 0,
	 hDlgSwapTribe = 0,
	 hDlgMarkers = 0,
	 hDlgAIAttrib = 0,
	 hDlgAIScript = 0,
	 hDlgMacro = 0,
	 hDlgObjectMore = 0;

GUID	guidDeviceOld;
bool	fHwDeviceOld,
		fFullScreenOld,
		fQuickDuplicate = false,
		fEngineActiveOld;
char	szDeviceNameOld[STR_SIZE];
DWORD	dwScreenWidthOld,
		dwScreenHeightOld,
		dwScreenDepthOld;
int		iDlgDeviceConfirmTime = 0;

COMPUTERPLAYERFILE CPAttrib;
char szCPFile[MAX_PATH];
bool fAIAttribChanged;

SCRIPTINFO AIScript;
char szAIScriptLog[MAX_PATH], szAIScriptSrc[MAX_PATH], szAIScriptDst[MAX_PATH];

char szLang_Text[MAX_PATH], szLang_Lang[MAX_PATH];

DEVICE_INFO	*selectedDevice = 0;
SCREEN_INFO *selectedMode = 0;

DWORD ObjectFilterFlags = OFF_ALL;

HBITMAP hBank0 = 0,
		hBank2 = 0,
		hBank3 = 0,
		hBank4 = 0,
		hBank5 = 0,
		hBank6 = 0,
		hBank7 = 0,
		hMap0 = 0,
		hMap1 = 0,
		hMap2 = 0,
		hMap3 = 0,
		hMap4 = 0,
		hMap5 = 0,
		hMap6 = 0,
		hMap7 = 0,
		hMap8 = 0,
		hMap9 = 0,
		hMapA = 0,
		hMapB = 0,
		hMapC = 0,
		hMapD = 0,
		hMapE = 0,
		hMapF = 0,
		hMapG = 0,
		hMapH = 0,
		hMapI = 0,
		hMapJ = 0,
		hMapK = 0,
		hMapL = 0,
		hMapM = 0,
		hMapN = 0,
		hMapO = 0,
		hMapP = 0,
		hMapQ = 0,
		hMapR = 0,
		hMapS = 0,
		hMapT = 0,
		hMapU = 0,
		hMapV = 0,
		hMapW = 0,
		hMapX = 0,
		hMapY = 0,
		hMapZ = 0;


void DeleteDuplicateTrees()
{
	const int nResult = MessageBox(NULL, _T("Are you sure you want to delete duplicate (stacked) trees?"), _T("Delete Trees"), MB_YESNO);
	if (nResult == IDNO) return;

	SaveThingsUndoState();
	std::vector<THING*> vecTreesList;
	int nDeletedTrees = 0;
	THING *pThing = Things;

	do
	{
		if ((pThing->Thing.Type == T_SCENERY) &&
			(pThing->Thing.Model >= M_SCENERY_TREE_1 && pThing->Thing.Model <= M_SCENERY_TREE_6))
			vecTreesList.push_back(pThing);

		pThing = pThing->Next;
	} while (pThing != Things);

	for (std::vector<THING*>::size_type i = 0; i < vecTreesList.size(); i++)
	{
		for (std::vector<THING*>::size_type j = i + 1; j < vecTreesList.size(); j++)
		{
			if ((vecTreesList[i]) && (vecTreesList[i]->x == vecTreesList[j]->x) && (vecTreesList[i]->z == vecTreesList[j]->z))
			{
				if (net.IsInitialized())
				{
					struct Packet *p = new Packet;
					p->wType = PACKETTYPE_DELETE_OBJECT;
					p->wData[0] = vecTreesList[i]->Idx;
					net.SendPacket(p);
					p->del();
				}

				ThingsIndices[vecTreesList[i]->Idx] = 0;
				nDeletedTrees++;
				UNLINK(Things, vecTreesList[i]);
				DlgObjectUnlinkObj(vecTreesList[i]);
				delete vecTreesList[i];
				ObjectsCount--;
			}
		}
	}

	DlgInfoUpdate(hDlgInfo);
	DlgObjectUpdateInfo(hDlgObject);
	EngineUpdateMiniMap();
	sprintf(str, "%d", nDeletedTrees);
	MessageBox(NULL, str, _T("Trees Deleted"), MB_ICONINFORMATION | MB_OK);
}

void DeleteWilds()
{
	const int nResult = MessageBox(NULL, _T("Are you sure you want to delete ALL wildmen?"), _T("Delete Wildmen"), MB_YESNO);
	if (nResult == IDNO) return;

	std::vector<THING*> vecWildmenList;
	int nDeletedWilds = 0;
	THING *pThing = Things;

	do
	{
		if ((pThing->Thing.Type == T_PERSON) && (pThing->Thing.Model == M_PERSON_WILD))
			vecWildmenList.push_back(pThing);

		pThing = pThing->Next;
	} while (pThing != Things);

	for (const auto& t : vecWildmenList)
	{
		if (net.IsInitialized())
		{
			struct Packet *p = new Packet;
			p->wType = PACKETTYPE_DELETE_OBJECT;
			p->wData[0] = t->Idx;
			net.SendPacket(p);
			p->del();
		}

		nDeletedWilds++;
		ThingsIndices[t->Idx] = 0;
		UNLINK(Things, t);
		DlgObjectUnlinkObj(t);
		delete t;
		ObjectsCount--;
	}

	DlgInfoUpdate(hDlgInfo);
	DlgObjectUpdateInfo(hDlgObject);
	EngineUpdateMiniMap();
	sprintf(str, "%d", nDeletedWilds);
	MessageBox(NULL, str, _T("Wildmen Deleted"), MB_ICONINFORMATION | MB_OK);
}

int g_Angles[] = {
	ANGLE_0, ANGLE_90, ANGLE_180, ANGLE_270, // BUILDINGS
	ANGLE_45, ANGLE_135, ANGLE_225, ANGLE_315 // SCENERY
};

void RotateTrees()
{
	const int nResult = MessageBox(NULL, _T("Are you sure you want to randomly rotate all trees?"), _T("Rotate Trees"), MB_YESNO);
	if (nResult == IDNO) return;

	int nRotatedThings = 0;
	THING *pThing = Things;

	do
	{
		if (pThing->Thing.Type == T_SCENERY)
		{
			if (pThing->Thing.Model >= M_SCENERY_TREE_1 && pThing->Thing.Model <= M_SCENERY_TREE_6)
			{
				pThing->Thing.Scenery.Angle = g_Angles[EngineGetRand(0, 7)];
				nRotatedThings++;
			}
		}

		pThing = pThing->Next;
	} while (pThing != Things);

	sprintf(str, "%d", nRotatedThings);
	MessageBox(NULL, str, _T("Trees Rotated"), MB_ICONINFORMATION | MB_OK);
}

void RotateBuildings()
{
	const int nResult = MessageBox(NULL, _T("Are you sure you want to randomly rotate all buildings?"), _T("Rotate Buildings"), MB_YESNO);
	if (nResult == IDNO) return;

	int nRotatedThings = 0;
	THING *pThing = Things;

	do
	{
		if (pThing->Thing.Type == T_BUILDING)
		{
			if ((pThing->Thing.Model >= M_BUILDING_HUT1 && pThing->Thing.Model <= M_BUILDING_FIREWARRIOR_TRAIN) ||
				((pThing->Thing.Model >= M_BUILDING_BOAT_HUT_1 && pThing->Thing.Model <= M_BUILDING_AIRSHIP_HUT_2)))
			{
				pThing->Thing.Building.Angle = g_Angles[EngineGetRand(0, 3)];
				nRotatedThings++;
			}
		}

		pThing = pThing->Next;
	} while (pThing != Things);

	DlgInfoUpdate(hDlgInfo);
	DlgObjectUpdateInfo(hDlgObject);
	sprintf(str, "%d", nRotatedThings);
	MessageBox(NULL, str, _T("Buildings Rotated"), MB_ICONINFORMATION | MB_OK);
}

void Neutralizer()
{
	const int nResult = MessageBox(NULL, _T("Are you sure you want to change the owner of all wildmen and scenery to neutral?"), _T("Neutralizer"), MB_YESNO);
	if (nResult == IDNO) return;

	int nFoundObjects = 0;
	THING *pThing = Things;

	do
	{
		if (pThing->Thing.Owner != OWNER_NEUTRAL)
		{
			if (pThing->Thing.Type == T_PERSON)
			{
				if (pThing->Thing.Model == M_PERSON_WILD)
				{
					pThing->Thing.Owner = OWNER_NEUTRAL;
					nFoundObjects++;
				}
			}
			else if (pThing->Thing.Type == T_SCENERY)
			{
				pThing->Thing.Owner = OWNER_NEUTRAL;
				nFoundObjects++;
			}
			else if (pThing->Thing.Type == T_BUILDING)
			{
				if (pThing->Thing.Model == M_BUILDING_KNOWLEDGE)
				{
					pThing->Thing.Owner = OWNER_NEUTRAL;
					nFoundObjects++;
				}
			}
		}

		if (pThing->Thing.Owner == OWNER_HOSTBOT)
			pThing->Thing.Owner = OWNER_NEUTRAL;

		pThing = pThing->Next;
	} while (pThing != Things);

	DlgInfoUpdate(hDlgInfo);
	DlgObjectUpdateInfo(hDlgObject);
	EngineUpdateMiniMap();
	sprintf(str, "%d", nFoundObjects);
	MessageBox(NULL, str, _T("Objects Updated"), MB_ICONINFORMATION | MB_OK);
}

void Levl3ToLevl2()
{
	const int nResult = MessageBox(NULL, _T("This option will convert the owner of existing objects to support the levl2 format. Create a back-up of the level first. Continue?"), _T("Levl3 to Levl2"), MB_YESNO);
	if (nResult == IDNO) return;

	int nFoundObjects = 0;
	THING *pThing = Things;

	do
	{
		if (pThing->Thing.Owner != OWNER_NEUTRAL)
		{
			if (pThing->Thing.Type == T_PERSON)
			{
				if (pThing->Thing.Model == M_PERSON_WILD)
				{
					pThing->Thing.Owner = OWNER_NEUTRAL;
					nFoundObjects++;
				}
				else
				{
					switch (pThing->Thing.Owner)
					{
					case OWNER_CYAN:
						pThing->Thing.Owner = OWNER_BLUE;
						nFoundObjects++;
						break;
					case OWNER_MAGENTA:
						pThing->Thing.Owner = OWNER_RED;
						nFoundObjects++;
						break;
					case OWNER_BLACK:
						pThing->Thing.Owner = OWNER_YELLOW;
						nFoundObjects++;
						break;
					case OWNER_ORANGE:
						pThing->Thing.Owner = OWNER_GREEN;
						nFoundObjects++;
						break;
					}
				}
			}
			else if (pThing->Thing.Type == T_SCENERY)
			{
				pThing->Thing.Owner = OWNER_NEUTRAL;
				nFoundObjects++;
			}
			else if (pThing->Thing.Type == T_BUILDING)
			{
				if (pThing->Thing.Model == M_BUILDING_KNOWLEDGE)
				{
					pThing->Thing.Owner = OWNER_NEUTRAL;
					nFoundObjects++;
				}
				else
				{
					switch (pThing->Thing.Owner)
					{
					case OWNER_CYAN:
						pThing->Thing.Owner = OWNER_BLUE;
						nFoundObjects++;
						break;
					case OWNER_MAGENTA:
						pThing->Thing.Owner = OWNER_RED;
						nFoundObjects++;
						break;
					case OWNER_BLACK:
						pThing->Thing.Owner = OWNER_YELLOW;
						nFoundObjects++;
						break;
					case OWNER_ORANGE:
						pThing->Thing.Owner = OWNER_GREEN;
						nFoundObjects++;
						break;
					}
				}
			}
		}

		switch (pThing->Thing.Owner)
		{
		case OWNER_CYAN:
			pThing->Thing.Owner = OWNER_BLUE;
			nFoundObjects++;
			break;
		case OWNER_MAGENTA:
			pThing->Thing.Owner = OWNER_RED;
			nFoundObjects++;
			break;
		case OWNER_BLACK:
			pThing->Thing.Owner = OWNER_YELLOW;
			nFoundObjects++;
			break;
		case OWNER_ORANGE:
			pThing->Thing.Owner = OWNER_GREEN;
			nFoundObjects++;
			break;
		}

		if (pThing->Thing.Owner == OWNER_HOSTBOT)
		{
			nFoundObjects++;
			pThing->Thing.Owner = OWNER_NEUTRAL;
		}

		pThing = pThing->Next;
	} while (pThing != Things);

	DlgInfoUpdate(hDlgInfo);
	DlgObjectUpdateInfo(hDlgObject);
	EngineUpdateMiniMap();
	sprintf(str, "%d", nFoundObjects);
	MessageBox(NULL, str, _T("Objects Updated"), MB_ICONINFORMATION | MB_OK);
}

void LevelOpen()
{
	char filename[MAX_PATH],
		 strInitDir[MAX_PATH];

	GetUserDocumentsDirectory(strInitDir);
	strcpy(filename, szLevel);

	OPENFILENAME ofn;
	SecureZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.lpstrTitle = SZ_OPENLEVEL_TITLE;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrInitialDir = strInitDir;
	ofn.lpstrFilter = SZ_OPENLEVEL_FILTER;
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	LockDialogs(true);
	if(GetOpenFileNameA(&ofn))
	{
		if(EngineLoadLevelV3(filename) != S_OK)
			ModalMsg(SZ_OPENLEVEL_FAILED, APPNAME, MB_ICONEXCLAMATION);
	}
	LockDialogs(false);
}


void LevelNew()
{
	if (net.IsInitialized() && net.IsConnected())
	{
		if (!net.GetNewMap())
		{
			net.SetNewMap(true);
			struct Packet *p = new Packet;
			p->wType = PACKETTYPE_NEW_MAP_ASK;
			net.SendPacket(p);
			p->del();
		}
		else
		{
			ModalMsg(SZ_CONFIRM_NEW_LEVEL_CD, APPNAME, MB_ICONWARNING | MB_OK | MB_DEFBUTTON2);
		}
	}
	else
	{
		if (ModalMsg(SZ_CONFIRM_NEW_LEVEL, APPNAME, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDYES)
			EngineNewMap();
	}
}


void LevelSave()
{
	if(lstrlen(szLevel) != 0)
	{
		if((LevelFormatMode == LEVEL_FORMAT_MODE_V3 ? EngineSaveLevelV3(szLevel) : EngineSaveLevel(szLevel)) != S_OK)
			ModalMsg(SZ_SAVELEVEL_FAILED, APPNAME, MB_ICONEXCLAMATION);
		else
			ModalMsg((LevelFormatMode == LEVEL_FORMAT_MODE_V3 ? SZ_SAVELEVEL_DONE_V3 : SZ_SAVELEVEL_DONE_V2), APPNAME, MB_ICONASTERISK);
	}
	else
	{
		LevelSaveAs();
	}
}


void LevelSaveAs()
{
	char filename[MAX_PATH],
		 strInitDir[MAX_PATH];

	GetUserDocumentsDirectory(strInitDir);
	strcpy(filename, szLevel);

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.lpstrDefExt = SZ_SAVELEVEL_DEFEXT;
	ofn.lpstrTitle = SZ_SAVELEVEL_TITLE;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrInitialDir = strInitDir;
	ofn.lpstrFilter = SZ_SAVELEVEL_FILTER;
	ofn.nFilterIndex = (LevelFormatMode == LEVEL_FORMAT_MODE_V3 ? 1 : 2);  //default the save as format based on the current level format
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

	LockDialogs(true);
	if(GetSaveFileName(&ofn))
	{
		//user selected level3 format
		if (ofn.nFilterIndex == 1) {  //fyi, the selection starts with index 1, not 0 like you would think...
			if (EngineSaveLevelV3(filename) != S_OK) {
				ModalMsg(SZ_SAVELEVEL_FAILED, APPNAME, MB_ICONEXCLAMATION);
			}
			else {
				ModalMsg(SZ_SAVELEVEL_DONE_V3, APPNAME, MB_ICONASTERISK);
			}
		}
		//user selected level2 format
		else {
			if (EngineSaveLevel(filename) != S_OK) {
				ModalMsg(SZ_SAVELEVEL_FAILED, APPNAME, MB_ICONEXCLAMATION);
			}
			else {
				ModalMsg(SZ_SAVELEVEL_DONE_V2, APPNAME, MB_ICONASTERISK);
			}
		}
	}
	LockDialogs(false);
}

long EngineLoadMapOnly(char *filename)
{
	// -=-=- .dat -=-=-
	HANDLE h = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (h == INVALID_HANDLE_VALUE)
	{
		//
		//sprintf(str, SZ_ERR_CREATEFILE, filename);
		//LogWrite(str);
		//
		return -1;
	}

	LEVELDATv2 *level2dat;
	level2dat = (LEVELDATv2*)malloc(sizeof(LEVELDATv2));
	memset(level2dat, 0, sizeof(LEVELDATv2));

	dwRW = 0;
	ReadFile(h, level2dat, sizeof(LEVELDATv2), &dwRW, 0);
	if (dwRW != sizeof(LEVELDATv2))
	{
		free(level2dat);
		CloseHandle(h);

		sprintf(str, SZ_ERR_READERROR, filename);
		LogWrite(str);
		return -1;
	}

	WORD temp_land[MAP_SIZE];
	memcpy(temp_land, level2dat->GroundHeight, sizeof(wEngineGround));

	Preset.Land.clear();
	Preset.Rotation = 0;

	for (int z = 0; z < 128; z++)
	{
		for (int x = 0; x < 128; x++)
		{
			while (x < 0) x += GROUND_X_SIZE;
			while (z < 0) z += GROUND_Z_SIZE;
			while (x >= GROUND_X_SIZE) x -= GROUND_X_SIZE;
			while (z >= GROUND_Z_SIZE) z -= GROUND_Z_SIZE;

			Preset.Land.push_back({ x, z, temp_land[z * GROUND_Z_SIZE + x], {0} });
		}
	}

	Preset.LandSize = 127;
	EngineUpdateView();
	EngineUpdateMiniMap();
	bLandPresetMode = true;

	free(level2dat);
	CloseHandle(h);
	return S_OK;
}

void ImportMapPreset()
{
	char filename[MAX_PATH],
		strInitDir[MAX_PATH];

	GetUserDocumentsDirectory(strInitDir);
	strcpy(filename, szLevel);

	OPENFILENAME ofn;
	SecureZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.lpstrTitle = SZ_OPENLEVEL_TITLE;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrInitialDir = strInitDir;
	ofn.lpstrFilter = SZ_OPENLEVEL_FILTER;
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	LockDialogs(true);
	if (GetOpenFileNameA(&ofn))
	{
		if (EngineLoadMapOnly(filename) != S_OK)
			ModalMsg(SZ_OPENLEVEL_FAILED, APPNAME, MB_ICONEXCLAMATION);
	}
	LockDialogs(false);
}

void ExportHeightMap()
{
	char filename[STR_SIZE],
		 strInitDir[STR_SIZE];

	GetUserDocumentsDirectory(strInitDir);
	memset(filename, 0, sizeof(filename));

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.lpstrDefExt = SZ_EXPORT_HEIGHTMAP_DEFEXT;
	ofn.lpstrTitle = SZ_EXPORT_HEIGHTMAP_TITLE;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrInitialDir = strInitDir;
	ofn.lpstrFilter = SZ_EXPORT_HEIGHTMAP_FILTER;
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

	LockDialogs(true);
	int f = GetSaveFileName(&ofn);
	LockDialogs(false);

	if(!f) return;

	//

	BITMAPINFOHEADER bih;
	BITMAPFILEHEADER bfh;
	BYTE bits[128][128];

	bih.biSize          = sizeof(BITMAPINFOHEADER);
	bih.biWidth         = 128;
	bih.biHeight        = 128;
	bih.biPlanes        = 1;
	bih.biBitCount      = 8;
	bih.biCompression   = BI_RGB;
	bih.biSizeImage     = 128 * 128 * 8;
	bih.biXPelsPerMeter =
	bih.biYPelsPerMeter = 0;
	bih.biClrUsed       = 256;
	bih.biClrImportant  = 0;

	bfh.bfType = 0x4D42;
	bfh.bfSize = sizeof(BITMAPFILEHEADER) + bih.biSize + bih.biClrUsed * sizeof(RGBQUAD) + bih.biSizeImage;
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + bih.biSize + bih.biClrUsed * sizeof(RGBQUAD);

	//

	DWORD LANDSCAPE_COLORS[256] = {
		0x2D2D96, 0x028000, 0x048000, 0x078000, 0x088000, 0x0A8000, 0x0C8000, 0x0E8000,
		0x108000, 0x128000, 0x158000, 0x168000, 0x188000, 0x1A8000, 0x1C8000, 0x1E8000,
		0x208000, 0x228000, 0x248000, 0x278000, 0x288000, 0x2A8000, 0x2C8000, 0x2E8000,
		0x308000, 0x328000, 0x348000, 0x378000, 0x388000, 0x3A8000, 0x3C8000, 0x3E8000,
		0x408000, 0x428000, 0x458000, 0x468000, 0x488000, 0x4A8000, 0x4D8000, 0x4E8000,
		0x508000, 0x528000, 0x548000, 0x568000, 0x588000, 0x5A8000, 0x5C8000, 0x5E8000,
		0x608100, 0x628100, 0x648100, 0x668100, 0x688100, 0x6A8100, 0x6C8100, 0x6E8100,
		0x708100, 0x728100, 0x748100, 0x768100, 0x788100, 0x7A8100, 0x7C8100, 0x7E8100,
		0x808100, 0x828100, 0x848100, 0x868100, 0x888100, 0x8A8100, 0x8C8100, 0x8E8100,
		0x908200, 0x928200, 0x948200, 0x968200, 0x988200, 0x9A8200, 0x9C8200, 0x9E8200,
		0xA08200, 0xA28200, 0xA48200, 0xA68200, 0xA88200, 0xAA8200, 0xAC8200, 0xAE8200,
		0xB08200, 0xB28200, 0xB48200, 0xB68200, 0xB88200, 0xBA8200, 0xBC8200, 0xBE8200,
		0xC08300, 0xC08402, 0xC18504, 0xC18606, 0xC28709, 0xC2880A, 0xC3890D, 0xC3890E,
		0xC38A0F, 0xC48B10, 0xC48B11, 0xC48C12, 0xC58D14, 0xC58D15, 0xC58E16, 0xC58E17,
		0xC68F18, 0xC68F1A, 0xC6901B, 0xC6901C, 0xC7911E, 0xC7921F, 0xC79220, 0xC89321,
		0xC89422, 0xC89423, 0xC99524, 0xC99526, 0xC99627, 0xCA9728, 0xCA972A, 0xCA982B,
		0xCA982C, 0xCB992D, 0xCB992E, 0xCB9A2F, 0xCC9B30, 0xCC9B32, 0xCC9C33, 0xCC9C34,
		0xCD9D35, 0xCD9E37, 0xCD9E38, 0xCE9F39, 0xCE9F3A, 0xCEA03C, 0xCFA13D, 0xCFA13E,
		0xCFA23F, 0xCFA240, 0xD0A341, 0xD0A342, 0xD0A444, 0xD1A545, 0xD1A546, 0xD1A648,
		0xD2A749, 0xD2A74A, 0xD2A84B, 0xD2A84C, 0xD3A94D, 0xD3AA4F, 0xD3AA50, 0xD4AB51,
		0xD4AB52, 0xD4AC54, 0xD4AC55, 0xD5AD56, 0xD5AE57, 0xD5AE58, 0xD6AF5A, 0xD6B05C,
		0xD7B15E, 0xD7B25F, 0xD7B261, 0xD8B362, 0xD8B463, 0xD8B465, 0xD9B566, 0xD9B668,
		0xDAB76A, 0xDAB86B, 0xDAB86D, 0xDBB96E, 0xDBBA6F, 0xDBBA70, 0xDCBB72, 0xDCBC74,
		0xDDBD77, 0xDDBE78, 0xDEBF7A, 0xDEC07C, 0xDFC17E, 0xDFC282, 0xE0C384, 0xE0C486,
		0xE1C689, 0xE2C68B, 0xE2C78D, 0xE3C88E, 0xE3C991, 0xE4CA93, 0xE4CB94, 0xE5CC96,
		0xE5CD98, 0xE5CE9A, 0xE6CE9B, 0xE6CF9D, 0xE7D1A0, 0xE7D1A2, 0xE8D2A3, 0xE8D3A5,
		0xE9D4A7, 0xE9D5A9, 0xEAD6AA, 0xEAD7AC, 0xEBD8AF, 0xEBD9B0, 0xECD9B2, 0xECDAB4,
		0xEDDBB6, 0xEDDCB7, 0xEDDDB9, 0xEEDEBB, 0xEEDFBD, 0xEFE0BF, 0xEFE1C1, 0xF0E2C3,
		0xF0E2C5, 0xF1E3C6, 0xF1E4C8, 0xF2E5CA, 0xF2E6CC, 0xF3E7CE, 0xF3E8D0, 0xF4E9D2,
		0xF4EAD3, 0xF5EAD5, 0xF5EBD7, 0xF5ECD9, 0xF6EDDA, 0xF7EEDD, 0xF7EFDF, 0xF7F0E1,
		0xF8F1E2, 0xF8F2E4, 0xF9F2E6, 0xF9F3E8, 0xFAF4E9, 0xFAF5EC, 0xFBF6EE, 0xFCF8F1,
		0xFCF9F3, 0xFDFAF5, 0xFDFBF6, 0xC0C0C0, 0x00FF00, 0xFFFF00, 0xFF0000, 0x00FFFF };

	//

	for(int z = 0; z < GROUND_Z_SIZE; z++)
	for(int x = 0; x < GROUND_X_SIZE; x++)
	{
		WORD w = EngineGetGroundHeight(x + (int)fEnginePosX - 64, z + (int)fEnginePosZ - 64) / 8;
		//WORD w = EngineGetGroundHeight(x, z) / 8;
		if(w > 0xFA) w = 0xFB;
		bits[z][x] = (BYTE)w;
	}

	if(Things && !(dwEngineFlags & EF_HIDE_OBJS_ON_MINIMAP))
	{
		THING *t = Things;
		do
		{
			BYTE c;

			if(IsNotNeutral(t))
			{
				switch(t->Thing.Owner)
				{
				case OWNER_BLUE:	c = 0xFF; break;
				case OWNER_RED:		c = 0xFE; break;
				case OWNER_YELLOW:	c = 0xFD; break;
				case OWNER_GREEN:	c = 0xFC; break;
				case OWNER_CYAN:	c = 0xDF; break;
				case OWNER_MAGENTA:	c = 0xDE; break;
				case OWNER_BLACK:	c = 0xDD; break;
				case OWNER_ORANGE:	c = 0xDC; break;
				default: c = 0xFB;
				}
			}
			else
				c = 0xFB;

			int x, z;

			x = ((t->Thing.PosX >> 8) / 2) - (int)fEnginePosX - 64;
			while(x < 0) x += GROUND_X_SIZE;
			while(x >= GROUND_X_SIZE) x -= GROUND_X_SIZE;

			z = ((t->Thing.PosZ >> 8) / 2) - (int)fEnginePosZ - 64;
			while(z < 0) z += GROUND_Z_SIZE;
			while(z >= GROUND_Z_SIZE) z -= GROUND_Z_SIZE;

			bits[z][x] = c;

			t = t->Next;
		}
		while(t != Things);
	}

	//

	HANDLE h = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(h == INVALID_HANDLE_VALUE)
	{
		ModalMsg(SZ_EXPORT_HEIGHTMAP_FAILED, APPNAME, MB_ICONEXCLAMATION);
		return;
	}

	dwRW = 0;
	WriteFile(h, &bfh, sizeof(BITMAPFILEHEADER), &dwRW, 0);
	if(dwRW != sizeof(BITMAPFILEHEADER)) goto _write_failed;

	dwRW = 0;
	WriteFile(h, &bih, sizeof(BITMAPINFOHEADER), &dwRW, 0);
	if(dwRW != sizeof(BITMAPINFOHEADER)) goto _write_failed;

	dwRW = 0;
	WriteFile(h, LANDSCAPE_COLORS, sizeof(LANDSCAPE_COLORS), &dwRW, 0);
	if(dwRW != sizeof(LANDSCAPE_COLORS)) goto _write_failed;

	dwRW = 0;
	WriteFile(h, bits, sizeof(bits), &dwRW, 0);
	if(dwRW != sizeof(bits)) goto _write_failed;

	CloseHandle(h);

	//ModalMsg(SZ_EXPORT_HEIGHTMAP_DONE, APPNAME, MB_ICONASTERISK);

	return;

_write_failed:
	CloseHandle(h);

	sprintf(str, SZ_ERR_WRITEERROR, filename);
	LogWrite(str);

	ModalMsg(SZ_EXPORT_HEIGHTMAP_FAILED, APPNAME, MB_ICONEXCLAMATION);
}


void Lang2Text()
{
	char flang[MAX_PATH], ftext[MAX_PATH],
		 strInitDir[MAX_PATH];

	GetUserDocumentsDirectory(strInitDir);
	strcpy(flang, szLang_Lang);

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.lpstrTitle = SZ_LANG2TEXT_SRC_TITLE;
	ofn.lpstrFile = flang;
	ofn.nMaxFile = sizeof(flang);
	ofn.lpstrInitialDir = strInitDir;
	ofn.lpstrFilter = SZ_LANG_LANG_FILTER;
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	LockDialogs(true);
	int i = GetOpenFileName(&ofn);
	LockDialogs(false);

	if(!i) return;

    //

	GetUserDocumentsDirectory(strInitDir);
	strcpy(ftext, szLang_Text);

	//OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.lpstrTitle = SZ_LANG2TEXT_DST_TITLE;
	ofn.lpstrFile = ftext;
	ofn.nMaxFile = sizeof(ftext);
	ofn.lpstrInitialDir = strInitDir;
	ofn.lpstrFilter = SZ_LANG_TEXT_FILTER;
	ofn.lpstrDefExt = SZ_LANG_TEXT_DEFEXT;
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

	LockDialogs(true);
	i = GetSaveFileName(&ofn);
	LockDialogs(false);

	if(!i) return;

	//

	HANDLE hSrc, hDst;
	WORD buf2[4096];
	BYTE buf1[sizeof(buf2)];
	DWORD size, buf, i1, i2;

	hSrc = CreateFile(flang, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(hSrc == INVALID_HANDLE_VALUE)
	{
		ModalMsg(SZ_LANG_SRC_OPEN_FAILED, APPNAME, MB_ICONEXCLAMATION);
		return;
	}

	size = SetFilePointer(hSrc, 0, 0, 2);
	SetFilePointer(hSrc, 0, 0, 0);
	if((size % 2) == 1) size--;

	hDst = CreateFile(ftext, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(hDst == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hSrc);
		ModalMsg(SZ_LANG_DST_CREATE_FAILED, APPNAME, MB_ICONEXCLAMATION);
		return;
	}

	while(size > 0)
	{
		if(size > sizeof(buf2))
			buf = sizeof(buf2);
		else
			buf = size;

		size -= buf;

		dwRW = 0;
		ReadFile(hSrc, buf2, buf, &dwRW, 0);
		if(buf != dwRW)
		{
			CloseHandle(hSrc);
			CloseHandle(hDst);
			ModalMsg(SZ_LANG_SRC_READ_FAILED, APPNAME, MB_ICONEXCLAMATION);
			return;
		}

		buf /= 2;

		i1 = 0;
		i2 = 0;
		while(i2 < buf)
		{
			if(buf2[i2] == 0)
			{
				buf1[i1] = '\r';
				i1++;
				buf1[i1] = '\n';
			}
			else
			{
				buf1[i1] = (BYTE)buf2[i2];
			}
			i1++;
			i2++;
		}

		dwRW = 0;
		WriteFile(hDst, buf1, i1, &dwRW, 0);
		if(i1 != dwRW)
		{
			CloseHandle(hSrc);
			CloseHandle(hDst);
			ModalMsg(SZ_LANG_DST_WRITE_FAILED, APPNAME, MB_ICONEXCLAMATION);
			return;
		}
	}

	CloseHandle(hSrc);
	CloseHandle(hDst);

	ModalMsg(SZ_LANG_DONE, APPNAME, MB_ICONASTERISK);

	strcpy(szLang_Text, ftext);
	strcpy(szLang_Lang, flang);
}


void Text2Lang()
{
	char flang[MAX_PATH], ftext[MAX_PATH],
		 strInitDir[MAX_PATH];

	GetUserDocumentsDirectory(strInitDir);
	strcpy(ftext, szLang_Text);

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.lpstrTitle = SZ_TEXT2LANG_SRC_TITLE;
	ofn.lpstrFile = ftext;
	ofn.nMaxFile = sizeof(ftext);
	ofn.lpstrInitialDir = strInitDir;
	ofn.lpstrFilter = SZ_LANG_TEXT_FILTER;
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	LockDialogs(true);
	int i = GetOpenFileName(&ofn);
	LockDialogs(false);

	if(!i) return;

    //

	GetUserDocumentsDirectory(strInitDir);
	strcpy(flang, szLang_Lang);

	//OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.lpstrTitle = SZ_TEXT2LANG_DST_TITLE;
	ofn.lpstrFile = flang;
	ofn.nMaxFile = sizeof(flang);
	ofn.lpstrInitialDir = strInitDir;
	ofn.lpstrFilter = SZ_LANG_LANG_FILTER;
	ofn.lpstrDefExt = SZ_LANG_LANG_DEFEXT;
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

	LockDialogs(true);
	i = GetSaveFileName(&ofn);
	LockDialogs(false);

	if(!i) return;

	//

	HANDLE hSrc, hDst;
	WORD buf2[4096];
	BYTE buf1[4096];
	DWORD size, buf, i1, i2;

	hSrc = CreateFile(ftext, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(hSrc == INVALID_HANDLE_VALUE)
	{
		ModalMsg(SZ_LANG_SRC_OPEN_FAILED, APPNAME, MB_ICONEXCLAMATION);
		return;
	}

	size = SetFilePointer(hSrc, 0, 0, 2);
	SetFilePointer(hSrc, 0, 0, 0);

	hDst = CreateFile(flang, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(hDst == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hSrc);
		ModalMsg(SZ_LANG_DST_CREATE_FAILED, APPNAME, MB_ICONEXCLAMATION);
		return;
	}

	while(size > 0)
	{
		if(size > sizeof(buf1))
			buf = sizeof(buf1);
		else
			buf = size;

		size -= buf;

		dwRW = 0;
		ReadFile(hSrc, buf1, buf, &dwRW, 0);
		if(buf != dwRW)
		{
			CloseHandle(hSrc);
			CloseHandle(hDst);
			ModalMsg(SZ_LANG_SRC_READ_FAILED, APPNAME, MB_ICONEXCLAMATION);
			return;
		}

		i1 = 0;
		i2 = 0;
		while(i1 < buf)
		{
			if(buf1[i1] == '\r')
			{
				i1++;
				continue;
			}

			if(buf1[i1] == '\n')
			{
				buf2[i2] = 0;
			}
			else
			{
				buf2[i2] = buf1[i1];
			}
			i1++;
			i2++;
		}

		i2 *= 2;

		dwRW = 0;
		WriteFile(hDst, buf2, i2, &dwRW, 0);
		if(i2 != dwRW)
		{
			CloseHandle(hSrc);
			CloseHandle(hDst);
			ModalMsg(SZ_LANG_DST_WRITE_FAILED, APPNAME, MB_ICONEXCLAMATION);
			return;
		}
	}

	CloseHandle(hSrc);
	CloseHandle(hDst);

	ModalMsg(SZ_LANG_DONE, APPNAME, MB_ICONASTERISK);

	strcpy(szLang_Text, ftext);
	strcpy(szLang_Lang, flang);
}


void ShowHelp()
{
	ShowWindow(hMainWnd, SW_MINIMIZE);
	ShellExecute(hMainWnd, 0, "PopEdt.chm", 0, szStartPath, SW_SHOW);
}


bool GetInt(char *str, int *i)
{
	if(!str) return false;

    bool p = true;
	int a = 0;

	if(*str == '+') str++;
	else if(*str == '-')
	{
		str++;
		p = false;
	}

	while(true)
	switch(*str)
	{
	case '0': str++; a = a * 10 + 0; break;
	case '1': str++; a = a * 10 + 1; break;
	case '2': str++; a = a * 10 + 2; break;
	case '3': str++; a = a * 10 + 3; break;
	case '4': str++; a = a * 10 + 4; break;
	case '5': str++; a = a * 10 + 5; break;
	case '6': str++; a = a * 10 + 6; break;
	case '7': str++; a = a * 10 + 7; break;
	case '8': str++; a = a * 10 + 8; break;
	case '9': str++; a = a * 10 + 9; break;
	case 0: *i = p ? a : -a; return true;
	default: return false;
	}
}


long InitializeDialogs()
{
	InitCommonControls();
	if(!InitializeMenuBar()) return -1;

	char szPath[STR_SIZE];
	GetCurrentDirectory(STR_SIZE, szPath);
	SetCurrentDirectory(szStartPath);

	hBank0 = (HBITMAP)LoadImage(0, SZ_TX_OBJ_BANK_0, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE); 
	if(!hBank0)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_OBJ_BANK_0);
		LogWrite(str);
	}
	hBank2 = (HBITMAP)LoadImage(0, SZ_TX_OBJ_BANK_2, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE); 
	if(!hBank2)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_OBJ_BANK_2);
		LogWrite(str);
	}
	hBank3 = (HBITMAP)LoadImage(0, SZ_TX_OBJ_BANK_3, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE); 
	if(!hBank3)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_OBJ_BANK_3);
		LogWrite(str);
	}
	hBank4 = (HBITMAP)LoadImage(0, SZ_TX_OBJ_BANK_4, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE); 
	if(!hBank4)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_OBJ_BANK_4);
		LogWrite(str);
	}
	hBank5 = (HBITMAP)LoadImage(0, SZ_TX_OBJ_BANK_5, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE); 
	if(!hBank5)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_OBJ_BANK_5);
		LogWrite(str);
	}
	hBank6 = (HBITMAP)LoadImage(0, SZ_TX_OBJ_BANK_6, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE); 
	if(!hBank6)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_OBJ_BANK_6);
		LogWrite(str);
	}
	hBank7 = (HBITMAP)LoadImage(0, SZ_TX_OBJ_BANK_7, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE); 
	if(!hBank7)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_OBJ_BANK_7);
		LogWrite(str);
	}

	hMap0 = (HBITMAP)LoadImage(0, SZ_TX_MAP_0, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMap0)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_0);
		LogWrite(str);
	}
	hMap1 = (HBITMAP)LoadImage(0, SZ_TX_MAP_1, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMap1)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_1);
		LogWrite(str);
	}
	hMap2 = (HBITMAP)LoadImage(0, SZ_TX_MAP_2, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMap2)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_2);
		LogWrite(str);
	}
	hMap3 = (HBITMAP)LoadImage(0, SZ_TX_MAP_3, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMap3)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_3);
		LogWrite(str);
	}
	hMap4 = (HBITMAP)LoadImage(0, SZ_TX_MAP_4, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMap4)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_4);
		LogWrite(str);
	}
	hMap5 = (HBITMAP)LoadImage(0, SZ_TX_MAP_5, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMap5)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_5);
		LogWrite(str);
	}
	hMap6 = (HBITMAP)LoadImage(0, SZ_TX_MAP_6, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMap6)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_6);
		LogWrite(str);
	}
	hMap7 = (HBITMAP)LoadImage(0, SZ_TX_MAP_7, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMap7)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_7);
		LogWrite(str);
	}
	hMap8 = (HBITMAP)LoadImage(0, SZ_TX_MAP_8, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMap8)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_8);
		LogWrite(str);
	}
	hMap9 = (HBITMAP)LoadImage(0, SZ_TX_MAP_9, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMap9)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_9);
		LogWrite(str);
	}
	hMapA = (HBITMAP)LoadImage(0, SZ_TX_MAP_A, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapA)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_A);
		LogWrite(str);
	}
	hMapB = (HBITMAP)LoadImage(0, SZ_TX_MAP_B, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapB)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_B);
		LogWrite(str);
	}
	hMapC = (HBITMAP)LoadImage(0, SZ_TX_MAP_C, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapC)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_C);
		LogWrite(str);
	}
	hMapD = (HBITMAP)LoadImage(0, SZ_TX_MAP_D, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapD)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_D);
		LogWrite(str);
	}
	hMapE = (HBITMAP)LoadImage(0, SZ_TX_MAP_E, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapE)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_E);
		LogWrite(str);
	}
	hMapF = (HBITMAP)LoadImage(0, SZ_TX_MAP_F, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapF)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_F);
		LogWrite(str);
	}
	hMapG = (HBITMAP)LoadImage(0, SZ_TX_MAP_G, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapG)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_G);
		LogWrite(str);
	}
	hMapH = (HBITMAP)LoadImage(0, SZ_TX_MAP_H, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapH)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_H);
		LogWrite(str);
	}
	hMapI = (HBITMAP)LoadImage(0, SZ_TX_MAP_I, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapI)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_I);
		LogWrite(str);
	}
	hMapJ = (HBITMAP)LoadImage(0, SZ_TX_MAP_J, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapJ)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_J);
		LogWrite(str);
	}
	hMapK = (HBITMAP)LoadImage(0, SZ_TX_MAP_K, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapK)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_K);
		LogWrite(str);
	}
	hMapL = (HBITMAP)LoadImage(0, SZ_TX_MAP_L, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapL)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_L);
		LogWrite(str);
	}
	hMapM = (HBITMAP)LoadImage(0, SZ_TX_MAP_M, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapM)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_M);
		LogWrite(str);
	}
	hMapN = (HBITMAP)LoadImage(0, SZ_TX_MAP_N, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapN)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_N);
		LogWrite(str);
	}
	hMapO = (HBITMAP)LoadImage(0, SZ_TX_MAP_O, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapO)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_O);
		LogWrite(str);
	}
	hMapP = (HBITMAP)LoadImage(0, SZ_TX_MAP_P, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapP)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_P);
		LogWrite(str);
	}
	hMapQ = (HBITMAP)LoadImage(0, SZ_TX_MAP_Q, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapQ)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_Q);
		LogWrite(str);
	}
	hMapR = (HBITMAP)LoadImage(0, SZ_TX_MAP_R, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapR)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_R);
		LogWrite(str);
	}
	hMapS = (HBITMAP)LoadImage(0, SZ_TX_MAP_S, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapS)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_S);
		LogWrite(str);
	}
	hMapT = (HBITMAP)LoadImage(0, SZ_TX_MAP_T, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapT)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_T);
		LogWrite(str);
	}
	hMapU = (HBITMAP)LoadImage(0, SZ_TX_MAP_U, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapU)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_U);
		LogWrite(str);
	}
	hMapV = (HBITMAP)LoadImage(0, SZ_TX_MAP_V, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapV)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_V);
		LogWrite(str);
	}
	hMapW = (HBITMAP)LoadImage(0, SZ_TX_MAP_W, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapW)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_W);
		LogWrite(str);
	}
	hMapX = (HBITMAP)LoadImage(0, SZ_TX_MAP_X, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapX)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_X);
		LogWrite(str);
	}
	hMapY = (HBITMAP)LoadImage(0, SZ_TX_MAP_Y, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapY)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_Y);
		LogWrite(str);
	}
	hMapZ = (HBITMAP)LoadImage(0, SZ_TX_MAP_Z, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(!hMapZ)
	{
		sprintf(str, SZ_ERR_ENGINE_LOADTEXTURE, SZ_TX_MAP_Z);
		LogWrite(str);
	}

	SetCurrentDirectory(szPath);


	return S_OK;
}


long FinishDialogs()
{
	if(hBank0)
	{
		DeleteObject(hBank0);
		hBank0 = 0;
	}
	if(hBank2)
	{
		DeleteObject(hBank2);
		hBank2 = 0;
	}
	if(hBank3)
	{
		DeleteObject(hBank3);
		hBank3 = 0;
	}
	if(hBank4)
	{
		DeleteObject(hBank4);
		hBank4 = 0;
	}
	if(hBank5)
	{
		DeleteObject(hBank5);
		hBank5 = 0;
	}
	if(hBank6)
	{
		DeleteObject(hBank6);
		hBank6 = 0;
	}
	if(hBank7)
	{
		DeleteObject(hBank7);
		hBank7 = 0;
	}

	if(hMap0)
	{
		DeleteObject(hMap0);
		hMap0 = 0;
	}
	if(hMap1)
	{
		DeleteObject(hMap1);
		hMap1 = 0;
	}
	if(hMap2)
	{
		DeleteObject(hMap2);
		hMap2 = 0;
	}
	if(hMap3)
	{
		DeleteObject(hMap3);
		hMap3 = 0;
	}
	if(hMap4)
	{
		DeleteObject(hMap4);
		hMap4 = 0;
	}
	if(hMap5)
	{
		DeleteObject(hMap5);
		hMap5 = 0;
	}
	if(hMap6)
	{
		DeleteObject(hMap6);
		hMap6 = 0;
	}
	if(hMap7)
	{
		DeleteObject(hMap7);
		hMap7 = 0;
	}
	if(hMap8)
	{
		DeleteObject(hMap8);
		hMap8 = 0;
	}
	if(hMap9)
	{
		DeleteObject(hMap9);
		hMap9 = 0;
	}
	if(hMapA)
	{
		DeleteObject(hMapA);
		hMapA = 0;
	}
	if(hMapB)
	{
		DeleteObject(hMapB);
		hMapB = 0;
	}
	if(hMapC)
	{
		DeleteObject(hMapC);
		hMapC = 0;
	}
	if(hMapD)
	{
		DeleteObject(hMapD);
		hMapD = 0;
	}
	if(hMapE)
	{
		DeleteObject(hMapE);
		hMapE = 0;
	}
	if(hMapF)
	{
		DeleteObject(hMapF);
		hMapF = 0;
	}
	if(hMapG)
	{
		DeleteObject(hMapG);
		hMapG = 0;
	}
	if(hMapH)
	{
		DeleteObject(hMapH);
		hMapH = 0;
	}
	if(hMapI)
	{
		DeleteObject(hMapI);
		hMapI = 0;
	}
	if(hMapJ)
	{
		DeleteObject(hMapJ);
		hMapJ = 0;
	}
	if(hMapK)
	{
		DeleteObject(hMapK);
		hMapK = 0;
	}
	if(hMapL)
	{
		DeleteObject(hMapL);
		hMapL = 0;
	}
	if(hMapM)
	{
		DeleteObject(hMapM);
		hMapM = 0;
	}
	if(hMapN)
	{
		DeleteObject(hMapN);
		hMapN = 0;
	}
	if(hMapO)
	{
		DeleteObject(hMapO);
		hMapO = 0;
	}
	if(hMapP)
	{
		DeleteObject(hMapP);
		hMapP = 0;
	}
	if(hMapQ)
	{
		DeleteObject(hMapQ);
		hMapQ = 0;
	}
	if(hMapR)
	{
		DeleteObject(hMapR);
		hMapR = 0;
	}
	if(hMapS)
	{
		DeleteObject(hMapS);
		hMapS = 0;
	}
	if(hMapT)
	{
		DeleteObject(hMapT);
		hMapT = 0;
	}
	if(hMapU)
	{
		DeleteObject(hMapU);
		hMapU = 0;
	}
	if(hMapV)
	{
		DeleteObject(hMapV);
		hMapV = 0;
	}
	if(hMapW)
	{
		DeleteObject(hMapW);
		hMapW = 0;
	}
	if(hMapX)
	{
		DeleteObject(hMapX);
		hMapX = 0;
	}
	if(hMapY)
	{
		DeleteObject(hMapY);
		hMapY = 0;
	}
	if(hMapZ)
	{
		DeleteObject(hMapZ);
		hMapZ = 0;
	}

	if(hDlgDevice)				DlgDeviceToggle();
	if(hDlgDeviceConfirm)		DlgDeviceConfirmToggle();
	if(hDlgMenuBar)				MenuBarToggle();
	if(hDlgBrush)				DlgBrushToggle();
	if(hDlgFlagsBrush)			DlgFlagsBrushToggle();
	if(hDlgObject)				DlgObjectToggle();
	if(hDlgFilter)				DlgFilterToggle();
	if(hDlgDiscovery)			DlgDiscoveryToggle();
	if(hDlgTrigger)				DlgTriggerToggle();
	if(hDlgLink)				DlgLinkToggle();
	if(hDlgList)				DlgListToggle();
	if(hDlgAbout)				DlgAboutToggle();
	if(hDlgAllies)				DlgAlliesToggle();
	if(hDlgSpellsBuildings)		DlgSpellsBuildingsToggle();
	if (hDlgScript2)			DlgScript2();
	if(hDlgSpellsNotCharging)	DlgSpellsNotChargingToggle();
	if(hDlgHeader)				DlgHeaderToggle();
	if(hDlgObjBank)				DlgObjBankToggle();
	if(hDlgMapType)				DlgMapTypeToggle();
	if(hDlgInfo)				DlgInfoToggle();
	if(hDlgSwapTribe)			DlgSwapTribeToggle();
	if(hDlgMarkers)				DlgMarkersToggle();
	if(hDlgAIAttrib)			DlgAIAttribToggle();
	if(hDlgAIScript)			DlgAIScriptToggle();

	return S_OK;
}


void LockDialogs(bool lock)
{
	if(hDlgDevice)				EnableWindow(hDlgDevice, !lock);
	if(hDlgMenuBar)				EnableWindow(hDlgMenuBar, !lock);
	if(hDlgBrush)				EnableWindow(hDlgBrush, !lock);
	if(hDlgObject)				EnableWindow(hDlgObject, !lock);
	if(hDlgFilter)				EnableWindow(hDlgFilter, !lock);
	if(hDlgDiscovery)			EnableWindow(hDlgDiscovery, !lock);
	if(hDlgTrigger)				EnableWindow(hDlgTrigger, !lock);
	if(hDlgLink)				EnableWindow(hDlgLink, !lock);
	if(hDlgList)				EnableWindow(hDlgList, !lock);
	if(hDlgAbout)				EnableWindow(hDlgAbout, !lock);
	if(hDlgAllies)				EnableWindow(hDlgAllies, !lock);
	if(hDlgSpellsBuildings)		EnableWindow(hDlgSpellsBuildings, !lock);
	if(hDlgSpellsNotCharging)	EnableWindow(hDlgSpellsNotCharging, !lock);
	if(hDlgHeader)				EnableWindow(hDlgHeader, !lock);
	if(hDlgObjBank)				EnableWindow(hDlgObjBank, !lock);
	if(hDlgMapType)				EnableWindow(hDlgMapType, !lock);
	if(hDlgInfo)				EnableWindow(hDlgInfo, !lock);
	if(hDlgSwapTribe)			EnableWindow(hDlgSwapTribe, !lock);
	if(hDlgMarkers)				EnableWindow(hDlgMarkers, !lock);
	if(hDlgAIAttrib)			EnableWindow(hDlgAIAttrib, !lock);
	if(hDlgAIScript)			EnableWindow(hDlgAIScript, !lock);
}


int ModalMsg(LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	LockDialogs(true);
	int i = MessageBox(hMainWnd, lpText, lpCaption, uType);
	LockDialogs(false);
	return i;
}


void CenterDialog(HWND hWnd)
{
	RECT rc;
	GetWindowRect(hWnd, &rc);
	rc.right -= rc.left;
	rc.bottom -= rc.top;
	rc.left = fFullScreen ? ((dwScreenWidth - rc.right) / 2) : ((dwScreenWidthN - rc.right) / 2);
	rc.top  = fFullScreen ? ((dwScreenHeight - rc.bottom) / 2) : ((dwScreenHeightN - rc.bottom) / 2);
	SetWindowPos(hWnd, 0, rc.left, rc.top, 0, 0, SWP_NOSIZE);
}


void GetThingName(THING *t, char *str)
{
	switch(t->Thing.Type)
	{
	case T_PERSON:
		switch(t->Thing.Model)
		{
		case M_PERSON_WILD:         strcpy(str, SZ_PERSON_WILD);         break;
		case M_PERSON_BRAVE:        strcpy(str, SZ_PERSON_BRAVE);        break;
		case M_PERSON_WARRIOR:      strcpy(str, SZ_PERSON_WARRIOR);      break;
		case M_PERSON_PRIEST:       strcpy(str, SZ_PERSON_PRIEST);       break;
		case M_PERSON_SPY:          strcpy(str, SZ_PERSON_SPY);          break;
		case M_PERSON_FIREWARRIOR:  strcpy(str, SZ_PERSON_FIREWARRIOR);  break;
		case M_PERSON_SHAMAN:       strcpy(str, SZ_PERSON_SHAMAN);       break;
		case M_PERSON_ANGELOFDEATH: strcpy(str, SZ_PERSON_ANGELOFDEATH); break;
		default: sprintf(str, "%s: %s", SZ_PERSON, SZ_UNKNOW);
		}
		break;
	case T_BUILDING:
		switch(t->Thing.Model)
		{
		case M_BUILDING_HUT1:              strcpy(str, SZ_BUILDING_HUT1);              break;
		case M_BUILDING_HUT2:              strcpy(str, SZ_BUILDING_HUT2);              break;
		case M_BUILDING_HUT3:              strcpy(str, SZ_BUILDING_HUT3);              break;
		case M_BUILDING_TOWER:             strcpy(str, SZ_BUILDING_TOWER);             break;
		case M_BUILDING_TEMPLE:            strcpy(str, SZ_BUILDING_TEMPLE);            break;
		case M_BUILDING_SPY_TRAIN:         strcpy(str, SZ_BUILDING_SPY_TRAIN);         break;
		case M_BUILDING_WARRIOR_TRAIN:     strcpy(str, SZ_BUILDING_WARRIOR_TRAIN);     break;
		case M_BUILDING_FIREWARRIOR_TRAIN: strcpy(str, SZ_BUILDING_FIREWARRIOR_TRAIN); break;
		case M_BUILDING_BOAT_HUT_1:
		case M_BUILDING_BOAT_HUT_2:        strcpy(str, SZ_BUILDING_BOAT_HUT);          break;
		case M_BUILDING_AIRSHIP_HUT_1:
		case M_BUILDING_AIRSHIP_HUT_2:     strcpy(str, SZ_BUILDING_AIRSHIP_HUT);       break;
		case M_BUILDING_KNOWLEDGE:         strcpy(str, SZ_BUILDING_KNOWLEDGE);         break;
		case M_BUILDING_PRISON:            strcpy(str, SZ_BUILDING_PRISON);            break;
		default: sprintf(str, "%s: %s", SZ_BUILDING, SZ_UNKNOW);
		}
		break;
	case T_SCENERY:
		if(t->Thing.Model == M_SCENERY_TOP_LEVEL_SCENERY)
		{
			switch(t->Thing.Scenery.IslandNum)
			{
			case M_SPECIAL_A: strcpy(str, SZ_SPECIAL_A); break;
			case M_SPECIAL_B: strcpy(str, SZ_SPECIAL_B); break;
			case M_SPECIAL_C: strcpy(str, SZ_SPECIAL_C); break;
			case M_SPECIAL_D: strcpy(str, SZ_SPECIAL_D); break;
			case M_SPECIAL_E: strcpy(str, SZ_SPECIAL_E); break;
			case M_SPECIAL_F: strcpy(str, SZ_SPECIAL_F); break;
			default: sprintf(str, "%s: %s", SZ_SPECIAL, SZ_UNKNOW);
			}
			break;
		}
		else if (t->Thing.Model == M_SCENERY_SUB_LEVEL_SCENERY)
		{
			switch(t->Thing.Scenery.IslandNum)
			{
			case M_DECORATION_BOOK: strcpy(str, SZ_DECORATION_BOOK); break;
			case M_DECORATION_SHIELD: strcpy(str, SZ_DECORATION_SHIELD); break;
			case M_DECORATION_WELL: strcpy(str, SZ_DECORATION_WELL); break;
			case M_DECORATION_WINDMILL: strcpy(str, SZ_DECORATION_WINDMILL); break;
			case M_DECORATION_WOODEN_HUT: strcpy(str, SZ_DECORATION_WOODEN_HUT); break;
			case M_DECORATION_B_TOWER: strcpy(str, SZ_DECORATION_B_TOWER); break;
			case M_DECORATION_R_TOWER: strcpy(str, SZ_DECORATION_R_TOWER); break;
			case M_DECORATION_Y_TOWER: strcpy(str, SZ_DECORATION_Y_TOWER); break;
			case M_DECORATION_G_TOWER: strcpy(str, SZ_DECORATION_G_TOWER); break;
			case M_DECORATION_B_BOAT_HUT: strcpy(str, SZ_DECORATION_B_BOAT_HUT); break;
			case M_DECORATION_R_BOAT_HUT: strcpy(str, SZ_DECORATION_R_BOAT_HUT); break;
			case M_DECORATION_Y_BOAT_HUT: strcpy(str, SZ_DECORATION_Y_BOAT_HUT); break;
			case M_DECORATION_G_BOAT_HUT: strcpy(str, SZ_DECORATION_G_BOAT_HUT); break;
			case M_DECORATION_B_AIRSHIP_HUT: strcpy(str, SZ_DECORATION_B_AIRSHIP_HUT); break;
			case M_DECORATION_R_AIRSHIP_HUT: strcpy(str, SZ_DECORATION_R_AIRSHIP_HUT); break;
			case M_DECORATION_Y_AIRSHIP_HUT: strcpy(str, SZ_DECORATION_Y_AIRSHIP_HUT); break;
			case M_DECORATION_G_AIRSHIP_HUT: strcpy(str, SZ_DECORATION_G_AIRSHIP_HUT); break;
			case M_DECORATION_B_SPY_TRAIN: strcpy(str, SZ_DECORATION_B_SPY_TRAIN); break;
			case M_DECORATION_R_SPY_TRAIN: strcpy(str, SZ_DECORATION_R_SPY_TRAIN); break;
			case M_DECORATION_Y_SPY_TRAIN: strcpy(str, SZ_DECORATION_Y_SPY_TRAIN); break;
			case M_DECORATION_G_SPY_TRAIN: strcpy(str, SZ_DECORATION_G_SPY_TRAIN); break;
			case M_DECORATION_B_TEMPLE: strcpy(str, SZ_DECORATION_B_TEMPLE); break;
			case M_DECORATION_R_TEMPLE: strcpy(str, SZ_DECORATION_R_TEMPLE); break;
			case M_DECORATION_Y_TEMPLE: strcpy(str, SZ_DECORATION_Y_TEMPLE); break;
			case M_DECORATION_G_TEMPLE: strcpy(str, SZ_DECORATION_G_TEMPLE); break;
			case M_DECORATION_B_FIREWARRIOR_TRAIN: strcpy(str, SZ_DECORATION_B_FIREWARRIOR_TRAIN); break;
			case M_DECORATION_R_FIREWARRIOR_TRAIN: strcpy(str, SZ_DECORATION_R_FIREWARRIOR_TRAIN); break;
			case M_DECORATION_Y_FIREWARRIOR_TRAIN: strcpy(str, SZ_DECORATION_Y_FIREWARRIOR_TRAIN); break;
			case M_DECORATION_G_FIREWARRIOR_TRAIN: strcpy(str, SZ_DECORATION_G_FIREWARRIOR_TRAIN); break;
			case M_DECORATION_B_WARRIOR_TRAIN: strcpy(str, SZ_DECORATION_B_WARRIOR_TRAIN); break;
			case M_DECORATION_R_WARRIOR_TRAIN: strcpy(str, SZ_DECORATION_R_WARRIOR_TRAIN); break;
			case M_DECORATION_Y_WARRIOR_TRAIN: strcpy(str, SZ_DECORATION_Y_WARRIOR_TRAIN); break;
			case M_DECORATION_G_WARRIOR_TRAIN: strcpy(str, SZ_DECORATION_G_WARRIOR_TRAIN); break;
			case M_DECORATION_B_HUT1_A: strcpy(str, SZ_DECORATION_B_HUT1_A); break;
			case M_DECORATION_B_HUT1_B: strcpy(str, SZ_DECORATION_B_HUT1_B); break;
			case M_DECORATION_B_HUT1_C: strcpy(str, SZ_DECORATION_B_HUT1_C); break;
			case M_DECORATION_R_HUT1_A: strcpy(str, SZ_DECORATION_R_HUT1_A); break;
			case M_DECORATION_R_HUT1_B: strcpy(str, SZ_DECORATION_R_HUT1_B); break;
			case M_DECORATION_R_HUT1_C: strcpy(str, SZ_DECORATION_R_HUT1_C); break;
			case M_DECORATION_Y_HUT1_A: strcpy(str, SZ_DECORATION_Y_HUT1_A); break;
			case M_DECORATION_Y_HUT1_B: strcpy(str, SZ_DECORATION_Y_HUT1_B); break;
			case M_DECORATION_Y_HUT1_C: strcpy(str, SZ_DECORATION_Y_HUT1_C); break;
			case M_DECORATION_G_HUT1_A: strcpy(str, SZ_DECORATION_G_HUT1_A); break;
			case M_DECORATION_G_HUT1_B: strcpy(str, SZ_DECORATION_G_HUT1_B); break;
			case M_DECORATION_G_HUT1_C: strcpy(str, SZ_DECORATION_G_HUT1_C); break;
			case M_DECORATION_B_HUT2_A: strcpy(str, SZ_DECORATION_B_HUT2_A); break;
			case M_DECORATION_B_HUT2_B: strcpy(str, SZ_DECORATION_B_HUT2_B); break;
			case M_DECORATION_B_HUT2_C: strcpy(str, SZ_DECORATION_B_HUT2_C); break;
			case M_DECORATION_R_HUT2_A: strcpy(str, SZ_DECORATION_R_HUT2_A); break;
			case M_DECORATION_R_HUT2_B: strcpy(str, SZ_DECORATION_R_HUT2_B); break;
			case M_DECORATION_R_HUT2_C: strcpy(str, SZ_DECORATION_R_HUT2_C); break;
			case M_DECORATION_Y_HUT2_A: strcpy(str, SZ_DECORATION_Y_HUT2_A); break;
			case M_DECORATION_Y_HUT2_B: strcpy(str, SZ_DECORATION_Y_HUT2_B); break;
			case M_DECORATION_Y_HUT2_C: strcpy(str, SZ_DECORATION_Y_HUT2_C); break;
			case M_DECORATION_G_HUT2_A: strcpy(str, SZ_DECORATION_G_HUT2_A); break;
			case M_DECORATION_G_HUT2_B: strcpy(str, SZ_DECORATION_G_HUT2_B); break;
			case M_DECORATION_G_HUT2_C: strcpy(str, SZ_DECORATION_G_HUT2_C); break;
			case M_DECORATION_B_HUT3_A: strcpy(str, SZ_DECORATION_B_HUT3_A); break;
			case M_DECORATION_B_HUT3_B: strcpy(str, SZ_DECORATION_B_HUT3_B); break;
			case M_DECORATION_B_HUT3_C: strcpy(str, SZ_DECORATION_B_HUT3_C); break;
			case M_DECORATION_R_HUT3_A: strcpy(str, SZ_DECORATION_R_HUT3_A); break;
			case M_DECORATION_R_HUT3_B: strcpy(str, SZ_DECORATION_R_HUT3_B); break;
			case M_DECORATION_R_HUT3_C: strcpy(str, SZ_DECORATION_R_HUT3_C); break;
			case M_DECORATION_Y_HUT3_A: strcpy(str, SZ_DECORATION_Y_HUT3_A); break;
			case M_DECORATION_Y_HUT3_B: strcpy(str, SZ_DECORATION_Y_HUT3_B); break;
			case M_DECORATION_Y_HUT3_C: strcpy(str, SZ_DECORATION_Y_HUT3_C); break;
			case M_DECORATION_G_HUT3_A: strcpy(str, SZ_DECORATION_G_HUT3_A); break;
			case M_DECORATION_G_HUT3_B: strcpy(str, SZ_DECORATION_G_HUT3_B); break;
			case M_DECORATION_G_HUT3_C: strcpy(str, SZ_DECORATION_G_HUT3_C); break;
			case M_DECORATION_BOAT: strcpy(str, SZ_DECORATION_BOAT); break;
			case M_DECORATION_BALLOON: strcpy(str, SZ_DECORATION_BALLOON); break;
			case M_DECORATION_TOTEM_0: strcpy(str, SZ_DECORATION_TOTEM_0); break;
			case M_DECORATION_TOTEM_1: strcpy(str, SZ_DECORATION_TOTEM_1); break;
			case M_DECORATION_KNOWLEDGE_0: strcpy(str, SZ_DECORATION_KNOWLEDGE_0); break;
			case M_DECORATION_KNOWLEDGE_1: strcpy(str, SZ_DECORATION_KNOWLEDGE_1); break;
			case M_DECORATION_KNOWLEDGE_2: strcpy(str, SZ_DECORATION_KNOWLEDGE_2); break;
			case M_DECORATION_PRISON: strcpy(str, SZ_DECORATION_PRISON); break;
			case M_DECORATION_OBELISK: strcpy(str, SZ_DECORATION_OBELISK); break;
			case M_DECORATION_GARGOYLE_0: strcpy(str, SZ_DECORATION_GARGOYLE_0); break;
			case M_DECORATION_GARGOYLE_1: strcpy(str, SZ_DECORATION_GARGOYLE_1); break;
			case M_DECORATION_GARGOYLE_2: strcpy(str, SZ_DECORATION_GARGOYLE_2); break;
			case M_DECORATION_PILLAR: strcpy(str, SZ_DECORATION_PILLAR); break;
			case M_DECORATION_AOD_0: strcpy(str, SZ_DECORATION_AOD_0); break;
			case M_DECORATION_AOD_1: strcpy(str, SZ_DECORATION_AOD_1); break;
			case M_DECORATION_STONEHEAD: strcpy(str, SZ_DECORATION_STONEHEAD); break;
			case M_DECORATION_TREE_1: strcpy(str, SZ_DECORATION_TREE_1); break;
			case M_DECORATION_TREE_2: strcpy(str, SZ_DECORATION_TREE_2); break;
			case M_DECORATION_TREE_3: strcpy(str, SZ_DECORATION_TREE_3); break;
			case M_DECORATION_TREE_4: strcpy(str, SZ_DECORATION_TREE_4); break;
			case M_DECORATION_TREE_5: strcpy(str, SZ_DECORATION_TREE_5); break;
			case M_DECORATION_TREE_6: strcpy(str, SZ_DECORATION_TREE_6); break;
			default: sprintf(str, "%s: %s", SZ_SPECIAL, SZ_UNKNOW);
			}
			break;
		}
	
		
		switch (t->Thing.Model)
		{
		case M_SCENERY_TREE_1:     strcpy(str, SZ_SCENERY_TREE_1);     break;
		case M_SCENERY_TREE_2:     strcpy(str, SZ_SCENERY_TREE_2);     break;
		case M_SCENERY_TREE_3:     strcpy(str, SZ_SCENERY_TREE_3);     break;
		case M_SCENERY_TREE_4:     strcpy(str, SZ_SCENERY_TREE_4);     break;
		case M_SCENERY_TREE_5:     strcpy(str, SZ_SCENERY_TREE_5);     break;
		case M_SCENERY_TREE_6:     strcpy(str, SZ_SCENERY_TREE_6);     break;
		case M_SCENERY_PLANT_1:    strcpy(str, SZ_SCENERY_PLANT_1);    break;
		case M_SCENERY_PLANT_2:    strcpy(str, SZ_SCENERY_PLANT_2);    break;
		case M_SCENERY_STONE_HEAD: strcpy(str, SZ_SCENERY_STONE_HEAD); break;
		case M_SCENERY_WOOD_PILE:  strcpy(str, SZ_SCENERY_WOOD_PILE);  break;
		case M_SCENERY_RS_PILLAR:  strcpy(str, SZ_SCENERY_RS_PILLAR);  break;
		case M_SCENERY_ROCK:       strcpy(str, SZ_SCENERY_ROCK);       break;
		case M_SCENERY_FIRE:       strcpy(str, SZ_SCENERY_FIRE);       break;
		case M_SCENERY_FORBIDDEN:  strcpy(str, SZ_SCENERY_FORBIDDEN);  break;
		default: sprintf(str, "%s: %s", SZ_SCENERY, SZ_UNKNOW);
		}
		break;
	case T_GENERAL:
		switch(t->Thing.Model)
		{
		case M_GENERAL_DISCOVERY:       strcpy(str, SZ_GENERAL_DISCOVERY);       break;
		case M_GENERAL_TRIGGER:         strcpy(str, SZ_GENERAL_TRIGGER);         break;
		case M_GENERAL_BUILDING_ADD_ON: strcpy(str, SZ_GENERAL_BUILDING_ADD_ON); break;
		case M_GENERAL_LIGHT:           strcpy(str, SZ_GENERAL_LIGHT);           break;
		default: sprintf(str, "%s: %s", SZ_GENERAL, SZ_UNKNOW);
		}
		break;
	case T_SPELL:
		switch(t->Thing.Model)
		{
		case M_SPELL_BURN:           strcpy(str, SZ_SPELL_BURN);           break;
		case M_SPELL_BLAST:          strcpy(str, SZ_SPELL_BLAST);          break;
		case M_SPELL_LIGHTNING:      strcpy(str, SZ_SPELL_LIGHTNING);      break;
		case M_SPELL_TORNADO:        strcpy(str, SZ_SPELL_TORNADO);        break;
		case M_SPELL_SWARM:          strcpy(str, SZ_SPELL_SWARM);          break;
		case M_SPELL_INVISIBILITY:   strcpy(str, SZ_SPELL_INVISIBILITY);   break;
		case M_SPELL_HYPNOTISM:      strcpy(str, SZ_SPELL_HYPNOTISM);      break;
		case M_SPELL_FIRESTORM:      strcpy(str, SZ_SPELL_FIRESTORM);      break;
		case M_SPELL_GHOST_ARMY:     strcpy(str, SZ_SPELL_GHOST_ARMY);     break;
		case M_SPELL_ERODE:          strcpy(str, SZ_SPELL_ERODE);          break;
		case M_SPELL_SWAMP:          strcpy(str, SZ_SPELL_SWAMP);          break;
		case M_SPELL_LAND_BRIDGE:    strcpy(str, SZ_SPELL_LAND_BRIDGE);    break;
		case M_SPELL_ANGEL_OF_DEATH: strcpy(str, SZ_SPELL_ANGEL_OF_DEATH); break;
		case M_SPELL_EARTHQUAKE:     strcpy(str, SZ_SPELL_EARTHQUAKE);     break;
		case M_SPELL_FLATTEN:        strcpy(str, SZ_SPELL_FLATTEN);        break;
		case M_SPELL_VOLCANO:        strcpy(str, SZ_SPELL_VOLCANO);        break;
		case M_SPELL_CONVERT:        strcpy(str, SZ_SPELL_CONVERT);        break;
		case M_SPELL_ARMAGEDDON:     strcpy(str, SZ_SPELL_ARMAGEDDON);     break;
		case M_SPELL_MAGICAL_SHIELD: strcpy(str, SZ_SPELL_MAGICAL_SHIELD); break;
		case M_SPELL_BLOODLUST:      strcpy(str, SZ_SPELL_BLOODLUST);      break;
		case M_SPELL_TELEPORT:       strcpy(str, SZ_SPELL_TELEPORT);       break;
		default: sprintf(str, "%s: %s", SZ_SPELL, SZ_UNKNOW);
		}
		break;
	case T_EFFECT:
		sprintf(str, "%s: ", SZ_EFFECT);
		switch(t->Thing.Model)
		{
		case M_EFFECT_SIMPLE_BLAST:            strcat(str, SZ_EFFECT_SIMPLE_BLAST);            break;
		case M_EFFECT_SPRITE_CIRCLES:          strcat(str, SZ_EFFECT_SPRITE_CIRCLES);          break;
		case M_EFFECT_SMOKE:                   strcat(str, SZ_EFFECT_SMOKE);                   break;
		case M_EFFECT_LIGHTNING_ELEM:          strcat(str, SZ_EFFECT_LIGHTNING_ELEM);          break;
		case M_EFFECT_BURN_CELL_OBSTACLES:     strcat(str, SZ_EFFECT_BURN_CELL_OBSTACLES);     break;
		case M_EFFECT_FLATTEN_LAND:            strcat(str, SZ_EFFECT_FLATTEN_LAND);            break;
		case M_EFFECT_SPHERE_EXPLODE_1:        strcat(str, SZ_EFFECT_SPHERE_EXPLODE_1);        break;
		case M_EFFECT_FIREBALL:                strcat(str, SZ_EFFECT_FIREBALL);                break;
		case M_EFFECT_FIRECLOUD:               strcat(str, SZ_EFFECT_FIRECLOUD);               break;
		case M_EFFECT_GHOST_ARMY:              strcat(str, SZ_EFFECT_GHOST_ARMY);              break;
		case M_EFFECT_INVISIBILITY:            strcat(str, SZ_EFFECT_INVISIBILITY);            break;
		case M_EFFECT_VOLCANO:                 strcat(str, SZ_EFFECT_VOLCANO);                 break;
		case M_EFFECT_HYPNOTISM:               strcat(str, SZ_EFFECT_HYPNOTISM);               break;
		case M_EFFECT_LIGHTNING_BOLT:          strcat(str, SZ_EFFECT_LIGHTNING_BOLT);          break;
		case M_EFFECT_SWAMP:                   strcat(str, SZ_EFFECT_SWAMP);                   break;
		case M_EFFECT_ANGEL_OF_DEATH:          strcat(str, SZ_EFFECT_ANGEL_OF_DEATH);          break;
		case M_EFFECT_WHIRLWIND:               strcat(str, SZ_EFFECT_WHIRLWIND);               break;
		case M_EFFECT_INSECT_PLAGUE:           strcat(str, SZ_EFFECT_INSECT_PLAGUE);           break;
		case M_EFFECT_FIRESTORM:               strcat(str, SZ_EFFECT_FIRESTORM);               break;
		case M_EFFECT_EROSION:                 strcat(str, SZ_EFFECT_EROSION);                 break;
		case M_EFFECT_LAND_BRIDGE:             strcat(str, SZ_EFFECT_LAND_BRIDGE);             break;
		case M_EFFECT_EARTHQUAKE:              strcat(str, SZ_EFFECT_EARTHQUAKE);              break;
		case M_EFFECT_FLY_THINGUMMY:           strcat(str, SZ_EFFECT_FLY_THINGUMMY);           break;
		case M_EFFECT_SPHERE_EXPLODE_AND_FIRE: strcat(str, SZ_EFFECT_SPHERE_EXPLODE_AND_FIRE); break;
		case M_EFFECT_BIG_FIRE:                strcat(str, SZ_EFFECT_BIG_FIRE);                break;
		case M_EFFECT_LIGHTNING:               strcat(str, SZ_EFFECT_LIGHTNING);               break;
		case M_EFFECT_FLATTEN:                 strcat(str, SZ_EFFECT_FLATTEN);                 break;
		case M_EFFECT_LAVA_FLOW:               strcat(str, SZ_EFFECT_LAVA_FLOW);               break;
		case M_EFFECT_PURIFY_LAND:             strcat(str, SZ_EFFECT_PURIFY_LAND);             break;
		case M_EFFECT_EXPLOSION_1:             strcat(str, SZ_EFFECT_EXPLOSION_1);             break;
		case M_EFFECT_EXPLOSION_2:             strcat(str, SZ_EFFECT_EXPLOSION_2);             break;
		case M_EFFECT_LAVA_SQUARE:             strcat(str, SZ_EFFECT_LAVA_SQUARE);             break;
		case M_EFFECT_WW_ELEMENT:              strcat(str, SZ_EFFECT_WW_ELEMENT);              break;
		case M_EFFECT_WW_DUST:                 strcat(str, SZ_EFFECT_WW_DUST);                 break;
		case M_EFFECT_HILL:                    strcat(str, SZ_EFFECT_HILL);                    break;
		case M_EFFECT_VALLEY:                  strcat(str, SZ_EFFECT_VALLEY);                  break;
		case M_EFFECT_PLACE_TREE:              strcat(str, SZ_EFFECT_PLACE_TREE);              break;
		case M_EFFECT_RISE:                    strcat(str, SZ_EFFECT_RISE);                    break;
		case M_EFFECT_DIP:                     strcat(str, SZ_EFFECT_DIP);                     break;
		case M_EFFECT_REIN_ROCK_DEBRIS:        strcat(str, SZ_EFFECT_REIN_ROCK_DEBRIS);        break;
		case M_EFFECT_PLACE_SHAMAN:            strcat(str, SZ_EFFECT_PLACE_SHAMAN);            break;
		case M_EFFECT_PLACE_WILD:              strcat(str, SZ_EFFECT_PLACE_WILD);              break;
		case M_EFFECT_BLDG_SMOKE:              strcat(str, SZ_EFFECT_BLDG_SMOKE);              break;
		case M_EFFECT_MUCH_SIMPLER_BLAST:      strcat(str, SZ_EFFECT_MUCH_SIMPLER_BLAST);      break;
		case M_EFFECT_TUMBLING_BRANCH:         strcat(str, SZ_EFFECT_TUMBLING_BRANCH);         break;
		case M_EFFECT_SPARKLE:                 strcat(str, SZ_EFFECT_SPARKLE);                 break;
		case M_EFFECT_SMALL_SPARKLE:           strcat(str, SZ_EFFECT_SMALL_SPARKLE);           break;
		case M_EFFECT_EXPLOSION_3:             strcat(str, SZ_EFFECT_EXPLOSION_3);             break;
		case M_EFFECT_ROCK_EXPLOSION:          strcat(str, SZ_EFFECT_ROCK_EXPLOSION);          break;
		case M_EFFECT_LAVA_GLOOP:              strcat(str, SZ_EFFECT_LAVA_GLOOP);              break;
		case M_EFFECT_SPLASH:                  strcat(str, SZ_EFFECT_SPLASH);                  break;
		case M_EFFECT_SMOKE_CLOUD:             strcat(str, SZ_EFFECT_SMOKE_CLOUD);             break;
		case M_EFFECT_SMOKE_CLOUD_CONSTANT:    strcat(str, SZ_EFFECT_SMOKE_CLOUD_CONSTANT);    break;
		case M_EFFECT_FIREBALL_2:              strcat(str, SZ_EFFECT_FIREBALL_2);              break;
		case M_EFFECT_GROUND_SHOCKWAVE:        strcat(str, SZ_EFFECT_GROUND_SHOCKWAVE);        break;
		case M_EFFECT_ORBITER:                 strcat(str, SZ_EFFECT_ORBITER);                 break;
		case M_EFFECT_BIG_SPARKLE:             strcat(str, SZ_EFFECT_BIG_SPARKLE);             break;
		case M_EFFECT_METEOR:                  strcat(str, SZ_EFFECT_METEOR);                  break;
		case M_EFFECT_CONVERT_WILD:            strcat(str, SZ_EFFECT_CONVERT_WILD);            break;
		case M_EFFECT_BLDG_SMOKE_2_FULL:       strcat(str, SZ_EFFECT_BLDG_SMOKE_2_FULL);       break;
		case M_EFFECT_BLDG_SMOKE_2_PARTIAL:    strcat(str, SZ_EFFECT_BLDG_SMOKE_2_PARTIAL);    break;
		case M_EFFECT_BLDG_DAMAGED_SMOKE:      strcat(str, SZ_EFFECT_BLDG_DAMAGED_SMOKE);      break;
		case M_EFFECT_DELETE_RS_PILLARS:       strcat(str, SZ_EFFECT_DELETE_RS_PILLARS);       break;
		case M_EFFECT_SPELL_BLAST:             strcat(str, SZ_EFFECT_SPELL_BLAST);             break;
		case M_EFFECT_FIRESTORM_SMOKE:         strcat(str, SZ_EFFECT_FIRESTORM_SMOKE);         break;
		case M_EFFECT_REVEAL_FOG_AREA:         strcat(str, SZ_EFFECT_REVEAL_FOG_AREA);         break;
		case M_EFFECT_SHIELD:                  strcat(str, SZ_EFFECT_SHIELD);                  break;
		case M_EFFECT_BOAT_HUT_REPAIR:         strcat(str, SZ_EFFECT_BOAT_HUT_REPAIR);         break;
		case M_EFFECT_REEDY_GRASS:             strcat(str, SZ_EFFECT_REEDY_GRASS);             break;
		case M_EFFECT_SWAMP_MIST:              strcat(str, SZ_EFFECT_SWAMP_MIST);              break;
		case M_EFFECT_ARMAGEDDON:              strcat(str, SZ_EFFECT_ARMAGEDDON);              break;
		case M_EFFECT_BLOODLUST:               strcat(str, SZ_EFFECT_BLOODLUST);               break;
		case M_EFFECT_TELEPORT:                strcat(str, SZ_EFFECT_TELEPORT);                break;
		case M_EFFECT_ATLANTIS_SET:            strcat(str, SZ_EFFECT_ATLANTIS_SET);            break;
		case M_EFFECT_ATLANTIS_INVOKE:         strcat(str, SZ_EFFECT_ATLANTIS_INVOKE);         break;
		case M_EFFECT_STATUE_TO_AOD:           strcat(str, SZ_EFFECT_STATUE_TO_AOD);           break;
		case M_EFFECT_FILL_ONE_SHOTS:          strcat(str, SZ_EFFECT_FILL_ONE_SHOTS);          break;
#if _ASCENSION
		case M_EFFECT_RESURRECT:			   strcat(str, SZ_EFFECT_RESURRECT);			   break;
        case M_EFFECT_BIG_SMOKE:			   strcat(str, SZ_EFFECT_BIG_SMOKE);			   break;
		case M_EFFECT_FIRESTORM_BALL:		   strcat(str, SZ_EFFECT_FIRESTORM_BALL);		   break;
		case M_EFFECT_LAVA_FLOW_NEW:		   strcat(str, SZ_EFFECT_LAVA_FLOW_NEW);		   break;
		case M_EFFECT_LAVA_FLOW_NEW_SHALLOW:   strcat(str, SZ_EFFECT_LAVA_FLOW_NEW_SHALLOW);   break;
		case M_EFFECT_LAVA_FLOW_NEW_DEEP:	   strcat(str, SZ_EFFECT_LAVA_FLOW_NEW_DEEP);	   break;
		case M_EFFECT_WATER:				   strcat(str, SZ_EFFECT_WATER);				   break;
		case M_EFFECT_WATER_1X1:			   strcat(str, SZ_EFFECT_WATER_1X1);			   break;
		case M_EFFECT_LAVA_FLOW_NEW_WHITE_SHALLOW:		strcat(str, SZ_EFFECT_LAVA_FLOW_NEW_WHITE_SHALLOW);			   break;
		case M_EFFECT_LAVA_FLOW_NEW_WHITE_DEEP:			strcat(str, SZ_EFFECT_LAVA_FLOW_NEW_WHITE_DEEP);			   break;
		case M_EFFECT_LAVA_FIXED:						strcat(str, SZ_EFFECT_LAVA_FIXED);			   break;
		case M_EFFECT_SOUL:						strcat(str, SZ_EFFECT_SOUL);			   break;
		case M_EFFECT_NEW_PLANT1:			   strcat(str, SZ_EFFECT_NEW_PLANT1);			   break;
		case M_EFFECT_NEW_PLANT2:			   strcat(str, SZ_EFFECT_NEW_PLANT2);			   break;
		case M_EFFECT_NEW_PLANT3:			   strcat(str, SZ_EFFECT_NEW_PLANT3);			   break;
		case M_EFFECT_NEW_PLANT4:			   strcat(str, SZ_EFFECT_NEW_PLANT4);			   break;
		case M_EFFECT_NEW_BRANCH:			   strcat(str, SZ_EFFECT_NEW_BRANCH);			   break;
		case M_EFFECT_BONEPILE:				   strcat(str, SZ_EFFECT_BONEPILE);				   break;
		case M_EFFECT_SMALL_SKULL:			   strcat(str, SZ_EFFECT_SMALL_SKULL);			   break;
		case M_EFFECT_SMALL_SKULL2:			   strcat(str, SZ_EFFECT_SMALL_SKULL2);			   break;
		case M_EFFECT_SMALL_SKULL3:			   strcat(str, SZ_EFFECT_SMALL_SKULL3);			   break;
		case M_EFFECT_SKULL_THING:			   strcat(str, SZ_EFFECT_SKULL_THING);			   break;
		case M_EFFECT_SKULL_THING_FIRE:		   strcat(str, SZ_EFFECT_SKULL_THING_FIRE);		   break;
		case M_EFFECT_GHOST_SKULL:			   strcat(str, SZ_EFFECT_GHOST_SKULL);			   break;
#endif
		default: strcat(str, SZ_UNKNOW);
		}
		break;
    case T_SHOT:
        sprintf(str, "%s: ", SZ_SHOT);
        switch (t->Thing.Model)
        {
        case M_SHOT_STANDARD:                  strcat(str, SZ_SHOT_STANDARD);                  break;
        case M_SHOT_STANDARD_2:                strcat(str, SZ_SHOT_STANDARD_2);                break;
        case M_SHOT_STANDARD_3:                strcat(str, SZ_SHOT_STANDARD_3);                break;
        case M_SHOT_FIREBALL:                  strcat(str, SZ_SHOT_FIREBALL);                  break;
        case M_SHOT_LIGHTNING:                 strcat(str, SZ_SHOT_LIGHTNING);                 break;
        case M_SHOT_SUPER_WARRIOR:             strcat(str, SZ_SHOT_SUPER_WARRIOR);             break;
        case M_SHOT_VOLCANO_FIREBALL_1:        strcat(str, SZ_SHOT_VOLCANO_FIREBALL_1);        break;
        case M_SHOT_VOLCANO_FIREBALL_2:        strcat(str, SZ_SHOT_VOLCANO_FIREBALL_2);        break;
        default: strcat(str, SZ_UNKNOW);
        }
        break;
	case T_CREATURE:
		switch(t->Thing.Model)
		{
		case M_CREATURE_EAGLE: strcpy(str, SZ_CREATURE_EAGLE); break;
		default: sprintf(str, "%s: %s", SZ_CREATURE, SZ_UNKNOW);
		}
		break;
	case T_VEHICLE:
		switch(t->Thing.Model)
		{
		case M_VEHICLE_BOAT_1:
		case M_VEHICLE_BOAT_2:
			strcpy(str, SZ_VEHICLE_BOAT);
			break;
		case M_VEHICLE_AIRSHIP_1:
		case M_VEHICLE_AIRSHIP_2:
			strcpy(str, SZ_VEHICLE_AIRSHIP);
			break;
		}
		break;
	default: strcpy(str, SZ_UNKNOW);
	}
}


// -=-=- device dialog -=-=-

void DlgDeviceToggle()
{
	if(hDlgDevice)
	{
		DestroyWindow(hDlgDevice);
		hDlgDevice = 0;
	}
	else
	{
		hDlgDevice = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DEVICE), hMainWnd, DlgDeviceProc, 0);
		ShowWindow(hDlgDevice, SW_SHOW);
	}
}


int __stdcall DlgDeviceProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND hItem;
			int i;

			// devices

			i = 0;
			selectedDevice = 0;
			
			hItem = GetDlgItem(hWnd, IDC_DEVICE_DEVICES);

			DEVICE_INFO *di = DeviceInfo;
			if(di) do
			{
				if(memcmp(&di->guid, &guidDevice, sizeof(GUID)) == 0) selectedDevice = di;
				if(!selectedDevice) i++;
				SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)di->szName);
				di = di->Next;
			}
			while(di != DeviceInfo);

			if(selectedDevice)
				SendMessage(hItem, CB_SETCURSEL, i, 0);
			else
				SendMessage(hItem, CB_SETCURSEL, -1, 0);

			// modes

			i = 0;
			selectedMode = 0;

			hItem = GetDlgItem(hWnd, IDC_DEVICE_MODES);

			SCREEN_INFO *si = ScreenInfo;
			if(si) do
			{
				if(si->Width == dwScreenWidth && si->Height == dwScreenHeight && si->Depth == dwScreenDepth) selectedMode = si;
				if(!selectedMode) i++;
				sprintf(str, "%dx%dx%d", si->Width, si->Height, si->Depth);
                SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)str);
				si = si->Next;
			}
			while(si != ScreenInfo);

			if(selectedMode)
				SendMessage(hItem, CB_SETCURSEL, i, 0);
			else
				SendMessage(hItem, CB_SETCURSEL, -1, 0);

			// full screen

			if(fFullScreen)
				CheckDlgButton(hWnd, IDC_DEVICE_FULLSCREEN, BST_CHECKED);
			else
				CheckDlgButton(hWnd, IDC_DEVICE_FULLSCREEN, BST_UNCHECKED);
		}
		break;

	case WM_CLOSE:
		DlgDeviceToggle();
		return 1;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_DEVICE_DEVICES:
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				int i = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
				if(i < 0) break;

				DEVICE_INFO *di = DeviceInfo;
				if(di) do
				{
					if(i == 0)
					{
						selectedDevice = di;
						break;
					}
					i--;
					di = di->Next;
				}
				while(di != DeviceInfo);
			}
			break;

		case IDC_DEVICE_MODES:
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				int i = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
				if(i < 0) break;

				SCREEN_INFO *si = ScreenInfo;
				if(si) do
				{
					if(i == 0)
					{
						selectedMode = si;
						break;
					}
					i--;
					si = si->Next;
				}
				while(si != ScreenInfo);
			}
			break;

		case IDC_DEVICE_APPLY:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				fEngineActiveOld = fEngineActive;

				EngineDestroy();
				if(selectedDevice)
				{
					guidDeviceOld = guidDevice;
					fHwDeviceOld  = fHwDevice;
					strcpy(szDeviceNameOld, szDeviceName);
					guidDevice = selectedDevice->guid;
					fHwDevice  = selectedDevice->fHw;
					strcpy(szDeviceName, selectedDevice->szName);
				}
				if(selectedMode)
				{
					dwScreenWidthOld  = dwScreenWidth;
					dwScreenHeightOld = dwScreenHeight;
					dwScreenDepthOld  = dwScreenDepth;
					dwScreenWidth  = selectedMode->Width;
					dwScreenHeight = selectedMode->Height;
					dwScreenDepth  = selectedMode->Depth;
				}
				fFullScreenOld = fFullScreen;
				fFullScreen = (IsDlgButtonChecked(hWnd, IDC_DEVICE_FULLSCREEN) == BST_CHECKED) ? true : false;
				rs = EngineCreate();

				if(fFullScreen)
					CheckDlgButton(hWnd, IDC_DEVICE_FULLSCREEN, BST_CHECKED);
				else
					CheckDlgButton(hWnd, IDC_DEVICE_FULLSCREEN, BST_UNCHECKED);

				if FAILED(rs)
				{
					EngineDestroy();
					CenterDialog(hWnd);
					if(IDYES == ModalMsg(SZ_ENGINE_FAILED_RESTORE, APPNAME, MB_ICONHAND | MB_YESNO))
						DlgDeviceConfirmToggle();
				}
				else
				{
					DlgDeviceConfirmToggle();
				}
			}
			break;
		}
		break;
	}

	return 0;
}


// -=-=- device confirm dialog -=-=-

void DlgDeviceConfirmToggle()
{
	if(hDlgDeviceConfirm)
	{
		KillTimer(hDlgDeviceConfirm, DLG_DEVICE_CONFIRM_TIMERID);
		DestroyWindow(hDlgDeviceConfirm);
		hDlgDeviceConfirm = 0;
	}
	else
	{
		if(hDlgDevice) DlgDeviceToggle();
		hDlgDeviceConfirm = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DEVICECONFIRM), hMainWnd, DlgDeviceConfirmProc, 0);
		ShowWindow(hDlgDeviceConfirm, SW_SHOW);
	}
}


void DlgDeviceConfirmUpdate(HWND hWnd)
{
	sprintf(str, SZ_DEVICECONFIRM_TIME, iDlgDeviceConfirmTime);
	SendDlgItemMessage(hWnd, IDC_DEVICECONFIRM_TXT, WM_SETTEXT, 0, (LPARAM)str);
}


void DlgDeviceConfirmRestore()
{
	DlgDeviceConfirmToggle();
	EngineDestroy();
	guidDevice = guidDeviceOld;
	fHwDevice  = fHwDeviceOld;
	fFullScreen = fFullScreenOld;
	strcpy(szDeviceName, szDeviceNameOld);
	dwScreenWidth = dwScreenWidthOld;
	dwScreenHeight = dwScreenHeightOld;
	dwScreenDepth = dwScreenDepthOld;
	if(!fEngineActiveOld) return;
	EngineCreate();
}


int __stdcall DlgDeviceConfirmProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		iDlgDeviceConfirmTime = 10;
		SetTimer(hWnd, DLG_DEVICE_CONFIRM_TIMERID, 1000, 0);	
		DlgDeviceConfirmUpdate(hWnd);
		return 0;

	case WM_CLOSE:
		DlgDeviceConfirmRestore();
		if(!hDlgDevice) DlgDeviceToggle();
		return 0;

	case WM_COMMAND:
		if(wParam == IDCANCEL)
		{
			DlgDeviceConfirmRestore();
			if(!hDlgDevice) DlgDeviceToggle();
		}
		else if(wParam == IDOK)
		{
			DlgDeviceConfirmToggle();
			if(!hDlgDevice) DlgDeviceToggle();
		}
		return 0;

	case WM_TIMER:
		if(wParam != DLG_DEVICE_CONFIRM_TIMERID) return 0;
		iDlgDeviceConfirmTime--;
		DlgDeviceConfirmUpdate(hWnd);
		if(iDlgDeviceConfirmTime == 0)
		{
			DlgDeviceConfirmRestore();
			if(!hDlgDevice) DlgDeviceToggle();
		}
		return 0;
	}

	return 0;
}


// -=-=- menu bar -=-=-

long InitializeMenuBar()
{
	WNDCLASS wc;
	wc.cbClsExtra =
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = 0;
	wc.hInstance = hInst;
	wc.lpfnWndProc = MenuBarProc;
	wc.lpszClassName = POPEDT_MENUBAR_CLASS;
	wc.lpszMenuName = 0;
	wc.style = 0;
	return RegisterClass(&wc);
}


void MenuBarToggle()
{
	if(hDlgMenuBar)
	{
		DestroyWindow(hDlgMenuBar);
		hDlgMenuBar = 0;
	}
	else
	{
		POINT pt = {0, 0};
		ClientToScreen(hMainWnd, &pt);

		hDlgMenuBar = CreateWindowEx(WS_EX_TOOLWINDOW, POPEDT_MENUBAR_CLASS, POPEDT_MENUBAR_NAME,
			WS_POPUP | WS_SYSMENU | WS_CAPTION,
			pt.x, pt.y,
			POPEDT_MENUBAR_WIDTH, POPEDT_MENUBAR_HEIGHT,
			hMainWnd, 0, hInst, 0);

		ShowWindow(hDlgMenuBar, SW_SHOW);
		UpdateWindow(hDlgMenuBar);
	}
}


long __stdcall MenuBarProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		{
			RECT rc;
			GetClientRect(hWnd, &rc);

			HWND hToolBar = CreateWindowEx(0, TOOLBARCLASSNAME, 0, WS_CHILD | CCS_NORESIZE | CCS_NOPARENTALIGN | TBSTYLE_FLAT | TBSTYLE_LIST,
				0, 0, rc.right, rc.bottom, hWnd, (HMENU) IDD_MENUBAR_TOOLBAR, hInst, 0);

			SendMessage(hToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);

			TBBUTTON tbb[3];
			int i;

			i = SendMessage(hToolBar, TB_ADDSTRING, 0, (LPARAM)SZ_MENU_FILE);
			tbb[0].dwData = 0;
			tbb[0].fsState = TBSTATE_ENABLED;
			tbb[0].fsStyle = TBSTYLE_BUTTON | BTNS_DROPDOWN | TBSTYLE_AUTOSIZE;
			tbb[0].iBitmap = I_IMAGENONE;
			tbb[0].idCommand = IDC_MENUBAR_FILE;
			tbb[0].iString = i;

			i = SendMessage(hToolBar, TB_ADDSTRING, 0, (LPARAM)SZ_MENU_EDIT);
			tbb[1].dwData = 0;
			tbb[1].fsState = TBSTATE_ENABLED;
			tbb[1].fsStyle = TBSTYLE_BUTTON | BTNS_DROPDOWN | TBSTYLE_AUTOSIZE;
			tbb[1].iBitmap = I_IMAGENONE;
			tbb[1].idCommand = IDC_MENUBAR_EDIT;
			tbb[1].iString = i;

			i = SendMessage(hToolBar, TB_ADDSTRING, 0, (LPARAM)SZ_MENU_OPTIONS);
			tbb[2].dwData = 0;
			tbb[2].fsState = TBSTATE_ENABLED;
			tbb[2].fsStyle = TBSTYLE_BUTTON | BTNS_DROPDOWN | TBSTYLE_AUTOSIZE;
			tbb[2].iBitmap = I_IMAGENONE;
			tbb[2].idCommand = IDC_MENUBAR_OPTIONS;
			tbb[2].iString = i;

			SendMessage(hToolBar, TB_ADDBUTTONS, (WPARAM)3, (LPARAM) &tbb);
			SendMessage(hToolBar, TB_AUTOSIZE, 0, 0);

			ShowWindow(hToolBar, SW_SHOW);
		}
		return 0;

	case WM_CLOSE:
		MenuBarToggle();
		return 0;

	case WM_NOTIFY:
		switch(wParam)
		{
		case IDD_MENUBAR_TOOLBAR:
			if((*(NMTOOLBAR*)lParam).hdr.code == TBN_DROPDOWN)
			{
				HWND hToolBar = (*(NMTOOLBAR*)lParam).hdr.hwndFrom;

				long state = SendMessage(hToolBar, TB_GETSTATE, (*(NMTOOLBAR*)lParam).iItem, 0);				
				SendMessage(hToolBar, TB_SETSTATE, (*(NMTOOLBAR*)lParam).iItem, state | TBSTATE_PRESSED);

				RECT rc;
				SendMessage(hToolBar, TB_GETRECT, (*(NMTOOLBAR*)lParam).iItem, (LPARAM)&rc);

				POINT pt;
				pt.x = rc.left;
				pt.y = rc.bottom;

				ClientToScreen(hToolBar, &pt);

				HMENU hMenu = 0;

				switch((*(NMTOOLBAR*)lParam).iItem)
				{
				case IDC_MENUBAR_FILE:
					hMenu = LoadMenu(hInst, MAKEINTRESOURCE(ID_FILE));
					break;

				case IDC_MENUBAR_EDIT:
					hMenu = LoadMenu(hInst, MAKEINTRESOURCE(ID_EDIT));
					if(fEngineEditLand) CheckMenuItem(hMenu, ID_EDIT_LANDSCAPE, MF_BYCOMMAND | MF_CHECKED);
					if(fEngineEditObjs) CheckMenuItem(hMenu, ID_EDIT_OBJECTS, MF_BYCOMMAND | MF_CHECKED);
					if(fEngineEditMarkers) CheckMenuItem(hMenu, ID_EDIT_MARKERS, MF_BYCOMMAND | MF_CHECKED);
					break;

				case IDC_MENUBAR_OPTIONS:
					hMenu = LoadMenu(hInst, MAKEINTRESOURCE(ID_OPTIONS));
					if(dwEngineFlags & EF_SHOW_GRID) CheckMenuItem(hMenu, ID_OPTIONS_SHOWGRID, MF_BYCOMMAND | MF_CHECKED);
					if(dwEngineFlags & EF_GRID_SEE_THROUGH) CheckMenuItem(hMenu, ID_OPTIONS_GRIDSEETHROUGH, MF_BYCOMMAND | MF_CHECKED);
					if(dwEngineFlags & EF_WIREFRAME_LAND) CheckMenuItem(hMenu, ID_OPTIONS_WIREFRAME_LAND, MF_BYCOMMAND | MF_CHECKED);
					if(dwEngineFlags & EF_WIREFRAME_OBJS) CheckMenuItem(hMenu, ID_OPTIONS_WIREFRAME_OBJS, MF_BYCOMMAND | MF_CHECKED);
					if(dwEngineFlags & EF_DITHER) CheckMenuItem(hMenu, ID_OPTIONS_DITHER, MF_BYCOMMAND | MF_CHECKED);
					if(dwEngineFlags & EF_TEXTURE_FILTER) CheckMenuItem(hMenu, ID_OPTIONS_TEXTURE_FILTER, MF_BYCOMMAND | MF_CHECKED);
					if(dwEngineFlags & EF_FRAMERATE) CheckMenuItem(hMenu, ID_OPTIONS_FRAMERATE, MF_BYCOMMAND | MF_CHECKED);
					if(dwEngineFlags & EF_SHOW_MINIMAP) CheckMenuItem(hMenu, ID_OPTIONS_MINIMAP_SHOW, MF_BYCOMMAND | MF_CHECKED);
					if(dwEngineFlags & EF_TRANSPARENT_MINIMAP) CheckMenuItem(hMenu, ID_OPTIONS_MINIMAP_TRANSPARENT, MF_BYCOMMAND | MF_CHECKED);
					if(dwEngineFlags & EF_HIDE_OBJS_ON_MINIMAP) CheckMenuItem(hMenu, ID_OPTIONS_MINIMAP_HIDEOBJECTS, MF_BYCOMMAND | MF_CHECKED);
					if(dwEngineFlags & EF_HIDE_OBJECTS) CheckMenuItem(hMenu, ID_OPTIONS_HIDEOBJECTS, MF_BYCOMMAND | MF_CHECKED);
					if(dwEngineFlags & EF_BOUNDINGBOX) CheckMenuItem(hMenu, ID_OPTIONS_BOUNDINGBOX, MF_BYCOMMAND | MF_CHECKED);
					if(dwEngineFlags & EF_SHOW_MARKERS) CheckMenuItem(hMenu, ID_OPTIONS_SHOW_MARKERS, MF_BYCOMMAND | MF_CHECKED);
					if(dwEngineFlags & EF_CENTERTHINGS) CheckMenuItem(hMenu, ID_OPTIONS_CENTERTHINGS, MF_BYCOMMAND | MF_CHECKED);
					break;
				}

				if(!hMenu) return 0;

				/*
				MENUINFO mi;
				memset(&mi, 0, sizeof(mi));
				mi.cbSize = sizeof(mi);
				mi.fMask = MIM_STYLE;
				GetMenuInfo(hMenu, &mi);
				mi.fMask |= MIM_APPLYTOSUBMENUS;
				mi.dwStyle |= MNS_MODELESS;
				SetMenuInfo(hMenu, &mi);
				*/

				TrackPopupMenu(GetSubMenu(hMenu, 0), TPM_VERPOSANIMATION, pt.x, pt.y, 0, hWnd, 0);

				DestroyMenu(hMenu);

				SendMessage(hToolBar, TB_SETSTATE, (*(NMTOOLBAR*)lParam).iItem, state);
			}
			break;
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_FILE_LEVEL_NEW:
			LevelNew();
			break;

		case ID_FILE_LEVEL_OPEN:
			LevelOpen();
			break;

		case ID_FILE_LEVEL_SAVE:
			LevelSave();
			break;

		case ID_FILE_LEVEL_SAVE_AS:
			LevelSaveAs();
			break;

		case ID_FILE_EXPORTHEIGHTMAP:
			ExportHeightMap();
			break;

		case ID_IMPORT_MAP_PRESET:
			ImportMapPreset();
			break;

		case ID_FILE_HOST:
			net.InitNetwork(true);
			break;

		case ID_FILE_CLIENT:
			net.InitNetwork(false);
			break;

		case ID_FILE_SYNC_MAP:
			net.OnSyncMapAsk();
			break;

		case ID_FILE_EXIT:
			PostMessage(hMainWnd, WM_CLOSE, 0, 0);
			break;

		case ID_EDIT_LANDSCAPE:
			SetEditLand();
			break;

		case ID_EDIT_OBJECTS:
			SetEditObjects();
			break;

		case ID_EDIT_MARKERS:
			SetEditMarkers();
			break;

		case ID_EDIT_SWAPTRIBE:
			DlgSwapTribeToggle();
			break;

		case ID_PURGE_OBJECTS:
			{
				const int nResult = MessageBox(NULL, _T("Are you sure you want to delete ALL objects?"), _T("Delete Objects"), MB_YESNO);
				if (nResult == IDYES)
				{
					if (net.IsInitialized())
					{
						struct Packet* p = new Packet;
						p->wType = PACKETTYPE_DELETE_ALL_OBJECTS;
						net.SendPacket(p);
						p->del();
					}
					PurgeObjects();
				}
			}
			break;

		case ID_DELETE_DUPLICATE_TREES:
			DeleteDuplicateTrees();
		break;

		case ID_DELETE_WILDS:
			DeleteWilds();
		break;

		case ID_ROTATE_BUILDINGS:
			RotateBuildings();
			break;

		case ID_ROTATE_TREES:
			RotateTrees();
			break;

		case ID_NEUTRALIZER:
			Neutralizer();
			break;

		case ID_LEVL3_TO_LEVL2:
			Levl3ToLevl2();
			break;

		case ID_RESET_MARKERS:
			{
				int nResult = MessageBox(NULL, _T("Are you sure you want to reset ALL markers?"), _T("Reset Markers"), MB_YESNO);
				if (nResult == IDYES)
				{
					if (net.IsInitialized())
					{
						struct Packet* p = new Packet;
						p->wType = PACKETTYPE_DELETE_ALL_MARKERS;
						net.SendPacket(p);
						p->del();
					}
					ResetMarkers();
				}
			}
			break;

		case ID_HEADER_ALLIES:
			DlgAlliesToggle();
			break;

		case ID_HEADER_AVAILABLE_SPELLS_BUILDINGS:
			DlgSpellsBuildingsToggle();
			break;

		case ID_HEADER_SCRIPT2:
			DlgScript2();
			break;

		case ID_HEADER_SPELLSNOTCHARGING:
			DlgSpellsNotChargingToggle();
			break;

		case ID_HEADER_GENERAL:
			DlgHeaderToggle();
			break;

		case ID_HEADER_OBJECTBANK:
			DlgObjBankToggle();
			break;

		case ID_HEADER_MAPTEXTURE:
			DlgMapTypeToggle();
			break;

		case ID_AISCRIPT_ATTRIBUTES:
			DlgAIAttribToggle();
			break;

		case ID_AISCRIPT_SCRIPT:
			DlgAIScriptToggle();
			break;

		case ID_LANGUAGE_LANG2TEXT:
			Lang2Text();
			break;

		case ID_LANGUAGE_TEXT2LANG:
			Text2Lang();
			break;

		case ID_OPTIONS_SCREENMODE:
			DlgDeviceToggle();
			break;

		case ID_OPTIONS_BRUSH:
			DlgBrushToggle();
			break;

		case ID_OPTIONS_FLAGS_BRUSH:
			DlgFlagsBrushToggle();
			break;

		case ID_OPTIONS_OBJECTLIST:
			DlgListToggle();
			break;

		case ID_OPTIONS_SHOWGRID:
			if(dwEngineFlags & EF_SHOW_GRID)
				dwEngineFlags &= ~EF_SHOW_GRID;
			else
			{
				dwEngineFlags |= EF_SHOW_GRID;
				dwEngineFlags &= ~EF_WIREFRAME_LAND;
			}
			break;

		case ID_OPTIONS_GRIDSEETHROUGH:
			if(dwEngineFlags & EF_GRID_SEE_THROUGH)
				dwEngineFlags &= ~EF_GRID_SEE_THROUGH;
			else
				dwEngineFlags |= EF_GRID_SEE_THROUGH;
			break;

		case ID_OPTIONS_WIREFRAME_LAND:
			if(dwEngineFlags & EF_WIREFRAME_LAND)
				dwEngineFlags &= ~EF_WIREFRAME_LAND;
			else
			{
				dwEngineFlags |= EF_WIREFRAME_LAND;
				dwEngineFlags &= ~EF_SHOW_GRID;
			}
			break;

		case ID_OPTIONS_WIREFRAME_OBJS:
			if(dwEngineFlags & EF_WIREFRAME_OBJS)
				dwEngineFlags &= ~EF_WIREFRAME_OBJS;
			else
				dwEngineFlags |= EF_WIREFRAME_OBJS;
			break;

		case ID_OPTIONS_DITHER:
			if(dwEngineFlags & EF_DITHER)
			{
				dwEngineFlags &= ~EF_DITHER;
				if(fEngineActive) lpD3DDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, false);
			}
			else
			{
				dwEngineFlags |= EF_DITHER;
				if(fEngineActive) lpD3DDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, true);
			}
			break;

		case ID_OPTIONS_TEXTURE_FILTER:
			if(dwEngineFlags & EF_TEXTURE_FILTER)
			{
				dwEngineFlags &= ~EF_TEXTURE_FILTER;
				if(fEngineActive)
				{
					lpD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
					lpD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_POINT);
				}
			}
			else
			{
				dwEngineFlags |= EF_TEXTURE_FILTER;
				if(fEngineActive)
				{
					lpD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
					lpD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
				}
			}
			break;

		case ID_OPTIONS_FRAMERATE:
			if(dwEngineFlags & EF_FRAMERATE)
				dwEngineFlags &= ~EF_FRAMERATE;
			else
				dwEngineFlags |= EF_FRAMERATE;
			break;

		case ID_OPTIONS_MINIMAP_SHOW:
			if(dwEngineFlags & EF_SHOW_MINIMAP)
				dwEngineFlags &= ~EF_SHOW_MINIMAP;
			else
				dwEngineFlags |= EF_SHOW_MINIMAP;
			break;

		case ID_OPTIONS_MINIMAP_TRANSPARENT:
			if(dwEngineFlags & EF_TRANSPARENT_MINIMAP)
				dwEngineFlags &= ~EF_TRANSPARENT_MINIMAP;
			else
				dwEngineFlags |= EF_TRANSPARENT_MINIMAP;
			break;

		case ID_OPTIONS_MINIMAP_HIDEOBJECTS:
			if(dwEngineFlags & EF_HIDE_OBJS_ON_MINIMAP)
				dwEngineFlags &= ~EF_HIDE_OBJS_ON_MINIMAP;
			else
				dwEngineFlags |= EF_HIDE_OBJS_ON_MINIMAP;
			EngineUpdateMiniMap();
			break;

		case ID_OPTIONS_HIDEOBJECTS:
			if(dwEngineFlags & EF_HIDE_OBJECTS)
				dwEngineFlags &= ~EF_HIDE_OBJECTS;
			else
				dwEngineFlags |= EF_HIDE_OBJECTS;
			break;

		case ID_OPTIONS_BOUNDINGBOX:
			if(dwEngineFlags & EF_BOUNDINGBOX)
				dwEngineFlags &= ~EF_BOUNDINGBOX;
			else
				dwEngineFlags |= EF_BOUNDINGBOX;
			break;

		case ID_OPTIONS_SHOW_MARKERS:
			if(dwEngineFlags & EF_SHOW_MARKERS)
				dwEngineFlags &= ~EF_SHOW_MARKERS;
			else
				dwEngineFlags |= EF_SHOW_MARKERS;
			break;

		case ID_OPTIONS_CENTERTHINGS:
			if (dwEngineFlags & EF_CENTERTHINGS)
				dwEngineFlags &= ~EF_CENTERTHINGS;
			else
				dwEngineFlags |= EF_CENTERTHINGS;
			break;

		case ID_OPTIONS_INFO:
			DlgInfoToggle();
			break;

		case ID_OPTIONS_HELP:
			ShowHelp();
			break;

		case ID_OPTIONS_ABOUT:
			DlgAboutToggle();
			break;

		case ID_MACRO:
			DlgMacroToggle();
			break;
		}
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


void SetEditLand()
{
	if(fEngineEditLand)
	{
		fEngineEditLand = false;
		memset(wLandPreset, 0, sizeof(wLandPreset));
		EngineUpdateView();
	}
	else
	{
		fEngineEditObjs    = false;
		fEngineEditLand    = true;
		fEngineEditMarkers = false;
		if(hDlgObject) DlgObjectToggle();
		if(hDlgMarkers) DlgMarkersToggle();
		if (!hDlgBrush) DlgBrushToggle();
		if (hMainWnd) SetFocus(hMainWnd);
	}
}


void SetEditObjects()
{
	if(fEngineEditObjs)
	{
		fEngineEditObjs = false;
		if(hDlgObject) DlgObjectToggle();
	}
	else
	{
		memset(wLandPreset, 0, sizeof(wLandPreset));
		fEngineEditLand    = false;
		fEngineEditObjs    = true;
		fEngineEditMarkers = false;
		if(hDlgMarkers) DlgMarkersToggle();
		if(!hDlgObject) DlgObjectToggle();
		EngineUpdateView();
	}
}


void SetEditMarkers()
{
	if(fEngineEditMarkers)
	{
		fEngineEditMarkers = false;
		if(hDlgMarkers) DlgMarkersToggle();
	}
	else
	{
		memset(wLandPreset, 0, sizeof(wLandPreset));
		fEngineEditLand    = false;
		fEngineEditObjs    = false;
		fEngineEditMarkers = true;
		if(hDlgObject) DlgObjectToggle();
		if(!hDlgMarkers) DlgMarkersToggle();
		EngineUpdateView();
	}
}


// -=-=- brush dialog -=-=-

void DlgBrushToggle()
{
	if(hDlgBrush)
	{
		DestroyWindow(hDlgBrush);
		hDlgBrush = 0;
	}
	else
	{
		hDlgBrush = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_BRUSH), hMainWnd, DlgBrushProc, 0);
		ShowWindow(hDlgBrush, SW_SHOW);
	}
}


void DlgBrushUpdate(HWND hWnd)
{
	if(!hWnd) return;

	//

	SendMessage(GetDlgItem(hWnd, IDC_BRUSH_SIZE), TBM_SETPOS, true, GroundEditBrushSize);
	sprintf(str, SZ_BRUSH_SIZE_TXT, GroundEditBrushSize);
	SetDlgItemText(hWnd, IDC_BRUSH_SIZE_TXT, str);

	SendMessage(GetDlgItem(hWnd, IDC_BRUSH_SPEED), TBM_SETPOS, true, GroundEditBrushSpeed);
	sprintf(str, SZ_BRUSH_SPEED_TXT, GroundEditBrushSpeed);
	SetDlgItemText(hWnd, IDC_BRUSH_SPEED_TXT, str);
	
	if(fFlatten)
		CheckDlgButton(hWnd, IDC_BRUSH_FLATTEN, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BRUSH_FLATTEN, BST_UNCHECKED);

	if(fSmooth)
		CheckDlgButton(hWnd, IDC_BRUSH_SMOOTH, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BRUSH_SMOOTH, BST_UNCHECKED);

	if (fRaise)
		CheckDlgButton(hWnd, IDC_BRUSH_ONLY_RAISE, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BRUSH_ONLY_RAISE, BST_UNCHECKED);

	if (fLower)
		CheckDlgButton(hWnd, IDC_BRUSH_ONLY_LOWER, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BRUSH_ONLY_LOWER, BST_UNCHECKED);

	if (fBrushMaxSize)
		CheckDlgButton(hWnd, IDC_BRUSH_MAX_SIZE, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BRUSH_MAX_SIZE, BST_UNCHECKED);

	if (fCopyObjects)
		CheckDlgButton(hWnd, IDC_BRUSH_COPY_OBJECTS, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BRUSH_COPY_OBJECTS, BST_UNCHECKED);
}


int __stdcall DlgBrushProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND Item;
			//
			Item = GetDlgItem(hWnd, IDC_BRUSH_SIZE);
			SendMessage(Item, TBM_SETRANGE, true, MAKELONG(BRUSH_SIZE_MIN, BRUSH_SIZE_MAX));
			SendMessage(Item, TBM_SETPAGESIZE, 0, 1);
			//
			Item = GetDlgItem(hWnd, IDC_BRUSH_SPEED);
			SendMessage(Item, TBM_SETRANGE, true, MAKELONG(BRUSH_SPEED_MIN, BRUSH_SPEED_MAX));
			SendMessage(Item, TBM_SETPAGESIZE, 0, 1);
			//
			DlgBrushUpdate(hWnd);
		}
		return 0;

	case WM_COMMAND:		
		if(wParam == IDC_BRUSH_FLATTEN)
		{
			if(IsDlgButtonChecked(hWnd, IDC_BRUSH_FLATTEN) == BST_CHECKED)
			{
				fFlatten = true;
				fSmooth = false;
				fRaise = false;
				fLower = false;
				CheckDlgButton(hWnd, IDC_BRUSH_SMOOTH, BST_UNCHECKED);
				CheckDlgButton(hWnd, IDC_BRUSH_ONLY_RAISE, BST_UNCHECKED);
				CheckDlgButton(hWnd, IDC_BRUSH_ONLY_LOWER, BST_UNCHECKED);
			}
			else
			{
				fFlatten = false;
			}
		}
		else if(wParam == IDC_BRUSH_SMOOTH)
		{
			if(IsDlgButtonChecked(hWnd, IDC_BRUSH_SMOOTH) == BST_CHECKED)
			{
				fSmooth = true;
                fFlatten = false;
				fRaise = false;
				fLower = false;
				CheckDlgButton(hWnd, IDC_BRUSH_FLATTEN, BST_UNCHECKED);
				CheckDlgButton(hWnd, IDC_BRUSH_ONLY_RAISE, BST_UNCHECKED);
				CheckDlgButton(hWnd, IDC_BRUSH_ONLY_LOWER, BST_UNCHECKED);
			}
			else
			{
				fSmooth = false;
			}
		}
		else if (wParam == IDC_BRUSH_ONLY_RAISE)
		{
			if (IsDlgButtonChecked(hWnd, IDC_BRUSH_ONLY_RAISE) == BST_CHECKED)
			{
				fRaise = true;
				fSmooth = false;
				fFlatten = false;
				CheckDlgButton(hWnd, IDC_BRUSH_FLATTEN, BST_UNCHECKED);
				CheckDlgButton(hWnd, IDC_BRUSH_SMOOTH, BST_UNCHECKED);
			}
			else
			{
				fRaise = false;
			}
		}
		else if (wParam == IDC_BRUSH_ONLY_LOWER)
		{
			if (IsDlgButtonChecked(hWnd, IDC_BRUSH_ONLY_LOWER) == BST_CHECKED)
			{
				fLower = true;
				fSmooth = false;
				fFlatten = false;
				CheckDlgButton(hWnd, IDC_BRUSH_FLATTEN, BST_UNCHECKED);
				CheckDlgButton(hWnd, IDC_BRUSH_SMOOTH, BST_UNCHECKED);
			}
			else
			{
				fLower = false;
			}
		}
		else if (wParam == IDC_BRUSH_MAX_SIZE)
		{
			if (IsDlgButtonChecked(hWnd, IDC_BRUSH_MAX_SIZE) == BST_CHECKED)
			{
				fBrushMaxSize = true;
				PreviousBrushSize = GroundEditBrushSize;
				GroundEditBrushSize = 128;
			}
			else
			{
				fBrushMaxSize = false;
				GroundEditBrushSize = PreviousBrushSize;
			}

			sprintf(str, SZ_BRUSH_SIZE_TXT, GroundEditBrushSize);
			SendDlgItemMessage(hWnd, IDC_BRUSH_SIZE_TXT, WM_SETTEXT, 0, (LPARAM)str);
		}
		else if (wParam == IDC_BRUSH_COPY_OBJECTS)
		{
			if (IsDlgButtonChecked(hWnd, IDC_BRUSH_COPY_OBJECTS) == BST_CHECKED)
			{
				fCopyObjects = true;
			}
			else
			{
				fCopyObjects = false;
			}
		}
		return 0;

	case WM_HSCROLL:
		if(GetDlgCtrlID((HWND)lParam) == IDC_BRUSH_SIZE)
		{
			if (fBrushMaxSize)
				return 0;

			GroundEditBrushSize = (int) SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);

			if(GroundEditBrushSize < BRUSH_SIZE_MIN) GroundEditBrushSize = BRUSH_SIZE_MIN;
			if(GroundEditBrushSize > BRUSH_SIZE_MAX) GroundEditBrushSize = BRUSH_SIZE_MAX;

			sprintf(str, SZ_BRUSH_SIZE_TXT, GroundEditBrushSize);
			SendDlgItemMessage(hWnd, IDC_BRUSH_SIZE_TXT, WM_SETTEXT, 0, (LPARAM)str);
		}
		else if(GetDlgCtrlID((HWND)lParam) == IDC_BRUSH_SPEED)
		{
			GroundEditBrushSpeed = (int) SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);

			if(GroundEditBrushSpeed < BRUSH_SPEED_MIN) GroundEditBrushSpeed = BRUSH_SPEED_MIN;
			if(GroundEditBrushSpeed > BRUSH_SPEED_MAX) GroundEditBrushSpeed = BRUSH_SPEED_MAX;

			sprintf(str, SZ_BRUSH_SPEED_TXT, GroundEditBrushSpeed);
			SendDlgItemMessage(hWnd, IDC_BRUSH_SPEED_TXT, WM_SETTEXT, 0, (LPARAM)str);
		}
		return 0;

	case WM_CLOSE:
		DlgBrushToggle();
		return 0;
	}

	return 0;
}

// -=-=- flags brush dialog -=-=-

void DlgFlagsBrushToggle()
{
#if _ASCENSION
	if (hDlgFlagsBrush)
	{
		DestroyWindow(hDlgFlagsBrush);
		hDlgFlagsBrush = 0;
	}
	else
	{
		hDlgFlagsBrush = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_FLAGS_BRUSH), hMainWnd, DlgFlagsBrushProc, 0);
		ShowWindow(hDlgFlagsBrush, SW_SHOW);
	}
#endif
}


void DlgFlagsBrushUpdate(HWND hWnd)
{
	if (!hWnd) return;

	SendMessage(GetDlgItem(hWnd, IDC_FLAG_BRUSH_VALUE), TBM_SETPOS, true, nFlagPaintValue);
	sprintf(str, SZ_FLAGS_BRUSH_VALUE_TXT, nFlagPaintValue);
	SetDlgItemText(hWnd, IDC_FLAG_BRUSH_VALUE_TXT, str);

	if (fFlagPaint)
		CheckDlgButton(hWnd, IDC_FLAG_PAINT, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_FLAG_PAINT, BST_UNCHECKED);
}


int __stdcall DlgFlagsBrushProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		HWND Item;
		Item = GetDlgItem(hWnd, IDC_FLAG_BRUSH_VALUE);
		SendMessage(Item, TBM_SETRANGE, true, MAKELONG(0, 1024));
		SendMessage(Item, TBM_SETPAGESIZE, 0, 1);
		DlgFlagsBrushUpdate(hWnd);
	}
	return 0;

	case WM_COMMAND:
		if (wParam == IDC_FLAG_PAINT)
		{
			if (IsDlgButtonChecked(hWnd, IDC_FLAG_PAINT) == BST_CHECKED)
			{
				fFlagPaint = true;
			}
			else
			{
				fFlagPaint = false;
			}
		}
		return 0;

	case WM_HSCROLL:
		if (GetDlgCtrlID((HWND)lParam) == IDC_FLAG_BRUSH_VALUE)
		{
			nFlagPaintValue = (int)SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);

			if (nFlagPaintValue < 0) nFlagPaintValue = 0;
			if (nFlagPaintValue > 1024) nFlagPaintValue = 1024;

			sprintf(str, SZ_FLAGS_BRUSH_VALUE_TXT, nFlagPaintValue);
			SendDlgItemMessage(hWnd, IDC_FLAG_BRUSH_VALUE_TXT, WM_SETTEXT, 0, (LPARAM)str);
		}
		return 0;

	case WM_CLOSE:
		DlgFlagsBrushToggle();
		return 0;
	}

	return 0;
}

// -=-=- object dialog -=-=-

void DlgObjectToggle()
{
	if(hDlgObject)
	{
		DestroyWindow(hDlgObject);
		hDlgObject = 0;
		fEngineEditObjs = false;
		if(hDlgDiscovery) DlgDiscoveryToggle();
		if(hDlgTrigger) DlgTriggerToggle();
		if(hDlgLink) DlgLinkToggle();
		DlgObjectSelect(0);
	}
	else
	{
		ThingSelected = 0;
		hDlgObject = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_OBJECT), hMainWnd, DlgObjectProc, 0);
		ShowWindow(hDlgObject, SW_SHOW);
	}
}


int __stdcall DlgObjectProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND hItem;

			hItem = GetDlgItem(hWnd, IDC_OBJECT_OWNER);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OWNER_NEUTRAL);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OWNER_BLUE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OWNER_RED);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OWNER_YELLOW);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OWNER_GREEN);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OWNER_CYAN);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OWNER_MAGENTA);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OWNER_BLACK);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OWNER_ORANGE);
      SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OWNER_HOSTBOT);

			hItem = GetDlgItem(hWnd, IDC_OBJECT_TYPE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_PERSON);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SCENERY);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_GENERAL);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_CREATURE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_VEHICLE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPECIAL);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT);
            SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SHOT);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION);
			DlgObjectUpdateInfo(hWnd, false);
		}
		return 0;

	case WM_COMMAND:
		switch(wParam)
		{
		case IDC_OBJECT_NEXT:
			DlgObjectNextObj();
			return 0;

		case IDC_OBJECT_PREV:
			DlgObjectPrevObj();
			return 0;

		case IDC_OBJECT_FILTER:
			DlgFilterToggle();
			return 0;

		case IDC_OBJECT_NEW:
			DlgObjectNewObj();
			return 0;

		case IDC_OBJECT_DELETE:
			DlgObjectDeleteObj();
			return 0;

		case IDC_PAINT_DECORATIONS:
			DlgPaintDecorations();
			return 0;

		case IDC_OBJECT_MORE:
			DlgObjectMoreToggle();
			return 0;
		}

		if(!ThingSelected) return 0;

		switch(LOWORD(wParam))
		{
		case IDC_OBJECT_OPTIONS:
			if(HIWORD(wParam) == BN_CLICKED)
			{

				if(ThingSelected->Thing.Type == T_BUILDING)
				{
					SaveThingsUndoState();
					switch(ThingSelected->Thing.Building.Angle)
					{
					case ANGLE_0:   ThingSelected->Thing.Building.Angle = ANGLE_90;  break;
					case ANGLE_90:  ThingSelected->Thing.Building.Angle = ANGLE_180; break;
					case ANGLE_180: ThingSelected->Thing.Building.Angle = ANGLE_270; break;
					default: ThingSelected->Thing.Building.Angle = ANGLE_0;
					}
				}
				else if(ThingSelected->Thing.Type == T_SCENERY)
				{
					SaveThingsUndoState();
					switch(ThingSelected->Thing.Scenery.Angle)
					{
					case ANGLE_0:   ThingSelected->Thing.Scenery.Angle = ANGLE_45;  break;
					case ANGLE_45:  ThingSelected->Thing.Scenery.Angle = ANGLE_90;  break;
					case ANGLE_90:  ThingSelected->Thing.Scenery.Angle = ANGLE_135; break;
					case ANGLE_135: ThingSelected->Thing.Scenery.Angle = ANGLE_180; break;
					case ANGLE_180: ThingSelected->Thing.Scenery.Angle = ANGLE_225; break;
					case ANGLE_225: ThingSelected->Thing.Scenery.Angle = ANGLE_270; break;
					case ANGLE_270: ThingSelected->Thing.Scenery.Angle = ANGLE_315; break;
					default: ThingSelected->Thing.Scenery.Angle = ANGLE_0;
					}
				}
				else if(ThingSelected->Thing.Type == T_GENERAL)
				{
					if(ThingSelected->Thing.Model == M_GENERAL_DISCOVERY) DlgDiscoveryToggle();
					else if(ThingSelected->Thing.Model == M_GENERAL_TRIGGER) DlgTriggerToggle();
				}
				else if(ThingSelected->Thing.Type == T_EFFECT && ThingSelected->Thing.Model == M_EFFECT_LAND_BRIDGE)
				{
					SaveThingsUndoState();
					HWND hItem = GetDlgItem(hWnd, IDC_OBJECT_OPTIONS);

					if(ThingSelected->flags & TF_EDIT_LANDBRIDGE)
					{
						ThingSelected->flags &= ~TF_EDIT_LANDBRIDGE;
						SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)SZ_LANDBRIDGE_END);

						fEnginePosX = ThingSelected->x;
						fEnginePosZ = ThingSelected->z;
						EngineUpdateView();
					}
					else
					{
						ThingSelected->flags |= TF_EDIT_LANDBRIDGE;
						SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)SZ_LANDBRIDGE_START);

						fEnginePosX = ThingSelected->LandBridge.x;
						fEnginePosZ = ThingSelected->LandBridge.z;
						EngineUpdateView();
					}

					EnableWindow(hItem, true);
				}

				if (net.IsInitialized())
				{
					if (ThingSelected->Thing.Type == T_BUILDING || ThingSelected->Thing.Type == T_SCENERY)
					{
						struct Packet *p = new Packet;
						p->wType = PACKETTYPE_ROTATE_OBJECT;
						p->wData[0] = ThingSelected->Idx;
						p->wData[1] = ThingSelected->Thing.Type;

						if (ThingSelected->Thing.Type == T_BUILDING)
							p->wData[2] = ThingSelected->Thing.Building.Angle;
						else
							p->wData[2] = ThingSelected->Thing.Scenery.Angle;

						net.SendPacket(p);
						p->del();
					}
				}
			}
			return 0;

		case IDC_OBJECT_OWNER:
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				SaveThingsUndoState();
				switch(SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))
				{
				case IDX_OWNER_NEUTRAL:
					if((ThingSelected->Thing.Type == T_BUILDING) && (ThingSelected->Thing.Model == M_BUILDING_PRISON))
						ThingSelected->Thing.Owner = -2;
					else
						ThingSelected->Thing.Owner = OWNER_NEUTRAL;
					break;
				case IDX_OWNER_BLUE:    ThingSelected->Thing.Owner = OWNER_BLUE;    break;
				case IDX_OWNER_RED:     ThingSelected->Thing.Owner = OWNER_RED;     break;
				case IDX_OWNER_YELLOW:  ThingSelected->Thing.Owner = OWNER_YELLOW;  break;
				case IDX_OWNER_GREEN:   ThingSelected->Thing.Owner = OWNER_GREEN;   break;
        case IDX_OWNER_CYAN:    ThingSelected->Thing.Owner = OWNER_CYAN;    break;
				case IDX_OWNER_MAGENTA:     ThingSelected->Thing.Owner = OWNER_MAGENTA;     break;
				case IDX_OWNER_BLACK:  ThingSelected->Thing.Owner = OWNER_BLACK;  break;
				case IDX_OWNER_ORANGE:   ThingSelected->Thing.Owner = OWNER_ORANGE;   break;
        case IDX_OWNER_HOSTBOT: ThingSelected->Thing.Owner = OWNER_HOSTBOT; break;
        default:
            ThingSelected->Thing.Owner = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
				}

				if (net.IsInitialized())
				{
					struct Packet *p = new Packet;
					p->wType = PACKETTYPE_OWNER_OBJECT;
					p->wData[0] = ThingSelected->Idx;
					p->wData[1] = ThingSelected->Thing.Owner;
					net.SendPacket(p);
					p->del();
				}

				DlgObjectUpdateInfo(hWnd);
				DlgInfoUpdate(hDlgInfo);
			}
			return 0;

		case IDC_OBJECT_TYPE:
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				SaveThingsUndoState();
				ThingSelected->Thing.Model = 1;
				switch(SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))
				{
				case IDX_T_PERSON:
					ThingSelected->Thing.Type = T_PERSON;
					break;
				case IDX_T_BUILDING:
					ThingSelected->Thing.Type = T_BUILDING;
					break;
				case IDX_T_SCENERY:
					ThingSelected->Thing.Type = T_SCENERY;
					break;
				case IDX_T_GENERAL:
					ThingSelected->Thing.Type = T_GENERAL;
					break;
				case IDX_T_SPELL:
					ThingSelected->Thing.Type = T_SPELL;
					break;
				case IDX_T_CREATURE:
					ThingSelected->Thing.Type = T_CREATURE;
					break;
				case IDX_T_VEHICLE:
					ThingSelected->Thing.Type = T_VEHICLE;
					break;
				case IDX_T_SPECIAL:
					ThingSelected->Thing.Type = T_SCENERY;
					ThingSelected->Thing.Model = M_SCENERY_TOP_LEVEL_SCENERY;
					break;
				case IDX_T_EFFECT:
					ThingSelected->Thing.Type = T_EFFECT;
					break;
                case IDX_T_SHOT:
                    ThingSelected->Thing.Type = T_SHOT;
                    break;
				case IDX_T_DECORATION:
					ThingSelected->Thing.Type = T_SCENERY;
					ThingSelected->Thing.Model = M_SCENERY_SUB_LEVEL_SCENERY;
                    break;
				}

				if (net.IsInitialized())
				{
					struct Packet *p = new Packet;
					p->wType = PACKETTYPE_TYPE_OBJECT;
					p->wData[0] = ThingSelected->Idx;
					p->wData[1] = ThingSelected->Thing.Type;
					p->wData[2] = ThingSelected->Thing.Model;
					net.SendPacket(p);
					p->del();
				}

				memset(&ThingSelected->Thing.Bluff, 0, sizeof(ThingSelected->Thing.Bluff));
				DlgObjectUpdateInfo(hWnd);
				DlgInfoUpdate(hDlgInfo);
			}
			return 0;

		case IDC_OBJECT_MODEL:
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				SaveThingsUndoState();
				memset(&ThingSelected->Thing.Bluff, 0, sizeof(ThingSelected->Thing.Bluff));
				switch(ThingSelected->Thing.Type)
				{
				case T_PERSON:
					switch(SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))
					{
					case IDX_M_PERSON_WILD:         ThingSelected->Thing.Model = M_PERSON_WILD;         break;
					case IDX_M_PERSON_BRAVE:        ThingSelected->Thing.Model = M_PERSON_BRAVE;        break;
					case IDX_M_PERSON_WARRIOR:      ThingSelected->Thing.Model = M_PERSON_WARRIOR;      break;
					case IDX_M_PERSON_PRIEST:       ThingSelected->Thing.Model = M_PERSON_PRIEST;       break;
					case IDX_M_PERSON_SPY:          ThingSelected->Thing.Model = M_PERSON_SPY;          break;
					case IDX_M_PERSON_FIREWARRIOR:  ThingSelected->Thing.Model = M_PERSON_FIREWARRIOR;  break;
					case IDX_M_PERSON_SHAMAN:       ThingSelected->Thing.Model = M_PERSON_SHAMAN;       break;
					case IDX_M_PERSON_ANGELOFDEATH: ThingSelected->Thing.Model = M_PERSON_ANGELOFDEATH; break; 
					}
					break;
				case T_BUILDING:
					switch(SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))
					{
					case IDX_M_BUILDING_HUT1:              ThingSelected->Thing.Model = M_BUILDING_HUT1;              break;
					case IDX_M_BUILDING_HUT2:              ThingSelected->Thing.Model = M_BUILDING_HUT2;              break;
					case IDX_M_BUILDING_HUT3:              ThingSelected->Thing.Model = M_BUILDING_HUT3;              break;
					case IDX_M_BUILDING_TOWER:             ThingSelected->Thing.Model = M_BUILDING_TOWER;             break;
					case IDX_M_BUILDING_TEMPLE:            ThingSelected->Thing.Model = M_BUILDING_TEMPLE;            break;
					case IDX_M_BUILDING_SPY_TRAIN:         ThingSelected->Thing.Model = M_BUILDING_SPY_TRAIN;         break;
					case IDX_M_BUILDING_WARRIOR_TRAIN:     ThingSelected->Thing.Model = M_BUILDING_WARRIOR_TRAIN;     break;
					case IDX_M_BUILDING_FIREWARRIOR_TRAIN: ThingSelected->Thing.Model = M_BUILDING_FIREWARRIOR_TRAIN; break;
					case IDX_M_BUILDING_BOAT_HUT:          ThingSelected->Thing.Model = M_BUILDING_BOAT_HUT_1;        break;
					case IDX_M_BUILDING_AIRSHIP_HUT:       ThingSelected->Thing.Model = M_BUILDING_AIRSHIP_HUT_1;     break;
					case IDX_M_BUILDING_KNOWLEDGE:         ThingSelected->Thing.Model = M_BUILDING_KNOWLEDGE;         break;
					case IDX_M_BUILDING_PRISON:            ThingSelected->Thing.Model = M_BUILDING_PRISON;            break;
					}
					break;
				case T_SCENERY:
					if(ThingSelected->Thing.Model == M_SCENERY_TOP_LEVEL_SCENERY)
					{
						switch(SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))
						{
						case IDX_M_SPECIAL_A: ThingSelected->Thing.Scenery.IslandNum = M_SPECIAL_A; break;
						case IDX_M_SPECIAL_B: ThingSelected->Thing.Scenery.IslandNum = M_SPECIAL_B; break;
						case IDX_M_SPECIAL_C: ThingSelected->Thing.Scenery.IslandNum = M_SPECIAL_C; break;
						case IDX_M_SPECIAL_D: ThingSelected->Thing.Scenery.IslandNum = M_SPECIAL_D; break;
						case IDX_M_SPECIAL_E: ThingSelected->Thing.Scenery.IslandNum = M_SPECIAL_E; break;
						case IDX_M_SPECIAL_F: ThingSelected->Thing.Scenery.IslandNum = M_SPECIAL_F; break;
						}
						break;
					}
					else if((ThingSelected->Thing.Model == M_SCENERY_SUB_LEVEL_SCENERY)) //|| (ThingSelected->Thing.Model < M_SPECIAL_A || ThingSelected->Thing.Model > M_SPECIAL_F))
					{
						switch(SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))
						{
							case IDX_M_DECORATION_BOOK:				ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_BOOK;			break;
							case IDX_M_DECORATION_SHIELD:			ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_SHIELD;		break;
							case IDX_M_DECORATION_WELL:				ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_WELL;			break;
							case IDX_M_DECORATION_WINDMILL:			ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_WINDMILL;		break;
							case IDX_M_DECORATION_WOODEN_HUT:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_WOODEN_HUT;	break;

							case IDX_M_DECORATION_B_TOWER:			ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_B_TOWER;	break;
							case IDX_M_DECORATION_R_TOWER:			ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_R_TOWER;	break;
							case IDX_M_DECORATION_Y_TOWER:			ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_Y_TOWER;	break;
							case IDX_M_DECORATION_G_TOWER:			ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_G_TOWER;	break;

							case IDX_M_DECORATION_B_BOAT_HUT:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_B_BOAT_HUT;	break;
							case IDX_M_DECORATION_R_BOAT_HUT:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_R_BOAT_HUT;	break;
							case IDX_M_DECORATION_Y_BOAT_HUT:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_Y_BOAT_HUT;	break;
							case IDX_M_DECORATION_G_BOAT_HUT:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_G_BOAT_HUT;	break;

							case IDX_M_DECORATION_B_AIRSHIP_HUT:	ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_B_AIRSHIP_HUT;	break;
							case IDX_M_DECORATION_R_AIRSHIP_HUT:	ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_R_AIRSHIP_HUT;	break;
							case IDX_M_DECORATION_Y_AIRSHIP_HUT:	ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_Y_AIRSHIP_HUT;	break;
							case IDX_M_DECORATION_G_AIRSHIP_HUT:	ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_G_AIRSHIP_HUT;	break;

							case IDX_M_DECORATION_B_SPY_TRAIN:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_B_SPY_TRAIN;	break;
							case IDX_M_DECORATION_R_SPY_TRAIN:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_R_SPY_TRAIN;	break;
							case IDX_M_DECORATION_Y_SPY_TRAIN:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_Y_SPY_TRAIN;	break;
							case IDX_M_DECORATION_G_SPY_TRAIN:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_G_SPY_TRAIN;	break;

							case IDX_M_DECORATION_B_TEMPLE:			ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_B_TEMPLE;	break;
							case IDX_M_DECORATION_R_TEMPLE:			ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_R_TEMPLE;	break;
							case IDX_M_DECORATION_Y_TEMPLE:			ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_Y_TEMPLE;	break;
							case IDX_M_DECORATION_G_TEMPLE:			ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_G_TEMPLE;	break;
							
							case IDX_M_DECORATION_B_FIREWARRIOR_TRAIN:	ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_B_FIREWARRIOR_TRAIN;	break;
							case IDX_M_DECORATION_R_FIREWARRIOR_TRAIN:	ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_R_FIREWARRIOR_TRAIN;	break;
							case IDX_M_DECORATION_Y_FIREWARRIOR_TRAIN:	ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_Y_FIREWARRIOR_TRAIN;	break;
							case IDX_M_DECORATION_G_FIREWARRIOR_TRAIN:	ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_G_FIREWARRIOR_TRAIN;	break;
							
							case IDX_M_DECORATION_B_WARRIOR_TRAIN:	ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_B_WARRIOR_TRAIN;	break;
							case IDX_M_DECORATION_R_WARRIOR_TRAIN:	ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_R_WARRIOR_TRAIN;	break;
							case IDX_M_DECORATION_Y_WARRIOR_TRAIN:	ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_Y_WARRIOR_TRAIN;	break;
							case IDX_M_DECORATION_G_WARRIOR_TRAIN:	ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_G_WARRIOR_TRAIN;	break;
							
							case IDX_M_DECORATION_B_HUT1_A:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_B_HUT1_A;	break;
							case IDX_M_DECORATION_B_HUT1_B:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_B_HUT1_B;	break;
							case IDX_M_DECORATION_B_HUT1_C:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_B_HUT1_C;	break;
							
							case IDX_M_DECORATION_R_HUT1_A:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_R_HUT1_A;	break;
							case IDX_M_DECORATION_R_HUT1_B:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_R_HUT1_B;	break;
							case IDX_M_DECORATION_R_HUT1_C:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_R_HUT1_C;	break;

							case IDX_M_DECORATION_Y_HUT1_A:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_Y_HUT1_A;	break;
							case IDX_M_DECORATION_Y_HUT1_B:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_Y_HUT1_B;	break;
							case IDX_M_DECORATION_Y_HUT1_C:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_Y_HUT1_C;	break;
							
							case IDX_M_DECORATION_G_HUT1_A:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_G_HUT1_A;	break;
							case IDX_M_DECORATION_G_HUT1_B:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_G_HUT1_B;	break;
							case IDX_M_DECORATION_G_HUT1_C:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_G_HUT1_C;	break;
							
							case IDX_M_DECORATION_B_HUT2_A:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_B_HUT2_A;	break;
							case IDX_M_DECORATION_B_HUT2_B:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_B_HUT2_B;	break;
							case IDX_M_DECORATION_B_HUT2_C:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_B_HUT2_C;	break;
							
							case IDX_M_DECORATION_R_HUT2_A:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_R_HUT2_A;	break;
							case IDX_M_DECORATION_R_HUT2_B:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_R_HUT2_B;	break;
							case IDX_M_DECORATION_R_HUT2_C:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_R_HUT2_C;	break;
							
							case IDX_M_DECORATION_Y_HUT2_A:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_Y_HUT2_A;	break;
							case IDX_M_DECORATION_Y_HUT2_B:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_Y_HUT2_B;	break;
							case IDX_M_DECORATION_Y_HUT2_C:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_Y_HUT2_C;	break;
							
							case IDX_M_DECORATION_G_HUT2_A:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_G_HUT2_A;	break;
							case IDX_M_DECORATION_G_HUT2_B:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_G_HUT2_B;	break;
							case IDX_M_DECORATION_G_HUT2_C:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_G_HUT2_C;	break;
							
							case IDX_M_DECORATION_B_HUT3_A:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_B_HUT3_A;	break;
							case IDX_M_DECORATION_B_HUT3_B:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_B_HUT3_B;	break;
							case IDX_M_DECORATION_B_HUT3_C:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_B_HUT3_C;	break;
							
							case IDX_M_DECORATION_R_HUT3_A:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_R_HUT3_A;	break;
							case IDX_M_DECORATION_R_HUT3_B:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_R_HUT3_B;	break;
							case IDX_M_DECORATION_R_HUT3_C:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_R_HUT3_C;	break;
							
							case IDX_M_DECORATION_Y_HUT3_A:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_Y_HUT3_A;	break;
							case IDX_M_DECORATION_Y_HUT3_B:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_Y_HUT3_B;	break;
							case IDX_M_DECORATION_Y_HUT3_C:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_Y_HUT3_C;	break;
							
							case IDX_M_DECORATION_G_HUT3_A:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_G_HUT3_A;	break;
							case IDX_M_DECORATION_G_HUT3_B:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_G_HUT3_B;	break;
							case IDX_M_DECORATION_G_HUT3_C:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_G_HUT3_C;	break;
							
							case IDX_M_DECORATION_BOAT:			ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_BOAT;	break;
							case IDX_M_DECORATION_BALLOON:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_BALLOON;	break;

							case IDX_M_DECORATION_TOTEM_0:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_TOTEM_0;		break;
							case IDX_M_DECORATION_TOTEM_1:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_TOTEM_1;		break;
							case IDX_M_DECORATION_KNOWLEDGE_0:	ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_KNOWLEDGE_0;	break;
							case IDX_M_DECORATION_KNOWLEDGE_1:	ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_KNOWLEDGE_1;	break;
							case IDX_M_DECORATION_KNOWLEDGE_2:	ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_KNOWLEDGE_2;	break;
							case IDX_M_DECORATION_PRISON:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_PRISON;		break;
							case IDX_M_DECORATION_OBELISK:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_OBELISK;		break;
							case IDX_M_DECORATION_GARGOYLE_0:   ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_GARGOYLE_0;     break;
							case IDX_M_DECORATION_GARGOYLE_1:   ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_GARGOYLE_1;     break;
							case IDX_M_DECORATION_GARGOYLE_2:   ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_GARGOYLE_2;     break;
							case IDX_M_DECORATION_PILLAR:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_PILLAR;     break;
							case IDX_M_DECORATION_AOD_0:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_AOD_0;     break;
							case IDX_M_DECORATION_AOD_1:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_AOD_1;     break;
							case IDX_M_DECORATION_STONEHEAD:    ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_STONEHEAD;    break;
							case IDX_M_DECORATION_TREE_1:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_TREE_1;		break;
							case IDX_M_DECORATION_TREE_2:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_TREE_2;		break;
							case IDX_M_DECORATION_TREE_3:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_TREE_3;		break;
							case IDX_M_DECORATION_TREE_4:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_TREE_4;		break;
							case IDX_M_DECORATION_TREE_5:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_TREE_5;		break;
							case IDX_M_DECORATION_TREE_6:		ThingSelected->Thing.Scenery.IslandNum = M_DECORATION_TREE_6;		break;
						}
						break;
					}
					
					switch (SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))
					{
					case IDX_M_SCENERY_TREE_1:     ThingSelected->Thing.Model = M_SCENERY_TREE_1;     break;
					case IDX_M_SCENERY_TREE_2:     ThingSelected->Thing.Model = M_SCENERY_TREE_2;     break;
					case IDX_M_SCENERY_TREE_3:     ThingSelected->Thing.Model = M_SCENERY_TREE_3;     break;
					case IDX_M_SCENERY_TREE_4:     ThingSelected->Thing.Model = M_SCENERY_TREE_4;     break;
					case IDX_M_SCENERY_TREE_5:     ThingSelected->Thing.Model = M_SCENERY_TREE_5;     break;
					case IDX_M_SCENERY_TREE_6:     ThingSelected->Thing.Model = M_SCENERY_TREE_6;     break;
					case IDX_M_SCENERY_PLANT_1:    ThingSelected->Thing.Model = M_SCENERY_PLANT_1;    break;
					case IDX_M_SCENERY_PLANT_2:    ThingSelected->Thing.Model = M_SCENERY_PLANT_2;    break;
					case IDX_M_SCENERY_STONE_HEAD: ThingSelected->Thing.Model = M_SCENERY_STONE_HEAD; break;
					case IDX_M_SCENERY_WOOD_PILE:  ThingSelected->Thing.Model = M_SCENERY_WOOD_PILE;  break;
					case IDX_M_SCENERY_RS_PILLAR:  ThingSelected->Thing.Model = M_SCENERY_RS_PILLAR;  break;
					case IDX_M_SCENERY_ROCK:       ThingSelected->Thing.Model = M_SCENERY_ROCK;       break;
					case IDX_M_SCENERY_FIRE:       ThingSelected->Thing.Model = M_SCENERY_FIRE;       break;
					case IDX_M_SCENERY_FORBIDDEN:  ThingSelected->Thing.Model = M_SCENERY_FORBIDDEN;  break;
					}
					break;
					
				case T_GENERAL:
					switch(SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))
					{
					case IDX_M_GENERAL_DISCOVERY:
						ThingSelected->Thing.Model = M_GENERAL_DISCOVERY;
						ThingSelected->Thing.General.TriggerType = DISCOVERY_TRIGGER_IMMEDIATE;
						break;
					case IDX_M_GENERAL_TRIGGER:
						ThingSelected->Thing.Model = M_GENERAL_TRIGGER;
						break;
					case IDX_M_GENERAL_BUILDING_ADD_ON:
						ThingSelected->Thing.Model = M_GENERAL_BUILDING_ADD_ON;
						break;
					case IDX_M_GENERAL_LIGHT:
						ThingSelected->Thing.Model = M_GENERAL_LIGHT;
						break;
					}
					break;
				case T_SPELL:
					switch(SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))
					{
					case IDX_M_SPELL_BURN:           ThingSelected->Thing.Model = M_SPELL_BURN;           break;
					case IDX_M_SPELL_BLAST:          ThingSelected->Thing.Model = M_SPELL_BLAST;          break;
					case IDX_M_SPELL_LIGHTNING:      ThingSelected->Thing.Model = M_SPELL_LIGHTNING;      break;
					case IDX_M_SPELL_TORNADO:        ThingSelected->Thing.Model = M_SPELL_TORNADO;        break;
					case IDX_M_SPELL_SWARM:          ThingSelected->Thing.Model = M_SPELL_SWARM;          break;
					case IDX_M_SPELL_INVISIBILITY:   ThingSelected->Thing.Model = M_SPELL_INVISIBILITY;   break;
					case IDX_M_SPELL_HYPNOTISM:      ThingSelected->Thing.Model = M_SPELL_HYPNOTISM;      break;
					case IDX_M_SPELL_FIRESTORM:      ThingSelected->Thing.Model = M_SPELL_FIRESTORM;      break;
					case IDX_M_SPELL_GHOST_ARMY:     ThingSelected->Thing.Model = M_SPELL_GHOST_ARMY;     break;
					case IDX_M_SPELL_ERODE:          ThingSelected->Thing.Model = M_SPELL_ERODE;          break;
					case IDX_M_SPELL_SWAMP:          ThingSelected->Thing.Model = M_SPELL_SWAMP;          break;
					case IDX_M_SPELL_LAND_BRIDGE:    ThingSelected->Thing.Model = M_SPELL_LAND_BRIDGE;    break;
					case IDX_M_SPELL_ANGEL_OF_DEATH: ThingSelected->Thing.Model = M_SPELL_ANGEL_OF_DEATH; break;
					case IDX_M_SPELL_EARTHQUAKE:     ThingSelected->Thing.Model = M_SPELL_EARTHQUAKE;     break;
					case IDX_M_SPELL_FLATTEN:        ThingSelected->Thing.Model = M_SPELL_FLATTEN;        break;
					case IDX_M_SPELL_VOLCANO:        ThingSelected->Thing.Model = M_SPELL_VOLCANO;        break;
					case IDX_M_SPELL_CONVERT:        ThingSelected->Thing.Model = M_SPELL_CONVERT;        break;
					case IDX_M_SPELL_ARMAGEDDON:     ThingSelected->Thing.Model = M_SPELL_ARMAGEDDON;     break;
					case IDX_M_SPELL_MAGICAL_SHIELD: ThingSelected->Thing.Model = M_SPELL_MAGICAL_SHIELD; break;
					case IDX_M_SPELL_BLOODLUST:      ThingSelected->Thing.Model = M_SPELL_BLOODLUST;      break;
					case IDX_M_SPELL_TELEPORT:       ThingSelected->Thing.Model = M_SPELL_TELEPORT;       break;
					}
					break;
				case T_CREATURE:
					switch(SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))
					{
					case IDX_M_CREATURE_EAGLE: ThingSelected->Thing.Model = M_CREATURE_EAGLE; break;
					}
					break;
				case T_VEHICLE:
					switch(SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))
					{
					case IDX_M_VEHICLE_BOAT:    ThingSelected->Thing.Model = M_VEHICLE_BOAT_1;    break;
					case IDX_M_VEHICLE_AIRSHIP: ThingSelected->Thing.Model = M_VEHICLE_AIRSHIP_1; break;
					}
					break;
                case T_SHOT:
                    switch (SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))
                    {
                    case IDX_M_SHOT_STANDARD:                  ThingSelected->Thing.Model = M_SHOT_STANDARD;                  break;
                    case IDX_M_SHOT_STANDARD_2:                ThingSelected->Thing.Model = M_SHOT_STANDARD_2;                break;
                    case IDX_M_SHOT_STANDARD_3:                ThingSelected->Thing.Model = M_SHOT_STANDARD_3;                break;
                    case IDX_M_SHOT_FIREBALL:                  ThingSelected->Thing.Model = M_SHOT_FIREBALL;                  break;
                    case IDX_M_SHOT_LIGHTNING:                 ThingSelected->Thing.Model = M_SHOT_LIGHTNING;                 break;
                    case IDX_M_SHOT_SUPER_WARRIOR:             ThingSelected->Thing.Model = M_SHOT_SUPER_WARRIOR;             break;
                    case IDX_M_SHOT_VOLCANO_FIREBALL_1:        ThingSelected->Thing.Model = M_SHOT_VOLCANO_FIREBALL_1;        break;
                    case IDX_M_SHOT_VOLCANO_FIREBALL_2:        ThingSelected->Thing.Model = M_SHOT_VOLCANO_FIREBALL_2;        break;
                    }
                    break;
				case T_EFFECT:
					switch(SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))
					{
					case IDX_M_EFFECT_SIMPLE_BLAST:            ThingSelected->Thing.Model = M_EFFECT_SIMPLE_BLAST;            break;
					case IDX_M_EFFECT_SPRITE_CIRCLES:          ThingSelected->Thing.Model = M_EFFECT_SPRITE_CIRCLES;          break;
					case IDX_M_EFFECT_SMOKE:                   ThingSelected->Thing.Model = M_EFFECT_SMOKE;                   break;
					case IDX_M_EFFECT_LIGHTNING_ELEM:          ThingSelected->Thing.Model = M_EFFECT_LIGHTNING_ELEM;          break;
					case IDX_M_EFFECT_BURN_CELL_OBSTACLES:     ThingSelected->Thing.Model = M_EFFECT_BURN_CELL_OBSTACLES;     break;
					case IDX_M_EFFECT_FLATTEN_LAND:            ThingSelected->Thing.Model = M_EFFECT_FLATTEN_LAND;            break;
					case IDX_M_EFFECT_SPHERE_EXPLODE_1:        ThingSelected->Thing.Model = M_EFFECT_SPHERE_EXPLODE_1;        break;
					case IDX_M_EFFECT_FIREBALL:                ThingSelected->Thing.Model = M_EFFECT_FIREBALL;                break;
					case IDX_M_EFFECT_FIRECLOUD:               ThingSelected->Thing.Model = M_EFFECT_FIRECLOUD;               break;
					case IDX_M_EFFECT_GHOST_ARMY:              ThingSelected->Thing.Model = M_EFFECT_GHOST_ARMY;              break;
					case IDX_M_EFFECT_INVISIBILITY:            ThingSelected->Thing.Model = M_EFFECT_INVISIBILITY;            break;
					case IDX_M_EFFECT_VOLCANO:                 ThingSelected->Thing.Model = M_EFFECT_VOLCANO;                 break;
					case IDX_M_EFFECT_HYPNOTISM:               ThingSelected->Thing.Model = M_EFFECT_HYPNOTISM;               break;
					case IDX_M_EFFECT_LIGHTNING_BOLT:          ThingSelected->Thing.Model = M_EFFECT_LIGHTNING_BOLT;          break;
					case IDX_M_EFFECT_SWAMP:                   ThingSelected->Thing.Model = M_EFFECT_SWAMP;                   break;
					case IDX_M_EFFECT_ANGEL_OF_DEATH:          ThingSelected->Thing.Model = M_EFFECT_ANGEL_OF_DEATH;          break;
					case IDX_M_EFFECT_WHIRLWIND:               ThingSelected->Thing.Model = M_EFFECT_WHIRLWIND;               break;
					case IDX_M_EFFECT_INSECT_PLAGUE:           ThingSelected->Thing.Model = M_EFFECT_INSECT_PLAGUE;           break;
					case IDX_M_EFFECT_FIRESTORM:               ThingSelected->Thing.Model = M_EFFECT_FIRESTORM;               break;
					case IDX_M_EFFECT_EROSION:                 ThingSelected->Thing.Model = M_EFFECT_EROSION;                 break;
					case IDX_M_EFFECT_EARTHQUAKE:              ThingSelected->Thing.Model = M_EFFECT_EARTHQUAKE;              break;
					case IDX_M_EFFECT_FLY_THINGUMMY:           ThingSelected->Thing.Model = M_EFFECT_FLY_THINGUMMY;           break;
					case IDX_M_EFFECT_SPHERE_EXPLODE_AND_FIRE: ThingSelected->Thing.Model = M_EFFECT_SPHERE_EXPLODE_AND_FIRE; break;
					case IDX_M_EFFECT_BIG_FIRE:                ThingSelected->Thing.Model = M_EFFECT_BIG_FIRE;                break;
					case IDX_M_EFFECT_LIGHTNING:               ThingSelected->Thing.Model = M_EFFECT_LIGHTNING;               break;
					case IDX_M_EFFECT_FLATTEN:                 ThingSelected->Thing.Model = M_EFFECT_FLATTEN;                 break;
					case IDX_M_EFFECT_LAVA_FLOW:               ThingSelected->Thing.Model = M_EFFECT_LAVA_FLOW;               break;
					case IDX_M_EFFECT_PURIFY_LAND:             ThingSelected->Thing.Model = M_EFFECT_PURIFY_LAND;             break;
					case IDX_M_EFFECT_EXPLOSION_1:             ThingSelected->Thing.Model = M_EFFECT_EXPLOSION_1;             break;
					case IDX_M_EFFECT_EXPLOSION_2:             ThingSelected->Thing.Model = M_EFFECT_EXPLOSION_2;             break;
					case IDX_M_EFFECT_LAVA_SQUARE:             ThingSelected->Thing.Model = M_EFFECT_LAVA_SQUARE;             break;
					case IDX_M_EFFECT_WW_ELEMENT:              ThingSelected->Thing.Model = M_EFFECT_WW_ELEMENT;              break;
					case IDX_M_EFFECT_WW_DUST:                 ThingSelected->Thing.Model = M_EFFECT_WW_DUST;                 break;
					case IDX_M_EFFECT_HILL:                    ThingSelected->Thing.Model = M_EFFECT_HILL;                    break;
					case IDX_M_EFFECT_VALLEY:                  ThingSelected->Thing.Model = M_EFFECT_VALLEY;                  break;
					case IDX_M_EFFECT_PLACE_TREE:              ThingSelected->Thing.Model = M_EFFECT_PLACE_TREE;              break;
					case IDX_M_EFFECT_RISE:                    ThingSelected->Thing.Model = M_EFFECT_RISE;                    break;
					case IDX_M_EFFECT_DIP:                     ThingSelected->Thing.Model = M_EFFECT_DIP;                     break;
					case IDX_M_EFFECT_REIN_ROCK_DEBRIS:        ThingSelected->Thing.Model = M_EFFECT_REIN_ROCK_DEBRIS;        break;
					case IDX_M_EFFECT_PLACE_SHAMAN:            ThingSelected->Thing.Model = M_EFFECT_PLACE_SHAMAN;            break;
					case IDX_M_EFFECT_PLACE_WILD:              ThingSelected->Thing.Model = M_EFFECT_PLACE_WILD;              break;
					case IDX_M_EFFECT_BLDG_SMOKE:              ThingSelected->Thing.Model = M_EFFECT_BLDG_SMOKE;              break;
					case IDX_M_EFFECT_MUCH_SIMPLER_BLAST:      ThingSelected->Thing.Model = M_EFFECT_MUCH_SIMPLER_BLAST;      break;
					case IDX_M_EFFECT_TUMBLING_BRANCH:         ThingSelected->Thing.Model = M_EFFECT_TUMBLING_BRANCH;         break;
					case IDX_M_EFFECT_SPARKLE:                 ThingSelected->Thing.Model = M_EFFECT_SPARKLE;                 break;
					case IDX_M_EFFECT_SMALL_SPARKLE:           ThingSelected->Thing.Model = M_EFFECT_SMALL_SPARKLE;           break;
					case IDX_M_EFFECT_EXPLOSION_3:             ThingSelected->Thing.Model = M_EFFECT_EXPLOSION_3;             break;
					case IDX_M_EFFECT_ROCK_EXPLOSION:          ThingSelected->Thing.Model = M_EFFECT_ROCK_EXPLOSION;          break;
					case IDX_M_EFFECT_LAVA_GLOOP:              ThingSelected->Thing.Model = M_EFFECT_LAVA_GLOOP;              break;
					case IDX_M_EFFECT_SPLASH:                  ThingSelected->Thing.Model = M_EFFECT_SPLASH;                  break;
					case IDX_M_EFFECT_SMOKE_CLOUD:             ThingSelected->Thing.Model = M_EFFECT_SMOKE_CLOUD;             break;
					case IDX_M_EFFECT_SMOKE_CLOUD_CONSTANT:    ThingSelected->Thing.Model = M_EFFECT_SMOKE_CLOUD_CONSTANT;    break;
					case IDX_M_EFFECT_FIREBALL_2:              ThingSelected->Thing.Model = M_EFFECT_FIREBALL_2;              break;
					case IDX_M_EFFECT_GROUND_SHOCKWAVE:        ThingSelected->Thing.Model = M_EFFECT_GROUND_SHOCKWAVE;        break;
					case IDX_M_EFFECT_ORBITER:                 ThingSelected->Thing.Model = M_EFFECT_ORBITER;                 break;
					case IDX_M_EFFECT_BIG_SPARKLE:             ThingSelected->Thing.Model = M_EFFECT_BIG_SPARKLE;             break;
					case IDX_M_EFFECT_METEOR:                  ThingSelected->Thing.Model = M_EFFECT_METEOR;                  break;
					case IDX_M_EFFECT_CONVERT_WILD:            ThingSelected->Thing.Model = M_EFFECT_CONVERT_WILD;            break;
					case IDX_M_EFFECT_BLDG_SMOKE_2_FULL:       ThingSelected->Thing.Model = M_EFFECT_BLDG_SMOKE_2_FULL;       break;
					case IDX_M_EFFECT_BLDG_SMOKE_2_PARTIAL:    ThingSelected->Thing.Model = M_EFFECT_BLDG_SMOKE_2_PARTIAL;    break;
					case IDX_M_EFFECT_BLDG_DAMAGED_SMOKE:      ThingSelected->Thing.Model = M_EFFECT_BLDG_DAMAGED_SMOKE;      break;
					case IDX_M_EFFECT_DELETE_RS_PILLARS:       ThingSelected->Thing.Model = M_EFFECT_DELETE_RS_PILLARS;       break;
					case IDX_M_EFFECT_SPELL_BLAST:             ThingSelected->Thing.Model = M_EFFECT_SPELL_BLAST;             break;
					case IDX_M_EFFECT_FIRESTORM_SMOKE:         ThingSelected->Thing.Model = M_EFFECT_FIRESTORM_SMOKE;         break;
					case IDX_M_EFFECT_REVEAL_FOG_AREA:         ThingSelected->Thing.Model = M_EFFECT_REVEAL_FOG_AREA;         break;
					case IDX_M_EFFECT_SHIELD:                  ThingSelected->Thing.Model = M_EFFECT_SHIELD;                  break;
					case IDX_M_EFFECT_BOAT_HUT_REPAIR:         ThingSelected->Thing.Model = M_EFFECT_BOAT_HUT_REPAIR;         break;
					case IDX_M_EFFECT_REEDY_GRASS:             ThingSelected->Thing.Model = M_EFFECT_REEDY_GRASS;             break;
					case IDX_M_EFFECT_SWAMP_MIST:              ThingSelected->Thing.Model = M_EFFECT_SWAMP_MIST;              break;
					case IDX_M_EFFECT_ARMAGEDDON:              ThingSelected->Thing.Model = M_EFFECT_ARMAGEDDON;              break;
					case IDX_M_EFFECT_BLOODLUST:               ThingSelected->Thing.Model = M_EFFECT_BLOODLUST;               break;
					case IDX_M_EFFECT_TELEPORT:                ThingSelected->Thing.Model = M_EFFECT_TELEPORT;                break;
					case IDX_M_EFFECT_ATLANTIS_SET:            ThingSelected->Thing.Model = M_EFFECT_ATLANTIS_SET;            break;
					case IDX_M_EFFECT_ATLANTIS_INVOKE:         ThingSelected->Thing.Model = M_EFFECT_ATLANTIS_INVOKE;         break;
					case IDX_M_EFFECT_STATUE_TO_AOD:           ThingSelected->Thing.Model = M_EFFECT_STATUE_TO_AOD;           break;
					case IDX_M_EFFECT_FILL_ONE_SHOTS:          ThingSelected->Thing.Model = M_EFFECT_FILL_ONE_SHOTS;          break;
#if _ASCENSION
					case IDX_M_EFFECT_RESURRECT:				ThingSelected->Thing.Model = M_EFFECT_RESURRECT;			  break;
					case IDX_M_EFFECT_BIG_SMOKE:				ThingSelected->Thing.Model = M_EFFECT_BIG_SMOKE;			  break;
					case IDX_M_EFFECT_FIRESTORM_BALL:			ThingSelected->Thing.Model = M_EFFECT_FIRESTORM_BALL;         break;
					case IDX_M_EFFECT_LAVA_FLOW_NEW:			ThingSelected->Thing.Model = M_EFFECT_LAVA_FLOW_NEW;          break;
					case IDX_M_EFFECT_LAVA_FLOW_NEW_SHALLOW:    ThingSelected->Thing.Model = M_EFFECT_LAVA_FLOW_NEW_SHALLOW;  break;
					case IDX_M_EFFECT_LAVA_FLOW_NEW_DEEP:       ThingSelected->Thing.Model = M_EFFECT_LAVA_FLOW_NEW_DEEP;     break;
					case IDX_M_EFFECT_WATER:					ThingSelected->Thing.Model = M_EFFECT_WATER;			  break;
					case IDX_M_EFFECT_WATER_1X1:				ThingSelected->Thing.Model = M_EFFECT_WATER_1X1;          break;
					case IDX_M_EFFECT_LAVA_FLOW_NEW_WHITE_SHALLOW:     ThingSelected->Thing.Model = M_EFFECT_LAVA_FLOW_NEW_WHITE_SHALLOW;   break;
					case IDX_M_EFFECT_LAVA_FLOW_NEW_WHITE_DEEP: ThingSelected->Thing.Model = M_EFFECT_LAVA_FLOW_NEW_WHITE_DEEP;				break;
					case IDX_M_EFFECT_LAVA_FIXED: ThingSelected->Thing.Model = M_EFFECT_LAVA_FIXED;				break;
					case IDX_M_EFFECT_SOUL: ThingSelected->Thing.Model = M_EFFECT_SOUL;				break;
					case IDX_M_EFFECT_NEW_PLANT1: ThingSelected->Thing.Model = M_EFFECT_NEW_PLANT1;				break;
					case IDX_M_EFFECT_NEW_PLANT2: ThingSelected->Thing.Model = M_EFFECT_NEW_PLANT2;				break;
					case IDX_M_EFFECT_NEW_PLANT3: ThingSelected->Thing.Model = M_EFFECT_NEW_PLANT3;				break;
					case IDX_M_EFFECT_NEW_PLANT4: ThingSelected->Thing.Model = M_EFFECT_NEW_PLANT4;				break;
					case IDX_M_EFFECT_NEW_BRANCH: ThingSelected->Thing.Model = M_EFFECT_NEW_BRANCH;				break;
					case IDX_M_EFFECT_BONEPILE: ThingSelected->Thing.Model = M_EFFECT_BONEPILE;				break;
					case IDX_M_EFFECT_SMALL_SKULL: ThingSelected->Thing.Model = M_EFFECT_SMALL_SKULL;				break;
					case IDX_M_EFFECT_SMALL_SKULL2: ThingSelected->Thing.Model = M_EFFECT_SMALL_SKULL2;				break;
					case IDX_M_EFFECT_SMALL_SKULL3: ThingSelected->Thing.Model = M_EFFECT_SMALL_SKULL3;				break;
					case IDX_M_EFFECT_SKULL_THING: ThingSelected->Thing.Model = M_EFFECT_SKULL_THING;				break;
					case IDX_M_EFFECT_SKULL_THING_FIRE: ThingSelected->Thing.Model = M_EFFECT_SKULL_THING_FIRE;				break;
					case IDX_M_EFFECT_GHOST_SKULL: ThingSelected->Thing.Model = M_EFFECT_GHOST_SKULL;				break;
#endif
					case IDX_M_EFFECT_LAND_BRIDGE:
						ThingSelected->Thing.Model = M_EFFECT_LAND_BRIDGE;
						ThingSelected->Thing.Bluff[0] = ThingSelected->Thing.PosX;
						ThingSelected->Thing.Bluff[1] = ThingSelected->Thing.PosZ;
						ThingSelected->LandBridge.x = (float)(((ThingSelected->Thing.Bluff[0] & 0xFFFF) >> 8) / 2) + 0.5f;
						ThingSelected->LandBridge.z = (float)(((ThingSelected->Thing.Bluff[1] & 0xFFFF) >> 8) / 2) + 0.5f;
						ThingSelected->LandBridge.ex = ThingSelected->ex;
						ThingSelected->LandBridge.ey = ThingSelected->ey;
						ThingSelected->LandBridge.ez = ThingSelected->ez;
						ThingSelected->flags |= TF_DRAW_LANDBRIDGE;
						goto _effect_lb;
					}
					memset(&ThingSelected->Thing.Bluff, 0, sizeof(ThingSelected->Thing.Bluff));
_effect_lb:
					break;
				}

				if (net.IsInitialized())
				{
					struct Packet *p = new Packet;
					p->wType = PACKETTYPE_MODEL_OBJECT;
					p->wData[0] = ThingSelected->Idx;
					p->wData[1] = ThingSelected->Thing.Model;
					p->wData[2] = ThingSelected->Thing.Scenery.IslandNum;
					net.SendPacket(p);
					p->del();
				}

				DlgObjectUpdateInfo(hWnd);
				DlgInfoUpdate(hDlgInfo);
			}
			return 0;
		}
		return 0;

	case WM_CLOSE:
		DlgObjectToggle();
		return 0;
	}

	return 0;
}


void DlgObjectCenterSelected()
{
	if(!ThingSelected) return;

	fEnginePosX = ThingSelected->x;
	fEnginePosZ = ThingSelected->z;
	EngineUpdateView();
}


void DlgObjectSelect(THING *t, bool center)
{
	if(t) t->flags &= ~TF_EDIT_LANDBRIDGE;
	ThingSelected = t;
	DlgObjectUpdateInfo(hDlgObject);

	if(center)
		DlgObjectCenterSelected();
	else
		EngineUpdateView();
}


void DlgObjectUpdateInfo(HWND hWnd, bool lock)
{
	if(!hWnd) goto _end;

	if(lock) SendMessage(hWnd, WM_SETREDRAW, false, 0);

	HWND hItem;
	if(ThingSelected)
	{
		char szTemp[255];
		sprintf(szTemp, _T("Object - M: %d, I: %d"), ThingSelected->Thing.Model, ThingSelected->Thing.Scenery.IslandNum);
		SetWindowText(hWnd, szTemp);
		EnableWindow(GetDlgItem(hWnd, IDC_OBJECT_DELETE), true);
		EnableWindow(GetDlgItem(hWnd, IDC_OBJECT_OPTIONS), false);

		hItem = GetDlgItem(hWnd, IDC_OBJECT_OWNER);
		EnableWindow(hItem, true);
		switch(ThingSelected->Thing.Owner)
		{
        case OWNER_NEUTRAL: SendMessage(hItem, CB_SETCURSEL, IDX_OWNER_NEUTRAL, 0); break;
		case OWNER_BLUE:   SendMessage(hItem, CB_SETCURSEL, IDX_OWNER_BLUE, 0);   break;
		case OWNER_RED:    SendMessage(hItem, CB_SETCURSEL, IDX_OWNER_RED, 0);    break;
		case OWNER_YELLOW: SendMessage(hItem, CB_SETCURSEL, IDX_OWNER_YELLOW, 0); break;
		case OWNER_GREEN:  SendMessage(hItem, CB_SETCURSEL, IDX_OWNER_GREEN, 0);  break;
		case OWNER_CYAN:   SendMessage(hItem, CB_SETCURSEL, IDX_OWNER_CYAN, 0);   break;
		case OWNER_MAGENTA:    SendMessage(hItem, CB_SETCURSEL, IDX_OWNER_MAGENTA, 0);    break;
		case OWNER_BLACK: SendMessage(hItem, CB_SETCURSEL, IDX_OWNER_BLACK, 0); break;
		case OWNER_ORANGE:  SendMessage(hItem, CB_SETCURSEL, IDX_OWNER_ORANGE, 0);  break;
        case OWNER_HOSTBOT:  SendMessage(hItem, CB_SETCURSEL, IDX_OWNER_HOSTBOT, 0);  break;
		default:
            SendMessage(hItem, CB_SETCURSEL, ThingSelected->Thing.Owner, 0);

		}
		hItem = GetDlgItem(hWnd, IDC_OBJECT_TYPE);
		EnableWindow(hItem, true);
		switch(ThingSelected->Thing.Type)
		{
		case T_PERSON:
			SendMessage(hItem, CB_SETCURSEL, IDX_T_PERSON, 0);
			DlgObjectSetPersonList(hWnd);
			hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
			EnableWindow(hItem, true);
			switch(ThingSelected->Thing.Model)
			{
			case M_PERSON_WILD:         SendMessage(hItem, CB_SETCURSEL, IDX_M_PERSON_WILD, 0);         break;
			case M_PERSON_BRAVE:        SendMessage(hItem, CB_SETCURSEL, IDX_M_PERSON_BRAVE, 0);        break;
			case M_PERSON_WARRIOR:      SendMessage(hItem, CB_SETCURSEL, IDX_M_PERSON_WARRIOR, 0);      break;
			case M_PERSON_PRIEST:       SendMessage(hItem, CB_SETCURSEL, IDX_M_PERSON_PRIEST, 0);       break;
			case M_PERSON_SPY:          SendMessage(hItem, CB_SETCURSEL, IDX_M_PERSON_SPY, 0);          break;
			case M_PERSON_FIREWARRIOR:  SendMessage(hItem, CB_SETCURSEL, IDX_M_PERSON_FIREWARRIOR, 0);  break;
			case M_PERSON_SHAMAN:       SendMessage(hItem, CB_SETCURSEL, IDX_M_PERSON_SHAMAN, 0);       break;
			case M_PERSON_ANGELOFDEATH: SendMessage(hItem, CB_SETCURSEL, IDX_M_PERSON_ANGELOFDEATH, 0); break; 
			default: SendMessage(hItem, CB_SETCURSEL, -1, 0);
			}
			break;
		case T_BUILDING:
			SendMessage(hItem, CB_SETCURSEL, IDX_T_BUILDING, 0);
			DlgObjectSetBuildingList(hWnd);
			hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
			EnableWindow(hItem, true);
			switch(ThingSelected->Thing.Model)
			{
			case M_BUILDING_HUT1:              SendMessage(hItem, CB_SETCURSEL, IDX_M_BUILDING_HUT1, 0);              break;
			case M_BUILDING_HUT2:              SendMessage(hItem, CB_SETCURSEL, IDX_M_BUILDING_HUT2, 0);              break;
			case M_BUILDING_HUT3:              SendMessage(hItem, CB_SETCURSEL, IDX_M_BUILDING_HUT3, 0);              break;
			case M_BUILDING_TOWER:             SendMessage(hItem, CB_SETCURSEL, IDX_M_BUILDING_TOWER, 0);             break;
			case M_BUILDING_TEMPLE:            SendMessage(hItem, CB_SETCURSEL, IDX_M_BUILDING_TEMPLE, 0);            break;
			case M_BUILDING_SPY_TRAIN:         SendMessage(hItem, CB_SETCURSEL, IDX_M_BUILDING_SPY_TRAIN, 0);         break;
			case M_BUILDING_WARRIOR_TRAIN:     SendMessage(hItem, CB_SETCURSEL, IDX_M_BUILDING_WARRIOR_TRAIN, 0);     break;
			case M_BUILDING_FIREWARRIOR_TRAIN: SendMessage(hItem, CB_SETCURSEL, IDX_M_BUILDING_FIREWARRIOR_TRAIN, 0); break;
			case M_BUILDING_BOAT_HUT_1:
			case M_BUILDING_BOAT_HUT_2:		   SendMessage(hItem, CB_SETCURSEL, IDX_M_BUILDING_BOAT_HUT, 0);          break;
			case M_BUILDING_AIRSHIP_HUT_1:
			case M_BUILDING_AIRSHIP_HUT_2:     SendMessage(hItem, CB_SETCURSEL, IDX_M_BUILDING_AIRSHIP_HUT, 0);       break;
			case M_BUILDING_KNOWLEDGE:         SendMessage(hItem, CB_SETCURSEL, IDX_M_BUILDING_KNOWLEDGE, 0);         break;
			case M_BUILDING_PRISON:            SendMessage(hItem, CB_SETCURSEL, IDX_M_BUILDING_PRISON, 0);            break;
			default: SendMessage(hItem, CB_SETCURSEL, -1, 0);
			}
			hItem = GetDlgItem(hWnd, IDC_OBJECT_OPTIONS);
			SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)SZ_ROTATE);
			EnableWindow(hItem, true);
			break;
		case T_SCENERY:
			if(ThingSelected->Thing.Model == M_SCENERY_TOP_LEVEL_SCENERY)
			{
				SendMessage(hItem, CB_SETCURSEL, IDX_T_SPECIAL, 0);
				DlgObjectSetSpecialList(hWnd);
				hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
				EnableWindow(hItem, true);
				switch(ThingSelected->Thing.Scenery.IslandNum)
				{
				case M_SPECIAL_A: SendMessage(hItem, CB_SETCURSEL, IDX_M_SPECIAL_A, 0); break;
				case M_SPECIAL_B: SendMessage(hItem, CB_SETCURSEL, IDX_M_SPECIAL_B, 0); break;
				case M_SPECIAL_C: SendMessage(hItem, CB_SETCURSEL, IDX_M_SPECIAL_C, 0); break;
				case M_SPECIAL_D: SendMessage(hItem, CB_SETCURSEL, IDX_M_SPECIAL_D, 0); break;
				case M_SPECIAL_E: SendMessage(hItem, CB_SETCURSEL, IDX_M_SPECIAL_E, 0); break;
				case M_SPECIAL_F: SendMessage(hItem, CB_SETCURSEL, IDX_M_SPECIAL_F, 0); break;
				default: SendMessage(hItem, CB_SETCURSEL, -1, 0); goto _scenery_skip_rotate;
				}
				goto _scenery_skip;
			}
			else if (ThingSelected->Thing.Model == M_SCENERY_SUB_LEVEL_SCENERY)
			{
				SendMessage(hItem, CB_SETCURSEL, IDX_T_DECORATION, 0);
				DlgObjectSetDecorationList(hWnd);
				hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
				EnableWindow(hItem, true);

				switch (ThingSelected->Thing.Scenery.IslandNum)
				{
				case M_DECORATION_BOOK:						SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_BOOK, 0);						break;
				case M_DECORATION_SHIELD:                   SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_SHIELD, 0);					break;
				case M_DECORATION_WELL:                     SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_WELL, 0);						break;
				case M_DECORATION_WINDMILL:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_WINDMILL, 0);					break;
				case M_DECORATION_WOODEN_HUT:               SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_WOODEN_HUT, 0);				break;

				case M_DECORATION_B_TOWER:                  SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_B_TOWER, 0);					break;
				case M_DECORATION_R_TOWER:                  SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_R_TOWER, 0);					break;
				case M_DECORATION_Y_TOWER:                  SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_Y_TOWER, 0);					break;
				case M_DECORATION_G_TOWER:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_G_TOWER, 0);					break;
					
				case M_DECORATION_B_BOAT_HUT:               SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_B_BOAT_HUT, 0);				break;
				case M_DECORATION_R_BOAT_HUT:               SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_R_BOAT_HUT, 0);				break;
				case M_DECORATION_Y_BOAT_HUT:               SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_Y_BOAT_HUT, 0);				break;
				case M_DECORATION_G_BOAT_HUT:               SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_G_BOAT_HUT, 0);				break;
					
				case M_DECORATION_B_AIRSHIP_HUT:            SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_B_AIRSHIP_HUT, 0);			break;
				case M_DECORATION_R_AIRSHIP_HUT:            SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_R_AIRSHIP_HUT, 0);			break;
				case M_DECORATION_Y_AIRSHIP_HUT:            SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_Y_AIRSHIP_HUT, 0);			break;
				case M_DECORATION_G_AIRSHIP_HUT:            SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_G_AIRSHIP_HUT, 0);			break;
					
				case M_DECORATION_B_SPY_TRAIN:              SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_B_SPY_TRAIN, 0);				break;
				case M_DECORATION_R_SPY_TRAIN:              SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_R_SPY_TRAIN, 0);				break;
				case M_DECORATION_Y_SPY_TRAIN:              SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_Y_SPY_TRAIN, 0);				break;
				case M_DECORATION_G_SPY_TRAIN:              SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_G_SPY_TRAIN, 0);				break;
					
				case M_DECORATION_B_TEMPLE:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_B_TEMPLE, 0);					break;
				case M_DECORATION_R_TEMPLE:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_R_TEMPLE, 0);					break;
				case M_DECORATION_Y_TEMPLE:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_Y_TEMPLE, 0);					break;
				case M_DECORATION_G_TEMPLE:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_G_TEMPLE, 0);					break;
					
				case M_DECORATION_B_FIREWARRIOR_TRAIN:      SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_B_FIREWARRIOR_TRAIN, 0);		break;
				case M_DECORATION_R_FIREWARRIOR_TRAIN:      SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_R_FIREWARRIOR_TRAIN, 0);		break;
				case M_DECORATION_Y_FIREWARRIOR_TRAIN:      SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_Y_FIREWARRIOR_TRAIN, 0);		break;
				case M_DECORATION_G_FIREWARRIOR_TRAIN:      SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_G_FIREWARRIOR_TRAIN, 0);		break;

				case M_DECORATION_B_WARRIOR_TRAIN:          SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_B_WARRIOR_TRAIN, 0);			break;
				case M_DECORATION_R_WARRIOR_TRAIN:          SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_R_WARRIOR_TRAIN, 0);			break;
				case M_DECORATION_Y_WARRIOR_TRAIN:          SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_Y_WARRIOR_TRAIN, 0);			break;
				case M_DECORATION_G_WARRIOR_TRAIN:          SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_G_WARRIOR_TRAIN, 0);			break;
					
				case M_DECORATION_B_HUT1_A:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_B_HUT1_A, 0);					break;
				case M_DECORATION_B_HUT1_B:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_B_HUT1_B, 0);					break;
				case M_DECORATION_B_HUT1_C:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_B_HUT1_C, 0);					break;
					
				case M_DECORATION_R_HUT1_A:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_R_HUT1_A, 0);					break;
				case M_DECORATION_R_HUT1_B:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_R_HUT1_B, 0);					break;
				case M_DECORATION_R_HUT1_C:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_R_HUT1_C, 0);					break;
					
				case M_DECORATION_Y_HUT1_A:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_Y_HUT1_A, 0);					break;
				case M_DECORATION_Y_HUT1_B:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_Y_HUT1_B, 0);					break;
				case M_DECORATION_Y_HUT1_C:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_Y_HUT1_C, 0);					break;
					
				case M_DECORATION_G_HUT1_A:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_G_HUT1_A, 0);					break;
				case M_DECORATION_G_HUT1_B:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_G_HUT1_B, 0);					break;
				case M_DECORATION_G_HUT1_C:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_G_HUT1_C, 0);					break;

				case M_DECORATION_B_HUT2_A:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_B_HUT2_A, 0);					break;
				case M_DECORATION_B_HUT2_B:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_B_HUT2_B, 0);					break;
				case M_DECORATION_B_HUT2_C:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_B_HUT2_C, 0);					break;
					
				case M_DECORATION_R_HUT2_A:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_R_HUT2_A, 0);					break;
				case M_DECORATION_R_HUT2_B:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_R_HUT2_B, 0);					break;
				case M_DECORATION_R_HUT2_C:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_R_HUT2_C, 0);					break;
					
				case M_DECORATION_Y_HUT2_A:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_Y_HUT2_A, 0);					break;
				case M_DECORATION_Y_HUT2_B:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_Y_HUT2_B, 0);					break;
				case M_DECORATION_Y_HUT2_C:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_Y_HUT2_C, 0);					break;
					
				case M_DECORATION_G_HUT2_A:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_G_HUT2_A, 0);					break;
				case M_DECORATION_G_HUT2_B:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_G_HUT2_B, 0);					break;
				case M_DECORATION_G_HUT2_C:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_G_HUT2_C, 0);					break;

				case M_DECORATION_B_HUT3_A:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_B_HUT3_A, 0);					break;
				case M_DECORATION_B_HUT3_B:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_B_HUT3_B, 0);					break;
				case M_DECORATION_B_HUT3_C:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_B_HUT3_C, 0);					break;
					
				case M_DECORATION_R_HUT3_A:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_R_HUT3_A, 0);					break;
				case M_DECORATION_R_HUT3_B:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_R_HUT3_B, 0);					break;
				case M_DECORATION_R_HUT3_C:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_R_HUT3_C, 0);					break;
					
				case M_DECORATION_Y_HUT3_A:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_Y_HUT3_A, 0);					break;
				case M_DECORATION_Y_HUT3_B:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_Y_HUT3_B, 0);					break;
				case M_DECORATION_Y_HUT3_C:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_Y_HUT3_C, 0);					break;
					
				case M_DECORATION_G_HUT3_A:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_G_HUT3_A, 0);					break;
				case M_DECORATION_G_HUT3_B:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_G_HUT3_B, 0);					break;
				case M_DECORATION_G_HUT3_C:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_G_HUT3_C, 0);					break;

				case M_DECORATION_BOAT:						SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_BOAT, 0);						break;
				case M_DECORATION_BALLOON:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_BALLOON, 0);					break;

				case M_DECORATION_TOTEM_0:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_TOTEM_0, 0);					break;
				case M_DECORATION_TOTEM_1:                  SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_TOTEM_1, 0);					break;
				case M_DECORATION_KNOWLEDGE_0:              SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_KNOWLEDGE_0, 0);				break;
				case M_DECORATION_KNOWLEDGE_1:              SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_KNOWLEDGE_1, 0);				break;
				case M_DECORATION_KNOWLEDGE_2:              SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_KNOWLEDGE_2, 0);				break;
				case M_DECORATION_PRISON:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_PRISON, 0);					break;
				case M_DECORATION_OBELISK:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_OBELISK, 0);					break;
				case M_DECORATION_GARGOYLE_0:				SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_GARGOYLE_0, 0);				break;
				case M_DECORATION_GARGOYLE_1:				SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_GARGOYLE_1, 0);				break;
				case M_DECORATION_GARGOYLE_2:				SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_GARGOYLE_2, 0);				break;
				case M_DECORATION_PILLAR:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_PILLAR, 0);					break;
				case M_DECORATION_AOD_0:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_AOD_0, 0);					break;
				case M_DECORATION_AOD_1:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_AOD_1, 0);					break;
				case M_DECORATION_STONEHEAD:                SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_STONEHEAD, 0);				break;

				case M_DECORATION_TREE_1:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_TREE_1, 0);					break;
				case M_DECORATION_TREE_2:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_TREE_2, 0);					break;
				case M_DECORATION_TREE_3:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_TREE_3, 0);					break;
				case M_DECORATION_TREE_4:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_TREE_4, 0);					break;
				case M_DECORATION_TREE_5:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_TREE_5, 0);					break;
				case M_DECORATION_TREE_6:					SendMessage(hItem, CB_SETCURSEL, IDX_M_DECORATION_TREE_6, 0);					break;
				default: SendMessage(hItem, CB_SETCURSEL, -1, 0);  goto _scenery_skip_rotate;
				}
		goto _scenery_skip;
			}

			SendMessage(hItem, CB_SETCURSEL, IDX_T_SCENERY, 0);
			DlgObjectSetSceneryList(hWnd);
			hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
			EnableWindow(hItem, true);
			switch (ThingSelected->Thing.Model)
			{
			case M_SCENERY_TREE_1:     SendMessage(hItem, CB_SETCURSEL, IDX_M_SCENERY_TREE_1, 0);     break;
			case M_SCENERY_TREE_2:     SendMessage(hItem, CB_SETCURSEL, IDX_M_SCENERY_TREE_2, 0);     break;
			case M_SCENERY_TREE_3:     SendMessage(hItem, CB_SETCURSEL, IDX_M_SCENERY_TREE_3, 0);     break;
			case M_SCENERY_TREE_4:     SendMessage(hItem, CB_SETCURSEL, IDX_M_SCENERY_TREE_4, 0);     break;
			case M_SCENERY_TREE_5:     SendMessage(hItem, CB_SETCURSEL, IDX_M_SCENERY_TREE_5, 0);     break;
			case M_SCENERY_TREE_6:     SendMessage(hItem, CB_SETCURSEL, IDX_M_SCENERY_TREE_6, 0);     break;
			case M_SCENERY_PLANT_1:    SendMessage(hItem, CB_SETCURSEL, IDX_M_SCENERY_PLANT_1, 0);    goto _scenery_skip_rotate;
			case M_SCENERY_PLANT_2:    SendMessage(hItem, CB_SETCURSEL, IDX_M_SCENERY_PLANT_2, 0);    goto _scenery_skip_rotate;
			case M_SCENERY_STONE_HEAD: SendMessage(hItem, CB_SETCURSEL, IDX_M_SCENERY_STONE_HEAD, 0); break;
			case M_SCENERY_WOOD_PILE:  SendMessage(hItem, CB_SETCURSEL, IDX_M_SCENERY_WOOD_PILE, 0);  goto _scenery_skip_rotate;
			case M_SCENERY_RS_PILLAR:  SendMessage(hItem, CB_SETCURSEL, IDX_M_SCENERY_RS_PILLAR, 0);  break;
			case M_SCENERY_ROCK:       SendMessage(hItem, CB_SETCURSEL, IDX_M_SCENERY_ROCK, 0);       goto _scenery_skip_rotate;
			case M_SCENERY_FIRE:       SendMessage(hItem, CB_SETCURSEL, IDX_M_SCENERY_FIRE, 0);       goto _scenery_skip_rotate;
			case M_SCENERY_FORBIDDEN:  SendMessage(hItem, CB_SETCURSEL, IDX_M_SCENERY_FORBIDDEN, 0);  goto _scenery_skip_rotate;
			default: SendMessage(hItem, CB_SETCURSEL, -1, 0); goto _scenery_skip_rotate;
			}
		_scenery_skip:
			hItem = GetDlgItem(hWnd, IDC_OBJECT_OPTIONS);
			SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)SZ_ROTATE);
			EnableWindow(hItem, true);
		_scenery_skip_rotate:
			break;
		case T_GENERAL:
			SendMessage(hItem, CB_SETCURSEL, IDX_T_GENERAL, 0);
			DlgObjectSetGeneralList(hWnd);
			hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
			EnableWindow(hItem, true);
			switch(ThingSelected->Thing.Model)
			{
			case M_GENERAL_DISCOVERY:
				SendMessage(hItem, CB_SETCURSEL, IDX_M_GENERAL_DISCOVERY, 0);
				hItem = GetDlgItem(hWnd, IDC_OBJECT_OPTIONS);
				SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)SZ_OPTIONS);
				EnableWindow(hItem, true);
				break;
			case M_GENERAL_TRIGGER:
				SendMessage(hItem, CB_SETCURSEL, IDX_M_GENERAL_TRIGGER, 0);
				hItem = GetDlgItem(hWnd, IDC_OBJECT_OPTIONS);
				SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)SZ_OPTIONS);
				EnableWindow(hItem, true);
				break;
			case M_GENERAL_BUILDING_ADD_ON:
				SendMessage(hItem, CB_SETCURSEL, IDX_M_GENERAL_BUILDING_ADD_ON, 0);
				break;
			case M_GENERAL_LIGHT:
				SendMessage(hItem, CB_SETCURSEL, IDX_M_GENERAL_LIGHT, 0);
				break;
			default: SendMessage(hItem, CB_SETCURSEL, -1, 0);
			}
			break;
		case T_SPELL:
			SendMessage(hItem, CB_SETCURSEL, IDX_T_SPELL, 0);
			DlgObjectSetSpellList(hWnd);
			hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
			EnableWindow(hItem, true);
			switch(ThingSelected->Thing.Model)
			{
			case M_SPELL_BURN:           SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_BURN, 0);           break;
			case M_SPELL_BLAST:          SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_BLAST, 0);          break;
			case M_SPELL_LIGHTNING:      SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_LIGHTNING, 0);      break;
			case M_SPELL_TORNADO:        SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_TORNADO, 0);        break;
			case M_SPELL_SWARM:          SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_SWARM, 0);          break;
			case M_SPELL_INVISIBILITY:   SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_INVISIBILITY, 0);   break;
			case M_SPELL_HYPNOTISM:      SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_HYPNOTISM, 0);      break;
			case M_SPELL_FIRESTORM:      SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_FIRESTORM, 0);      break;
			case M_SPELL_GHOST_ARMY:     SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_GHOST_ARMY, 0);     break;
			case M_SPELL_ERODE:          SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_ERODE, 0);          break;
			case M_SPELL_SWAMP:          SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_SWAMP, 0);          break;
			case M_SPELL_LAND_BRIDGE:    SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_LAND_BRIDGE, 0);    break;
			case M_SPELL_ANGEL_OF_DEATH: SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_ANGEL_OF_DEATH, 0); break;
			case M_SPELL_EARTHQUAKE:     SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_EARTHQUAKE, 0);     break;
			case M_SPELL_FLATTEN:        SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_FLATTEN, 0);        break;
			case M_SPELL_VOLCANO:        SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_VOLCANO, 0);        break;
			case M_SPELL_CONVERT:        SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_CONVERT, 0);        break;
			case M_SPELL_ARMAGEDDON:     SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_ARMAGEDDON, 0);     break;
			case M_SPELL_MAGICAL_SHIELD: SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_MAGICAL_SHIELD, 0); break;
			case M_SPELL_BLOODLUST:      SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_BLOODLUST, 0);      break;
			case M_SPELL_TELEPORT:       SendMessage(hItem, CB_SETCURSEL, IDX_M_SPELL_TELEPORT, 0);       break;
			default: SendMessage(hItem, CB_SETCURSEL, -1, 0);
			}
			break;
		case T_EFFECT:
			SendMessage(hItem, CB_SETCURSEL, IDX_T_EFFECT, 0);
			DlgObjectSetEffectList(hWnd);
			hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
			EnableWindow(hItem, true);
			switch(ThingSelected->Thing.Model)
			{
			case M_EFFECT_SIMPLE_BLAST:            SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SIMPLE_BLAST, 0);            break;
			case M_EFFECT_SPRITE_CIRCLES:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SPRITE_CIRCLES, 0);          break;
			case M_EFFECT_SMOKE:                   SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SMOKE, 0);                   break;
			case M_EFFECT_LIGHTNING_ELEM:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_LIGHTNING_ELEM, 0);          break;
			case M_EFFECT_BURN_CELL_OBSTACLES:     SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_BURN_CELL_OBSTACLES, 0);     break;
			case M_EFFECT_FLATTEN_LAND:            SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_FLATTEN_LAND, 0);            break;
			case M_EFFECT_SPHERE_EXPLODE_1:        SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SPHERE_EXPLODE_1, 0);        break;
			case M_EFFECT_FIREBALL:                SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_FIREBALL, 0);                break;
			case M_EFFECT_FIRECLOUD:               SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_FIRECLOUD, 0);               break;
			case M_EFFECT_GHOST_ARMY:              SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_GHOST_ARMY, 0);              break;
			case M_EFFECT_INVISIBILITY:            SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_INVISIBILITY, 0);            break;
			case M_EFFECT_VOLCANO:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_VOLCANO, 0);                 break;
			case M_EFFECT_HYPNOTISM:               SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_HYPNOTISM, 0);               break;
			case M_EFFECT_LIGHTNING_BOLT:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_LIGHTNING_BOLT, 0);          break;
			case M_EFFECT_SWAMP:                   SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SWAMP, 0);                   break;
			case M_EFFECT_ANGEL_OF_DEATH:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_ANGEL_OF_DEATH, 0);          break;
			case M_EFFECT_WHIRLWIND:               SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_WHIRLWIND, 0);               break;
			case M_EFFECT_INSECT_PLAGUE:           SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_INSECT_PLAGUE, 0);           break;
			case M_EFFECT_FIRESTORM:               SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_FIRESTORM, 0);               break;
			case M_EFFECT_EROSION:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_EROSION, 0);                 break;
			case M_EFFECT_EARTHQUAKE:              SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_EARTHQUAKE, 0);              break;
			case M_EFFECT_FLY_THINGUMMY:           SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_FLY_THINGUMMY, 0);           break;
			case M_EFFECT_SPHERE_EXPLODE_AND_FIRE: SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SPHERE_EXPLODE_AND_FIRE, 0); break;
			case M_EFFECT_BIG_FIRE:                SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_BIG_FIRE, 0);                break;
			case M_EFFECT_LIGHTNING:               SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_LIGHTNING, 0);               break;
			case M_EFFECT_FLATTEN:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_FLATTEN, 0);                 break;
			case M_EFFECT_LAVA_FLOW:               SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_LAVA_FLOW, 0);               break;
			case M_EFFECT_PURIFY_LAND:             SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_PURIFY_LAND, 0);             break;
			case M_EFFECT_EXPLOSION_1:             SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_EXPLOSION_1, 0);             break;
			case M_EFFECT_EXPLOSION_2:             SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_EXPLOSION_2, 0);             break;
			case M_EFFECT_LAVA_SQUARE:             SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_LAVA_SQUARE, 0);             break;
			case M_EFFECT_WW_ELEMENT:              SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_WW_ELEMENT, 0);              break;
			case M_EFFECT_WW_DUST:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_WW_DUST, 0);                 break;
			case M_EFFECT_HILL:                    SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_HILL, 0);                    break;
			case M_EFFECT_VALLEY:                  SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_VALLEY, 0);                  break;
			case M_EFFECT_PLACE_TREE:              SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_PLACE_TREE, 0);              break;
			case M_EFFECT_RISE:                    SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_RISE, 0);                    break;
			case M_EFFECT_DIP:                     SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_DIP, 0);                     break;
			case M_EFFECT_REIN_ROCK_DEBRIS:        SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_REIN_ROCK_DEBRIS, 0);        break;
			case M_EFFECT_PLACE_SHAMAN:            SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_PLACE_SHAMAN, 0);            break;
			case M_EFFECT_PLACE_WILD:              SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_PLACE_WILD, 0);              break;
			case M_EFFECT_BLDG_SMOKE:              SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_BLDG_SMOKE, 0);              break;
			case M_EFFECT_MUCH_SIMPLER_BLAST:      SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_MUCH_SIMPLER_BLAST, 0);      break;
			case M_EFFECT_TUMBLING_BRANCH:         SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_TUMBLING_BRANCH, 0);         break;
			case M_EFFECT_SPARKLE:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SPARKLE, 0);                 break;
			case M_EFFECT_SMALL_SPARKLE:           SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SMALL_SPARKLE, 0);           break;
			case M_EFFECT_EXPLOSION_3:             SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_EXPLOSION_3, 0);             break;
			case M_EFFECT_ROCK_EXPLOSION:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_ROCK_EXPLOSION, 0);          break;
			case M_EFFECT_LAVA_GLOOP:              SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_LAVA_GLOOP, 0);              break;
			case M_EFFECT_SPLASH:                  SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SPLASH, 0);                  break;
			case M_EFFECT_SMOKE_CLOUD:             SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SMOKE_CLOUD, 0);             break;
			case M_EFFECT_SMOKE_CLOUD_CONSTANT:    SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SMOKE_CLOUD_CONSTANT, 0);    break;
			case M_EFFECT_FIREBALL_2:              SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_FIREBALL_2, 0);              break;
			case M_EFFECT_GROUND_SHOCKWAVE:        SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_GROUND_SHOCKWAVE, 0);        break;
			case M_EFFECT_ORBITER:                 SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_ORBITER, 0);                 break;
			case M_EFFECT_BIG_SPARKLE:             SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_BIG_SPARKLE, 0);             break;
			case M_EFFECT_METEOR:                  SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_METEOR, 0);                  break;
			case M_EFFECT_CONVERT_WILD:            SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_CONVERT_WILD, 0);            break;
			case M_EFFECT_BLDG_SMOKE_2_FULL:       SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_BLDG_SMOKE_2_FULL, 0);       break;
			case M_EFFECT_BLDG_SMOKE_2_PARTIAL:    SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_BLDG_SMOKE_2_PARTIAL, 0);    break;
			case M_EFFECT_BLDG_DAMAGED_SMOKE:      SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_BLDG_DAMAGED_SMOKE, 0);      break;
			case M_EFFECT_DELETE_RS_PILLARS:       SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_DELETE_RS_PILLARS, 0);       break;
			case M_EFFECT_SPELL_BLAST:             SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SPELL_BLAST, 0);             break;
			case M_EFFECT_FIRESTORM_SMOKE:         SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_FIRESTORM_SMOKE, 0);         break;
			case M_EFFECT_REVEAL_FOG_AREA:         SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_REVEAL_FOG_AREA, 0);         break;
			case M_EFFECT_SHIELD:                  SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SHIELD, 0);                  break;
			case M_EFFECT_BOAT_HUT_REPAIR:         SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_BOAT_HUT_REPAIR, 0);         break;
			case M_EFFECT_REEDY_GRASS:             SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_REEDY_GRASS, 0);             break;
			case M_EFFECT_SWAMP_MIST:              SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SWAMP_MIST, 0);              break;
			case M_EFFECT_ARMAGEDDON:              SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_ARMAGEDDON, 0);              break;
			case M_EFFECT_BLOODLUST:               SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_BLOODLUST, 0);               break;
			case M_EFFECT_TELEPORT:                SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_TELEPORT, 0);                break;
			case M_EFFECT_ATLANTIS_SET:            SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_ATLANTIS_SET, 0);            break;
			case M_EFFECT_ATLANTIS_INVOKE:         SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_ATLANTIS_INVOKE, 0);         break;
			case M_EFFECT_STATUE_TO_AOD:           SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_STATUE_TO_AOD, 0);           break;
			case M_EFFECT_FILL_ONE_SHOTS:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_FILL_ONE_SHOTS, 0);          break;
#if _ASCENSION
			case M_EFFECT_RESURRECT:              SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_RESURRECT, 0);              break;
			case M_EFFECT_BIG_SMOKE:              SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_BIG_SMOKE, 0);              break;
			case M_EFFECT_FIRESTORM_BALL:               SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_FIRESTORM_BALL, 0);               break;
			case M_EFFECT_LAVA_FLOW_NEW:                SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_LAVA_FLOW_NEW, 0);                break;
			case M_EFFECT_LAVA_FLOW_NEW_SHALLOW:            SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_LAVA_FLOW_NEW_SHALLOW, 0);            break;
			case M_EFFECT_LAVA_FLOW_NEW_DEEP:         SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_LAVA_FLOW_NEW_DEEP, 0);         break;
			case M_EFFECT_WATER:           SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_WATER, 0);           break;
			case M_EFFECT_WATER_1X1:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_WATER_1X1, 0);          break;
			case M_EFFECT_LAVA_FLOW_NEW_WHITE_SHALLOW:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_LAVA_FLOW_NEW_WHITE_SHALLOW, 0);          break;
			case M_EFFECT_LAVA_FLOW_NEW_WHITE_DEEP:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_LAVA_FLOW_NEW_WHITE_DEEP, 0);          break;
			case M_EFFECT_LAVA_FIXED:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_LAVA_FIXED, 0);          break;
			case M_EFFECT_SOUL:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SOUL, 0);          break;
			case M_EFFECT_NEW_PLANT1:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_NEW_PLANT1, 0);          break;
			case M_EFFECT_NEW_PLANT2:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_NEW_PLANT2, 0);          break;
			case M_EFFECT_NEW_PLANT3:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_NEW_PLANT3, 0);          break;
			case M_EFFECT_NEW_PLANT4:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_NEW_PLANT4, 0);          break;
			case M_EFFECT_NEW_BRANCH:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_NEW_BRANCH, 0);          break;
			case M_EFFECT_BONEPILE:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_BONEPILE, 0);          break;
			case M_EFFECT_SMALL_SKULL:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SMALL_SKULL, 0);          break;
			case M_EFFECT_SMALL_SKULL2:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SMALL_SKULL2, 0);          break;
			case M_EFFECT_SMALL_SKULL3:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SMALL_SKULL3, 0);          break;
			case M_EFFECT_SKULL_THING:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SKULL_THING, 0);          break;
			case M_EFFECT_SKULL_THING_FIRE:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_SKULL_THING_FIRE, 0);          break;
			case M_EFFECT_GHOST_SKULL:          SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_GHOST_SKULL, 0);          break;
#endif
			case M_EFFECT_LAND_BRIDGE:
				{
					SendMessage(hItem, CB_SETCURSEL, IDX_M_EFFECT_LAND_BRIDGE, 0);

					hItem = GetDlgItem(hWnd, IDC_OBJECT_OPTIONS);

					if(ThingSelected->flags & TF_EDIT_LANDBRIDGE)
						SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)SZ_LANDBRIDGE_START);
					else
						SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)SZ_LANDBRIDGE_END);

					EnableWindow(hItem, true);
				}
				break;

            case T_SHOT:
                SendMessage(hItem, CB_SETCURSEL, IDX_T_SHOT, 0);
                DlgObjectSetEffectList(hWnd);
                hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
                EnableWindow(hItem, true);
                switch (ThingSelected->Thing.Model)
                {
                case IDX_M_SHOT_STANDARD:                  SendMessage(hItem, CB_SETCURSEL, M_SHOT_STANDARD, 0);                  break;
                case IDX_M_SHOT_STANDARD_2:                SendMessage(hItem, CB_SETCURSEL, M_SHOT_STANDARD_2, 0);                break;
                case IDX_M_SHOT_STANDARD_3:                SendMessage(hItem, CB_SETCURSEL, M_SHOT_STANDARD_3, 0);                break;
                case IDX_M_SHOT_FIREBALL:                  SendMessage(hItem, CB_SETCURSEL, M_SHOT_FIREBALL, 0);                  break;
                case IDX_M_SHOT_LIGHTNING:                 SendMessage(hItem, CB_SETCURSEL, M_SHOT_LIGHTNING, 0);                 break;
                case IDX_M_SHOT_SUPER_WARRIOR:             SendMessage(hItem, CB_SETCURSEL, M_SHOT_SUPER_WARRIOR, 0);             break;
                case IDX_M_SHOT_VOLCANO_FIREBALL_1:        SendMessage(hItem, CB_SETCURSEL, M_SHOT_VOLCANO_FIREBALL_1, 0);        break;
                case IDX_M_SHOT_VOLCANO_FIREBALL_2:        SendMessage(hItem, CB_SETCURSEL, M_SHOT_VOLCANO_FIREBALL_2, 0);        break;
                }
                break;

			default: SendMessage(hItem, CB_SETCURSEL, -1, 0);
			}
			break;
		case T_CREATURE:
			SendMessage(hItem, CB_SETCURSEL, IDX_T_CREATURE, 0);
			DlgObjectSetCreatureList(hWnd);
			hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
			EnableWindow(hItem, true);
			switch(ThingSelected->Thing.Model)
			{
			case M_CREATURE_EAGLE: SendMessage(hItem, CB_SETCURSEL, IDX_M_CREATURE_EAGLE, 0); break;
			default: SendMessage(hItem, CB_SETCURSEL, -1, 0);
			}
			break;
		case T_VEHICLE:
			SendMessage(hItem, CB_SETCURSEL, IDX_T_VEHICLE, 0);
			DlgObjectSetVehicleList(hWnd);
			hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
			EnableWindow(hItem, true);
			switch(ThingSelected->Thing.Model)
			{
			case M_VEHICLE_BOAT_1:
			case M_VEHICLE_BOAT_2:    SendMessage(hItem, CB_SETCURSEL, IDX_M_VEHICLE_BOAT, 0); break;
			case M_VEHICLE_AIRSHIP_1:
			case M_VEHICLE_AIRSHIP_2: SendMessage(hItem, CB_SETCURSEL, IDX_M_VEHICLE_AIRSHIP, 0); break;
			}
			break;
        case T_SHOT:
            SendMessage(hItem, CB_SETCURSEL, IDX_T_SHOT, 0);
            DlgObjectSetShotList(hWnd);
            hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
            EnableWindow(hItem, true);
            switch (ThingSelected->Thing.Model)
            {
            case M_SHOT_STANDARD:                       SendMessage(hItem, CB_SETCURSEL, IDX_M_SHOT_STANDARD, 0);                      break;
            case M_SHOT_STANDARD_2:                     SendMessage(hItem, CB_SETCURSEL, IDX_M_SHOT_STANDARD_2, 0);                    break;
            case M_SHOT_STANDARD_3:                     SendMessage(hItem, CB_SETCURSEL, IDX_M_SHOT_STANDARD_3, 0);                    break;
            case M_SHOT_FIREBALL:                       SendMessage(hItem, CB_SETCURSEL, IDX_M_SHOT_FIREBALL, 0);                      break;
            case M_SHOT_LIGHTNING:                      SendMessage(hItem, CB_SETCURSEL, IDX_M_SHOT_LIGHTNING, 0);                     break;
            case M_SHOT_SUPER_WARRIOR:                  SendMessage(hItem, CB_SETCURSEL, IDX_M_SHOT_SUPER_WARRIOR, 0);                 break;
            case M_SHOT_VOLCANO_FIREBALL_1:             SendMessage(hItem, CB_SETCURSEL, IDX_M_SHOT_VOLCANO_FIREBALL_1, 0);            break;
            case M_SHOT_VOLCANO_FIREBALL_2:             SendMessage(hItem, CB_SETCURSEL, IDX_M_SHOT_VOLCANO_FIREBALL_2, 0);            break;
            }
            break;
		default:
			SendMessage(hItem, CB_SETCURSEL, -1, 0);
			hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
			SendMessage(hItem, CB_RESETCONTENT, 0, 0);
			EnableWindow(hItem, false);
		}
	}
	else
	{
		EnableWindow(GetDlgItem(hWnd, IDC_OBJECT_DELETE), false);
		EnableWindow(GetDlgItem(hWnd, IDC_OBJECT_OPTIONS), false);

		hItem = GetDlgItem(hWnd, IDC_OBJECT_OWNER);
		SendMessage(hItem, CB_SETCURSEL, -1, 0);
		EnableWindow(hItem, false);

		hItem = GetDlgItem(hWnd, IDC_OBJECT_TYPE);
		SendMessage(hItem, CB_SETCURSEL, -1, 0);
		EnableWindow(hItem, false);

		hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
		SendMessage(hItem, CB_RESETCONTENT, 0, 0);
		EnableWindow(hItem, false);
	}

	if(Things)
	{
		EnableWindow(GetDlgItem(hWnd, IDC_OBJECT_NEXT), /*(Things->Next == Things) ? false :*/ true);
		EnableWindow(GetDlgItem(hWnd, IDC_OBJECT_PREV), /*(Things->Next == Things) ? false :*/ true);
	}
	else
	{
		EnableWindow(GetDlgItem(hWnd, IDC_OBJECT_NEXT), false);
		EnableWindow(GetDlgItem(hWnd, IDC_OBJECT_PREV), false);
	}

	if(lock)
	{
		SendMessage(hWnd, WM_SETREDRAW, true, 0);
		InvalidateRect(hWnd, 0, false);
	}

_end:
	DlgDiscoveryUpdateInfo(hDlgDiscovery);
	DlgTriggerUpdateInfo(hDlgTrigger);
	DlgLinkUpdateInfo(hDlgLink);
	DlgListUpdateInfo(hDlgList);
}


void DlgObjectSetPersonList(HWND hWnd)
{
	if(!hWnd) return;

	//

	HWND hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
	if(!hItem) return;
	SendMessage(hItem, CB_RESETCONTENT, 0, 0);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_PERSON_WILD);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_PERSON_BRAVE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_PERSON_WARRIOR);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_PERSON_PRIEST);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_PERSON_SPY);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_PERSON_FIREWARRIOR);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_PERSON_SHAMAN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_PERSON_ANGELOFDEATH);
}


void DlgObjectSetBuildingList(HWND hWnd)
{
	if(!hWnd) return;

	//

	HWND hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
	if(!hItem) return;
	SendMessage(hItem, CB_RESETCONTENT, 0, 0);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_HUT1);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_HUT2);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_HUT3);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_TOWER);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_TEMPLE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_SPY_TRAIN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_WARRIOR_TRAIN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_FIREWARRIOR_TRAIN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_BOAT_HUT);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_AIRSHIP_HUT);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_KNOWLEDGE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_PRISON);
}


void DlgObjectSetSceneryList(HWND hWnd)
{
	if(!hWnd) return;

	//

	HWND hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
	if(!hItem) return;
	SendMessage(hItem, CB_RESETCONTENT, 0, 0);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SCENERY_TREE_1);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SCENERY_TREE_2);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SCENERY_TREE_3);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SCENERY_TREE_4);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SCENERY_TREE_5);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SCENERY_TREE_6);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SCENERY_PLANT_1);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SCENERY_PLANT_2);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SCENERY_STONE_HEAD);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SCENERY_WOOD_PILE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SCENERY_RS_PILLAR);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SCENERY_ROCK);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SCENERY_FIRE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SCENERY_FORBIDDEN);
}


void DlgObjectSetSpecialList(HWND hWnd)
{
	if(!hWnd) return;

	//

	HWND hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
	if(!hItem) return;
	SendMessage(hItem, CB_RESETCONTENT, 0, 0);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPECIAL_A);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPECIAL_B);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPECIAL_C);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPECIAL_D);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPECIAL_E);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPECIAL_F);
}


void DlgObjectSetGeneralList(HWND hWnd)
{
	if(!hWnd) return;

	//

	HWND hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
	if(!hItem) return;
	SendMessage(hItem, CB_RESETCONTENT, 0, 0);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_GENERAL_DISCOVERY);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_GENERAL_TRIGGER);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_GENERAL_BUILDING_ADD_ON);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_GENERAL_LIGHT);
}


void DlgObjectSetSpellList(HWND hWnd)
{
	if(!hWnd) return;

	//

	HWND hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
	if(!hItem) return;
	SendMessage(hItem, CB_RESETCONTENT, 0, 0);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_BURN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_BLAST);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_LIGHTNING);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_TORNADO);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_SWARM);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_INVISIBILITY);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_HYPNOTISM);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_FIRESTORM);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_GHOST_ARMY);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_ERODE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_SWAMP);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_LAND_BRIDGE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_ANGEL_OF_DEATH);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_EARTHQUAKE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_FLATTEN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_VOLCANO);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_CONVERT);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_ARMAGEDDON);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_MAGICAL_SHIELD);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_BLOODLUST);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_TELEPORT);
}


void DlgObjectSetEffectList(HWND hWnd)
{
	if(!hWnd) return;

	//

	HWND hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
	if(!hItem) return;
	SendMessage(hItem, CB_RESETCONTENT, 0, 0);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SIMPLE_BLAST);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SPRITE_CIRCLES);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SMOKE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_LIGHTNING_ELEM);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_BURN_CELL_OBSTACLES);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_FLATTEN_LAND);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SPHERE_EXPLODE_1);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_FIREBALL);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_FIRECLOUD);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_GHOST_ARMY);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_INVISIBILITY);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_VOLCANO);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_HYPNOTISM);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_LIGHTNING_BOLT);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SWAMP);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_ANGEL_OF_DEATH);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_WHIRLWIND);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_INSECT_PLAGUE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_FIRESTORM);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_EROSION);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_LAND_BRIDGE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_EARTHQUAKE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_FLY_THINGUMMY);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SPHERE_EXPLODE_AND_FIRE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_BIG_FIRE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_LIGHTNING);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_FLATTEN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_LAVA_FLOW);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_PURIFY_LAND);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_EXPLOSION_1);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_EXPLOSION_2);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_LAVA_SQUARE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_WW_ELEMENT);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_WW_DUST);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_HILL);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_VALLEY);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_PLACE_TREE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_RISE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_DIP);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_REIN_ROCK_DEBRIS);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_PLACE_SHAMAN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_PLACE_WILD);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_BLDG_SMOKE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_MUCH_SIMPLER_BLAST);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_TUMBLING_BRANCH);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SPARKLE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SMALL_SPARKLE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_EXPLOSION_3);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_ROCK_EXPLOSION);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_LAVA_GLOOP);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SPLASH);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SMOKE_CLOUD);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SMOKE_CLOUD_CONSTANT);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_FIREBALL_2);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_GROUND_SHOCKWAVE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_ORBITER);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_BIG_SPARKLE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_METEOR);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_CONVERT_WILD);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_BLDG_SMOKE_2_FULL);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_BLDG_SMOKE_2_PARTIAL);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_BLDG_DAMAGED_SMOKE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_DELETE_RS_PILLARS);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SPELL_BLAST);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_FIRESTORM_SMOKE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_REVEAL_FOG_AREA);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SHIELD);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_BOAT_HUT_REPAIR);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_REEDY_GRASS);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SWAMP_MIST);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_ARMAGEDDON);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_BLOODLUST);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_TELEPORT);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_ATLANTIS_SET);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_ATLANTIS_INVOKE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_STATUE_TO_AOD);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_FILL_ONE_SHOTS);
#if _ASCENSION
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_RESURRECT);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_BIG_SMOKE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_FIRESTORM_BALL);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_LAVA_FLOW_NEW);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_LAVA_FLOW_NEW_SHALLOW);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_LAVA_FLOW_NEW_DEEP);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_WATER);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_WATER_1X1);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_LAVA_FLOW_NEW_WHITE_SHALLOW);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_LAVA_FLOW_NEW_WHITE_DEEP);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_LAVA_FIXED);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SOUL);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_NEW_PLANT1);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_NEW_PLANT2);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_NEW_PLANT3);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_NEW_PLANT4);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_NEW_BRANCH);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_BONEPILE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SMALL_SKULL);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SMALL_SKULL2);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SMALL_SKULL3);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SKULL_THING);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_SKULL_THING_FIRE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_EFFECT_GHOST_SKULL);
#endif
}


void DlgObjectSetCreatureList(HWND hWnd)
{
	if(!hWnd) return;

	//

	HWND hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
	if(!hItem) return;
	SendMessage(hItem, CB_RESETCONTENT, 0, 0);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_CREATURE_EAGLE);
}


void DlgObjectSetVehicleList(HWND hWnd)
{
	if(!hWnd) return;

	//

	HWND hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
	if(!hItem) return;
	SendMessage(hItem, CB_RESETCONTENT, 0, 0);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_VEHICLE_BOAT);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_VEHICLE_AIRSHIP);
}

void DlgObjectSetShotList(HWND hWnd)
{
    if (!hWnd) return;

    //

    HWND hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
    if (!hItem) return;
    SendMessage(hItem, CB_RESETCONTENT, 0, 0);
    SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SHOT_STANDARD);
    SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SHOT_STANDARD_2);
    SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SHOT_STANDARD_3);
    SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SHOT_FIREBALL);
    SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SHOT_LIGHTNING);
    SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SHOT_SUPER_WARRIOR);
    SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SHOT_VOLCANO_FIREBALL_1);
    SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SHOT_VOLCANO_FIREBALL_2);
}

void DlgObjectSetDecorationList(HWND hWnd)
{
	if(!hWnd) return;
	HWND hItem = GetDlgItem(hWnd, IDC_OBJECT_MODEL);
	if(!hItem) return;
	SendMessage(hItem, CB_RESETCONTENT, 0, 0);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_BOOK);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_SHIELD);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_WELL);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_WINDMILL);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_WOODEN_HUT);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_B_TOWER);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_R_TOWER);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_Y_TOWER);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_G_TOWER);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_B_BOAT_HUT);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_R_BOAT_HUT);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_Y_BOAT_HUT);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_G_BOAT_HUT);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_B_AIRSHIP_HUT);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_R_AIRSHIP_HUT);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_Y_AIRSHIP_HUT);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_G_AIRSHIP_HUT);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_B_SPY_TRAIN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_R_SPY_TRAIN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_Y_SPY_TRAIN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_G_SPY_TRAIN);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_B_TEMPLE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_R_TEMPLE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_Y_TEMPLE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_G_TEMPLE);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_B_FIREWARRIOR_TRAIN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_R_FIREWARRIOR_TRAIN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_Y_FIREWARRIOR_TRAIN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_G_FIREWARRIOR_TRAIN);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_B_WARRIOR_TRAIN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_R_WARRIOR_TRAIN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_Y_WARRIOR_TRAIN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_G_WARRIOR_TRAIN);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_B_HUT1_A);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_B_HUT1_B);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_B_HUT1_C);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_R_HUT1_A);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_R_HUT1_B);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_R_HUT1_C);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_Y_HUT1_A);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_Y_HUT1_B);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_Y_HUT1_C);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_G_HUT1_A);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_G_HUT1_B);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_G_HUT1_C);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_B_HUT2_A);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_B_HUT2_B);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_B_HUT2_C);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_R_HUT2_A);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_R_HUT2_B);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_R_HUT2_C);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_Y_HUT2_A);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_Y_HUT2_B);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_Y_HUT2_C);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_G_HUT2_A);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_G_HUT2_B);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_G_HUT2_C);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_B_HUT3_A);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_B_HUT3_B);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_B_HUT3_C);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_R_HUT3_A);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_R_HUT3_B);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_R_HUT3_C);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_Y_HUT3_A);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_Y_HUT3_B);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_Y_HUT3_C);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_G_HUT3_A);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_G_HUT3_B);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_G_HUT3_C);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_BOAT);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_BALLOON);

	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_TOTEM_0);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_TOTEM_1);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_KNOWLEDGE_0);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_KNOWLEDGE_1);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_KNOWLEDGE_2);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_PRISON);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_OBELISK);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_GARGOYLE_0);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_GARGOYLE_1);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_GARGOYLE_2);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_PILLAR);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_AOD_0);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_AOD_1);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_STONEHEAD);


	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_TREE_1);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_TREE_2);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_TREE_3);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_TREE_4);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_TREE_5);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_DECORATION_TREE_6);
}

void DlgObjectNextObj()
{
	if(!Things) return;

	THING *t;

	if(ThingSelected)
	{
		t = ThingSelected->Next;
	}
	else
	{
		ThingSelected = Things;
		t = ThingSelected;
	}

	do
	{
		if(DlgFilterIsInFilter(t))
		{
			DlgObjectSelect(t);
			return;
		}
		t = t->Next;
	}
	while(t != ThingSelected);

	if(DlgFilterIsInFilter(ThingSelected))
		DlgObjectSelect(ThingSelected);
	else
		DlgObjectSelect(0);
}


void DlgObjectPrevObj()
{
	if(!Things) return;

	THING *t;

	if(ThingSelected)
	{
		t = ThingSelected->Prev;
	}
	else
	{
		ThingSelected = Things;
		t = ThingSelected;
	}

	do
	{
		if(DlgFilterIsInFilter(t))
		{
			DlgObjectSelect(t);
			return;
		}
		t = t->Prev;
	}
	while(t != ThingSelected);

	if(DlgFilterIsInFilter(ThingSelected))
		DlgObjectSelect(ThingSelected);
	else
		DlgObjectSelect(0);
}


THING* DlgObjectNewObj(float x, float z, UBYTE nType, UBYTE nModel, SBYTE nOwner)
{
	if(ObjectsCount >= MAX_THINGS) return nullptr;

	THING *t;
	t = new THING;

	if (nModel != 255)
	{
		memset(t, 0, sizeof(THING));
		t->Thing.Type = nType;
		t->Thing.Model = nModel;
		t->Thing.Owner = nOwner;
		t->x = (float)(int)x + 0.5f;
		t->z = (float)(int)z + 0.5f;
		t->Thing.PosX = ((int)x * 2) << 8;
		t->Thing.PosZ = ((int)z * 2) << 8;
	}
	else if (fQuickDuplicate && ThingSelected)
	{
		memcpy(t, ThingSelected, sizeof(THING));
		t->Idx = 0;
	}
	else if(ThingSelected && ((GetKeyState(VK_LSHIFT) & 0x80000000) || (GetKeyState(VK_RSHIFT) & 0x80000000)))
	{
		memcpy(t, ThingSelected, sizeof(THING));
		t->Idx = 0;
	}
	else
	{
		memset(t, 0, sizeof(THING));

		t->Thing.Type = T_PERSON;
		t->Thing.Model = M_PERSON_BRAVE;
		t->x = (float)(int)fEnginePosX + 0.5f;
		t->z = (float)(int)fEnginePosZ + 0.5f;
		t->Thing.PosX = ((int)fEnginePosX * 2) << 8;
		t->Thing.PosZ = ((int)fEnginePosZ * 2) << 8;
	}

	if (net.IsInitialized())
	{
		struct Packet *p = new Packet;
		p->wType = PACKETTYPE_CREATE_OBJECT;
		p->wData[0] = t->Thing.Type;
		p->wData[1] = t->Thing.Model;
		p->wData[2] = t->Thing.Owner;
		p->wData[3] = (WORD)(t->x - 0.5f);
		p->wData[4] = (WORD)(t->z - 0.5f);
		net.SendPacket(p);
		p->del();
	}

	LINK(Things, t);
	ObjectsCount++;
	DlgObjectSelect(t, false);
	EngineUpdateMiniMap();
	DlgInfoUpdate(hDlgInfo);
	DlgSetThingIndex(t);
	return t;
}


void DlgObjectDeleteObj()
{
	if(!ThingSelected) return;
	SaveThingsUndoState();
	THING *t = ThingSelected;
	ThingsIndices[ThingSelected->Idx] = 0;

	if (net.IsInitialized())
	{
		struct Packet *p = new Packet;
		p->wType = PACKETTYPE_DELETE_OBJECT;
		p->wData[0] = ThingSelected->Idx;
		net.SendPacket(p);
		p->del();
	}

	DlgObjectSelect(0);
	UNLINK(Things, t);
	DlgObjectUnlinkObj(t);
	delete t;
	ObjectsCount--;
	DlgObjectUpdateInfo(hDlgObject);
	EngineUpdateMiniMap();
	DlgInfoUpdate(hDlgInfo);
}


void DlgSetThingIndex(THING* pThing)
{
	for (int i = 1; i <= ObjectsCount; i++)
	{
		if (pThing->Idx == 0)
		{
			if (ThingsIndices[i] == 0)
			{
				pThing->Idx = i;
				ThingsIndices[i] = i;
			}
		}
		else
			break;
	}
}


THING* DlgObjectFindIdx(UWORD Idx)
{
	if(!Things) return 0;

	THING *t = Things;
	do
	{
		if(t->Idx == Idx) return t;
		t = t->Next;
	}
	while(t != Things);

	return 0;
}


void DlgObjectIdxToLink()
{
	if(!Things) return;

	THING *t = Things;

	do
	{
		if(t->Thing.Type == T_GENERAL && t->Thing.Model == M_GENERAL_TRIGGER)
		{
			for(int i = 0; i < 10; i++)
			{
				if(t->Thing.Trigger.ThingIdxs[i])
					t->Links[i] = DlgObjectFindIdx(t->Thing.Trigger.ThingIdxs[i]);
				else
					t->Links[i] = 0;
			}
		}
		t = t->Next;
	}
	while(t != Things);
}


void DlgObjectResetIdxs()
{
	if(!Things) return;

	THING *t = Things;

	UWORD Idx = 1;

	do
	{
		t->Idx = Idx;
		Idx++;
		t = t->Next;
	}
	while(t != Things);
}


void DlgObjectLinkToIdx()
{
	if(!Things) return;

	DlgObjectResetIdxs();

	THING *t = Things;

	do
	{
		if(t->Thing.Type == T_GENERAL && t->Thing.Model == M_GENERAL_TRIGGER)
		{
			for(int n = 0; n < 10; n++)
			{
				if(t->Links[n])
					t->Thing.Trigger.ThingIdxs[n] = t->Links[n]->Idx;
				else
					t->Thing.Trigger.ThingIdxs[n] = 0;
			}
		}
		t = t->Next;
	}
	while(t != Things);
}


void DlgObjectUnlinkObj(THING *a)
{
	if(!Things) return;

    THING *t = Things;

	do
	{
		for(int i = 0; i < 10; i++)
		{
			if(t->Links[i] == a) t->Links[i] = 0;
		}
		t = t->Next;
	}
	while(Things != t);
}


// -=-=- object filter dialog -=-=-

void DlgFilterToggle()
{
	if(hDlgFilter)
	{
		DestroyWindow(hDlgFilter);
		hDlgFilter = 0;
		DlgListUpdateInfo(hDlgList);
	}
	else
	{
		hDlgFilter = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_FILTER), hMainWnd, DlgFilterProc, 0);
		ShowWindow(hDlgFilter, SW_SHOW);
	}
}


int __stdcall DlgFilterProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		DlgFilterUpdate(hWnd);
		return 0;

	case WM_COMMAND:
		switch(wParam)
		{
		case IDC_FILTER_ALL:
			if(ObjectFilterFlags & OFF_ALL)
				ObjectFilterFlags &= ~OFF_ALL;
			else
				ObjectFilterFlags |= OFF_ALL;
			DlgFilterUpdate(hWnd);
			break;
		case IDC_FILTER_NEUTRAL:
			if(ObjectFilterFlags & OFF_NEUTRAL)
				ObjectFilterFlags &= ~OFF_NEUTRAL;
			else
				ObjectFilterFlags |= OFF_NEUTRAL;
			break;
		case IDC_FILTER_BLUE:
			if(ObjectFilterFlags & OFF_BLUE)
				ObjectFilterFlags &= ~OFF_BLUE;
			else
				ObjectFilterFlags |= OFF_BLUE;
			break;
		case IDC_FILTER_RED:
			if(ObjectFilterFlags & OFF_RED)
				ObjectFilterFlags &= ~OFF_RED;
			else
				ObjectFilterFlags |= OFF_RED;
			break;
		case IDC_FILTER_YELLOW:
			if(ObjectFilterFlags & OFF_YELLOW)
				ObjectFilterFlags &= ~OFF_YELLOW;
			else
				ObjectFilterFlags |= OFF_YELLOW;
			break;
		case IDC_FILTER_GREEN:
			if(ObjectFilterFlags & OFF_GREEN)
				ObjectFilterFlags &= ~OFF_GREEN;
			else
				ObjectFilterFlags |= OFF_GREEN;
			break;
		case IDC_FILTER_CYAN:
			if(ObjectFilterFlags & OFF_CYAN)
				ObjectFilterFlags &= ~OFF_CYAN;
			else
				ObjectFilterFlags |= OFF_CYAN;
			break;
		case IDC_FILTER_MAGENTA:
			if(ObjectFilterFlags & OFF_MAGENTA)
				ObjectFilterFlags &= ~OFF_MAGENTA;
			else
				ObjectFilterFlags |= OFF_MAGENTA;
			break;
		case IDC_FILTER_BLACK:
			if(ObjectFilterFlags & OFF_BLACK)
				ObjectFilterFlags &= ~OFF_BLACK;
			else
				ObjectFilterFlags |= OFF_BLACK;
			break;
		case IDC_FILTER_ORANGE:
			if(ObjectFilterFlags & OFF_ORANGE)
				ObjectFilterFlags &= ~OFF_ORANGE;
			else
				ObjectFilterFlags |= OFF_ORANGE;
			break;
		case IDC_FILTER_PERSON:
			if(IsDlgButtonChecked(hDlgFilter, IDC_FILTER_PERSON) == BST_CHECKED)
				ObjectFilterFlags |= OFF_PERSON;
			else
				ObjectFilterFlags &= ~OFF_PERSON;
			break;
		case IDC_FILTER_BUILDING:
			if(IsDlgButtonChecked(hDlgFilter, IDC_FILTER_BUILDING) == BST_CHECKED)
				ObjectFilterFlags |= OFF_BUILDING;
			else
				ObjectFilterFlags &= ~OFF_BUILDING;
			break;
		case IDC_FILTER_SCENERY:
			if(IsDlgButtonChecked(hDlgFilter, IDC_FILTER_SCENERY) == BST_CHECKED)
				ObjectFilterFlags |= OFF_SCENERY;
			else
				ObjectFilterFlags &= ~OFF_SCENERY;
			break;
		case IDC_FILTER_GENERAL:
			if(IsDlgButtonChecked(hDlgFilter, IDC_FILTER_GENERAL) == BST_CHECKED)
				ObjectFilterFlags |= OFF_GENERAL;
			else
				ObjectFilterFlags &= ~OFF_GENERAL;
			break;
		case IDC_FILTER_VEHICLE:
			if(IsDlgButtonChecked(hDlgFilter, IDC_FILTER_VEHICLE) == BST_CHECKED)
				ObjectFilterFlags |= OFF_VEHICLE;
			else
				ObjectFilterFlags &= ~OFF_VEHICLE;
			break;
		case IDC_FILTER_SPELL:
			if(IsDlgButtonChecked(hDlgFilter, IDC_FILTER_SPELL) == BST_CHECKED)
				ObjectFilterFlags |= OFF_SPELL;
			else
				ObjectFilterFlags &= ~OFF_SPELL;
			break;
		case IDC_FILTER_EFFECT:
			if(IsDlgButtonChecked(hDlgFilter, IDC_FILTER_EFFECT) == BST_CHECKED)
				ObjectFilterFlags |= OFF_EFFECT;
			else
				ObjectFilterFlags &= ~OFF_EFFECT;
			break;
        case IDC_FILTER_SHOT:
            if (IsDlgButtonChecked(hDlgFilter, IDC_FILTER_SHOT) == BST_CHECKED)
                ObjectFilterFlags |= OFF_SHOT;
            else
                ObjectFilterFlags &= ~OFF_SHOT;
            break;
		case IDC_FILTER_UNKNOW:
			if(IsDlgButtonChecked(hDlgFilter, IDC_FILTER_UNKNOW) == BST_CHECKED)
				ObjectFilterFlags |= OFF_UNKNOW;
			else
				ObjectFilterFlags &= ~OFF_UNKNOW;
			break;
		}
		return 0;

	case WM_CLOSE:
		DlgFilterToggle();
		return 0;
	}

	return 0;
}


void DlgFilterUpdate(HWND hWnd)
{
	if(!hWnd) return;

	//

	if(ObjectFilterFlags & OFF_NEUTRAL)
		CheckDlgButton(hWnd, IDC_FILTER_NEUTRAL, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_FILTER_NEUTRAL, BST_UNCHECKED);

	if(ObjectFilterFlags & OFF_BLUE)
		CheckDlgButton(hWnd, IDC_FILTER_BLUE, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_FILTER_BLUE, BST_UNCHECKED);

	if(ObjectFilterFlags & OFF_RED)
		CheckDlgButton(hWnd, IDC_FILTER_RED, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_FILTER_RED, BST_UNCHECKED);

	if(ObjectFilterFlags & OFF_YELLOW)
		CheckDlgButton(hWnd, IDC_FILTER_YELLOW, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_FILTER_YELLOW, BST_UNCHECKED);

	if(ObjectFilterFlags & OFF_GREEN)
		CheckDlgButton(hWnd, IDC_FILTER_GREEN, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_FILTER_GREEN, BST_UNCHECKED);
  
  if(ObjectFilterFlags & OFF_CYAN)
		CheckDlgButton(hWnd, IDC_FILTER_CYAN, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_FILTER_CYAN, BST_UNCHECKED);

	if(ObjectFilterFlags & OFF_MAGENTA)
		CheckDlgButton(hWnd, IDC_FILTER_MAGENTA, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_FILTER_MAGENTA, BST_UNCHECKED);

	if(ObjectFilterFlags & OFF_BLACK)
		CheckDlgButton(hWnd, IDC_FILTER_BLACK, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_FILTER_BLACK, BST_UNCHECKED);

	if(ObjectFilterFlags & OFF_ORANGE)
		CheckDlgButton(hWnd, IDC_FILTER_ORANGE, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_FILTER_ORANGE, BST_UNCHECKED);

	if(ObjectFilterFlags & OFF_PERSON)
		CheckDlgButton(hWnd, IDC_FILTER_PERSON, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_FILTER_PERSON, BST_UNCHECKED);

	if(ObjectFilterFlags & OFF_BUILDING)
		CheckDlgButton(hWnd, IDC_FILTER_BUILDING, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_FILTER_BUILDING, BST_UNCHECKED);

	if(ObjectFilterFlags & OFF_SCENERY)
		CheckDlgButton(hWnd, IDC_FILTER_SCENERY, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_FILTER_SCENERY, BST_UNCHECKED);

	if(ObjectFilterFlags & OFF_GENERAL)
		CheckDlgButton(hWnd, IDC_FILTER_GENERAL, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_FILTER_GENERAL, BST_UNCHECKED);

	if(ObjectFilterFlags & OFF_VEHICLE)
		CheckDlgButton(hWnd, IDC_FILTER_VEHICLE, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_FILTER_VEHICLE, BST_UNCHECKED);

	if(ObjectFilterFlags & OFF_SPELL)
		CheckDlgButton(hWnd, IDC_FILTER_SPELL, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_FILTER_SPELL, BST_UNCHECKED);

	if(ObjectFilterFlags & OFF_EFFECT)
		CheckDlgButton(hWnd, IDC_FILTER_EFFECT, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_FILTER_EFFECT, BST_UNCHECKED);

    if (ObjectFilterFlags & OFF_SHOT)
        CheckDlgButton(hWnd, IDC_FILTER_SHOT, BST_CHECKED);
    else
        CheckDlgButton(hWnd, IDC_FILTER_SHOT, BST_UNCHECKED);

	if(ObjectFilterFlags & OFF_UNKNOW)
		CheckDlgButton(hWnd, IDC_FILTER_UNKNOW, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_FILTER_UNKNOW, BST_UNCHECKED);
}


bool DlgFilterIsInFilter(THING *t)
{
	if(!t) return false;

	switch(t->Thing.Owner)
	{
	case OWNER_BLUE:
		if(!(ObjectFilterFlags & OFF_BLUE)) return false;
		break;
	case OWNER_RED:
		if(!(ObjectFilterFlags & OFF_RED)) return false;
		break;
	case OWNER_YELLOW:
		if(!(ObjectFilterFlags & OFF_YELLOW)) return false;
		break;
	case OWNER_GREEN:
		if(!(ObjectFilterFlags & OFF_GREEN)) return false;
		break;
  case OWNER_CYAN:
		if(!(ObjectFilterFlags & OFF_CYAN)) return false;
		break;
	case OWNER_MAGENTA:
		if(!(ObjectFilterFlags & OFF_MAGENTA)) return false;
		break;
	case OWNER_BLACK:
		if(!(ObjectFilterFlags & OFF_BLACK)) return false;
		break;
	case OWNER_ORANGE:
		if(!(ObjectFilterFlags & OFF_ORANGE)) return false;
		break;
    case OWNER_HOSTBOT:
        if (!(ObjectFilterFlags & OFF_HOSTBOT)) return false;
        break;
	default:
		if(!(ObjectFilterFlags & OFF_NEUTRAL)) return false;
	}

	switch(t->Thing.Type)
	{
	case T_PERSON:
		if(ObjectFilterFlags & OFF_PERSON) return true;
		break;
	case T_BUILDING:
		if(ObjectFilterFlags & OFF_BUILDING) return true;
		break;
	case T_SCENERY:
		if(ObjectFilterFlags & OFF_SCENERY) return true;
		break;
	case T_GENERAL:
		if(ObjectFilterFlags & OFF_GENERAL) return true;
		break;
	case T_VEHICLE:
		if(ObjectFilterFlags & OFF_VEHICLE) return true;
		break;
	case T_SPELL:
		if(ObjectFilterFlags & OFF_SPELL) return true;
		break;
	case T_EFFECT:
		if(ObjectFilterFlags & OFF_EFFECT) return true;
		break;
    case T_SHOT:
        if (ObjectFilterFlags & OFF_SHOT) return true;
        break;
	default:
		if(ObjectFilterFlags & OFF_UNKNOW) return true;
	}

	return false;
}


// -=-=- object discovery dialog -=-=-

void DlgDiscoveryToggle()
{
	if(hDlgDiscovery)
	{
		DestroyWindow(hDlgDiscovery);
		hDlgDiscovery = 0;
	}
	else
	{
		hDlgDiscovery = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DISCOVERY), hMainWnd, DlgDiscoveryProc, 0);
		ShowWindow(hDlgDiscovery, SW_SHOW);
	}
}


int __stdcall DlgDiscoveryProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool bNetwork = false;
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND hItem;

			hItem = GetDlgItem(hWnd, IDC_DISCOVERY_AVAILABILITY);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_AVAILABILITY_PERMANENT);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_AVAILABILITY_LEVEL);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_AVAILABILITY_ONCE);

			hItem = GetDlgItem(hWnd, IDC_DISCOVERY_TYPE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MANA);

			SendDlgItemMessage(hWnd, IDC_DISCOVERY_MANA, EM_LIMITTEXT, 8, 0);

			DlgDiscoveryUpdateInfo(hWnd, false);
		}
		return 0;

	case WM_COMMAND:
		if(!ThingSelected || ThingSelected->Thing.Type != T_GENERAL || ThingSelected->Thing.Model != M_GENERAL_DISCOVERY) return 0;
		switch(LOWORD(wParam))
		{
		case IDC_DISCOVERY_AVAILABILITY:
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				switch(SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))
				{
				case IDX_DISCOVERY_AVAILABILITY_PERMANENT: ThingSelected->Thing.General.AvailabilityType = AVAILABILITY_PERMANENT; break;
				case IDX_DISCOVERY_AVAILABILITY_LEVEL:     ThingSelected->Thing.General.AvailabilityType = AVAILABILITY_LEVEL;     break;
				case IDX_DISCOVERY_AVAILABILITY_ONCE:      ThingSelected->Thing.General.AvailabilityType = AVAILABILITY_ONCE;      break;
				}
				bNetwork = true;
			}
			break;
		case IDC_DISCOVERY_TYPE:
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				switch(SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))
				{
				case IDX_DISCOVERY_T_BUILDING:
					ThingSelected->Thing.General.DiscoveryType = T_BUILDING;
					ThingSelected->Thing.General.DiscoveryModel = 1;
					break;
				case IDX_DISCOVERY_T_SPELL:
					ThingSelected->Thing.General.DiscoveryType = T_SPELL;
					ThingSelected->Thing.General.DiscoveryModel = 1;
					break;
				case IDX_DISCOVERY_T_MANA:
					ThingSelected->Thing.General.DiscoveryType = T_GENERAL;
					ThingSelected->Thing.General.DiscoveryModel = M_GENERAL_DEBUG_FLAG;
					ThingSelected->Thing.General.ManaAmt = 0;
					break;
				}
				bNetwork = true;
				DlgDiscoveryUpdateInfo(hDlgDiscovery);
			}
			break;
		case IDC_DISCOVERY_MODEL:
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				switch(ThingSelected->Thing.General.DiscoveryType)
				{
				case T_BUILDING:
					switch(SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))
					{
					case IDX_DISCOVERY_M_BUILDING_HUT1:              ThingSelected->Thing.General.DiscoveryModel = M_BUILDING_HUT1;              break;
					case IDX_DISCOVERY_M_BUILDING_TOWER:             ThingSelected->Thing.General.DiscoveryModel = M_BUILDING_TOWER;             break;
					case IDX_DISCOVERY_M_BUILDING_TEMPLE:            ThingSelected->Thing.General.DiscoveryModel = M_BUILDING_TEMPLE;            break;
					case IDX_DISCOVERY_M_BUILDING_SPY_TRAIN:         ThingSelected->Thing.General.DiscoveryModel = M_BUILDING_SPY_TRAIN;         break;
					case IDX_DISCOVERY_M_BUILDING_WARRIOR_TRAIN:     ThingSelected->Thing.General.DiscoveryModel = M_BUILDING_WARRIOR_TRAIN;     break;
					case IDX_DISCOVERY_M_BUILDING_FIREWARRIOR_TRAIN: ThingSelected->Thing.General.DiscoveryModel = M_BUILDING_FIREWARRIOR_TRAIN; break;
					case IDX_DISCOVERY_M_BUILDING_BOAT_HUT:          ThingSelected->Thing.General.DiscoveryModel = M_BUILDING_BOAT_HUT_1;        break;
					case IDX_DISCOVERY_M_BUILDING_AIRSHIP_HUT:       ThingSelected->Thing.General.DiscoveryModel = M_BUILDING_AIRSHIP_HUT_1;     break;
					}
					break;
				case T_SPELL:
					switch(SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))
					{
					case IDX_DISCOVERY_M_SPELL_BLAST:          ThingSelected->Thing.General.DiscoveryModel = M_SPELL_BLAST;          break;
					case IDX_DISCOVERY_M_SPELL_LIGHTNING:      ThingSelected->Thing.General.DiscoveryModel = M_SPELL_LIGHTNING;      break;
					case IDX_DISCOVERY_M_SPELL_TORNADO:        ThingSelected->Thing.General.DiscoveryModel = M_SPELL_TORNADO;        break;
					case IDX_DISCOVERY_M_SPELL_SWARM:          ThingSelected->Thing.General.DiscoveryModel = M_SPELL_SWARM;          break;
					case IDX_DISCOVERY_M_SPELL_INVISIBILITY:   ThingSelected->Thing.General.DiscoveryModel = M_SPELL_INVISIBILITY;   break;
					case IDX_DISCOVERY_M_SPELL_HYPNOTISM:      ThingSelected->Thing.General.DiscoveryModel = M_SPELL_HYPNOTISM;      break;
					case IDX_DISCOVERY_M_SPELL_FIRESTORM:      ThingSelected->Thing.General.DiscoveryModel = M_SPELL_FIRESTORM;      break;
					case IDX_DISCOVERY_M_SPELL_GHOST_ARMY:     ThingSelected->Thing.General.DiscoveryModel = M_SPELL_GHOST_ARMY;     break;
					case IDX_DISCOVERY_M_SPELL_ERODE:          ThingSelected->Thing.General.DiscoveryModel = M_SPELL_ERODE;          break;
					case IDX_DISCOVERY_M_SPELL_SWAMP:          ThingSelected->Thing.General.DiscoveryModel = M_SPELL_SWAMP;          break;
					case IDX_DISCOVERY_M_SPELL_LAND_BRIDGE:    ThingSelected->Thing.General.DiscoveryModel = M_SPELL_LAND_BRIDGE;    break;
					case IDX_DISCOVERY_M_SPELL_ANGEL_OF_DEATH: ThingSelected->Thing.General.DiscoveryModel = M_SPELL_ANGEL_OF_DEATH; break;
					case IDX_DISCOVERY_M_SPELL_EARTHQUAKE:     ThingSelected->Thing.General.DiscoveryModel = M_SPELL_EARTHQUAKE;     break;
					case IDX_DISCOVERY_M_SPELL_FLATTEN:        ThingSelected->Thing.General.DiscoveryModel = M_SPELL_FLATTEN;        break;
					case IDX_DISCOVERY_M_SPELL_VOLCANO:        ThingSelected->Thing.General.DiscoveryModel = M_SPELL_VOLCANO;        break;
					case IDX_DISCOVERY_M_SPELL_CONVERT:        ThingSelected->Thing.General.DiscoveryModel = M_SPELL_CONVERT;        break;
					case IDX_DISCOVERY_M_SPELL_ARMAGEDDON:     ThingSelected->Thing.General.DiscoveryModel = M_SPELL_ARMAGEDDON;     break;
					case IDX_DISCOVERY_M_SPELL_MAGICAL_SHIELD: ThingSelected->Thing.General.DiscoveryModel = M_SPELL_MAGICAL_SHIELD; break;
					case IDX_DISCOVERY_M_SPELL_BLOODLUST:      ThingSelected->Thing.General.DiscoveryModel = M_SPELL_BLOODLUST;      break;
					case IDX_DISCOVERY_M_SPELL_TELEPORT:       ThingSelected->Thing.General.DiscoveryModel = M_SPELL_TELEPORT;       break;
					}
					break;
				}
				bNetwork = true;
				DlgDiscoveryUpdateInfo(hDlgDiscovery);
			}
		case IDC_DISCOVERY_MANA:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				{
					GetWindowText((HWND)lParam, str, sizeof(str));
					int i;
					if(GetInt(str, &i))
					{
						if(i < 0) i = 0;
						else if(i > MAX_MANA_VALUE) i = MAX_MANA_VALUE;
						ThingSelected->Thing.General.ManaAmt = i;
					}
				}
				break;
			case EN_KILLFOCUS:
				sprintf(str, "%d", ThingSelected->Thing.General.ManaAmt);
				SendMessage((HWND)lParam, WM_SETTEXT, 0, (LPARAM)str);
				if (net.IsInitialized())
				{
					char szPacket[100];
					sprintf(szPacket, "msg %d %d %d", PACKETTYPEMSG_DISCOVERY_MANA, ThingSelected->Idx, ThingSelected->Thing.General.ManaAmt);
					net.SendPacket(szPacket);
				}
				break;
			}
			break;
		}

		if (net.IsInitialized() && bNetwork)
		{
			struct Packet *p = new Packet;
			p->wType = PACKETTYPE_DISCOVERY_OPTIONS;
			p->wData[0] = ThingSelected->Idx;
			p->wData[1] = ThingSelected->Thing.General.AvailabilityType;
			p->wData[2] = ThingSelected->Thing.General.DiscoveryType;
			p->wData[3] = ThingSelected->Thing.General.DiscoveryModel;
			net.SendPacket(p);
			p->del();
		}
		return 0;

	case WM_CLOSE:
		DlgDiscoveryToggle();
		return 0;
	}

	return 0;
}


void DlgDiscoveryUpdateInfo(HWND hWnd, bool lock)
{
	if(!hWnd) return;

	//

	if(lock) SendMessage(hWnd, WM_SETREDRAW, false, 0);

	HWND hItem;
	if(ThingSelected && ThingSelected->Thing.Type == T_GENERAL && ThingSelected->Thing.Model == M_GENERAL_DISCOVERY)
	{
		hItem = GetDlgItem(hWnd, IDC_DISCOVERY_AVAILABILITY);
		EnableWindow(hItem, true);
		switch(ThingSelected->Thing.General.AvailabilityType)
		{
		case AVAILABILITY_PERMANENT: SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_AVAILABILITY_PERMANENT, 0); break;
		case AVAILABILITY_LEVEL:     SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_AVAILABILITY_LEVEL, 0); break;
		case AVAILABILITY_ONCE:      SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_AVAILABILITY_ONCE, 0); break;
		default: SendMessage(hItem, CB_SETCURSEL, -1, 0);
		}
		hItem = GetDlgItem(hWnd, IDC_DISCOVERY_TYPE);
		EnableWindow(hItem, true);
		switch(ThingSelected->Thing.General.DiscoveryType)
		{
		case T_BUILDING:
			SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_T_BUILDING, 0);

			hItem = GetDlgItem(hWnd, IDC_DISCOVERY_MANA);
			SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)"");
			EnableWindow(hItem, false);

			DlgDiscoverySetBuildingList(hWnd);
			hItem = GetDlgItem(hWnd, IDC_DISCOVERY_MODEL);
			EnableWindow(hItem, true);

			switch(ThingSelected->Thing.General.DiscoveryModel)
			{
			case M_BUILDING_HUT1:              SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_BUILDING_HUT1, 0);              break;
			case M_BUILDING_TOWER:             SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_BUILDING_TOWER, 0);             break;
			case M_BUILDING_TEMPLE:            SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_BUILDING_TEMPLE, 0);            break;
			case M_BUILDING_SPY_TRAIN:         SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_BUILDING_SPY_TRAIN, 0);         break;
			case M_BUILDING_WARRIOR_TRAIN:     SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_BUILDING_WARRIOR_TRAIN, 0);     break;
			case M_BUILDING_FIREWARRIOR_TRAIN: SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_BUILDING_FIREWARRIOR_TRAIN, 0); break;
			case M_BUILDING_BOAT_HUT_1:
			case M_BUILDING_BOAT_HUT_2:        SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_BUILDING_BOAT_HUT, 0);          break;
			case M_BUILDING_AIRSHIP_HUT_1:
			case M_BUILDING_AIRSHIP_HUT_2:     SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_BUILDING_AIRSHIP_HUT, 0);       break;
			default: SendMessage(hItem, CB_SETCURSEL, -1, 0);
			}
			break;
		case T_SPELL:
			SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_T_SPELL, 0);

			hItem = GetDlgItem(hWnd, IDC_DISCOVERY_MANA);
			SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)"");
			EnableWindow(hItem, false);

			DlgDiscoverySetSpellList(hWnd);
			hItem = GetDlgItem(hWnd, IDC_DISCOVERY_MODEL);
			EnableWindow(hItem, true);

			switch(ThingSelected->Thing.General.DiscoveryModel)
			{
			case M_SPELL_BLAST:          SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_BLAST, 0);          break;
			case M_SPELL_LIGHTNING:      SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_LIGHTNING, 0);      break;
			case M_SPELL_TORNADO:        SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_TORNADO, 0);        break;
			case M_SPELL_SWARM:          SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_SWARM, 0);          break;
			case M_SPELL_INVISIBILITY:   SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_INVISIBILITY, 0);   break;
			case M_SPELL_HYPNOTISM:      SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_HYPNOTISM, 0);      break;
			case M_SPELL_FIRESTORM:      SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_FIRESTORM, 0);      break;
			case M_SPELL_GHOST_ARMY:     SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_GHOST_ARMY, 0);     break;
			case M_SPELL_ERODE:          SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_ERODE, 0);          break;
			case M_SPELL_SWAMP:          SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_SWAMP, 0);          break;
			case M_SPELL_LAND_BRIDGE:    SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_LAND_BRIDGE, 0);    break;
			case M_SPELL_ANGEL_OF_DEATH: SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_ANGEL_OF_DEATH, 0); break;
			case M_SPELL_EARTHQUAKE:     SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_EARTHQUAKE, 0);     break;
			case M_SPELL_FLATTEN:        SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_FLATTEN, 0);        break;
			case M_SPELL_VOLCANO:        SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_VOLCANO, 0);        break;
			case M_SPELL_CONVERT:        SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_CONVERT, 0);        break;
			case M_SPELL_ARMAGEDDON:     SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_ARMAGEDDON, 0);     break;
			case M_SPELL_MAGICAL_SHIELD: SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_MAGICAL_SHIELD, 0); break;
			case M_SPELL_BLOODLUST:      SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_BLOODLUST, 0);      break;
			case M_SPELL_TELEPORT:       SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_M_SPELL_TELEPORT, 0);       break;
			default: SendMessage(hItem, CB_SETCURSEL, -1, 0);
			}
			break;
		case T_GENERAL:
			SendMessage(hItem, CB_SETCURSEL, IDX_DISCOVERY_T_MANA, 0);

			hItem = GetDlgItem(hWnd, IDC_DISCOVERY_MODEL);
			SendMessage(hItem, CB_RESETCONTENT, 0, 0);
			EnableWindow(hItem, false);

			hItem = GetDlgItem(hWnd, IDC_DISCOVERY_MANA);
			EnableWindow(hItem, true);
			sprintf(str, "%d", ThingSelected->Thing.General.ManaAmt);
			SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)str);
			break;
		default:
			SendMessage(hItem, CB_SETCURSEL, -1, 0);

			hItem = GetDlgItem(hWnd, IDC_DISCOVERY_MANA);
			SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)"");
			EnableWindow(hItem, false);

			hItem = GetDlgItem(hWnd, IDC_DISCOVERY_MODEL);
			SendMessage(hItem, CB_RESETCONTENT, 0, 0);
			EnableWindow(hItem, false);
		}
	}
	else
	{
		hItem = GetDlgItem(hWnd, IDC_DISCOVERY_AVAILABILITY);
		SendMessage(hItem, CB_SETCURSEL, -1, 0);
		EnableWindow(hItem, false);

		hItem = GetDlgItem(hWnd, IDC_DISCOVERY_TYPE);
		SendMessage(hItem, CB_SETCURSEL, -1, 0);
		EnableWindow(hItem, false);

		hItem = GetDlgItem(hWnd, IDC_DISCOVERY_MODEL);
		SendMessage(hItem, CB_RESETCONTENT, 0, 0);
		EnableWindow(hItem, false);

		hItem = GetDlgItem(hWnd, IDC_DISCOVERY_MANA);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)"");
		EnableWindow(hItem, false);
	}

	if(lock)
	{
		SendMessage(hWnd, WM_SETREDRAW, true, 0);
		InvalidateRect(hWnd, 0, false);
	}
}


void DlgDiscoverySetBuildingList(HWND hWnd)
{
	if(!hWnd) return;

	//

	HWND hItem = GetDlgItem(hWnd, IDC_DISCOVERY_MODEL);
	if(!hItem) return;
	SendMessage(hItem, CB_RESETCONTENT, 0, 0);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_HUT1);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_TOWER);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_TEMPLE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_SPY_TRAIN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_WARRIOR_TRAIN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_FIREWARRIOR_TRAIN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_BOAT_HUT);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_BUILDING_AIRSHIP_HUT);
}


void DlgDiscoverySetSpellList(HWND hWnd)
{
	if(!hWnd) return;

	//

	HWND hItem = GetDlgItem(hWnd, IDC_DISCOVERY_MODEL);
	if(!hItem) return;
	SendMessage(hItem, CB_RESETCONTENT, 0, 0);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_BLAST);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_LIGHTNING);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_TORNADO);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_SWARM);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_INVISIBILITY);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_HYPNOTISM);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_FIRESTORM);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_GHOST_ARMY);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_ERODE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_SWAMP);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_LAND_BRIDGE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_ANGEL_OF_DEATH);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_EARTHQUAKE);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_FLATTEN);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_VOLCANO);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_CONVERT);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_ARMAGEDDON);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_MAGICAL_SHIELD);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_BLOODLUST);
	SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_SPELL_TELEPORT);
}


// -=-=- trigger dialog -=-=-

void DlgTriggerToggle()
{
	if(hDlgTrigger)
	{
		DestroyWindow(hDlgTrigger);
		hDlgTrigger = 0;
	}
	else
	{
		hDlgTrigger = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_TRIGGER), hMainWnd, DlgTriggerProc, 0);
		ShowWindow(hDlgTrigger, SW_SHOW);
	}
}


int __stdcall DlgTriggerProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	bool bNetwork = false;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND hItem = GetDlgItem(hWnd, IDC_TRIGGER_TYPE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_TRIGGER_TYPE_PROXIMITY);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_TRIGGER_TYPE_TIMED);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_TRIGGER_TYPE_PLAYER_DEATH);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_TRIGGER_TYPE_SHAMAN_PROXIMITY);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_TRIGGER_TYPE_LIBRARY);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_TRIGGER_TYPE_SHAMAN_AOD);

			SendDlgItemMessage(hWnd, IDC_TRIGGER_TYPE, EM_LIMITTEXT, 8, 0);
			SendDlgItemMessage(hWnd, IDC_TRIGGER_CELLRADIUS, EM_LIMITTEXT, 8, 0);
			SendDlgItemMessage(hWnd, IDC_TRIGGER_OCCURENCES, EM_LIMITTEXT, 8, 0);
			SendDlgItemMessage(hWnd, IDC_TRIGGER_COUNT, EM_LIMITTEXT, 8, 0);
			SendDlgItemMessage(hWnd, IDC_TRIGGER_PRAY, EM_LIMITTEXT, 8, 0);
			SendDlgItemMessage(hWnd, IDC_TRIGGER_INACTIVETIME, EM_LIMITTEXT, 8, 0);

			DlgTriggerUpdateInfo(hWnd, false);
		}
		return 0;

	case WM_CLOSE:
		DlgTriggerToggle();
		return 0;

	case WM_COMMAND:
		if(wParam == IDC_TRIGGER_LINKS)
		{
			ThingLink = ThingSelected;
			DlgLinkToggle();
			break;
		}

		if(!ThingSelected || ThingSelected->Thing.Type != T_GENERAL || ThingSelected->Thing.Model != M_GENERAL_TRIGGER) return 0;

		switch(LOWORD(wParam))
		{
		case IDC_TRIGGER_TYPE:
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				switch(SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))
				{
				case IDX_TRIGGER_TYPE_PROXIMITY:
					ThingSelected->Thing.Trigger.TriggerType = TRIGGER_TYPE_PROXIMITY;
					break;
				case IDX_TRIGGER_TYPE_TIMED:
					ThingSelected->Thing.Trigger.TriggerType = TRIGGER_TYPE_TIMED;
					break;
				case IDX_TRIGGER_TYPE_PLAYER_DEATH:
					ThingSelected->Thing.Trigger.TriggerType = TRIGGER_TYPE_PLAYER_DEATH;
					ThingSelected->Thing.Trigger.CellRadius = 0;
					ThingSelected->Thing.Trigger.PrayTime = 0;
					ThingSelected->Thing.Trigger.InactiveTime = 0;
					ThingSelected->Thing.Trigger.NumOccurences = 1;
					DlgTriggerUpdateInfo(hDlgTrigger);
					break;
				case IDX_TRIGGER_TYPE_SHAMAN_PROXIMITY:
					ThingSelected->Thing.Trigger.TriggerType = TRIGGER_TYPE_SHAMAN_PROXIMITY;
					break;
				case IDX_TRIGGER_TYPE_LIBRARY:
					ThingSelected->Thing.Trigger.TriggerType = TRIGGER_TYPE_LIBRARY;
					break;
				case IDX_TRIGGER_TYPE_SHAMAN_AOD:
					ThingSelected->Thing.Trigger.TriggerType = TRIGGER_TYPE_SHAMAN_AOD;
					break;
				}
				bNetwork = true;
			}
			break;

		case IDC_TRIGGER_CELLRADIUS:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				{
					GetWindowText((HWND)lParam, str, sizeof(str));
					int i;
					if(GetInt(str, &i))
					{
						if(i < 0) i = 0;
						//else if(i > 4) i = 4;
						ThingSelected->Thing.Trigger.CellRadius = i;
					}
				}
				break;
			case EN_KILLFOCUS:
				sprintf(str, "%d", ThingSelected->Thing.Trigger.CellRadius);
				SendMessage((HWND)lParam, WM_SETTEXT, 0, (LPARAM)str);
				bNetwork = true;
				break;
			}
			break;

		case IDC_TRIGGER_OCCURENCES:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				{
					GetWindowText((HWND)lParam, str, sizeof(str));
					int i;
					if(GetInt(str, &i))
					{
						if(i < -1) i = -1;
						else if(i > 120) i = 120;
						ThingSelected->Thing.Trigger.NumOccurences = i;
					}
				}
				break;
			case EN_KILLFOCUS:
				sprintf(str, "%d", ThingSelected->Thing.Trigger.NumOccurences);
				SendMessage((HWND)lParam, WM_SETTEXT, 0, (LPARAM)str);
				bNetwork = true;
				break;
			}
			break;

		case IDC_TRIGGER_COUNT:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				{
					GetWindowText((HWND)lParam, str, sizeof(str));
					int i;
					if(GetInt(str, &i))
					{
						if(i < 0) i = 0;
						else if(i > 32000) i = 32000;
						ThingSelected->Thing.Trigger.TriggerCount = i;
					}
				}
				break;
			case EN_KILLFOCUS:
				sprintf(str, "%d", ThingSelected->Thing.Trigger.TriggerCount);
				SendMessage((HWND)lParam, WM_SETTEXT, 0, (LPARAM)str);
				bNetwork = true;
				break;
			}
			break;

		case IDC_TRIGGER_PRAY:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				{
					GetWindowText((HWND)lParam, str, sizeof(str));
					int i;
					if(GetInt(str, &i))
					{
						if(i < 0) i = 0;
						else if(i > 1000) i = 1000;
						ThingSelected->Thing.Trigger.PrayTime = i;
					}
				}
				break;
			case EN_KILLFOCUS:
				sprintf(str, "%d", ThingSelected->Thing.Trigger.PrayTime);
				SendMessage((HWND)lParam, WM_SETTEXT, 0, (LPARAM)str);
				bNetwork = true;
				break;
			}
			break;

		case IDC_TRIGGER_INACTIVETIME:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				{
					GetWindowText((HWND)lParam, str, sizeof(str));
					int i;
					if(GetInt(str, &i))
					{
						if(i < 0) i = 0;
						else if(i > 1000) i = 1000;
						ThingSelected->Thing.Trigger.InactiveTime = i;
					}
				}
				break;
			case EN_KILLFOCUS:
				sprintf(str, "%d", ThingSelected->Thing.Trigger.InactiveTime);
				SendMessage((HWND)lParam, WM_SETTEXT, 0, (LPARAM)str);
				bNetwork = true;
				break;
			}
			break;

		case IDC_TRIGGER_STARTINACTIVE:
			if(HIWORD(wParam) == BN_CLICKED) ThingSelected->Thing.Trigger.StartInactive = (IsDlgButtonChecked(hWnd, IDC_TRIGGER_STARTINACTIVE) == BST_CHECKED) ? 1 : 0;
			bNetwork = true;
			break;

		case IDC_TRIGGER_CREATEOWNED:
			if(HIWORD(wParam) == BN_CLICKED) ThingSelected->Thing.Trigger.CreatePlayerOwned = (IsDlgButtonChecked(hWnd, IDC_TRIGGER_CREATEOWNED) == BST_CHECKED) ? 1 : 0;
			bNetwork = true;
			break;
		}

		if (net.IsInitialized() && bNetwork)
		{
			struct Packet *p = new Packet;
			p->wType = PACKETTYPE_TRIGGER_OPTIONS;
			p->wData[0] = ThingSelected->Idx;
			p->wData[1] = ThingSelected->Thing.Trigger.TriggerType;
			p->wData[2] = ThingSelected->Thing.Trigger.CellRadius;
			p->wData[3] = ThingSelected->Thing.Trigger.NumOccurences;
			p->wData[4] = ThingSelected->Thing.Trigger.TriggerCount;
			p->wData[5] = ThingSelected->Thing.Trigger.PrayTime;
			p->wData[6] = ThingSelected->Thing.Trigger.InactiveTime;
			p->wData[7] = ThingSelected->Thing.Trigger.StartInactive;
			p->wData[8] = ThingSelected->Thing.Trigger.CreatePlayerOwned;
			net.SendPacket(p);
			p->del();
		}

		return 0;
	}

	return 0;
}


void DlgTriggerUpdateInfo(HWND hWnd, bool lock)
{
	if(!hWnd) return;

	if(lock) SendMessage(hWnd, WM_SETREDRAW, false, 0);

	HWND hItem;
	if(ThingSelected && ThingSelected->Thing.Type == T_GENERAL && ThingSelected->Thing.Model == M_GENERAL_TRIGGER)
	{
		hItem = GetDlgItem(hWnd, IDC_TRIGGER_TYPE);
		EnableWindow(hItem, true);
		switch(ThingSelected->Thing.Trigger.TriggerType)
		{
		case TRIGGER_TYPE_PROXIMITY:        SendMessage(hItem, CB_SETCURSEL, IDX_TRIGGER_TYPE_PROXIMITY, 0);        break;
		case TRIGGER_TYPE_TIMED:            SendMessage(hItem, CB_SETCURSEL, IDX_TRIGGER_TYPE_TIMED, 0);            break;
		case TRIGGER_TYPE_PLAYER_DEATH:     SendMessage(hItem, CB_SETCURSEL, IDX_TRIGGER_TYPE_PLAYER_DEATH, 0);     break;
		case TRIGGER_TYPE_SHAMAN_PROXIMITY: SendMessage(hItem, CB_SETCURSEL, IDX_TRIGGER_TYPE_SHAMAN_PROXIMITY, 0); break;
		case TRIGGER_TYPE_LIBRARY:          SendMessage(hItem, CB_SETCURSEL, IDX_TRIGGER_TYPE_LIBRARY, 0);          break;
		case TRIGGER_TYPE_SHAMAN_AOD:       SendMessage(hItem, CB_SETCURSEL, IDX_TRIGGER_TYPE_SHAMAN_AOD, 0);       break;
		default: SendMessage(hItem, CB_SETCURSEL, -1, 0);
		}

		hItem = GetDlgItem(hWnd, IDC_TRIGGER_CELLRADIUS);
		EnableWindow(hItem, true);
		sprintf(str, "%d", ThingSelected->Thing.Trigger.CellRadius);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)str);

		hItem = GetDlgItem(hWnd, IDC_TRIGGER_OCCURENCES);
		EnableWindow(hItem, true);
		sprintf(str, "%d", ThingSelected->Thing.Trigger.NumOccurences);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)str);

		hItem = GetDlgItem(hWnd, IDC_TRIGGER_COUNT);
		EnableWindow(hItem, true);
		sprintf(str, "%d", ThingSelected->Thing.Trigger.TriggerCount);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)str);

		hItem = GetDlgItem(hWnd, IDC_TRIGGER_PRAY);
		EnableWindow(hItem, true);
		sprintf(str, "%d", ThingSelected->Thing.Trigger.PrayTime);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)str);

		hItem = GetDlgItem(hWnd, IDC_TRIGGER_INACTIVETIME);
		EnableWindow(hItem, true);
		sprintf(str, "%d", ThingSelected->Thing.Trigger.InactiveTime);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)str);

		if(ThingSelected->Thing.Trigger.StartInactive)
			CheckDlgButton(hWnd, IDC_TRIGGER_STARTINACTIVE, BST_CHECKED);
		else
			CheckDlgButton(hWnd, IDC_TRIGGER_STARTINACTIVE, BST_UNCHECKED);
		EnableWindow(GetDlgItem(hWnd, IDC_TRIGGER_STARTINACTIVE), true);

		if(ThingSelected->Thing.Trigger.CreatePlayerOwned)
			CheckDlgButton(hWnd, IDC_TRIGGER_CREATEOWNED, BST_CHECKED);
		else
			CheckDlgButton(hWnd, IDC_TRIGGER_CREATEOWNED, BST_UNCHECKED);
		EnableWindow(GetDlgItem(hWnd, IDC_TRIGGER_CREATEOWNED), true);

		EnableWindow(GetDlgItem(hWnd, IDC_TRIGGER_LINKS), true);
	}
	else
	{
		hItem = GetDlgItem(hWnd, IDC_TRIGGER_TYPE);
		SendMessage(hItem, CB_SETCURSEL, -1, 0);
		EnableWindow(hItem, false);

		hItem = GetDlgItem(hWnd, IDC_TRIGGER_CELLRADIUS);
		EnableWindow(hItem, false);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)"");

		hItem = GetDlgItem(hWnd, IDC_TRIGGER_OCCURENCES);
		EnableWindow(hItem, false);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)"");

		hItem = GetDlgItem(hWnd, IDC_TRIGGER_COUNT);
		EnableWindow(hItem, false);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)"");

		hItem = GetDlgItem(hWnd, IDC_TRIGGER_PRAY);
		EnableWindow(hItem, false);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)"");

		hItem = GetDlgItem(hWnd, IDC_TRIGGER_INACTIVETIME);
		EnableWindow(hItem, false);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)"");

		EnableWindow(GetDlgItem(hWnd, IDC_TRIGGER_STARTINACTIVE), false);
		EnableWindow(GetDlgItem(hWnd, IDC_TRIGGER_CREATEOWNED), false);
		if(!hDlgLink) EnableWindow(GetDlgItem(hWnd, IDC_TRIGGER_LINKS), false);
	}

	if(lock)
	{
		SendMessage(hWnd, WM_SETREDRAW, true, 0);
		InvalidateRect(hWnd, 0, false);
	}
}


// -=-=- link dialog -=-=-

void DlgLinkToggle()
{
	if(hDlgLink)
	{
		DestroyWindow(hDlgLink);
		hDlgLink = 0;
		ThingLink = 0;
	}
	else
	{
		hDlgLink = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_LINK), hMainWnd, DlgLinkProc, 0);
		ShowWindow(hDlgLink, SW_SHOW);
	}
	DlgObjectUpdateInfo(hDlgObject);
}


int __stdcall DlgLinkProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		DlgLinkUpdateInfo(hWnd, false);
		return 0;

	case WM_CLOSE:
		DlgLinkToggle();
		return 0;

	case WM_DRAWITEM:
		if(wParam != IDC_LINK_LIST) return 0;
		{
			DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT*)lParam;
			if(di->itemID == -1) return 0;
			if(di->itemAction != ODA_SELECT && di->itemAction != ODA_DRAWENTIRE) return 1;

			DWORD dw;

			if(ThingLink && (di->itemID >= 0 && di->itemID <= 9) && ThingLink->Links[di->itemID])
			{
				switch(ThingLink->Links[di->itemID]->Thing.Owner)
				{
				case OWNER_BLUE:   dw = COLOR_LIST_BLUE;   break;
				case OWNER_RED:    dw = COLOR_LIST_RED;    break;
				case OWNER_YELLOW: dw = COLOR_LIST_YELLOW; break;
				case OWNER_GREEN:  dw = COLOR_LIST_GREEN;  break;
				case OWNER_CYAN:   dw = COLOR_LIST_CYAN;   break;
				case OWNER_MAGENTA:    dw = COLOR_LIST_MAGENTA;    break;
				case OWNER_BLACK: dw = COLOR_LIST_BLACK; break;
				case OWNER_ORANGE:  dw = COLOR_LIST_ORANGE;  break;
                case OWNER_HOSTBOT: dw = COLOR_LIST_NEUTRAL; break;
				default: dw = COLOR_LIST_NEUTRAL;
				}
			}
			else
				dw = COLOR_LIST_NEUTRAL;

			if(di->itemState & ODS_SELECTED)
			{
				SetTextColor(di->hDC, COLOR_LIST_BG);
				SetBkColor(di->hDC, dw);
			}
			else
			{
				SetTextColor(di->hDC, dw);
				SetBkColor(di->hDC, COLOR_LIST_BG);
			}

			TEXTMETRIC tm;
			GetTextMetrics(di->hDC, &tm);

			SendMessage(di->hwndItem, LB_GETTEXT, di->itemID, (LPARAM)str);
			ExtTextOut(di->hDC, di->rcItem.left + 3, (di->rcItem.top + di->rcItem.bottom - tm.tmHeight) / 2, ETO_CLIPPED | ETO_OPAQUE, &di->rcItem, str, strlen(str), 0);
		}
		return 1;

	case WM_COMMAND:
		if(wParam == IDC_LINK_BACK)
		{
			DlgObjectSelect(ThingLink);
			if(!hDlgTrigger) DlgTriggerToggle();
			DlgLinkToggle();
		}

		if(!ThingLink || ThingLink->Thing.Type != T_GENERAL || ThingLink->Thing.Model != M_GENERAL_TRIGGER) return 0;

		switch(LOWORD(wParam))
		{
		case IDC_LINK_LIST:
			if(HIWORD(wParam) == LBN_SELCHANGE)
			{
				int n = SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0);
				int c = SendMessage((HWND)lParam, LB_GETCOUNT, 0, 0);
				if(n >= 0 && n < 10 && c > 0)
				{
					DlgObjectSelect(ThingLink->Links[n]);
					EnableWindow(GetDlgItem(hWnd, IDC_LINK_DEL), true);
					EnableWindow(GetDlgItem(hWnd, IDC_LINK_UP), (n > 0 && c > 1) ? true : false);
					EnableWindow(GetDlgItem(hWnd, IDC_LINK_DOWN), (n < (c - 1) && c > 1) ? true : false);
				}
				else
				{
					EnableWindow(GetDlgItem(hWnd, IDC_LINK_UP), false);
					EnableWindow(GetDlgItem(hWnd, IDC_LINK_DOWN), false);
					EnableWindow(GetDlgItem(hWnd, IDC_LINK_DEL), false);
				}
			}
			break;

		case IDC_LINK_UP:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				HWND hList = GetDlgItem(hWnd, IDC_LINK_LIST);
				int n = SendMessage(hList, LB_GETCURSEL, 0, 0);
				if(n < 1 || n > 9) break;

				THING *t = ThingLink->Links[n];
				ThingLink->Links[n] = ThingLink->Links[n - 1];
				ThingLink->Links[n - 1] = t;

				DlgLinkUpdateInfo(hWnd);

				SendMessage(hList, LB_SETCURSEL, (n - 1), 0);
				DlgLinkProc(hWnd, WM_COMMAND, MAKELONG(IDC_LINK_LIST, LBN_SELCHANGE), (LPARAM)hList);
			}
			break;
		case IDC_LINK_DOWN:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				HWND hList = GetDlgItem(hWnd, IDC_LINK_LIST);
				int n = SendMessage(hList, LB_GETCURSEL, 0, 0);
				if(n < 0 || n > 8) break;

				THING *t = ThingLink->Links[n];
				ThingLink->Links[n] = ThingLink->Links[n + 1];
				ThingLink->Links[n + 1] = t;

				DlgLinkUpdateInfo(hWnd);

				SendMessage(hList, LB_SETCURSEL, (n + 1), 0);
				DlgLinkProc(hWnd, WM_COMMAND, MAKELONG(IDC_LINK_LIST, LBN_SELCHANGE), (LPARAM)hList);
			}
			break;
		case IDC_LINK_DEL:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				int n = SendDlgItemMessage(hWnd, IDC_LINK_LIST, LB_GETCURSEL, 0, 0);
				if(n < 0 || n > 9) break;

				ThingLink->Links[n] = 0;
				DlgLinkFixLinks();

				DlgLinkUpdateInfo(hWnd);

				struct Packet *p = new Packet;
				p->wType = PACKETTYPE_TRIGGER_LINK;
				p->wData[0] = false;
				p->wData[1] = ThingLink->Idx;
				p->wData[2] = ThingSelected->Idx;
				p->wData[3] = n;
				net.SendPacket(p);
				p->del();
			}
			break;
		case IDC_LINK_LINK:
			if(HIWORD(wParam) == BN_CLICKED && ThingSelected)
			{
				int n = 0;
				while(n < 10)
				{
					if(!ThingLink->Links[n])
					{
						ThingLink->Links[n] = ThingSelected;
						DlgLinkUpdateInfo(hWnd);
						HWND hList = GetDlgItem(hWnd, IDC_LINK_LIST);
						SendMessage(hList, LB_SETCURSEL, n, 0);
						DlgLinkProc(hWnd, WM_COMMAND, MAKELONG(IDC_LINK_LIST, LBN_SELCHANGE), (LPARAM)hList);

						struct Packet *p = new Packet;
						p->wType = PACKETTYPE_TRIGGER_LINK;
						p->wData[0] = true;
						p->wData[1] = ThingLink->Idx;
						p->wData[2] = ThingSelected->Idx;
						p->wData[3] = n;
						net.SendPacket(p);
						p->del();
						break;
					}
					n++;
				}
			}
			break;
		}

		return 0;
	}

	return 0;
}


void DlgLinkUpdateInfo(HWND hWnd, bool lock)
{
	if(!hWnd) return;

	if(lock) SendMessage(hWnd, WM_SETREDRAW, false, 0);

	HWND hItem;
	if(ThingLink && ThingLink->Thing.Type == T_GENERAL && ThingLink->Thing.Model == M_GENERAL_TRIGGER)
	{
		EnableWindow(GetDlgItem(hWnd, IDC_LINK_BACK), true);
		EnableWindow(GetDlgItem(hWnd, IDC_LINK_UP), false);
		EnableWindow(GetDlgItem(hWnd, IDC_LINK_DOWN), false);
		EnableWindow(GetDlgItem(hWnd, IDC_LINK_DEL), false);

		DlgLinkFixLinks();

		hItem = GetDlgItem(hWnd, IDC_LINK_LIST);
		SendMessage(hItem, WM_SETREDRAW, false, 0);

		int n, c, i;

		c = SendMessage(hItem, LB_GETCOUNT, 0, 0);
		n = 0;

		i = 0;
		while(i < 10)
		{
			if(ThingLink->Links[i])
			{
				if(n < c)
				{
					THING *t;
					t = (THING*)SendMessage(hItem, LB_GETITEMDATA, n, 0);
					if(*(int*)t == LB_ERR || t != ThingLink->Links[i])
					{
						GetThingName(ThingLink->Links[i], str);
						SendMessage(hItem, LB_INSERTSTRING, n, (LPARAM)str);
						SendMessage(hItem, LB_SETITEMDATA, n, (LPARAM)ThingLink->Links[i]);
						c++;
					}
					else
					{
						char s[STR_SIZE];
						SendMessage(hItem, LB_GETTEXT, n, (LPARAM)s);
						GetThingName(t, str);
						if(lstrcmp(s, str) != 0)
						{
							SendMessage(hItem, LB_DELETESTRING, n, 0);
							SendMessage(hItem, LB_INSERTSTRING, n, (LPARAM)str);
							SendMessage(hItem, LB_SETITEMDATA, n, (LPARAM)ThingLink->Links[i]);
						}
					}
				}
				else
				{
					GetThingName(ThingLink->Links[i], str);
					n = SendMessage(hItem, LB_ADDSTRING, 0, (LPARAM)str);
					SendMessage(hItem, LB_SETITEMDATA, n, (LPARAM)ThingLink->Links[i]);
					c++;
				}
				n++;
			}
			else
			{
				break;
			}
			i++;
		}

		while(n < c)
		{
			SendMessage(hItem, LB_DELETESTRING, n, 0);
			c--;
		}

		SendMessage(hItem, WM_SETREDRAW, true, 0);
		InvalidateRect(hItem, 0, false);

		EnableWindow(GetDlgItem(hWnd, IDC_LINK_LIST), true);

		EnableWindow(GetDlgItem(hWnd, IDC_LINK_LINK), ThingLink->Links[9] ? false : true);
	}
	else
	{
		SendDlgItemMessage(hWnd, IDC_LINK_LIST, LB_RESETCONTENT, 0, 0);
		EnableWindow(GetDlgItem(hWnd, IDC_LINK_LIST), false);
		EnableWindow(GetDlgItem(hWnd, IDC_LINK_BACK), false);
		EnableWindow(GetDlgItem(hWnd, IDC_LINK_UP), false);
		EnableWindow(GetDlgItem(hWnd, IDC_LINK_DOWN), false);
		EnableWindow(GetDlgItem(hWnd, IDC_LINK_DEL), false);
		EnableWindow(GetDlgItem(hWnd, IDC_LINK_LINK), false);
	}

	if(lock)
	{
		SendMessage(hWnd, WM_SETREDRAW, true, 0);
		InvalidateRect(hWnd, 0, false);
	}
}


void DlgLinkFixLinks()
{
	if(!ThingLink) return;

	int i, j;

	i = 0;
	while(i < 10)
	{
		if(!ThingLink->Links[i])
		{
			j = i + 1;
			while(j < 10)
			{
				if(ThingLink->Links[j])
				{
					ThingLink->Links[i] = ThingLink->Links[j];
					ThingLink->Links[j] = 0;
					break;
				}
				j++;
			}
		}
		i++;
	}
}

// -=-=- list dialog -=-=-

void DlgListToggle()
{
	if(hDlgList)
	{
		DestroyWindow(hDlgList);
		hDlgList = 0;
	}
	else
	{
		hDlgList = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_LIST), hMainWnd, DlgListProc, 0);
		ShowWindow(hDlgList, SW_SHOW);
	}
}


int __stdcall DlgListProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		DlgListUpdateInfo(hWnd, false);
		return 0;

	case WM_CLOSE:
		DlgListToggle();
		return 0;

	case WM_DRAWITEM:
		if(wParam != IDC_LIST_LIST) return 0;
		{
			DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT*)lParam;
			if(di->itemID == -1) return 1;
			if(di->itemAction != ODA_SELECT && di->itemAction != ODA_DRAWENTIRE) return 1;

			DWORD dw;

			THING *t = 0;
			t = (THING*)SendMessage(di->hwndItem, LB_GETITEMDATA, di->itemID, 0);
			if(*(int*)t != LB_ERR && t)
			{
				switch(t->Thing.Owner)
				{
				case OWNER_BLUE:   dw = COLOR_LIST_BLUE;   break;
				case OWNER_RED:    dw = COLOR_LIST_RED;    break;
				case OWNER_YELLOW: dw = COLOR_LIST_YELLOW; break;
				case OWNER_GREEN:  dw = COLOR_LIST_GREEN;  break;
				case OWNER_CYAN:   dw = COLOR_LIST_CYAN;   break;
				case OWNER_MAGENTA:    dw = COLOR_LIST_MAGENTA;    break;
				case OWNER_BLACK: dw = COLOR_LIST_BLACK; break;
				case OWNER_ORANGE:  dw = COLOR_LIST_ORANGE;  break;
                case OWNER_HOSTBOT: dw = COLOR_LIST_NEUTRAL; break;
				default: dw = COLOR_LIST_NEUTRAL;
				}
			}
			else
				dw = COLOR_LIST_NEUTRAL;

			if(di->itemState & ODS_SELECTED)
			{
				SetTextColor(di->hDC, COLOR_LIST_BG);
				SetBkColor(di->hDC, dw);
			}
			else
			{
				SetTextColor(di->hDC, dw);
				SetBkColor(di->hDC, COLOR_LIST_BG);
			}

			TEXTMETRIC tm;
			GetTextMetrics(di->hDC, &tm);

			SendMessage(di->hwndItem, LB_GETTEXT, di->itemID, (LPARAM)str);
			ExtTextOut(di->hDC, di->rcItem.left + 3, (di->rcItem.top + di->rcItem.bottom - tm.tmHeight) / 2, ETO_CLIPPED | ETO_OPAQUE, &di->rcItem, str, strlen(str), 0);
		}
		return 1;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_LIST_FILTER:
			if(HIWORD(wParam) == BN_CLICKED) DlgFilterToggle();
			break;

		case IDC_LIST_LIST:
			if(HIWORD(wParam) == LBN_SELCHANGE)
			{
				int n = SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0);
				int c = SendMessage((HWND)lParam, LB_GETCOUNT, 0, 0);

				if(c > 0 && n >= 0 && n < c)
				{
					THING *t;
					t = (THING*)SendMessage((HWND)lParam, LB_GETITEMDATA, n, 0);
					if(*(int*)t != LB_ERR && t) DlgObjectSelect(t);
				}
			}
			break;
		}
		return 0;
	}

	return 0;
}


void DlgListUpdateInfo(HWND hWnd, bool lock)
{
	if(!hWnd) return;

	if(lock) SendMessage(hWnd, WM_SETREDRAW, false, 0);

	HWND hItem;
	hItem = GetDlgItem(hWnd, IDC_LIST_LIST);

	if(0 == ThingSelected) SendMessage(hItem, LB_SETCURSEL, -1, 0);

	THING *t = Things;
	if(t)
	{
		int n, c;
		c = SendMessage(hItem, LB_GETCOUNT, 0, 0);
		n = 0;

		do
		{
			if(DlgFilterIsInFilter(t))
			{
				if(n < c)
				{
					THING *a;
					a = (THING*)SendMessage(hItem, LB_GETITEMDATA, n, 0);
					if(*(int*)a == LB_ERR || a != t)
					{
						GetThingName(t, str);
						SendMessage(hItem, LB_INSERTSTRING, n, (LPARAM)str);
						SendMessage(hItem, LB_SETITEMDATA, n, (LPARAM)t);
						if(t == ThingSelected) SendMessage(hItem, LB_SETCURSEL, n, 0);
						c++;
					}
					else
					{
						char s[STR_SIZE];
						SendMessage(hItem, LB_GETTEXT, n, (LPARAM)s);
						GetThingName(t, str);
						if(lstrcmp(s, str) != 0)
						{
							SendMessage(hItem, LB_DELETESTRING, n, 0);
							SendMessage(hItem, LB_INSERTSTRING, n, (LPARAM)str);
							SendMessage(hItem, LB_SETITEMDATA, n, (LPARAM)t);
						}
						if(t == ThingSelected) SendMessage(hItem, LB_SETCURSEL, n, 0);
					}
				}
				else
				{
					GetThingName(t, str);
					n = SendMessage(hItem, LB_ADDSTRING, 0, (LPARAM)str);
					SendMessage(hItem, LB_SETITEMDATA, n, (LPARAM)t);
					if(t == ThingSelected) SendMessage(hItem, LB_SETCURSEL, n, 0);
					c++;
				}
				n++;
			}
			t = t->Next;
		}
		while(t != Things);

		while(n < c)
		{
			SendMessage(hItem, LB_DELETESTRING, n, 0);
			c--;
		}
	}
	else
	{
		SendMessage(hItem, LB_RESETCONTENT, 0, 0);
	}

	if(lock)
	{
		SendMessage(hWnd, WM_SETREDRAW, true, 0);
		InvalidateRect(hWnd, 0, false);
	}
}


// -=-=- about dialog -=-=-

void DlgAboutToggle()
{
	if(hDlgAbout)
	{
		DestroyWindow(hDlgAbout);
		hDlgAbout = 0;
	}
	else
	{
		hDlgAbout = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ABOUT), hMainWnd, DlgAboutProc, 0);
		ShowWindow(hDlgAbout, SW_SHOW);
	}
}


int __stdcall DlgAboutProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		sprintf(str, "Version: %s", POPEDT_VERSION);
		SetDlgItemText(hWnd, IDC_ABOUT_VERSION, str);

		sprintf(str, "Compilation: %s, %s", __DATE__, __TIME__);
		SetDlgItemText(hWnd, IDC_ABOUT_DATE, str);
		return 0;

	case WM_CLOSE:
		DlgAboutToggle();
		return 0;

	case WM_COMMAND:
		if(wParam == IDOK) DlgAboutToggle();
		return 0;
	}

	return 0;
}


// -=-=- header dialogs -=-=-

void UpdateHeaderDialogs()
{
	DlgAlliesUpdate(hDlgAllies);
	DlgSpellsBuildingsUpdate(hDlgSpellsBuildings);
	DlgScript2Update(hDlgScript2);
	DlgSpellsNotChargingUpdate(hDlgSpellsNotCharging);
	DlgHeaderUpdate(hDlgHeader);
	DlgObjBankUpdate(hDlgObjBank);
	DlgMapTypeUpdate(hDlgMapType);
}

// -=-=- allies dialog -=-=-

void DlgAlliesToggle()
{
	if(hDlgAllies)
	{
		DestroyWindow(hDlgAllies);
		hDlgAllies = 0;
	}
	else
	{
		hDlgAllies = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ALLIES), hMainWnd, DlgAlliesProc, 0);
		ShowWindow(hDlgAllies, SW_SHOW);
	}
}

void DlgAlliesProcModify(WPARAM wParam, bool bNetwork)
{
	switch(wParam)
	{
	// blue
	case IDC_ALLIES_BLUE_RED:
		if (leveldat->Header.v2.DefaultAllies[0] & TRIBE_RED)
			leveldat->Header.v2.DefaultAllies[0] &= ~TRIBE_RED;
		else
			leveldat->Header.v2.DefaultAllies[0] |= TRIBE_RED;
		break;
	case IDC_ALLIES_BLUE_YELLOW:
		if (leveldat->Header.v2.DefaultAllies[0] & TRIBE_YELLOW)
			leveldat->Header.v2.DefaultAllies[0] &= ~TRIBE_YELLOW;
		else
			leveldat->Header.v2.DefaultAllies[0] |= TRIBE_YELLOW;
		break;
	case IDC_ALLIES_BLUE_GREEN:
		if (leveldat->Header.v2.DefaultAllies[0] & TRIBE_GREEN)
			leveldat->Header.v2.DefaultAllies[0] &= ~TRIBE_GREEN;
		else
			leveldat->Header.v2.DefaultAllies[0] |= TRIBE_GREEN;
		break;
		// red
	case IDC_ALLIES_RED_BLUE:
		if (leveldat->Header.v2.DefaultAllies[1] & TRIBE_BLUE)
			leveldat->Header.v2.DefaultAllies[1] &= ~TRIBE_BLUE;
		else
			leveldat->Header.v2.DefaultAllies[1] |= TRIBE_BLUE;
		break;
	case IDC_ALLIES_RED_YELLOW:
		if (leveldat->Header.v2.DefaultAllies[1] & TRIBE_YELLOW)
			leveldat->Header.v2.DefaultAllies[1] &= ~TRIBE_YELLOW;
		else
			leveldat->Header.v2.DefaultAllies[1] |= TRIBE_YELLOW;
		break;
	case IDC_ALLIES_RED_GREEN:
		if (leveldat->Header.v2.DefaultAllies[1] & TRIBE_GREEN)
			leveldat->Header.v2.DefaultAllies[1] &= ~TRIBE_GREEN;
		else
			leveldat->Header.v2.DefaultAllies[1] |= TRIBE_GREEN;
		break;
		// yellow
	case IDC_ALLIES_YELLOW_BLUE:
		if (leveldat->Header.v2.DefaultAllies[2] & TRIBE_BLUE)
			leveldat->Header.v2.DefaultAllies[2] &= ~TRIBE_BLUE;
		else
			leveldat->Header.v2.DefaultAllies[2] |= TRIBE_BLUE;
		break;
	case IDC_ALLIES_YELLOW_RED:
		if (leveldat->Header.v2.DefaultAllies[2] & TRIBE_RED)
			leveldat->Header.v2.DefaultAllies[2] &= ~TRIBE_RED;
		else
			leveldat->Header.v2.DefaultAllies[2] |= TRIBE_RED;
		break;
	case IDC_ALLIES_YELLOW_GREEN:
		if (leveldat->Header.v2.DefaultAllies[2] & TRIBE_GREEN)
			leveldat->Header.v2.DefaultAllies[2] &= ~TRIBE_GREEN;
		else
			leveldat->Header.v2.DefaultAllies[2] |= TRIBE_GREEN;
		break;
		// green
	case IDC_ALLIES_GREEN_BLUE:
		if (leveldat->Header.v2.DefaultAllies[3] & TRIBE_BLUE)
			leveldat->Header.v2.DefaultAllies[3] &= ~TRIBE_BLUE;
		else
			leveldat->Header.v2.DefaultAllies[3] |= TRIBE_BLUE;
		break;
	case IDC_ALLIES_GREEN_RED:
		if (leveldat->Header.v2.DefaultAllies[3] & TRIBE_RED)
			leveldat->Header.v2.DefaultAllies[3] &= ~TRIBE_RED;
		else
			leveldat->Header.v2.DefaultAllies[3] |= TRIBE_RED;
		break;
	case IDC_ALLIES_GREEN_YELLOW:
		if (leveldat->Header.v2.DefaultAllies[3] & TRIBE_YELLOW)
			leveldat->Header.v2.DefaultAllies[3] &= ~TRIBE_YELLOW;
		else
			leveldat->Header.v2.DefaultAllies[3] |= TRIBE_YELLOW;
		break;
	}

	if (bNetwork)
		DlgAlliesUpdate(hDlgAllies);
}


int __stdcall DlgAlliesProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hWnd, IDC_ALLIES_BLUE_BLUE, BST_CHECKED);
		CheckDlgButton(hWnd, IDC_ALLIES_RED_RED, BST_CHECKED);
		CheckDlgButton(hWnd, IDC_ALLIES_YELLOW_YELLOW, BST_CHECKED);
		CheckDlgButton(hWnd, IDC_ALLIES_GREEN_GREEN, BST_CHECKED);
		DlgAlliesUpdate(hWnd);
		return 0;

	case WM_CLOSE:
		DlgAlliesToggle();
		return 0;

	case WM_COMMAND:
		DlgAlliesProcModify(wParam);
		if (net.IsInitialized())
		{
			struct Packet *p = new Packet;
			p->wType = PACKETTYPE_ALLIES;
			p->wData[0] = wParam;
			net.SendPacket(p);
			p->del();
		}
		return 0;
	}

	return 0;
}


void DlgAlliesUpdate(HWND hWnd)
{
	if(!hWnd) return;

	//

	leveldat->Header.v2.DefaultAllies[0] |= TRIBE_BLUE;
	leveldat->Header.v2.DefaultAllies[1] |= TRIBE_RED;
	leveldat->Header.v2.DefaultAllies[2] |= TRIBE_YELLOW;
	leveldat->Header.v2.DefaultAllies[3] |= TRIBE_GREEN;

	// blue

	if(leveldat->Header.v2.DefaultAllies[0] & TRIBE_RED)
		CheckDlgButton(hWnd, IDC_ALLIES_BLUE_RED, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_ALLIES_BLUE_RED, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultAllies[0] & TRIBE_YELLOW)
		CheckDlgButton(hWnd, IDC_ALLIES_BLUE_YELLOW, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_ALLIES_BLUE_YELLOW, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultAllies[0] & TRIBE_GREEN)
		CheckDlgButton(hWnd, IDC_ALLIES_BLUE_GREEN, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_ALLIES_BLUE_GREEN, BST_UNCHECKED);

	// red

	if(leveldat->Header.v2.DefaultAllies[1] & TRIBE_BLUE)
		CheckDlgButton(hWnd, IDC_ALLIES_RED_BLUE, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_ALLIES_RED_BLUE, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultAllies[1] & TRIBE_YELLOW)
		CheckDlgButton(hWnd, IDC_ALLIES_RED_YELLOW, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_ALLIES_RED_YELLOW, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultAllies[1] & TRIBE_GREEN)
		CheckDlgButton(hWnd, IDC_ALLIES_RED_GREEN, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_ALLIES_RED_GREEN, BST_UNCHECKED);

	// yellow

	if(leveldat->Header.v2.DefaultAllies[2] & TRIBE_BLUE)
		CheckDlgButton(hWnd, IDC_ALLIES_YELLOW_BLUE, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_ALLIES_YELLOW_BLUE, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultAllies[2] & TRIBE_RED)
		CheckDlgButton(hWnd, IDC_ALLIES_YELLOW_RED, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_ALLIES_YELLOW_RED, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultAllies[2] & TRIBE_GREEN)
		CheckDlgButton(hWnd, IDC_ALLIES_YELLOW_GREEN, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_ALLIES_YELLOW_GREEN, BST_UNCHECKED);

	// green

	if(leveldat->Header.v2.DefaultAllies[3] & TRIBE_BLUE)
		CheckDlgButton(hWnd, IDC_ALLIES_GREEN_BLUE, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_ALLIES_GREEN_BLUE, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultAllies[3] & TRIBE_RED)
		CheckDlgButton(hWnd, IDC_ALLIES_GREEN_RED, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_ALLIES_GREEN_RED, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultAllies[3] & TRIBE_YELLOW)
		CheckDlgButton(hWnd, IDC_ALLIES_GREEN_YELLOW, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_ALLIES_GREEN_YELLOW, BST_UNCHECKED);
}

// -=-=- spells / buildings dialog -=-=-

void DlgScript2()
{
	if (hDlgScript2)
	{
		DestroyWindow(hDlgScript2);
		hDlgScript2 = 0;
	}
	else
	{
		hDlgScript2 = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_SCRIPT2), hMainWnd, DlgScript2Proc, 0);
		ShowWindow(hDlgScript2, SW_SHOW);
	}
}

int __stdcall DlgScript2Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hWnd, IDC_SCRIPT2_1, EM_LIMITTEXT, 31, 0);
		SendDlgItemMessage(hWnd, IDC_SCRIPT2_2, EM_LIMITTEXT, 31, 0);
		SendDlgItemMessage(hWnd, IDC_SCRIPT2_3, EM_LIMITTEXT, 31, 0);
		SendDlgItemMessage(hWnd, IDC_SCRIPT2_4, EM_LIMITTEXT, 31, 0);
		SendDlgItemMessage(hWnd, IDC_SCRIPT2_5, EM_LIMITTEXT, 31, 0);
		SendDlgItemMessage(hWnd, IDC_SCRIPT2_6, EM_LIMITTEXT, 31, 0);
		SendDlgItemMessage(hWnd, IDC_SCRIPT2_7, EM_LIMITTEXT, 31, 0);
		SendDlgItemMessage(hWnd, IDC_SCRIPT2_8, EM_LIMITTEXT, 31, 0);
		SendDlgItemMessage(hWnd, IDC_SCRIPT2_9, EM_LIMITTEXT, 31, 0);
		SendDlgItemMessage(hWnd, IDC_SCRIPT2_10, EM_LIMITTEXT, 31, 0);
		DlgScript2Update(hWnd);
		return 0;

	case WM_CLOSE:
		DlgScript2();
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_SCRIPT2_1:
			if (EN_KILLFOCUS == HIWORD(wParam))
			{
				GetWindowText((HWND)lParam, str, sizeof(str));
				strcpy(leveldat->Header.Script2[0], str);
			}
			break;
		case IDC_SCRIPT2_2:
			if (EN_KILLFOCUS == HIWORD(wParam))
			{
				GetWindowText((HWND)lParam, str, sizeof(str));
				strcpy(leveldat->Header.Script2[1], str);
			}
			break;
		case IDC_SCRIPT2_3:
			if (EN_KILLFOCUS == HIWORD(wParam))
			{
				GetWindowText((HWND)lParam, str, sizeof(str));
				strcpy(leveldat->Header.Script2[2], str);
			}
			break;
		case IDC_SCRIPT2_4:
			if (EN_KILLFOCUS == HIWORD(wParam))
			{
				GetWindowText((HWND)lParam, str, sizeof(str));
				strcpy(leveldat->Header.Script2[3], str);
			}
			break;
		case IDC_SCRIPT2_5:
			if (EN_KILLFOCUS == HIWORD(wParam))
			{
				GetWindowText((HWND)lParam, str, sizeof(str));
				strcpy(leveldat->Header.Script2[4], str);
			}
			break;
		case IDC_SCRIPT2_6:
			if (EN_KILLFOCUS == HIWORD(wParam))
			{
				GetWindowText((HWND)lParam, str, sizeof(str));
				strcpy(leveldat->Header.Script2[5], str);
			}
			break;
		case IDC_SCRIPT2_7:
			if (EN_KILLFOCUS == HIWORD(wParam))
			{
				GetWindowText((HWND)lParam, str, sizeof(str));
				strcpy(leveldat->Header.Script2[6], str);
			}
			break;
		case IDC_SCRIPT2_8:
			if (EN_KILLFOCUS == HIWORD(wParam))
			{
				GetWindowText((HWND)lParam, str, sizeof(str));
				strcpy(leveldat->Header.Script2[7], str);
			}
			break;
		case IDC_SCRIPT2_9:
			if (EN_KILLFOCUS == HIWORD(wParam))
			{
				GetWindowText((HWND)lParam, str, sizeof(str));
				strcpy(leveldat->Header.Script2[8], str);
			}
			break;
		case IDC_SCRIPT2_10:
			if (EN_KILLFOCUS == HIWORD(wParam))
			{
				GetWindowText((HWND)lParam, str, sizeof(str));
				strcpy(leveldat->Header.Script2[9], str);
			}
			break;
		}
		return 0;
	}
	return 0;
}

void DlgScript2Update(HWND hWnd) {
	if (!hWnd) return;

	leveldat->Header.Script2[0][31] = 0; // no overflow
	SetDlgItemText(hWnd, IDC_SCRIPT2_1, leveldat->Header.Script2[0]);

	leveldat->Header.Script2[1][31] = 0; // no overflow
	SetDlgItemText(hWnd, IDC_SCRIPT2_2, leveldat->Header.Script2[1]);

	leveldat->Header.Script2[2][31] = 0; // no overflow
	SetDlgItemText(hWnd, IDC_SCRIPT2_3, leveldat->Header.Script2[2]);

	leveldat->Header.Script2[3][31] = 0; // no overflow
	SetDlgItemText(hWnd, IDC_SCRIPT2_4, leveldat->Header.Script2[3]);

	leveldat->Header.Script2[4][31] = 0; // no overflow
	SetDlgItemText(hWnd, IDC_SCRIPT2_5, leveldat->Header.Script2[4]);

	leveldat->Header.Script2[5][31] = 0; // no overflow
	SetDlgItemText(hWnd, IDC_SCRIPT2_6, leveldat->Header.Script2[5]);

	leveldat->Header.Script2[6][31] = 0; // no overflow
	SetDlgItemText(hWnd, IDC_SCRIPT2_7, leveldat->Header.Script2[6]);

	leveldat->Header.Script2[7][31] = 0; // no overflow
	SetDlgItemText(hWnd, IDC_SCRIPT2_8, leveldat->Header.Script2[7]);

	leveldat->Header.Script2[8][31] = 0; // no overflow
	SetDlgItemText(hWnd, IDC_SCRIPT2_9, leveldat->Header.Script2[8]);

	leveldat->Header.Script2[9][31] = 0; // no overflow
	SetDlgItemText(hWnd, IDC_SCRIPT2_10, leveldat->Header.Script2[9]);

}

// -=-=- spells / buildings dialog -=-=-

void DlgSpellsBuildingsToggle()
{
	if(hDlgSpellsBuildings)
	{
		DestroyWindow(hDlgSpellsBuildings);
		hDlgSpellsBuildings = 0;
	}
	else
	{
		hDlgSpellsBuildings = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_SPELLS_BUILDINGS), hMainWnd, DlgSpellsBuildingsProc, 0);
		ShowWindow(hDlgSpellsBuildings, SW_SHOW);
	}
}

void DlgSpellsBuildingsProcModify(WPARAM wParam, bool bNetwork)
{
	switch(wParam)
	{
	// spells
	case IDC_SPELL_BLAST:
		if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_BLAST))
			leveldat->Header.v2.DefaultThings.SpellsAvailable &= ~(1 << M_SPELL_BLAST);
		else
			leveldat->Header.v2.DefaultThings.SpellsAvailable |= (1 << M_SPELL_BLAST);
		break;
	case IDC_SPELL_LIGHTNING:
		if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_LIGHTNING))
			leveldat->Header.v2.DefaultThings.SpellsAvailable &= ~(1 << M_SPELL_LIGHTNING);
		else
			leveldat->Header.v2.DefaultThings.SpellsAvailable |= (1 << M_SPELL_LIGHTNING);
		break;
	case IDC_SPELL_TORNADO:
		if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_TORNADO))
			leveldat->Header.v2.DefaultThings.SpellsAvailable &= ~(1 << M_SPELL_TORNADO);
		else
			leveldat->Header.v2.DefaultThings.SpellsAvailable |= (1 << M_SPELL_TORNADO);
		break;
	case IDC_SPELL_SWARM:
		if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_SWARM))
			leveldat->Header.v2.DefaultThings.SpellsAvailable &= ~(1 << M_SPELL_SWARM);
		else
			leveldat->Header.v2.DefaultThings.SpellsAvailable |= (1 << M_SPELL_SWARM);
		break;
	case IDC_SPELL_INVISIBILITY:
		if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_INVISIBILITY))
			leveldat->Header.v2.DefaultThings.SpellsAvailable &= ~(1 << M_SPELL_INVISIBILITY);
		else
			leveldat->Header.v2.DefaultThings.SpellsAvailable |= (1 << M_SPELL_INVISIBILITY);
		break;
	case IDC_SPELL_HYPNOTISM:
		if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_HYPNOTISM))
			leveldat->Header.v2.DefaultThings.SpellsAvailable &= ~(1 << M_SPELL_HYPNOTISM);
		else
			leveldat->Header.v2.DefaultThings.SpellsAvailable |= (1 << M_SPELL_HYPNOTISM);
		break;
	case IDC_SPELL_FIRESTORM:
		if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_FIRESTORM))
			leveldat->Header.v2.DefaultThings.SpellsAvailable &= ~(1 << M_SPELL_FIRESTORM);
		else
			leveldat->Header.v2.DefaultThings.SpellsAvailable |= (1 << M_SPELL_FIRESTORM);
		break;
	case IDC_SPELL_GHOST:
		if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_GHOST_ARMY))
			leveldat->Header.v2.DefaultThings.SpellsAvailable &= ~(1 << M_SPELL_GHOST_ARMY);
		else
			leveldat->Header.v2.DefaultThings.SpellsAvailable |= (1 << M_SPELL_GHOST_ARMY);
		break;
	case IDC_SPELL_ERODE:
		if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_ERODE))
			leveldat->Header.v2.DefaultThings.SpellsAvailable &= ~(1 << M_SPELL_ERODE);
		else
			leveldat->Header.v2.DefaultThings.SpellsAvailable |= (1 << M_SPELL_ERODE);
		break;
	case IDC_SPELL_SWAMP:
		if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_SWAMP))
			leveldat->Header.v2.DefaultThings.SpellsAvailable &= ~(1 << M_SPELL_SWAMP);
		else
			leveldat->Header.v2.DefaultThings.SpellsAvailable |= (1 << M_SPELL_SWAMP);
		break;
	case IDC_SPELL_LANDBRIDGE:
		if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_LAND_BRIDGE))
			leveldat->Header.v2.DefaultThings.SpellsAvailable &= ~(1 << M_SPELL_LAND_BRIDGE);
		else
			leveldat->Header.v2.DefaultThings.SpellsAvailable |= (1 << M_SPELL_LAND_BRIDGE);
		break;
	case IDC_SPELL_AOD:
		if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_ANGEL_OF_DEATH))
			leveldat->Header.v2.DefaultThings.SpellsAvailable &= ~(1 << M_SPELL_ANGEL_OF_DEATH);
		else
			leveldat->Header.v2.DefaultThings.SpellsAvailable |= (1 << M_SPELL_ANGEL_OF_DEATH);
		break;
	case IDC_SPELL_EARTHQUAKE:
		if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_EARTHQUAKE))
			leveldat->Header.v2.DefaultThings.SpellsAvailable &= ~(1 << M_SPELL_EARTHQUAKE);
		else
			leveldat->Header.v2.DefaultThings.SpellsAvailable |= (1 << M_SPELL_EARTHQUAKE);
		break;
	case IDC_SPELL_FLATTEN:
		if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_FLATTEN))
			leveldat->Header.v2.DefaultThings.SpellsAvailable &= ~(1 << M_SPELL_FLATTEN);
		else
			leveldat->Header.v2.DefaultThings.SpellsAvailable |= (1 << M_SPELL_FLATTEN);
		break;
	case IDC_SPELL_VOLCANO:
		if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_VOLCANO))
			leveldat->Header.v2.DefaultThings.SpellsAvailable &= ~(1 << M_SPELL_VOLCANO);
		else
			leveldat->Header.v2.DefaultThings.SpellsAvailable |= (1 << M_SPELL_VOLCANO);
		break;
	case IDC_SPELL_CONVERT:
		if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_CONVERT))
			leveldat->Header.v2.DefaultThings.SpellsAvailable &= ~(1 << M_SPELL_CONVERT);
		else
			leveldat->Header.v2.DefaultThings.SpellsAvailable |= (1 << M_SPELL_CONVERT);
		break;
	case IDC_SPELL_MAGICALSHIELD:
		if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_MAGICAL_SHIELD))
			leveldat->Header.v2.DefaultThings.SpellsAvailable &= ~(1 << M_SPELL_MAGICAL_SHIELD);
		else
			leveldat->Header.v2.DefaultThings.SpellsAvailable |= (1 << M_SPELL_MAGICAL_SHIELD);
		break;

	// buildings

	case IDC_BUILDING_HUT:
		if(leveldat->Header.v2.DefaultThings.BuildingsAvailable & (1 << M_BUILDING_HUT1))
			leveldat->Header.v2.DefaultThings.BuildingsAvailable &= ~(1 << M_BUILDING_HUT1);
		else
			leveldat->Header.v2.DefaultThings.BuildingsAvailable |= (1 << M_BUILDING_HUT1);
		break;
	case IDC_BUILDING_TOWER:
		if(leveldat->Header.v2.DefaultThings.BuildingsAvailable & (1 << M_BUILDING_TOWER))
			leveldat->Header.v2.DefaultThings.BuildingsAvailable &= ~(1 << M_BUILDING_TOWER);
		else
			leveldat->Header.v2.DefaultThings.BuildingsAvailable |= (1 << M_BUILDING_TOWER);
		break;
	case IDC_BUILDING_TEMPLE:
		if(leveldat->Header.v2.DefaultThings.BuildingsAvailable & (1 << M_BUILDING_TEMPLE))
			leveldat->Header.v2.DefaultThings.BuildingsAvailable &= ~(1 << M_BUILDING_TEMPLE);
		else
			leveldat->Header.v2.DefaultThings.BuildingsAvailable |= (1 << M_BUILDING_TEMPLE);
		break;
	case IDC_BUILDING_SPY:
		if(leveldat->Header.v2.DefaultThings.BuildingsAvailable & (1 << M_BUILDING_SPY_TRAIN))
			leveldat->Header.v2.DefaultThings.BuildingsAvailable &= ~(1 << M_BUILDING_SPY_TRAIN);
		else
			leveldat->Header.v2.DefaultThings.BuildingsAvailable |= (1 << M_BUILDING_SPY_TRAIN);
		break;
	case IDC_BUILDING_WARRIOR:
		if(leveldat->Header.v2.DefaultThings.BuildingsAvailable & (1 << M_BUILDING_WARRIOR_TRAIN))
			leveldat->Header.v2.DefaultThings.BuildingsAvailable &= ~(1 << M_BUILDING_WARRIOR_TRAIN);
		else
			leveldat->Header.v2.DefaultThings.BuildingsAvailable |= (1 << M_BUILDING_WARRIOR_TRAIN);
		break;
	case IDC_BUILDING_FIREWARRIOR:
		if(leveldat->Header.v2.DefaultThings.BuildingsAvailable & (1 << M_BUILDING_FIREWARRIOR_TRAIN))
			leveldat->Header.v2.DefaultThings.BuildingsAvailable &= ~(1 << M_BUILDING_FIREWARRIOR_TRAIN);
		else
			leveldat->Header.v2.DefaultThings.BuildingsAvailable |= (1 << M_BUILDING_FIREWARRIOR_TRAIN);
		break;
	case IDC_BUILDING_BOAT:
		if(leveldat->Header.v2.DefaultThings.BuildingsAvailable & (1 << M_BUILDING_BOAT_HUT_1))
			leveldat->Header.v2.DefaultThings.BuildingsAvailable &= ~(1 << M_BUILDING_BOAT_HUT_1);
		else
			leveldat->Header.v2.DefaultThings.BuildingsAvailable |= (1 << M_BUILDING_BOAT_HUT_1);
		break;
	case IDC_BUILDING_BALLON:
		if(leveldat->Header.v2.DefaultThings.BuildingsAvailable & (1 << M_BUILDING_AIRSHIP_HUT_1))
			leveldat->Header.v2.DefaultThings.BuildingsAvailable &= ~(1 << M_BUILDING_AIRSHIP_HUT_1);
		else
			leveldat->Header.v2.DefaultThings.BuildingsAvailable |= (1 << M_BUILDING_AIRSHIP_HUT_1);
		break;
	}

	if (bNetwork)
		DlgSpellsBuildingsUpdate(hDlgSpellsBuildings);
}

int __stdcall DlgSpellsBuildingsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		DlgSpellsBuildingsUpdate(hWnd);
		return 0;

	case WM_CLOSE:
		DlgSpellsBuildingsToggle();
		return 0;

	case WM_COMMAND:
	{
		DlgSpellsBuildingsProcModify(wParam);
		if (net.IsInitialized())
		{
			struct Packet *p = new Packet;
			p->wType = PACKETTYPE_AVAILABLE_DISC;
			p->wData[0] = wParam;
			net.SendPacket(p);
			p->del();
		}
	}
		return 0;
	}

	return 0;
}


void DlgSpellsBuildingsUpdate(HWND hWnd)
{
	if(!hWnd) return;

	//

	if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_BLAST))
		CheckDlgButton(hWnd, IDC_SPELL_BLAST, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_BLAST, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_LIGHTNING))
		CheckDlgButton(hWnd, IDC_SPELL_LIGHTNING, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_LIGHTNING, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_TORNADO))
		CheckDlgButton(hWnd, IDC_SPELL_TORNADO, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_TORNADO, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_SWARM))
		CheckDlgButton(hWnd, IDC_SPELL_SWARM, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_SWARM, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_INVISIBILITY))
		CheckDlgButton(hWnd, IDC_SPELL_INVISIBILITY, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_INVISIBILITY, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_HYPNOTISM))
		CheckDlgButton(hWnd, IDC_SPELL_HYPNOTISM, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_HYPNOTISM, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_FIRESTORM))
		CheckDlgButton(hWnd, IDC_SPELL_FIRESTORM, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_FIRESTORM, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_GHOST_ARMY))
		CheckDlgButton(hWnd, IDC_SPELL_GHOST, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_GHOST, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_ERODE))
		CheckDlgButton(hWnd, IDC_SPELL_ERODE, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_ERODE, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_SWAMP))
		CheckDlgButton(hWnd, IDC_SPELL_SWAMP, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_SWAMP, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_LAND_BRIDGE))
		CheckDlgButton(hWnd, IDC_SPELL_LANDBRIDGE, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_LANDBRIDGE, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_ANGEL_OF_DEATH))
		CheckDlgButton(hWnd, IDC_SPELL_AOD, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_AOD, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_EARTHQUAKE))
		CheckDlgButton(hWnd, IDC_SPELL_EARTHQUAKE, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_EARTHQUAKE, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_FLATTEN))
		CheckDlgButton(hWnd, IDC_SPELL_FLATTEN, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_FLATTEN, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_VOLCANO))
		CheckDlgButton(hWnd, IDC_SPELL_VOLCANO, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_VOLCANO, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_CONVERT))
		CheckDlgButton(hWnd, IDC_SPELL_CONVERT, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_CONVERT, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsAvailable & (1 << M_SPELL_MAGICAL_SHIELD))
		CheckDlgButton(hWnd, IDC_SPELL_MAGICALSHIELD, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_MAGICALSHIELD, BST_UNCHECKED);

	//

	if(leveldat->Header.v2.DefaultThings.BuildingsAvailable & (1 << M_BUILDING_HUT1))
		CheckDlgButton(hWnd, IDC_BUILDING_HUT, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BUILDING_HUT, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.BuildingsAvailable & (1 << M_BUILDING_TOWER))
		CheckDlgButton(hWnd, IDC_BUILDING_TOWER, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BUILDING_TOWER, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.BuildingsAvailable & (1 << M_BUILDING_TEMPLE))
		CheckDlgButton(hWnd, IDC_BUILDING_TEMPLE, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BUILDING_TEMPLE, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.BuildingsAvailable & (1 << M_BUILDING_SPY_TRAIN))
		CheckDlgButton(hWnd, IDC_BUILDING_SPY, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BUILDING_SPY, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.BuildingsAvailable & (1 << M_BUILDING_WARRIOR_TRAIN))
		CheckDlgButton(hWnd, IDC_BUILDING_WARRIOR, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BUILDING_WARRIOR, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.BuildingsAvailable & (1 << M_BUILDING_FIREWARRIOR_TRAIN))
		CheckDlgButton(hWnd, IDC_BUILDING_FIREWARRIOR, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BUILDING_FIREWARRIOR, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.BuildingsAvailable & (1 << M_BUILDING_BOAT_HUT_1))
		CheckDlgButton(hWnd, IDC_BUILDING_BOAT, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BUILDING_BOAT, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.BuildingsAvailable & (1 << M_BUILDING_AIRSHIP_HUT_1))
		CheckDlgButton(hWnd, IDC_BUILDING_BALLON, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BUILDING_BALLON, BST_UNCHECKED);
}


// -=-=- spells not charging dialog -=-=-

void DlgSpellsNotChargingToggle()
{
	if(hDlgSpellsNotCharging)
	{
		DestroyWindow(hDlgSpellsNotCharging);
		hDlgSpellsNotCharging = 0;
	}
	else
	{
		hDlgSpellsNotCharging = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_SPELLS_NOT_CHARGING), hMainWnd, DlgSpellsNotChargingProc, 0);
		ShowWindow(hDlgSpellsNotCharging, SW_SHOW);
	}
}


void DlgSpellsNotChargingProcModify(WPARAM wParam, bool bNetwork)
{
	switch(wParam)
	{
	case IDC_SPELL_BLAST:
		if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_BLAST - 1)))
			leveldat->Header.v2.DefaultThings.SpellsNotCharging &= ~(1 << (M_SPELL_BLAST - 1));
		else
			leveldat->Header.v2.DefaultThings.SpellsNotCharging |= (1 << (M_SPELL_BLAST - 1));
		break;
	case IDC_SPELL_LIGHTNING:
		if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_LIGHTNING - 1)))
			leveldat->Header.v2.DefaultThings.SpellsNotCharging &= ~(1 << (M_SPELL_LIGHTNING - 1));
		else
			leveldat->Header.v2.DefaultThings.SpellsNotCharging |= (1 << (M_SPELL_LIGHTNING - 1));
		break;
	case IDC_SPELL_TORNADO:
		if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_TORNADO - 1)))
			leveldat->Header.v2.DefaultThings.SpellsNotCharging &= ~(1 << (M_SPELL_TORNADO - 1));
		else
			leveldat->Header.v2.DefaultThings.SpellsNotCharging |= (1 << (M_SPELL_TORNADO - 1));
		break;
	case IDC_SPELL_SWARM:
		if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_SWARM - 1)))
			leveldat->Header.v2.DefaultThings.SpellsNotCharging &= ~(1 << (M_SPELL_SWARM - 1));
		else
			leveldat->Header.v2.DefaultThings.SpellsNotCharging |= (1 << (M_SPELL_SWARM - 1));
		break;
	case IDC_SPELL_INVISIBILITY:
		if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_INVISIBILITY - 1)))
			leveldat->Header.v2.DefaultThings.SpellsNotCharging &= ~(1 << (M_SPELL_INVISIBILITY - 1));
		else
			leveldat->Header.v2.DefaultThings.SpellsNotCharging |= (1 << (M_SPELL_INVISIBILITY - 1));
		break;
	case IDC_SPELL_HYPNOTISM:
		if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_HYPNOTISM - 1)))
			leveldat->Header.v2.DefaultThings.SpellsNotCharging &= ~(1 << (M_SPELL_HYPNOTISM - 1));
		else
			leveldat->Header.v2.DefaultThings.SpellsNotCharging |= (1 << (M_SPELL_HYPNOTISM - 1));
		break;
	case IDC_SPELL_FIRESTORM:
		if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_FIRESTORM - 1)))
			leveldat->Header.v2.DefaultThings.SpellsNotCharging &= ~(1 << (M_SPELL_FIRESTORM - 1));
		else
			leveldat->Header.v2.DefaultThings.SpellsNotCharging |= (1 << (M_SPELL_FIRESTORM - 1));
		break;
	case IDC_SPELL_ERODE:
		if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_ERODE - 1)))
			leveldat->Header.v2.DefaultThings.SpellsNotCharging &= ~(1 << (M_SPELL_ERODE - 1));
		else
			leveldat->Header.v2.DefaultThings.SpellsNotCharging |= (1 << (M_SPELL_ERODE - 1));
		break;
	case IDC_SPELL_SWAMP:
		if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_SWAMP - 1)))
			leveldat->Header.v2.DefaultThings.SpellsNotCharging &= ~(1 << (M_SPELL_SWAMP - 1));
		else
			leveldat->Header.v2.DefaultThings.SpellsNotCharging |= (1 << (M_SPELL_SWAMP - 1));
		break;
	case IDC_SPELL_LANDBRIDGE:
		if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_LAND_BRIDGE - 1)))
			leveldat->Header.v2.DefaultThings.SpellsNotCharging &= ~(1 << (M_SPELL_LAND_BRIDGE - 1));
		else
			leveldat->Header.v2.DefaultThings.SpellsNotCharging |= (1 << (M_SPELL_LAND_BRIDGE - 1));
		break;
	case IDC_SPELL_AOD:
		if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_ANGEL_OF_DEATH - 1)))
			leveldat->Header.v2.DefaultThings.SpellsNotCharging &= ~(1 << (M_SPELL_ANGEL_OF_DEATH - 1));
		else
			leveldat->Header.v2.DefaultThings.SpellsNotCharging |= (1 << (M_SPELL_ANGEL_OF_DEATH - 1));
		break;
	case IDC_SPELL_EARTHQUAKE:
		if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_EARTHQUAKE - 1)))
			leveldat->Header.v2.DefaultThings.SpellsNotCharging &= ~(1 << (M_SPELL_EARTHQUAKE - 1));
		else
			leveldat->Header.v2.DefaultThings.SpellsNotCharging |= (1 << (M_SPELL_EARTHQUAKE - 1));
		break;
	case IDC_SPELL_FLATTEN:
		if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_FLATTEN - 1)))
			leveldat->Header.v2.DefaultThings.SpellsNotCharging &= ~(1 << (M_SPELL_FLATTEN - 1));
		else
			leveldat->Header.v2.DefaultThings.SpellsNotCharging |= (1 << (M_SPELL_FLATTEN - 1));
		break;
	case IDC_SPELL_VOLCANO:
		if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_VOLCANO - 1)))
			leveldat->Header.v2.DefaultThings.SpellsNotCharging &= ~(1 << (M_SPELL_VOLCANO - 1));
		else
			leveldat->Header.v2.DefaultThings.SpellsNotCharging |= (1 << (M_SPELL_VOLCANO - 1));
		break;
	case IDC_SPELL_CONVERT:
		if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_CONVERT - 1)))
			leveldat->Header.v2.DefaultThings.SpellsNotCharging &= ~(1 << (M_SPELL_CONVERT - 1));
		else
			leveldat->Header.v2.DefaultThings.SpellsNotCharging |= (1 << (M_SPELL_CONVERT - 1));
		break;
	case IDC_SPELL_MAGICALSHIELD:
		if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_MAGICAL_SHIELD - 1)))
			leveldat->Header.v2.DefaultThings.SpellsNotCharging &= ~(1 << (M_SPELL_MAGICAL_SHIELD - 1));
		else
			leveldat->Header.v2.DefaultThings.SpellsNotCharging |= (1 << (M_SPELL_MAGICAL_SHIELD - 1));
		break;
	}

	if (bNetwork)
		DlgSpellsNotChargingUpdate(hDlgSpellsNotCharging);
}


int __stdcall DlgSpellsNotChargingProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		DlgSpellsNotChargingUpdate(hWnd);
		return 0;

	case WM_CLOSE:
		DlgSpellsNotChargingToggle();
		return 0;

	case WM_COMMAND:
		DlgSpellsNotChargingProcModify(wParam);
		if (net.IsInitialized())
		{
			struct Packet *p = new Packet;
			p->wType = PACKETTYPE_SPELL_RESTRICTIONS;
			p->wData[0] = wParam;
			net.SendPacket(p);
			p->del();
		}
		return 0;
	}

	return 0;
}


void DlgSpellsNotChargingUpdate(HWND hWnd)
{
	if(!hWnd) return;

	//

	if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_BLAST - 1)))
		CheckDlgButton(hWnd, IDC_SPELL_BLAST, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_BLAST, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_LIGHTNING - 1)))
		CheckDlgButton(hWnd, IDC_SPELL_LIGHTNING, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_LIGHTNING, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_TORNADO - 1)))
		CheckDlgButton(hWnd, IDC_SPELL_TORNADO, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_TORNADO, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_SWARM - 1)))
		CheckDlgButton(hWnd, IDC_SPELL_SWARM, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_SWARM, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_INVISIBILITY - 1)))
		CheckDlgButton(hWnd, IDC_SPELL_INVISIBILITY, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_INVISIBILITY, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_HYPNOTISM - 1)))
		CheckDlgButton(hWnd, IDC_SPELL_HYPNOTISM, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_HYPNOTISM, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_FIRESTORM - 1)))
		CheckDlgButton(hWnd, IDC_SPELL_FIRESTORM, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_FIRESTORM, BST_UNCHECKED);

	/*
	if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_GHOST_ARMY - 1)))
		CheckDlgButton(hWnd, IDC_SPELL_GHOST_ARMY, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_GHOST_ARMY, BST_UNCHECKED);
	*/

	if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_ERODE - 1)))
		CheckDlgButton(hWnd, IDC_SPELL_ERODE, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_ERODE, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_SWAMP - 1)))
		CheckDlgButton(hWnd, IDC_SPELL_SWAMP, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_SWAMP, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_LAND_BRIDGE - 1)))
		CheckDlgButton(hWnd, IDC_SPELL_LANDBRIDGE, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_LANDBRIDGE, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_ANGEL_OF_DEATH - 1)))
		CheckDlgButton(hWnd, IDC_SPELL_AOD, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_AOD, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_EARTHQUAKE - 1)))
		CheckDlgButton(hWnd, IDC_SPELL_EARTHQUAKE, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_EARTHQUAKE, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_FLATTEN - 1)))
		CheckDlgButton(hWnd, IDC_SPELL_FLATTEN, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_FLATTEN, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_VOLCANO - 1)))
		CheckDlgButton(hWnd, IDC_SPELL_VOLCANO, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_VOLCANO, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_CONVERT - 1)))
		CheckDlgButton(hWnd, IDC_SPELL_CONVERT, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_CONVERT, BST_UNCHECKED);

	if(leveldat->Header.v2.DefaultThings.SpellsNotCharging & (1 << (M_SPELL_MAGICAL_SHIELD - 1)))
		CheckDlgButton(hWnd, IDC_SPELL_MAGICALSHIELD, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_MAGICALSHIELD, BST_UNCHECKED);
}


// -=-=- header dialog -=-=-

void DlgHeaderToggle()
{
	if(hDlgHeader)
	{
		DestroyWindow(hDlgHeader);
		hDlgHeader = 0;
		DlgInfoUpdate(hDlgInfo);
	}
	else
	{
		hDlgHeader = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_HEADER), hMainWnd, DlgHeaderProc, 0);
		ShowWindow(hDlgHeader, SW_SHOW);
	}
}


int __stdcall DlgHeaderProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hWnd, IDC_HEADER_LEVEL_NAME, EM_LIMITTEXT, 31, 0);
		SendDlgItemMessage(hWnd, IDC_HEADER_N_PLAYERS, EM_LIMITTEXT, 3, 0);
		SendDlgItemMessage(hWnd, IDC_HEADER_CP_RED, EM_LIMITTEXT, 3, 0);
		SendDlgItemMessage(hWnd, IDC_HEADER_CP_YELLOW, EM_LIMITTEXT, 3, 0);
		SendDlgItemMessage(hWnd, IDC_HEADER_CP_GREEN, EM_LIMITTEXT, 3, 0);
		DlgHeaderUpdate(hWnd);
		return 0;

	case WM_CLOSE:
		DlgHeaderToggle();
		return 0;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_HEADER_LEVEL_NAME:
			if(EN_KILLFOCUS == HIWORD(wParam))
			{
				GetWindowText((HWND)lParam, str, sizeof(str));
                strcpy(leveldat->Header.v2.Name, str);
			}
			break;

		case IDC_HEADER_N_PLAYERS:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				{
					GetWindowText((HWND)lParam, str, sizeof(str));
					int i;
					if(GetInt(str, &i))
					{
						if(i < 1) i = 1;
						//else if(i > 4) i = 4;
						leveldat->Header.v2.NumPlayers = i;
					}
				}
				break;
			case EN_KILLFOCUS:
				sprintf(str, "%d", leveldat->Header.v2.NumPlayers);
				SendMessage((HWND)lParam, WM_SETTEXT, 0, (LPARAM)str);

				if (net.IsInitialized())
				{
					struct Packet *p = new Packet;
					p->wType = PACKETTYPE_NUM_PLAYERS;
					p->wData[0] = leveldat->Header.v2.NumPlayers;
					net.SendPacket(p);
					p->del();
				}
				break;
			}
			break;

		case IDC_HEADER_FOG:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				if(IsDlgButtonChecked(hWnd, IDC_HEADER_FOG) == BST_CHECKED)
					leveldat->Header.v2.LevelFlags |= LEVEL_FLAGS_USE_FOG;
				else
					leveldat->Header.v2.LevelFlags &= ~LEVEL_FLAGS_USE_FOG;

				if (net.IsInitialized())
				{
					struct Packet *p = new Packet;
					p->wType = PACKETTYPE_FOG_OF_WAR;
					p->wData[0] = IsDlgButtonChecked(hWnd, IDC_HEADER_FOG);
					net.SendPacket(p);
					p->del();
				}
			}
			break;

		case IDC_HEADER_GOD:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				if(IsDlgButtonChecked(hWnd, IDC_HEADER_GOD) == BST_CHECKED)
					leveldat->Header.v2.LevelFlags |= LEVEL_FLAGS_SHAMAN_OMNI;
				else
					leveldat->Header.v2.LevelFlags &= ~LEVEL_FLAGS_SHAMAN_OMNI;

				if (net.IsInitialized())
				{
					struct Packet *p = new Packet;
					p->wType = PACKETTYPE_OMNIPRESENCE;
					p->wData[0] = IsDlgButtonChecked(hWnd, IDC_HEADER_GOD);
					net.SendPacket(p);
					p->del();
				}
			}
			break;

		case IDC_HEADER_NO_GUEST:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				if(IsDlgButtonChecked(hWnd, IDC_HEADER_NO_GUEST) == BST_CHECKED)
					leveldat->Header.v2.LevelFlags |= LEVEL_FLAGS_NO_GUEST;
				else
					leveldat->Header.v2.LevelFlags &= ~LEVEL_FLAGS_NO_GUEST;

				if (net.IsInitialized())
				{
					struct Packet *p = new Packet;
					p->wType = PACKETTYPE_NO_GUEST;
					p->wData[0] = IsDlgButtonChecked(hWnd, IDC_HEADER_NO_GUEST);
					net.SendPacket(p);
					p->del();
				}
			}
			break;

		case IDC_HEADER_NO_REINCARNATE_TIME:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				if (IsDlgButtonChecked(hWnd, IDC_HEADER_NO_REINCARNATE_TIME) == BST_CHECKED)
					leveldat->Header.v2.LevelFlags |= LEVEL_NO_REINCARNATE_TIME;
				else
					leveldat->Header.v2.LevelFlags &= ~LEVEL_NO_REINCARNATE_TIME;

				if (net.IsInitialized())
				{
					struct Packet *p = new Packet;
					p->wType = PACKETTYPE_NO_REINCARNATE_TIME;
					p->wData[0] = IsDlgButtonChecked(hWnd, IDC_HEADER_NO_REINCARNATE_TIME);
					net.SendPacket(p);
					p->del();
				}
			}
			break;

		case IDC_HEADER_CP_RED:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				{
					GetWindowText((HWND)lParam, str, sizeof(str));
					int i;
					if(GetInt(str, &i))
					{
						if(i < 0) i = 0;
						else if(i > 255) i = 255;
						leveldat->Header.v2.ComputerPlayerIndex[0] = i;
					}
				}
				break;

			case EN_KILLFOCUS:
				sprintf(str, "%d", leveldat->Header.v2.ComputerPlayerIndex[0]);
				SendMessage((HWND)lParam, WM_SETTEXT, 0, (LPARAM)str);

				if (net.IsInitialized())
				{
					struct Packet *p = new Packet;
					p->wType = PACKETTYPE_COMPUTER_PLAYER_AI;
					p->wData[0] = OWNER_RED;
					p->wData[1] = leveldat->Header.v2.ComputerPlayerIndex[0];
					net.SendPacket(p);
					p->del();
				}
				break;
			}
			break;

		case IDC_HEADER_CP_YELLOW:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				{
					GetWindowText((HWND)lParam, str, sizeof(str));
					int i;
					if(GetInt(str, &i))
					{
						if(i < 0) i = 0;
						else if(i > 255) i = 255;
						leveldat->Header.v2.ComputerPlayerIndex[1] = i;
					}
				}
				break;

			case EN_KILLFOCUS:
				sprintf(str, "%d", leveldat->Header.v2.ComputerPlayerIndex[1]);
				SendMessage((HWND)lParam, WM_SETTEXT, 0, (LPARAM)str);

				if (net.IsInitialized())
				{
					struct Packet *p = new Packet;
					p->wType = PACKETTYPE_COMPUTER_PLAYER_AI;
					p->wData[0] = OWNER_YELLOW;
					p->wData[1] = leveldat->Header.v2.ComputerPlayerIndex[1];
					net.SendPacket(p);
					p->del();
				}
				break;
			}
			break;

		case IDC_HEADER_CP_GREEN:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				{
					GetWindowText((HWND)lParam, str, sizeof(str));
					int i;
					if(GetInt(str, &i))
					{
						if(i < 0) i = 0;
						else if(i > 255) i = 255;
						leveldat->Header.v2.ComputerPlayerIndex[2] = i;
					}
				}
				break;

			case EN_KILLFOCUS:
				sprintf(str, "%d", leveldat->Header.v2.ComputerPlayerIndex[2]);
				SendMessage((HWND)lParam, WM_SETTEXT, 0, (LPARAM)str);

				if (net.IsInitialized())
				{
					struct Packet *p = new Packet;
					p->wType = PACKETTYPE_COMPUTER_PLAYER_AI;
					p->wData[0] = OWNER_GREEN;
					p->wData[1] = leveldat->Header.v2.ComputerPlayerIndex[2];
					net.SendPacket(p);
					p->del();
				}
				break;
			}
			break;
		}
		break;
	}

	return 0;
}


void DlgHeaderUpdate(HWND hWnd)
{
	if(!hWnd) return;

	//

	leveldat->Header.v2.Name[31] = 0; // no overflow
	SetDlgItemText(hWnd, IDC_HEADER_LEVEL_NAME, leveldat->Header.v2.Name);

	sprintf(str, "%d", leveldat->Header.v2.NumPlayers);
	SetDlgItemText(hWnd, IDC_HEADER_N_PLAYERS, str);

	sprintf(str, "%d", leveldat->Header.v2.ComputerPlayerIndex[0]);
	SetDlgItemText(hWnd, IDC_HEADER_CP_RED, str);

	sprintf(str, "%d", leveldat->Header.v2.ComputerPlayerIndex[1]);
	SetDlgItemText(hWnd, IDC_HEADER_CP_YELLOW, str);

	sprintf(str, "%d", leveldat->Header.v2.ComputerPlayerIndex[2]);
	SetDlgItemText(hWnd, IDC_HEADER_CP_GREEN, str);

	if(leveldat->Header.v2.LevelFlags & LEVEL_FLAGS_USE_FOG)
		CheckDlgButton(hWnd, IDC_HEADER_FOG, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_HEADER_FOG, BST_UNCHECKED);

	if(leveldat->Header.v2.LevelFlags & LEVEL_FLAGS_SHAMAN_OMNI)
		CheckDlgButton(hWnd, IDC_HEADER_GOD, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_HEADER_GOD, BST_UNCHECKED);

	if(leveldat->Header.v2.LevelFlags & LEVEL_FLAGS_NO_GUEST)
		CheckDlgButton(hWnd, IDC_HEADER_NO_GUEST, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_HEADER_NO_GUEST, BST_UNCHECKED);

	if (leveldat->Header.v2.LevelFlags & LEVEL_NO_REINCARNATE_TIME)
		CheckDlgButton(hWnd, IDC_HEADER_NO_REINCARNATE_TIME, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_HEADER_NO_REINCARNATE_TIME, BST_UNCHECKED);
}


// -=-=- obj bank dialog -=-=-

void DlgObjBankToggle()
{
	if(hDlgObjBank)
	{
		DestroyWindow(hDlgObjBank);
		hDlgObjBank = 0;
	}
	else
	{
		hDlgObjBank = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_OBJ_BANK), hMainWnd, DlgObjBankProc, 0);
		ShowWindow(hDlgObjBank, SW_SHOW);
	}
}


void DlgObjBankPaint(HDC hdc)
{
	HBITMAP hbmp = 0;

	switch(leveldat->Header.v2.ObjectsBankNum)
	{
	case 0: hbmp = hBank0; break;
	case 2: hbmp = hBank2; break;
	case 3: hbmp = hBank3; break;
	case 4: hbmp = hBank4; break;
	case 5: hbmp = hBank5; break;
	case 6: hbmp = hBank6; break;
	case 7: hbmp = hBank7; break;
	}

	HDC hdcbmp = CreateCompatibleDC(hdc);
	if(hdcbmp)
	{
		SelectObject(hdcbmp, hbmp);
		BitBlt(hdc, 10, 10, 115, 70, hdcbmp, 0, 0, SRCCOPY);
		DeleteDC(hdcbmp);
	}
}


void DlgObjBankUpdate(HWND hWnd)
{
	if(!hWnd) return;

	//

	HWND hItem = GetDlgItem(hWnd, IDC_OBJ_BANK);
	if(hItem)
	{
		switch(leveldat->Header.v2.ObjectsBankNum)
		{
		case 0: SendMessage(hItem, CB_SETCURSEL, 0, 0); break;
		case 2: SendMessage(hItem, CB_SETCURSEL, 1, 0); break;
		case 3: SendMessage(hItem, CB_SETCURSEL, 2, 0); break;
		case 4: SendMessage(hItem, CB_SETCURSEL, 3, 0); break;
		case 5: SendMessage(hItem, CB_SETCURSEL, 4, 0); break;
		case 6: SendMessage(hItem, CB_SETCURSEL, 5, 0); break;
		case 7: SendMessage(hItem, CB_SETCURSEL, 6, 0); break;
		default: SendMessage(hItem, CB_SETCURSEL, -1, 0);
		}
	}

	HDC hdc = GetDC(hWnd);
	DlgObjBankPaint(hdc);
	ReleaseDC(hWnd, hdc);
}


int __stdcall DlgObjBankProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND hItem = GetDlgItem(hWnd, IDC_OBJ_BANK);
			if(!hItem) return 0;

			SendMessage(hItem, CB_RESETCONTENT, 0, 0);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OBJ_BANK_0);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OBJ_BANK_2);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OBJ_BANK_3);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OBJ_BANK_4);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OBJ_BANK_5);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OBJ_BANK_6);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OBJ_BANK_7);

			DlgObjBankUpdate(hWnd);
		}
		return 0;

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			DlgObjBankPaint(ps.hdc);
			EndPaint(hWnd, &ps);
		}
		return 0;

	case WM_CLOSE:
		DlgObjBankToggle();
		return 0;

	case WM_COMMAND:
		if(LOWORD(wParam) != IDC_OBJ_BANK) return 0;
		if(HIWORD(wParam) != CBN_SELCHANGE) return 0;

		switch(SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))
		{
		case 0: leveldat->Header.v2.ObjectsBankNum = 0; break;
		case 1: leveldat->Header.v2.ObjectsBankNum = 2; break;
		case 2: leveldat->Header.v2.ObjectsBankNum = 3; break;
		case 3: leveldat->Header.v2.ObjectsBankNum = 4; break;
		case 4: leveldat->Header.v2.ObjectsBankNum = 5; break;
		case 5: leveldat->Header.v2.ObjectsBankNum = 6; break;
		case 6: leveldat->Header.v2.ObjectsBankNum = 7; break;
		default: leveldat->Header.v2.ObjectsBankNum = 0;
		}

		if (net.IsInitialized())
		{
			struct Packet *p = new Packet;
			p->wType = PACKETTYPE_OBJECT_BANK;
			p->wData[0] = leveldat->Header.v2.ObjectsBankNum;
			net.SendPacket(p);
			p->del();
		}

		EngineSetTreeType();

		{
			HDC hdc = GetDC(hWnd);
			DlgObjBankPaint(hdc);
			ReleaseDC(hWnd, hdc);
		}

		return 0;
	}

	return 0;
}


// -=-=- map type dialog -=-=-

void DlgMapTypeToggle()
{
	if(hDlgMapType)
	{
		DestroyWindow(hDlgMapType);
		hDlgMapType = 0;
	}
	else
	{
		hDlgMapType = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MAP_TYPE), hMainWnd, DlgMapTypeProc, 0);
		ShowWindow(hDlgMapType, SW_SHOW);
	}
}


void DlgMapTypePaint(HDC hdc)
{
	HBITMAP hbmp = 0;

	switch(leveldat->Header.v2.LevelType)
	{
	case 0: hbmp = hMap0; break;
	case 1: hbmp = hMap1; break;
	case 2: hbmp = hMap2; break;
	case 3: hbmp = hMap3; break;
	case 4: hbmp = hMap4; break;
	case 5: hbmp = hMap5; break;
	case 6: hbmp = hMap6; break;
	case 7: hbmp = hMap7; break;
	case 8: hbmp = hMap8; break;
	case 9: hbmp = hMap9; break;
	case 10: hbmp = hMapA; break;
	case 11: hbmp = hMapB; break;
	case 12: hbmp = hMapC; break;
	case 13: hbmp = hMapD; break;
	case 14: hbmp = hMapE; break;
	case 15: hbmp = hMapF; break;
	case 16: hbmp = hMapG; break;
	case 17: hbmp = hMapH; break;
	case 18: hbmp = hMapI; break;
	case 19: hbmp = hMapJ; break;
	case 20: hbmp = hMapK; break;
	case 21: hbmp = hMapL; break;
	case 22: hbmp = hMapM; break;
	case 23: hbmp = hMapN; break;
	case 24: hbmp = hMapO; break;
	case 25: hbmp = hMapP; break;
	case 26: hbmp = hMapQ; break;
	case 27: hbmp = hMapR; break;
	case 28: hbmp = hMapS; break;
	case 29: hbmp = hMapT; break;
	case 30: hbmp = hMapU; break;
	case 31: hbmp = hMapV; break;
	case 32: hbmp = hMapW; break;
	case 33: hbmp = hMapX; break;
	case 34: hbmp = hMapY; break;
	case 35: hbmp = hMapZ; break;
	}

	HDC hdcbmp = CreateCompatibleDC(hdc);
	if(hdcbmp)
	{
		SelectObject(hdcbmp, hbmp);
		BitBlt(hdc, 10, 10, 120, 120, hdcbmp, 0, 0, SRCCOPY);
		DeleteDC(hdcbmp);
	}
}


void DlgMapTypeUpdate(HWND hWnd)
{
	if(!hWnd) return;

	//

	HWND hItem = GetDlgItem(hWnd, IDC_MAP_TYPE);
	if(hItem)
	{
		int i = leveldat->Header.v2.LevelType;
		if(i >= 0 && i <= 35)
			SendMessage(hItem, CB_SETCURSEL, i, 0);
		else
			SendMessage(hItem, CB_SETCURSEL, -1, 0);
	}

	HDC hdc = GetDC(hWnd);
	DlgMapTypePaint(hdc);
	ReleaseDC(hWnd, hdc);
}


int __stdcall DlgMapTypeProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND hItem = GetDlgItem(hWnd, IDC_MAP_TYPE);
			if(!hItem) return 0;

			SendMessage(hItem, CB_RESETCONTENT, 0, 0);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_0);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_1);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_2);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_3);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_4);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_5);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_6);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_7);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_8);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_9);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_A);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_B);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_C);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_D);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_E);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_F);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_G);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_H);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_I);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_J);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_K);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_L);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_M);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_N);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_O);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_P);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_Q);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_R);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_S);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_T);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_U);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_V);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_W);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_X);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_Y);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_MAP_Z);

			DlgMapTypeUpdate(hWnd);
		}
		return 0;

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			DlgMapTypePaint(ps.hdc);
			EndPaint(hWnd, &ps);
		}
		return 0;

	case WM_CLOSE:
		DlgMapTypeToggle();
		return 0;

	case WM_COMMAND:
		if(LOWORD(wParam) != IDC_MAP_TYPE) return 0;
		if(HIWORD(wParam) != CBN_SELCHANGE) return 0;

		int i = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
		if(i >= 0 && i <= 35)
			leveldat->Header.v2.LevelType = i;
		else
			leveldat->Header.v2.LevelType = 0;

		if (net.IsInitialized())
		{
			struct Packet *p = new Packet;
			p->wType = PACKETTYPE_MAP_TYPE;
			p->wData[0] = leveldat->Header.v2.LevelType;
			net.SendPacket(p);
			p->del();
		}

		{
			HDC hdc = GetDC(hWnd);
			DlgMapTypePaint(hdc);
			ReleaseDC(hWnd, hdc);
		}

		return 0;
	}

	return 0;
}


// -=-=- info dialog -=-=-

void DlgInfoToggle()
{
	if(hDlgInfo)
	{
		DestroyWindow(hDlgInfo);
		hDlgInfo = 0;
	}
	else
	{
		hDlgInfo = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_INFO), hMainWnd, DlgInfoProc, 0);
		ShowWindow(hDlgInfo, SW_SHOW);
	}
}


int __stdcall DlgInfoProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		DlgInfoUpdate(hWnd);
		return 0;
	case WM_CLOSE:
		DlgInfoToggle();
		return 0;
	}

	return 0;
}


void DlgInfoUpdate(HWND hWnd)
{
	if(!hWnd) return;

	//

	SetDlgItemText(hWnd, IDC_INFO_LEVEL_NAME, leveldat->Header.v2.Name);

	strcpy(str, szLevel);
	PathStripPath(str);
	SetDlgItemText(hWnd, IDC_INFO_FILENAME, str);

	strcpy(str, szLevel);
	PathRemoveFileSpec(str);
	SetDlgItemText(hWnd, IDC_INFO_PATH, str);

	sprintf(str, "%d", ObjectsCount);
	SetDlgItemText(hWnd, IDC_INFO_OBJ_COUNT, str);

	int Tree = 0;
	int Wildman = 0;
	int StoneHead = 0;
	int Knowledge = 0;
	int Triggers = 0;
	int Discovery = 0;
	int PopBlue = 0;
	int PopRed = 0;
	int PopYellow = 0;
	int PopGreen = 0;
	int PopCyan = 0;
	int PopMagenta = 0;
	int PopBlack = 0;
	int PopOrange = 0;
	int PopNeutral = 0;
	int BlgBlue = 0;
	int BlgRed = 0;
	int BlgYellow = 0;
	int BlgGreen = 0;
	int BlgCyan = 0;
	int BlgMagenta = 0;
	int BlgBlack = 0;
	int BlgOrange = 0;
	int BlgNeutral = 0;
	int Decorations = 0;

	if(Things)
	{
		THING *t = Things;
		do
		{
			if(t->Thing.Type == T_PERSON)
			{
				if(t->Thing.Model == M_PERSON_WILD) Wildman++;

				switch(t->Thing.Owner)
				{
				case OWNER_BLUE:	PopBlue++;		break;
				case OWNER_RED:		PopRed++;		break;
				case OWNER_YELLOW:	PopYellow++;	break;
				case OWNER_GREEN:	PopGreen++;		break;
				case OWNER_CYAN:	PopCyan++;		break;
				case OWNER_MAGENTA:	PopMagenta++;	break;
				case OWNER_BLACK:	PopBlack++;		break;
				case OWNER_ORANGE:	PopOrange++;	break;
				default: PopNeutral++;
				}
			}
			else if(t->Thing.Type == T_BUILDING)
			{
				if(t->Thing.Model == M_BUILDING_KNOWLEDGE) Knowledge++;

				switch(t->Thing.Owner)
				{
				case OWNER_BLUE:    BlgBlue++;		break;
				case OWNER_RED:     BlgRed++;		break;
				case OWNER_YELLOW:  BlgYellow++;	break;
				case OWNER_GREEN:   BlgGreen++;		break;
				case OWNER_CYAN:	BlgCyan++;		break;
				case OWNER_MAGENTA:	BlgMagenta++;	break;
				case OWNER_BLACK:	BlgBlack++;		break;
				case OWNER_ORANGE:	BlgOrange++;	break;
				default: BlgNeutral++;
				}
			}
			else if(t->Thing.Type == T_SCENERY)
			{
				if((t->Thing.Model == M_SCENERY_TREE_1) || (t->Thing.Model == M_SCENERY_TREE_2) || (t->Thing.Model == M_SCENERY_TREE_3) || (t->Thing.Model == M_SCENERY_TREE_4) || (t->Thing.Model == M_SCENERY_TREE_5) || (t->Thing.Model == M_SCENERY_TREE_6)) Tree++;
				else if(t->Thing.Model == M_SCENERY_STONE_HEAD) StoneHead++;

				if (t->Thing.Model == M_SCENERY_SUB_LEVEL_SCENERY)
					Decorations++;
			}
			else if(t->Thing.Type == T_GENERAL)
			{
				if(t->Thing.Model == M_GENERAL_TRIGGER) Triggers++;
				else if(t->Thing.Model == M_GENERAL_DISCOVERY) Discovery++;
			}

			t = t->Next;
		}
		while(Things != t);
	}

	sprintf(str, "%d", Tree);
	SetDlgItemText(hWnd, IDC_INFO_TREE_COUNT, str);
	sprintf(str, "%d", Wildman);
	SetDlgItemText(hWnd, IDC_INFO_WILDMAN_COUNT, str);
	sprintf(str, "%d", StoneHead);
	SetDlgItemText(hWnd, IDC_INFO_STONEHEAD_COUNT, str);
	sprintf(str, "%d", Knowledge);
	SetDlgItemText(hWnd, IDC_INFO_KNOWLEDGE_COUNT, str);
	sprintf(str, "%d", Discovery);
	SetDlgItemText(hWnd, IDC_INFO_DISCOVERY_COUNT, str);
	sprintf(str, "%d", Triggers);
	SetDlgItemText(hWnd, IDC_INFO_TRIGGER_COUNT, str);
	sprintf(str, "%d", Decorations);
	SetDlgItemText(hWnd, IDC_INFO_DECORATIONS_COUNT, str);

	sprintf(str, "%d", PopBlue);
	SetDlgItemText(hWnd, IDC_INFO_POP_BLUE, str);
	sprintf(str, "%d", PopRed);
	SetDlgItemText(hWnd, IDC_INFO_POP_RED, str);
	sprintf(str, "%d", PopYellow);
	SetDlgItemText(hWnd, IDC_INFO_POP_YELLOW, str);
	sprintf(str, "%d", PopGreen);
	SetDlgItemText(hWnd, IDC_INFO_POP_GREEN, str);
	sprintf(str, "%d", PopCyan);
	SetDlgItemText(hWnd, IDC_INFO_POP_CYAN, str);
	sprintf(str, "%d", PopMagenta);
	SetDlgItemText(hWnd, IDC_INFO_POP_MAGENTA, str);
	sprintf(str, "%d", PopBlack);
	SetDlgItemText(hWnd, IDC_INFO_POP_BLACK, str);
	sprintf(str, "%d", PopOrange);
	SetDlgItemText(hWnd, IDC_INFO_POP_ORANGE, str);
	sprintf(str, "%d", PopNeutral);
	SetDlgItemText(hWnd, IDC_INFO_POP_NEUTRAL, str);

	sprintf(str, "%d", BlgBlue);
	SetDlgItemText(hWnd, IDC_INFO_BLG_BLUE, str);
	sprintf(str, "%d", BlgRed);
	SetDlgItemText(hWnd, IDC_INFO_BLG_RED, str);
	sprintf(str, "%d", BlgYellow);
	SetDlgItemText(hWnd, IDC_INFO_BLG_YELLOW, str);
	sprintf(str, "%d", BlgGreen);
	SetDlgItemText(hWnd, IDC_INFO_BLG_GREEN, str);
	sprintf(str, "%d", BlgCyan);
	SetDlgItemText(hWnd, IDC_INFO_BLG_CYAN, str);
	sprintf(str, "%d", BlgMagenta);
	SetDlgItemText(hWnd, IDC_INFO_BLG_MAGENTA, str);
	sprintf(str, "%d", BlgBlack);
	SetDlgItemText(hWnd, IDC_INFO_BLG_BLACK, str);
	sprintf(str, "%d", BlgOrange);
	SetDlgItemText(hWnd, IDC_INFO_BLG_ORANGE, str);
	sprintf(str, "%d", BlgNeutral);
	SetDlgItemText(hWnd, IDC_INFO_BLG_NEUTRAL, str);
}


// -=-=- swap tribe dialog -=-=-

void DlgSwapTribeToggle()
{
	if(hDlgSwapTribe)
	{
		DestroyWindow(hDlgSwapTribe);
		hDlgSwapTribe = 0;
	}
	else
	{
		hDlgSwapTribe = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_SWAP_TRIBE), hMainWnd, DlgSwapTribeProc, 0);
		ShowWindow(hDlgSwapTribe, SW_SHOW);
	}
}


void DlgSwapTribeUpdate(HWND hWnd)
{
	// EXIT IF THE WINDOW HANDLE IS INVALID.
	if(!hWnd) return;

	// Create a lambda function for setting the enablement state of a tribe swap drop-down box to making
	// updating the drop-down simpler.
	const auto set_tribe_swap_drop_down_enablement = [hWnd](const int dialog_item_index, const bool enabled)
	{
		// RETRIEVE THE DROP-DOWN BOX.
		HWND hItem = GetDlgItem(hWnd, dialog_item_index);
		const bool item_handle_valid = (NULL != hItem);
		if (!item_handle_valid)
		{
			// The drop-down cannot be updated since it is not valid.
			return;
		}

		// SET THE SELECTED TRIBE.
		constexpr int NO_SELECTION = -1;
		WPARAM selected_tribe_index = enabled?
			BLUE_TRIBE_SWAP_DROP_DOWN_INDEX :
			NO_SELECTION;
		SendMessage(hItem, CB_SETCURSEL, selected_tribe_index, 0);

		// UPDATE THE ENABLED STATE.
		EnableWindow(hItem, static_cast<BOOL>(enabled));
	};

	// UPDATE THE DROP-DOWN CONTROLS FOR THE SECOND SWAP.
	const bool swap_two_enabled = (IsDlgButtonChecked(hWnd, IDC_SWAPTRIBE_SWAP2) == BST_CHECKED);
	set_tribe_swap_drop_down_enablement(IDC_SECOND_SWAP_TRIBE_SOURCE_DROP_DOWN, swap_two_enabled);
	set_tribe_swap_drop_down_enablement(IDC_SECOND_SWAP_TRIBE_DESTINATION_DROP_DOWN, swap_two_enabled);
}


void DlgSwapTribeSwap(HWND hWnd)
{
	if(!Things) return;
	SaveThingsUndoState();

	// Create a lambda function to simplify retrieving the selected tribe from each drop-down.
	const auto get_selected_tribe_from_drop_down = [hWnd](const int dialog_item_index) -> int
	{
		// RETRIEVE THE DROP-DOWN BOX.
		HWND hItem = GetDlgItem(hWnd, dialog_item_index);
		const bool item_handle_valid = (NULL != hItem);
		if (!item_handle_valid)
		{
			// The tribe cannot be retrieved since the drop-down is invalid.
			return CB_ERR;
		}

		// RETRIEVE THE TRIBE FROM THE DROP-DOWN VALUE.
		const auto drop_down_value = SendMessage(hItem, CB_GETCURSEL, 0, 0);
		switch (drop_down_value)
		{
		case BLUE_TRIBE_SWAP_DROP_DOWN_INDEX:
			return OWNER_BLUE;
		case RED_TRIBE_SWAP_DROP_DOWN_INDEX:
			return OWNER_RED;
		case YELLOW_TRIBE_SWAP_DROP_DOWN_INDEX:
			return OWNER_YELLOW;
		case GREEN_TRIBE_SWAP_DROP_DOWN_INDEX:
			return OWNER_GREEN;
		case CYAN_TRIBE_SWAP_DROP_DOWN_INDEX:
			return OWNER_CYAN;
		case MAGENTA_TRIBE_SWAP_DROP_DOWN_INDEX:
			return OWNER_MAGENTA;
		case BLACK_TRIBE_SWAP_DROP_DOWN_INDEX:
			return OWNER_BLACK;
		case ORANGE_TRIBE_SWAP_DROP_DOWN_INDEX:
			return OWNER_ORANGE;
		case NEUTRAL_TRIBE_SWAP_DROP_DOWN_INDEX:
			return OWNER_NEUTRAL;
		default:
			return CB_ERR;
		}
	};

	// DETERMINE WHICH TRIBES WILL BE SWAPPED FOR THE FIRST SWAP.
	int t1 = get_selected_tribe_from_drop_down(IDC_FIRST_SWAP_TRIBE_SOURCE_DROP_DOWN);
	int t2 = get_selected_tribe_from_drop_down(IDC_FIRST_SWAP_TRIBE_DESTINATION_DROP_DOWN);
	const bool first_swap_valid = (
		(CB_ERR != t1) &&
		(CB_ERR != t2));
	if (!first_swap_valid)
	{
		// No tribe swap can take place since the first swap had invalid selections.
		return;
	}

	// CHECK TO SEE IF TWO SWAPS ARE ENABLED.
	int t3, t4;
	bool perform_two_swaps = IsDlgButtonChecked(hWnd, IDC_SWAPTRIBE_SWAP2) == BST_CHECKED;
	if(perform_two_swaps)
	{
		// DETERMINE WHICH TRIBES WILL BE SWAPPED FOR THE SECOND SWAP.
		t3 = get_selected_tribe_from_drop_down(IDC_SECOND_SWAP_TRIBE_SOURCE_DROP_DOWN);
		t4 = get_selected_tribe_from_drop_down(IDC_SECOND_SWAP_TRIBE_DESTINATION_DROP_DOWN);
		const bool second_swap_valid = (
			(CB_ERR != t3) &&
			(CB_ERR != t4));
		if (!second_swap_valid)
		{
			// The second swap will not be performed since one or more of its selections were invalid.
			perform_two_swaps = false;
		}
	}

	if (net.IsInitialized())
	{
		struct Packet *p = new Packet;
		p->wType = PACKETTYPE_SWAP_TRIBE;
		p->wData[0] = t1;
		p->wData[1] = t2;
		p->wData[2] = t3;
		p->wData[3] = t4;
		p->wData[4] = perform_two_swaps;
		net.SendPacket(p);
		p->del();
	}

	// PERFORM THE TRIBE SWAP.
	THING *t = Things;
	do
	{
		if(IsNotNeutral(t))
		{
			if(t1 == t->Thing.Owner)
				t->Thing.Owner = t2;
			else if(perform_two_swaps && t3 == t->Thing.Owner)
				t->Thing.Owner = t4;
		}

		t = t->Next;
	}
	while(t != Things);

	DlgInfoUpdate(hDlgInfo);
	DlgObjectUpdateInfo(hWnd);
	EngineUpdateMiniMap();
}


int __stdcall DlgSwapTribeProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Create a lambda function for populating the drop-down box used for swapping tribes.
	// This makes it simpler to initialize the drop-down box.
	const auto populate_tribe_swap_drop_down = [hWnd](const int dialog_item_index)
	{
		// RETRIEVE THE DROP-DOWN BOX.
		HWND hItem = GetDlgItem(hWnd, dialog_item_index);
		const bool item_handle_valid = (NULL != hItem);
		if (!item_handle_valid)
		{
			// The drop-down cannot be populated since it is not valid.
			return;
		}

		// POPULATE THE DROP-DOWN.
		SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OWNER_BLUE);
		SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OWNER_RED);
		SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OWNER_YELLOW);
		SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OWNER_GREEN);
		SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OWNER_CYAN);
		SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OWNER_MAGENTA);
		SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OWNER_BLACK);
		SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OWNER_ORANGE);
		SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_OWNER_NEUTRAL);

		// SELECT THE BLUE TRIBE, BY DEFAULT.
		SendMessage(hItem, CB_SETCURSEL, BLUE_TRIBE_SWAP_DROP_DOWN_INDEX, 0);
	};

	switch(uMsg)
	{
	case WM_INITDIALOG:
		// POPULATE THE DROP-DOWNS FOR SWAPPING TRIBES.
		populate_tribe_swap_drop_down(IDC_FIRST_SWAP_TRIBE_SOURCE_DROP_DOWN);
		populate_tribe_swap_drop_down(IDC_FIRST_SWAP_TRIBE_DESTINATION_DROP_DOWN);
		populate_tribe_swap_drop_down(IDC_SECOND_SWAP_TRIBE_SOURCE_DROP_DOWN);
		populate_tribe_swap_drop_down(IDC_SECOND_SWAP_TRIBE_DESTINATION_DROP_DOWN);

		// CHECK THE BOX FOR PERMITTING TWO SWAPS TO TAKE PLACE BY DEFAULT.
		CheckDlgButton(hWnd, IDC_SWAPTRIBE_SWAP2, BST_CHECKED);
		return 0;

	case WM_COMMAND:
		switch (wParam)
		{
		case IDC_FIRST_SWAP_TRIBE_SOURCE_DROP_DOWN:
		case IDC_FIRST_SWAP_TRIBE_DESTINATION_DROP_DOWN:
		case IDC_SECOND_SWAP_TRIBE_SOURCE_DROP_DOWN:
		case IDC_SECOND_SWAP_TRIBE_DESTINATION_DROP_DOWN:
		case IDC_SWAPTRIBE_SWAP2:
			DlgSwapTribeUpdate(hWnd);
			break;
		case IDC_SWAPTRIBE_SWAP:
			DlgSwapTribeSwap(hWnd);
			break;
		}

		return 0;

	case WM_CLOSE:
		DlgSwapTribeToggle();
		return 0;
	}

	return 0;
}


// -=-=- markers dialog -=-=-

void DlgMarkersToggle()
{
	if(hDlgMarkers)
	{
		DestroyWindow(hDlgMarkers);
		hDlgMarkers = 0;
		MarkerSelected = -1;
		fEngineEditMarkers = 0;
	}
	else
	{
		hDlgMarkers = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MARKERS), hMainWnd, DlgMarkersProc, 0);
		ShowWindow(hDlgMarkers, SW_SHOW);
	}
}


void DlgMarkersUpdate(HWND hWnd)
{
	if(!hWnd) return;
	HWND hButton = GetDlgItem(hWnd, IDC_MARKERS_COPY);

	if(MarkerSelected == -1)
	{
		SetDlgItemText(hWnd, IDC_MARKERS_N, "-");
		SetDlgItemText(hWnd, IDC_MARKERS_POS, "-");
	}
	else
	{
		sprintf(str, "%d", MarkerSelected);
		SetDlgItemText(hWnd, IDC_MARKERS_N, str);

		sprintf(str, "%d, %d", (leveldat->Header.v2.Markers[MarkerSelected] & 0xFF), (leveldat->Header.v2.Markers[MarkerSelected] >> 8));
		SetDlgItemText(hWnd, IDC_MARKERS_POS, str);
	}
}


void DlgMarkersCenterSelected()
{
	if(MarkerSelected == -1) return;

	fEnginePosX = Markers[MarkerSelected].x;
	fEnginePosZ = Markers[MarkerSelected].z;
	EngineUpdateView();
	DlgMarkersUpdate(hDlgMarkers);
}


int __stdcall DlgMarkersProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		if(HIWORD(wParam) == BN_CLICKED)
		switch(LOWORD(wParam))
		{
		case IDC_MARKERS_NEXT:
			MarkerSelected++;
			if(MarkerSelected > 255) MarkerSelected = 0;
			DlgMarkersCenterSelected();
			break;
		case IDC_MARKERS_PREV:
			MarkerSelected--;
			if(MarkerSelected < 0) MarkerSelected = 255;
			DlgMarkersCenterSelected();
			break;
		case IDC_MARKERS_COPY:
			if (MarkerSelected != -1)
			{
				sprintf(str, "%d %d", leveldat->Header.v2.Markers[MarkerSelected] & 0xFF, leveldat->Header.v2.Markers[MarkerSelected] >> 8);
				CopyToClipboard(hWnd, str);
			}
			break;
		}
		return 0;

	case WM_CLOSE:
		DlgMarkersToggle();
		return 0;
	}

	return 0;
}


// -=-=- ai attrib dialog -=-=-

void DlgAIAttribToggle()
{
	if(hDlgAIAttrib)
	{
		DestroyWindow(hDlgAIAttrib);
		hDlgAIAttrib = 0;
	}
	else
	{
		hDlgAIAttrib = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_AI_ATTRIB), hMainWnd, DlgAIAttribProc, 0);
		ShowWindow(hDlgAIAttrib, SW_SHOW);
	}
}


int __stdcall DlgAIAttribProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			DlgAIAttribNew();

			SendDlgItemMessage(hWnd, IDC_AI_CP_NAME, EM_LIMITTEXT, 31, 0);
			SendDlgItemMessage(hWnd, IDC_AI_ATTRIB, EM_LIMITTEXT, 3, 0);

			HWND hItem;

			hItem = GetDlgItem(hWnd, IDC_AI_ATTRIB_LIST);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_EXPANSION);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_PREF_SPY_TRAINS);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_PREF_RELIGIOUS_TRAINS);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_PREF_WARRIOR_TRAINS);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_PREF_FIREWARRIOR_TRAINS);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_PREF_SPY_PEOPLE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_PREF_RELIGIOUS_PEOPLE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_PREF_WARRIOR_PEOPLE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_PREF_FIREWARRIOR_PEOPLE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_MAX_BUILDINGS_ON_GO);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_HOUSE_PERCENTAGE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_AWAY_BRAVE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_AWAY_WARRIOR);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_AWAY_RELIGIOUS);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_DEFENSE_RAD_INCR);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_MAX_DEFENSIVE_ACTIONS);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_AWAY_SPY);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_AWAY_FIREWARRIOR);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_ATTACK_PERCENTAGE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_AWAY_SHAMAN);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_PEOPLE_PER_BOAT);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_PEOPLE_PER_BALLOON);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_DONT_USE_BOATS);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_MAX_SPY_ATTACKS);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_ENEMY_SPY_MAX_STAND);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_MAX_ATTACKS);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_EMPTY_AT_WAYPOINT);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_SPY_CHECK_FREQUENCY);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_RETREAT_VALUE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_BASE_UNDER_ATTACK_RETREAT);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_RANDOM_BUILD_SIDE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_USE_PREACHER_FOR_DEFENCE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_SHAMEN_BLAST);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_MAX_TRAIN_AT_ONCE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_GROUP_OPTION);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_PREF_BOAT_HUTS);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_PREF_BALLOON_HUTS);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_PREF_BOAT_DRIVERS);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_PREF_BALLOON_DRIVERS);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_FIGHT_STOP_DISTANCE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_SPY_DISCOVER_CHANCE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_COUNT_PREACH_DAMAGE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_DONT_GROUP_AT_DT);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_SPELL_DELAY);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_DONT_DELETE_USELESS_BOAT_HOUSE);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_BOAT_HOUSE_BROKEN);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_DONT_AUTO_TRAIN_PREACHERS);
			SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)SZ_INT_ATTR_SPARE_6);

			SendMessage(hItem, CB_SETCURSEL, 0, 0);

			DlgAIAttribUpdate(hWnd);
		}
		return 0;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_AI_CP_NAME:
			if(EN_KILLFOCUS == HIWORD(wParam))
			{
				GetWindowText((HWND)lParam, str, sizeof(str));
				if(lstrcmpi(str, CPAttrib.Name) != 0)
				{
					strcpy(CPAttrib.Name, str);
					fAIAttribChanged = true;
				}
			}
			break;
		case IDC_AI_ATTRIB_LIST:
			if(HIWORD(wParam) == CBN_SELCHANGE)	DlgAIAttribUpdateAttrib(hWnd);
			break;
		case IDC_AI_ATTRIB:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				{
					GetWindowText((HWND)lParam, str, sizeof(str));
					int i;
					if(GetInt(str, &i))
					{
						if(i < 0) i = 0;
						else if(i > 255) i = 255;

						int c = SendMessage(GetDlgItem(hWnd, IDC_AI_ATTRIB_LIST), CB_GETCURSEL, 0, 0);
						if(c >= 0 && c < 48)
						{
							if(CPAttrib.Attribs[c] != i)
							{
								CPAttrib.Attribs[c] = i;
								fAIAttribChanged = true;
							}
						}
					}
				}
				break;
			case EN_KILLFOCUS:
				DlgAIAttribUpdateAttrib(hWnd);
				break;
			}
			break;

		case IDC_AI_NEW:
			if(fAIAttribChanged)
			{
				switch(ModalMsg(SZ_AIATTRIB_SAVE_CHANGE, APPNAME, MB_ICONEXCLAMATION | MB_YESNOCANCEL))
				{
				case IDYES: if(DlgAIAttribSave()) DlgAIAttribNew(); break;
				case IDNO:  DlgAIAttribNew(); break;
				// case IDCANCEL: break;
				}
			}
			else
			{
				DlgAIAttribNew();
			}
			break;
		case IDC_AI_OPEN:
			if(fAIAttribChanged)
			{
				switch(ModalMsg(SZ_AIATTRIB_SAVE_CHANGE, APPNAME, MB_ICONEXCLAMATION | MB_YESNOCANCEL))
				{
				case IDYES: if(DlgAIAttribSave()) DlgAIAttribOpen(); break;
				case IDNO:  DlgAIAttribOpen(); break;
				// case IDCANCEL: break;
				}
			}
			else
			{
				DlgAIAttribOpen();
			}
			break;
		case IDC_AI_SAVE:
			DlgAIAttribSave();
			break;

		// spells

		case IDC_SPELL_BLAST:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_BLAST))
				CPAttrib.StartingThings.SpellsAvailable &= ~(1 << M_SPELL_BLAST);
			else
				CPAttrib.StartingThings.SpellsAvailable |= (1 << M_SPELL_BLAST);
			break;
		case IDC_SPELL_LIGHTNING:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_LIGHTNING))
				CPAttrib.StartingThings.SpellsAvailable &= ~(1 << M_SPELL_LIGHTNING);
			else
				CPAttrib.StartingThings.SpellsAvailable |= (1 << M_SPELL_LIGHTNING);
			break;
		case IDC_SPELL_TORNADO:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_TORNADO))
				CPAttrib.StartingThings.SpellsAvailable &= ~(1 << M_SPELL_TORNADO);
			else
				CPAttrib.StartingThings.SpellsAvailable |= (1 << M_SPELL_TORNADO);
			break;
		case IDC_SPELL_SWARM:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_SWARM))
				CPAttrib.StartingThings.SpellsAvailable &= ~(1 << M_SPELL_SWARM);
			else
				CPAttrib.StartingThings.SpellsAvailable |= (1 << M_SPELL_SWARM);
			break;
		case IDC_SPELL_INVISIBILITY:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_INVISIBILITY))
				CPAttrib.StartingThings.SpellsAvailable &= ~(1 << M_SPELL_INVISIBILITY);
			else
				CPAttrib.StartingThings.SpellsAvailable |= (1 << M_SPELL_INVISIBILITY);
			break;
		case IDC_SPELL_HYPNOTISM:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_HYPNOTISM))
				CPAttrib.StartingThings.SpellsAvailable &= ~(1 << M_SPELL_HYPNOTISM);
			else
				CPAttrib.StartingThings.SpellsAvailable |= (1 << M_SPELL_HYPNOTISM);
			break;
		case IDC_SPELL_FIRESTORM:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_FIRESTORM))
				CPAttrib.StartingThings.SpellsAvailable &= ~(1 << M_SPELL_FIRESTORM);
			else
				CPAttrib.StartingThings.SpellsAvailable |= (1 << M_SPELL_FIRESTORM);
			break;
		case IDC_SPELL_GHOST:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_GHOST_ARMY))
				CPAttrib.StartingThings.SpellsAvailable &= ~(1 << M_SPELL_GHOST_ARMY);
			else
				CPAttrib.StartingThings.SpellsAvailable |= (1 << M_SPELL_GHOST_ARMY);
			break;
		case IDC_SPELL_ERODE:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_ERODE))
				CPAttrib.StartingThings.SpellsAvailable &= ~(1 << M_SPELL_ERODE);
			else
				CPAttrib.StartingThings.SpellsAvailable |= (1 << M_SPELL_ERODE);
			break;
		case IDC_SPELL_SWAMP:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_SWAMP))
				CPAttrib.StartingThings.SpellsAvailable &= ~(1 << M_SPELL_SWAMP);
			else
				CPAttrib.StartingThings.SpellsAvailable |= (1 << M_SPELL_SWAMP);
			break;
		case IDC_SPELL_LANDBRIDGE:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_LAND_BRIDGE))
				CPAttrib.StartingThings.SpellsAvailable &= ~(1 << M_SPELL_LAND_BRIDGE);
			else
				CPAttrib.StartingThings.SpellsAvailable |= (1 << M_SPELL_LAND_BRIDGE);
			break;
		case IDC_SPELL_AOD:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_ANGEL_OF_DEATH))
				CPAttrib.StartingThings.SpellsAvailable &= ~(1 << M_SPELL_ANGEL_OF_DEATH);
			else
				CPAttrib.StartingThings.SpellsAvailable |= (1 << M_SPELL_ANGEL_OF_DEATH);
			break;
		case IDC_SPELL_EARTHQUAKE:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_EARTHQUAKE))
				CPAttrib.StartingThings.SpellsAvailable &= ~(1 << M_SPELL_EARTHQUAKE);
			else
				CPAttrib.StartingThings.SpellsAvailable |= (1 << M_SPELL_EARTHQUAKE);
			break;
		case IDC_SPELL_FLATTEN:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_FLATTEN))
				CPAttrib.StartingThings.SpellsAvailable &= ~(1 << M_SPELL_FLATTEN);
			else
				CPAttrib.StartingThings.SpellsAvailable |= (1 << M_SPELL_FLATTEN);
			break;
		case IDC_SPELL_VOLCANO:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_VOLCANO))
				CPAttrib.StartingThings.SpellsAvailable &= ~(1 << M_SPELL_VOLCANO);
			else
				CPAttrib.StartingThings.SpellsAvailable |= (1 << M_SPELL_VOLCANO);
			break;
		case IDC_SPELL_CONVERT:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_CONVERT))
				CPAttrib.StartingThings.SpellsAvailable &= ~(1 << M_SPELL_CONVERT);
			else
				CPAttrib.StartingThings.SpellsAvailable |= (1 << M_SPELL_CONVERT);
			break;
		case IDC_SPELL_MAGICALSHIELD:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_MAGICAL_SHIELD))
				CPAttrib.StartingThings.SpellsAvailable &= ~(1 << M_SPELL_MAGICAL_SHIELD);
			else
				CPAttrib.StartingThings.SpellsAvailable |= (1 << M_SPELL_MAGICAL_SHIELD);
			break;

		// buildings

		case IDC_BUILDING_HUT:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.BuildingsAvailable & (1 << M_BUILDING_HUT1))
				CPAttrib.StartingThings.BuildingsAvailable &= ~(1 << M_BUILDING_HUT1);
			else
				CPAttrib.StartingThings.BuildingsAvailable |= (1 << M_BUILDING_HUT1);
			break;
		case IDC_BUILDING_TOWER:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.BuildingsAvailable & (1 << M_BUILDING_TOWER))
				CPAttrib.StartingThings.BuildingsAvailable &= ~(1 << M_BUILDING_TOWER);
			else
				CPAttrib.StartingThings.BuildingsAvailable |= (1 << M_BUILDING_TOWER);
			break;
		case IDC_BUILDING_TEMPLE:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.BuildingsAvailable & (1 << M_BUILDING_TEMPLE))
				CPAttrib.StartingThings.BuildingsAvailable &= ~(1 << M_BUILDING_TEMPLE);
			else
				CPAttrib.StartingThings.BuildingsAvailable |= (1 << M_BUILDING_TEMPLE);
			break;
		case IDC_BUILDING_SPY:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.BuildingsAvailable & (1 << M_BUILDING_SPY_TRAIN))
				CPAttrib.StartingThings.BuildingsAvailable &= ~(1 << M_BUILDING_SPY_TRAIN);
			else
				CPAttrib.StartingThings.BuildingsAvailable |= (1 << M_BUILDING_SPY_TRAIN);
			break;
		case IDC_BUILDING_WARRIOR:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.BuildingsAvailable & (1 << M_BUILDING_WARRIOR_TRAIN))
				CPAttrib.StartingThings.BuildingsAvailable &= ~(1 << M_BUILDING_WARRIOR_TRAIN);
			else
				CPAttrib.StartingThings.BuildingsAvailable |= (1 << M_BUILDING_WARRIOR_TRAIN);
			break;
		case IDC_BUILDING_FIREWARRIOR:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.BuildingsAvailable & (1 << M_BUILDING_FIREWARRIOR_TRAIN))
				CPAttrib.StartingThings.BuildingsAvailable &= ~(1 << M_BUILDING_FIREWARRIOR_TRAIN);
			else
				CPAttrib.StartingThings.BuildingsAvailable |= (1 << M_BUILDING_FIREWARRIOR_TRAIN);
			break;
		case IDC_BUILDING_BOAT:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.BuildingsAvailable & (1 << M_BUILDING_BOAT_HUT_1))
				CPAttrib.StartingThings.BuildingsAvailable &= ~(1 << M_BUILDING_BOAT_HUT_1);
			else
				CPAttrib.StartingThings.BuildingsAvailable |= (1 << M_BUILDING_BOAT_HUT_1);
			break;
		case IDC_BUILDING_BALLON:
			fAIAttribChanged = true;
			if(CPAttrib.StartingThings.BuildingsAvailable & (1 << M_BUILDING_AIRSHIP_HUT_1))
				CPAttrib.StartingThings.BuildingsAvailable &= ~(1 << M_BUILDING_AIRSHIP_HUT_1);
			else
				CPAttrib.StartingThings.BuildingsAvailable |= (1 << M_BUILDING_AIRSHIP_HUT_1);
			break;
		}
		return 0;

	case WM_CLOSE:
		if(fAIAttribChanged)
		{
			switch(ModalMsg(SZ_AIATTRIB_SAVE_CHANGE, APPNAME, MB_ICONEXCLAMATION | MB_YESNOCANCEL))
			{
			case IDYES: if(DlgAIAttribSave()) DlgAIAttribToggle(); break;
			case IDNO:  DlgAIAttribToggle(); break;
			// case IDCANCEL: break;
			}
		}
		else
		{
			DlgAIAttribToggle();
		}
		return 0;
	}

	return 0;
}


void DlgAIAttribUpdate(HWND hWnd)
{
	if(!hWnd) return;

	//

	CPAttrib.Name[31] = 0; // no overflow
	SetDlgItemText(hWnd, IDC_AI_CP_NAME, CPAttrib.Name);

	DlgAIAttribUpdateAttrib(hWnd);

	//

	if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_BLAST))
		CheckDlgButton(hWnd, IDC_SPELL_BLAST, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_BLAST, BST_UNCHECKED);

	if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_LIGHTNING))
		CheckDlgButton(hWnd, IDC_SPELL_LIGHTNING, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_LIGHTNING, BST_UNCHECKED);

	if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_TORNADO))
		CheckDlgButton(hWnd, IDC_SPELL_TORNADO, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_TORNADO, BST_UNCHECKED);

	if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_SWARM))
		CheckDlgButton(hWnd, IDC_SPELL_SWARM, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_SWARM, BST_UNCHECKED);

	if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_INVISIBILITY))
		CheckDlgButton(hWnd, IDC_SPELL_INVISIBILITY, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_INVISIBILITY, BST_UNCHECKED);

	if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_HYPNOTISM))
		CheckDlgButton(hWnd, IDC_SPELL_HYPNOTISM, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_HYPNOTISM, BST_UNCHECKED);

	if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_FIRESTORM))
		CheckDlgButton(hWnd, IDC_SPELL_FIRESTORM, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_FIRESTORM, BST_UNCHECKED);

	if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_GHOST_ARMY))
		CheckDlgButton(hWnd, IDC_SPELL_GHOST, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_GHOST, BST_UNCHECKED);

	if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_ERODE))
		CheckDlgButton(hWnd, IDC_SPELL_ERODE, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_ERODE, BST_UNCHECKED);

	if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_SWAMP))
		CheckDlgButton(hWnd, IDC_SPELL_SWAMP, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_SWAMP, BST_UNCHECKED);

	if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_LAND_BRIDGE))
		CheckDlgButton(hWnd, IDC_SPELL_LANDBRIDGE, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_LANDBRIDGE, BST_UNCHECKED);

	if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_ANGEL_OF_DEATH))
		CheckDlgButton(hWnd, IDC_SPELL_AOD, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_AOD, BST_UNCHECKED);

	if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_EARTHQUAKE))
		CheckDlgButton(hWnd, IDC_SPELL_EARTHQUAKE, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_EARTHQUAKE, BST_UNCHECKED);

	if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_FLATTEN))
		CheckDlgButton(hWnd, IDC_SPELL_FLATTEN, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_FLATTEN, BST_UNCHECKED);

	if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_VOLCANO))
		CheckDlgButton(hWnd, IDC_SPELL_VOLCANO, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_VOLCANO, BST_UNCHECKED);

	if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_CONVERT))
		CheckDlgButton(hWnd, IDC_SPELL_CONVERT, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_CONVERT, BST_UNCHECKED);

	if(CPAttrib.StartingThings.SpellsAvailable & (1 << M_SPELL_MAGICAL_SHIELD))
		CheckDlgButton(hWnd, IDC_SPELL_MAGICALSHIELD, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_SPELL_MAGICALSHIELD, BST_UNCHECKED);

	//

	if(CPAttrib.StartingThings.BuildingsAvailable & (1 << M_BUILDING_HUT1))
		CheckDlgButton(hWnd, IDC_BUILDING_HUT, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BUILDING_HUT, BST_UNCHECKED);

	if(CPAttrib.StartingThings.BuildingsAvailable & (1 << M_BUILDING_TOWER))
		CheckDlgButton(hWnd, IDC_BUILDING_TOWER, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BUILDING_TOWER, BST_UNCHECKED);

	if(CPAttrib.StartingThings.BuildingsAvailable & (1 << M_BUILDING_TEMPLE))
		CheckDlgButton(hWnd, IDC_BUILDING_TEMPLE, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BUILDING_TEMPLE, BST_UNCHECKED);

	if(CPAttrib.StartingThings.BuildingsAvailable & (1 << M_BUILDING_SPY_TRAIN))
		CheckDlgButton(hWnd, IDC_BUILDING_SPY, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BUILDING_SPY, BST_UNCHECKED);

	if(CPAttrib.StartingThings.BuildingsAvailable & (1 << M_BUILDING_WARRIOR_TRAIN))
		CheckDlgButton(hWnd, IDC_BUILDING_WARRIOR, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BUILDING_WARRIOR, BST_UNCHECKED);

	if(CPAttrib.StartingThings.BuildingsAvailable & (1 << M_BUILDING_FIREWARRIOR_TRAIN))
		CheckDlgButton(hWnd, IDC_BUILDING_FIREWARRIOR, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BUILDING_FIREWARRIOR, BST_UNCHECKED);

	if(CPAttrib.StartingThings.BuildingsAvailable & (1 << M_BUILDING_BOAT_HUT_1))
		CheckDlgButton(hWnd, IDC_BUILDING_BOAT, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BUILDING_BOAT, BST_UNCHECKED);

	if(CPAttrib.StartingThings.BuildingsAvailable & (1 << M_BUILDING_AIRSHIP_HUT_1))
		CheckDlgButton(hWnd, IDC_BUILDING_BALLON, BST_CHECKED);
	else
		CheckDlgButton(hWnd, IDC_BUILDING_BALLON, BST_UNCHECKED);
}


void DlgAIAttribUpdateAttrib(HWND hWnd)
{
	if(!hWnd) return;

	int c = SendMessage(GetDlgItem(hWnd, IDC_AI_ATTRIB_LIST), CB_GETCURSEL, 0, 0);

	if((c < 0) || (c > 47))
	{
		SetDlgItemText(hWnd, IDC_AI_ATTRIB, "");
		return;
	}

	sprintf(str, "%d", CPAttrib.Attribs[c]);
	SetDlgItemText(hWnd, IDC_AI_ATTRIB, str);
}


bool DlgAIAttribSave()
{
	bool f = false;

	if(lstrlen(szCPFile) != 0)
	{
		if(DlgAIAttribSaveFile(szCPFile))
		{
			f = true;
			ModalMsg(SZ_SAVE_AIATTRIB_DONE, APPNAME, MB_ICONASTERISK);
		}
		else
		{
			ModalMsg(SZ_SAVE_AIATTRIB_FAILED, APPNAME, MB_ICONEXCLAMATION);
		}
	}
	else
	{
		f = DlgAIAttribSaveAs();
	}

	return f;
}


bool DlgAIAttribSaveAs()
{
	bool f = false;

	char filename[MAX_PATH],
		 strInitDir[MAX_PATH];

	GetUserDocumentsDirectory(strInitDir);
	memset(filename, 0, sizeof(filename));

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.lpstrDefExt = SZ_SAVE_AIATTRIB_DEFEXT;
	ofn.lpstrTitle = SZ_SAVE_AIATTRIB_TITLE;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrInitialDir = strInitDir;
	ofn.lpstrFilter = SZ_OPEN_AIATTRIB_FILTER;
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

	LockDialogs(true);
	if(GetSaveFileName(&ofn))
	{
		if(DlgAIAttribSaveFile(filename))
		{
			f = true;
			ModalMsg(SZ_SAVE_AIATTRIB_DONE, APPNAME, MB_ICONASTERISK);
		}
		else
		{
			ModalMsg(SZ_SAVE_AIATTRIB_FAILED, APPNAME, MB_ICONEXCLAMATION);
		}
	}
	LockDialogs(false);

	return f;
}


bool DlgAIAttribOpen()
{
	bool f = false;

	char filename[MAX_PATH],
		 strInitDir[MAX_PATH];

	GetUserDocumentsDirectory(strInitDir);
	memset(filename, 0, sizeof(filename));

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.lpstrTitle = SZ_OPEN_AIATTRIB_TITLE;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrInitialDir = strInitDir;
	ofn.lpstrFilter = SZ_OPEN_AIATTRIB_FILTER;
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	LockDialogs(true);
	if(GetOpenFileName(&ofn))
	{
		if(DlgAIAttribOpenFile(filename))
			f = true;
		else
			ModalMsg(SZ_OPEN_AIATTRIB_FAILED, APPNAME, MB_ICONEXCLAMATION);
	}
	LockDialogs(false);

	return f;
}


bool DlgAIAttribSaveFile(char *filename)
{
	HANDLE h;

	h = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(h == INVALID_HANDLE_VALUE) return false;

	dwRW = 0;
	WriteFile(h, &CPAttrib, sizeof(COMPUTERPLAYERFILE), &dwRW, 0);

	CloseHandle(h);

	if(dwRW != sizeof(COMPUTERPLAYERFILE)) return false;

	fAIAttribChanged = false;
	strcpy(szCPFile, filename);

	return true;
}


bool DlgAIAttribOpenFile(char *filename)
{
	DlgAIAttribNew();

	HANDLE h;

	h = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(h == INVALID_HANDLE_VALUE) return false;

	dwRW = 0;
	ReadFile(h, &CPAttrib, sizeof(COMPUTERPLAYERFILE), &dwRW, 0);

	CloseHandle(h);

	if(dwRW != sizeof(COMPUTERPLAYERFILE))
	{
		DlgAIAttribNew();
		return false;
	}

	// fAIAttribChanged = false; // already changed at DlgAIAttribNew
	strcpy(szCPFile, filename);

	DlgAIAttribUpdate(hDlgAIAttrib);

	return true;
}


void DlgAIAttribNew()
{
	memset(&CPAttrib, 0, sizeof(COMPUTERPLAYERFILE));
	szCPFile[0] = 0;
	fAIAttribChanged = false;
	DlgAIAttribUpdate(hDlgAIAttrib);
}


// -=-=- ai script dialog -=-=-

void DlgAIScriptToggle()
{
	if(hDlgAIScript)
	{
		DestroyWindow(hDlgAIScript);
		hDlgAIScript = 0;
	}
	else
	{
		hDlgAIScript = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_AI_SCRIPT), hMainWnd, DlgAIScriptProc, 0);
		ShowWindow(hDlgAIScript, SW_SHOW);
	}
}


int __stdcall DlgAIScriptProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hWnd, IDC_AI_COMPILE, BST_CHECKED);
		DlgAIScriptSetCompile(hWnd);
		return 0;

	case WM_COMMAND:
		if(HIWORD(wParam) == BN_CLICKED)
		switch(LOWORD(wParam))
		{
		case IDC_AI_COMPILE:   DlgAIScriptSetCompile(hWnd);   break;
		case IDC_AI_DECOMPILE: DlgAIScriptSetDecompile(hWnd); break;
		case IDC_AI_LOG: DlgAIScriptLog(); break;
		case IDC_AI_SRC: DlgAIScriptSrc(); break;
		case IDC_AI_DST: DlgAIScriptDst(); break;
		}
		return 0;

	case WM_CLOSE:
		DlgAIScriptToggle();
		return 0;
	}

	return 0;
}


void DlgAIScriptSetCompile(HWND hWnd)
{
	SetDlgItemText(hWnd, IDC_AI_SRC, SZ_COMPILE_SRC);
	SetDlgItemText(hWnd, IDC_AI_DST, SZ_COMPILE_DST);
	EnableWindow(GetDlgItem(hWnd, IDC_AI_EDT_LOG), true);
	EnableWindow(GetDlgItem(hWnd, IDC_AI_LOG), true);

	szAIScriptSrc[0] = 0;
	szAIScriptDst[0] = 0;
	SetDlgItemText(hWnd, IDC_AI_EDT_SRC, "");
	SetDlgItemText(hWnd, IDC_AI_EDT_DST, "");
}


void DlgAIScriptSetDecompile(HWND hWnd)
{
	SetDlgItemText(hWnd, IDC_AI_SRC, SZ_DECOMPILE_SRC);
	SetDlgItemText(hWnd, IDC_AI_DST, SZ_DECOMPILE_DST);
	EnableWindow(GetDlgItem(hWnd, IDC_AI_EDT_LOG), false);
	EnableWindow(GetDlgItem(hWnd, IDC_AI_LOG), false);

	szAIScriptSrc[0] = 0;
	szAIScriptDst[0] = 0;
	SetDlgItemText(hWnd, IDC_AI_EDT_SRC, "");
	SetDlgItemText(hWnd, IDC_AI_EDT_DST, "");
}


void DlgAIScriptCompile()
{
	HANDLE hSrc, hDst, hLog;
	char *szf;
	DWORD len;

	hLog = CreateFile(szAIScriptLog, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(hLog == INVALID_HANDLE_VALUE)
	{
		sprintf(str, SZ_COMPILE_LOG_CREATE_FAILED, szAIScriptLog);
		ModalMsg(str, APPNAME, MB_ICONEXCLAMATION);
		return;
	}

	//

	hSrc = CreateFile(szAIScriptSrc, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(hSrc == INVALID_HANDLE_VALUE)
	{
		sprintf(str, SZ_COMPILE_SRC_OPEN_FAILED, szAIScriptSrc);

		ModalMsg(str, APPNAME, MB_ICONEXCLAMATION);

		strcat(str, "\r\n");
		dwRW = 0;
		WriteFile(hLog, str, lstrlen(str), &dwRW, 0);
		CloseHandle(hLog);

		return;
	}

	len = SetFilePointer(hSrc, 0, 0, 2);
	SetFilePointer(hSrc, 0, 0, 0);

	szf = (char*)malloc(len + 1);
	szf[len] = 0;

	dwRW = 0;
	ReadFile(hSrc, szf, len, &dwRW, 0);
	CloseHandle(hSrc);

	if(dwRW != len)
	{
		free(szf);

		sprintf(str, SZ_COMPILE_SRC_READ_FAILED, szAIScriptSrc);

		ModalMsg(str, APPNAME, MB_ICONEXCLAMATION);

		strcat(str, "\r\n");
		dwRW = 0;
		WriteFile(hLog, str, lstrlen(str), &dwRW, 0);
		CloseHandle(hLog);

		return;
	}

	//

	bool f;
	f = ScriptCompile(&AIScript, szf, hLog);

	free(szf);

	if(f)
	{
		hDst = CreateFile(szAIScriptDst, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if(hDst == INVALID_HANDLE_VALUE)
		{
			sprintf(str, SZ_COMPILE_DST_CREATE_FAILED, szAIScriptDst);

			ModalMsg(str, APPNAME, MB_ICONEXCLAMATION);

			strcat(str, "\r\n");
			dwRW = 0;
			WriteFile(hLog, str, lstrlen(str), &dwRW, 0);
			CloseHandle(hLog);

			return;
		}

		dwRW = 0;
		WriteFile(hDst, &AIScript, sizeof(SCRIPTINFO), &dwRW, 0);
		CloseHandle(hDst);

		if(dwRW != sizeof(SCRIPTINFO))
		{
			sprintf(str, SZ_COMPILE_DST_WRITE_FAILED, szAIScriptDst);

			ModalMsg(str, APPNAME, MB_ICONEXCLAMATION);

			strcat(str, "\r\n");
			dwRW = 0;
			WriteFile(hLog, str, lstrlen(str), &dwRW, 0);
			CloseHandle(hLog);

			return;
		}
	}

	CloseHandle(hLog);

	if(f)
	{
		ModalMsg(SZ_COMPILE_DONE, APPNAME, MB_ICONASTERISK);
	}
	else
	{
		ModalMsg(SZ_COMPILE_FAILED, APPNAME, MB_ICONEXCLAMATION);
	}
}


void DlgAIScriptDecompile()
{
	HANDLE hSrc, hDst;

	hDst = CreateFile(szAIScriptDst, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(hDst == INVALID_HANDLE_VALUE)
	{
		sprintf(str, SZ_DECOMPILE_DST_CREATE_FAILED, szAIScriptDst);
		ModalMsg(str, APPNAME, MB_ICONEXCLAMATION);
		return;
	}

	//


	hSrc = CreateFile(szAIScriptSrc, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(hSrc == INVALID_HANDLE_VALUE)
	{
		sprintf(str, SZ_DECOMPILE_SRC_OPEN_FAILED, szAIScriptSrc);

		ModalMsg(str, APPNAME, MB_ICONEXCLAMATION);

		strcat(str, "\r\n");
		dwRW = 0;
		WriteFile(hDst, str, lstrlen(str), &dwRW, 0);
		CloseHandle(hDst);

		return;
	}

	dwRW = 0;
	ReadFile(hSrc, &AIScript, sizeof(SCRIPTINFO), &dwRW, 0);
	CloseHandle(hSrc);

	if(dwRW != sizeof(SCRIPTINFO))
	{
		sprintf(str, SZ_DECOMPILE_SRC_READ_FAILED, szAIScriptSrc);

		ModalMsg(str, APPNAME, MB_ICONEXCLAMATION);

		strcat(str, "\r\n");
		dwRW = 0;
		WriteFile(hDst, str, lstrlen(str), &dwRW, 0);
		CloseHandle(hDst);

		return;
	}

	//

	bool f;
	f = ScriptDecompile(&AIScript, hDst);

	CloseHandle(hDst);

	if(f)
	{
		ModalMsg(SZ_DECOMPILE_DONE, APPNAME, MB_ICONASTERISK);
	}
	else
	{
		ModalMsg(SZ_DECOMPILE_FAILED, APPNAME, MB_ICONEXCLAMATION);
	}
}


void DlgAIScriptLog()
{
	char filename[MAX_PATH],
		 strInitDir[MAX_PATH];

	GetUserDocumentsDirectory(strInitDir);
	strcpy(filename, szAIScriptLog);

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.lpstrDefExt = SZ_AISCRIPT_LOG_DEFEXT;
	ofn.lpstrTitle = SZ_AISCRIPT_LOG_TITLE;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrInitialDir = strInitDir;
	ofn.lpstrFilter = SZ_AISCRIPT_LOG_FILTER;
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

	LockDialogs(true);
	if(GetSaveFileName(&ofn))
	{
		strcpy(szAIScriptLog, filename);

		PathStripPath(filename);
		SetDlgItemText(hDlgAIScript, IDC_AI_EDT_LOG, filename);
	}
	LockDialogs(false);
}

void DlgAIScriptSrc()
{
	char filename[MAX_PATH],
		 strInitDir[MAX_PATH];

	GetUserDocumentsDirectory(strInitDir);
	strcpy(filename, szAIScriptSrc);

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrInitialDir = strInitDir;
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if(IsDlgButtonChecked(hDlgAIScript, IDC_AI_COMPILE) == BST_CHECKED)
	{
		ofn.lpstrTitle  = SZ_AISCRIPT_TXT_TITLE;
		ofn.lpstrFilter = SZ_AISCRIPT_TXT_FILTER;
	}
	else if(IsDlgButtonChecked(hDlgAIScript, IDC_AI_DECOMPILE) == BST_CHECKED)
	{
		ofn.lpstrTitle  = SZ_AISCRIPT_SCR_TITLE;
		ofn.lpstrFilter = SZ_AISCRIPT_SCR_FILTER;
	}
	else
	{
		return;
	}

	LockDialogs(true);
	if(GetOpenFileName(&ofn))
	{
		strcpy(szAIScriptSrc, filename);

		PathStripPath(filename);
		SetDlgItemText(hDlgAIScript, IDC_AI_EDT_SRC, filename);
	}
	LockDialogs(false);
}


void DlgAIScriptDst()
{
	bool c;

	if(IsDlgButtonChecked(hDlgAIScript, IDC_AI_COMPILE) == BST_CHECKED)
		c = true;
	else if(IsDlgButtonChecked(hDlgAIScript, IDC_AI_DECOMPILE) == BST_CHECKED)
		c = false;
	else
		return;

	if(c && (lstrlen(szAIScriptLog) == 0))
	{
		ModalMsg(SZ_AISCRIPT_MISSING_LOG, APPNAME, MB_ICONEXCLAMATION);
		return;
	}

	if(lstrlen(szAIScriptSrc) == 0)
	{
		ModalMsg(SZ_AISCRIPT_MISSING_SRC, APPNAME, MB_ICONEXCLAMATION);
		return;
	}

	char filename[MAX_PATH],
		 strInitDir[MAX_PATH];

	GetUserDocumentsDirectory(strInitDir);
	strcpy(filename, szAIScriptDst);

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrInitialDir = strInitDir;
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

	if(c)
	{
		ofn.lpstrTitle  = SZ_AISCRIPT_SCR_TITLE;
		ofn.lpstrFilter = SZ_AISCRIPT_SCR_FILTER;
		ofn.lpstrDefExt = SZ_AISCRIPT_SCR_DEFEXT;
	}
	else
	{
		ofn.lpstrTitle  = SZ_AISCRIPT_TXT_TITLE;
		ofn.lpstrFilter = SZ_AISCRIPT_TXT_FILTER;
		ofn.lpstrDefExt = SZ_AISCRIPT_TXT_DEFEXT;
	}

	LockDialogs(true);
	if(GetSaveFileName(&ofn))
	{
		strcpy(szAIScriptDst, filename);

		PathStripPath(filename);
		SetDlgItemText(hDlgAIScript, IDC_AI_EDT_DST, filename);

		if(c)
			DlgAIScriptCompile();
		else
			DlgAIScriptDecompile();
	}
	LockDialogs(false);
}



void PurgeObjects()
{
	THING *pThing;
	int nDeletedObjs = 0;

	while(Things)
	{
		nDeletedObjs++;
		pThing = Things;
		ThingsIndices[pThing->Idx] = 0;

		UNLINK(Things, pThing);
		delete pThing;
	}

	EngineUpdateMiniMap();
	sprintf(str, "%d", nDeletedObjs);
	MessageBox(NULL, str, _T("Objects Deleted"), MB_ICONINFORMATION | MB_OK);
} 

void ResetMarkers()
{
	memset(&Markers, 0, sizeof(Markers));
	memset(leveldat->Header.v2.Markers, 0, sizeof(leveldat->Header.v2.Markers));
	DlgMarkersUpdate(hDlgMarkers);
	EngineUpdateView();

	MessageBox(NULL, "Done!" , _T("Markers Reset"), MB_ICONINFORMATION | MB_OK);
} 


void DlgMacroToggle()
{
	if(hDlgMacro)
	{
		DestroyWindow(hDlgMacro);
		hDlgMacro = 0;
	}
	else
	{
		hDlgMacro = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MACRO), hMainWnd, DlgMacroProc, 0);
		ShowWindow(hDlgMacro, SW_SHOW);
	}
}

int __stdcall DlgMacroProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CLOSE:
		DlgMacroToggle();
		return 0;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_MACRO_TXT_BOX:
		{
			if(EN_KILLFOCUS == HIWORD(wParam))
			{
				char strBuffer[10240];
				GetWindowText((HWND)lParam, strBuffer, sizeof(strBuffer));
				strMacro = strBuffer;
			}
			break;
		}
		case IDC_MACRO_CREATE: 
			DlgMacroCreate(strMacro);
			break;
		}
	
		return 0;
	}
	return 0;
}

void DlgMacroCreate(std::string strMacro)
{
	if (net.IsInitialized() && net.IsConnected())
		return;

#if _DEBUG
	std::cout << strMacro << std::endl;
#endif

	std::string strBuffer;
    std::stringstream ss(strMacro);
    std::vector<std::string> Token;

	while (ss >> strBuffer)
		Token.push_back(strBuffer);

	THING *pThing;
	WORD x = 0,
		 z = 0;
	int nThingIndex = 0,
		nLinkSource = 0,
		nLinkTarget = 0,
		nThingsIndices[64] = { 0 };

	for (int i = 0; i < Token.size(); i++)
	{
		if (Token[i] == "coords")
		{
			i++; x = std::stoi(Token[i]) / 2;
			i++; z = std::stoi(Token[i]) / 2;
		}
		else if (Token[i] == "create")
		{
			i++; nThingIndex = std::stoi(Token[i]);

			pThing = DlgObjectNewObj(x, z, std::stoi(Token[i+1]), std::stoi(Token[i+2]), std::stoi(Token[i+3]));
			i += 3;

			if (!pThing) return;
			nThingsIndices[nThingIndex] = pThing->Idx;
			
			if (pThing->Thing.Type == T_GENERAL && pThing->Thing.Model == M_GENERAL_TRIGGER)
			{
				i++; pThing->Thing.Trigger.TriggerType = std::stoi(Token[i]);
				i++; pThing->Thing.Trigger.CellRadius = std::stoi(Token[i]);
				i++; pThing->Thing.Trigger.PrayTime = std::stoi(Token[i]);
				i++; pThing->Thing.Trigger.InactiveTime = std::stoi(Token[i]);
				i++; pThing->Thing.Trigger.NumOccurences = std::stoi(Token[i]);
				i++; pThing->Thing.Trigger.TriggerCount = std::stoi(Token[i]);
				i++; pThing->Thing.Trigger.StartInactive = std::stoi(Token[i]);
				i++; pThing->Thing.Trigger.CreatePlayerOwned = std::stoi(Token[i]);
			}
			else if (pThing->Thing.Type == T_GENERAL && pThing->Thing.Model == M_GENERAL_DISCOVERY)
			{
				i++; pThing->Thing.General.AvailabilityType = std::stoi(Token[i]);
				i++; pThing->Thing.General.DiscoveryType = std::stoi(Token[i]);

				if (pThing->Thing.General.DiscoveryType == T_BUILDING || pThing->Thing.General.DiscoveryType == T_SPELL)
				{
					pThing->Thing.General.DiscoveryModel = 1;
				}
				else
				{
					pThing->Thing.General.DiscoveryModel = M_GENERAL_DEBUG_FLAG;
					i++; pThing->Thing.General.ManaAmt = std::stoi(Token[i]);
				}

			}
		}
		else if (Token[i] == "link")
		{
			i++; nLinkSource = std::stoi(Token[i]);
			i++; nLinkTarget = std::stoi(Token[i]);

			ThingLink = DlgObjectFindIdx(nThingsIndices[nLinkSource]);
			for (int j = 0; j < 10; j++)
			{
				if(!ThingLink->Links[j])
				{
					ThingLink->Links[j] = DlgObjectFindIdx(nThingsIndices[nLinkTarget]);
					break;
				}
			}
		}
		else if (Token[i] == "end")
		{
			DlgObjectCenterSelected();
			break;
		}
	}
}


void DlgObjectMoreToggle()
{
	if(hDlgObjectMore)
	{
		DestroyWindow(hDlgObjectMore);
		hDlgObjectMore = 0;
		//DlgListUpdateInfo(hDlgList);
	}
	else
	{
		hDlgObjectMore = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_OBJMORE), hMainWnd, DlgObjectMoreProc, 0);
		ShowWindow(hDlgObjectMore, SW_SHOW);
	}
}


int __stdcall DlgObjectMoreProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		DlgObjectMoreUpdate(hWnd);
		return 0;

	case WM_COMMAND:
		switch(wParam)
		{

		case IDC_OBJMORE_PRI_PERSONS:
				nSelectPriority = T_PERSON;
			break;

		case IDC_OBJMORE_PRI_BUILDINGS:
				nSelectPriority = T_BUILDING;
			break;

		case IDC_OBJMORE_PRI_CREATURES:
				nSelectPriority = T_CREATURE;
			break;
		
		case IDC_OBJMORE_PRI_VEHICLES:
				nSelectPriority = T_VEHICLE;
			break;

		case IDC_OBJMORE_PRI_SCENERY:
				nSelectPriority = T_SCENERY;
			break;

		case IDC_OBJMORE_PRI_GENERAL:
				nSelectPriority = T_GENERAL;
			break;

		case IDC_OBJMORE_PRI_EFFECTS:
				nSelectPriority = T_EFFECT;
			break;

		case IDC_OBJMORE_PRI_SHOTS:
				nSelectPriority = T_SHOT;
			break;

		case IDC_OBJMORE_PRI_SPELLS:
				nSelectPriority = T_SPELL;
			break;

		case IDC_OBJMORE_PRI_NONE:
				nSelectPriority = 0;
			break;


		case IDC_OBJMORE_CREATE:
		{
			if (ThingSelected)
			{
				memset(str, 0, sizeof(str));
				GetWindowText(GetDlgItem(hWnd, IDC_OBJMORE_CREATE_TXT_BOX), str, sizeof(str));
				if (str[0] != 0)
				{
					for (int i = 0; i < std::stoi(str); i++)
					{
						fQuickDuplicate = true;
						DlgObjectNewObj();
					}
				}
			}
		}
		
			break;
		}
		return 0;

	case WM_CLOSE:
		DlgObjectMoreToggle();
		return 0;
	}

	return 0;
}


void DlgObjectMoreUpdate(HWND hWnd)
{
	if(!hWnd) return;

	if (nSelectPriority == 0)
		CheckDlgButton(hWnd, IDC_OBJMORE_PRI_NONE, BST_CHECKED);
	else if (nSelectPriority == T_PERSON)
		CheckDlgButton(hWnd, IDC_OBJMORE_PRI_PERSONS, BST_CHECKED);
	else if (nSelectPriority == T_BUILDING)
		CheckDlgButton(hWnd, IDC_OBJMORE_PRI_BUILDINGS, BST_CHECKED);
	else if (nSelectPriority == T_CREATURE)
		CheckDlgButton(hWnd, IDC_OBJMORE_PRI_CREATURES, BST_CHECKED);
	else if (nSelectPriority == T_VEHICLE)
		CheckDlgButton(hWnd, IDC_OBJMORE_PRI_VEHICLES, BST_CHECKED);
	else if (nSelectPriority == T_SCENERY)
		CheckDlgButton(hWnd, IDC_OBJMORE_PRI_SCENERY, BST_CHECKED);
	else if (nSelectPriority == T_GENERAL)
		CheckDlgButton(hWnd, IDC_OBJMORE_PRI_GENERAL, BST_CHECKED);
	else if (nSelectPriority == T_EFFECT)
		CheckDlgButton(hWnd, IDC_OBJMORE_PRI_EFFECTS, BST_CHECKED);
	else if (nSelectPriority == T_SHOT)
		CheckDlgButton(hWnd, IDC_OBJMORE_PRI_SHOTS, BST_CHECKED);
	else if (nSelectPriority == T_SPELL)
		CheckDlgButton(hWnd, IDC_OBJMORE_PRI_SPELLS, BST_CHECKED);
}