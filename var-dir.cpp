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
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#ifdef __WIN32__
 #include <windows.h>
#endif
#include "var-dir.h"
#include "variavel.h"
#include "instr.h"
#include "misc.h"

#ifdef __WIN32__
 #define DIR_VALIDO (wdir != INVALID_HANDLE_VALUE)
#else
 #define DIR_VALIDO (dir != nullptr)
#endif

//------------------------------------------------------------------------------
void TVarDir::Criar()
{
#ifdef __WIN32__
    wdir = INVALID_HANDLE_VALUE;
#else
    dir = nullptr;
#endif
}

//------------------------------------------------------------------------------
void TVarDir::Apagar()
{
#ifdef __WIN32__
    if (wdir != INVALID_HANDLE_VALUE)
        FindClose(wdir);
    wdir = INVALID_HANDLE_VALUE;
#else
    if (dir)
        closedir(dir);
    dir = nullptr;
#endif
}


//------------------------------------------------------------------------------
bool TVarDir::Func(TVariavel * v, const char * nome)
{
// Lista das funções de varmem
// Deve obrigatoriamente estar em letras minúsculas e ordem alfabética
    static const struct {
        const char * Nome;
        bool (TVarDir::*Func)(TVariavel * v); } ExecFunc[] = {
        { "abrir",        &TVarDir::FuncAbrir },
        { "apagar",       &TVarDir::FuncApagar },
        { "apagardir",    &TVarDir::FuncApagarDir },
        { "atempo",       &TVarDir::FuncAtempo },
        { "barra",        &TVarDir::FuncBarra },
        { "criardir",     &TVarDir::FuncCriarDir },
        { "depois",       &TVarDir::FuncDepois },
        { "fechar",       &TVarDir::FuncFechar },
        { "lin",          &TVarDir::FuncLin },
        { "mtempo",       &TVarDir::FuncMtempo },
        { "renomear",     &TVarDir::FuncRenomear },
        { "tamanho",      &TVarDir::FuncTamanho },
        { "texto",        &TVarDir::FuncTexto },
        { "tipo",         &TVarDir::FuncTipo } };
// Procura a função correspondente e executa
    int ini = 0;
    int fim = sizeof(ExecFunc) / sizeof(ExecFunc[0]) - 1;
    char mens[80];
    copiastrmin(mens, nome, sizeof(mens));
    while (ini <= fim)
    {
        int meio = (ini + fim) / 2;
        int resultado = strcmp(mens, ExecFunc[meio].Nome);
        if (resultado == 0) // Se encontrou...
            return (this->*ExecFunc[meio].Func)(v);
        if (resultado < 0) fim = meio - 1; else ini = meio + 1;
    }
    return false;
}

//------------------------------------------------------------------------------
bool TVarDir::FuncLin(TVariavel * v)
{
    bool b = DIR_VALIDO;
    Instr::ApagarVar(v);
    return Instr::CriarVarInt(b);
}

//------------------------------------------------------------------------------
bool TVarDir::FuncTexto(TVariavel * v)
{
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(DIR_VALIDO ? arqdir : "");
}

//------------------------------------------------------------------------------
bool TVarDir::FuncDepois(TVariavel * v)
{
    Proximo();
    return false;
}

//------------------------------------------------------------------------------
bool TVarDir::FuncBarra(TVariavel * v)
{
    const char * txt = "";  // Texto
    char mens[BUF_MENS];    // Resultado
    char * destino = mens;
    if (Instr::VarAtual >= v + 1)
        txt = v[1].getTxt();
    while (destino < mens+sizeof(mens) - 1)
    {
        char ch = *txt++;
        if (ch == 0)
            break;
#ifdef __WIN32__
        if (ch == '/')
            ch = '\\';
#else
        if (ch == '\\')
            ch = '/';
#endif
        *destino++ = ch;
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(mens, destino-mens);
}

//------------------------------------------------------------------------------
// Abrir/Fechar diretório
bool TVarDir::FuncAbrir(TVariavel * v)
{
    char mens[512];
// Fecha diretório
    Apagar();
// Obtém nome do diretório
    *mens = 0;
    if (Instr::VarAtual >= v + 1)
        copiastr(mens, v[1].getTxt(), sizeof(mens) - 2);
    if (*mens == 0)
        strcpy(mens, ".");
// Checa se nome válido
    if (!arqvalido(mens))
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto("Nome não permitido");
    }
// Abre o diretório
#ifdef __WIN32__
    WIN32_FIND_DATA ffd;
    strcat(mens, "\\*");
    wdir = FindFirstFile(mens, &ffd);
    if (wdir == INVALID_HANDLE_VALUE)
        return Instr::CriarVarTexto("");
    while (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0)
        if (FindNextFile(wdir, &ffd) == 0)
        {
            FindClose(wdir);
            wdir = INVALID_HANDLE_VALUE;
            return Instr::CriarVarTexto("");
        }
    copiastr(arqdir, ffd.cFileName, sizeof(arqdir));
    arqtipo = (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                ? 'D' : 'A';
#else
    dir = opendir(mens);
    if (dir == nullptr)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(strerror(errno));
    }
    Proximo();
#endif
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto("");
}

//------------------------------------------------------------------------------
bool TVarDir::FuncFechar(TVariavel * v)
{
    Apagar();
    return false;
}

//------------------------------------------------------------------------------
// Atributos do arquivo
bool TVarDir::FuncTipo(TVariavel * v)
{
// Sem argumentos: entrada encontrada em abrir()
    if (Instr::VarAtual < v + 1)
    {
        char txt[2] = "?";
        if (DIR_VALIDO)
            txt[0] = arqtipo;
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(txt);
    }
// Com argumento
    if (Instr::VarAtual < v + 1)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto("?");
    }
    struct stat buf;
    char mens[512];
    copiastr(mens, v[1].getTxt(), sizeof(mens));
    if (!arqvalido(mens)) // Não permitido
        mens[0] = '?';
    else if (stat(mens, &buf) < 0) // Não existe
        mens[0] = '?';
    else if (S_ISDIR(buf.st_mode)) // Diretório
        mens[0] = 'D';
    else if (S_ISREG(buf.st_mode)) // Arquivo normal
        mens[0] = 'A';
    else
        mens[0] = 'O';
    mens[1] = 0;
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(mens);
}

//------------------------------------------------------------------------------
bool TVarDir::FuncTamanho(TVariavel * v)
{
    char mens[512];
    double tam = 0;
    while (Instr::VarAtual >= v+1)
    {
        copiastr(mens, v[1].getTxt(), sizeof(mens));
        if (!arqvalido(mens))
            break;
#ifdef __WIN32__
        HANDLE hFile = CreateFile(mens, // file to open
                GENERIC_READ,     // open for reading
                FILE_SHARE_READ,  // share for reading
                NULL,             // default security
                OPEN_EXISTING,    // existing file only
                FILE_ATTRIBUTE_NORMAL, // normal file
                NULL);            // no attribute template
        if (hFile == INVALID_HANDLE_VALUE)
            break;
        LARGE_INTEGER lFileSize;
        if (GetFileSizeEx(hFile, &lFileSize))
            tam = lFileSize.QuadPart;
        CloseHandle(hFile);
#else
        struct stat buf;
        if (stat(mens, &buf) >= 0)
            tam = buf.st_size;
#endif
        break;
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarDouble(tam);
}

//------------------------------------------------------------------------------
/// Obtém data e hora de um arquivo
/** @param nomearq Nome do arquivo
 *  @param buffer Aonde colocar data e hora, se conseguiu obter
 *  @param tipo 1=mtempo, 2=atempo
 *  @note Se não conseguir obter data e hora, não altera o conteúdo de buffer */
static inline void VarDirObtemTempo(char * nomearq, char * buffer, int tipo)
{
    if (!arqvalido(nomearq))
        return;
#ifdef __WIN32__
    FILETIME ftCreate, ftAccess, ftWrite;
    SYSTEMTIME stUTC, stLocal;
    HANDLE hFile = CreateFile(nomearq, // file to open
            GENERIC_READ,     // open for reading
            FILE_SHARE_READ,  // share for reading
            NULL,             // default security
            OPEN_EXISTING,    // existing file only
            FILE_ATTRIBUTE_NORMAL, // normal file
            NULL);            // no attribute template
    if (hFile == INVALID_HANDLE_VALUE)
        return;
    if (GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
    {
        if (tipo == 1)
            FileTimeToSystemTime(&ftWrite, &stUTC);
        else
            FileTimeToSystemTime(&ftAccess, &stUTC);
        SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
        sprintf(buffer, "%d %d %d %d %d %d",
                stLocal.wYear, stLocal.wMonth, stLocal.wDay,
                stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
    }
    CloseHandle(hFile);
#else
    struct stat buf;
    if (stat(nomearq, &buf) < 0)
        return;
    time_t tempoatual;
    struct tm * tempolocal;
    if (tipo == 1)    // mtempo
        tempoatual = buf.st_mtime;
    else            // atempo
        tempoatual = buf.st_atime;
    // localtime() Converte para representação local de tempo
    tempolocal = localtime(&tempoatual);
    sprintf(buffer, "%d %d %d %d %d %d",
            tempolocal->tm_year + 1900, // Ano começa no 1900
            tempolocal->tm_mon + 1, // Mês começa no 1
            tempolocal->tm_mday, // Dia começa no 0
            tempolocal->tm_hour,
            tempolocal->tm_min,
            tempolocal->tm_sec);
#endif
}

//------------------------------------------------------------------------------
bool TVarDir::FuncMtempo(TVariavel * v)
{
    char buffer[256] = "";
    char nomearq[1024];
    if (Instr::VarAtual >= v + 1)
    {
        copiastr(nomearq, v[1].getTxt(), sizeof(nomearq));
        VarDirObtemTempo(nomearq, buffer, 1);
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(buffer);
}

//------------------------------------------------------------------------------
bool TVarDir::FuncAtempo(TVariavel * v)
{
    char buffer[256] = "";
    char nomearq[1024];
    if (Instr::VarAtual >= v + 1)
    {
        copiastr(nomearq, v[1].getTxt(), sizeof(nomearq));
        VarDirObtemTempo(nomearq, buffer, 2);
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(buffer);
}

//------------------------------------------------------------------------------
bool TVarDir::FuncApagarDir(TVariavel * v)
{
    char mens[512];
    if (Instr::VarAtual < v+1)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto("");
    }
    copiastr(mens, v[1].getTxt(), sizeof(mens));
    Instr::ApagarVar(v);
    if (!arqvalido(mens))
        return Instr::CriarVarTexto("Nome de diretório não permitido");
    int err = rmdir(mens);
    if (err >= 0)
        *mens = 0;
    else
        copiastr(mens, strerror(errno), sizeof(mens));
    return Instr::CriarVarTexto(mens);
}

//------------------------------------------------------------------------------
bool TVarDir::FuncCriarDir(TVariavel * v)
{
    char mens[512];
    if (Instr::VarAtual < v+1)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto("");
    }
    copiastr(mens, v[1].getTxt(), sizeof(mens));
    Instr::ApagarVar(v);
    if (!arqvalido(mens))
        return Instr::CriarVarTexto("Nome de diretório não permitido");
#ifdef __WIN32__
    int err = mkdir(mens);
#else
    int err = mkdir(mens, S_IRWXU|S_IRWXG|S_IRWXO);
#endif
    if (err >= 0)
        *mens = 0;
    else
        copiastr(mens, strerror(errno), sizeof(mens));
    return Instr::CriarVarTexto(mens);
}

//------------------------------------------------------------------------------
// Apgar arquivo
bool TVarDir::FuncApagar(TVariavel * v)
{
    char mens[512];
    if (Instr::VarAtual < v+1)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto("");
    }
    copiastr(mens, v[1].getTxt(), sizeof(mens));
    Instr::ApagarVar(v);
    if (!arqvalido(mens, false))
        return Instr::CriarVarTexto("Nome de arquivo não permitido");
    if (remove(mens) >= 0) // remove ou unlink
        *mens = 0;
    else
        copiastr(mens, strerror(errno), sizeof(mens));
    return Instr::CriarVarTexto(mens);
}

//------------------------------------------------------------------------------
// Renomear arquivo/diretório
bool TVarDir::FuncRenomear(TVariavel * v)
{
    char antes[512], depois[512];
    if (Instr::VarAtual < v + 2)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto("");
    }
    copiastr(antes, v[1].getTxt(), sizeof(antes));
    copiastr(depois, v[2].getTxt(), sizeof(depois));
    Instr::ApagarVar(v);
    if (!arqvalido(antes, true) || !arqvalido(depois, true))
        return Instr::CriarVarTexto("Nome de arquivo não permitido");
#ifdef __WIN32__
    if (MoveFileEx(antes, depois,
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED))
#else
    if (rename(antes, depois) >= 0)
#endif
        *antes = 0;
    else
        copiastr(antes, strerror(errno), sizeof(antes));
    return Instr::CriarVarTexto(antes);
}

//------------------------------------------------------------------------------
void TVarDir::Proximo()
{
#ifdef __WIN32__
    WIN32_FIND_DATA ffd;
    if (wdir == INVALID_HANDLE_VALUE)
        return;
    while (FindNextFile(wdir, &ffd) != 0)
    {
        if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0)
            continue;
        copiastr(arqdir, ffd.cFileName, sizeof(arqdir));
        arqtipo = (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    ? 'D' : 'A';
        return;
    }
    FindClose(wdir);
    wdir = INVALID_HANDLE_VALUE;
#else
    if (dir == nullptr)
        return;
    while (true)
    {
        struct dirent * sdir = readdir(dir);
        if (sdir == nullptr)
            break;
        if (strcmp(sdir->d_name, ".") == 0 || strcmp(sdir->d_name, "..") == 0)
            continue;
        copiastr(arqdir, sdir->d_name, sizeof(arqdir));
        switch (sdir->d_type)
        {
        case DT_REG: arqtipo = 'A'; break;
        case DT_DIR: arqtipo = 'D'; break;
        default: arqtipo = 'O';
        }
        return;
    }
    closedir(dir);
    dir = nullptr;
#endif
}
