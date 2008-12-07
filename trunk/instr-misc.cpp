/* Este programa � software livre; voc� pode redistribuir e/ou
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

#include "instr.h"
#include "variavel.h"
#include "misc.h"

//------------------------------------------------------------------------------
/// Retorna um n�mero que corresponde � prioridade do operador
/** @param operador Operador em Instr::Expressao
    @retval 2-0x2F N�mero que corresponde � prioridade do operador
    @retval 0 Operador inv�lido */
int Instr::Prioridade(int operador)
{
    switch (operador)
    {
    case exo_virgula:    return 20;
    case exo_neg:        return 2;
    case exo_exclamacao: return 2;
    case exo_mul:        return 3;
    case exo_div:        return 3;
    case exo_porcent:    return 3;
    case exo_add:        return 4;
    case exo_sub:        return 4;
    case exo_menor:      return 5;
    case exo_menorigual: return 5;
    case exo_maior:      return 5;
    case exo_maiorigual: return 5;
    case exo_igual:      return 6;
    case exo_diferente:  return 6;
    case exo_e:          return 7;
    case exo_ou:         return 8;
    case exo_igualmul:   return 9;
    case exo_igualdiv:   return 9;
    case exo_igualporcent: return 9;
    case exo_igualadd:   return 9;
    case exo_igualsub:   return 9;
    }
    return 0;
}

//------------------------------------------------------------------------------
/// Verifica se instru��o codificada � herda e cont�m a classe especificada
bool Instr::ChecaHerda(const char * instr, const char * nomeclasse)
{
    if (instr[0]==0 && instr[1]==0)
        return false;
    if (instr[2] != cHerda)
        return false;
    int x = (unsigned char)instr[3];
    for (instr+=4; x; x--)
    {
        if (comparaZ(instr, nomeclasse)==0)
            return true;
        while (*instr++);
    }
    return false;
}

//------------------------------------------------------------------------------
Instr::ChecaLinha::ChecaLinha()
{
    esperando=0;
}

//------------------------------------------------------------------------------
void Instr::ChecaLinha::Inicio()
{
    esperando=0;
}

//------------------------------------------------------------------------------
const char * Instr::ChecaLinha::Instr(const char * instr)
{
    if (instr[0]==0 && instr[1]==0)
        return 0;
    unsigned char cod = *(unsigned char*)(instr+2);
// Instru��o Herda
    if (cod==cHerda)
    {
        if (esperando!=0)
            return "Instru��o Herda deve ser a primeira da classe";
        esperando=1;
        return 0;
    }
    if (esperando==0)
        esperando=1;
// Coment�rio
    if (cod==cComent)
        return 0;
// Fun��o
    if (cod==cFunc || cod==cVarFunc)
    {
        esperando=2;
        return 0;
    }
// Constante
    if (cod==cConstNulo || cod==cConstTxt ||
        cod==cConstNum  || cod==cConstExpr)
    {
        if (esperando!=1)
            esperando=4;
        return 0;
    }
// Vari�vel
    if (cod >= cVariaveis)
    {
        if (esperando == 3)
            return "Vari�vel deve ser definidas no in�cio da fun��o";
        if (esperando > 3)
            return "Vari�vel n�o pertence a uma classe ou uma fun��o";
        return 0;
    }
// Instru��o
    if (esperando<2 || esperando>3)
        return "Instru��o n�o pertence a uma fun��o";
    esperando=3;
    return 0;
}

//------------------------------------------------------------------------------
/// Criar vari�vel no topo da pilha de vari�veis (Instr::VarPilha)
/** @return true se conseguiu criar, false se mem�ria insuficiente */
bool Instr::CriarVar(const char * def)
{
    if (VarAtual >= VarFim-1)
        return false;
// Verifica se mem�ria suficiente
    int tam = TVariavel::Tamanho(def);
    tam = (tam+3) & ~3;
    if (Instr::DadosTopo + tam > Instr::DadosFim)
        return false;
// Acerta vari�veis
    VarAtual++;
    VarAtual->defvar = def;
    VarAtual->bit = 1;
// Cria vari�vel
    if (tam)
    {
        VarAtual->endvar = Instr::DadosTopo;
        VarAtual->local = 1;
        Instr::DadosTopo += tam;
        VarAtual->Criar(0, 0);
    }
    else
    {
        VarAtual->endvar = 0;
        VarAtual->local = 0;
    }
    return true;
}

//----------------------------------------------------------------------------
/// Apaga vari�veis na pilha a partir da vari�vel v
void Instr::ApagarVar(TVariavel * v)
{
    while (VarAtual >= v)
    {
        if (VarAtual->local)
        {
            VarAtual->Apagar();
            if (VarAtual->endvar)
                DadosTopo = (char*)VarAtual->endvar;
        }
        VarAtual--;
    }
}
