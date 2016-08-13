#pragma once

#include "Includes.h"
#pragma comment(lib, "Comctl32.lib")

#define MENUID_FILE 1
#define MENUID_FILE_OPEN 2

#define CONTROLID_BROWSE_OPEN	100
#define CONTROLID_BROWSE_SAVE	CONTROLID_BROWSE_OPEN	+ 1
#define CONTROLID_FILENAME		CONTROLID_BROWSE_SAVE	+ 1	
#define CONTROLID_CONVERT		CONTROLID_FILENAME		+ 1
#define CONTROLID_PROGRESS		CONTROLID_CONVERT		+ 1

class Application
{
public:
	Application();
	~Application();

	int Main();
	LRESULT MessageHandler( UINT Message, WPARAM wParam, LPARAM lParam );
	void OnNonClientCreate( HWND WinHandle );
private:
	void OnOpen();
	void OnSave();
	LRESULT OnCreate();
	void OnConvert();
	void OnDestroy();
	void OnClose();

	bool IsError(LPWSTR ErrorMessage);
private:
	ATOM m_appId;
	
	HWND m_HandleParent;
	HWND m_ButtonOpen, m_ButtonSave, m_ButtonConvert;
	HWND m_TextboxLoadFile, m_TextboxSaveFile, m_TextboxStatus;
	
	std::wstring m_classname;
	std::wstring m_FilenameLoad, m_FilenameSave;

	UINT m_width, m_height;
	bool m_btnLeftPressed;
	
	HBRUSH m_BrushBlack = CreateSolidBrush( 0 );
	HBRUSH m_BrushBlue = CreateSolidBrush( 255 << 16 );

};

LRESULT CALLBACK WndProc( HWND WinHandle, UINT Message, WPARAM wParam, LPARAM lParam );