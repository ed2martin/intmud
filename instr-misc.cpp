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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
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
    case exo_igual2:     return 6;
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
// Verifica se tamanho da vari�vel � nulo
    int tam = TVariavel::Tamanho(def);
    if (tam==0)
    {
        VarAtual++;
        VarAtual->defvar = def;
        VarAtual->endvar = 0;
        VarAtual->tamanho = 0;
        VarAtual->indice = (def[endVetor]==0 ? 0 : 0xFF);
        VarAtual->bit = 1;
        return true;
    }
// Verifica se mem�ria suficiente
    char * p = Instr::DadosTopo;
    if ((tam&1)==0)
        p += ((p-Instr::DadosPilha)&1);
    if ((tam&3)==0)
        p += ((p-Instr::DadosPilha)&2);
    if (p + tam > Instr::DadosFim)
        return false;
// Cria vari�vel
    VarAtual++;
    VarAtual->endvar = p;
    VarAtual->defvar = def;
    VarAtual->indice = (def[endVetor]==0 ? 0 : 0xFF);
    VarAtual->bit = 1;
    VarAtual->tamanho = VarAtual->Tamanho();
    Instr::DadosTopo = p + VarAtual->tamanho;
    VarAtual->Criar(0, 0);
    return true;
}

//----------------------------------------------------------------------------
/// Apaga vari�veis na pilha a partir da vari�vel v
void Instr::ApagarVar(TVariavel * v)
{
    while (VarAtual >= v)
    {
        if (VarAtual->tamanho)
        {
            VarAtual->Apagar();
            if (VarAtual->endvar)
                DadosTopo = (char*)VarAtual->endvar;
        }
        VarAtual--;
    }
}

//----------------------------------------------------------------------------
/// Apaga vari�veis na pilha de v a VarAtual-1
void Instr::ApagarRet(TVariavel * v)
{
    for (TVariavel * var = VarAtual-1; var>=v; var--)
    {
        if (var->tamanho==0)
            continue;
        DadosTopo = (char*)var->endvar;
        if (var->endvar == VarAtual->endvar)
            VarAtual->tamanho = var->tamanho;
        else
            var->Apagar();
    }
    *v = *VarAtual;
    VarAtual = v;
    if (VarAtual->tamanho)
    {
        char * p = DadosTopo;
        if (VarAtual->defvar[2] != cTxtFixo)
        {
            if ((VarAtual->tamanho&1)==0)
                p += ((p-Instr::DadosPilha)&1);
            if ((VarAtual->tamanho&3)==0)
                p += ((p-Instr::DadosPilha)&2);
        }
        if (p < VarAtual->endvar)
            VarAtual->Mover(p, 0, 0);
        DadosTopo = (char*)VarAtual->endvar + VarAtual->tamanho;
    }
}
//----------------------------------------------------------------------------
/// Inicializa��o de varfunc
/** Prepara para executar varfunc
   @param varini Procura vari�veis varfunc a partir de varini
   @return true se vai executar varfunc, false se n�o vai
*/
bool Instr::VarFuncIni(TVariavel * varini)
{
    for (TVariavel * v=VarAtual; v>=varini; v--)
        if (v->defvar[2] == cVarFunc)
        {
            if (FuncAtual+1 >= FuncFim)
                return false;
        // Cria fun��o
            FuncAtual++;
            FuncAtual->linha = v->defvar + Num16(v->defvar);
            FuncAtual->este = (TObjeto*)v->endvar;
            FuncAtual->expr = 0;
            FuncAtual->inivar = VarAtual + 1;
            FuncAtual->fimvar = VarAtual + 1;
            FuncAtual->numarg = 0;
            FuncAtual->tipo = 1;
            return true;
        }
    return false;
}

//----------------------------------------------------------------------------
/// Finaliza��o de varfunc
/** Coloca �ltima vari�vel da pilha no lugar da �ltima vari�vel cVarFunc
    @return true se conseguiu, false se mem�ria insuficiente em DadosPilha
*/
bool Instr::VarFuncFim()
{
    TVariavel * vfunc = VarAtual;
    char * endini = 0;

// Procura �ltima vari�vel cVarFunc da pilha
    while (true)
    {
        vfunc--;
        assert(vfunc >= VarPilha);
        if (vfunc->defvar[2] == cVarFunc) // Se achou varfunc...
            break;
        if (vfunc->tamanho == 0) // Se est� definida em outro lugar...
            continue;
        endini = (char*)vfunc->endvar;
        if (endini != VarAtual->endvar) // Se n�o � a mesma vari�vel...
            continue;
        VarAtual->tamanho = vfunc->tamanho; // VarAtual � a primeira
        vfunc->tamanho = 0; // A outra � c�pia
        endini = 0;         // Por enquanto est� na ordem correta em DadosPilha
    }
    assert(vfunc->tamanho==0);

// Verifica se precisa acertar DadosPilha
    if (VarAtual->tamanho==0 || // Se vari�vel n�o est� em DadosPilha
            endini==0)  // Se vari�veis ent�o na ordem correta em DadosPilha
    {
        *vfunc = *VarAtual;
        VarAtual--;
        return true;
    }

// Move vari�veis VarAtual a (vfunc+1) para o fim da �rea de dados
    char * destino = DadosFim;
    for (TVariavel * v = VarAtual; v>vfunc; v--)
        if (v->tamanho)
        {
            char * origem = (char*)v->endvar;
            destino -= (v->tamanho * 4) & ~3;
            if (v != VarAtual)
                DadosTopo = origem;
            if (DadosTopo > destino)
                return false;
            v->Mover(destino, 0, 0);
            for (TVariavel * vtemp = VarAtual; vtemp>vfunc; vtemp--)
                if (vtemp->endvar == origem)
                    vtemp->endvar = destino;
        }

// Move VarAtual para vfunc em VarPilha
    *vfunc = *VarAtual;
    VarAtual--;

// Move vari�veis vfunc a VarAtual para onde dever�o ficar
    for (TVariavel * v = vfunc; v<=VarAtual; v++)
        if (v->tamanho)
        {
            if (v->defvar[2] != cTxtFixo) // Alinhamento conforme vari�vel
            {
                if ((v->tamanho&1)==0)
                    endini += ((endini-Instr::DadosPilha)&1);
                if ((v->tamanho&3)==0)
                    endini += ((endini-Instr::DadosPilha)&2);
            }
            char * origem = (char*)v->endvar;
            v->Mover(endini, 0, 0);
            for (TVariavel * vtemp = vfunc; vtemp<=VarAtual; vtemp++)
                if (vtemp->endvar == origem)
                    vtemp->endvar = endini;
            endini += v->tamanho;
        }
    DadosTopo = endini;
    return true;
}

//----------------------------------------------------------------------------
/// Procura valor (vide Instr::Expressao) em express�o num�rica
/** @return endere�o do valor encontrado ou 0 se n�o encontrou
 *  @note N�o procura entre ex_varini e ex_varfim
 */
const char * Instr::ProcuraExpr(const char * expr, int valor)
{
    int contagem=0;
    while (true)
    {
        //printf(">> %d %s\n", contagem, NomeExpr(*expr)); fflush(stdout);
        if (*expr==valor && contagem<=0)
            return expr;
        switch (*expr++)
        {
        case ex_fim:
        case ex_coment:
            return 0;
        case ex_arg:    // In�cio da lista de argumentos
        case ex_abre:   // Abre colchetes; segue express�o num�rica + ex_fecha
            break;
        case ex_varabre:
            contagem++;
            break;
        case ex_varini: // Ser� fechado com ex_varfim
            contagem++;
        case ex_fecha:  // Aberto com ex_abre
        case ex_ponto:  // Aberto com ex_arg
        case ex_doispontos:
            while (*expr!=ex_arg && *expr!=ex_abre && *expr!=ex_varfim)
            {
                assert(*expr!=0);
                expr++;
            }
            break;
        case ex_varfim:
            contagem--;
            break;

        case ex_nulo:
            break;
        case ex_txt:
            while (*expr)
                expr++;
            expr++;
            break;
        case ex_num32p:
        case ex_num32n:
            expr+=2;
        case ex_num16p:
        case ex_num16n:
            expr++;
        case ex_num8p:
        case ex_num8n:
            expr++;
        case ex_num0:
        case ex_num1:
            while (*expr>=ex_div1 && *expr<=ex_div6)
                expr++;
            break;

        case exo_virgula:
        case exo_neg:
        case exo_exclamacao:
        case exo_mul:
        case exo_div:
        case exo_porcent:
        case exo_add:
        case exo_sub:
        case exo_menor:
        case exo_menorigual:
        case exo_maior:
        case exo_maiorigual:
        case exo_igual:
        case exo_igual2:
        case exo_diferente:
        case exo_igualmul:
        case exo_igualdiv:
        case exo_igualporcent:
        case exo_igualadd:
        case exo_igualsub:
            break;
        case exo_e:
        case exo_ou:
            contagem--;
            break;
        case exo_ee:
        case exo_ouou:
            contagem++;
            break;
        default:
            assert(0);
        }
    }
}

//----------------------------------------------------------------------------
/// Obt�m o nome de um valor de Instr::Comando
/** @param  valor Valor de Instr::Comando
 *  @return nome, ou 0 se n�o encontrou
 */
const char * Instr::NomeComando(int valor)
{
    switch ((Instr::Comando)valor)
    {
    case cHerda:            return "cHerda";
    case cExpr:             return "cExpr";
    case cComent:           return "cComent";

    case cSe:               return "cSe";
    case cSenao1:           return "cSenao1";
    case cSenao2:           return "cSenao2";
    case cFimSe:            return "cFimSe";
    case cEnquanto:         return "cEnquanto";
    case cEFim:             return "cEFim";
    case cRet1:             return "cRet1";
    case cRet2:             return "cRet2";
    case cSair:             return "cSair";
    case cContinuar:        return "cContinuar";
    case cTerminar:         return "cTerminar";

    case cVariaveis:        return "cVariaveis";
    case cTxt1:             return "cTxt1";
    case cTxt2:             return "cTxt2";
    case cInt1:             return "cInt1";
    case cInt8:             return "cInt8";
    case cUInt8:            return "cUInt8";
    case cInt16:            return "cInt16";
    case cUInt16:           return "cUInt16";
    case cInt32:            return "cInt32";
    case cUInt32:           return "cUInt32";
    case cIntInc:           return "cIntInc";
    case cIntDec:           return "cIntDec";
    case cReal:             return "cReal";
    case cRef:              return "cRef";
    case cConstNulo:        return "cConstNulo";
    case cConstTxt:         return "cConstTxt";
    case cConstNum:         return "cConstNum";
    case cConstExpr:        return "cConstExpr";
    case cFunc:             return "cFunc";
    case cVarFunc:          return "cVarFunc";

    case cListaObj:         return "cListaObj";
    case cListaItem:        return "cListaItem";
    case cListaTxt:         return "cListaTxt";
    case cListaMsg:         return "cListaMsg";
    case cNomeObj:          return "cNomeObj";
    case cLog:              return "cLog";
    case cIntTempo:         return "cIntTempo";
    case cSocket:           return "cSocket";
    case cServ:             return "cServ";
    case cSalvar:           return "cSalvar";
    case cProg:             return "cProg";
    case cIndice:           return "cIndice";

    case cTxtFixo:          return "cTxtFixo";
    case cVarNome:          return "cVarNome";
    case cVarInicio:        return "cVarInicio";
    case cVarClasse:        return "cVarClasse";
    case cVarObjeto:        return "cVarObjeto";
    case cVarInt:           return "cVarInt";
    case cTotalComandos:    break;
    }
    return 0;
}

//----------------------------------------------------------------------------
/// Obt�m o nome de um valor de Instr::Expressao
/** @param  valor Valor de Instr::Expressao
 *  @return nome, ou 0 se n�o encontrou
 */
const char * Instr::NomeExpr(int valor)
{
    switch ((Instr::Expressao)valor)
    {
    case ex_fim:            return "ex_fim";
    case ex_coment:         return "ex_coment";

    case ex_barra_n:        return "ex_barra_n";
    case ex_barra_b:        return "ex_barra_b";
    case ex_barra_c:        return "ex_barra_c";
    case ex_barra_d:        return "ex_barra_d";

    case ex_varini:         return "ex_varini";
    case ex_varfim:         return "ex_varfim";
    case ex_doispontos:     return "ex_doispontos";
    case ex_ponto:          return "ex_ponto";
    case ex_arg:            return "ex_arg";
    case ex_varabre:        return "ex_varabre";
    case ex_abre:           return "ex_abre";
    case ex_fecha:          return "ex_fecha";

    case ex_nulo:           return "ex_nulo";
    case ex_txt:            return "ex_txt";
    case ex_num0:           return "ex_num0";
    case ex_num1:           return "ex_num1";
    case ex_num8p:          return "ex_num8p";
    case ex_num16p:         return "ex_num16p";
    case ex_num32p:         return "ex_num32p";
    case ex_num8n:          return "ex_num8n";
    case ex_num16n:         return "ex_num16n";
    case ex_num32n:         return "ex_num32n";
    case ex_div1:           return "ex_div1";
    case ex_div2:           return "ex_div2";
    case ex_div3:           return "ex_div3";
    case ex_div4:           return "ex_div4";
    case ex_div5:           return "ex_div5";
    case ex_div6:           return "ex_div6";

    case exo_ini:           return "exo_ini";
    case exo_virgula:       return "exo_virgula";
    case exo_neg:           return "exo_neg";
    case exo_exclamacao:    return "exo_exclamacao";
    case exo_mul:           return "exo_mul";
    case exo_div:           return "exo_div";
    case exo_porcent:       return "exo_porcent";
    case exo_add:           return "exo_add";
    case exo_sub:           return "exo_sub";
    case exo_menor:         return "exo_menor";
    case exo_menorigual:    return "exo_menorigual";
    case exo_maior:         return "exo_maior";
    case exo_maiorigual:    return "exo_maiorigual";
    case exo_igual:         return "exo_igual";
    case exo_igual2:        return "exo_igual2";
    case exo_diferente:     return "exo_diferente";
    case exo_e:             return "exo_e";
    case exo_ou:            return "exo_ou";
    case exo_igualmul:      return "exo_igualmul";
    case exo_igualdiv:      return "exo_igualdiv";
    case exo_igualporcent:  return "exo_igualporcent";
    case exo_igualadd:      return "exo_igualadd";
    case exo_igualsub:      return "exo_igualsub";
    case exo_fim:           return "exo_fim";
    case exo_ee:            return "exo_ee";
    case exo_ouou:          return "exo_ouou";

    case ex_var1:           return "ex_var1";
    case ex_var2:           return "ex_var2";
    case ex_colchetes:      return "ex_colchetes";
    case ex_parenteses:     return "ex_parenteses";
    }
    return 0;
}