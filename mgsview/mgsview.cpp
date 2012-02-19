// mgsview.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <Windows.h>
#include <gl/GL.h>
#include <stdio.h>
#include <math.h>
#include <CommDlg.h>
#include "ArcBall.h"
#include "kmd.h"
#include "dar.h"
#include "vram.h"
#include "OBJExport.h"

HDC			hDC=NULL;		// Private GDI Device Context
HGLRC		hRC=NULL;		// Permanent Rendering Context
HWND		hWnd=NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;	
BOOL fullscreen = false;
BOOL active;
#define FILE_SIZE 0x310
short *buffer = (short*)malloc(FILE_SIZE);
GLuint dl = 0;

const float PI2 = 2.0*3.1415926535f;								// PI Squared

Matrix4fT   Transform   = {  1.0f,  0.0f,  0.0f,  0.0f,				// NEW: Final Transform
                             0.0f,  1.0f,  0.0f,  0.0f,
                             0.0f,  0.0f,  1.0f,  0.0f,
                             0.0f,  0.0f,  0.0f,  1.0f };

Matrix3fT   LastRot     = {  1.0f,  0.0f,  0.0f,					// NEW: Last Rotation
                             0.0f,  1.0f,  0.0f,
                             0.0f,  0.0f,  1.0f };

Matrix3fT   ThisRot     = {  1.0f,  0.0f,  0.0f,					// NEW: This Rotation
                             0.0f,  1.0f,  0.0f,
                             0.0f,  0.0f,  1.0f };

ArcBallT    ArcBall(640.0f, 480.0f);				                // NEW: ArcBall Instance
Point2fT    MousePt;												// NEW: Current Mouse Point
bool        isClicked  = false;										// NEW: Clicking The Mouse?
bool        isRClicked = false;										// NEW: Clicking The Right Mouse Button?
bool        isDragging = false;	

GLvoid ReSizeGLScene(GLsizei width, GLsizei height);

LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
		case WM_ACTIVATE:							// Watch For Window Activate Message
		{
			if (!HIWORD(wParam))					// Check Minimization State
			{
				active=TRUE;						// Program Is Active
			}
			else
			{
				active=FALSE;						// Program Is No Longer Active
			}

			return 0;								// Return To The Message Loop
		}

		case WM_SYSCOMMAND:							// Intercept System Commands
		{
			switch (wParam)							// Check System Calls
			{
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
				return 0;							// Prevent From Happening
			}
			break;									// Exit
		}

		case WM_CLOSE:								// Did We Receive A Close Message?
		{
			PostQuitMessage(0);						// Send A Quit Message
			return 0;								// Jump Back
		}

		case WM_KEYDOWN:							// Is A Key Being Held Down?
		{
			return 0;								// Jump Back
		}

		case WM_KEYUP:								// Has A Key Been Released?
		{
			return 0;								// Jump Back
		}

		case WM_SIZE:								// Resize The OpenGL Window
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		}
		case WM_RBUTTONDOWN:
				isRClicked = true;
				break;
		case WM_RBUTTONUP:
			isRClicked = false;
			break;
		case WM_LBUTTONDOWN:
			isClicked   = true;
			break;				
		case WM_LBUTTONUP:
				isClicked  = false;
				break;		
		case WM_MOUSEMOVE:
			//System::IntPtr value;
			//System::IntPtr buttons;
			//value = m.LParam;
			//buttons = m.WParam;

			MousePt.s.X = (GLfloat)LOWORD(lParam);
			MousePt.s.Y = (GLfloat)HIWORD(lParam);
			//printf("%f\n%f\n\n", MousePt.s.X, MousePt.s.Y);
			isClicked   = (LOWORD(wParam) & MK_LBUTTON) ? true : false;
			isRClicked  = (LOWORD(wParam) & MK_RBUTTON) ? true : false;
			break;
	}

	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	double ymax, ymin, xmax, xmin = 0;
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();		
	
	ymax = 0.1f * tan(45.0f * M_PI / 360.0);
	ymin = -ymax;
	xmin = ymin * (GLfloat)width/(GLfloat)height;
	xmax = ymax * (GLfloat)width/(GLfloat)height;
	glFrustum(xmin, xmax, ymin, ymax, 0.1f, 5000.0f);
	//glFrontFace(GL_CW);
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
	ArcBall.setBounds(width, height);
}

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations	
	return TRUE;										// Initialization Went OK
}

int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{	
	if (isRClicked)													// If Right Mouse Clicked, Reset All Rotations
			{
				Matrix3fSetIdentity(&LastRot);								// Reset Rotation
				Matrix3fSetIdentity(&ThisRot);								// Reset Rotation
				Matrix4fSetRotationFromMatrix3f(&Transform, &ThisRot);		// Reset Rotation
			}

			if (!isDragging)												// Not Dragging
			{
				if (isClicked)												// First Click
				{
					isDragging = true;										// Prepare For Dragging
					LastRot = ThisRot;										// Set Last Static Rotation To Last Dynamic One
					ArcBall.click(&MousePt);								// Update Start Vector And Prepare For Dragging
				}
			}
			else
			{
				if (isClicked)												// Still Clicked, So Still Dragging
				{
					Quat4fT     ThisQuat;

					ArcBall.drag(&MousePt, &ThisQuat);						// Update End Vector And Get Rotation As Quaternion
					Matrix3fSetRotationFromQuat4f(&ThisRot, &ThisQuat);		// Convert Quaternion Into Matrix3fT
					Matrix3fMulMatrix3f(&ThisRot, &LastRot);				// Accumulate Last Rotation Into This One
					Matrix4fSetRotationFromMatrix3f(&Transform, &ThisRot);	// Set Our Final Transform's Rotation From This One
				}
				else														// No Longer Dragging
					isDragging = false;
			}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glTranslatef(0, 0, -1.0f);
	glPushMatrix();													// NEW: Prepare Dynamic Transform
	glMultMatrixf(Transform.M);	
	glCallList(dl);
	glPopMatrix();
	glLoadIdentity();									// Reset The Current Modelview Matrix
	return TRUE;										// Everything Went OK
}

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBoxA(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBoxA(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
	{
		MessageBoxA(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBoxA(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClassA("OpenGL",hInstance))			// Are We Able To Unregister Class
	{
		MessageBoxA(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}
}

BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASSA	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClassA(&wc))									// Attempt To Register The Window Class
	{
		MessageBoxA(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}
	
	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			if (MessageBoxA(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				MessageBoxA(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	if (!(hWnd=CreateWindowExA(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
					HINSTANCE	hPrevInstance,		// Previous Instance
					LPSTR		lpCmdLine,			// Command Line Parameters
					int			nCmdShow)			// Window Show State
{
	MSG		msg;									// Windows Message Structure
	BOOL	done=FALSE;								// Bool Variable To Exit Loop

	OPENFILENAMEA ofn;
    char szFileName[MAX_PATH] = "";
	char *fn;
	fn = GetCommandLineA();
	char *realfn = strrchr(fn, ' ')+1;

	if (!CreateGLWindow("MGSView",640,480,16,fullscreen))
	{
		return 0;									// Quit If Window Was Not Created
	}

	
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = "Konami Model (*.KMD)\0*.kmd\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "kmd";

    if(GetOpenFileNameA(&ofn)){
		KMD_Load(ofn.lpstrFile);
		//KMD_Load(realfn);
		dl = KMD_DrawPoints();
		//return 0;
	}
	else
		return -1;
	
	ofn.lpstrFilter = "Konami Archive (*.DAR)\0*.dar\0All Files (*.*)\0*.*\0";
	ofn.lpstrDefExt = "dar";
	/*
	if(GetOpenFileNameA(&ofn))
		DAR_LoadTextures(ofn.lpstrFile);
	else
		return -1;
	GetOpenFileNameA(&ofn);
	DAR_LoadTextures(ofn.lpstrFile);
	GetOpenFileNameA(&ofn);
	DAR_LoadTextures(ofn.lpstrFile);
	GetOpenFileNameA(&ofn);
	DAR_LoadTextures(ofn.lpstrFile);*/
	
	//VRAM_Save();
	//return 0;
	//KMD_Export();
	//return 0;
	while(!done)									// Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
			{
				done=TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			if (active)								// Program Active?
			{
				if(dl){
					DrawGLScene();					// Draw The Scene
					SwapBuffers(hDC);				// Swap Buffers (Double Buffering)
				}
			}			
		}
	}
	
	DrawGLScene();					// Draw The Scene
	OBJExport();
	KillGLWindow();									// Kill The Window
	return (msg.wParam);							// Exit The Program
}
