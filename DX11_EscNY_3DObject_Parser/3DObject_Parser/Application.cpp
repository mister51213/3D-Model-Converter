#include "Application.h"
#include "OBJLoader.h"

Application::Application()
	:
	m_classname( L"MyClass" ),
	m_btnLeftPressed(false)
{
	// Initialize the COM objects
	CoInitializeEx( nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE );

	INITCOMMONCONTROLSEX initCtrl{ sizeof( initCtrl ), ICC_PROGRESS_CLASS };
	InitCommonControlsEx( &initCtrl );

	int winStyle = WS_OVERLAPPEDWINDOW;
	int winExStyle = WS_OVERLAPPED;

	WNDCLASSEX wc{};
	wc.cbSize = sizeof( WNDCLASSEX );
	wc.hInstance = GetModuleHandle( nullptr );
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = m_classname.c_str();
	wc.hbrBackground = (HBRUSH)( COLOR_WINDOW + 1 );
	wc.style = CS_HREDRAW | CS_VREDRAW;
	
	m_appId = RegisterClassEx( &wc );

	m_width = 265;
	m_height = 56;

	RECT rc{};
	rc.left = 400;
	rc.top = 300;
	rc.right = rc.left + m_width;
	rc.bottom = rc.top + m_height;

	AdjustWindowRectEx( &rc, winStyle, FALSE, winExStyle );

	UINT winWidth = rc.right - rc.left;
	UINT winHeight = rc.bottom - rc.top;
	
	CreateWindowEx(
		winExStyle, 
		m_classname.c_str(), 
		L"Mesh Converter 0.001", 
		winStyle,
		rc.left, 
		rc.top, 
		winWidth, 
		winHeight, 
		nullptr, 
		nullptr, 
		wc.hInstance,
		this
	);
		
	ShowWindow( m_HandleParent, SW_SHOWDEFAULT );
	ShowWindow( m_ButtonConvert, SW_HIDE );

}

Application::~Application()
{
	// Uninitialize the COM object classes
	CoUninitialize();

	// Unregister user window class
	UnregisterClass( m_classname.c_str(), GetModuleHandle( nullptr ) );
}

int Application::Main()
{
	MSG msg{};

	while( GetMessage(&msg, m_HandleParent, 0,0) == TRUE )
	{
		UpdateWindow( m_HandleParent );
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	return 0;
}

LRESULT Application::MessageHandler( UINT Message, WPARAM wParam, LPARAM lParam )
{
	LRESULT result = 0;

	// All messages handled by this app are forwarded to functions in the 
	// Application class
	switch( Message )
	{
		case WM_CREATE:
		{
			result = OnCreate();
			break;
		}
		case WM_CLOSE:
			OnClose();
			break;
		case WM_DESTROY:
			OnDestroy();
			break;
		case WM_COMMAND:
			switch( wParam )
			{				
				case CONTROLID_BROWSE_OPEN:
				{
					OnOpen();
					break;
				}
				case CONTROLID_BROWSE_SAVE:
				{
					OnSave();
					break;
				}
				case CONTROLID_CONVERT:
				{
					OnConvert();
					break;
				}
			}
			break;
		default:
			result = DefWindowProc( m_HandleParent, Message, wParam, lParam );
	}

	return result;
}

void Application::OnOpen()
{
	// Create the IFileOpenDialog COM object 
	Microsoft::WRL::ComPtr<IFileOpenDialog> pFileDialog;
	HRESULT hr = CoCreateInstance(
		CLSID_FileOpenDialog,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS( pFileDialog.GetAddressOf() )
	);
	assert( SUCCEEDED( hr ) );

	// Declares the file types the program can process
	std::vector<COMDLG_FILTERSPEC> filters
	{
		//{L"txt", L"*.txt"}, // <- Loading of txt not implemented
		{L"obj", L"*.obj"}
	};

	// Sets the file types previously declared, must be done before 
	// the dialog box is shown.
	hr = pFileDialog->SetFileTypes( filters.size(), filters.data());

	// Show the open file dialog
	hr = pFileDialog->Show( m_HandleParent );
	
	if( SUCCEEDED( hr ) )
	{
		// Initialize the IShellItem COM object that will store the
		// selected file from the open dialog box
		Microsoft::WRL::ComPtr<IShellItem>pItem;		
		hr = pFileDialog->GetResult( &pItem );
		
		assert( SUCCEEDED( hr ) );

		// Create wchar_t pointers to get the absolute path and relative path
		// of the file selected.  
		LPWSTR pFilepath = nullptr;
		LPWSTR pRelPath = nullptr;

		// The absolute path will be used for loading the file
		hr = pItem->GetDisplayName( SIGDN_FILESYSPATH, &pFilepath );

		// Store the absolute file path to load later when user clicks
		// the convert button
		m_FilenameLoad = pFilepath;

		// The relative path is used just for displaying the file name
		hr = pItem->GetDisplayName( SIGDN_PARENTRELATIVE, &pRelPath );

		// Set the window text of the text box with the file name only
		SendMessage( 
			m_TextboxLoadFile,
			WM_SETTEXT, 
			0, 
			reinterpret_cast<LPARAM>( pRelPath ) );
		//SetWindowText( m_filenameTextBox, pRelPath );

		// Unhide the convert button now that we have a file to use
		ShowWindow( m_ButtonConvert, SW_SHOW );

		// Free the memory that was allocated by the system to store 
		// the two path strings.
		CoTaskMemFree( pFilepath );
		CoTaskMemFree( pRelPath );
	}
}

void Application::OnSave()
{
	// Create the IFileOpenDialog COM object 
	Microsoft::WRL::ComPtr<IFileSaveDialog> pFileDialog;
	HRESULT hr = CoCreateInstance(
		CLSID_FileSaveDialog,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS( pFileDialog.GetAddressOf() )
	);
	assert( SUCCEEDED( hr ) );

	// Declares the file types the program can process
	std::vector<COMDLG_FILTERSPEC> filters
	{
		{ L"BinaryMesh", L"*.BinaryMesh" },
		{ L"txt", L"*.txt" } 
	};

	// Sets the file types previously declared, must be done before 
	// the dialog box is shown.
	hr = pFileDialog->SetFileTypes( filters.size(), filters.data() );

	// Sets the default extension of the chosen file, if you don't put
	// file.BinaryMesh, the extension will be added for you.
	hr = pFileDialog->SetDefaultExtension( L"BinaryMesh" );

	// Show the open file dialog
	hr = pFileDialog->Show( m_HandleParent );
	
	if( SUCCEEDED( hr ) )
	{
		// Initialize the IShellItem COM object that will store the
		// selected file from the open dialog box
		Microsoft::WRL::ComPtr<IShellItem>pItem;
		hr = pFileDialog->GetResult( &pItem );
		assert( SUCCEEDED( hr ) );

		// Create wchar_t pointers to get the absolute path and relative path
		// of the file selected.  
		LPWSTR pFilepath = nullptr;
		LPWSTR pRelPath = nullptr;

		// The absolute path will be used for loading the file
		hr = pItem->GetDisplayName( SIGDN_FILESYSPATH, &pFilepath );

		// Store the absolute file path to load later when user clicks
		// the convert button
		m_FilenameSave = pFilepath;

		// The relative path is used just for displaying the file name
		hr = pItem->GetDisplayName( SIGDN_PARENTRELATIVE, &pRelPath );

		// Set the window text of the text box with the file name only		
		SetWindowText( m_TextboxSaveFile, pRelPath );

		// Unhide the convert button now that we have a file to use
		ShowWindow( m_ButtonConvert, SW_SHOW );

		// Free the memory that was allocated by the system to store 
		// the two path strings.
		CoTaskMemFree( pFilepath );
		CoTaskMemFree( pRelPath );
	}
}

void Application::OnNonClientCreate( HWND WinHandle )
{
	SetWindowLongPtr( 
		WinHandle, 
		GWLP_USERDATA, 
		reinterpret_cast<LONG_PTR>( this ) );

	m_HandleParent = WinHandle;
}

LRESULT Application::OnCreate()
{
	// TODO: Test delaying child window creation
	// Perhaps putting this somewhere other than during parent creation
	// would work better.  There are some issues with the main window.

	HINSTANCE hInst = GetModuleHandle( nullptr );

	// Get the client window dimensions to be use in calculating where
	// all the child window controls will be placed.
	RECT rc{};
	GetClientRect( m_HandleParent, &rc );

	// Setup constants for unifying layout
	const int vSpacing = 4;		// vertical spacing between controls
	const int hSpacing = 8;		// horizontal spacing between controls
	const int tbWidth = 200;	// text box width
	const int btnWidth = 64;	// button width
	const int ctrlHeight = 24;	// control height, used for all controls

	// Calculate the top position of the open text box and button
	int openCtrlTop = vSpacing + rc.top;
	// Calculate the top position of the save text box and button
	int saveCtrlTop = openCtrlTop + vSpacing + ctrlHeight;
	// Calculate the top position of the convert button
	int convBtnTop = saveCtrlTop + ctrlHeight + vSpacing;
	// Calculate the left position of the open/save text boxes
	int tbCtrlLeft = rc.left + hSpacing;
	// Calculate the left position of the open/save buttons
	int fileBtnLeft = tbWidth + ( hSpacing * 2 );
	// Calculate the left position of the convert button
	int convBtnLeft = tbCtrlLeft;
	
	// Calculate the total width of all controls
	int ctrlWidthSum = fileBtnLeft + btnWidth + hSpacing;
	// Calculate the total height of all controls
	int ctrlHeightSum = convBtnTop + ctrlHeight + vSpacing;

	// Determine if window needs to be resized
	int xDiff = ( rc.right < ctrlWidthSum ) ? ctrlWidthSum - rc.right : 0;
	int yDiff = ( rc.bottom < ctrlHeightSum ) ? ctrlHeightSum - rc.bottom : 0;
		
	rc.right += xDiff;
	rc.bottom += yDiff;

	// Adjust the window size to account for visual style of the window
	AdjustWindowRectEx( &rc, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW );

	// Update window size
	BOOL swp_result = SetWindowPos( m_HandleParent, nullptr, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE );
	if( IsError( L"Could not resize window" ) )
	{
		return -1;
	}
	SetLastError( 0 );

	// Load file text box
	m_TextboxLoadFile = CreateWindowEx( WS_EX_CLIENTEDGE,
		L"Edit",										// Predefined edit class
		L"Ex. Mesh.obj",								// Default message
		WS_CHILD | WS_VISIBLE,							// Window style
		tbCtrlLeft, openCtrlTop, tbWidth, ctrlHeight,	// Position and size
		m_HandleParent,									// Parent HWND
		reinterpret_cast<HMENU>( CONTROLID_FILENAME ),	// Control id
		hInst, nullptr );								// HISNTANCE, user data
	if( IsError( L"Couldn't create the TextBox." ) )
	{
		return -1;
	}

	// Save file text box
	m_TextboxSaveFile = CreateWindowEx( WS_EX_CLIENTEDGE,
		L"Edit",										// Predefined edit class
		L"Ex. Mesh.BinaryMesh", 						// Default message
		WS_CHILD | WS_VISIBLE, 							// Window style
		tbCtrlLeft, saveCtrlTop, tbWidth, ctrlHeight,	// Position and size
		m_HandleParent,									// Parent HWND
		reinterpret_cast<HMENU>( CONTROLID_FILENAME ),	// Control id
		hInst, nullptr );								// HISNTANCE, user data
	if( IsError( L"Couldn't create the TextBox." ) )
	{
		return -1;
	}

	// Browse button to open find the file to open for conversion
	m_ButtonOpen = CreateWindowEx( 0,
		L"Button",										 // Predefined button class
		L"Browse", 										 // Button text
		WS_CHILD | WS_VISIBLE, 							 // Window style
		fileBtnLeft, openCtrlTop, 60, ctrlHeight,		 // Position and size
		m_HandleParent,									 // Parent HWND
		reinterpret_cast<HMENU>( CONTROLID_BROWSE_OPEN ),// Control id,
		hInst, nullptr );								 // HISNTANCE, user data
	if( IsError( L"Couldn't create the Button." ) )
	{
		return -1;
	}

	// Brows button to choose destination of converted file
	m_ButtonSave = CreateWindowEx( 0,
		L"Button",										 // Predefined button class
		L"Browse", 										 // Button text
		WS_CHILD | WS_VISIBLE, 							 // Window style
		fileBtnLeft, saveCtrlTop, btnWidth, ctrlHeight,	 // Position and size
		m_HandleParent, 								 // Parent HWND
		reinterpret_cast<HMENU>( CONTROLID_BROWSE_SAVE ),// Control id,
		hInst, nullptr );								 // HISNTANCE, user data
	if( IsError( L"Couldn't create the Button." ) )
	{
		return -1;
	}

	// Convert button
	m_ButtonConvert = CreateWindowEx( 0,
		L"Button",										 // Predefined button class
		L"Convert", 									 // Button text
		WS_CHILD | BS_DEFPUSHBUTTON, 					 // Window style
		convBtnLeft, convBtnTop, btnWidth, ctrlHeight,	 // Position and size
		m_HandleParent,									 // Parent HWND
		reinterpret_cast<HMENU>( CONTROLID_CONVERT ),	 // Control id,
		hInst, nullptr );								 // HISNTANCE, user data
	if( IsError( L"Couldn't create the Button." ) )
	{
		return -1;
	}

	return 0;
}

void Application::OnConvert()
{
	// Convert obj data to custom binary layout

	// Get current window size and position to restore after progress bar
	// runs.
	RECT rectLoad{}, rectSave{};
	GetWindowRect( m_HandleParent, &rectSave );
	GetWindowRect( m_HandleParent, &rectLoad );

	// TODO: Try other values for calculations
	// Calculate how big the window should be to show the progress bar
	int width = rectLoad.right - rectLoad.left;
	int height = ( rectLoad.bottom - rectLoad.top ) + 25;

	// Resize the window to make room for the progress bar
	SetWindowPos( m_HandleParent, nullptr, rectLoad.left, rectLoad.top, width, height, SWP_NOMOVE );

	// Reset any error state
	SetLastError( 0 );

	// Create the progress bar window
	HWND progress = CreateWindowEx( 0,
		PROGRESS_CLASS,								// Predefined progress bar class
		L"Operation Status:",						// Title?
		WS_CHILD | WS_VISIBLE,						// Style
		0, height - 30, width, 25,					// Size and position
		m_HandleParent,								// Main window handle
		nullptr,									// Control not handled by app
		GetModuleHandle( nullptr ), nullptr );		// HINSTANCE, user data

	// TODO: Figure out why I keep getting errors, but get a handle anyway
	auto error = GetLastError();
	BOOL isWindow = IsWindow( progress );

	ShowWindow( progress, SW_SHOW );
	UpdateWindow( m_HandleParent );

	// Progress bar range will be 0 to 100
	SendMessageW(
		progress,
		PBM_SETRANGE,
		static_cast<WPARAM>( 0 ),
		MAKELPARAM( 0, 100 )
	);

	// Progress bar will increment by 1 by default
	auto lresult = SendMessageW(
		progress, 
		PBM_SETSTEP, 
		static_cast<WPARAM>(1), 
		static_cast<LPARAM>(0)
	);

	OBJLoader loader;
	bool result = loader.Load(progress, m_FilenameLoad, m_FilenameSave );
	if( !result )
	{
		MessageBox( 
			m_HandleParent, 
			L"File might not have been on OBJ file.", 
			L"Problem loading file.", 
			MB_OK );
	}
	ShowWindow( m_ButtonConvert, SW_HIDE );

	if( progress )
	{
		DestroyWindow( progress );
		width = rectSave.right - rectSave.left;
		height = rectSave.bottom - rectSave.top;
		SetWindowPos( m_HandleParent, nullptr, rectSave.left, rectSave.top, width, height, SWP_NOMOVE );
	}
}

void Application::OnDestroy()
{
	PostQuitMessage( 0 );
}

void Application::OnClose()
{
	DestroyWindow( m_HandleParent );
}

bool Application::IsError( LPWSTR ErrorMessage )
{
	auto error = GetLastError();
	if( error != 0 )
	{
		MessageBox( m_HandleParent, ErrorMessage, L"Ummm.", MB_OK );
		DestroyWindow( m_HandleParent );

		return true;
	}

	return false;

}
