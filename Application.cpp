/*******************************************************************************
** Source File : Applicationn.cpp -- An application that simulates a dumb terminal.
**
** Program : dumbTerminal
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
** The program begins in command mode where the port's settings and
** port number can be changed before a connection begins.Once connection
** occurs all menu items except "Disconnect" become unavailable as the program
** enters a read and write loop until the 'ESC' key is pressed or the
** disconnect button is pressed.
**
** The program will monitor a port for characters being sent by another
** device, however due to a non - overlapped structure the program will
** hang unless characters are being sent and receive at the same time.
**
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
HWND textField;
HDC hdc;
string buffer;
string tempBuffer;
RECT txtWindow;

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
				BuildBuffer(strSend);
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
	return 0;
}


/*******************************************************************************
**	FUNCTION: InstantiateWindow
**
** DATE:	September 16th, 2014
**
** REVISIONS:	 N/A
**
** DESIGNER:	Filip Gutica
**
** PROGRAMMER:	Filip Gutica
**
** INTERFACE:	void InstantiateWindow(HINSTANCE hInst)
**
** RETURNS: void
**
** NOTES:
** This function is called upon window creation to create the
** basic window.
*******************************************************************/
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


	button = CreateWindow( 
				"button", 
				"Send Msssage",
                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
				//x
                520,
				//y
				370, 
				//width
                150, 
				//height
				50,
                hwnd, (HMENU) IDM_SEND_BUTTON,
                hInst, NULL );

	textField = CreateWindowEx(WS_EX_CLIENTEDGE,
								"Edit",
								NULL,
								WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
								//x
								10,
								//y
								390, 
								//width
								500,
								//height
								25,
								hwnd, (HMENU) IDM_TEXT,
								hInst, NULL);


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
		MessageBox(hwnd, "Pressed button", "Button", MB_OK);
		value = GetWindowTextLength(GetDlgItem(hwnd, IDM_TEXT));

		//Display chars if the length is greater than 0
		if ( value > 0 )
		{
			int i;
			char* buf;

			buf = (char*)GlobalAlloc(GPTR, value + 1);
			GetDlgItemText(hwnd, IDM_TEXT, buf, value + 1);
			//append the buffer to the global buffer
			tempBuffer.append(buf);

			//Clear the textfield
			SetDlgItemText(hwnd, IDM_TEXT, "");

			OutputText(tempBuffer);
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