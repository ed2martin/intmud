/* Este arquivo é software livre; você pode redistribuir e/ou
 * modificar nos termos das licenças GPL ou LGPL. Vide arquivos
 * COPYING e COPYING2.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GPL or the LGP licenses.
 * See files COPYING e COPYING2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifdef __WIN32__
 #include <windows.h>
#else
 #include <termios.h>
 #include <fcntl.h>
 #include <sys/ioctl.h>
#endif
#include "console.h"
#include "misc.h"

TConsole * Console = 0;

//---------------------------------------------------------------------------
#ifdef __WIN32__
HANDLE TConsole::Stdin(void) { return con_in; } ///< Entrada padrão
HANDLE TConsole::Stdout(void) { return con_out; } ///< Saída padrão
#else
int TConsole::Stdin(void) { return STDIN_FILENO; } ///< Entrada padrão
int TConsole::Stdout(void) { return STDOUT_FILENO; } ///< Saída padrão
#endif

//---------------------------------------------------------------------------
bool TConsole::Inic(bool completo)
{
// Verifica se já está aberto
    if (Aberto >= 2)
        return true;

// Inicializa variáveis
    LinTotal = 24;
    ColTotal = 80;
    LinAtual = 0;
    CorAtual = 0x1F00; //0x70;
    Charset = 0x100;
    LerUTF8 = 0;

#ifdef __WIN32__
// Aloca console
    if (!AllocConsole())
        return false;

// Obtém HANDLE de entrada e saída
    con_in  = GetStdHandle(STD_INPUT_HANDLE);
    con_out = GetStdHandle(STD_OUTPUT_HANDLE);

// Obtém o conjunto de caracteres
    switch (GetConsoleOutputCP())
    {
    case 28591: // iso8859-1
        Charset = 0x101;
        break;
    case 1252: // cp1252
        Charset = 0x102;
        break;
    case 850:  // ibm850
        Charset = 0x103;
        break;
    case 65001: // utf-8
        Charset = 0x80;
        break;
    }

// Configuração do console
    if (!SetConsoleMode(con_in, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT))
    {
        FreeConsole();
        return false;
    }

// Tamanho da tela
    CONSOLE_SCREEN_BUFFER_INFO atrib;
    if (GetConsoleScreenBufferInfo(con_out, &atrib))
    {
        ColTotal = atrib.dwSize.X;
        LinTotal = atrib.dwSize.Y;
    }

// Acerta cor e variáveis
    MoverCursor = false;
    LerCont = 0;
    LerTotal = 0;

#else
// Obtém o tamanho do terminal
// Nota: pode-se usar 'signal' para ler o sinal SIGWINCH - quando tamanho mudar
    struct winsize w;
    memset(&w, 0, sizeof(w));
    int stat = ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if (stat == 0)
    {
        ColTotal = w.ws_col;
        LinTotal = w.ws_row;
    }

// Obtém o conjunto de caracteres
// Nota: para obter os nomes possíveis executar: locale -m
    const char * lang = getenv("LANG");
    if (lang)
    {
        char mens[0x100];
        char * d = mens;
        for (; *lang && d<mens+sizeof(mens)-2; lang++)
            if (*lang>='a' && *lang<='z')
                *d++ = *lang - 0x20;
            else if (*lang != '-' && *lang != ' ')
                *d++ = *lang;
        *d++ = '-';
        *d = 0;
        if (strstr(mens, "UTF8-")!=0)
            Charset = 0x80;
        else if (strstr(mens, "ISO88591-")!=0)
            Charset = 0x101;
        else if (strstr(mens, "CP1252-")!=0)
            Charset = 0x102;
        else if (strstr(mens, "CP850-")!=0 || strstr(mens, "IBM850-")!=0)
            Charset = 0x103;
    }

// Acerta variáveis
    LerPont = 0;
    LerTexto[0] = 0;
#endif

// Acerta StrConv[]
    const unsigned char StrA0[] = {
            // A0-A7: símbolos
          0x20, 0xAD, 0xBD, 0x9C, 0xCF, 0xBE, 0xDD, 0xF5,
            // A8-AF: símbolos
          0xF9, 0xB8, 0xA6, 0xAE, 0xAA, 0xF0, 0xA9, 0xEE,
            // B0-B7: símbolos
          0xF8, 0xF1, 0xFD, 0xFC, 0xEF, 0xE6, 0xF4, 0xFA,
            // B8-BF: símbolos
          0xF7, 0xFB, 0xA7, 0xAF, 0xAC, 0xAB, 0xF3, 0xA8,
            // C0-C5: A crase, agudo, circunflexo, til, trema, bola
          0xB7, 0xB5, 0xB6, 0xC7, 0x8E, 0x8F,
            // C6-C7: AE, C cedilha
          0x92, 0x80,
            // C8-CB: E grave, agudo, circunflexo, trema
          0xD4, 0x90, 0xD2, 0xD3,
            // CC-CF: I grave, agudo, circunflexo, trema
          0xDE, 0xD6, 0xD7, 0xD8,
            // D0-D1: D cortado, N til
          0xD1, 0xA5,
            // D2-D6: O grave, agudo, circunflexo, til, trema
          0xE3, 0xE0, 0xE2, 0xE5, 0x99,
            // D7-D8: x, conjunto vazio maiúsculo
          0x9E, 0x9D,
            // D9-DC: U grave, agudo, circunflexo, trema
          0xEB, 0xE9, 0xEA, 0x9A,
            // DD-DF: Y agudo, p, beta
          0xED, 0xE7, 0xE1,
            // E0-E5: a crase, agudo, circunflexo, til, trema, bola
          0x85, 0xA0, 0x83, 0xC6, 0x84, 0x86,
            // E6-E7: ae, c cedilha
          0x91, 0x87,
            // E8-EB: e grave, agudo, circunflexo, trema
          0x8A, 0x82, 0x88, 0x89,
            // EC-EF: i grave, agudo, circunflexo, trema
          0x8D, 0xA1, 0x8C, 0x8B,
            // F0-F1: ???, n til
          0xD0, 0xA4,
            // F2-F6: o grave, agudo, circunflexo, til, trema
          0x95, 0xA2, 0x93, 0xE4, 0x94,
            // F7-F8: dividir, conjunto vazio minúsculo
          0xF6, 0x9B,
            // F9-FC: u grave, agudo, circunflexo, trema
          0x97, 0xA3, 0x96, 0x81,
            // FD-FF: y agudo, P, y trema
          0xEC, 0xE8, 0x98 };
// 0 ~ 0x7F é padronizado
    memset(StrConv, ' ', sizeof(StrConv));
    for (int x=0; x<0x80; x++)
        StrConv[x] = x;
// IBM850
    if (Charset==0x103)
    {
        memcpy(StrConv+0xA0, StrA0, 0x60);
        memset(StrLer, ' ', sizeof(StrLer));
    }
// Desconhecido
    else if (Charset==0x100)
    {
        const char StrNormal[] =
                "AAAAAA C" "EEEEIIII"  // 0xC0
                "DNOOOOOx" " UUUUY  "  // 0xD0
                "aaaaaa c" "eeeeiiii"  // 0xE0
                " nooooo " " uuuuy y"; // 0xF0
        memcpy(StrConv+0xC0, StrNormal, 0x40);
    }
// ISO8859-1
    else
        for (int x=0xA1; x<0x100; x++)
            StrConv[x] = x;

// Acerta StrLer a partir de StrConv
    for (int x=0xFF; x>=0; x--)
        StrLer[ (unsigned char)StrConv[x] ] = x;
    //printf("Charset = %x\n", Charset); fflush(stdout);

// Verifica se configuração completa
    if (!completo)
    {
        Aberto = 1;
        return true;
    }

#ifndef __WIN32__
// Verifica se é um terminal
    if (!isatty(STDIN_FILENO))
        return false;

// Lê configuração do terminal
    struct termios terminal;
    if (tcgetattr(STDIN_FILENO, &terminal))
        return false;
    term_antes = terminal;

// Altera configuração do terminal
    terminal.c_iflag &= ~(INLCR|IGNCR|ICRNL | ISTRIP | PARMRK);
    terminal.c_lflag &= ~(ICANON
                          | ECHO|ECHONL // Não ecoa caracteres recebidos
                          //| ISIG        // Desabilita caracteres INTR, QUIT e SUSP
                          | IEXTEN);    // Desabilita caracteres LNEXT e DISCARD
    terminal.c_cc[VMIN]=0;         // Retornar imediatamente
    terminal.c_cc[VTIME]=0;        // Não retornar após algum tempo
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal);
    fcntl_block = true;
    fcntl(STDIN_FILENO, F_SETFL, 0);
#endif

    Aberto = 2;
    return true;
}

//---------------------------------------------------------------------------
void TConsole::Fim()
{
    int antes = Aberto;
    Aberto = 0;
    if (antes < 2)
        return;
#ifdef __WIN32__
    FreeConsole();
#else
    printf("\x1B[0m\n");
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_antes);
#endif
}

//---------------------------------------------------------------------------
void TConsole::MudaTitulo(const char * texto)
{
#ifdef __WIN32__
    char mens[100];
    copiastr(mens, texto, sizeof(mens));
    for (char * p = mens; *p; p++)
        *p = StrConv[*(unsigned char*)p];
    SetConsoleTitle(mens);
#endif
}

//---------------------------------------------------------------------------
void TConsole::Beep()
{
#ifdef __WIN32__
    MessageBeep(0xFFFFFFFF);
#else
    putchar(7);
#endif
}

//---------------------------------------------------------------------------
const char * TConsole::LerTecla()
{
#ifdef __WIN32__
    while (true)
    {
        if (LerCont >= LerTotal)
        {
            LerCont = 0;
            GetNumberOfConsoleInputEvents(con_in, &LerTotal);
            if (LerTotal==0)
                break;
            ReadConsoleInput(con_in, LerEventos,
                     sizeof(LerEventos)/sizeof(LerEventos[0]), &LerTotal);
            continue;
        }
        int indice = LerCont++;
        if (LerEventos[indice].EventType == MOUSE_EVENT)
        {
            int valor = LerEventos[indice].Event.MouseEvent.dwButtonState;
            char ch = 0;
            if (valor & FROM_LEFT_1ST_BUTTON_PRESSED)
                ch = '1';
            else if (valor & RIGHTMOST_BUTTON_PRESSED)
                ch = '2';
            else if (valor & FROM_LEFT_2ND_BUTTON_PRESSED)
                ch = '3';
            if (ch)
            {
                static char mens[30];
                sprintf(mens, "MOUSE%c %d %d", ch,
                    LerEventos[indice].Event.MouseEvent.dwMousePosition.X,
                    LerEventos[indice].Event.MouseEvent.dwMousePosition.Y);
                return mens;
            }
        }
        if (LerEventos[indice].EventType != KEY_EVENT ||
            LerEventos[indice].Event.KeyEvent.bKeyDown == 0)
            continue;
        // Nota: nos testes, o Windows não envia a informação correta
        //       com codepage 65001 (UTF-8)
        int ch = (unsigned char)LerEventos[indice].Event.KeyEvent.uChar.AsciiChar;
        if (ch >= Charset)
        {
            if (ch >= 0xC0) // Primeiro byte do caracter
            {
                if (ch < 0xE0)
                    LerUTF8 = ch & 0x1F;
                else
                    LerUTF8 = 0;
                continue;
            }
            if (LerUTF8 == 0) // Ignorando
                continue;
            ch = LerUTF8 * 0x40 + (ch & 0x3F); // Obtém caracter
            LerUTF8 = 0;
            if (ch >= 0x100) // Ignora caracteres não ISO8859-1
                continue;
        }
        else
            ch = (unsigned char)StrLer[ch];
        LerTexto[0] = ch;
        LerTexto[1] = 0;
        if ((unsigned char)LerTexto[0] >= ' ')
            return LerTexto;
        DWORD state = LerEventos[indice].Event.KeyEvent.dwControlKeyState;
        bool shift = state & SHIFT_PRESSED;
        bool ctrl = state & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED);
        switch (LerEventos[indice].Event.KeyEvent.wVirtualKeyCode)
        {
        case VK_BACK:   return "BACK";
        case VK_TAB:    return (shift ? "S_TAB" : "TAB");
        case VK_RETURN: return (shift ? "S_ENTER" : "ENTER");
        case VK_ESCAPE: return "ESC";
        case VK_PRIOR:  return (ctrl ? "C_PGUP" : "PGUP");
        case VK_NEXT:   return (ctrl ? "C_PGDN" : "PGDN");
        case VK_END:    return (ctrl ? "C_END" : "END");
        case VK_HOME:   return (ctrl ? "C_HOME" : "HOME");
        case VK_LEFT:   return (ctrl ? "C_LEFT" : "LEFT");
        case VK_UP:     return (ctrl ? "C_UP" : "UP");
        case VK_RIGHT:  return (ctrl ? "C_RIGHT" : "RIGHT");
        case VK_DOWN:   return (ctrl ? "C_DOWN" : "DOWN");
        case VK_INSERT: return (ctrl ? "C_INS" : "INS");
        case VK_DELETE: return (ctrl ? "C_DEL" : "DEL");
        case VK_F1:     return (shift ? "S_F1" : "F1");
        case VK_F2:     return (shift ? "S_F2" : "F2");
        case VK_F3:     return (shift ? "S_F3" : "F3");
        case VK_F4:     return (shift ? "S_F4" : "F4");
        case VK_F5:     return (shift ? "S_F5" : "F5");
        case VK_F6:     return (shift ? "S_F6" : "F6");
        case VK_F7:     return (shift ? "S_F7" : "F7");
        case VK_F8:     return (shift ? "S_F8" : "F8");
        case VK_F9:     return (shift ? "S_F9" : "F9");
        case VK_F10:    return (shift ? "S_F10" : "F10");
        case VK_F11:    return (shift ? "S_F11" : "F11");
        case VK_F12:    return (shift ? "S_F12" : "F12");
        case 'A':       if (ctrl) return "C_A"; break;
        case 'B':       if (ctrl) return "C_B"; break;
        case 'C':       if (ctrl) return "C_C"; break;
        case 'D':       if (ctrl) return "C_D"; break;
        case 'E':       if (ctrl) return "C_E"; break;
        case 'F':       if (ctrl) return "C_F"; break;
        case 'G':       if (ctrl) return "C_G"; break;
        case 'H':       if (ctrl) return "C_H"; break;
        case 'I':       if (ctrl) return "C_I"; break;
        case 'J':       if (ctrl) return "C_J"; break;
        case 'K':       if (ctrl) return "C_K"; break;
        case 'L':       if (ctrl) return "C_L"; break;
        case 'M':       if (ctrl) return "C_M"; break;
        case 'N':       if (ctrl) return "C_N"; break;
        case 'O':       if (ctrl) return "C_O"; break;
        case 'P':       if (ctrl) return "C_P"; break;
        case 'Q':       if (ctrl) return "C_Q"; break;
        case 'R':       if (ctrl) return "C_R"; break;
        case 'S':       if (ctrl) return "C_S"; break;
        case 'T':       if (ctrl) return "C_T"; break;
        case 'U':       if (ctrl) return "C_U"; break;
        case 'V':       if (ctrl) return "C_V"; break;
        case 'W':       if (ctrl) return "C_W"; break;
        case 'X':       if (ctrl) return "C_X"; break;
        case 'Y':       if (ctrl) return "C_Y"; break;
        case 'Z':       if (ctrl) return "C_Z"; break;
        }
    }

#else

    if (fcntl_block)
    {
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
        fcntl_block=false;
    }
#if 0
    int ch = getc(stdin);
    if (ch<0) return 0;
    if (ch==3) return "BREAK";
    if (ch<' ') sprintf(LerTexto, "[%d]", ch);
    else sprintf(LerTexto, "%c[%d]", ch, ch);
    return LerTexto;
#endif
    while (true)
    {
    // Lê próxima tecla
        int ch;
        if (LerTexto[0]==0)
        {
            ch = getc(stdin);
            if (ch >= Charset)
            {
                if (ch >= 0xC0) // Primeiro byte do caracter
                {
                    if (ch < 0xE0)
                        LerUTF8 = ch & 0x1F;
                    else
                        LerUTF8 = 0;
                    continue;
                }
                if (LerUTF8 == 0) // Ignorando
                    continue;
                ch = LerUTF8 * 0x40 + (ch & 0x3F); // Obtém caracter
                LerUTF8 = 0;
                if (ch >= 0x100) // Ignora caracteres não ISO8859-1
                    continue;
            }
            else if (ch >= 0)
                ch = (unsigned char)StrLer[ch];
        }
        else
            ch = (unsigned char)LerTexto[0], LerTexto[0] = 0;
        if (ch <= 0)
        {
            if (LerPont!=1)
                break;
            LerPont=0;
            return "ESC";
        }
        //if (ch==3) return "BREAK";
    // Checa teclas sem o ESC
        if (LerPont==0)
        {
            if (ch==127) return "BACK";
        // Caracter normal
            if ((unsigned char)ch>=' ')
            {
                LerTexto[1] = ch;
                LerTexto[2] = 0;
                return LerTexto + 1;
            }
        // Outros caracteres
            switch (ch)
            {
            case 1: return "C_A";
            case 2: return "C_B";
            case 3: return "C_C"; // Control+C termina o programa
            case 4: return "C_D";
            case 5: return "C_E";
            case 6: return "C_F";
            case 7: return "C_G";
            case 8: return "C_H";
            case 9: return "TAB"; // Control+I = TAB
            case 10: return "C_J";
            case 11: return "C_K";
            case 12: return "C_L";
            case 13: return "ENTER"; // Control+M = ENTER
            case 14: return "C_N";
            case 15: return "C_O";
            case 16: return "C_P";
            case 17: return "C_Q";
            case 18: return "C_R";
            case 19: return "C_S";
            case 20: return "C_T";
            case 21: return "C_U";
            case 22: return "C_V";
            case 23: return "C_W";
            case 24: return "C_X";
            case 25: return "C_Y";
            case 26: return "C_Z"; // Control+Z coloca em segundo plano
            case 27: LerPont=1; break; // ESC
            }
            continue;
        }
    // Obtém string de teclas com o ESC
        if (LerPont==1)
        {
            if (ch!='[' && ch!='O')
            {
                LerTexto[0] = ch;
                LerPont = 0;
                return "ESC";
            }
            LerTexto[1] = ch;
            LerPont++;
            continue;
        }
        if (LerPont < 14)
            LerTexto[LerPont++] = ch;
        if (ch < 'A' || ch=='[')
            continue;
        LerTexto[LerPont] = 0;
        LerPont = 0;
        //Escrever(LerTexto+1);
    // Obtém o nome da tecla a partir da string

    // Códigos começam com ESC + [ ou ESC + O e terminam com caracter >= A
    // Após o ESC, obrigatoriamente em ordem alfabética:
        const char * cod_esc[] = {
            "O2P\0"     "S_F1",
            "O2Q\0"     "S_F2",
            "O2R\0"     "S_F3",
            "O2S\0"     "S_F4",
            "OM\0"      "S_ENTER",
            "OP\0"      "F1",
            "OQ\0"      "F2",
            "OR\0"      "F3",
            "OS\0"      "F4",
            "[15;2~\0"  "S_F5",
            "[15~\0"    "F5",
            "[17;2~\0"  "S_F6",
            "[17~\0"    "F6",
            "[18;2~\0"  "S_F7",
            "[18~\0"    "F7",
            "[19;2~\0"  "S_F8",
            "[19~\0"    "F8",
            "[1;5A\0"   "C_UP",
            "[1;5B\0"   "C_DOWN",
            "[1;5C\0"   "C_RIGHT",
            "[1;5D\0"   "C_LEFT",
            "[1;5F\0"   "C_END",
            "[1;5H\0"   "C_HOME",
            "[1~\0"     "HOME",
            "[20;2~\0"  "S_F9",
            "[20~\0"    "F9",
            "[21;2~\0"  "S_F10",
            "[21~\0"    "F10",
            "[23;2~\0"  "S_F11",
            "[23~\0"    "F11",
            "[24;2~\0"  "S_F12",
            "[24~\0"    "F12",
            "[2;5~\0"   "C_INS",
            "[2~\0"     "INS",
            "[3;5~\0"   "C_DEL",
            "[3~\0"     "DEL",
            "[4~\0"     "END",
            "[5;5~\0"   "C_PGUP",
            "[5~\0"     "PGUP",
            "[6;5~\0"   "C_PGDN",
            "[6~\0"     "PGDN",
            "[A\0"      "UP",
            "[B\0"      "DOWN",
            "[C\0"      "RIGHT",
            "[D\0"      "LEFT",
            "[F\0"      "END",
            "[H\0"      "HOME",
            "[Z\0"      "S_TAB",
            "[[A\0"     "F1",
            "[[B\0"     "F2",
            "[[C\0"     "F3",
            "[[D\0"     "F4",
            "[[E\0"     "F5" };
        int ini = 0;
        int fim = sizeof(cod_esc) / sizeof(cod_esc[0]) - 1;
        while (ini <= fim)
        {
            int meio = (ini+fim)/2;
            int valor = strcmp(LerTexto+1, cod_esc[meio]);
            if (valor==0)
            {
                const char * p = cod_esc[meio];
                while (*p++);
                return p;
            }
            if (valor<0)
                fim = meio - 1;
            else
                ini = meio + 1;
        }
    }
#endif

    return 0;
}

//---------------------------------------------------------------------------
void TConsole::Flush()
{
#ifdef __WIN32__
    if (MoverCursor)
    {
        COORD posic;
        posic.X = ColAtual;
        posic.Y = LinAtual;
        SetConsoleCursorPosition(con_out,  posic);
        MoverCursor = false;
        //char txt[100];
        //sprintf(txt, "[%d,%d]\n", LinAtual, ColAtual);
        //EnvTxt(txt, strlen(txt));
    }
#else
    fflush(stdout);
#endif
}

//---------------------------------------------------------------------------
void TConsole::EnvTxt(const char * texto, int tamanho)
{
#ifdef __WIN32__
    MoverCursor = true;
    while (tamanho>0)
    {
        CHAR_INFO mens[1024];
        PCHAR_INFO dest = mens;
        COORD mensposic; // Posição no buffer, 0
        COORD menstam; // Tamanho do buffer
        SMALL_RECT mensrect; // Retângulo a copiar
        mensposic.X = 0; // Tamanho do buffer
        mensposic.Y = 0;
        menstam.X = 1024; // Posição no buffer
        menstam.Y = 1;
        int total = sizeof(mens)/sizeof(mens[0]);
        if (total>tamanho) total=tamanho;
        tamanho -= total;
        for (; total>0; total--,texto++)
        {
        // Qualquer caracter exceto nova linha
            if (*texto != '\n')
            {
                unsigned char ch = StrConv[ *(unsigned char*)texto ];
                dest->Char.AsciiChar = ch;
                dest->Attributes = CorAtributos;
                dest++;
                if (++ColAtual < ColTotal)
                    continue;
            }
        // Envia mensagem
            mensrect.Top = mensrect.Bottom = LinAtual;
            mensrect.Left = ColAtual - (dest-mens);
            mensrect.Right = ColAtual - 1;
            WriteConsoleOutput(con_out, mens, menstam, mensposic, &mensrect);
            dest = mens;
        // Acerta posição do cursor
            ColAtual = 0;
            if (++LinAtual < LinTotal)
                continue;
        // Rolagem da tela
            LinAtual = LinTotal - 1;
            mensrect.Top = 0;      // Região que será movida
            mensrect.Bottom = LinTotal - 1;
            mensrect.Left = 0;
            mensrect.Right = ColTotal - 1;
            mens[0].Char.AsciiChar = ' '; // Como preencher a região vazia
            mens[0].Attributes = CorAtributos;
            COORD mensdest; // Para onde mover
            mensdest.X = 0;
            mensdest.Y = -1;
            ScrollConsoleScreenBuffer(con_out, &mensrect, NULL, mensdest, mens);
        }
        if (dest > mens) // Mensagens pendentes
        {
            mensrect.Top = mensrect.Bottom = LinAtual;
            mensrect.Left = ColAtual - (dest-mens);
            mensrect.Right = ColAtual - 1;
            WriteConsoleOutput(con_out, mens, menstam, mensposic, &mensrect);
        }
    }
#else
    if (!fcntl_block)
        { fcntl(STDIN_FILENO, F_SETFL, 0); fcntl_block=true; }
    while (tamanho>0)
    {
        char mens[1024];
        char * dest = mens;
        int total = sizeof(mens)/2;
        if (total>tamanho) total=tamanho;
        tamanho -= total;
        for (; total>0; total--,texto++)
        {
            if (*texto == '\n')
            {
                LinAtual++, ColAtual = 0;
                *dest++ = '\n';
                continue;
            }
            unsigned char ch = StrConv[ *(unsigned char*)texto ];
            if (ch < Charset)
                *dest++ = ch;
            else
            {
                *dest++ = 0xC0 + ch / 0x40;
                *dest++ = 0x80 + (ch & 0x3F);
            }
            if (++ColAtual >= ColTotal)
                ColAtual = 0, LinAtual++, *dest++ = '\n';
        }
        fwrite(mens, 1, dest-mens, stdout);
    }
    if (LinAtual >= LinTotal)
        LinAtual = LinTotal - 1;
#endif
}

//---------------------------------------------------------------------------
void TConsole::CorTxt(unsigned int novacor)
{
// Acerta a cor
    unsigned int antes = CorAtual;
    CorAtual = novacor;
// Troca frente com fundo
    if (antes & 0x200)
        antes = (antes&~0x277) | ((antes>>4)&7) | ((antes<<4)&0x70);
    if (novacor & 0x200)
        novacor=(novacor&~0x277) | ((novacor>>4)&7) | ((novacor<<4)&0x70);
#ifdef __WIN32__
    if (((antes ^ novacor) & 0xF7) == 0)
        return;
    WORD atributos = 0;
    if (novacor&1) atributos |= BACKGROUND_RED;
    if (novacor&2) atributos |= BACKGROUND_GREEN;
    if (novacor&4) atributos |= BACKGROUND_BLUE;
    if (novacor&16) atributos |= FOREGROUND_RED;
    if (novacor&32) atributos |= FOREGROUND_GREEN;
    if (novacor&64) atributos |= FOREGROUND_BLUE;
    if (novacor&128) atributos |= FOREGROUND_INTENSITY;
    // Nota: a opção de texto sublinhado é ignorada pelo Windows
    // Vide:
    // http://blogs.msdn.com/b/michkap/archive/2005/10/10/478911.aspx
    // http://support.microsoft.com/default.aspx?scid=kb;en-us;145925
    //if (novacor&256) atributos |= 0x8000; // COMMON_LVB_UNDERSCORE;
    SetConsoleTextAttribute(con_out, atributos);
    CorAtributos = atributos;
#else
    if (antes == novacor)
        return;
    char mens[32];
    char * destino = mens;
    char ini='[';
    //sprintf(destino, "%02x", cor); destino+=2;
    *destino++ = 0x1B;
                        // Tirar negrito, sublinhado e piscando
    if (((antes ^ novacor) & antes & 0x580) || novacor == 0x70)
    {
        destino[0] = ini;
        destino[1] = '0';
        destino+=2, ini=';';
        antes = 0x70;
    }
    if ((antes&0x80)==0 && (novacor&0x80)) // Negrito
    {
        destino[0] = ini;
        destino[1] = '1';
        destino+=2, ini=';';                    
    }
    if ((antes&0x100)==0 && (novacor&0x100)) // Sublinhado
    {
        destino[0] = ini;
        destino[1] = '4';
        destino+=2, ini=';';
    }
    if ((antes&0x400)==0 && (novacor&0x400)) // Texto piscando
    {
        destino[0] = ini;
        destino[1] = '5';
        destino+=2, ini=';';
    }
    if ((antes^novacor)&0x70) // Frente
    {
        destino[0] = ini;
        destino[1] = '3';
        destino[2] = ((novacor>>4)&7)+'0';
        destino+=3, ini=';';
    }
    if ((antes^novacor)&15) // Fundo
    {
        destino[0] = ini;
        destino[1] = '4';
        destino[2] = (novacor&7)+'0';
        destino+=3, ini=';';
    }
    *destino++ = 'm';
    *destino = 0;
    if (!fcntl_block)
        { fcntl(STDIN_FILENO, F_SETFL, 0); fcntl_block=true; }
    printf("%s", mens);
#endif
}

//---------------------------------------------------------------------------
void TConsole::CursorIni()
{
#ifdef __WIN32__
    MoverCursor = true;
#else
    putchar('\r');
#endif
    ColAtual = 0;
}

//---------------------------------------------------------------------------
void TConsole::CursorLin(int valor)
{
    if (valor==0)
        return;
#ifdef __WIN32__
    MoverCursor = true;
#else
    if (!fcntl_block)
        { fcntl(STDIN_FILENO, F_SETFL, 0); fcntl_block=true; }
    if (valor>0)
        printf("\x1B[%dB", valor);
    else
        printf("\x1B[%dA", -valor);
#endif
    LinAtual += valor;
    if (LinAtual < 0)
        LinAtual = 0;
    if (LinAtual >= LinTotal)
        LinAtual = LinTotal-1;
}

//---------------------------------------------------------------------------
void TConsole::CursorCol(int valor)
{
    if (valor==0)
        return;
#ifdef __WIN32__
    MoverCursor = true;
#else
    if (!fcntl_block)
        { fcntl(STDIN_FILENO, F_SETFL, 0); fcntl_block=true; }
    if (valor>0)
        printf("\x1B[%dC", valor);
    else
        printf("\x1B[%dD", -valor);
#endif
    ColAtual += valor;
    if (ColAtual < 0)
        ColAtual = 0;
    if (ColAtual >= ColTotal)
        ColAtual = ColTotal-1;
}

//---------------------------------------------------------------------------
void TConsole::CursorPosic(int lin, int col)
{
    LinAtual = (lin<0 ? 0 : lin>=(int)LinTotal ? LinTotal-1 : lin);
    ColAtual = (col<0 ? 0 : col>=(int)ColTotal ? ColTotal-1 : col);
#ifdef __WIN32__
    MoverCursor = true;
#else
    if (!fcntl_block)
        { fcntl(STDIN_FILENO, F_SETFL, 0); fcntl_block=true; }
    printf("\x1B[%d;%dH", LinAtual+1, ColAtual+1);
#endif
}

//---------------------------------------------------------------------------
void TConsole::InsereLin(int valor)
{
    if (valor<=0)
        return;
#ifdef __WIN32__
    SMALL_RECT scroll;  // Região que será movida
    COORD dest;         // Para onde mover
    CHAR_INFO charinfo; // Como preencher a região que ficou vazia
    scroll.Top = LinAtual;
    scroll.Bottom = LinTotal - 1;
    scroll.Left = 0;
    scroll.Right = ColTotal - 1;
    dest.X = 0;
    dest.Y = LinAtual + valor;
    charinfo.Char.AsciiChar = ' ';
    charinfo.Attributes = CorAtributos;
    ScrollConsoleScreenBuffer(con_out, &scroll, NULL, dest, &charinfo);
#else
    if (!fcntl_block)
        { fcntl(STDIN_FILENO, F_SETFL, 0); fcntl_block=true; }
    printf("\x1B[%dL", valor);
#endif
}

//---------------------------------------------------------------------------
void TConsole::ApagaLin(int valor)
{
    if (valor<=0)
        return;
#ifdef __WIN32__
    SMALL_RECT scroll;  // Região que será movida
    COORD dest;         // Para onde mover
    CHAR_INFO charinfo; // Como preencher a região que ficou vazia
    scroll.Top = LinAtual + valor;
    scroll.Bottom = LinTotal - 1 + valor;
    scroll.Left = 0;
    scroll.Right = ColTotal - 1;
    dest.X = 0;
    dest.Y = LinAtual;
    charinfo.Char.AsciiChar = ' ';
    charinfo.Attributes = CorAtributos;
    ScrollConsoleScreenBuffer(con_out, &scroll, NULL, dest, &charinfo);
#else
    if (!fcntl_block)
        { fcntl(STDIN_FILENO, F_SETFL, 0); fcntl_block=true; }
    printf("\x1B[%dM", valor);
#endif
}

//---------------------------------------------------------------------------
void TConsole::InsereCol(int valor)
{
    if (valor<=0)
        return;
#ifdef __WIN32__
    SMALL_RECT scroll;  // Região que será movida
    COORD dest;         // Para onde mover
    CHAR_INFO charinfo; // Como preencher a região que ficou vazia
    dest.Y = scroll.Top = scroll.Bottom = LinAtual;
    scroll.Left = ColAtual;
    scroll.Right = ColTotal - 1;
    dest.X = ColAtual + valor;
    charinfo.Char.AsciiChar = ' ';
    charinfo.Attributes = CorAtributos;
    ScrollConsoleScreenBuffer(con_out, &scroll, NULL, dest, &charinfo);
#else
    if (!fcntl_block)
        { fcntl(STDIN_FILENO, F_SETFL, 0); fcntl_block=true; }
    printf("\x1B[%d@", valor);
#endif
}

//---------------------------------------------------------------------------
void TConsole::ApagaCol(int valor)
{
    if (valor<=0)
        return;
#ifdef __WIN32__
    SMALL_RECT scroll;  // Região que será movida
    COORD dest;         // Para onde mover
    CHAR_INFO charinfo; // Como preencher a região que ficou vazia
    dest.Y = scroll.Top = scroll.Bottom = LinAtual;
    scroll.Left = ColAtual + valor;
    scroll.Right = ColTotal - 1 + valor;
    dest.X = ColAtual;
    charinfo.Char.AsciiChar = ' ';
    charinfo.Attributes = CorAtributos;
    ScrollConsoleScreenBuffer(con_out, &scroll, NULL, dest, &charinfo);
#else
    if (!fcntl_block)
        { fcntl(STDIN_FILENO, F_SETFL, 0); fcntl_block=true; }
    printf("\x1B[%dP", valor);

    printf("\x1B[s" // Salva o cursor
           "\x1B[500C"); // Vai para o fim da linha
    if (valor > 1)
        printf("\x1B[%dD", valor-1); // Retorna um ou mais caracteres
    printf("\x1B[K" // Apaga até o fim da linha
           "\x1B[u"); // Restaura o cursor
#endif
}

//---------------------------------------------------------------------------
void TConsole::LimpaFim()
{
#ifdef __WIN32__
    DWORD CharsWritten;
    COORD posic;
    posic.X = ColAtual;
    posic.Y = LinAtual;
    FillConsoleOutputCharacter(con_out, ' ', ColTotal-ColAtual,
                posic, &CharsWritten);
    FillConsoleOutputAttribute(con_out, CorAtributos, ColTotal-ColAtual,
                posic, &CharsWritten);
#else
    if (!fcntl_block)
        { fcntl(STDIN_FILENO, F_SETFL, 0); fcntl_block=true; }
    printf("\x1B[K");
#endif
}

//---------------------------------------------------------------------------
void TConsole::LimpaTela()
{
#ifdef __WIN32__
    DWORD CharsWritten;
    COORD posic = { 0, 0 };
    FillConsoleOutputCharacter(con_out, ' ', ColTotal*LinTotal,
                posic, &CharsWritten);
    FillConsoleOutputAttribute(con_out, CorAtributos, ColTotal*LinTotal,
                posic, &CharsWritten);
#else
    if (!fcntl_block)
        { fcntl(STDIN_FILENO, F_SETFL, 0); fcntl_block=true; }
    printf("\x1B[2J");
#endif
}
