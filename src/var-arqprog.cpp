/* Este arquivo é software livre; você pode redistribuir e/ou
 * modificar nos termos da licença LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "var-arqprog.h"
#include "variavel.h"
#include "arqmapa.h"
#include "instr.h"
#include "misc.h"

char * TArqIncluir::arqnome = nullptr;
TArqIncluir * TArqIncluir::Inicio = nullptr;
TArqIncluir * TArqIncluir::Fim = nullptr;

//----------------------------------------------------------------------------
TArqIncluir::TArqIncluir(const char * nome)
{
    copiastr(Padrao, nome, sizeof(Padrao));
    Anterior = Fim, Proximo = nullptr;
    (Fim ? Fim->Proximo : Inicio) = this;
    Fim = this;
}

//----------------------------------------------------------------------------
TArqIncluir::~TArqIncluir()
{
    (Anterior ? Anterior->Proximo : Inicio) = Proximo;
    (Proximo ? Proximo->Anterior : Fim) = Anterior;
}

//----------------------------------------------------------------------------
bool TArqIncluir::ProcPadrao(const char * nome)
{
    if (*nome == 0)
        return true;
    for (TArqIncluir * obj = Inicio; obj; obj = obj->Proximo)
        if (strcmp(nome, obj->Padrao) == 0)
            return true;
    return false;
}

//----------------------------------------------------------------------------
bool TArqIncluir::ProcArq(const char * nome)
{
    for (TArqIncluir * obj = Inicio; obj; obj=obj->Proximo)
    {
        int tam = strlen(obj->Padrao);
        if (memcmp(nome, obj->Padrao, tam) != 0)
            continue;
        const char * p = nome + tam;
        while (*p && *p != '\\' && *p != '/')
            p++;
        if (*p == 0)
            return true;
    }
    return false;
}

//----------------------------------------------------------------------------
void TArqIncluir::ArqNome(const char * nome)
{
    if (arqnome)
    {
        delete[] arqnome;
        arqnome = nullptr;
    }
    if (nome == nullptr)
        return;
    int tam = strlen(nome) + 1;
    arqnome = new char[tam];
    memcpy(arqnome, nome, tam);
}

//----------------------------------------------------------------------------
const char * TArqIncluir::ArqNome()
{
    return arqnome;
}

//----------------------------------------------------------------------------
const TVarInfo * TVarArqProg::Inicializa()
{
    static TVarInfo::FuncItem ListaFuncEnd[] = {
        { "abrir",        &TVarArqProg::FuncAbrir },
        { "depois",       &TVarArqProg::FuncDepois },
        { "fechar",       &TVarArqProg::FuncFechar },
        { "lin",          &TVarArqProg::FuncLin },
        { "texto",        &TVarArqProg::FuncTexto } };
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        TVarInfo::FTipoOutros,
        FRedim,
        FMoverEnd,
        TVarInfo::FMoverDef0,
        TVarInfo::FGetBoolFalse,
        TVarInfo::FGetInt0,
        TVarInfo::FGetDouble0,
        TVarInfo::FGetTxtVazio,
        TVarInfo::FGetObjNulo,
        TVarInfo::FOperadorAtribVazio,
        TVarInfo::FOperadorAddFalse,
        TVarInfo::FOperadorIgual2Var,
        TVarInfo::FOperadorComparaVar,
        TVarInfo::FFuncTextoFalse,
        TVarInfo::FFuncVetorFalse,
        ListaFuncEnd,
        sizeof(ListaFuncEnd) / sizeof(ListaFuncEnd[0]) - 1);
    return &var;
}

//----------------------------------------------------------------------------
void TVarArqProg::Criar()
{
#ifdef __WIN32__
    wdir = INVALID_HANDLE_VALUE;
#else
    dir = nullptr;
#endif
    Incluir = nullptr;
    *Arquivo = 0;
}

//----------------------------------------------------------------------------
void TVarArqProg::Apagar()
{
    Fechar();
}

//----------------------------------------------------------------------------
void TVarArqProg::Abrir()
{
    Fechar();
    copiastr(Arquivo, TArqIncluir::ArqNome(), sizeof(Arquivo));
    Incluir = TArqIncluir::Inicio;
}

//----------------------------------------------------------------------------
void TVarArqProg::Fechar()
{
    *Arquivo = 0;
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

//----------------------------------------------------------------------------
void TVarArqProg::Proximo()
{
    char mens[ARQINCLUIR_TAM]; // Para procurar arquivos em um diretório
    const char * nomearq = nullptr; // Nome do arquivo encontrado

    while (true)
    {

// Se diretório fechado: começa a busca e obtém a primeira entrada
#ifdef __WIN32__
        if (wdir == INVALID_HANDLE_VALUE)
#else
        if (dir == nullptr)
#endif
        {
        // Se não há próximo diretório: fim da busca
            if (Incluir == nullptr)
            {
                *Arquivo = 0;
                return;
            }
        // Acerta variáveis Arquivo, ArqBarra e ArqPadrao
            copiastr(Arquivo, Incluir->Padrao, sizeof(Arquivo));
            ArqBarra = 0;
            const char * p;
            for (p = Arquivo; *p; p++)
                if (*p == '/' || *p == '\\')
                    ArqBarra = p + 1 - Arquivo;
            ArqPadrao = p - Arquivo;
#ifdef __WIN32__
        // Obtém o nome a procurar
            copiastr(mens, Incluir->Padrao, sizeof(mens) - 1);
            strcat(mens, "*");
            for (char * p = mens; *p; p++)
                if (*p == '/')
                    *p = '\\';
        // Passa para o próximo Incluir
            Incluir = Incluir->Proximo;
        // Abre o diretório e obtém a primeira entrada
            WIN32_FIND_DATA ffd;
            wdir = FindFirstFile(mens, &ffd);
            if (wdir == INVALID_HANDLE_VALUE)
                continue;
            if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;
            nomearq = ffd.cFileName;
#else
        // Obtém o nome a procurar
            copiastr(mens, Incluir->Padrao, sizeof(mens) - 1);
            strcat(mens, "*");
            char * p2 = mens;
            for (char * p = mens; *p; p++)
                switch (*p)
                {
                case '\\': *p = '/';
                case '/':  p2 = p;
                }
            *p2 = 0;
            if (p2 == mens)
                strcpy(mens, ".");
        // Passa para o próximo Incluir
            Incluir = Incluir->Proximo;
        // Abre o diretório e obtém a primeira entrada
            dir = opendir(mens);
            if (dir == nullptr)
                continue;
            struct dirent * sdir = readdir(dir);
            if (sdir == nullptr)
            {
                closedir(dir);
                dir = nullptr;
                continue;
            }
            if (sdir->d_type != DT_REG)
                continue;
            nomearq = sdir->d_name;
#endif
        }
        else

// Diretório está aberto: obtém a próxima entrada
        {
#ifdef __WIN32__
            WIN32_FIND_DATA ffd;
            if (FindNextFile(wdir, &ffd) == 0)
            {
                FindClose(wdir);
                wdir = INVALID_HANDLE_VALUE;
                continue;
            }
            if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;
            nomearq = ffd.cFileName;
#else
            struct dirent * sdir = readdir(dir);
            if (sdir == nullptr)
            {
                closedir(dir);
                dir = nullptr;
                continue;
            }
            if (sdir->d_type != DT_REG)
                continue;
            nomearq = sdir->d_name;
#endif
        }

// Ignora entradas "." e ".."
        //if (strcmp(nomearq, ".")==0 || strcmp(nomearq, "..")==0)
        //    continue;

        if (*nomearq == '.') // Ignora arquivos cujo nome começa com um ponto
            continue;

// Checa início do nome do arquivo
        if (memcmp(nomearq, Arquivo+ArqBarra, ArqPadrao-ArqBarra) != 0)
            continue;

// Checa fim do nome do arquivo
        int tam = strlen(nomearq) - 4;
        if (tam < ArqPadrao - ArqBarra)
            continue;
        if (strcmp(nomearq + tam, ".int") != 0)
            continue;

// Checa se nome válido para arquivo ".int"
        if (!TArqMapa::NomeValido(nomearq))
            continue;

// Anota nome do arquivo
        if (tam > static_cast<int>(sizeof(Arquivo)) - ArqBarra + 1)
            continue;
        memcpy(Arquivo + ArqBarra, nomearq, tam);
        Arquivo[ArqBarra + tam] = 0;

// Checa arquivo inicial
        if (strcmp(Arquivo + ArqBarra, TArqIncluir::ArqNome()) == 0)
            continue;
        return;
    }
}

//------------------------------------------------------------------------------
bool TVarArqProg::FuncAbrir(TVariavel * v)
{
    TVarArqProg * ref = reinterpret_cast<TVarArqProg*>(v->endvar) + v->indice;
    ref->Fechar();
    ref->Abrir();
    return Instr::CriarVarTxtFixo(v, "");
}
//------------------------------------------------------------------------------
bool TVarArqProg::FuncFechar(TVariavel * v)
{
    TVarArqProg * ref = reinterpret_cast<TVarArqProg*>(v->endvar) + v->indice;
    ref->Fechar();
    return false;
}

//------------------------------------------------------------------------------
bool TVarArqProg::FuncDepois(TVariavel * v)
{
    TVarArqProg * ref = reinterpret_cast<TVarArqProg*>(v->endvar) + v->indice;
    ref->Proximo();
    return false;
}

//------------------------------------------------------------------------------
bool TVarArqProg::FuncLin(TVariavel * v)
{
    TVarArqProg * ref = reinterpret_cast<TVarArqProg*>(v->endvar) + v->indice;
    bool b = (*ref->Arquivo != 0);
    return Instr::CriarVarInt(v, b);
}

//------------------------------------------------------------------------------
bool TVarArqProg::FuncTexto(TVariavel * v)
{
    TVarArqProg * ref = reinterpret_cast<TVarArqProg*>(v->endvar) + v->indice;
    char arq[ARQINCLUIR_TAM];
    copiastr(arq, ref->Arquivo, sizeof(arq));
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(arq);
}

//------------------------------------------------------------------------------
int TVarArqProg::FTamanho(const char * instr)
{
    return sizeof(TVarArqProg);
}

//------------------------------------------------------------------------------
int TVarArqProg::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TVarArqProg);
}

//------------------------------------------------------------------------------
void TVarArqProg::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TVarArqProg * ref = reinterpret_cast<TVarArqProg*>(v->endvar);
    for (; antes < depois; antes++)
        ref[antes].Criar();
    for (; depois < antes; depois++)
        ref[depois].Apagar();
}

//------------------------------------------------------------------------------
void TVarArqProg::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    int total = (unsigned char)v->defvar[Instr::endVetor];
    memmove(destino, v->endvar, (total ? total : 1) * sizeof(TVarArqProg));
}
