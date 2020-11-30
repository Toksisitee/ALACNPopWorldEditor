#pragma once
#include <tchar.h>

extern BOOL GetApplicationUserDataStorageDirectory(TCHAR* path);
extern BOOL GetApplicationUserDataFilePath(const TCHAR* file_name, TCHAR* path);
extern BOOL GetUserDocumentsDirectory(TCHAR* path);
extern BOOL InitializeUserFiles();