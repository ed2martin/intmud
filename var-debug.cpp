/* Este arquivo � software livre; voc� pode redistribuir e/ou
 * modificar nos termos da licen�a LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifdef __WIN32__
#include <windows.h>
#else
#include <unistd.h>
#include <sys/resource.h>
#endif
#include "var-debug.h"
#include "config.h"
#include "mudarclasse.h"
#include "objeto.h"
#include "variavel.h"
#include "variavel-def.h"
#include "instr.h"
#include "instr-enum.h"
#include "misc.h"

TVarDebug * TVarDebug::Inicio = nullptr;
TVarDebug * TVarDebug::ObjAtual = nullptr;

size_t getPeakRSS();
size_t getCurrentRSS();

//----------------------------------------------------------------------------
const TVarInfo * TVarDebug::Inicializa()
{
    static TVarInfo::FuncItem ListaFuncEnd[] = {
        { "cmd",          &TVarDebug::FuncCmd },
        { "data",         &TVarDebug::FuncData },
        { "exec",         &TVarDebug::FuncExec },
        { "func",         &TVarDebug::FuncFunc },
        { "ini",          &TVarDebug::FuncIni },
        { "mem",          &TVarDebug::FuncMem },
        { "memmax",       &TVarDebug::FuncMemMax },
        { "passo",        &TVarDebug::FuncPasso },
        { "stempo",       &TVarDebug::FuncStempo },
        { "utempo",       &TVarDebug::FuncUtempo },
        { "ver",          &TVarDebug::FuncVer } };
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        FTipo,
        FRedim,
        FMoverEnd,
        FMoverDef,
        FGetBool,
        FGetInt,
        FGetDouble,
        FGetTxt,
        TVarInfo::FGetObjNulo,
        FOperadorAtrib,
        TVarInfo::FOperadorAddFalse,
        FOperadorIgual2,
        FOperadorCompara,
        TVarInfo::FFuncTextoFalse,
        TVarInfo::FFuncVetorFalse,
        ListaFuncEnd,
        sizeof(ListaFuncEnd) / sizeof(ListaFuncEnd[0]) - 1);
    return &var;
}

//------------------------------------------------------------------------------
inline void TVarDebug::Criar()
{
    Antes = nullptr;
    Depois = Inicio;
    if (Inicio)
        Inicio->Antes = this;
    Inicio = this;
}

//------------------------------------------------------------------------------
inline void TVarDebug::Apagar()
{
    (Antes ? Antes->Depois : Inicio) = Depois;
    if (Depois)
        Depois->Antes = Antes;
    if (ObjAtual == this)
        ObjAtual = Depois;
}

//------------------------------------------------------------------------------
inline void TVarDebug::Mover(TVarDebug * destino)
{
    (Antes ? Antes->Depois : Inicio) = destino;
    if (Depois)
        Depois->Antes = destino;
    if (ObjAtual == this)
        ObjAtual = destino;
    memmove(destino, this, sizeof(TVarDebug));
}

//------------------------------------------------------------------------------
inline void TVarDebug::EndObjeto(TClasse * c, TObjeto * o)
{
    if (o)
        endobjeto = o, b_objeto = true;
    else
        endclasse = c, b_objeto = false;
}

//------------------------------------------------------------------------------
void TVarDebug::FuncEvento(const char * evento, const char * texto)
{
    //printf("FuncEvento [%s] [%s]\n", evento, texto); fflush(stdout);
    for (TVarDebug * vobj = Inicio; vobj;)
    {
        bool prossegue = false;
    // Definido em objeto: prepara para executar
        if (vobj->b_objeto)
        {
            char mens[80];
            sprintf(mens, "%s_%s", vobj->defvar + Instr::endNome, evento);
            prossegue = Instr::ExecIni(vobj->endobjeto, mens);
        }
    // Definido em classe: prepara para executar
        else if (vobj->endclasse)
        {
            char mens[80];
            sprintf(mens, "%s_%s", vobj->defvar + Instr::endNome, evento);
            prossegue = Instr::ExecIni(vobj->endclasse, mens);
        }
    // Executa
        ObjAtual = vobj->Depois;
        if (prossegue)
        {
            if (texto)
                Instr::ExecArg(texto);
            Instr::ExecArg(vobj->indice);
            Instr::ExecX();
            Instr::ExecFim();
        }
    // Passa para pr�ximo objeto
        vobj = ObjAtual;
    } // for (TVarDebug ...
}

//----------------------------------------------------------------------------
bool TVarDebug::FuncIni(TVariavel * v)
{
    Instr::VarExec = Instr::VarExecIni;
    return false;
}

//----------------------------------------------------------------------------
bool TVarDebug::FuncExec(TVariavel * v)
{
    Instr::ApagarVar(v + 1);
    v->numfunc = 1;
    return true;
}

//----------------------------------------------------------------------------
bool TVarDebug::FuncUtempo(TVariavel * v)
{
    Instr::ApagarVar(v + 1);
    v->numfunc = 2;
    return true;
}

//----------------------------------------------------------------------------
bool TVarDebug::FuncStempo(TVariavel * v)
{
    Instr::ApagarVar(v + 1);
    v->numfunc = 3;
    return true;
}

//----------------------------------------------------------------------------
bool TVarDebug::FuncMem(TVariavel * v)
{
    Instr::ApagarVar(v + 1);
    v->numfunc = 4;
    return true;
}

//----------------------------------------------------------------------------
bool TVarDebug::FuncMemMax(TVariavel * v)
{
    Instr::ApagarVar(v + 1);
    v->numfunc = 5;
    return true;
}

//----------------------------------------------------------------------------
// N�vel da fun��o atual
bool TVarDebug::FuncFunc(TVariavel * v)
{
    return Instr::CriarVarInt(v, Instr::FuncAtual - Instr::FuncPilha);
}

//----------------------------------------------------------------------------
// Executar instru��es contidas em um texto
bool TVarDebug::FuncCmd(TVariavel * v)
{
    if (Instr::VarAtual < v + 1)
        return false;
    if (Instr::FuncAtual >= Instr::FuncFim - 2 || Instr::FuncAtual->este == 0)
        return false;
// Obt�m o objeto "este"
    const char * def_instr = nullptr;
    TObjeto * obj = nullptr;
    if (Instr::VarAtual >= v + 2)
        obj = v[1].getObj(), def_instr = v[2].getTxt();
    else
        obj = Instr::FuncAtual->este, def_instr = v[1].getTxt();
// Codifica as instru��es
    TAddBuffer mens;
        // Adiciona defini��o de fun��o
    assert(TMudarAux::CodifInstr(&mens, "func f"));
    unsigned int TotalFunc = mens.Total;
    if (!TMudarAux::CodifInstr(&mens, def_instr)) // Checa se erro
    {
        mens.Add("\x00\x00", 2); // Zero no fim da mensagem
        mens.AnotarBuf();    // Anota resultado em mens.Buf
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(mens.Buf);
    }
    if (mens.Total == TotalFunc) // Nenhuma instru��o
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTxtFixo("");
    }
    mens.Add("\x00\x00", 2); // Zero no fim da mensagem
    mens.AnotarBuf();    // Anota resultado em mens.Buf
// Verifica se bloco v�lido
    int linha = 1;
    Instr::ChecaLinha checalinha;
    checalinha.Inicio();
    checalinha.Instr(Instr::InstrDebugFunc);
    for (char * com = mens.Buf; com[0] || com[1]; com += Num16(com), linha++)
    {
        const char * p = checalinha.Instr(com);
        if (com[2] == Instr::cHerda)
            p = "Instru��o herda n�o � suportada por cmd";
        if (p)
        {
            char txt1[1024];
            mprintf(txt1, sizeof(txt1), "%d: %s", linha, p);
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(txt1);
        }
    }
    const char * p = checalinha.Fim();
    if (p)
    {
        char txt1[1024];
        mprintf(txt1, sizeof(txt1), "%d: %s", linha, p);
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(txt1);
    }
// Acerta o bloco
    TClasse::AcertaComandos(mens.Buf);
// Anota instru��es em DadosPilha
    Instr::ApagarVar(v);
    if (!Instr::CriarVarTexto(mens.Buf + TotalFunc, mens.Total - TotalFunc))
        return Instr::CriarVarTxtFixo(
                "Quantidade de instru��es muito grande");
// Para mostrar o que codificou
#if 0
    for (const char * p = VarAtual->endchar; Num16(p); p+=Num16(p))
    {
        char mens[BUF_MENS];
        int total = Num16(p);
        putchar('-');
        for (int x=0; x<total; x++)
            printf(" %02X", (unsigned char)p[x]);
        putchar('\n');
        if (Instr::Mostra(mens, p, sizeof(mens)))
            printf("+ %s\n", mens);
        if (Instr::Decod(mens, p, sizeof(mens)))
            printf("%s\n", mens);
        else
        {
            printf("**** Erro\n");
            exit(EXIT_FAILURE);
        }
    }
    putchar('\n');
#endif
// Acerta fun��o
    Instr::FuncAtual++;
    Instr::FuncAtual->nome = Instr::VarAtual->endchar;
    Instr::FuncAtual->linha = Instr::VarAtual->endchar;
    Instr::FuncAtual->este = obj;
    Instr::FuncAtual->expr = nullptr;
    Instr::FuncAtual->inivar = Instr::VarAtual + 1;
    Instr::FuncAtual->fimvar = Instr::VarAtual + 1;
    Instr::FuncAtual->numarg = 0;
    Instr::FuncAtual->tipo = 4;
    Instr::FuncAtual->indent = 0;
    Instr::FuncAtual->objdebug = Instr::FuncAtual[-1].objdebug;
    Instr::FuncAtual->funcdebug = Instr::FuncAtual[-1].funcdebug;
    return true;
}

//----------------------------------------------------------------------------
// Execu��o passo-a-passo
bool TVarDebug::FuncPasso(TVariavel * v)
{
    Instr::FuncAtual->funcdebug = nullptr;
    if (Instr::VarAtual < v + 2)
        return false;
// Obt�m o objeto
    TObjeto * obj = v[1].getObj();
    if (obj == nullptr)
        return false;
// Obt�m a instru��o na classe
    TClasse * cl = obj->Classe;
    int indice = cl->IndiceNome(v[2].getTxt());
    if (indice < 0)
        return false;
    char * instr = cl->InstrVar[indice];
// Verifica se � fun��o ou constante que possa ser executada
    if (instr[2] != Instr::cFunc &&
        instr[2] != Instr::cVarFunc)
        return false;
// Inicia execu��o passo-a-passo
    Instr::FuncAtual->objdebug = obj;
    Instr::FuncAtual->funcdebug = instr;
    return false;
}

//----------------------------------------------------------------------------
bool TVarDebug::FuncVer(TVariavel * v)
{
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(VERSION);
}

//----------------------------------------------------------------------------
bool TVarDebug::FuncData(TVariavel * v)
{
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(__DATE__);
}

//----------------------------------------------------------------------------
int TVarDebug::getInt(int numfunc)
{
    if (numfunc == 1)
        return Instr::VarExec;
    else
        return DoubleToInt(getDouble(numfunc));
    return 0;
}

//----------------------------------------------------------------------------
double TVarDebug::getDouble(int numfunc)
{
    if (numfunc == 1)
        return Instr::VarExec;
    else if (numfunc == 2 || numfunc == 3)
    {
#ifdef __WIN32__
        FILETIME CreationTime;
        FILETIME ExitTime;
        FILETIME KernelTime; // 1 = 100nanossegundos
        FILETIME UserTime;   // 1 = 100nanossegundos
        if (GetProcessTimes(GetCurrentProcess(),
            &CreationTime, &ExitTime, &KernelTime, &UserTime) == 0)
            return -1;
        if (numfunc == 2)
            return UserTime.dwLowDateTime  * 0.0001 +
                   UserTime.dwHighDateTime * 0.0002 * 0x80000000;
        return KernelTime.dwLowDateTime  * 0.0001 +
               KernelTime.dwHighDateTime * 0.0002 * 0x80000000;
#else
        struct rusage uso;
        if (getrusage(RUSAGE_SELF, &uso) < 0)
            return -1;
        if (numfunc == 2)
            return uso.ru_utime.tv_sec * 1000.0 +
                   uso.ru_utime.tv_usec / 1000.0;
        return uso.ru_stime.tv_sec * 1000.0 +
               uso.ru_stime.tv_usec / 1000.0;
#endif
    }
    else if (numfunc == 4)
        return getCurrentRSS();
    else if (numfunc == 5)
        return getPeakRSS();
    return 0;
}

//----------------------------------------------------------------------------
void TVarDebug::Exec()
{
    using namespace Instr;

// Verifica se pode criar fun��o
    const char * linha = FuncAtual->linha;
    char * exec = FuncAtual->funcdebug;
    if (exec == nullptr || FuncAtual + 1 >= FuncFim)
        return;
    if (linha == nullptr || (linha[0] == 0 && linha[1] == 0))
        return;
    switch (linha[2])
    {
    case cFunc:
    case cVarFunc:
    case cConstNulo:
    case cConstTxt:
    case cConstNum:
    case cConstExpr:
    case cConstVar:
        return;
    }

// Cria fun��o
    FuncAtual++;
    FuncAtual->nome = exec;
    FuncAtual->linha = exec + Num16(exec);
    FuncAtual->este = FuncAtual[-1].objdebug;
    FuncAtual->expr = nullptr;
    FuncAtual->inivar = VarAtual + 1;
    FuncAtual->fimvar = VarAtual + 1;
    FuncAtual->numarg = 0;
    FuncAtual->tipo = 2;
    FuncAtual->indent = 0;
    FuncAtual->funcdebug = nullptr;

// Argumento: objeto
    if (!CriarVar(InstrVarObjeto))
        return;
    FuncAtual->fimvar++;
    FuncAtual->numarg++;
    VarAtual->endvar = FuncAtual->este;

// Argumento: linha
    char mens[BUF_MENS];
    Instr::Decod(mens, linha, sizeof(mens));
    if (!CriarVarTexto(mens))
        return;
    FuncAtual->fimvar++;
    FuncAtual->numarg++;
}

//------------------------------------------------------------------------------
int TVarDebug::FTamanho(const char * instr)
{
    return sizeof(TVarDebug);
}

//------------------------------------------------------------------------------
int TVarDebug::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TVarDebug);
}

//----------------------------------------------------------------------------
TVarTipo TVarDebug::FTipo(TVariavel * v)
{
    switch (v->numfunc)
    {
    case 0: return varOutros;
    case 1: return varInt;
    }
    return varDouble;
}

//------------------------------------------------------------------------------
void TVarDebug::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TVarDebug * ref = reinterpret_cast<TVarDebug*>(v->endvar);
    for (; antes < depois; antes++)
    {
        ref[antes].Criar();
        ref[antes].defvar = v->defvar;
        ref[antes].indice = antes;
        ref[antes].EndObjeto(c, o);
    }
    for (; depois < antes; depois++)
        ref[depois].Apagar();
}

//------------------------------------------------------------------------------
void TVarDebug::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    VARIAVEL_MOVER_COMPLETO(TVarDebug)
}

//------------------------------------------------------------------------------
void TVarDebug::FMoverDef(TVariavel * v)
{
    VARIAVEL_MOVERDEF(TVarDebug)
}

//------------------------------------------------------------------------------
bool TVarDebug::FGetBool(TVariavel * v)
{
    return getInt(v->numfunc);
}

//------------------------------------------------------------------------------
int TVarDebug::FGetInt(TVariavel * v)
{
    return getInt(v->numfunc);
}

//------------------------------------------------------------------------------
double TVarDebug::FGetDouble(TVariavel * v)
{
    return getDouble(v->numfunc);
}

//------------------------------------------------------------------------------
const char * TVarDebug::FGetTxt(TVariavel * v)
{
    char * buf = TVarInfo::BufferTxt();
    sprintf(buf, "%d", getInt(v->numfunc));
    return buf;
}

//------------------------------------------------------------------------------
void TVarDebug::FOperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    if (v1->numfunc)
    {
        int valor = v2->getInt();
        Instr::VarExec = (valor < 1 ? 1 : valor);
    }
}

//------------------------------------------------------------------------------
bool TVarDebug::FOperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    if (v1->numfunc == 0)
        return (v1->endvar == v2->endvar && v1->indice == v2->indice &&
                v1->defvar[2] == v2->defvar[2] && v1->numfunc == v2->numfunc);
    return (v2->TipoNumerico() && getDouble(v1->numfunc) == v2->getDouble());
}

//------------------------------------------------------------------------------
unsigned char TVarDebug::FOperadorCompara(TVariavel * v1, TVariavel * v2)
{
    if (v1->numfunc == 0)
        return (v1->endvar == v2->endvar && v1->indice == v2->indice &&
                v1->defvar[2] == v2->defvar[2] &&
                v1->numfunc == v2->numfunc ? 2 : 0);
    return TVarInfo::ComparaDouble(getDouble(v1->numfunc), v2);
}
