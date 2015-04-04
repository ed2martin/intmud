/*
Para compilar no MinGW:
windres editmud-resource.rc editmud-resource.o
gcc -Wall -c editmud.c
gcc -o editmud.exe editmud.o editmud-resource.o -mms-bitfields -mwindows -lws2_32 -lwinmm
strip --strip-all editmud.exe

Para compilar no Linux:
Instalar mingw32 mingw32-binutils mingw32-runtime
i586-mingw32msvc-windres editmud-resource.rc editmud-resource.o
i586-mingw32msvc-gcc -Wall -c editmud.c
i586-mingw32msvc-gcc -o editmud.exe editmud.o editmud-resource.o -mms-bitfields -mwindows -lws2_32 -lwinmm
i586-mingw32msvc-strip --strip-all editmud.exe

Controles:  http://msdn.microsoft.com/en-us/library/ms632679(VS.85).aspx
Listbox:    http://msdn.microsoft.com/en-us/library/ms997541.aspx
Edit:       http://msdn.microsoft.com/en-us/library/bb775458(VS.85).aspx

Dialogex:   http://msdn.microsoft.com/en-us/library/aa381002(VS.85).aspx
Checkbox:   http://msdn.microsoft.com/en-us/library/aa380880(VS.85).aspx
*/

//---------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <windows.h>
#include <windowsx.h>
#include <winsock.h>
#include "editmud-resource.h"

const char g_szClassName[] = "myWindowClass";
HMENU hMenu;
HWND hEdit, hList, hLinha, hLabel, hPrograma;
HINSTANCE g_hinst;
WNDPROC oldProc = NULL;

int lista_ind=-1;   // Índice do arquivo selecionado
int lista_m=0;      // Se o texto mudou

char dlg_classe[80]="";         // DialogBox: Nome da classe
int  dlg_chk=0;                 // DialogBox: Checkbox
char dlg_serv[40]="localhost";  // DialogBox: Endereço do servidor
int  dlg_porta=2000;            // DialogBox: Porta
char dlg_senha[40]="modo";      // DialogBox: Senha

//---------------------------------------------------------------------------
SOCKET sock_manip; // Socket, para comunicação
char sock_modo; // 0=não está processando, 1=jogar no log, 2=ler
char * sock_env; // Texto para enviar
char * sock_rec; // Texto para receber
char * sock_penv=0; // Ponteiro para envio
char * sock_prec=0; // Ponteiro para receber

int Conectar(int destino); // Inicia conexão com o servidor
void ProcConec(); // Processa comunicação com o servidor

//---------------------------------------------------------------------------
// Semelhante a strcpy(), mas retorna endereço do byte =0 em destino
// tamanho = número de caracteres que pode escrever em destino
char * copiastr(char * destino, const char * origem, int tamanho)
{
    if (*origem==0 || tamanho<=1)
    {
        if (tamanho>=1)
            *destino=0;
        return destino;
    }
    *(destino++) = *(origem++);
    tamanho-=2;  // -2 por causa do zero no final
    for (; tamanho && *origem; tamanho--)
        *(destino++) = *(origem++);
    *destino=0;
    return destino;
}

//---------------------------------------------------------------------------
// Habilita / desabilita controles
void Habilitar(int enable)
{
    EnableMenuItem(hMenu, 9001, enable && lista_ind!=1 ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, 9002, enable && lista_ind>=2 ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, 9003, enable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, 9004, enable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, 9005, enable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, 9006, enable && lista_ind>=2 ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, 9007, enable ? MF_ENABLED : MF_GRAYED);
    Edit_Enable(hEdit, enable);
    ListBox_Enable(hList, enable);
    DrawMenuBar(hPrograma);
}

//---------------------------------------------------------------------------
void StatusBar(const char * texto)
{
    SendMessage(hLabel, WM_SETTEXT, 0, (LPARAM)TEXT(texto));
}

//---------------------------------------------------------------------------
void Edit_NomeClasse()
{
    char mens[65100];
    char * p = mens;
    int linha = Edit_LineFromChar(hEdit, -1);
    Edit_GetText(hEdit, mens, sizeof(mens)-2);
    *dlg_classe = 0;
    for (; linha>0; linha--,p++)
    {
        while (*p && *p!='\n')
            p++;
        if (*p==0)
            return;
    }
    while (*p && *(unsigned char*)p < ' ')
        p++;
    copiastr(dlg_classe, p, sizeof(dlg_classe));
    for (p=dlg_classe; *(unsigned char*)p >= ' '; p++);
    *p=0;
}

//---------------------------------------------------------------------------
BOOL CALLBACK ApagarDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        SetDlgItemText(hwnd, IDC_APAGARTXT, dlg_classe);
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            GetDlgItemText(hwnd, IDC_APAGARTXT, dlg_classe, sizeof(dlg_classe));
            EndDialog(hwnd, IDOK);
            break;
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;
        }
        return TRUE;
    }
    return FALSE;
}

//---------------------------------------------------------------------------
BOOL CALLBACK ArqNovoDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        Button_SetCheck(GetDlgItem(hwnd, IDC_CHKNOVO), BST_CHECKED);
        SetDlgItemText(hwnd, IDC_ARQNOVOTXT, dlg_classe);
        dlg_chk=1;
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            GetDlgItemText(hwnd, IDC_ARQNOVOTXT, dlg_classe, sizeof(dlg_classe));
            EndDialog(hwnd, IDOK);
            break;
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;
        case IDC_CHKNOVO:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                dlg_chk = !dlg_chk;
                Button_SetCheck(GetDlgItem(hwnd, IDC_CHKNOVO),
                        dlg_chk ? BST_CHECKED : BST_UNCHECKED);
            }
            break;
        }
        return TRUE;
    }
    return FALSE;
}

//---------------------------------------------------------------------------
BOOL CALLBACK ConfigDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char txtporta[10];
    switch (msg)
    {
    case WM_INITDIALOG:
        sprintf(txtporta, "%d", dlg_porta);
        SetDlgItemText(hwnd, IDC_ENDER, dlg_serv);
        SetDlgItemText(hwnd, IDC_PORTA, txtporta);
        SetDlgItemText(hwnd, IDC_SENHA, dlg_senha);
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            GetDlgItemText(hwnd, IDC_ENDER, dlg_serv, sizeof(dlg_serv));
            GetDlgItemText(hwnd, IDC_PORTA, txtporta, sizeof(txtporta));
            GetDlgItemText(hwnd, IDC_SENHA, dlg_senha, sizeof(dlg_senha));
            dlg_porta = atoi(txtporta);
            EndDialog(hwnd, IDOK);
            break;
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;
        }
        return TRUE;
    }
    return FALSE;
}

//---------------------------------------------------------------------------
// Lê/salva texto correspondente à lista
// Índice = índice na lista (ListBox_GetCurSel(hList))
// ler: 0=ler 1=salvar
void lista_lermudar(int ind, int ler)
{
    char mens[65100];
    FILE * arq;
    if (ind<0)
    {
        if (ler)
            Edit_SetText(hEdit, "");
        return;
    }
    if (ind==0)
        strcpy(mens, "mudclasse.txt");
    else if (ind==1)
        strcpy(mens, "mudlog.txt");
    else
    {
        strcpy(mens, "mud_");
        ListBox_GetText(hList, ind, mens+4);
        strcat(mens, ".txt");
    }
    arq = fopen(mens, ler ? "r" : "w");
    if (arq==0)
    {
        if (ler)
            Edit_SetText(hEdit, "");
        return;
    }
    if (ler)
    {
        ind = fread(mens, 1, sizeof(mens)-1, arq);
        mens[ind]=0;
        Edit_SetText(hEdit, mens);
        //puts("Leu"); fflush(stdout);
    }
    else
    {
        Edit_GetText(hEdit, mens, sizeof(mens)-2);
        fwrite(mens, 1, strlen(mens), arq);
        //puts("Salvou"); fflush(stdout);
    }
    fclose(arq);
}

//---------------------------------------------------------------------------
void txt_mudou()
{
    if (lista_m==0)
        lista_m = 70;
}

//---------------------------------------------------------------------------
void list_mudou()
{
    if (lista_m)
        lista_lermudar(lista_ind, 0); // Salva texto
    lista_ind = ListBox_GetCurSel(hList); // Atualiza índice
    lista_lermudar(lista_ind, 1); // Lê texto
    lista_m = 0;
    EnableMenuItem(hMenu, 9001, lista_ind!=1 ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, 9002, lista_ind>=2 ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, 9006, lista_ind>=2 ? MF_ENABLED : MF_GRAYED);
    DrawMenuBar(hPrograma);
}

//---------------------------------------------------------------------------
void menu_ler(HWND hwnd)
{
    if (lista_ind >= 2)
    {
        ListBox_GetText(hList, lista_ind, dlg_classe);
        sprintf(sock_env, "%s ler %s\n", dlg_senha, dlg_classe);
    }
    else
        sprintf(sock_env, "%s lista\n", dlg_senha);
    Conectar(1);
}

//---------------------------------------------------------------------------
void menu_enviar(HWND hwnd)
{
    char * p = sock_env;
    ListBox_GetText(hList, lista_ind, dlg_classe);
    sprintf(sock_env, "%s mudar %s\n", dlg_senha, dlg_classe);
    while (*p)
        p++;
    Edit_GetText(hEdit, p, 65000);
    strcat(p, "\n---\n");
    Conectar(0);
}

//---------------------------------------------------------------------------
void menu_apagar(HWND hwnd)
{
    int ret;
    if (lista_ind >= 2)
        ListBox_GetText(hList, lista_ind, dlg_classe);
    else
        Edit_NomeClasse();
    ret = DialogBox(GetModuleHandle(NULL),
            MAKEINTRESOURCE(IDD_APAGAR), hwnd, ApagarDlgProc);
    if (ret != IDOK)
    {
        if (ret == -1)
            MessageBox(hwnd, "Dialog failed!", "Error",
                        MB_OK | MB_ICONINFORMATION);
        return;
    }
    sprintf(sock_env, "%s apagar %s\n", dlg_senha, dlg_classe);
    Conectar(0);
}

//---------------------------------------------------------------------------
void menu_salvar(HWND hwnd)
{
    sprintf(sock_env, "%s salvar\n", dlg_senha);
    Conectar(0);
}

//---------------------------------------------------------------------------
void menu_arqcriar(HWND hwnd)
{
    int ret;
    char * p;
    Edit_NomeClasse();
    ret = DialogBox(GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDD_ARQNOVO), hwnd, ArqNovoDlgProc);
    if (ret != IDOK)
    {
        if (ret == -1)
            MessageBox(hwnd, "Dialog failed!", "Error",
                        MB_OK | MB_ICONINFORMATION);
        return;
    }
    for (p=dlg_classe; *p; p++)
        if (*p=='\\' || *p=='/' || *p==':' || *p=='?' || *p=='*')
        {
            MessageBox(hwnd, "Nome de classe inválido", "Erro",
                        MB_OK | MB_ICONINFORMATION);
            return;
        }
    ListBox_AddItemData(hList, dlg_classe);
    ListBox_SetCurSel(hList, ListBox_GetCount(hList)-1);
    list_mudou();
    if (dlg_chk)
        menu_ler(hwnd);
}

//---------------------------------------------------------------------------
void menu_arqapagar(HWND hwnd)
{
    char mens[512];
    int  index = lista_ind;
// Verifica se deve apagar
    if (MessageBox(hwnd, "Apagar arquivo?", "Apagar arquivo",
        MB_YESNO|MB_APPLMODAL)!=IDYES)
        return;
// Obtém texto do item
    strcpy(mens, "mud_");
    ListBox_GetText(hList, lista_ind, mens+4);
// Vai para o primeiro item
    ListBox_SetCurSel(hList, 0);
    list_mudou();
// Apaga item da lista
    ListBox_DeleteString(hList, index);
// Verifica se tem outra listbox com o mesmo nome
    if (ListBox_FindStringExact(hList,-1,mens)!=LB_ERR)
        return;
// Apaga arquivo
    strcat(mens, ".txt");
    remove(mens);
}

//---------------------------------------------------------------------------
void menu_config(HWND hwnd)
{
    int ret = DialogBox(GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDD_CONFIG), hwnd, ConfigDlgProc);
    if (ret != IDOK)
    {
        if (ret == -1)
            MessageBox(hwnd, "Dialog failed!", "Error",
                        MB_OK | MB_ICONINFORMATION);
        return;
    }
    FILE * arq = fopen("editmud.cfg", "w");
    if (arq==0)
    {
        MessageBox(hwnd, "Não consegui salvar arquivo de configuração",
                    "Erro", MB_OK | MB_ICONINFORMATION);
        return;
    }
    fprintf(arq, "%s\n%d\n%s\n", dlg_serv, dlg_porta, dlg_senha);
    fclose(arq);
}

//---------------------------------------------------------------------------
void timer_exec()
{
// Atualiza número da linha na tela
    static int atual=-1;
    int linha = Edit_LineFromChar(hEdit, -1);
    if (linha != atual)
    {
        char mens[80];
        atual = linha;
        sprintf(mens, "Linha %d", linha);
        SendMessage(hLinha, WM_SETTEXT, 0, (LPARAM)TEXT(mens));
    }
// Salva o texto após algum tempo
    if (lista_m)
    {
        lista_m--;
        if (lista_m==0)
            lista_lermudar(lista_ind, 0);
    }
// Comunicação com o servidor
    ProcConec();
}

//---------------------------------------------------------------------------
// Processa editbox
// Fonte: http://forum.builder.cz/read.php?16,3013813
LRESULT CALLBACK WindowProcEdit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_CHAR:
            //if (wParam==VK_RETURN)
            //    if (!(GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
            //        PostMessage(GetParent(hWnd),uMsg,wParam,lParam);
            //        return 0;
            //    }
            if (wParam==VK_TAB)
            {
                PostMessage(GetParent(hWnd),uMsg,wParam,lParam);
                return 0;
            }
            break;
        case WM_GETDLGCODE:
            if (wParam==VK_TAB)
                return DLGC_WANTALLKEYS;
            break;
    }
    return CallWindowProc(oldProc, hWnd, uMsg, wParam, lParam);
}

//---------------------------------------------------------------------------
// The Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_GETDLGCODE:
        {
            HWND hCtl;
            BOOL bPrevious = FALSE;
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
                bPrevious = TRUE;
            hCtl = GetNextDlgTabItem(hwnd,GetFocus(),bPrevious);
            if (hCtl)
                SetFocus(hCtl);
            return DLGC_WANTTAB;
        }
        case WM_CREATE:
        {
            hMenu = CreateMenu();
            AppendMenu(hMenu, MF_STRING, 9001, "&Ler");
            AppendMenu(hMenu, MF_STRING, 9002, "&Enviar");
            AppendMenu(hMenu, MF_STRING, 9003, "&Apagar");
            AppendMenu(hMenu, MF_STRING, 9004, "&Salvar prog");
            AppendMenu(hMenu, MF_STRING, 9005, "&Novo arquivo");
            AppendMenu(hMenu, MF_STRING, 9006, "A&pagar arquivo");
            AppendMenu(hMenu, MF_STRING, 9007, "&Configuração");
            SetMenu(hwnd, hMenu);
            hEdit = CreateWindow("edit", NULL,
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL |
                WS_BORDER | ES_MULTILINE | ES_WANTRETURN | ES_NOHIDESEL |
                ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                250, 20, 100, 25, hwnd, (HMENU)9010, g_hinst, NULL);
            if (hEdit)
            {
                oldProc = (WNDPROC)GetWindowLongPtr(hEdit,GWLP_WNDPROC);
                SetWindowLongPtr(hEdit,GWLP_WNDPROC,(LONG)WindowProcEdit);
                Edit_LimitText(hEdit, 65000);
            }
            hList = CreateWindow("listbox" , NULL,
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | LBS_NOTIFY | WS_BORDER,
                5, 5, 150, 220, hwnd, (HMENU)9011, g_hinst, NULL);
            ListBox_AddItemData(hList, "Lista de classes");
            ListBox_AddItemData(hList, "LOG");
            ListBox_SetCurSel(hList, 0);
            DIR * dir = opendir(".");
            if (dir)
            {
                while (1)
                {
                    char mens[100];
                    int tam;
                    struct dirent * sdir = readdir(dir);
                    if (sdir==0)
                        break;
                    if (memcmp(sdir->d_name, "mud_", 4)!=0)
                        continue;
                    tam = strlen(sdir->d_name+4);
                    if (tam <= 4 || tam > (int)sizeof(mens))
                        continue;
                    if (memcmp(sdir->d_name+tam, ".txt", 4)!=0)
                        continue;
                    memcpy(mens, sdir->d_name+4, tam-4);
                    mens[tam-4]=0;
                    ListBox_AddItemData(hList, mens);
                }
                closedir(dir);
            }
            lista_lermudar(0, 1);
            hLinha = CreateWindow("static" , NULL,
                WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
                10, 10, 30, 20, hwnd, (HMENU)9012, g_hinst, NULL);
            hLabel = CreateWindow("static" , NULL,
                WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
                40, 10, 150, 20, hwnd, (HMENU)9013, g_hinst, NULL);
            Habilitar(1);
            FILE * arq = fopen("editmud.cfg", "r");
            if (arq)
            {
                char * p1, * p2;
                char mens[2048];
                int lido = fread(mens, 1, sizeof(mens)-1, arq);
                fclose(arq);
                mens[lido]=0;
            // Servidor
                for (p1=mens; *p1 && *p1!='\n'; p1++);
                if (*p1) *p1++=0;
                copiastr(dlg_serv, mens, sizeof(dlg_serv));
            // Porta
                p2=p1;
                for (; *p1 && *p1!='\n'; p1++);
                if (*p1) *p1++=0;
                dlg_porta = atoi(p2);
            // Senha
                p2=p1;
                for (; *p1 && *p1!='\n'; p1++);
                if (*p1) *p1++=0;
                copiastr(dlg_senha, p2, sizeof(dlg_senha));
            }
            break;
        }
        case WM_SIZE:
        {
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);
            SetWindowPos(hList, NULL, 2, 2, 160, rcClient.bottom-24, SWP_NOZORDER);
            SetWindowPos(hEdit, NULL, 164, 2, rcClient.right-166, rcClient.bottom-24, SWP_NOZORDER);
            SetWindowPos(hLinha, NULL, 2, rcClient.bottom-20, 70, 18, SWP_NOZORDER);
            SetWindowPos(hLabel, NULL, 74, rcClient.bottom-20, rcClient.right-76, 18, SWP_NOZORDER);
            break;
        }
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case 9001: menu_ler(hwnd); break;
                case 9002: menu_enviar(hwnd); break;
                case 9003: menu_apagar(hwnd); break;
                case 9004: menu_salvar(hwnd); break;
                case 9005: menu_arqcriar(hwnd); break;
                case 9006: menu_arqapagar(hwnd); break;
                case 9007: menu_config(hwnd); break;
                case 9010: if (HIWORD(wParam)==EN_CHANGE) txt_mudou(); break;
                case 9011: if (HIWORD(wParam)==LBN_SELCHANGE) list_mudou(); break;
                break;
            }
            break;
        case WM_TIMER:
            timer_exec();
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

//---------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc;
    MSG msg;

    sock_env = (char*)malloc(65100); // Texto para enviar
    sock_rec = (char*)malloc(65100); // Texto para receber

    // Step 1: Registering the Window Class
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Step 2: Creating the Window
    hPrograma = CreateWindowEx(
        WS_EX_CLIENTEDGE | WS_EX_DLGMODALFRAME,
        g_szClassName,
        "EditMUD",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 480, 240,
        NULL, NULL, hInstance, NULL);

    if (hPrograma == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hPrograma, nCmdShow);
    UpdateWindow(hPrograma);

    SetTimer(hPrograma, 9020, 50, NULL);

    // Step 3: The Message Loop
    while (GetMessage(&msg, NULL, 0, 0) > 0)
        if (!IsDialogMessage(hPrograma, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

    if (lista_m)
        lista_lermudar(lista_ind, 0); // Salva texto

    return msg.wParam;
}

//---------------------------------------------------------------------------
// Conecta ao servidor
// destino: 0=jogar no log, 1=ler
int Conectar(int destino)
{
    const char * erro = "";
    sock_modo = 0;
    StatusBar("Conectando...");
    Habilitar(0);
    while (1)
    {
        struct sockaddr_in conSock;
        struct hostent *hnome;
        // Inicializa WinSock
        WSADATA info;
        if (WSAStartup(MAKEWORD(1, 1), &info) == SOCKET_ERROR)
        {
            erro = "Não consegui acessar interface SOCKET";
            break;
        }
        memset(&conSock,0,sizeof(conSock));
        conSock.sin_addr.s_addr=inet_addr(dlg_serv);
        if ( conSock.sin_addr.s_addr == INADDR_NONE )
        {
            if ( (hnome=gethostbyname(dlg_serv)) == NULL )
            {
                erro = "Endereço do servidor inválido";
                WSACleanup();
                break;
            }
            conSock.sin_addr = (*(struct in_addr *)hnome->h_addr);
        }
        conSock.sin_family=AF_INET;
        conSock.sin_port=htons( (u_short)dlg_porta );
        if ( (sock_manip = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
        {
            erro = "Erro em socket()";
            WSACleanup();
            break;
        }
        if ( connect(sock_manip, (struct sockaddr *)&conSock,
                                          sizeof(struct sockaddr)) == SOCKET_ERROR)
        {
            erro = "Não consegui conectar";
            closesocket(sock_manip);
            WSACleanup();
            break;
        }
        unsigned long argp=1; // 0=bloquear  1=nao bloquear
        if ( ioctlsocket(sock_manip, FIONBIO, &argp) !=0 )
        {
            erro = "Problema em ioctlsocket()";
            closesocket(sock_manip);
            WSACleanup();
            break;
        }
        StatusBar("Comunicando...");
        sock_modo = (destino ? 2 : 1);
        sock_penv = sock_env;
        sock_prec = sock_rec;
        return 1;
    }

    MessageBox(NULL, erro, "Erro", MB_ICONEXCLAMATION | MB_OK);
    StatusBar(erro);
    Habilitar(1);
    return 0;
}

//---------------------------------------------------------------------------
// Desconecta
void Desconectar()
{
    closesocket(sock_manip);
    WSACleanup();
    Habilitar(1);
    sock_modo = 0;
    sock_penv = 0;
    sock_prec = 0;
}

//---------------------------------------------------------------------------
// Processa comunicação com o servidor
void ProcConec()
{
    if (sock_modo==0)
        return;
// Envia mensagem
    while (*sock_penv)
    {
        int resposta;
        for (resposta=0; resposta<1024; resposta++)
            if (sock_penv[resposta]==0)
                break;
        resposta = send(sock_manip, sock_penv, resposta, 0);
        if (resposta==0)
            resposta=-1;
        else if (resposta==SOCKET_ERROR)
        {
            resposta=WSAGetLastError();
            resposta = (resposta==WSAEWOULDBLOCK ? 0 : -1);
        }
        if (resposta > 0)
            sock_penv += resposta;
        if (resposta < 0)
        {
            StatusBar("Erro: Servidor terminou a conexão");
            Desconectar();
            return;
        }
        if (resposta != 1024)
            break;
    }
// Recebendo mensagem
    while (1)
    {
        char buf[4096];
        int resposta;
        resposta=recv(sock_manip, buf, sizeof(buf)-1, 0);
        if (resposta==0)
            resposta=-1;
        else if (resposta==SOCKET_ERROR)
        {
            resposta=WSAGetLastError();
            resposta = (resposta==WSAEWOULDBLOCK ? 0 : -1);
        }
        if (resposta >= 0)
        {
            char *o=buf, *d=sock_prec;
            buf[resposta]=0;
            for (; *o && d<sock_rec+65090; o++)
                if (*o==10)
                    *d++=13, *d++=10;
                else if (*(unsigned char*)o >= ' ')
                    *d++=*o;
            sock_prec = d;
            if (resposta == sizeof(buf)-1)
                continue;
            break;
        }
    // Verifica se mensagem foi enviada e recebida
        while (sock_prec > sock_rec && *(unsigned char*)(sock_prec-1)<' ')
            sock_prec--;
        if (*sock_penv)
        {
            StatusBar("Erro: Servidor terminou a conexão");
            Desconectar();
            break;
        }
        if (sock_prec == sock_rec)
        {
            StatusBar("Erro: Nenhuma resposta; senha incorreta?");
            Desconectar();
            break;
        }
    // sock_prec aponta para última linha recebida
        *sock_prec=0;
        while (sock_prec > sock_rec && *(unsigned char*)(sock_prec-1)>=' ')
            sock_prec--;
    // Se mensagem recebida começa com "-" ou sock_modo!=2...
    //      Uma linha:
    //          Status bar
    //          Fim
    //      Caso contrário:
    //          Escolhe janela de log
        const char * status = (*sock_rec=='-' ? "Erro" : "Feito");
        if (*sock_rec=='-' || sock_modo!=2)
        {
            if (sock_prec == sock_rec)
            {
                char * p = copiastr(buf, status, 100);
                *p++ = ':';
                *p++ = ' ';
                copiastr(p, sock_rec, 1024);
                StatusBar(buf);
                Desconectar();
                break;
            }
            ListBox_SetCurSel(hList, 1);
            list_mudou();
        }
    // Se mensagem recebida não começa com "-": deve terminar com "---"
        if (*sock_rec!='-')
        {
            if (strcmp(sock_prec, "---")!=0)
            {
                StatusBar("Erro: Mensagem recebida incompleta");
                Desconectar();
                break;
            }
            while (sock_prec > sock_rec && (unsigned char)sock_prec[-1]<' ')
                sock_prec--;
            *sock_prec = 0;
        }
        StatusBar(status);
    // Texto na caixa de texto
        Edit_SetText(hEdit, sock_rec);
    // Indica texto alterado
        if (lista_m == 0)
            lista_m = 70;
        Desconectar();
        break;
    }
}
