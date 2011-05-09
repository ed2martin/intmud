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
#include <assert.h>
#include "instr.h"
#include "variavel.h"
#include "misc.h"

//------------------------------------------------------------------------------
/// Retorna um número que corresponde à prioridade do operador
/** @param operador Operador em Instr::Expressao
    @retval 2-0x2F Número que corresponde à prioridade do operador;
            20 é atribuição (procesado da direita para a esquerda)
    @retval 0 Operador inválido */
int Instr::Prioridade(int operador)
{
    switch (operador)
    {
    case exo_virgula:    return 22;
    case exo_neg:        return 2;
    case exo_exclamacao: return 2;
    case exo_b_comp:     return 2;
    case exo_mul:        return 3;
    case exo_div:        return 3;
    case exo_porcent:    return 3;
    case exo_add:        return 4;
    case exo_sub:        return 4;
    case exo_b_shl:      return 5;
    case exo_b_shr:      return 5;
    case exo_b_e:        return 6;
    case exo_b_ouou:     return 7;
    case exo_b_ou:       return 8;
    case exo_menor:      return 9;
    case exo_menorigual: return 9;
    case exo_maior:      return 9;
    case exo_maiorigual: return 9;
    case exo_igual:      return 10;
    case exo_igual2:     return 10;
    case exo_diferente:  return 10;
    case exo_e:          return 11;
    case exo_ou:         return 12;
    case exo_atrib:      return 20;
    case exo_igualmul:   return 20;
    case exo_igualdiv:   return 20;
    case exo_igualporcent: return 10;
    case exo_igualadd:   return 20;
    case exo_igualsub:   return 20;
    }
    return 0;
}

//------------------------------------------------------------------------------
/// Verifica se instrução codificada é herda e contém a classe especificada
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
// Instrução Herda
    if (cod==cHerda)
    {
        if (esperando!=0)
            return "Instrução Herda deve ser a primeira da classe";
        esperando=1;
        return 0;
    }
    if (esperando==0)
        esperando=1;
// Comentário
    if (cod==cComent)
        return 0;
// Função
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
// Variável
    if (cod >= cVariaveis)
    {
        if (esperando == 3)
            return "Variável deve ser definida no início da função";
        if (esperando > 3)
            return "Variável não pertence a uma classe ou uma função";
        return 0;
    }
// Instrução
    if (esperando<2 || esperando>3)
        return "Instrução não pertence a uma função";
    esperando=3;
    return 0;
}

//----------------------------------------------------------------------------
/// Cria variável int com um valor definido
/**
 *  @param valor Valor numérico da variável
 *  @return true se conseguiu criar, false se memória insuficiente */
bool Instr::CriarVarInt(int valor)
{
    if (VarAtual >= VarFim-1)
        return false;
    VarAtual++;
    VarAtual->defvar = InstrVarInt;
    VarAtual->valor_int = valor;
    VarAtual->tamanho = 0;
    VarAtual->indice = 0;
    return true;
}

//----------------------------------------------------------------------------
/// Cria variável de texto no topo da pilha de variáveis (Instr::VarPilha)
/**
 *  @param mens A mensagem que será retornada;
 *               Se for 0, apenas aloca memória para o texto
 *  @param tammens Tamanho da mensagem sem o 0 final;
 *                 Se for <0, assume o valor de strlen(mens)
 *  @return true se conseguiu criar, false se memória insuficiente
 *  @note Pode copiar apenas parte do texto se a memória for insuficiente */
bool Instr::CriarVarTexto(const char * mens, int tammens)
{
// Acerta variáveis
    if (VarAtual >= VarFim-1)
        return false;
// Verifica espaço disponível (sem o 0 no final do texto)
    if (tammens<0)
    {
        if (mens==0)
            return false;
        tammens=strlen(mens);
    }
    int disp = DadosFim - DadosTopo - 1;
    if (disp<0)
        return false;
    if (tammens>disp)
        tammens = disp;
// Copia texto
    if (tammens>0 && mens)
        memcpy(DadosTopo, mens, tammens);
    DadosTopo[tammens] = 0;
// Acerta variáveis
    VarAtual++;
    VarAtual->defvar = InstrTxtFixo;
    VarAtual->endvar = DadosTopo;
    VarAtual->tamanho = tammens+1;
    VarAtual->indice = 0;
    DadosTopo += tammens+1;
    return true;
}

//------------------------------------------------------------------------------
/// Cria variável no topo da pilha de variáveis (Instr::VarPilha)
/** @return true se conseguiu criar, false se memória insuficiente */
bool Instr::CriarVar(const char * def)
{
    if (VarAtual >= VarFim-1)
        return false;
// Verifica se tamanho da variável é nulo
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
// Acerta alinhamento do endereço da variável
    char * p = Instr::DadosTopo;
    if ((tam&1)==0)
        p += ((p-Instr::DadosPilha)&1);
    if ((tam&3)==0)
        p += ((p-Instr::DadosPilha)&2);
// Prepara variável
    VarAtual++;
    VarAtual->endvar = p;
    VarAtual->defvar = def;
    VarAtual->indice = (def[endVetor]==0 ? 0 : 0xFF);
    VarAtual->bit = 1;
    VarAtual->tamanho = VarAtual->Tamanho();
// Verifica se memória suficiente
    if (p + VarAtual->tamanho > Instr::DadosFim)
    {
        VarAtual--;
        return false;
    }
// Cria variável
    Instr::DadosTopo = p + VarAtual->tamanho;
    VarAtual->Criar(0, 0);
    return true;
}

//----------------------------------------------------------------------------
/// Apaga variáveis na pilha a partir da variável v
void Instr::ApagarVar(TVariavel * v)
{
    while (VarAtual >= v)
    {
        if (VarAtual->tamanho)
        {
            if (VarAtual->endvar)
                DadosTopo = (char*)VarAtual->endvar;
            VarAtual->Apagar(); // Pode alterar VarAtual->endvar
        }
        VarAtual--;
    }
}

//----------------------------------------------------------------------------
/// Apaga variáveis na pilha de v a VarAtual-1
void Instr::ApagarRet(TVariavel * v)
{
    for (TVariavel * var = VarAtual-1; var>=v; var--)
    {
        if (var->tamanho==0)
            continue;
        DadosTopo = (char*)var->endvar;
        if (var->endvar == VarAtual->endvar) // Se forem a mesma variável
            VarAtual->tamanho = var->tamanho; // Acerta o tamanho de VarAtual
                                // porque está com 0 (é cópia de var)
        else
            var->Apagar(); // Caso contrário, apaga a variável
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
            VarAtual->MoverEnd(p, 0, 0);
        DadosTopo = (char*)VarAtual->endvar + VarAtual->tamanho;
    }
}
//----------------------------------------------------------------------------
/// Inicialização de varfunc
/** Prepara para executar varfunc
   @param varini Procura variáveis varfunc a partir de varini
   @return true se vai executar varfunc, false se não vai
*/
bool Instr::VarFuncIni(TVariavel * varini)
{
    for (TVariavel * v=VarAtual; v>=varini; v--)
        if (v->defvar[2] == cVarFunc)
        {
            if (FuncAtual+1 >= FuncFim)
                return false;
        // Cria função
            FuncAtual++;
            FuncAtual->linha = v->defvar + Num16(v->defvar);
            FuncAtual->este = (TObjeto*)v->endvar;
            FuncAtual->expr = 0;
            FuncAtual->inivar = VarAtual + 1;
            FuncAtual->fimvar = VarAtual + 1;
            FuncAtual->numarg = 0;
            FuncAtual->tipo = 1;
            if (FuncAtual >= FuncPilha)
            {
                FuncAtual->objdebug = FuncAtual[-1].objdebug;
                FuncAtual->funcdebug = FuncAtual[-1].funcdebug;
            }
            else
                FuncAtual->funcdebug = 0;
            return true;
        }
    return false;
}

//----------------------------------------------------------------------------
/// Finalização de varfunc
/** Coloca última variável da pilha no lugar da última variável cVarFunc
    @return true se conseguiu, false se memória insuficiente em DadosPilha
*/
bool Instr::VarFuncFim()
{
    TVariavel * vfunc = VarAtual;
    char * endini = 0;

// Procura última variável cVarFunc da pilha
    while (true)
    {
        vfunc--;
        assert(vfunc >= VarPilha);
        if (vfunc->defvar[2] == cVarFunc) // Se achou varfunc...
            break;
        if (vfunc->tamanho == 0) // Se está definida em outro lugar...
            continue;
        endini = (char*)vfunc->endvar;
        if (endini != VarAtual->endvar) // Se não é a mesma variável...
            continue;
        VarAtual->tamanho = vfunc->tamanho; // VarAtual é a primeira
        vfunc->tamanho = 0; // A outra é cópia
        endini = 0;         // Por enquanto está na ordem correta em DadosPilha
    }
    assert(vfunc->tamanho==0);

// Verifica se precisa acertar DadosPilha
    if (VarAtual->tamanho==0 || // Se variável não está em DadosPilha
            endini==0)  // Se variáveis então na ordem correta em DadosPilha
    {
        *vfunc = *VarAtual;
        VarAtual--;
        return true;
    }

// Move variáveis VarAtual a (vfunc+1) para o fim da área de dados
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
            v->MoverEnd(destino, 0, 0);
            for (TVariavel * vtemp = VarAtual; vtemp>vfunc; vtemp--)
                if (vtemp->endvar == origem)
                    vtemp->endvar = destino;
        }

// Move VarAtual para vfunc em VarPilha
    *vfunc = *VarAtual;
    VarAtual--;

// Move variáveis vfunc a VarAtual para onde deverão ficar
    for (TVariavel * v = vfunc; v<=VarAtual; v++)
        if (v->tamanho)
        {
            if (v->defvar[2] != cTxtFixo) // Alinhamento conforme variável
            {
                if ((v->tamanho&1)==0)
                    endini += ((endini-Instr::DadosPilha)&1);
                if ((v->tamanho&3)==0)
                    endini += ((endini-Instr::DadosPilha)&2);
            }
            char * origem = (char*)v->endvar;
            v->MoverEnd(endini, 0, 0);
            for (TVariavel * vtemp = vfunc; vtemp<=VarAtual; vtemp++)
                if (vtemp->endvar == origem)
                    vtemp->endvar = endini;
            endini += v->tamanho;
        }
    DadosTopo = endini;
    return true;
}

//----------------------------------------------------------------------------
/// Procura valor (vide Instr::Expressao) em expressão numérica
/** @return endereço do valor encontrado ou 0 se não encontrou
 *  @note Não procura entre ex_varini e ex_varfim
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
        case ex_arg:    // Início da lista de argumentos
        case ex_abre:   // Abre colchetes; segue expressão numérica + ex_fecha
            break;
        case ex_varabre:
            contagem++;
            break;
        case ex_varini: // Será fechado com ex_varfim
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
        case exo_b_comp:
        case exo_mul:
        case exo_div:
        case exo_porcent:
        case exo_add:
        case exo_sub:
        case exo_b_shl:
        case exo_b_shr:
        case exo_b_e:
        case exo_b_ouou:
        case exo_b_ou:
        case exo_menor:
        case exo_menorigual:
        case exo_maior:
        case exo_maiorigual:
        case exo_igual:
        case exo_igual2:
        case exo_diferente:
        case exo_atrib:
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
/// Obtém o nome de uma instrução
/** @param  instr Instrução codificada
 *  @return nome, ou "" se não encontrou
 */
const char * Instr::NomeInstr(const char * instr)
{
    static char nome[16];
    switch ((Instr::Comando)instr[2])
    {
    case cHerda:            return "herda";
    case cExpr:             return "";
    case cComent:           return "#";

    case cSe:               return "se";
    case cSenao1:           return "senao1";
    case cSenao2:           return "senao2";
    case cFimSe:            return "fimse";
    case cEnquanto:         return "enquanto";
    case cEFim:             return "efim";
    case cCasoVar:          return "casovar";
    case cCasoSe:           return "casose";
    case cCasoSePadrao:     return "casosepadrao";
    case cCasoFim:          return "casofim";
    case cRet1:             return "ret";
    case cRet2:             return "ret";
    case cSair:             return "sair";
    case cContinuar:        return "continuar";
    case cTerminar:         return "terminar";

    case cVariaveis:        return "";
    case cTxt1:
        sprintf(nome, "txt%d", (unsigned char)instr[endIndice]+1);
        return nome;
    case cTxt2:
        sprintf(nome, "txt%d", (unsigned char)instr[endIndice]+257);
        return nome;
    case cInt1:             return "int1";
    case cInt8:             return "int8";
    case cUInt8:            return "uint8";
    case cInt16:            return "int16";
    case cUInt16:           return "uint16";
    case cInt32:            return "int32";
    case cUInt32:           return "uint32";
    case cIntInc:           return "intinc";
    case cIntDec:           return "intdec";
    case cReal:             return "real";
    case cRef:              return "ref";
    case cConstNulo:
    case cConstTxt:
    case cConstNum:
    case cConstExpr:        return "const";
    case cFunc:             return "func";
    case cVarFunc:          return "varfunc";

    case cListaObj:         return "listaobj";
    case cListaItem:        return "listaitem";
    case cTextoTxt:         return "textotxt";
    case cTextoPos:         return "textopos";
    case cNomeObj:          return "nomeobj";
    case cArqDir:           return "arqdir";
    case cArqLog:           return "arqlog";
    case cArqSav:           return "arqsav";
    case cArqTxt:           return "arqtxt";
    case cIntTempo:         return "inttempo";
    case cTelaTxt:          return "telatxt";
    case cSocket:           return "socket";
    case cServ:             return "serv";
    case cProg:             return "prog";
    case cDebug:            return "debug";
    case cIndiceObj:        return "indiceobj";
    case cIndiceItem:       return "indiceitem";
    case cDataHora:         return "datahora";

    case cTxtFixo:
    case cVarNome:
    case cVarInicio:
    case cVarClasse:
    case cVarObjeto:
    case cVarInt:           return "";
    case cTotalComandos:    break;
    }
    return "";
}

//----------------------------------------------------------------------------
/// Obtém o nome de um valor de Instr::Comando
/** @param  valor Valor de Instr::Comando
 *  @return nome, ou 0 se não encontrou
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
    case cCasoVar:          return "cCasoVar";
    case cCasoSe:           return "cCasoSe";
    case cCasoSePadrao:     return "cCasoSePadrao";
    case cCasoFim:          return "cCasoFim";
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
    case cTextoTxt:         return "cTextoTxt";
    case cTextoPos:         return "cTextoPos";
    case cNomeObj:          return "cNomeObj";
    case cArqDir:           return "cArqDir";
    case cArqLog:           return "cArqLog";
    case cArqSav:           return "cArqSav";
    case cArqTxt:           return "cArqTxt";
    case cIntTempo:         return "cIntTempo";
    case cTelaTxt:          return "cTelaTxt";
    case cSocket:           return "cSocket";
    case cServ:             return "cServ";
    case cProg:             return "cProg";
    case cDebug:            return "cDebug";
    case cIndiceObj:        return "cIndiceObj";
    case cIndiceItem:       return "cIndiceItem";
    case cDataHora:         return "cDataHora";

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
/// Obtém o nome de um valor de Instr::Expressao
/** @param  valor Valor de Instr::Expressao
 *  @return nome, ou 0 se não encontrou
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
    case exo_b_comp:        return "exo_b_comp";
    case exo_mul:           return "exo_mul";
    case exo_div:           return "exo_div";
    case exo_porcent:       return "exo_porcent";
    case exo_add:           return "exo_add";
    case exo_sub:           return "exo_sub";
    case exo_b_shl:         return "exo_b_shl";
    case exo_b_shr:         return "exo_b_shr";
    case exo_b_e:           return "exo_b_e";
    case exo_b_ouou:        return "exo_b_ouou";
    case exo_b_ou:          return "exo_b_ou";
    case exo_menor:         return "exo_menor";
    case exo_menorigual:    return "exo_menorigual";
    case exo_maior:         return "exo_maior";
    case exo_maiorigual:    return "exo_maiorigual";
    case exo_igual:         return "exo_igual";
    case exo_igual2:        return "exo_igual2";
    case exo_diferente:     return "exo_diferente";
    case exo_e:             return "exo_e";
    case exo_ou:            return "exo_ou";
    case exo_atrib:         return "exo_atrib";
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
