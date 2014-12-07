// myNotepad.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include "resource.h"
#include "Richedit.h"
#include "Commdlg.h"

HINSTANCE g_hInstance;
HWND      g_hEditWnd;
HWND      g_hWnd;
HANDLE    g_hFile;
HWND      g_hFind;     // handle to Find dialog box
HWND      g_hReplace;
bool      g_bReplace = false;
UINT      WM_FINDREPLACEMSG;
TCHAR     g_szFindWhat[256];//�����ַ�ȫ�ֱ���
TCHAR     g_szReplaceWith[256];//�����ַ�ȫ�ֱ���
FINDREPLACE g_fr = {0};// common dialog box structure
TCHAR     g_szLineNum[80];  

#define EM_SHOWSCROLLBAR		(WM_USER + 96)
bool OnSave(HWND hwnd);

//��ѯ�༭���Ƿ��޸Ĺ�
bool getEditState()
{
    if (SendMessage(g_hEditWnd, EM_GETMODIFY, 0, 0))
    {
        return true;
    }

    return false;
}

//�༭���޸ĺ��Ƿ񱻱���,false��ʾȡ������
bool  changeSave(HWND hwnd)
{
    //�༭���޸�
    if (getEditState())
    { 
        int nRet = MessageBox(g_hEditWnd,TEXT("�Ƿ񱣴����"),TEXT("���±�"),MB_YESNOCANCEL);
        
        if (nRet == IDYES)
        {
            return OnSave(hwnd);   
        }
        else if (nRet == IDCANCEL)
        {
            return false;
        }
    }

    return true;
}

void showError()
{
    LPVOID lpMsgBuf;
    FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL 
        );
    // Process any inserts in lpMsgBuf.
    // ...
    // Display the string.
    MessageBox( NULL, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK | MB_ICONINFORMATION );
    // Free the buffer.
    LocalFree( lpMsgBuf );
}

bool OnClose(HWND hwnd, UINT uMsg, WPARAM wParam,  LPARAM lParam)
{
    //ȡ������
    if (!changeSave(hwnd))
    {
        return false;
    }
    
    if (g_hFile != INVALID_HANDLE_VALUE && g_hFile != NULL)
    {
        CloseHandle(g_hFile);
        g_hFile = NULL;
    }

    return true;
}

bool OnDestroy(HWND hwnd, UINT uMsg, WPARAM wParam,  LPARAM lParam)
{
    
    PostMessage(hwnd,WM_QUIT,NULL,NULL);
    
    return false;
}

bool OnPaint(HWND hwnd, UINT uMsg, WPARAM wParam,  LPARAM lParam)
{
//     PAINTSTRUCT ps;
//     HDC hdc = BeginPaint(hwnd,&ps);
// 
//     RECT rc = {0};
//     
//     GetClientRect(hwnd,&rc);
// 
//     char pszStr[] = TEXT("Hello world!");
//     int nLen = _tcslen(pszStr);
//     
//     DrawText(hdc,pszStr,nLen,&rc,DT_CENTER | DT_SINGLELINE | DT_VCENTER);
// 
//     EndPaint(hwnd,&ps);
    return true;
}

//�Զ����в˵���Ӧ
bool OnMenu(HWND hwnd)
{
    HMENU hMenu = GetMenu(hwnd);
    UINT nState = GetMenuState(hMenu,IDM_EDAUTOLINE,MF_BYCOMMAND);


    
    if (LOWORD(nState) == MF_CHECKED)
    {
        //ȡ���Զ�����
        CheckMenuItem(hMenu,IDM_EDAUTOLINE,MF_UNCHECKED);
        SendMessage(g_hEditWnd, EM_SETTARGETDEVICE, 0, 1);
    }
    else 
    {
        CheckMenuItem(hMenu,IDM_EDAUTOLINE,MF_CHECKED);
        
        HDC hdc = GetDC(g_hEditWnd); 
        SendMessage(g_hEditWnd, EM_SETTARGETDEVICE,(WPARAM)hdc, 0 );
        ReleaseDC( g_hEditWnd, hdc );   
    }

    return true;
}

//��д�ļ��ص�����
DWORD CALLBACK EditStreamCallback(
  DWORD dwCookie, // application-defined value
  LPBYTE pbBuff,      // data buffer
  LONG cb,            // number of bytes to read or write
  LONG *pcb           // number of bytes transferred
)
{
    if (dwCookie)
    {
        ReadFile(g_hFile,pbBuff,cb,(unsigned long*)pcb,0);
    }
    else
    {
        WriteFile(g_hFile,pbBuff,cb,(unsigned long*)pcb,0);
    }
    
    return 0;
}


//���ļ��˵���Ӧ
bool OnOpen(HWND hwnd)
{
    //ȡ������
    if (!changeSave(hwnd))
    {
        return false;
    }
    
    if (g_hFile != INVALID_HANDLE_VALUE && g_hFile != NULL)
    {
        CloseHandle(g_hFile);
        g_hFile = NULL;
    }
    
    OPENFILENAME of = {0};
    TCHAR szFile[MAX_PATH] = {0};
    
    of.lStructSize = sizeof(OPENFILENAME);
    of.hwndOwner = g_hEditWnd;
    of.lpstrFilter = TEXT("�ı��ĵ�(*.txt)\0*.txt\0�����ļ�(*.*)\0*.*\0\0");
    of.lpstrCustomFilter  = TEXT("�ı��ĵ�(*.txt)\0*.txt\0\0");
    of.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST|OFN_EXPLORER;
    of.lpstrDefExt = TEXT("txt");
    of.nMaxFile = MAX_PATH;
    of.lpstrFile = szFile;
    BOOL bRet = GetOpenFileName(&of);

    if (bRet == 0)
    {
        return false;
    }

    if (g_hFile)
    {
        CloseHandle(g_hFile);
    }

    //��ָ���ļ�
    HANDLE hFile = CreateFile(szFile,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0);
    
    if (hFile == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL,TEXT("�޷���ָ���ļ�"),TEXT("��ʾ"),MB_OK);
        return false;
    }
    
    //EM_STREAMIN

    g_hFile = hFile;

    EDITSTREAM stEdit;
    stEdit.dwCookie = 1;//1��ʾ����0��ʾд
    stEdit.dwError = NULL;
    stEdit.pfnCallback = EditStreamCallback;

    SendMessage(g_hEditWnd,EM_STREAMIN,(WPARAM)SF_TEXT,(LPARAM)&stEdit);
    SendMessage(g_hEditWnd,EM_SETMODIFY,(WPARAM)TRUE,(LPARAM)0);
   
    return true;
}

//���Ϊ�ļ��˵���Ӧ
bool OnSaveAs(HWND hwnd)
{
    OPENFILENAME of = {0};
    TCHAR szFile[MAX_PATH] = {0};
    
    of.lStructSize = sizeof(OPENFILENAME);
    of.hwndOwner = g_hEditWnd;
    of.lpstrFilter = TEXT("�ı��ĵ�(*.txt)\0*.txt\0�����ļ�(*.*)\0*.*\0\0");
    of.lpstrCustomFilter  = TEXT("�ı��ĵ�(*.txt)\0*.txt\0\0");
    of.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST|OFN_EXPLORER;
    of.lpstrDefExt = TEXT("txt");
    of.nMaxFile = MAX_PATH;
    of.lpstrFile = szFile;
    BOOL bRet = GetSaveFileName(&of);

    if (bRet == 0)
    {
        return false;
    }

    if (g_hFile)
    {
        CloseHandle(g_hFile);
    }

    //��ָ���ļ�
    HANDLE hFile = CreateFile(szFile,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        0,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        0);
    
    if (hFile == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL,TEXT("�����ļ�����"),TEXT("��ʾ"),MB_OK);
        return false;
    }
    
    g_hFile = hFile;

    if (g_hFile != INVALID_HANDLE_VALUE && g_hFile != NULL)
    {
        DWORD dwRet = SetFilePointer(g_hFile,0,0,FILE_BEGIN);
        SetEndOfFile(g_hFile);
        EDITSTREAM stEdit;
        stEdit.dwCookie = 0;//1��ʾ����0��ʾд
        stEdit.dwError = NULL;
        stEdit.pfnCallback = EditStreamCallback;
        
        SendMessage(g_hEditWnd,EM_STREAMOUT,(WPARAM)SF_TEXT,(LPARAM)&stEdit);
        SendMessage(g_hEditWnd,EM_SETMODIFY,(WPARAM)FALSE,(LPARAM)0);
    }

    return true;
}

//�����ļ��˵���Ӧ
bool OnSave(HWND hwnd)
{
    if (g_hFile != INVALID_HANDLE_VALUE && g_hFile != NULL)
    {
        DWORD dwRet = SetFilePointer(g_hFile,0,0,FILE_BEGIN);
        SetEndOfFile(g_hFile);
        EDITSTREAM stEdit;
        stEdit.dwCookie = 0;//1��ʾ����0��ʾд
        stEdit.dwError = NULL;
        stEdit.pfnCallback = EditStreamCallback;
        
        SendMessage(g_hEditWnd,EM_STREAMOUT,(WPARAM)SF_TEXT,(LPARAM)&stEdit);
        SendMessage(g_hEditWnd,EM_SETMODIFY,(WPARAM)FALSE,(LPARAM)0);
    }
    else
    {
        return OnSaveAs(hwnd);
    }

    return true;
}


//�½��ļ��˵���Ӧ
bool OnNewFile(HWND hwnd)
{

    //ȡ������
    if (!changeSave(hwnd))
    {
        return false;
    }
    
    if (g_hFile != INVALID_HANDLE_VALUE && g_hFile != NULL)
    {
        CloseHandle(g_hFile);
        g_hFile = NULL;
    }
    
    //�жϿؼ��������Ƿ�Ϊ��
    
    int nRet = GetWindowTextLength(g_hEditWnd);
    if ( nRet != 0)
    {
        SendMessage(g_hEditWnd, EM_SETSEL, 0, -1);
        SendMessage(g_hEditWnd, WM_CLEAR, 0, 0); 
    }
      
    return true;
}

//���ò��ҶԻ���
bool OnFind(HWND hwnd)
{
    // Initialize FINDREPLACE
    g_hFind = FindText(&g_fr);
       
    return true;
}

//�����滻�Ի���
bool OnReplace(HWND hwnd)
{
    // Initialize FINDREPLACE
    g_hReplace = ReplaceText(&g_fr);
       
    return true;
}

//������һ��
bool OnFindNext(HWND hwnd)
{
    if (g_hFind != NULL && _tcsclen(g_fr.lpstrFindWhat) != 0)
    {
        SendMessage(hwnd,WM_FINDREPLACEMSG,0,(LPARAM)&g_fr);
    }
    else if(g_hFind == NULL)
    {
        OnFind(hwnd);
    }
  
    return true;
}

//�Ի���ص�����
INT_PTR CALLBACK DialogProc(
                            HWND hwndDlg,  // handle to dialog box
                            UINT uMsg,     // message
                            WPARAM wParam, // first message parameter
                            LPARAM lParam  // second message parameter
                            )
{
    int nNum = -1;
    int nRet = 0;
    switch (uMsg) 
    {
    case WM_INITDIALOG:
        {
            SetDlgItemInt(hwndDlg, IDE_LINENUM,SendMessage(g_hEditWnd,EM_GETLINECOUNT,0,0),FALSE);
            return TRUE;
        }
    case WM_CLOSE:
        {
            EndDialog(hwndDlg, 0);
            return TRUE;
        }
        break;
    case WM_COMMAND: 
        WORD wNotiCode = HIWORD(wParam); 
        WORD wID = LOWORD(wParam);
        HWND hWnd = (HWND)lParam;
        switch (LOWORD(wID)) 
        { 
        case IDOK: 
            {
                nNum = GetDlgItemInt(hwndDlg, IDE_LINENUM,&nRet,FALSE);       
                if (!nRet || nNum < 1)
                {
                    EndDialog(hwndDlg, wParam);
                    //SetFocus(g_hWnd);
                    return false;
                }
                DWORD nCount = SendMessage(g_hEditWnd,EM_GETLINECOUNT,0,0);
                
                if ((int)nCount >= nNum)
                {
                    int nLine = SendMessage(g_hEditWnd,EM_LINEINDEX,nNum-1,0);
                    SendMessage(g_hEditWnd,EM_SETSEL,nLine,nLine);
                }
            }
            // Fall through. 
            
        case IDCANCEL: 
            EndDialog(hwndDlg, wParam);
            //SetFocus(g_hWnd);
            return true; 
        } 
        
    }
    return false;
}


//��ת��ָ����
bool OnGoLine(HWND hwnd)
{
    DialogBoxParam(g_hInstance,MAKEINTRESOURCE(IDD_GOTOLINE),g_hEditWnd,(DLGPROC)DialogProc,(LPARAM)WM_INITDIALOG);
  
    return true;
}

//��������
bool OnDate(HWND hwnd)
{
    TCHAR szBuf[256] = {0};

    SYSTEMTIME st;
    GetLocalTime(&st);
    TCHAR szDayOfWeek[7][4] = 
    {
        TEXT("������"),
        TEXT("����һ"),
        TEXT("���ڶ�"),
        TEXT("������"),
        TEXT("������"),
        TEXT("������"),
        TEXT("������")
    };

    wsprintf(szBuf,TEXT("%2d:%02d %4d/%02d/%02d %s"),
        st.wHour,
        st.wMinute,
        st.wYear,
        st.wMonth,
        st.wDay,
        szDayOfWeek[st.wDayOfWeek]);
    SendMessage(g_hEditWnd,EM_REPLACESEL,(WPARAM)TRUE,(LPARAM)szBuf);
  
    return true;
}

//command��Ϣ��Ӧ
bool OnCommand(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WORD wNotifyCode = HIWORD(wParam);
    WORD wID = LOWORD(wParam);
      
    switch (wID) 
    { 
    case IDM_EDUNDO: 
        
        // Send WM_UNDO only if there is something 
        // to be undone. 
        
        if (SendMessage(g_hEditWnd, EM_CANUNDO, 0, 0)) 
            SendMessage(g_hEditWnd, WM_UNDO, 0, 0); 
        else 
        {
            MessageBox(g_hEditWnd, 
                TEXT("�޷�����"), 
                TEXT("��ʾ"), MB_OK); 
        }
        break; 
        
    case IDM_EDCUT: 
        SendMessage(g_hEditWnd, WM_CUT, 0, 0); 
        break; 
        
    case IDM_EDCOPY: 
        SendMessage(g_hEditWnd, WM_COPY, 0, 0); 
        break; 
        
    case IDM_EDPASTE: 
        SendMessage(g_hEditWnd, WM_PASTE, 0, 0); 
        break; 
        
    case IDM_EDDEL: 
        SendMessage(g_hEditWnd, WM_CLEAR, 0, 0); 
        break; 
        
    case IDM_EDSETALL: 
        SendMessage(g_hEditWnd, EM_SETSEL, 0, -1); 
        break; 

    case IDM_EDAUTOLINE: 
        OnMenu(hwnd);     
        break;

    case IDM_EDOPEN:
        OnOpen(hwnd);
        break;

    case IDM_EDSAVE:
        OnSave(hwnd);
        break;

    case IDM_EDOTHERSAVE:
        OnSaveAs(hwnd);
        break;
    
    case IDM_EDNEWFILE:
        OnNewFile(hwnd);
        break;

    case IDM_EDEXIT:
        SendMessage(hwnd, WM_CLOSE, 0, 0);
        break;

    case IDM_EDFIND:
        OnFind(hwnd);
        break;

    case IDM_EDFINDNEXT:
        OnFindNext(hwnd);
        break;

    case IDM_EDREPLACE:
        OnReplace(hwnd);
        break;

    case IDM_EDGOLINE:
        OnGoLine(hwnd);
        break;

    case IDM_EDDATE:
        OnDate(hwnd);
//         
//     case IDM_WRAP: 
//         SendMessage(g_hEditWnd, 
//             EM_SETWORDBREAKPROC, 
//             (WPARAM) 0, 
//             (LPARAM) (EDITWORDBREAKPROC) WordBreakProc); 
//         SendMessage(g_hEditWnd, 
//             EM_FMTLINES, 
//             (WPARAM) TRUE, 
//             (LPARAM) 0); 
//         SendMessage(g_hEditWnd, 
//             EM_SETSEL, 
//             0, -1); // select all text 
//         SendMessage(g_hEditWnd, WM_CUT, 0, 0); 
//         SendMessage(g_hEditWnd, WM_PASTE, 0, 0); 
//         break; 
//         
//     case IDM_ABOUT: 
//         DialogBox(hinst, // current instance 
//             "AboutBox",  // resource to use 
//             hwnd,       // parent handle 
//             (DLGPROC) About); 
//         break; 
        
    default: 
        break; 
    } 

    return false;
}

//size��Ϣ��Ӧ
bool OnSize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    MoveWindow(g_hEditWnd, 
        0, 0,           // starting x- and y-coordinates 
        LOWORD(lParam), // width of client area 
        HIWORD(lParam), // height of client area 
        TRUE);   
    //SendMessage(g_hEditWnd, EM_SHOWSCROLLBAR, SB_HORZ, TRUE);
    SendMessage(g_hEditWnd, EM_SHOWSCROLLBAR, SB_VERT, TRUE);

    return true;
}

/*

*/
bool SearchFile(FINDREPLACE* lpFr)
{
    TCHAR szText[256] = {0};
    FINDTEXTEX findText;
    findText.lpstrText =  lpFr->lpstrFindWhat;
    
    SendMessage(g_hEditWnd,EM_EXGETSEL,0,(LPARAM)&findText.chrg);

    //���û��ѡ��
    if (findText.chrg.cpMin == findText.chrg.cpMax)
    {
        //findText.chrg.cpMin = 0;
        findText.chrg.cpMax = -1;
    }
    else
    {
        if (lpFr->Flags & FR_DOWN)
        {
           findText.chrg.cpMin = findText.chrg.cpMax;
        }
        findText.chrg.cpMax = -1;    
    }
    
    DWORD flags = 0;

    if (lpFr->Flags & FR_MATCHCASE)
    {
        flags |= FR_MATCHCASE;
    }
    
    if (lpFr->Flags & FR_DOWN)
    {
        flags |= FR_DOWN;
        //findText.chrg.cpMax = findText.chrg.cpMin;
    }

//     if (lpFr->Flags & FR_WHOLEWORD)
//     {
//         flags |= FR_WHOLEWORD;
//     }

    //flags = flags &(FR_MATCHCASE | FR_DOWN | FR_WHOLEWORD);
    long bRet = SendMessage(g_hEditWnd,EM_FINDTEXTEX,(WPARAM)flags,(LPARAM)&findText);

    wsprintf(szText,TEXT("%s\"%s\""),TEXT("�Ҳ���"),findText.lpstrText);
    
    if (bRet == -1)
    {
        if (g_hFind != NULL)
        {
             ::MessageBox(g_hFind,szText,TEXT("��ʾ"),MB_ICONINFORMATION | MB_OK);
        }
        if (g_hReplace != NULL)
        {
             ::MessageBox(g_hReplace,szText,TEXT("��ʾ"),MB_ICONINFORMATION | MB_OK);
        }
        return false;
    }
    
    SendMessage(g_hEditWnd,EM_EXSETSEL,0,(LPARAM)&findText.chrgText);
    SendMessage(g_hEditWnd,EM_SCROLLCARET,0,0);
    
    
    return true;
}

//�滻
bool ReplaceFile(FINDREPLACE* lpFr)
{
   
    TCHAR szText[256] = {0};
    FINDTEXTEX findText;
    findText.lpstrText =  lpFr->lpstrFindWhat;

    if ( (lpFr->Flags & FR_REPLACE) &&  g_bReplace )
    {
        SendMessage(g_hEditWnd,EM_REPLACESEL,(WPARAM)TRUE,(LPARAM)g_fr.lpstrReplaceWith);
        g_bReplace = false;
    }
 
    if ((lpFr->Flags & FR_REPLACE) && !g_bReplace)
    {
        SendMessage(g_hEditWnd,EM_EXGETSEL,0,(LPARAM)&findText.chrg);
        
        //���û��ѡ��
        if (findText.chrg.cpMin == findText.chrg.cpMax)
        {
            //findText.chrg.cpMin = 0;
            findText.chrg.cpMax = -1;
        }
        else
        {
            if (lpFr->Flags & FR_DOWN)
            {
                findText.chrg.cpMin = findText.chrg.cpMax;
            }
            findText.chrg.cpMax = -1;    
        }
        
        DWORD flags = 0;
        
        if (lpFr->Flags & FR_MATCHCASE)
        {
            flags |= FR_MATCHCASE;
        }
        
        if (lpFr->Flags & FR_DOWN)
        {
            flags |= FR_DOWN;
            //findText.chrg.cpMax = findText.chrg.cpMin;
        }
        
        //     if (lpFr->Flags & FR_WHOLEWORD)
        //     {
        //         flags |= FR_WHOLEWORD;
        //     }
        
        //flags = flags &(FR_MATCHCASE | FR_DOWN | FR_WHOLEWORD);
        long bRet = SendMessage(g_hEditWnd,EM_FINDTEXTEX,(WPARAM)flags,(LPARAM)&findText);
        
        wsprintf(szText,TEXT("%s\"%s\""),TEXT("�Ҳ���"),findText.lpstrText);
        
        if (bRet == -1)
        {
            if (g_hReplace != NULL)
            {
                ::MessageBox(g_hReplace,szText,TEXT("��ʾ"),MB_ICONINFORMATION | MB_OK);
            }
            if (lpFr->Flags & FR_REPLACE)
            {
                g_bReplace = false;
            }
            return false;
        }
        
        SendMessage(g_hEditWnd,EM_EXSETSEL,0,(LPARAM)&findText.chrgText);
        SendMessage(g_hEditWnd,EM_SCROLLCARET,0,0);

        if (lpFr->Flags & FR_REPLACE)
        {
            g_bReplace = true;
        }
        
        return true;
    }
        
    return true;
}

//�滻ȫ��
bool ReplaceALLFile(FINDREPLACE* lpFr)
{
   
    TCHAR szText[256] = {0};
    FINDTEXTEX findText;
    findText.lpstrText =  lpFr->lpstrFindWhat;
    DWORD flags = 0;
    findText.chrg.cpMin = 0;
    findText.chrg.cpMax = -1;
    
    if (lpFr->Flags & FR_MATCHCASE)
    {
        flags |= FR_MATCHCASE;
    }
    
    if (lpFr->Flags & FR_DOWN)
    {
        flags |= FR_DOWN;
        //findText.chrg.cpMax = findText.chrg.cpMin;
    }

    while (true)
    {

        long bRet = SendMessage(g_hEditWnd,EM_FINDTEXTEX,(WPARAM)flags,(LPARAM)&findText);
        
        if (bRet == -1)
        {
            return false;
        }
        
        SendMessage(g_hEditWnd,EM_EXSETSEL,0,(LPARAM)&findText.chrgText);
        //SendMessage(g_hEditWnd,EM_SCROLLCARET,0,0);
        
        SendMessage(g_hEditWnd,EM_REPLACESEL,(WPARAM)TRUE,(LPARAM)g_fr.lpstrReplaceWith);

        SendMessage(g_hEditWnd,EM_EXGETSEL,0,(LPARAM)&findText.chrg);
        
        //���û��ѡ��
        if (findText.chrg.cpMin == findText.chrg.cpMax)
        {
            findText.chrg.cpMax = -1;
        }
        else
        {
            if (lpFr->Flags & FR_DOWN)
            {
                findText.chrg.cpMin = findText.chrg.cpMax;
            }
            findText.chrg.cpMax = -1;    
        }        
    }
    
    return true;
}

//�ı�menu״̬��Ϣ��Ӧ
bool OnInitmenu(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HMENU hMenu = (HMENU)wParam;
    CHARRANGE cRange;
    SendMessage( 
        g_hEditWnd,              // handle to destination window 
        EM_EXGETSEL,              // message to send
                  0,          // not used; must be zero
        (LPARAM)&cRange           // selection range (CHARRANGE *)
        );

    if (cRange.cpMin != cRange.cpMax)
    {
        EnableMenuItem(hMenu,IDM_EDCUT,MF_ENABLED);
        EnableMenuItem(hMenu,IDM_EDCOPY,MF_ENABLED);    
        EnableMenuItem(hMenu,IDM_EDDEL,MF_ENABLED);
    }
    else
    {
        EnableMenuItem(hMenu,IDM_EDCUT,MF_GRAYED);
        EnableMenuItem(hMenu,IDM_EDCOPY,MF_GRAYED);
        EnableMenuItem(hMenu,IDM_EDDEL,MF_GRAYED);
    }

    if (SendMessage(g_hEditWnd,EM_CANPASTE,0,0))
    {
        EnableMenuItem(hMenu,IDM_EDPASTE,MF_ENABLED);
    }
    else
    {
        EnableMenuItem(hMenu,IDM_EDPASTE,MF_GRAYED);
    }

    if (SendMessage(g_hEditWnd,EM_CANUNDO,0,0))
    {
        EnableMenuItem(hMenu,IDM_EDUNDO,MF_ENABLED);
    }
    else
    {
        EnableMenuItem(hMenu,IDM_EDUNDO,MF_GRAYED);
    }
    
    if (GetWindowTextLength(g_hEditWnd))
    {
        EnableMenuItem(hMenu,IDM_EDSETALL,MF_ENABLED);
        EnableMenuItem(hMenu,IDM_EDFIND,MF_ENABLED);
        EnableMenuItem(hMenu,IDM_EDREPLACE,MF_ENABLED);
        EnableMenuItem(hMenu,IDM_EDFINDNEXT,MF_ENABLED);
    }
    else
    {
        EnableMenuItem(hMenu,IDM_EDSETALL,MF_GRAYED);
        EnableMenuItem(hMenu,IDM_EDFIND,MF_GRAYED);
        EnableMenuItem(hMenu,IDM_EDREPLACE,MF_GRAYED);
        EnableMenuItem(hMenu,IDM_EDFINDNEXT,MF_GRAYED);
    }


     
    return true;
}

//create��Ϣ��Ӧ
bool OnCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    
    WM_FINDREPLACEMSG = RegisterWindowMessage(FINDMSGSTRING);
    g_fr.lStructSize = sizeof(FINDREPLACE);
    g_fr.hwndOwner = hwnd;
    g_fr.lpstrFindWhat = g_szFindWhat;
    g_fr.lpstrReplaceWith = g_szReplaceWith;
    g_fr.wFindWhatLen = 256;
    g_fr.wReplaceWithLen = 256;
    g_fr.Flags = 0;
    //g_fr.Flags &= ~FR_DIALOGTERM;
    
    g_hEditWnd = CreateWindow(
        TEXT("RichEdit20W"),//RichEdit�ؼ�����
        NULL,//TEXT("���±�"),//���ڱ���
        WS_CHILD | WS_VISIBLE | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
        | ES_MULTILINE | ES_NOHIDESEL
        | ES_WANTRETURN,//������ʽ
        0,//ˮƽƫ��
        0,//��ֱƫ��
        0,//���ڿ��
        0,//���ڳ���
        hwnd,//�����ھ��
        NULL,//�˵����
        g_hInstance,//ʵ�����
        NULL);
    if (g_hEditWnd == NULL)
    {
        showError();
        exit(-1);       
    }
    
    //SendMessage(g_hEditWnd, WM_SETTEXT, 0, (LPARAM) TEXT("��ӭʹ���ҵļ��±�!"));
    SendMessage(g_hEditWnd, EM_SHOWSCROLLBAR, SB_HORZ, TRUE);
    SendMessage(g_hEditWnd, EM_SHOWSCROLLBAR, SB_VERT, TRUE);
    
    CHARFORMAT cf;
    cf.cbSize = sizeof(CHARFORMAT);   
    cf.dwMask = CFM_FACE | CFM_SIZE | CFM_ITALIC | CFM_BOLD;
    cf.dwEffects = 0; 
    cf.yHeight = 12*20;
    cf.crTextColor = RGB(0,0,0);
    _tcscpy((TCHAR *)cf.szFaceName,TEXT("����"));
    cf.bCharSet = 0;
    cf.bPitchAndFamily=0;
    SendMessage(g_hEditWnd,EM_SETCHARFORMAT,SCF_ALL,(LPARAM)&cf);
    
    
    return false;
}

LRESULT CALLBACK WindowProc(
  HWND hwnd,      // handle to window
  UINT uMsg,      // message identifier
  WPARAM wParam,  // first message parameter
  LPARAM lParam   // second message parameter
)
{

    switch (uMsg)
    {
    case WM_CREATE:
        OnCreate(hwnd,uMsg,wParam,lParam);
        return 0;
        
    case WM_SIZE:
        OnSize(hwnd,uMsg,wParam,lParam);
        return 0;
        
    case WM_CLOSE:
        if (OnClose(hwnd,uMsg,wParam,lParam))
        {
            DestroyWindow(hwnd);
            //PostMessage(hwnd,WM_QUIT,,NULL);
            //PostQuitMessage(-1);
        }        
        return 0;
    case WM_INITMENU:
        OnInitmenu(hwnd,uMsg,wParam,lParam);
        break;
    case WM_DESTROY:
        OnDestroy(hwnd,uMsg,wParam,lParam);
        break;

    case WM_COMMAND:
        OnCommand(hwnd,uMsg,wParam,lParam);
        break;
        
    case WM_PAINT:
        //OutputDebugString(TEXT("WM_PAINT"));
        OnPaint(hwnd,uMsg,wParam,lParam);
        break;
        
    case WM_SETFOCUS: 
        SetFocus(g_hEditWnd); 
        return 0;
        
    default:
        break;
    }

    if (uMsg == WM_FINDREPLACEMSG)
    {
        LPFINDREPLACE lpfr;
        lpfr = (LPFINDREPLACE)lParam;

        if (lpfr->Flags & FR_DIALOGTERM){ 
            g_hFind  = NULL; 
            g_hReplace = NULL;
            return 0; 
        } 
        else
        {
            if (lpfr->Flags & FR_FINDNEXT)
            {
                SearchFile(lpfr); 
            }
            else if (lpfr->Flags & FR_REPLACE)
            {
                ReplaceFile(lpfr);
            }
            else if (lpfr->Flags & FR_REPLACEALL)
            {
                ReplaceALLFile(lpfr);
            }
        }
        
        return 0; 
          
    }

     return DefWindowProc(hwnd,uMsg,wParam,lParam);  
}


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.

    g_hInstance = hInstance;

    HMODULE  hMoudle = LoadLibrary(TEXT("riched20.dll"));
    
    if (hMoudle == NULL)
    {
        showError();
        exit(-1);
    }

    //ע���ݼ�
    HACCEL  hAccel = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDA_ACC));
    HICON hIcon =  LoadIcon(hInstance,MAKEINTRESOURCE(IDI_NOTEPAD));

    //���ڻ�������
    //1��ע�ᴰ����
    WNDCLASS wc = {0};
    wc.style = CS_VREDRAW | CS_HREDRAW;//ˮƽ����ֱ�ػ�
    wc.lpfnWndProc = WindowProc;//��Ϣ�ص�����������ָ��
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;//ʵ�����
    wc.hIcon = hIcon;
    wc.hCursor = NULL;
    wc.lpszMenuName = MAKEINTRESOURCE(1);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOWFRAME;
    wc.lpszClassName = TEXT("FirstWindow");
    
    if (RegisterClass(&wc) == 0)
    {
        showError();
        exit(-1);
    }
    
    //2����������
    HWND hWnd = CreateWindow(TEXT("FirstWindow"),
                             TEXT("���±�"),//���ڱ���
                             WS_OVERLAPPEDWINDOW ,//������ʽ
                             CW_USEDEFAULT,//ˮƽƫ��
                             CW_USEDEFAULT,//��ֱƫ��
                             CW_USEDEFAULT,//���ڿ��
                             CW_USEDEFAULT,//���ڳ���
                             NULL,//�����ھ��
                             NULL,//�˵����
                             hInstance,//ʵ�����
                             NULL);

    if (hWnd == NULL)
    {
        showError();
        exit(-1);       
    }
    
    g_hWnd = hWnd;
    //3����ʾ��ˢ�´���
    ShowWindow(hWnd,SW_SHOWDEFAULT);
//     if (!ShowWindow(hWnd,SW_SHOWDEFAULT))
//     {
//         showError();
//         //exit(-1);     
//     }
    //UpdateWindow(hWnd);

    if (!UpdateWindow(hWnd))
    {
        showError();
        exit(-1);     
    }

    //4����Ϣѭ��
    
    BOOL bRet;
    MSG msg;

    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        if (bRet == -1)
        {
            // handle the error and possibly exit
            //showError();
            //exit(-1);  
        }
        else
        {
            if (!TranslateAccelerator(hWnd,hAccel,&msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg); 
            }
        }
    }

	return 0;
}