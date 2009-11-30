/* Este programa é software livre; você pode redistribuir e/ou
 * modificar nos termos da GNU General Public License V2
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details at www.gnu.org
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifdef __WIN32__
 #include "windows.h"
#else
 #include <termios.h>
 #include <fcntl.h>
 #include <sys/ioctl.h>
#endif
#include "var-telatxt.h"
#include "variavel.h"
#include "instr.h"
#include "misc.h"

#define CONSOLE_COR_LINHA 0x74
#define CONSOLE_MAX 300

TConsole * TVarTelaTxt::Console = 0;
TVarTelaTxt * TVarTelaTxt::Inicio = 0;
TVarTelaTxt * TVarTelaTxt::ObjAtual = 0;

//---------------------------------------------------------------------------
#ifdef __WIN32__
HANDLE TConsole::Stdin(void) { return con_in; } ///< Entrada padrão
HANDLE TConsole::Stdout(void) { return con_out; } ///< Saída padrão
#else
int TConsole::Stdin(void) { return STDIN_FILENO; } ///< Entrada padrão
int TConsole::Stdout(void) { return STDOUT_FILENO; } ///< Saída padrão
#endif

//---------------------------------------------------------------------------
bool TConsole::Inic()
{
// Verifica se já está aberto
    if (Aberto)
        return true;

// Inicializa variáveis
    col_linha = 0;
    tam_linha = 0;
    *linha = 0;

    Editor = true;
    ColEscreve = 0;
    ColEditor = 0;
    CorTela = 0x70;
    LinTotal = 24;
    ColTotal = 80;
    LinAtual = 0;
    CorAtual = 0x70;

#ifdef __WIN32__
// Aloca console
    if (!AllocConsole())
        return false;

// Acerta StrConv[]
    const char StrA0[] = {
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
    memset(StrConv, ' ', sizeof(StrConv));
    for (int x=0; x<0x80; x++)
        StrConv[x] = x;
    memcpy(StrConv+0xA0, StrA0, 0x60);
    memset(StrLer, ' ', sizeof(StrLer));
    for (int x=0xFF; x>=0; x--)
        StrLer[ (unsigned char)StrConv[x] ] = x;

// Obtém HANDLE de entrada e saída
    con_in  = GetStdHandle(STD_INPUT_HANDLE);
    con_out = GetStdHandle(STD_OUTPUT_HANDLE);

// Conjunto de caracteres
    SetConsoleCP(850); // Sets the input code page
    SetConsoleOutputCP(850); // Sets the output code page
        // Na prática, SetConsoleOutputCP(28591) não tem efeito.
        // Por isso, melhor deixar em 850 e acertar os códigos dos
        // caracteres através de uma tabela (StrA0, StrConv e StrLer)

// Configuração do console
    if (!SetConsoleMode(con_in, ENABLE_WINDOW_INPUT))
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
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

// Acerta variáveis
    LerPont = 0;
    LerTexto[0] = 0;
    ini_linha = false;
#endif

    Aberto = true;
    return true;
}

//---------------------------------------------------------------------------
void TConsole::Fim()
{
    if (!Aberto)
        return;
    Aberto = false;
#ifdef __WIN32__
    FreeConsole();
#else
    printf("\x1B[0m\n");
    if (!Editor)
        putchar('\n');
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
        if (LerEventos[LerCont].EventType != KEY_EVENT ||
            LerEventos[LerCont].Event.KeyEvent.bKeyDown == 0)
        {
            LerCont++;
            continue;
        }
        unsigned char ch = LerEventos[LerCont].Event.KeyEvent.uChar.AsciiChar;
        LerTexto[0] = StrLer[ch];
        LerTexto[1] = 0;
        if ((unsigned char)LerTexto[0] >= ' ')
        {
            LerCont++;
            return LerTexto;
        }
        switch (LerEventos[LerCont++].Event.KeyEvent.wVirtualKeyCode)
        {
        case VK_BACK:   return "BACK";
        case VK_TAB:    return "TAB";
        case VK_RETURN: return "ENTER";
        case VK_ESCAPE: return "ESC";
        case VK_PRIOR:  return "PGUP";
        case VK_NEXT:   return "PGDN";
        case VK_END:    return "END";
        case VK_HOME:   return "HOME";
        case VK_LEFT:   return "LEFT";
        case VK_UP:     return "UP";
        case VK_RIGHT:  return "RIGHT";
        case VK_DOWN:   return "DOWN";
        case VK_INSERT: return "INS";
        case VK_DELETE: return "DEL";
        case VK_F1:     return "F1";
        case VK_F2:     return "F2";
        case VK_F3:     return "F3";
        case VK_F4:     return "F4";
        case VK_F5:     return "F5";
        case VK_F6:     return "F6";
        case VK_F7:     return "F7";
        case VK_F8:     return "F8";
        case VK_F9:     return "F9";
        case VK_F10:    return "F10";
        case VK_F11:    return "F11";
        case VK_F12:    return "F12";
        }
    }

#else
/*
    int ch = getc(stdin);
    if (ch<0) return 0;
    if (ch==3) return "BREAK";
    if (ch<' ') sprintf(LerTexto, "[%d]", ch);
    else sprintf(LerTexto, "%c[%d]", ch, ch);
    return LerTexto;
*/
    while (true)
    {
    // Lê próxima tecla
        int ch;
        if (LerTexto[0]==0)
            ch = getc(stdin);
        else
            ch = (unsigned char)LerTexto[0], LerTexto[0] = 0;
        if (ch <= 0)
        {
            if (LerPont!=1)
                break;
            LerPont=0;
            return "ESC";
        }
        if (ch==3) return "BREAK";
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
            if (ch==9)  return "TAB";
            if (ch==13) return "ENTER";
            if (ch==27) LerPont=1;
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
        if (ch < 'A')
            continue;
        LerTexto[LerPont] = 0;
        LerPont = 0;
        //Escrever(LerTexto+1);
    // Obtém o nome da tecla a partir da string

    // Códigos começam com ESC + [ ou ESC + O e terminam com caracter >= A
    // Após o ESC, obrigatoriamente em ordem alfabética:
        const char * cod_esc[] = {
            "OP\0" "F1",
            "OQ\0" "F2",
            "OR\0" "F3",
            "OS\0" "F4",
            "[15~\0" "F5",
            "[17~\0" "F6",
            "[18~\0" "F7",
            "[19~\0" "F8",
            "[20~\0" "F9",
            "[21~\0" "F10",
            "[23~\0" "F11",
            "[24~\0" "F12",
            "[2~\0" "INS",
            "[3~\0" "DEL",
            "[5~\0" "PGUP",
            "[6~\0" "PGDN",
            "[A\0" "UP",
            "[B\0" "DOWN",
            "[C\0" "RIGHT",
            "[D\0" "LEFT",
            "[F\0" "END",
            "[H\0" "HOME" };
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
    char mens[1024];
    DWORD cCharsWritten;
    Flush();
    while (tamanho>0)
    {
        char * dest = mens;
        int total = (tamanho > (int)sizeof(mens) ? sizeof(mens) : tamanho);
        tamanho -= total;
        for (; total>0; total--,texto++)
        {
            if (*texto == '\n')
            {
                if (ini_linha)
                {
                    ini_linha=false;
                    if (ColAtual==0)
                        continue;
                }
                LinAtual++, ColAtual = 0;
            }
            else if (++ColAtual >= ColTotal)
                ColAtual = 0, LinAtual++, ini_linha=true;
            *dest++ = StrConv[ *(unsigned char*)texto ];
        }
        WriteFile(con_out, mens, dest-mens, &cCharsWritten, NULL);
    }
#else
    fwrite(texto, 1, tamanho, stdout);
    for (; tamanho>0; tamanho--,texto++)
    {
        //putchar(*texto);
        if (*texto == '\n')
        {
            if (ini_linha)
            {
                ini_linha=false;
                if (ColAtual==0)
                    continue;
            }
            LinAtual++, ColAtual = 0;
        }
        else if (++ColAtual >= ColTotal)
            ColAtual = 0, LinAtual++, ini_linha=true;
    }
#endif
    if (LinAtual >= LinTotal)
        LinAtual = LinTotal - 1;
}

//---------------------------------------------------------------------------
void TConsole::CorTxt(int novacor)
{
#ifdef __WIN32__
    WORD atributos = 0;
    CorAtual = novacor;
    if (novacor&1) atributos |= BACKGROUND_RED;
    if (novacor&2) atributos |= BACKGROUND_GREEN;
    if (novacor&4) atributos |= BACKGROUND_BLUE;
    if (novacor&16) atributos |= FOREGROUND_RED;
    if (novacor&32) atributos |= FOREGROUND_GREEN;
    if (novacor&64) atributos |= FOREGROUND_BLUE;
    if (novacor&128) atributos |= FOREGROUND_INTENSITY;
    SetConsoleTextAttribute(con_out, atributos);
    CorAtributos = atributos;
#else
    char mens[32];
    char * destino = mens;
    char ini='[';
    int antes = CorAtual;
    CorAtual = novacor;
    //sprintf(destino, "%02x", cor); destino+=2;
    *destino++ = 0x1B;
                        // Tirar negrito
    if ((antes&0x80) && (novacor&0x80)==0)
    {
        destino[0] = ini;
        destino[1] = '0';
        destino+=2, ini=';';
        if ((antes&0x70)!=0x70) // Se frente != branco
        {                       // Deve acertar frente
            antes = ((antes&15) | (novacor&0xF0)) ^ 0x70;
        }
        if (antes&15)    // Se fundo != preto
            antes |= 15; // Deve acertar fundo
    }
    else if ((antes&0x80)==0 && (novacor&0x80)) // Negrito
    {
        destino[0] = ini;
        destino[1] = '1';
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
    printf(mens);
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
    ini_linha = false;
}

//---------------------------------------------------------------------------
void TConsole::CursorLin(int valor)
{
    if (valor==0)
        return;
#ifdef __WIN32__
    MoverCursor = true;
#else
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
    ini_linha = false;
}

//---------------------------------------------------------------------------
void TConsole::CursorCol(int valor)
{
    if (valor==0)
        return;
#ifdef __WIN32__
    MoverCursor = true;
#else
    if (valor>0)
        printf("\x1B[%dC", valor);
    else
        printf("\x1B[%dD", -valor);
#endif
    ColAtual += valor;
    if (ColAtual < 0)
        ColAtual = 0;
    if (ColAtual >= LinTotal)
        ColAtual = ColTotal-1;
    ini_linha = false;
}

//---------------------------------------------------------------------------
void TConsole::CursorPosic(int lin, int col)
{
    LinAtual = (lin<0 ? 0 : lin>=(int)LinTotal ? LinTotal-1 : lin);
    ColAtual = (col<0 ? 0 : col>=(int)ColTotal ? ColTotal-1 : col);
#ifdef __WIN32__
    MoverCursor = true;
#else
    printf("\x1B[%d;%dH", LinAtual+1, ColAtual+1);
#endif
    ini_linha = false;
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
    printf("\x1B[%dP", valor);
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
    printf("\x1B[2J");
#endif
}

//---------------------------------------------------------------------------
void TConsole::Escrever(const char * texto, int tamanho)
{
    if (Editor)
    {
        Editor = false;
        CursorLin(-1);      // Cursor sobe uma linha
        CursorIni();        // Cursor no início da linha
        if (ColEscreve && ColEscreve < ColTotal)
            CursorCol(ColEscreve); // Move o cursor para a direita
    }
    if (CorAtual != CorTela)
        CorTxt(CorTela);
    if (tamanho<0)
        tamanho = strlen(texto);
    for (; tamanho>0; texto++,tamanho--)
    {
        if (ColEscreve >= ColTotal)
        {
            EnvTxt("\n\n", 2);  // Dois '\n' para criar uma linha no final
            CursorLin(-1);      // Cursor sobe uma linha
            InsereLin(1);       // Insere uma linha
            ColEscreve = 0;
        }
        if (*texto=='\n')
            ColEscreve = 0xFFFF;
        else
        {
            EnvTxt(texto, 1);
            ColEscreve++;
        }
    }
}

//---------------------------------------------------------------------------
void TConsole::ProcTecla(const char * texto)
{
    if (texto==0)
        return;
// Cursor na linha de edição
    if (!Editor)
    {
        Editor = true;
        EnvTxt("\n", 1);
        if (ColEditor)
            CursorCol(ColEditor);
        if (CorAtual != CONSOLE_COR_LINHA)
            CorTxt(CONSOLE_COR_LINHA);
    }
// Tecla normal (texto)
    if (texto[0] && texto[1]==0)
    {
    // Verifica se tem espaço na linha
        if (tam_linha >= CONSOLE_MAX)
            return;
    // Altera a linha
        tam_linha++;
        char antes = texto[0];
        for (unsigned int x=col_linha + ColEditor; x<tam_linha; x++)
        {
            char valor = linha[x];
            linha[x] = antes;
            antes = valor;
        }
    // Não está no fim da linha na tela
        if (ColEditor < ColTotal-2)
        {
            ColEditor++;
            if (col_linha + ColEditor < tam_linha)
                InsereCol(1);
        }
        else
    // Está no fim da linha na tela
        {
            col_linha++;
            CursorIni(); // Cursor no início da linha
            ApagaCol(1); // Apaga 1 caracter
            CursorCol(ColEditor - 1); // Posiciona cursor
        }
        EnvTxt(linha + col_linha + ColEditor - 1, 1); // Mostra caracter
        return;
    }
// Esquerda
    if (strcmp(texto, "LEFT")==0)
    {
        if (ColEditor>0)
        {
            ColEditor--;
            CursorCol(-1);  // Recua cursor
        }
        else if (col_linha>0)
        {
            col_linha--;
            InsereCol(1);
            EnvTxt(linha + col_linha, 1); // Mostra caracter
            CursorCol(-1);          // Cursor no início da linha
        }
        return;
    }
// Direita
    if (strcmp(texto, "RIGHT")==0)
    {
        if (ColEditor + col_linha >= tam_linha)
            return;
        if (ColEditor < ColTotal-2)
        {
            ColEditor++;
            CursorCol(1);  // Recua cursor
            return;
        }
        col_linha++;
        CursorIni(); // Cursor no início da linha
        ApagaCol(1); // Apaga 1 caracter
        CursorCol(ColEditor); // Posiciona cursor
        if (col_linha + ColEditor >= tam_linha)
            return;
        EnvTxt(linha + col_linha + ColEditor, 1); // Mostra caracter
        CursorCol(-1); // Volta cursor
        return;
    }
// DEL
    if (strcmp(texto, "DEL")==0)
    {
        if (col_linha + ColEditor >= tam_linha)
            return;
    // Altera a linha
        tam_linha--;
        for (unsigned int x=col_linha + ColEditor; x<tam_linha; x++)
            linha[x] = linha[x+1];
    // Resultado na tela
        ApagaCol(1); // Apaga 1 caracter
        if (col_linha + ColTotal - 2 >= tam_linha)
            return;
        int move = ColTotal - ColEditor - 2;
        CursorCol(move);
        EnvTxt(linha + col_linha + ColTotal - 2, 1); // Mostra caracter
        LimpaFim(); // Limpa da posição do cursor até o final
        CursorCol(-move-1);
        return;
    }
// Backspace
    if (strcmp(texto, "BACK")==0)
    {
        if (col_linha + ColEditor <= 0)
            return;
    // Altera a linha
        tam_linha--;
        for (unsigned int x=col_linha + ColEditor - 1; x<tam_linha; x++)
            linha[x] = linha[x+1];
    // Resultado na tela
        if (ColEditor==0)
        {
            col_linha--;
            return;
        }
        ColEditor--;
        CursorCol(-1);  // Recua cursor
        ApagaCol(1); // Apaga 1 caracter
        if (col_linha + ColTotal - 2 >= tam_linha)
            return;
        int move = ColTotal - ColEditor - 2;
        CursorCol(move);
        EnvTxt(linha + col_linha + ColTotal - 2, 1); // Mostra caracter
        LimpaFim(); // Limpa da posição do cursor até o final
        CursorCol(-move-1);
        return;
    }
// ENTER
    if (strcmp(texto, "ENTER")==0)
    {
        col_linha = 0;
        tam_linha = 0;
        ColEditor = 0;
        CursorIni();  // Posicina o cursor no início da linha
        LimpaFim();   // Limpa a linha
        return;
    }
}

//---------------------------------------------------------------------------
void TConsole::ProcFim()
{
    if (!Editor)
    {
        Editor = true;
        EnvTxt("\n", 1);
        if (ColAtual)
            CursorCol(ColAtual);
        if (CorAtual != CONSOLE_COR_LINHA)
            CorTxt(CONSOLE_COR_LINHA);
    }
    Flush();
}

//---------------------------------------------------------------------------
void TConsole::ProcLimpa()
{
    CorTxt(0x70);
    LimpaTela();
    CursorPosic(1,0);
    CorTxt(CONSOLE_COR_LINHA);
    LimpaFim();  // Preenche do cursor até o fim da linha
    Editor = true;
    ColEscreve = 0;
    int total = tam_linha - col_linha;
    if (total > 0)
        EnvTxt(linha + col_linha, total<(int)ColTotal-1 ? total : ColTotal-1);
    CursorCol(ColEditor - ColAtual);
}

//------------------------------------------------------------------------------
void TVarTelaTxt::Criar()
{
    Antes = 0;
    Depois = Inicio;
    if (Inicio)
        Inicio->Antes = this;
    Inicio = this;
}

//------------------------------------------------------------------------------
void TVarTelaTxt::Apagar()
{
    (Antes ? Antes->Depois : Inicio) = Depois;
    if (Depois)
        Depois->Antes = Antes;
    if (ObjAtual==this)
        ObjAtual=Depois;
}

//------------------------------------------------------------------------------
void TVarTelaTxt::Mover(TVarTelaTxt * destino)
{
    (Antes ? Antes->Depois : Inicio) = destino;
    if (Depois)
        Depois->Antes = destino;
    move_mem(destino, this, sizeof(TVarTelaTxt));
}

//------------------------------------------------------------------------------
void TVarTelaTxt::EndObjeto(TClasse * c, TObjeto * o)
{
    if (o)
        endobjeto=o, b_objeto=true;
    else
        endclasse=c, b_objeto=false;
}

//------------------------------------------------------------------------------
int TVarTelaTxt::getValor()
{
    return 0;
}

//------------------------------------------------------------------------------
bool TVarTelaTxt::Inic()
{
    if (Console==0)
        return true;
    if (!Console->Inic())
        return false;
// Acerta linha de edição
    Console->EnvTxt("\n", 1);    // Adiciona uma linha
    Console->CorTxt(CONSOLE_COR_LINHA); // Define frente branca e fundo azul
    Console->LimpaFim();         // Preenche do cursor até o fim da linha
// Acerta o título da janela
    int total = arqext - arqinicio;
    if (total>0)
    {
        char mens[100];
        if (total > (int)sizeof(mens)-1)
            total = (int)sizeof(mens)-1;
        memcpy(mens, arqinicio, total);
        mens[total] = 0;
        Console->MudaTitulo(mens);
    }
    return true;
}

//------------------------------------------------------------------------------
void TVarTelaTxt::Processa()
{
    if (Console==0)
        return;
    while (true)
    {
    // Lê tecla
        const char * p = Console->LerTecla();
        if (p==0)
            break;
        //Console->Escrever(p);
    // Chama evento que processa tecla
        if (FuncEvento("tecla", p))
            continue;
    // Processa teclas exceto ENTER
        if (strcmp(p, "ENTER")!=0)
        {
            Console->ProcTecla(p);
            continue;
        }
    // Processa enter
        char mens[2048];
        strcpy(mens, Console->LerLinha());
        Console->ProcTecla(p);
        FuncEvento("msg", mens);
    }
}

//------------------------------------------------------------------------------
void TVarTelaTxt::ProcFim()
{
    if (Console)
        Console->ProcFim();
}

//------------------------------------------------------------------------------
bool TVarTelaTxt::FuncEvento(const char * evento, const char * texto)
{
    if (Console==0)
        return false;
    //printf("FuncEvento [%s] [%s]\n", evento, texto); fflush(stdout);
    for (TVarTelaTxt * vobj = Inicio; vobj;)
    {
        bool prossegue = false;
    // Definido em objeto: prepara para executar
        if (vobj->b_objeto)
        {
            char mens[80];
            sprintf(mens, "%s_%s", vobj->defvar+Instr::endNome, evento);
            prossegue = Instr::ExecIni(vobj->endobjeto, mens);
        }
    // Definido em classe: prepara para executar
        else if (vobj->endclasse)
        {
            char mens[80];
            sprintf(mens, "%s_%s", vobj->defvar+Instr::endNome, evento);
            prossegue = Instr::ExecIni(vobj->endclasse, mens);
        }
    // Executa
        ObjAtual = vobj->Depois;
        if (prossegue)
        {
            if (texto)
                Instr::ExecArg(texto);
            Instr::ExecArg(vobj->indice);
            if (Instr::ExecX())
                if (Instr::VarAtual >= Instr::VarPilha)
                    prossegue = !Instr::VarAtual->getBool();
            Instr::ExecFim();
            if (!prossegue)
                return true;
        }
    // Passa para próximo objeto
        vobj = ObjAtual;
    } // for (TVarSocket ...
    return false;
}

//------------------------------------------------------------------------------
bool TVarTelaTxt::Func(TVariavel * v, const char * nome)
{
// Envia mensagem
    if (comparaZ(nome, "msg")==0)
    {
        if (Console==0)
            return false;
        for (TVariavel * obj=v+1; obj<=Instr::VarAtual; obj++)
        {
            const char * texto = obj->getTxt();
            while (true)
            {
                const char * p = texto;
                while (*(unsigned char*)p >= ' ')
                    p++;
                if (p != texto)
                {
                    Console->Escrever(texto, p-texto);
                    texto = p;
                }
                if (*texto==0)
                    break;
                switch (*texto++)
                {
                case Instr::ex_barra_n:
                    Console->Escrever("\n", 1);
                    break;
                case Instr::ex_barra_b:
                    Console->CorTela = 0x70;
                    break;
                case Instr::ex_barra_c:
                    if (*texto>='0' && *texto<='9')
                    {
                        Console->CorTela = (Console->CorTela & 0x0F) +
                                            (*texto - '0') * 0x10;
                        p++;
                    }
                    else if ((*texto>='A' && *texto<='F') ||
                        (*texto>='a' && *texto<='f'))
                    {
                        Console->CorTela = (Console->CorTela & 0x0F) +
                                            ((*texto|0x20) - '7') * 0x10;
                        p++;
                    }
                    break;
                case Instr::ex_barra_d:
                    if (*texto>='0' && *texto<='7')
                    {
                        Console->CorTela = (Console->CorTela & 0xF0) +
                                            (*texto - '0');
                        p++;
                    }
                } // switch
            } // while
        } // for
        return false;
    }
// Posição do cursor
    if (comparaZ(nome, "posx")==0)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(Console ? Console->ColEscreve : 0);
    }
// Processa tecla
    if (comparaZ(nome, "tecla")==0)
    {
        if (Console==0)
            return false;
        for (TVariavel * obj=v+1; obj<=Instr::VarAtual; obj++)
        {
            const char * texto = obj->getTxt();
            if (*texto)
                Console->ProcTecla(texto);
        }
        return false;
    }
// Variável proto
    if (comparaZ(nome, "proto")==0)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(Console ? 5 : 0);
    }
// Gerar um bipe
    if (comparaZ(nome, "bipe")==0)
    {
        if (Console)
            Console->Beep();
        return false;
    }
// Limpa a tela
    if (comparaZ(nome, "limpa")==0)
    {
        if (Console)
            Console->ProcLimpa();
        return false;
    }

    return false;
}
