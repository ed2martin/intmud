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
#include "var-debug.h"
#include "mudarclasse.h"
#include "objeto.h"
#include "variavel.h"
#include "instr.h"
#include "misc.h"

const char DebugExec[] = { 8, 0, Instr::cDebug, 0, 0, 0, '=', '0', 0 };

//----------------------------------------------------------------------------
bool TVarDebug::Func(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "ini")==0)
    {
        Instr::VarExec = Instr::VarExecIni;
        return false;
    }
    if (comparaZ(nome, "exec")==0)
    {
        Instr::ApagarVar(v+1);
        Instr::VarAtual->defvar = DebugExec;
        return true;
    }
    if (comparaZ(nome, "cmd")==0)
    {
        if (Instr::VarAtual < v+1)
            return false;
        if (Instr::FuncAtual >= Instr::FuncFim - 2 ||
                Instr::FuncAtual->este==0)
            return false;
    // Obtém o objeto "este"
        const char * def_instr = 0;
        TObjeto * obj = 0;
        if (Instr::VarAtual >= v+2)
            obj = v[1].getObj(), def_instr = v[2].getTxt();
        else
            obj = Instr::FuncAtual->este, def_instr = v[1].getTxt();
    // Codifica as instruções
        char mens[16384];
        int  total = TMudarAux::CodifInstr(mens, def_instr, sizeof(mens)-2);
        if (total<0) // Erro
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(mens);
        }
        if (total==0) // Nenhuma instrução
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto("");
        }
        mens[total]=0;  // Marca o fim das instruções
        mens[total+1]=0;
        total += 2;
    // Verifica se bloco válido
        const char DebugFunc[] = { 7, 0, Instr::cFunc, 0, 0, 0, 'f', 0 };
        int linha=1;
        Instr::ChecaLinha checalinha;
        checalinha.Inicio();
        checalinha.Instr(DebugFunc);
        for (char * com = mens; com[0] || com[1]; com+=Num16(com),linha++)
        {
            const char * p = checalinha.Instr(com);
            if (com[2]==Instr::cHerda)
                p = "Instrução herda não é suportada por cmd";
            if (p)
            {
                mprintf(mens, sizeof(mens), "%d: %s", linha, p);
                Instr::ApagarVar(v);
                return Instr::CriarVarTexto(mens);
            }
        }
    // Acerta o bloco
        TClasse::AcertaComandos(mens);
    // Anota instruções em DadosPilha
        Instr::ApagarVar(v);
        if (Instr::CriarVarTexto(mens, total) < 0)
            return Instr::CriarVarTexto(
                    "Quantidade de instruções muito grande");
    // Acerta função
        Instr::FuncAtual++;
        Instr::FuncAtual->linha = Instr::VarAtual->end_char;
        Instr::FuncAtual->este = obj;
        Instr::FuncAtual->expr = 0;
        Instr::FuncAtual->inivar = Instr::VarAtual + 1;
        //Instr::FuncAtual->fimvar = VarAtual + 1;
        Instr::FuncAtual->numarg = 0;
        Instr::FuncAtual->tipo = 0;
    // Acerta variáveis
        int numargs = Instr::FuncAtual[-1].numarg;
        for (TVariavel * v1=Instr::FuncAtual[-1].inivar; v1<v-1; v1++)
        {
            if (Instr::VarAtual >= Instr::VarFim-1)
                break;
            Instr::VarAtual++;
            *Instr::VarAtual = *v1;
            Instr::VarAtual->tamanho = 0;
            if (numargs)
                numargs--, Instr::FuncAtual->numarg++;
        }
        Instr::FuncAtual->fimvar = Instr::VarAtual + 1;
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
int TVarDebug::getValor(const char * defvar1)
{
    if (defvar1[Instr::endNome]!='=')
        return 0;
    return Instr::VarExec;
}

//----------------------------------------------------------------------------
void TVarDebug::setValor(const char * defvar1, int valor)
{
    if (defvar1[Instr::endNome]=='=')
        Instr::VarExec = (valor<1 ? 1 : valor);
}
