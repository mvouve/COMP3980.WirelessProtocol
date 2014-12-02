/*******************************************************************************
** Source File : Applicationn.cpp -- A terminal application that simulates the
**									Grapefruit Protocol.
**
** Program : Grapefruit Protocol Implementation
**
** Functions :
**			LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM)
**			void InstantiateWindow(HINSTANCE hInst)
**			void CheckMenu(WPARAM wP)
**			void GetCharsFromPort(char *c)
**			void OutputText()
**			void DisableMenuItems(HMENU)
**			void EnableMenuItems(HMENU)
**			void changeBackgroundColor(HWND, COLORREF)
**			void commandMode(HMENU, HANDLE)
**			void connectedMode(HMENU, LPCSTR, HANDLE)
**
**	DATE: 		September 16, 2014
**
**	REVISIONS:	N/A
**
**	DESIGNER: 	Filip Gutica A00781910
**				Rhea Lauzon A00881688
**
**	PROGRAMMER: Filip Gutica A00781910
**				Rhea Lauzon A00881688
**
** Notes :
**
***********************************************************************/
#define STRICT
#define _CRT_SECURE_NO_WARNINGS	
#include "Application.h"

#pragma warning (disable: 4096)

//Background colors
COLORREF oppositeBackground = 0xCCFFCC;
COLORREF currentBackground = 0x00AAAAAA;
COLORREF backgroundColorCommand = 0x00AAAAAA;
COLORREF backgroundColorConnected = 0xCCFFCC;

HWND hwnd;
HWND button;
HWND sendDisplay;
HWND recievedDisplay;
HINSTANCE hInst;
HDC hdc;
string buffer;
string outText;
RECT txtWindow;
Statistics *stats;

/*******************************************************************
** Function: WinMain
**
** Date: September 3rd, 2014
**
** Revisions: September 16th, 2014
**				Altered to become Dumb Terminal compatiable.
**
** Designer: Aman Abdulla
**
** Programmer: Aman Abdulla with revisions by Rhea Lauzon
**
** Interface:
**			int WINAPI WinMain (HINSTANCE, HINSTANCE, LSTR, int)
** Returns:
**			int
**
** Notes:
** This function is called to create the Window with the menu items.
**
*******************************************************************/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
{
	MSG Msg;
	
	stats = Statistics::GetInstance();

	InstantiateWindow(hInst);

	ShowWindow(hwnd, nCmdShow);

	UpdateWindow(hwnd);

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	
	return Msg.wParam;
}

/*******************************************************************
** Function: WndProc
**
** Date: September 16th, 2014
**
** Revisions: N/A
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface: LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM)
**
** Returns: LRESULT
**
** Notes:
**This function is called on every frame of the window to receive
** various messages that the window may generate. Most of the messages
** are defaulted to be proccessed by default Windows.
*******************************************************************/
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message,
	WPARAM wParam, LPARAM lParam)
{
	HMENU hMenu;
	PAINTSTRUCT paintstruct;
	char * strSend = new char[LINE_SIZE];
	
	switch (Message)
	{
	case WM_CREATE:
			hdc = GetDC(hwnd);
			ReleaseDC(hwnd, hdc);
		break;

	case WM_COMMAND:
			// Check user menu selections
			CheckMenu(wParam);
		break;

	case WM_CHAR:							
			hdc = GetDC(hwnd);			
			//if (isConnected())
			//{
				sprintf(strSend, "%c", (char)wParam);
				// Send chars from keyboard to WritePort()
				//WritePort(strSend);
				//BuildBuffer(strSend);
			//}
			ReleaseDC(hwnd, hdc);							
		break;

	case WM_PAINT:								
			hdc = BeginPaint(hwnd, &paintstruct);
			SetBkMode(hdc, TRANSPARENT);
			if (isConnected())
			{
				// Output received characters
				OutputText();
			}
			EndPaint(hwnd, &paintstruct);
		break;
	case WM_SIZE:
		if ( sendDisplay != NULL )
			UpdateUI();
		break;
	//Change to colored backgrounds
	case WM_ERASEBKGND:
		HPEN pen;
		HBRUSH brush;
		RECT rect;
		pen = CreatePen(PS_SOLID, 1, currentBackground);
		brush = CreateSolidBrush(currentBackground);
		SelectObject((HDC)wParam, pen);
		SelectObject((HDC)wParam, brush);

		GetClientRect(hwnd, &rect);
		Rectangle((HDC)wParam, rect.left, rect.top, rect.right, rect.bottom);
		break;


	case WM_DESTROY:
			if (isConnected())
			{
				closePort();
			}
			PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	PrintStats();
	return 0;
}


/*******************************************************************************
**	FUNCTION: InstantiateWindow
**
** DATE:	September 16th, 2014
**
** REVISIONS:	 December 1st, 2014 / Moved UI stuff to another function.
**
** DESIGNER:	Filip Gutica
**
** PROGRAMMER:	Filip Gutica
**				Marc Vouve
**
** INTERFACE:	void InstantiateWindow(HINSTANCE hInst)
**
** RETURNS: void
**
** NOTES:
** This function is called upon window creation to create the
** basic window.
********************************************************************************/
void InstantiateWindow(HINSTANCE hInst)
{
	WNDCLASSEX Wcl;
	char Name[] = "Grapefruit Protocol Implementation";

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION);	// large icon 
	Wcl.hIconSm = NULL;								// use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);		// cursor style

	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //white background
	Wcl.lpszClassName = Name;

	Wcl.lpszMenuName = "MYMENU";					// The menu Class
	Wcl.cbClsExtra = 0;								// no extra memory needed
	Wcl.cbWndExtra = 0;

	if (!RegisterClassEx(&Wcl))
		return;

	hwnd = CreateWindow(Name, Name, WS_OVERLAPPEDWINDOW ,
		10, 10, 700, 500, NULL, NULL, hInst, NULL);

	InitializeUI();
}


/***********************************************************************************
 * FUNCTION: InitialiseUI
 *
 * DATE: December 1st, 2014
 *
 * REVISIONS: N/A
 *
 * DESIGNER:    Marc Vouve
 *
 * PROGRAMMER:  Filip Gutica
 *				Rhea Lauzon
 *              Marc Vouve
 *
 * INTERFACE:   void InitialiseUI
 *
 * RETURNS:     void
 *
 * NOTES:
 * This function was moved out of WinMain for conveniance and to allow the child
 * windows to be resized when the main window is resized.
 *
 **********************************************************************************/
void InitializeUI()
{


	// Get the current EXE's hInst, if you're making a DLL with this it wont work.
	PAINTSTRUCT  paintStruct;
	HINSTANCE hInst = GetModuleHandle(NULL);
	RECT clientRect;
	RECT statsRect;
	HDC hdc = GetDC(hwnd);

	// Get the users window
	GetClientRect(hwnd, &clientRect);

	INT windowWidth  = clientRect.right - clientRect.left;
	INT windowHeight = clientRect.bottom - clientRect.top;
	statsRect.bottom = clientRect.bottom;
	statsRect.top = clientRect.top + ( windowHeight / 20 ) * 10;
	statsRect.left = clientRect.left + (windowWidth / 20) * 8;
	statsRect.right = clientRect.right + (windowWidth / 20) * 12;
	
	HDC PaintDC = BeginPaint(hwnd, &paintStruct );

	DestroyWindow(recievedDisplay);
	DestroyWindow(sendDisplay);
	DestroyWindow(button);
	
	
	// Display area for messages recieved.
	recievedDisplay = CreateWindowA(
		"EDIT",
		buffer.c_str(),
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | ES_READONLY  | ES_MULTILINE,
		//x
		10,
		//y
		10,
		//width
		(windowWidth / 20 ) * 8 - 10,
		//height
		windowHeight - 20,
		hwnd, NULL,
		hInst, NULL
		);

	sendDisplay = CreateWindow(
		"EDIT",
		NULL,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE,
		(windowWidth / 20) * 12,
		10,
		( windowWidth / 20 ) * 8,
		windowHeight - 20,
		hwnd, (HMENU)IDM_TEXT,
		hInst, NULL
		);


	// Send button
	button = CreateWindow(
		"button",
		"Send Message",
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		//x
		(windowWidth / 2) - 60,
		//y
		(windowHeight / 2 ) - 60,
		//width
		120,
		//height
		40,
		hwnd, (HMENU) IDM_SEND_BUTTON,
		hInst, NULL
		);
	SetBkColor(hdc, currentBackground);
	
	DrawText(hdc, PrintStats().c_str(), -1, &statsRect, 0 );
	//TextOut(PaintDC, clientRect.right + ( windowWidth / 2 ), windowHeight / 2, PrintStats(), sizeof(PrintStats()));

	EndPaint( hwnd, &paintStruct );
	ReleaseDC(hwnd, hdc);
}


/***********************************************************************************
* FUNCTION: InitialiseUI
*
* DATE: December 1st, 2014
*
* REVISIONS: N/A
*
* DESIGNER:    Marc Vouve
*
* PROGRAMMER:  Filip Gutica
*				Rhea Lauzon
*              Marc Vouve
*
* INTERFACE:   void InitialiseUI
*
* RETURNS:     void
*
* NOTES:
* This function was moved out of WinMain for conveniance and to allow the child
* windows to be resized when the main window is resized.
*
**********************************************************************************/
void UpdateUI()
{


	// Get the current EXE's hInst, if you're making a DLL with this it wont work.
	PAINTSTRUCT  paintStruct;
	HINSTANCE hInst = GetModuleHandle(NULL);
	RECT clientRect;
	RECT statsRect;
	HDC hdc = GetDC(hwnd);

	// Get the users window
	GetClientRect(hwnd, &clientRect);

	INT windowWidth = clientRect.right - clientRect.left;
	INT windowHeight = clientRect.bottom - clientRect.top;
	statsRect.bottom = clientRect.bottom;
	statsRect.top = clientRect.top + (windowHeight / 20) * 10;
	statsRect.left = clientRect.left + (windowWidth / 20) * 8;
	statsRect.right = clientRect.right + (windowWidth / 20) * 12;




	// Display area for messages recieved.
	MoveWindow(
		recievedDisplay,
		//x
		clientRect.left + 10,
		//y
		clientRect.top + 10,
		//width
		(windowWidth / 20) * 8 - 10,
		//height
		windowHeight - 20,
		0
		);

	MoveWindow(
		sendDisplay,
		(windowWidth / 20) * 12,
		10,
		(windowWidth / 20) * 8,
		windowHeight - 20,
		0
		);


	// Send button
	MoveWindow(
		button,
		//x
		(windowWidth / 2) - 60,
		//y
		(windowHeight / 2) - 60,
		//width
		120,
		//height
		40,
		0
		);
	SetBkColor(hdc, currentBackground);

	DrawText(hdc, PrintStats().c_str(), -1, &statsRect, 0);
	//TextOut(PaintDC, clientRect.right + ( windowWidth / 2 ), windowHeight / 2, PrintStats(), sizeof(PrintStats()));

	ReleaseDC(hwnd, hdc);
	UpdateWindow(hwnd);
}


/**********************************************************************************
**	FUNCTION: CheckMenu
**
** DATE: September 16th, 2014
**
** REVISIONS:	N/A
**
** DESIGNER:	Filip Gutica
**
** PROGRAMMER:	Filip Gutica
**
** INTERFACE:	void CheckMenu(WPARAM wP)
**
** RETURNS:	void
**
** NOTES:
**This function is called on every frame of the window to receive
** various menu messages.
************************************************************************************/

void CheckMenu(WPARAM wP)
{
	int value;
	HMENU hMenu = GetMenu(hwnd);
	switch (LOWORD(wP))
	{
	case IDM_COM1:
			SetPortSettings("com1", hwnd);
		break;

	case IDM_COM2:
			SetPortSettings("com2", hwnd);
		break;

	case IDM_COM3:
			SetPortSettings("com3", hwnd);
		break;

	case IDM_CONNECT:
			Connect();
			ConnectMode(hMenu);
		break;

	case IDM_DISCONNECT:
			CommandMode(hMenu);
			setConnected(false);
			closePort();
		break;

	case IDM_HELP:
		MessageBox(hwnd, "This program implements the Grapefruit Protocol.\n"
			"\n Make sure your port matches the one on your computer."
			"\n You can set the settings of your port in the Port Settings.", "Help", MB_OK | MB_ICONQUESTION);
		break;

	
	case IDM_SEND_BUTTON:
		//handle button press
		//MessageBox(hwnd, "Pressed button", "Button", MB_OK);
		value = GetWindowTextLength(GetDlgItem(hwnd, IDM_TEXT));

		if (isConnected())
		{
			//Display chars if the length is greater than 0
			if ( value > 0 )
			{
				char* buf;

				buf = (char*)GlobalAlloc(GPTR, value + 1);
				GetDlgItemText(hwnd, IDM_TEXT, buf, value + 1);	

				if (getMode() == WAITING )
				{
					stats = Statistics::GetInstance();
					//Enter the write mode
					setMode(WRITE);
					WriteControlChar(ENQ);
					stats->IncrementENQS();
					UpdateStats();
				}
				PacketFactory(buf);
				BuildPacket();
				//append the buffer to the global buffer
				outText.append(buf);
				//Clear the textfield
				SetDlgItemText(hwnd, IDM_TEXT, "");
			}
		}
		break;

	case IDM_EXIT:
		PostQuitMessage(0);
		break;
	}
}

/**********************************************************************************
**	FUNCTION: GetCharsFromPort()
**
** DATE: September 16th, 2014
**
** REVISIONS:	N/A
**
** DESIGNER:	Filip Gutica
**
** PROGRAMMER:	Filip Gutica
**
** INTERFACE:	void GetCharsFromPort(char *)
**
** RETURNS:	void
**
** NOTES:
** This functions gets chars from the port and adds them to the buffer
************************************************************************************/
void GetCharsFromPort(char *c)
{
	//strcat((char *)buffer.c_str(), c);
	buffer += c;
	InvalidateRect(hwnd, NULL, FALSE);
}

/**********************************************************************************
**	FUNCTION: OutputText()
**
** DATE: September 16th, 2014
**
** REVISIONS:	N/A
**
** DESIGNER:	Filip Gutica
**
** PROGRAMMER:	Filip Gutica
**
** INTERFACE:	OutputText()
**
** RETURNS:	void
**
** NOTES:
** This functions displays the main buffer to the screen
************************************************************************************/
void OutputText()
{
	GetClientRect(hwnd, &txtWindow);
	hdc = GetDC(hwnd);

	//Change background to transparent
	SetBkMode(hdc, TRANSPARENT);

	OutputDebugString(buffer.c_str());
	DrawText(hdc, buffer.c_str(), 
			strlen(buffer.c_str()), 
			&txtWindow, 
			// Set the formatting of the drawn text
			DT_EDITCONTROL | DT_WORDBREAK );

	// Makes the text wrap around the window.
	ReleaseDC(hwnd, hdc);					 
}

/**********************************************************************************
**	FUNCTION: OutputText()
**
** DATE: September 16th, 2014
**
** REVISIONS:	N/A
**
** DESIGNER:	Filip Gutica
**
** PROGRAMMER:	Filip Gutica
**
** INTERFACE:	void OutputText(char *buffer)
**
** RETURNS:	void
**
** NOTES:
** This functions displays a buffer's contents to the screen
************************************************************************************/
void OutputText(string buffer)
{
	GetClientRect(hwnd, &txtWindow);
	hdc = GetDC(hwnd);

	//Change background to transparent
	SetBkMode(hdc, TRANSPARENT);

	DrawText(hdc, buffer.c_str(), 
			strlen(buffer.c_str()), 
			&txtWindow, 
			// Set the formatting of the drawn text
			DT_EDITCONTROL | DT_WORDBREAK );

	// Makes the text wrap around the window.
	ReleaseDC(hwnd, hdc);					 
}



/*********************************************************************
** Function: ConnectMode
**
** DATE:	September 18th, 2014
**
** REVISIONS:	N/A
**
** DESIGNER:	Rhea Lauzon
**
** PROGRAMMER:	Rhea Lauzon
**
** INTERFACE:	void connectedMode(HMENU)
**
** RETURNS:	void
**
** NOTES:
** This function is called when the program enters connected mode.
** The Window's background color is changed to green nd all menu items
** are disabled. The Connect button is also replaced by a disconnect button.
**********************************************************************/
void ConnectMode(HMENU menuHandle)
{
	if (isConnected())
	{
		ModifyMenu(menuHandle, IDM_CONNECT, MF_STRING, IDM_DISCONNECT, "Disconnect");
		disableMenuItems(menuHandle);
		ChangeBackgroundColor(hwnd, backgroundColorConnected);
		DrawMenuBar(hwnd);
		setConnected(true);
	}
	return;
}

/*********************************************************************
** Function: CommandMode
**
** DATE:	September 18th, 2014
**
** REVISIONS:	October 8th, 2014
**				Updated for RFID Scanner
**
** DESIGNER:	Rhea Lauzon
**
** PROGRAMMER:	Rhea Lauzon
**
** INTERFACE:	void CommandMode(HMENU)
**
** RETURNS:	void
**
** NOTES:
** This function is called after the terminal has been in connect mode.
** The Window's background color is changed back to gray and all menu items
** are enabled.
**********************************************************************/
void CommandMode(HMENU menuHandle)
{
	ChangeBackgroundColor(hwnd, backgroundColorCommand);
	ModifyMenu(menuHandle, IDM_DISCONNECT, MF_STRING, IDM_CONNECT, "Connect");
	enableMenuItems(menuHandle);
	DrawMenuBar(hwnd);
	setConnected(false);
	return;

}

/*******************************************************************
** Function: ChangeBackgroundColor
**
** DATE: September 22nd, 2014
**
** REVISIONS: October 9th, 2014
**				Added opposite color ability.
**
** DESIGNER: Rhea Lauzon
**
** PROGRAMMER: Rhea Lauzon
**
** INTERFACE: void ChangeBackgroundColor(HWND, COLORREF)
**
** RETURNS: void.
**
** NOTES:
** Changes the background depending on the terminal's states.
*******************************************************************/
void ChangeBackgroundColor(HWND hwnd, COLORREF color)
{
	oppositeBackground = currentBackground;
	currentBackground = color;
	InvalidateRect(hwnd, NULL, true);
}

/*******************************************************************
** Function: disableMenuItems
**
** Date: September 20th, 2014
**
** Revisions: N/A
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface: void disableMenuItems(HMENU)
**
** Returns: void
**
** Notes:
** This function is called when the terminal enters connect mode.
** All buttons are disabled.
*******************************************************************/
void disableMenuItems(HMENU menuHandle)
{
	EnableMenuItem(menuHandle, IDM_HELP, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(menuHandle, IDM_EXIT, MF_BYCOMMAND | MF_GRAYED);
	//EnableMenuItem(menuHandle, IDM_COM1, MF_BYCOMMAND | MF_GRAYED);
	//EnableMenuItem(menuHandle, IDM_COM2, MF_BYCOMMAND | MF_GRAYED);
	//EnableMenuItem(menuHandle, IDM_COM3, MF_BYCOMMAND | MF_GRAYED);
}

/*******************************************************************
** Function: enableMenuItems
**
** Date: September 20th, 2014
**
** Revisions: N/A
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface: void disableMenuItems(HMENU)
**
** Returns: void
**
** Notes:
** This function is called when the terminal re-enters command mode.
** All the menu items are re-enabled.
*******************************************************************/
void enableMenuItems(HMENU menuHandle)
{
	EnableMenuItem(menuHandle, IDM_HELP, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(menuHandle, IDM_EXIT, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(menuHandle, IDM_COM1, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(menuHandle, IDM_COM2, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(menuHandle, IDM_COM3, MF_BYCOMMAND | MF_ENABLED);
}


/*******************************************************************
** Function: PrintStats
**
** Date: November 29th, 2014
**
** Revisions: N/A
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface: void PrintStats()
**
** Returns: void
**
** Notes:
** This function prints the stats recorded from the protocol.
*******************************************************************/
std::string PrintStats()
{
	std::stringstream s;

	// Add number of ENQs
	s << "ENQS: " << stats->GetENQS() << "\r\n";
	// Add number of ACKs
	s << "ACKS Sent: " << stats->GetACKSent() << "\r\n";
	s << "ACKS Received: " << stats->GetACKReceived() << "\r\n";
	// Add number of NAKs
	s << "NAKS: " << stats->GetNAKS() << "\r\n";
	// Add number of Sent
	s << "Sent: " << stats->GetPacketsSent() << "\r\n";
	// Add number of Lost Packets
	s << "Lost: " << stats->GetPacketsLost() << "\r\n";
	// Add number of Recieved packets
	s << "Received: " << stats->GetReceived() << "\r\n";
	// Add number of received packets corrupted
	s << "Corrupted Packets: " << stats->GetReceived() << "\r\n";

	return ( s.str() );
}

void UpdateStats()
{
	UpdateUI();
}