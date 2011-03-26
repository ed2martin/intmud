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
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include "var-dir.h"
#include "variavel.h"
#include "instr.h"
#include "misc.h"

#ifdef __WIN32__
 #define DIR_VALIDO (wdir!=INVALID_HANDLE_VALUE)
#else
 #define DIR_VALIDO (dir!=0)
#endif

//------------------------------------------------------------------------------
void TVarDir::Criar()
{
#ifdef __WIN32__
    wdir = INVALID_HANDLE_VALUE;
#else
    dir = 0;
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
    dir = 0;
#endif
}

//------------------------------------------------------------------------------
bool TVarDir::Func(TVariavel * v, const char * nome)
{
// Pesquisar entrada atual no diretório
    if (comparaZ(nome, "lin")==0)
    {
        bool b = DIR_VALIDO;
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(b);
    }
    if (comparaZ(nome, "texto")==0)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(DIR_VALIDO ? arqdir : "");
    }
    if (comparaZ(nome, "depois")==0)
    {
        Proximo();
        return false;
    }

// Abrir/Fechar diretório
    if (comparaZ(nome, "abrir")==0)
    {
        char mens[512];
    // Fecha diretório
        Apagar();
    // Obtém nome do diretório
        *mens = 0;
        if (Instr::VarAtual >= v+1)
            copiastr(mens, v[1].getTxt(), sizeof(mens)-2);
        if (*mens==0)
            strcpy(mens, ".");
    // Checa se nome válido
        if (!arqvalido(mens, ""))
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
        while (strcmp(ffd.cFileName, ".")==0 || strcmp(ffd.cFileName, "..")==0)
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
        if (dir==0)
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(strerror(errno));
        }
        Proximo();
#endif
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto("");
    }
    if (comparaZ(nome, "fechar")==0)
    {
        Apagar();
        return false;
    }

// Atributos do arquivo
    int tipo = 0;
    if (comparaZ(nome, "tipo")==0)
    {
    // Sem argumentos: entrada encontrada em abrir()
        if (Instr::VarAtual < v+1)
        {
            char txt[2] = "?";
            if (DIR_VALIDO)
                txt[0] = arqtipo;
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(txt);
        }
   // Com argumento
        tipo = 1;
    }
    else if (comparaZ(nome, "tamanho")==0)
        tipo = 2;
    else if (comparaZ(nome, "mtempo")==0)
        tipo = 3;
    else if (comparaZ(nome, "atempo")==0)
        tipo = 4;
    if (tipo)
    {
        struct stat buf;
        char mens[512];
        int existe = 0; // 0=não existe, 1=não permitido, 2=existe
        if (Instr::VarAtual >= v+1)
        {
            copiastr(mens, v[1].getTxt(), sizeof(mens));
            if (!arqvalido(mens, ""))
                existe = 1;
            else if (stat(mens, &buf) >= 0)
                existe = 2;
        }
        Instr::ApagarVar(v);
        if (tipo==1) // Tipo
        {
            if (existe != 2) // Não existe ou não permitido
                mens[0] = '?';
            else if (S_ISDIR(buf.st_mode)) // Diretório
                mens[0] = 'D';
            else if (S_ISREG(buf.st_mode)) // Arquivo normal
                mens[0] = 'A';
            else
                mens[0] = 'O';
            mens[1] = 0;
            return Instr::CriarVarTexto(mens);
        }
        if (tipo==2) // Tamanho
        {
            if (!Instr::CriarVar(Instr::InstrDouble))
                return false;
            Instr::VarAtual->setDouble(existe==2 ? buf.st_size : 0);
            return true;
        }
        time_t tempoatual;
        struct tm * tempolocal;
        if (existe != 2)
            return Instr::CriarVarTexto("");
        if (tipo==3)    // mtempo
            tempoatual = buf.st_mtime;
        else            // atempo
            tempoatual = buf.st_atime;
        // localtime() Converte para representação local de tempo
        tempolocal=localtime(&tempoatual);
        sprintf(mens, "%d %d %d %d %d %d",
                tempolocal->tm_year + 1900, // Ano começa no 1900
                tempolocal->tm_mon + 1, // Mês começa no 1
                tempolocal->tm_mday, // Dia começa no 0
                tempolocal->tm_hour,
                tempolocal->tm_min,
                tempolocal->tm_sec);
        return Instr::CriarVarTexto(mens);
    }

// Criar/Apagar diretório
    if (comparaZ(nome, "criardir")==0)
        tipo = 1;
    else if (comparaZ(nome, "apagardir")==0)
        tipo = 2;
    if (tipo)
    {
        char mens[512];
        if (Instr::VarAtual < v+1)
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto("");
        }
        copiastr(mens, v[1].getTxt(), sizeof(mens));
        Instr::ApagarVar(v);
        if (!arqvalido(mens, ""))
            return Instr::CriarVarTexto("Nome de diretório não permitido");
        int err = 0;
        if (tipo==1)
#ifdef __WIN32__
            err = mkdir(mens);
#else
            err = mkdir(mens, S_IRWXU|S_IRWXG|S_IRWXO);
#endif
        else
            err = rmdir(mens);
        if (err>=0)
            *mens = 0;
        else
            copiastr(mens, strerror(errno), sizeof(mens));
        return Instr::CriarVarTexto(mens);
    }

// Apgar arquivo
    if (comparaZ(nome, "apagar")==0)
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
            return Instr::CriarVarTexto("Nome de arquivo não permitido");
        if (remove(mens) >= 0) // remove ou unlink
            *mens = 0;
        else
            copiastr(mens, strerror(errno), sizeof(mens));
        return Instr::CriarVarTexto(mens);
    }

// Renomear arquivo/diretório
    if (comparaZ(nome, "renomear")==0)
    {
        char antes[512], depois[512];
        if (Instr::VarAtual < v+2)
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto("");
        }
        copiastr(antes, v[1].getTxt(), sizeof(antes));
        copiastr(depois, v[2].getTxt(), sizeof(depois));
        Instr::ApagarVar(v);
        if (!arqvalido(antes) || !arqvalido(depois))
            return Instr::CriarVarTexto("Nome de arquivo não permitido");
        if (rename(antes, depois) >= 0)
            *antes = 0;
        else
            copiastr(antes, strerror(errno), sizeof(antes));
        return Instr::CriarVarTexto(antes);
    }
    return false;
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
        if (strcmp(ffd.cFileName, ".")==0 || strcmp(ffd.cFileName, "..")==0)
            continue;
        copiastr(arqdir, ffd.cFileName, sizeof(arqdir));
        arqtipo = (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    ? 'D' : 'A';
        return;
    }
    FindClose(wdir);
    wdir = INVALID_HANDLE_VALUE;
#else
    if (dir==0)
        return;
    while (true)
    {
        struct dirent * sdir = readdir(dir);
        if (sdir==0)
            break;
        if (strcmp(sdir->d_name, ".")==0 || strcmp(sdir->d_name, "..")==0)
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
    dir = 0;
#endif
}
