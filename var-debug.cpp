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
#ifdef __WIN32__
#include <windows.h>
#else
#include <sys/resource.h>
#endif
#include "var-debug.h"
#include "mudarclasse.h"
#include "objeto.h"
#include "variavel.h"
#include "instr.h"
#include "misc.h"

//----------------------------------------------------------------------------
bool TVarDebug::Func(TVariavel * v, const char * nome)
{
    using namespace Instr;
// Inicializa contador de instruções executadas
    if (comparaZ(nome, "ini")==0)
    {
        VarExec = VarExecIni;
        return false;
    }
// Contador de instruções executadas
    if (comparaZ(nome, "exec")==0)
    {
        ApagarVar(v+1);
        VarAtual->numfunc = 1;
        return true;
    }
// Quanto tempo usou do processador
    if (comparaZ(nome, "tempo")==0)
    {
        ApagarVar(v+1);
        VarAtual->numfunc = 2;
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
        if (!TMudarAux::CodifInstr(&mens, def_instr)) // Checa se erro
        {
            mens.Add("\x00", 1); // Zero no fim da mensagem
            mens.AnotarBuf();    // Anota resultado em mens.Buf
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(mens.Buf);
        }
        if (mens.Total==0) // Nenhuma instrução
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto("");
        }
        mens.Add("\x00", 1); // Zero no fim da mensagem
        mens.AnotarBuf();    // Anota resultado em mens.Buf
    // Verifica se bloco válido
        const char DebugFunc[] = { 7, 0, cFunc, 0, 0, 0, 'f', 0 };
        int linha=1;
        ChecaLinha checalinha;
        checalinha.Inicio();
        checalinha.Instr(DebugFunc);
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
    // Acerta o bloco
        TClasse::AcertaComandos(mens.Buf);
    // Anota instruções em DadosPilha
        ApagarVar(v);
        if (CriarVarTexto(mens.Buf, mens.Total) < 0)
            return CriarVarTexto(
                    "Quantidade de instruções muito grande");
    // Acerta função
        FuncAtual++;
        FuncAtual->linha = VarAtual->end_char;
        FuncAtual->este = obj;
        FuncAtual->expr = 0;
        FuncAtual->inivar = VarAtual + 1;
        //FuncAtual->fimvar = VarAtual + 1;
        FuncAtual->numarg = 0;
        FuncAtual->tipo = 0;
        FuncAtual->objdebug = FuncAtual[-1].objdebug;
        FuncAtual->funcdebug = FuncAtual[-1].funcdebug;
    // Acerta variáveis
        int numargs = FuncAtual[-1].numarg;
        for (TVariavel * v1=FuncAtual[-1].inivar; v1<v-1; v1++)
        {
            if (VarAtual >= VarFim-1)
                break;
            VarAtual++;
            *VarAtual = *v1;
            VarAtual->tamanho = 0;
            if (numargs)
                numargs--, FuncAtual->numarg++;
        }
        FuncAtual->fimvar = VarAtual + 1;
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
    return false;
}

//----------------------------------------------------------------------------
int TVarDebug::getTipo(int numfunc)
{
    switch (numfunc)
    {
    case 1: return varInt;
    case 2: return varDouble;
    }
    return varOutros;
}

//----------------------------------------------------------------------------
int TVarDebug::getInt(int numfunc)
{
    if (numfunc==1)
        return Instr::VarExec;
    else if (numfunc==2)
        return DoubleToInt(getDouble(numfunc));
    return 0;
}

//----------------------------------------------------------------------------
double TVarDebug::getDouble(int numfunc)
{
    if (numfunc==1)
        return Instr::VarExec;
    else if (numfunc==2)
    {
#ifdef __WIN32__
        FILETIME CreationTime;
        FILETIME ExitTime;
        FILETIME KernelTime; // 1 = 100nanossegundos
        FILETIME UserTime;   // 1 = 100nanossegundos
        if (GetProcessTimes(GetCurrentProcess(),
            &CreationTime, &ExitTime, &KernelTime, &UserTime)==0)
            return -1;
        return KernelTime.dwLowDateTime  * 0.0001 +
               KernelTime.dwHighDateTime * 0.0002 * 0x80000000 +
               UserTime.dwLowDateTime  * 0.0001 +
               UserTime.dwHighDateTime * 0.0002 * 0x80000000;
#else
        struct rusage uso;
        if (getrusage(RUSAGE_SELF, &uso)<0)
            return -1;
        return uso.ru_utime.tv_sec * 1000.0 +
               uso.ru_utime.tv_usec / 1000.0 +
               uso.ru_stime.tv_sec * 1000.0 +
               uso.ru_stime.tv_usec / 1000.0;
#endif
    }
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
        return;
    }

// Cria função
    FuncAtual++;
    FuncAtual->linha = exec + Num16(exec);
    FuncAtual->este = FuncAtual[-1].objdebug;
    FuncAtual->expr = 0;
    FuncAtual->inivar = VarAtual + 1;
    FuncAtual->fimvar = VarAtual + 1;
    FuncAtual->numarg = 0;
    FuncAtual->tipo = 2;
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
