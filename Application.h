/*****************************************************************************************************
**	SOURCE FILE:	Application.h -	Header file for Application.cpp
**
**	Purpose:	Contains function definitions used in Application.cpp
**
**	DATE: 		September 28, 2014
**
**	DESIGNER: 	Filip Gutica A00781910
**
**	PROGRAMMER: Filip Gutica A00781910
**
*********************************************************************************************************/
#ifndef application_h
#define application_h

//Physical layer header file
#include <sstream>
#include "Physical.h" 
#include "Protocol.h"
#include "Windowsx.h"
#include "statistics.h"

/* 
 Function prototypes used in Application.cpp and Physical.cpp
 as we will include this header in Physical.cpp as well
*/
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void CheckMenu(WPARAM);
void InstantiateWindow(HINSTANCE);
void OutputText();
void OutputText( string );
void GetCharsFromPort(char * c);
void ChangeBackgroundColor(HWND, COLORREF);
void ConnectMode(HMENU);
void CommandMode(HMENU);
void disableMenuItems(HMENU);
void enableMenuItems(HMENU);
void PrintStats();
void UpdateStats();

#endif
