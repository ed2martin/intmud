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

TVarDebug * TVarDebug::Inicio = 0;
TVarDebug * TVarDebug::ObjAtual = 0;

size_t getPeakRSS();
size_t getCurrentRSS();

//------------------------------------------------------------------------------
void TVarDebug::Criar()
{
    Antes = 0;
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
    if (ObjAtual==this)
        ObjAtual=Depois;
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
        endobjeto=o, b_objeto=true;
    else
        endclasse=c, b_objeto=false;
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
            Instr::ExecX();
            Instr::ExecFim();
        }
    // Passa para próximo objeto
        vobj = ObjAtual;
    } // for (TVarDebug ...
}

//----------------------------------------------------------------------------
bool TVarDebug::Func(TVariavel * v, const char * nome)
{
    using namespace Instr;
    if (comparaZ(nome, "ini")==0)
    {
        VarExec = VarExecIni;
        return false;
    }
    int num=0;
    if (comparaZ(nome, "exec")==0)
        num=1;
    else if (comparaZ(nome, "utempo")==0)
        num=2;
    else if (comparaZ(nome, "stempo")==0)
        num=3;
    else if (comparaZ(nome, "mem")==0)
        num=4;
    else if (comparaZ(nome, "memmax")==0)
        num=5;
    if (num)
    {
        ApagarVar(v+1);
        VarAtual->numfunc = num;
        return true;
    }
// Nível da função atual
    if (comparaZ(nome, "func")==0)
    {
        ApagarVar(v);
        return CriarVarInt(FuncAtual - FuncPilha);
    }
// Executar instruções contidas em um texto
    if (comparaZ(nome, "cmd")==0)
    {
        if (VarAtual < v+1)
            return false;
        if (FuncAtual >= FuncFim - 2 || FuncAtual->este==0)
            return false;
    // Obtém o objeto "este"
        const char * def_instr = 0;
        TObjeto * obj = 0;
        if (VarAtual >= v+2)
            obj = v[1].getObj(), def_instr = v[2].getTxt();
        else
            obj = FuncAtual->este, def_instr = v[1].getTxt();
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
        ChecaLinha checalinha;
        checalinha.Inicio();
        checalinha.Instr(InstrDebugFunc);
        for (char * com = mens.Buf; com[0] || com[1]; com+=Num16(com),linha++)
        {
            const char * p = checalinha.Instr(com);
            if (com[2]==cHerda)
                p = "Instrução herda não é suportada por cmd";
            if (p)
            {
                char txt1[1024];
                mprintf(txt1, sizeof(txt1), "%d: %s", linha, p);
                ApagarVar(v);
                return CriarVarTexto(txt1);
            }
        }
        const char * p = checalinha.Fim();
        if (p)
        {
            char txt1[1024];
            mprintf(txt1, sizeof(txt1), "%d: %s", linha, p);
            ApagarVar(v);
            return CriarVarTexto(txt1);
        }
    // Acerta o bloco
        TClasse::AcertaComandos(mens.Buf);
    // Anota instruções em DadosPilha
        ApagarVar(v);
        if (!CriarVarTexto(mens.Buf + TotalFunc, mens.Total - TotalFunc))
            return CriarVarTexto(
                    "Quantidade de instruções muito grande");
    // Para mostrar o que codificou
#if 0
        for (const char * p = VarAtual->end_char; Num16(p); p+=Num16(p))
        {
            char mens[4096];
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
        FuncAtual++;
        FuncAtual->nome = VarAtual->end_char;
        FuncAtual->linha = VarAtual->end_char;
        FuncAtual->este = obj;
        FuncAtual->expr = 0;
        FuncAtual->inivar = VarAtual + 1;
        FuncAtual->fimvar = VarAtual + 1;
        FuncAtual->numarg = 0;
        FuncAtual->tipo = 4;
        FuncAtual->indent = 0;
        FuncAtual->objdebug = FuncAtual[-1].objdebug;
        FuncAtual->funcdebug = FuncAtual[-1].funcdebug;
        return true;
    }
// Execução passo-a-passo
    if (comparaZ(nome, "passo")==0)
    {
        FuncAtual->funcdebug = 0;
        if (VarAtual < v+2)
            return false;
    // Obtém o objeto
        TObjeto * obj = v[1].getObj();
        if (obj==0)
            return false;
    // Obtém a instrução na classe
        TClasse * cl = obj->Classe;
        int indice = cl->IndiceNome(v[2].getTxt());
        if (indice < 0)
            return false;
        char * instr = cl->InstrVar[indice];
    // Verifica se é função ou constante que possa ser executada
        if (instr[2]!=cFunc &&
            instr[2]!=cVarFunc)
            return false;
    // Inicia execução passo-a-passo
        FuncAtual->objdebug = obj;
        FuncAtual->funcdebug = instr;
        return false;
    }
// Dados do IntMUD
    if (comparaZ(nome, "ver")==0)
    {
        ApagarVar(v);
        return CriarVarTexto(VERSION);
    }
    if (comparaZ(nome, "data")==0)
    {
        ApagarVar(v);
        return CriarVarTexto(__DATE__);
    }
    return false;
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
    if (numfunc==1)
        return Instr::VarExec;
    else
        return DoubleToInt(getDouble(numfunc));
    return 0;
}

//----------------------------------------------------------------------------
double TVarDebug::getDouble(int numfunc)
{
    if (numfunc==1)
        return Instr::VarExec;
    else if (numfunc==2 || numfunc==3)
    {
#ifdef __WIN32__
        FILETIME CreationTime;
        FILETIME ExitTime;
        FILETIME KernelTime; // 1 = 100nanossegundos
        FILETIME UserTime;   // 1 = 100nanossegundos
        if (GetProcessTimes(GetCurrentProcess(),
            &CreationTime, &ExitTime, &KernelTime, &UserTime)==0)
            return -1;
        if (numfunc==2)
            return UserTime.dwLowDateTime  * 0.0001 +
                   UserTime.dwHighDateTime * 0.0002 * 0x80000000;
        return KernelTime.dwLowDateTime  * 0.0001 +
               KernelTime.dwHighDateTime * 0.0002 * 0x80000000;
#else
        struct rusage uso;
        if (getrusage(RUSAGE_SELF, &uso)<0)
            return -1;
        if (numfunc==2)
            return uso.ru_utime.tv_sec * 1000.0 +
                   uso.ru_utime.tv_usec / 1000.0;
        return uso.ru_stime.tv_sec * 1000.0 +
               uso.ru_stime.tv_usec / 1000.0;
#endif
    }
    else if (numfunc==4)
        return getCurrentRSS();
    else if (numfunc==5)
        return getPeakRSS();
    return 0;
}

//----------------------------------------------------------------------------
void TVarDebug::setValor(int numfunc, int valor)
{
    if (numfunc)
        Instr::VarExec = (valor<1 ? 1 : valor);
}

//----------------------------------------------------------------------------
void TVarDebug::Exec()
{
    using namespace Instr;

// Verifica se pode criar função
    const char * linha = FuncAtual->linha;
    char * exec = FuncAtual->funcdebug;
    if (exec==0 || FuncAtual+1 >= FuncFim)
        return;
    if (linha==0 || (linha[0]==0 && linha[1]==0))
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
    FuncAtual->expr = 0;
    FuncAtual->inivar = VarAtual + 1;
    FuncAtual->fimvar = VarAtual + 1;
    FuncAtual->numarg = 0;
    FuncAtual->tipo = 2;
    FuncAtual->indent = 0;
    FuncAtual->funcdebug = 0;

// Argumento: objeto
    if (!CriarVar(InstrVarObjeto))
        return;
    FuncAtual->fimvar++;
    FuncAtual->numarg++;
    VarAtual->endvar = FuncAtual->este;

// Argumento: linha
    char mens[4096];
    Instr::Decod(mens, linha, sizeof(mens));
    if (!CriarVarTexto(mens))
        return;
    FuncAtual->fimvar++;
    FuncAtual->numarg++;
}
