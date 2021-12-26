#ifndef _FILE_DEFINED
struct _iobuf {
    char* _ptr;
    int   _cnt;
    char* _base;
    int   _flag;
    int   _file;
    int   _charbuf;
    int   _bufsiz;
    char* _tmpfname;
};
typedef struct _iobuf FILE;
#define _FILE_DEFINED
#endif
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <Windows.h>
#include <string>
#include <thread>


typedef struct WINDOWSTRUCTURE_t
{
    int x, y;
    int width, height;
    long WindowStyle;
}WindowStructure;

struct Point
{
    int x, y;
};

struct ColorRGB
{
    int r, g, b;
};

struct Pixel
{
    Point m_Point;
    ColorRGB m_Color;
};

static struct ThreadSharedData
{
    bool Lock;
    bool Interrupt;
    bool Pause;
    bool DualLock;
    int IdleIntervalMillis;
    Pixel PixelBuffer;
    HWND* hWnd;
}SharedData;


bool IsPointEquals(const Point& source, const Point& target)
{
    return source.x == target.x && source.y == target.y;
}
bool IsColorEquals(const ColorRGB& source, const ColorRGB& target)
{
    return source.r == target.r && source.g == target.g && source.b == target.b;
}
bool IsPixelEquals(const Pixel& source, const Pixel& target)
{
    return IsPointEquals(source.m_Point, target.m_Point) && IsColorEquals(source.m_Color, target.m_Color);
}
void GetPixelData(Pixel* ptrPx, int x, int y)
{
    ptrPx->m_Point = { x,y };
    COLORREF c = GetPixel(GetDC(NULL), ptrPx->m_Point.x, ptrPx->m_Point.y);
    ptrPx->m_Color = { GetRValue(c),GetGValue(c),GetBValue(c) };
}
void GetPixelData(Pixel* ptrPx, const Point& p)
{
    ptrPx->m_Point = { p.x,p.y };
    COLORREF c = GetPixel(GetDC(NULL), ptrPx->m_Point.x, ptrPx->m_Point.y);
    ptrPx->m_Color = { GetRValue(c),GetGValue(c),GetBValue(c) };
}
void GetPixelData(Pixel* ptrPx)
{
    COLORREF c = GetPixel(GetDC(NULL), ptrPx->m_Point.x, ptrPx->m_Point.y);
    ptrPx->m_Color = { GetRValue(c),GetGValue(c),GetBValue(c) };
}


std::thread thread;

HWND* pMainWindowHandle;
WNDCLASSW* pMainWindowClass;
WindowStructure* pWndStructure;

void ThreadProc(ThreadSharedData* pSharedData)
{
    pSharedData->Lock = true;
    while (!pSharedData->Interrupt)
    {
        Sleep(pSharedData->IdleIntervalMillis);

        if (GetAsyncKeyState(VK_F1) != 0)
        {
            pSharedData->Pause = false;
        }
        else if (GetAsyncKeyState(VK_F2) != 0)
        {
            pSharedData->Pause = true;
        }
        if (pSharedData->Pause)
            continue;
        POINT p;
        GetCursorPos(&p);
        Pixel tmp{};
        GetPixelData(&tmp, { p.x, p.y });
        if (!IsPixelEquals(tmp, pSharedData->PixelBuffer))
        {
            pSharedData->PixelBuffer = tmp;
        	InvalidateRect(*pMainWindowHandle, 0, false);
        }
    }
    pSharedData->Lock = false;
}

bool InitThread()
{
    if (!SharedData.Lock)
    {
        thread = std::thread(ThreadProc, &SharedData);
        return true;
    }
    return false;
}

bool TerminateThread()
{
    if (SharedData.Lock)
    {
        SharedData.Interrupt = true;
        thread.join();
        return true;
    }
    return false;
}


typedef LRESULT (*WINPROC)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HWND Label_RelToX;
HWND Label_RelToY;
HWND Label_X;
HWND Label_Y;
HWND Label_RelX;
HWND Label_RelY;

HWND Label_R;
HWND Label_G;
HWND Label_B;

HWND Field_RelToX;
HWND Field_RelToY;
HWND Field_X;
HWND Field_Y;
HWND Field_RelX;
HWND Field_RelY;


HWND Field_R;
HWND Field_G;
HWND Field_B;

RECT Field_COLOR;

HWND Label_PauseInfo;
HWND Label_ResumeInfo;

POINT RelativeToPoint;

bool RegisterWindowClass(WNDCLASSW* buff, PCWCHAR className, WINPROC winProc)
{
    *buff = {
        CS_HREDRAW | CS_VREDRAW,
        winProc,
        0,
        0,
        0, // hinst
        LoadIcon(NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW),
        GetSysColorBrush(COLOR_3DFACE),
        NULL,
        className
        };
    return RegisterClassW(buff);
}

bool CreateHandle(HWND* pHwnd, PCWCHAR className, PCWCHAR windowName, HWND parent, const WINDOWSTRUCTURE_t* winStruct)
{
    *pHwnd = CreateWindow(className, windowName,winStruct->WindowStyle, winStruct->x, winStruct->y, winStruct->width, winStruct->height, parent, 0, 0,0);
    return *pHwnd;
}


LRESULT CALLBACK MainWindowLoop(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        //printf("Window created\n");
        break;
    case WM_DESTROY:

        PostQuitMessage(0);
        break;
    
    case WM_COMMAND:
        if (HIWORD(wParam) == EN_CHANGE)
        {

            int x, y;
            PCHAR str1 = (PCHAR)calloc(5,sizeof(CHAR) );
            PCHAR str2 = (PCHAR)calloc(5,sizeof(CHAR));
            if (str1)
            {
                GetWindowTextA(Field_RelToX, str1, 5);
                x = atoi(str1);
                if (RelativeToPoint.x != x) {
                    RelativeToPoint.x = x;
                    break;
                }
            }

            if (str2)
            {
                GetWindowTextA(Field_RelToY, str2, 5);
                y = atoi(str2);
                if (RelativeToPoint.y != y) {
                    RelativeToPoint.y = y;
                    break;
                }
            }
        }
        break;

    case WM_PAINT:
        SetWindowTextA(Field_X, std::to_string(SharedData.PixelBuffer.m_Point.x).c_str());
        SetWindowTextA(Field_Y, std::to_string(SharedData.PixelBuffer.m_Point.y).c_str());

        double rx = ((double)SharedData.PixelBuffer.m_Point.x) / (RelativeToPoint.x);
        double ry = ((double)(SharedData.PixelBuffer.m_Point.y)) / (RelativeToPoint.y);
        SetWindowTextA(Field_RelX, std::to_string(rx).c_str());
        SetWindowTextA(Field_RelY, std::to_string(ry).c_str());

        SetWindowTextA(Field_R, std::to_string(SharedData.PixelBuffer.m_Color.r).c_str());
        SetWindowTextA(Field_G, std::to_string(SharedData.PixelBuffer.m_Color.g).c_str());
        SetWindowTextA(Field_B, std::to_string(SharedData.PixelBuffer.m_Color.b).c_str());

        HDC hdc = GetDC(*pMainWindowHandle);
        FillRect(hdc, &Field_COLOR, CreateSolidBrush(RGB(SharedData.PixelBuffer.m_Color.r, SharedData.PixelBuffer.m_Color.g, SharedData.PixelBuffer.m_Color.b)));
        ReleaseDC(*pMainWindowHandle, hdc);

        break;
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}

void CreateConsole()
{
    AllocConsole();

    HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
    int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
    FILE* hf_out = _fdopen(hCrt, "w");
    setvbuf(hf_out, NULL, _IONBF, 1);
    *stdout = *hf_out;

    HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
    hCrt = _open_osfhandle((long)handle_in, _O_TEXT);
    FILE* hf_in = _fdopen(hCrt, "r");
    setvbuf(hf_in, NULL, _IONBF, 128);
    *stdin = *hf_in;
}



void InitResources()
{
    pMainWindowHandle = (HWND*)malloc(sizeof(HWND));
    pMainWindowClass = (WNDCLASSW*)malloc(sizeof(WNDCLASSW));
    pWndStructure = (WindowStructure*)malloc(sizeof(WindowStructure));
    pWndStructure->x = CW_USEDEFAULT;
    pWndStructure->y = CW_USEDEFAULT;
    pWndStructure->width = 280;
    pWndStructure->height = 400;
    pWndStructure->WindowStyle = WS_VISIBLE | WS_OVERLAPPEDWINDOW;

    SharedData.IdleIntervalMillis = 20;
    SharedData.hWnd = pMainWindowHandle;
    SystemParametersInfo(SPI_SETFONTSMOOTHING,
        TRUE,
        0,
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    SystemParametersInfo(SPI_SETFONTSMOOTHINGTYPE,
        0,
        (PVOID)FE_FONTSMOOTHINGCLEARTYPE,
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

}





void InitializeComponents()
{
    const wchar_t* Common_className = L"STATIC";
    const  int CommonWidth = 60;
    const int CommonHeight = 30;
    
    const int CommonLabelX = 50;
    const int CommonFieldX = 100+ CommonLabelX;

    long CommonStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP ;

    const int VerticalMargin = 30;
    int count = 0;

    const  int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    const  int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    RelativeToPoint = { screenWidth,screenHeight };
    CreateHandle(&Label_RelToX, Common_className, L"RelToX: ", *pMainWindowHandle, new WindowStructure{ CommonLabelX,VerticalMargin * (count),CommonWidth,CommonHeight,CommonStyle });
    CreateHandle(&Field_RelToX, L"edit", std::to_wstring(screenWidth).c_str(), *pMainWindowHandle, new WindowStructure{ CommonFieldX,VerticalMargin * (count++),CommonWidth,20,CommonStyle | WS_BORDER | ES_RIGHT | ES_NUMBER });

    CreateHandle(&Label_RelToY, Common_className, L"RelToY: ", *pMainWindowHandle, new WindowStructure{ CommonLabelX,VerticalMargin * (count),CommonWidth,CommonHeight,CommonStyle });
    CreateHandle(&Field_RelToY,L"edit", std::to_wstring(screenHeight).c_str(), *pMainWindowHandle, new WindowStructure{CommonFieldX,VerticalMargin * (count++),CommonWidth,20,CommonStyle|WS_BORDER|ES_RIGHT|ES_NUMBER});

    CreateHandle(&Label_X, Common_className, L"X: ", *pMainWindowHandle, new WindowStructure{ CommonLabelX,VerticalMargin*(count),CommonWidth,CommonHeight,CommonStyle});
    CreateHandle(&Field_X, Common_className, L"0", *pMainWindowHandle, new WindowStructure{ CommonFieldX,VerticalMargin * (count++),CommonWidth,CommonHeight,CommonStyle });

    CreateHandle(&Label_Y, Common_className, L"Y: ", *pMainWindowHandle, new WindowStructure{ CommonLabelX,VerticalMargin * (count),CommonWidth,CommonHeight,CommonStyle });
    CreateHandle(&Field_Y, Common_className, L"0", *pMainWindowHandle, new WindowStructure{ CommonFieldX,VerticalMargin * (count++),CommonWidth,CommonHeight,CommonStyle });

    CreateHandle(&Label_RelX, Common_className, L"RelX: ", *pMainWindowHandle, new WindowStructure{ CommonLabelX,VerticalMargin * (count),CommonWidth,CommonHeight,CommonStyle });
    CreateHandle(&Field_RelX, Common_className, L"0", *pMainWindowHandle, new WindowStructure{ CommonFieldX,VerticalMargin * (count++),CommonWidth,CommonHeight,CommonStyle });

    CreateHandle(&Label_RelY, Common_className, L"RelY: ", *pMainWindowHandle, new WindowStructure{ CommonLabelX,VerticalMargin * (count),CommonWidth,CommonHeight,CommonStyle });
    CreateHandle(&Field_RelY, Common_className, L"0", *pMainWindowHandle, new WindowStructure{ CommonFieldX,VerticalMargin * (count++),CommonWidth,CommonHeight,CommonStyle });



    CreateHandle(&Label_R, Common_className, L"R: ", *pMainWindowHandle, new WindowStructure{ CommonLabelX,VerticalMargin * (count),CommonWidth,CommonHeight,CommonStyle });
    CreateHandle(&Field_R, Common_className, L"0", *pMainWindowHandle, new WindowStructure{ CommonFieldX,VerticalMargin * (count++),CommonWidth,CommonHeight,CommonStyle });

    CreateHandle(&Label_G, Common_className, L"G: ", *pMainWindowHandle, new WindowStructure{ CommonLabelX,VerticalMargin * (count),CommonWidth,CommonHeight,CommonStyle });
    CreateHandle(&Field_G, Common_className, L"0", *pMainWindowHandle, new WindowStructure{ CommonFieldX,VerticalMargin * (count++),CommonWidth,CommonHeight,CommonStyle });

    CreateHandle(&Label_B, Common_className, L"B: ", *pMainWindowHandle, new WindowStructure{ CommonLabelX,VerticalMargin * (count),CommonWidth,CommonHeight,CommonStyle });
    CreateHandle(&Field_B, Common_className, L"0", *pMainWindowHandle, new WindowStructure{ CommonFieldX,VerticalMargin * (count++),CommonWidth,CommonHeight,CommonStyle });

    Field_COLOR = { CommonLabelX,VerticalMargin * (count),CommonWidth + CommonFieldX,CommonHeight + VerticalMargin * (count++) };

    CreateHandle(&Label_PauseInfo, Common_className, L"Press F2 to PAUSE", *pMainWindowHandle, new WindowStructure{ CommonLabelX,VerticalMargin * (count++),150,CommonHeight,CommonStyle });
    CreateHandle(&Label_ResumeInfo, Common_className, L"Press F1 to RESUME", *pMainWindowHandle, new WindowStructure{ CommonLabelX,VerticalMargin * (count++),150,CommonHeight,CommonStyle });
    

    SetParent(Label_RelToX, *pMainWindowHandle);
    SetParent(Field_RelToX, *pMainWindowHandle);
    SetParent(Label_RelToY, *pMainWindowHandle);
    SetParent(Field_RelToY, *pMainWindowHandle);

    SetParent(Label_RelX, *pMainWindowHandle);
    SetParent(Field_RelX, *pMainWindowHandle);
    SetParent(Label_RelY, *pMainWindowHandle);
    SetParent(Field_RelY, *pMainWindowHandle);

    

    SetParent(Label_X, *pMainWindowHandle);
    SetParent(Field_X, *pMainWindowHandle);
    SetParent(Label_Y, *pMainWindowHandle);
    SetParent(Field_Y, *pMainWindowHandle);

    SetParent(Label_R, *pMainWindowHandle);
    SetParent(Field_R, *pMainWindowHandle);
    SetParent(Label_G, *pMainWindowHandle);
    SetParent(Field_G, *pMainWindowHandle);
    SetParent(Label_B, *pMainWindowHandle);
    SetParent(Field_B, *pMainWindowHandle);

    SetParent(Label_PauseInfo, *pMainWindowHandle);
    SetParent(Label_ResumeInfo, *pMainWindowHandle);
    
}



int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR szCmdLine, int CmdShow) {
   // CreateConsole();

    InitResources();
    
    if (!RegisterWindowClass(pMainWindowClass, L"MainWindow", MainWindowLoop))
        return 1;
    if (!CreateHandle(pMainWindowHandle, L"MainWindow", L"Pixel Finder", 0, pWndStructure))
        return 2;
    InitializeComponents();
    
    ShowWindow(*pMainWindowHandle, SW_SHOW);
    UpdateWindow(*pMainWindowHandle);
    
    InitThread();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        DefWindowProc(NULL, msg.message, msg.wParam, msg.lParam);
        
        
    }
    if (!TerminateThread())
        return 3;
    
    return 0;

}

