/* Este arquivo � software livre; voc� pode redistribuir e/ou
 * modificar nos termos da licen�a LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "var-arqtxt.h"
#include "variavel.h"
#include "variavel-def.h"
#include "instr.h"
#include "instr-enum.h"
#include "misc.h"

//#define DEBUG

//----------------------------------------------------------------------------
const TVarInfo * TVarArqTxt::Inicializa()
{
    static TVarInfo::FuncItem ListaFuncEnd[] = {
        { "abrir",        &TVarArqTxt::FuncAbrir },
        { "eof",          &TVarArqTxt::FuncEof },
        { "escr",         &TVarArqTxt::FuncEscr },
        { "existe",       &TVarArqTxt::FuncExiste },
        { "fechar",       &TVarArqTxt::FuncFechar },
        { "flush",        &TVarArqTxt::FuncFlush },
        { "ler",          &TVarArqTxt::FuncLer },
        { "pos",          &TVarArqTxt::FuncPos },
        { "truncar",      &TVarArqTxt::FuncTruncar },
        { "valido",       &TVarArqTxt::FuncValido }  };
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        TVarInfo::FTipoOutros,
        FRedim,
        FMoverEnd,
        TVarInfo::FMoverDef0,
        FGetBool,
        FGetInt,
        FGetDouble,
        FGetTxt,
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

//------------------------------------------------------------------------------
inline void TVarArqTxt::Criar()
{
    arq = nullptr;
}

//------------------------------------------------------------------------------
inline void TVarArqTxt::Apagar()
{
    if (arq)
        fclose(arq);
}

//------------------------------------------------------------------------------
void TVarArqTxt::Fechar()
{
    if (arq == nullptr)
        return;
    fclose(arq);
    arq = nullptr;
}

//------------------------------------------------------------------------------
inline int TVarArqTxt::getValor()
{
    return (arq != nullptr);
}

//------------------------------------------------------------------------------
bool TVarArqTxt::FuncLer(TVariavel * v)
{
    TVarArqTxt * ref = reinterpret_cast<TVarArqTxt*>(v->endvar) + v->indice;
    char mens[BUF_MENS];
    int pmens = 0;
// L� o texto do arquivo
    if (ref->arq) // Se o arquivo est� aberto...
    {
        if (ref->ModoBinario)
        {
            int tam = 1;
            if (Instr::VarAtual > v)
                tam = v[1].getInt();
            if (tam < 0)
                tam = 0;
            if (tam > (int)sizeof(mens) / 2)
                tam = (int)sizeof(mens) / 2;
            tam *= 2;
            while (pmens < tam)
            {
                int lido = fgetc(ref->arq);
                if (lido == EOF)
                    break;
                char ch = ((lido >> 4) & 15);
                mens[pmens++] = (ch < 10 ? ch + '0' : ch + 'a' - 10);
                ch = (lido & 15);
                mens[pmens++] = (ch < 10 ? ch + '0' : ch + 'a' - 10);
            }
        }
        else if (Instr::VarAtual > v) // Ler o tamanho especificado
        {
            int tam = v[1].getInt();
            if (tam > (int)sizeof(mens))
                tam = (int)sizeof(mens);
            while (pmens < tam)
            {
                int lido = fgetc(ref->arq);
                if (lido == EOF)
                    break;
                if (lido == '\n')
                    mens[pmens++] = Instr::ex_barra_n;
                else if (lido >= ' ')
                    mens[pmens++] = lido;
            }
        }
        else // Nenhum argumento: ler uma linha
            while (pmens < (int)sizeof(mens))
            {
                int lido = fgetc(ref->arq);
                if (lido == EOF || lido == '\n')
                    break;
                if (lido >= ' ')
                    mens[pmens++] = lido;
            }
    }
// Acerta vari�veis
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(mens, pmens);
}

//------------------------------------------------------------------------------
bool TVarArqTxt::FuncEscr(TVariavel * v)
{
    TVarArqTxt * ref = reinterpret_cast<TVarArqTxt*>(v->endvar) + v->indice;
    if (ref->arq == nullptr)
        return false;
    for (TVariavel * v1 = v + 1; v1 <= Instr::VarAtual; v1++)
    {
        const char * txt = v1->getTxt();
        char mens[BUF_MENS];
        int pmens = 0;
        if (ref->ModoBinario)
        {
            int valor = 1;
            while (true)
            {
                unsigned char ch = *txt++;
                if (ch == 0)
                    break;
                else if (ch >= '0' && ch <= '9')
                    valor = valor * 16 + ch - '0';
                else if (ch >= 'A' && ch <= 'F')
                    valor = valor * 16 + ch - 'A' + 10;
                else if (ch >= 'a' && ch <= 'f')
                    valor = valor * 16 + ch - 'a' + 10;
                else
                    continue;
                if (valor < 0x100)
                    continue;
                mens[pmens++] = valor, valor = 1;
                if (pmens >= (int)sizeof(mens))
                {
                    fwrite(mens, 1, pmens, ref->arq);
                    pmens = 0;
                }
            }
            if (pmens)
                fwrite(mens, 1, pmens, ref->arq);
            continue;
        }
        while (*txt)
        {
            if (pmens >= (int)sizeof(mens)-2)
            {
                fwrite(mens, 1, pmens, ref->arq);
                pmens = 0;
            }
            switch (*txt)
            {
            case Instr::ex_barra_b:
                txt++;
                break;
            case Instr::ex_barra_c:
                if ((txt[1] >= '0' && txt[1] <= '9') ||
                        (txt[1] >= 'A' && txt[1] <= 'J') ||
                        (txt[1] >= 'a' && txt[1] <= 'j'))
                    txt += 2;
                else
                    txt++;
                break;
            case Instr::ex_barra_d:
                if (txt[1] >= '0' && txt[1] <= '7')
                    txt += 2;
                else
                    txt++;
                break;
            case Instr::ex_barra_n:
                mens[pmens++] = '\n';
                txt++;
                break;
            default:
                mens[pmens++] = *txt++;
            }
        }
        if (pmens)
            fwrite(mens, 1, pmens, ref->arq);
    }
    return false;
}

//------------------------------------------------------------------------------
bool TVarArqTxt::FuncEof(TVariavel * v)
{
    TVarArqTxt * ref = reinterpret_cast<TVarArqTxt*>(v->endvar) + v->indice;
    int result = (ref->arq == nullptr || feof(ref->arq));
#ifdef DEBUG
    printf("EOF = %d\n", result);
    fflush(stdout);
#endif
    return Instr::CriarVarInt(v, result);
}

//------------------------------------------------------------------------------
bool TVarArqTxt::FuncPos(TVariavel * v)
{
    TVarArqTxt * ref = reinterpret_cast<TVarArqTxt*>(v->endvar) + v->indice;
    if (ref->arq && Instr::VarAtual >= v + 1)
    {
        int pos = v[1].getInt();
        int modo = 0;
        if (Instr::VarAtual >= v + 2)
            modo = v[2].getInt();
        switch (modo)
        {
        case 0: fseek(ref->arq, pos, SEEK_SET); break;
        case 1: fseek(ref->arq, pos, SEEK_CUR); break;
        case 2: fseek(ref->arq, pos, SEEK_END); break;
        }
    }
    int novapos = (ref->arq ? ftell(ref->arq) : 0);
    return Instr::CriarVarInt(v, novapos);
}

//------------------------------------------------------------------------------
bool TVarArqTxt::FuncFechar(TVariavel * v)
{
    TVarArqTxt * ref = reinterpret_cast<TVarArqTxt*>(v->endvar) + v->indice;
    ref->Fechar();
    return false;
}

//------------------------------------------------------------------------------
bool TVarArqTxt::FuncFlush(TVariavel * v)
{
    TVarArqTxt * ref = reinterpret_cast<TVarArqTxt*>(v->endvar) + v->indice;
    if (ref->arq)
        fflush(ref->arq);
    return false;
}

//------------------------------------------------------------------------------
bool TVarArqTxt::FuncValido(TVariavel * v)
{
    char arqnome[300] = ""; // Nome do arquivo; nulo se n�o for v�lido
    if (Instr::VarAtual >= v + 1)
    {
        copiastr(arqnome, v[1].getTxt(), sizeof(arqnome) - 4);
        if (!arqvalido(arqnome, false))
            *arqnome = 0;
    }
    return Instr::CriarVarInt(v, *arqnome != 0);
}

//------------------------------------------------------------------------------
bool TVarArqTxt::FuncExiste(TVariavel * v)
{
    char arqnome[300] = ""; // Nome do arquivo; nulo se n�o for v�lido
    if (Instr::VarAtual >= v + 1)
    {
        copiastr(arqnome, v[1].getTxt(), sizeof(arqnome) - 4);
        if (!arqvalido(arqnome, false))
            *arqnome = 0;
    }
    struct stat buf;
    if (*arqnome && stat(arqnome, &buf) < 0)
        *arqnome = 0;
    return Instr::CriarVarInt(v, *arqnome != 0);
}

//------------------------------------------------------------------------------
bool TVarArqTxt::FuncAbrir(TVariavel * v)
{
// Obt�m o nome do arquivo
    char arqnome[300] = ""; // Nome do arquivo; nulo se n�o for v�lido
    bool escrita = false;
    if (Instr::VarAtual >= v + 1)
    {
        copiastr(arqnome, v[1].getTxt(), sizeof(arqnome) - 4);
    // Verifica se nome permitido
        if (arqvalido(arqnome, true))
            escrita = true;
        else if (!arqvalido(arqnome, false))
            *arqnome = 0;
    }
    FILE * descr = nullptr;
    int modo = -1;
    if (*arqnome && Instr::VarAtual >= v + 2)
        modo = v[2].getInt();
// Abre arquivo
    TVarArqTxt * ref = reinterpret_cast<TVarArqTxt*>(v->endvar) + v->indice;
    switch (modo)
    {
    case 0:
        ref->ModoBinario = false, descr = fopen(arqnome, "r");
        break;
    case 1:
        ref->ModoBinario = false, descr = fopen(arqnome, "r+");
        break;
    case 2:
        if (escrita)
            ref->ModoBinario = false, descr = fopen(arqnome, "w");
        break;
    case 3:
        if (escrita)
            ref->ModoBinario = false, descr = fopen(arqnome, "a");
        break;
    case 4:
        ref->ModoBinario = true, descr = fopen(arqnome, "rb");
        break;
    case 5:
        ref->ModoBinario = true, descr = fopen(arqnome, "rb+");
        break;
    case 6:
        if (escrita)
            ref->ModoBinario = true, descr = fopen(arqnome, "wb");
        break;
    case 7:
        if (escrita)
            ref->ModoBinario = true, descr = fopen(arqnome, "ab");
        break;
    }
#ifdef DEBUG
    printf("Abrindo [%s] : %s\n", nome, descr ? "OK" : "ERRO");
    fflush(stdout);
#endif
// Se conseguiu abrir arquivo...
    if (descr != nullptr)
    {
        if (ref->arq)
            fclose(ref->arq);
        ref->arq = descr;
    }
// Resultado
    return Instr::CriarVarInt(v, descr != nullptr);
}

//------------------------------------------------------------------------------
bool TVarArqTxt::FuncTruncar(TVariavel * v)
{
// Obt�m o nome do arquivo
    char arqnome[300] = ""; // Nome do arquivo; nulo se n�o for v�lido
    if (Instr::VarAtual >= v + 1)
    {
        copiastr(arqnome, v[1].getTxt(), sizeof(arqnome) - 4);
        if (!arqvalido(arqnome, true)) // Verifica se nome permitido
            *arqnome=0;
    }
    int descr = -1;
    int tamanho = 0;
    int valor = 0;
// Obt�m novo tamanho do arquivo
    if (Instr::VarAtual >= v + 2)
        tamanho = v[2].getInt();
// Abre arquivo
    if (*arqnome)
        descr = open(arqnome, O_RDWR);
// Se conseguiu abrir arquivo...
    if (descr >= 0)
    {
#ifdef __WIN32__
        valor = (chsize(descr, tamanho) == 0);
#else
        valor = (ftruncate(descr, tamanho) == 0);
#endif
        close(descr);
    }
    return Instr::CriarVarInt(v, valor);
}

//------------------------------------------------------------------------------
int TVarArqTxt::FTamanho(const char * instr)
{
    return sizeof(TVarArqTxt);
}

//------------------------------------------------------------------------------
int TVarArqTxt::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TVarArqTxt);
}

//------------------------------------------------------------------------------
void TVarArqTxt::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TVarArqTxt * ref = reinterpret_cast<TVarArqTxt*>(v->endvar);
    for (; antes < depois; antes++)
        ref[antes].Criar();
    for (; depois < antes; depois++)
        ref[depois].Apagar();
}

//------------------------------------------------------------------------------
void TVarArqTxt::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    int total = (unsigned char)v->defvar[Instr::endVetor];
    memmove(destino, v->endvar, (total ? total : 1) * sizeof(TVarArqTxt));
}

//------------------------------------------------------------------------------
bool TVarArqTxt::FGetBool(TVariavel * v) VARIAVEL_FGETINT0(TVarArqTxt)
int TVarArqTxt::FGetInt(TVariavel * v) VARIAVEL_FGETINT0(TVarArqTxt)
double TVarArqTxt::FGetDouble(TVariavel * v) VARIAVEL_FGETINT0(TVarArqTxt)
const char * TVarArqTxt::FGetTxt(TVariavel * v) VARIAVEL_FGETTXT0(TVarArqTxt)
