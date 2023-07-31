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
#include "instr.h"
#include "misc.h"

TVarDebug * TVarDebug::Inicio = nullptr;
TVarDebug * TVarDebug::ObjAtual = nullptr;

size_t getPeakRSS();
size_t getCurrentRSS();

//------------------------------------------------------------------------------
void TVarDebug::Criar()
{
    Antes = nullptr;
    Depois = Inicio;
    if (Inicio)
        Inicio->Antes = this;
    Inicio = this;
}

//------------------------------------------------------------------------------
void TVarDebug::Apagar()
{
    (Antes ? Antes->Depois : Inicio) = Depois;
    if (Depois)
        Depois->Antes = Antes;
    if (ObjAtual == this)
        ObjAtual = Depois;
}

//------------------------------------------------------------------------------
void TVarDebug::Mover(TVarDebug * destino)
{
    (Antes ? Antes->Depois : Inicio) = destino;
    if (Depois)
        Depois->Antes = destino;
    if (ObjAtual == this)
        ObjAtual = destino;
    memmove(destino, this, sizeof(TVarDebug));
}

//------------------------------------------------------------------------------
void TVarDebug::EndObjeto(TClasse * c, TObjeto * o)
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
    // Passa para próximo objeto
        vobj = ObjAtual;
    } // for (TVarDebug ...
}

//------------------------------------------------------------------------------
bool TVarDebug::Func(TVariavel * v, const char * nome)
{
// Lista das funções de varmem
// Deve obrigatoriamente estar em letras minúsculas e ordem alfabética
    static const struct {
        const char * Nome;
        bool (*Func)(TVariavel * v); } ExecFunc[] = {
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
            return (ExecFunc[meio].Func)(v);
        if (resultado < 0) fim = meio - 1; else ini = meio + 1;
    }
    return false;
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
// Nível da função atual
bool TVarDebug::FuncFunc(TVariavel * v)
{
    Instr::ApagarVar(v);
    return Instr::CriarVarInt(Instr::FuncAtual - Instr::FuncPilha);
}

//----------------------------------------------------------------------------
// Executar instruções contidas em um texto
bool TVarDebug::FuncCmd(TVariavel * v)
{
    if (Instr::VarAtual < v + 1)
        return false;
    if (Instr::FuncAtual >= Instr::FuncFim - 2 || Instr::FuncAtual->este == 0)
        return false;
// Obtém o objeto "este"
    const char * def_instr = nullptr;
    TObjeto * obj = nullptr;
    if (Instr::VarAtual >= v + 2)
        obj = v[1].getObj(), def_instr = v[2].getTxt();
    else
        obj = Instr::FuncAtual->este, def_instr = v[1].getTxt();
// Codifica as instruções
    TAddBuffer mens;
        // Adiciona definição de função
    assert(TMudarAux::CodifInstr(&mens, "func f"));
    unsigned int TotalFunc = mens.Total;
    if (!TMudarAux::CodifInstr(&mens, def_instr)) // Checa se erro
    {
        mens.Add("\x00\x00", 2); // Zero no fim da mensagem
        mens.AnotarBuf();    // Anota resultado em mens.Buf
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(mens.Buf);
    }
    if (mens.Total == TotalFunc) // Nenhuma instrução
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto("");
    }
    mens.Add("\x00\x00", 2); // Zero no fim da mensagem
    mens.AnotarBuf();    // Anota resultado em mens.Buf
// Verifica se bloco válido
    int linha=1;
    Instr::ChecaLinha checalinha;
    checalinha.Inicio();
    checalinha.Instr(Instr::InstrDebugFunc);
    for (char * com = mens.Buf; com[0] || com[1]; com += Num16(com), linha++)
    {
        const char * p = checalinha.Instr(com);
        if (com[2] == Instr::cHerda)
            p = "Instrução herda não é suportada por cmd";
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
// Anota instruções em DadosPilha
    Instr::ApagarVar(v);
    if (!Instr::CriarVarTexto(mens.Buf + TotalFunc, mens.Total - TotalFunc))
        return Instr::CriarVarTexto(
                "Quantidade de instruções muito grande");
// Para mostrar o que codificou
#if 0
    for (const char * p = VarAtual->end_char; Num16(p); p+=Num16(p))
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
// Acerta função
    Instr::FuncAtual++;
    Instr::FuncAtual->nome = Instr::VarAtual->end_char;
    Instr::FuncAtual->linha = Instr::VarAtual->end_char;
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
// Execução passo-a-passo
bool TVarDebug::FuncPasso(TVariavel * v)
{
    Instr::FuncAtual->funcdebug = nullptr;
    if (Instr::VarAtual < v + 2)
        return false;
// Obtém o objeto
    TObjeto * obj = v[1].getObj();
    if (obj == nullptr)
        return false;
// Obtém a instrução na classe
    TClasse * cl = obj->Classe;
    int indice = cl->IndiceNome(v[2].getTxt());
    if (indice < 0)
        return false;
    char * instr = cl->InstrVar[indice];
// Verifica se é função ou constante que possa ser executada
    if (instr[2] != Instr::cFunc &&
        instr[2] != Instr::cVarFunc)
        return false;
// Inicia execução passo-a-passo
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
int TVarDebug::getTipo(int numfunc)
{
    switch (numfunc)
    {
    case 0: return varOutros;
    case 1: return varInt;
    }
    return varDouble;
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
void TVarDebug::setValor(int numfunc, int valor)
{
    if (numfunc)
        Instr::VarExec = (valor < 1 ? 1 : valor);
}

//----------------------------------------------------------------------------
void TVarDebug::Exec()
{
    using namespace Instr;

// Verifica se pode criar função
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

// Cria função
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
