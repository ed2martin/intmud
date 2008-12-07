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
#include "classe.h"
#include "objeto.h"
#include "misc.h"

/** @defgroup exec Instr::Exec - Algoritmo para executar fun��es

@par Algoritmo geral para executar uma fun��o:

@verbatim
Localiza o objeto e a fun��o
Coloca argumentos na pilha
Executa fun��o
Para cada objeto marcado para exclus�o:
    Executa fun��o fim
    Exclui objeto
Para cada modifica��o agendada nas classes:
    Se for excluir classe:
        Enquanto a classe possuir algum objeto:
            Executa fun��o fim do primeiro objeto da classe
            Exclui objeto
        Exclui classe
    Se for criar classe:
        Cria classe
        Executa fun��o ini da classe
        Para cada objeto marcado para exclus�o:
            Executa fun��o fim
            Exclui objeto
    Se for modificar classe:
        Modifica a classe
    Se for salvar mapa:
        Salva o mapa em arquivo
    Retira modifica��o da lista
@endverbatim

@par Valores de Instr::Comando usados somente aqui:

- cVarInicio  -> Esperando texto logo ap�s ex_varini
- cVarClasse  -> Leu ':', TVariavel::endvar = endere�o do objeto TClasse
- cVarObjeto  -> TVariavel::endvar = endere�o do objeto TObjeto

@par Algoritmo simplificado - apenas a parte de executar uma fun��o:

@verbatim
Limpa pilha de vari�veis
Cria vari�vel cNulo no topo da pilha
Limpa pilha de fun��es
Cria fun��o referente ao que deve ser executado
Cria argumentos da fun��o: vari�veis no topo da pilha

(in�cio)
Se FuncAtual->expr == 0:
  *** Est� processando uma linha:
  Se for fim da fun��o ou instru��o ret sem par�metros:
     Apaga vari�veis criadas na fun��o (a partir de FuncAtual->endvar)
     Coloca NULO na pilha de vari�veis
     Se n�o houver fun��o anterior:
       Fim do algoritmo
     Apaga fun��o atual (FuncAtual--)
     Vai para (in�cio)
  Se for vari�vel:
     Cria vari�vel local da fun��o atual
  Se linha possui express�o num�rica:
     FuncAtual->expr = in�cio da express�o num�rica
     FuncAtual->exprvar = VarAtual + 1 (para desempilhar vari�veis depois)
     Vai para (in�cio)
  Processa instru��o que n�o requer express�o num�rica
  Passa para pr�xima instru��o
  Vai para (in�cio)
Se FuncAtual->expr != 0:
  *** Est� processando uma express�o num�rica
  *** FuncAtual->expr[0] cont�m a pr�xima instru��o da express�o num�rica
  Se for ex_fim:
     FuncAtual->expr = 0
     Processa instru��o que depende de express�o num�rica
     Passa para pr�xima instru��o
     Apaga vari�veis de FuncAtual->exprvar em diante
     Vai para (in�cio)
  Se for um valor fixo (n�mero, texto ou NULO):
     Coloca vari�vel na pilha de vari�veis
     Avan�a FuncAtual->expr
     Vai para (in�cio)
  Se for um operador:
     Processa operador conforme as vari�veis do topo da pilha
     Os operadores & e | podem avan�ar na express�o num�rica
     Avan�a FuncAtual->expr
     Vai para (in�cio)
  *** A partir daqui processa fun��es e vari�veis
  *** Cria-se uma vari�vel cNomeVar, que cont�m parte do nome da vari�vel
  *** Cria-se uma vari�vel em seguida, que cont�m o resultado anterior
  *** Isso torna poss�vel processar v�rias vari�veis ligadas por ponto
  *** Exemplo: a.b.c
  Se for ex_varini ou ex_varabre:
     Cria vari�vel cNomeVar na pilha de vari�veis para armazenar o nome
     Cria vari�vel cVarInicio na pilha de vari�veis
     Avan�a FuncAtual->Expr
     Se era ex_varini:
       Copia o que puder do nome em (um texto) para �ltima vari�vel cNomeVar
     Vai para (in�cio)
  Se for ex_arg (argumento da fun��o) ou ex_abre (abre colchetes):
     Avan�a FuncAtual->expr
     Vai para (in�cio)
  Se for ex_fecha (fecha colchetes):
     Avan�a FuncAtual->expr
     Sendo que: X=�ltima vari�vel cNomeVar da pilha e Y=vari�vel ap�s X
     Transforma em texto as vari�veis ap�s Y e anoma em X
     Apaga vari�veis ap�s Y
     Copia o que puder do nome em FuncAtual->expr (um texto) para X
     Vai para (in�cio)
  Se for ex_varfim:
     Avan�a FuncAtual->expr
     Sendo que: X=�ltima vari�vel cNomeVar da pilha e Y=vari�vel ap�s X
     Se vari�vel Y for cVarInicio, cVarClasse ou cVarObjeto:
       Apaga vari�veis de X em diante
       Cria vari�vel cNulo
     Caso contr�rio:
       Apaga vari�veis ap�s Y
       Apaga vari�vel X
       Vari�vel Y passa a ocupar o lugar da vari�vel X
     Vai para (in�cio)
  Se for ex_doispontos:
     Procura �ltima vari�vel cNomeVar da pilha
     Se nome de classe em cNomeVar existe e pr�xima vari�vel for cVarInicio:
       Muda a pr�xima vari�vel para cVarClasse
       Anota o endere�o da classe
     Caso contr�rio:
       Apaga da vari�vel cNomeVar em diante
       Cria vari�vel cNulo na pilha de vari�veis
       Avan�a FuncAtual->expr at� ex_varfim
     Avan�a FuncAtual->expr
     Vai para (in�cio)
  Se for ex_ponto:
     Avan�a FuncAtual->expr
     Procura �ltima vari�vel cNomeVar da pilha
     Checa pr�xima vari�vel e texto em cNomeVar:
        cVarInicio -> 1. Verifica se � vari�vel/comando interno do programa
                      2. Verifica se � vari�vel local da fun��o
                      3. Verifica se � vari�vel ou fun��o do objeto
        cVarClasse -> Verifica se � fun��o da classe especificada
        cVarObjeto -> Verifica se � vari�vel ou fun��o do objeto em cVarObjeto
     Depois de decidido que a��o tomar:
       Limpa texto em cNomeVar
       Copia o que puder do nome em FuncAtual->expr (um texto) para cNomeVar
     Executa a��o, que pode ser:
     1. Processar fun��o interna do programa:
        Apaga vari�veis ap�s cNomeVar
        Cria vari�vel com o resultado
     2. Executar uma fun��o de uma classe:
        FuncAtual++
        FuncAtual->linha = primeira linha ap�s o nome da fun��o
        FuncAtual->este = endere�o do objeto "este", ou NULL
        FuncAtual->expr = 0
        FuncAtual->exprvar = 0
        FuncAtual->endvar = primeira vari�vel ap�s cNomeVar
        FuncAtual->numvar = 0
        FuncAtual->numarg = n�mero de vari�veis ap�s cNomeVar
     3. Vari�vel definida em uma classe:
        Apaga vari�veis a partir da anterior a cNomeVar
        Anota vari�vel no topo da pilha de vari�veis
     4. Uma vari�vel qualquer:
        Consulta a vari�vel sobre o que fazer; pode cair em 2 ou 3
  Nunca dever� chegar aqui; executar assert(0)
@endverbatim
*/

//------------------------------------------------------------------------------
// Pilha de dados - para criar/apagar vari�veis
char * const Instr::DadosPilha = new char[0x10000];
char * const Instr::DadosFim   = Instr::DadosPilha + 0x10000;
char * Instr::DadosTopo  = Instr::DadosPilha;

// Pilha de vari�veis
TVariavel * const Instr::VarPilha   = new TVariavel[500];
TVariavel * const Instr::VarFim     = Instr::VarPilha + 500;
TVariavel * Instr::VarAtual   = Instr::VarPilha;

// Pilha de fun��es
Instr::ExecFunc * const Instr::FuncPilha  = new Instr::ExecFunc[40];
Instr::ExecFunc * const Instr::FuncFim = Instr::FuncPilha + 40;
Instr::ExecFunc * Instr::FuncAtual  = Instr::FuncPilha;

//----------------------------------------------------------------------------
static const char InstrNulo[] = { 7, 0, Instr::cConstNulo, 0, 0, '+', 0 };
static const char InstrDouble[] = { 7, 0, Instr::cReal, 0, 0, '+', 0 };
static const char InstrInt32[] = { 7, 0, Instr::cInt32, 0, 0, '+', 0 };
static const char InstrUInt32[] = { 7, 0, Instr::cUInt32, 0, 0, '+', 0 };
static const char InstrTxtFixo[] = { 7, 0, Instr::cTxtFixo, 0, 0, '+', 0 };

//----------------------------------------------------------------------------
/// Prepara para executar
/**
 *  @param classe Endere�o da classe (objeto este � NULO)
 *  @param func Nome da fun��o (string ASCIIZ)
 *  @retval true Fun��o encontrada; pode executar
 *  @retval false Fun��o n�o existe, n�o pode executar
 *  @sa exec
 */
bool Instr::ExecIni(TClasse * classe, const char * func)
{
    static char vazio[2] = { 0, 0 };
    int indice = classe->IndiceNome(func);
    if (indice<0)
        return false;
    char * instr = classe->InstrVar[indice];
    if (instr[0]==0 && instr[1]==0)
        return false;
    if (instr[2]!=cFunc && instr[2]!=cVarFunc)
        return false;
    instr += Num16(instr);
    if (instr[0] && instr[1])
        switch (instr[2])
        {
        case cFunc:
        case cVarFunc:
        case cConstNulo:
        case cConstTxt:
        case cConstNum:
        case cConstExpr:
            instr = vazio;
        }
// Acerta pilhas
    DadosTopo = DadosPilha; // Limpa pilha de dados
    VarAtual = VarPilha;    // Limpa pilha de vari�veis
    VarAtual->Limpar();     // Vari�vel atual � nulo
    VarAtual->defvar = InstrNulo;
    FuncAtual = FuncPilha;  // Limpa pilha de fun��es
    FuncAtual->este = 0;    // Nenhum objeto
    FuncAtual->linha = instr;// Primeira instru��o da fun��o
    FuncAtual->expr = 0;    // N�o est� processando express�o num�rica
    FuncAtual->exprvar = 0;
    FuncAtual->endvar = VarAtual + 1; // Endere�o do primeiro argumento
    FuncAtual->numvar = 0;  // N�mero de vari�veis da fun��o
    FuncAtual->numarg = 0;  // N�mero de argumentos da fun��o
    return true;
}

//----------------------------------------------------------------------------
/// Prepara para executar
/**
 *  @param este Endere�o do objeto "este"
 *  @param func Nome da fun��o (string ASCIIZ)
 *  @retval true Fun��o encontrada; pode executar
 *  @retval false Fun��o n�o existe, n�o pode executar
 *  @sa exec
 */
bool Instr::ExecIni(TObjeto * este, const char * func)
{
    if (este==0)
        return false;
    if (ExecIni(este->Classe, func)==false)
        return false;
    FuncAtual->este = este;
    return true;
}

//----------------------------------------------------------------------------
static bool RetornoErro(void)
{
// Apaga vari�veis da pilha
    Instr::ApagarVar(Instr::VarPilha+1);
// Cria NULO na pilha
    Instr::VarAtual++;
    Instr::VarAtual->Limpar();
    Instr::VarAtual->defvar = InstrNulo;
    return false;
}

//----------------------------------------------------------------------------
/// Executa fun��o
/** @return true se executou normal, false se cancelou por falta de mem�ria
 *  @note Independente de retornar true ou false, executar ExecFim() depois
 */
bool Instr::ExecX()
{
    while (true)
    {
        if (FuncAtual->expr==0)
        {
    // Est� processando uma linha

        // Retorno sem par�metros
            if (FuncAtual->linha[0]==0 && FuncAtual->linha[1]==0)
                FuncAtual->linha = 0;
            else
                switch (FuncAtual->linha[2])
                {
                case cRet1:
                case cFunc:
                case cVarFunc:
                case cConstNulo:
                case cConstTxt:
                case cConstNum:
                case cConstExpr:
                    FuncAtual->linha = 0;
                }
            if (FuncAtual->linha == 0)
            {
            // Apaga vari�veis da fun��o
                ApagarVar(FuncAtual->endvar);
            // Cria NULO na pilha de vari�vels (retorno da fun��o)
                if (VarAtual >= VarFim-1)
                    return RetornoErro();
                VarAtual++;
                VarAtual->Limpar();
                VarAtual->defvar = InstrNulo;
            // Apaga fun��o
                if (FuncAtual==FuncPilha)
                    return true;
                FuncAtual--;
                continue;
            }

        // Vari�vel da fun��o
            if (FuncAtual->linha[2] > cVariaveis)
            {
                if (!CriarVar(FuncAtual->linha))
                    return RetornoErro();
                continue;
            }

        // Processa instru��o
            switch (FuncAtual->linha[2])
            {
            case cSe:   // Processa express�o num�rica
            case cSenao2:
            case cEnquanto:
                FuncAtual->expr = FuncAtual->linha + 5;
                FuncAtual->exprvar = VarAtual + 1;
                break;
            case cRet2: // Processa express�o num�rica
            case cExpr:
                FuncAtual->expr = FuncAtual->linha + 3;
                FuncAtual->exprvar = VarAtual + 1;
                break;
            case cComent: // Coment�rio
                FuncAtual->linha += Num16(FuncAtual->linha);
                break;
            case cSenao1:
            case cFimSe:
            case cEFim:
            case cSair:
            case cContinuar:
            case cTerminar:



                FuncAtual->linha += Num16(FuncAtual->linha);
                break;
            default:  // Instru��o desconhecida
                assert(0);
            }
            continue;
        }

    // Est� processando express�o num�rica
    // FuncAtual->expr[0] cont�m a pr�xima instru��o da express�o num�rica
        switch (FuncAtual->expr[0])
        {
        case ex_fim:
            FuncAtual->expr = 0;
            // Processa instru��o que depende de express�o num�rica
            FuncAtual->linha += Num16(FuncAtual->linha);
            ApagarVar(FuncAtual->exprvar);
            break;
        case ex_nulo:
            FuncAtual->expr++;
            if (VarAtual >= VarFim-1)
                return RetornoErro();
            VarAtual++;
            VarAtual->Limpar();
            VarAtual->defvar = InstrNulo;
            break;
        case ex_txt:
            FuncAtual->expr++;
            if (VarAtual >= VarFim-1)
                return RetornoErro();
            VarAtual++;
            VarAtual->Limpar();
            VarAtual->defvar = InstrTxtFixo;
            VarAtual->endvar = FuncAtual->expr;
            while (FuncAtual->expr)
                FuncAtual->expr++;
            VarAtual++;
            break;
        case ex_num0:
            if (!CriarVar(InstrUInt32))
                return RetornoErro();
            FuncAtual->expr++;
            break;
        case ex_num1:
            if (!CriarVar(InstrUInt32))
                return RetornoErro();
            *(unsigned char*)VarAtual->endvar=1;
            FuncAtual->expr++;
            break;
        case ex_num8p:
            if (!CriarVar(InstrUInt32))
                return RetornoErro();
            *(unsigned char*)VarAtual->endvar = FuncAtual->expr[1];
            FuncAtual->expr += 2;
            break;
        case ex_num16p:
            if (!CriarVar(InstrUInt32))
                return RetornoErro();
            *(unsigned char*)VarAtual->endvar = FuncAtual->expr[1];
            *((unsigned char*)VarAtual->endvar+1) = FuncAtual->expr[2];
            FuncAtual->expr += 3;
            break;
        case ex_num32p:
            if (!CriarVar(InstrUInt32))
                return RetornoErro();
            memcpy(VarAtual->endvar, FuncAtual->expr+1, 4);
            FuncAtual->expr += 5;
            break;
        case ex_num8n:
            if (!CriarVar(InstrInt32))
                return RetornoErro();
            VarAtual->setInt(-(int)(unsigned char)FuncAtual->expr[1]);
            FuncAtual->expr += 2;
            break;
        case ex_num16n:
            if (!CriarVar(InstrInt32))
                return RetornoErro();
            VarAtual->setInt(-(int)Num16(FuncAtual->expr+1));
            FuncAtual->expr += 3;
            break;
        case ex_num32n:
            if (!CriarVar(InstrDouble))
                return RetornoErro();
            VarAtual->setDouble(-(double)Num32(FuncAtual->expr+1));
            FuncAtual->expr += 5;
            break;
        case ex_div1:
        case ex_div2:
        case ex_div3:
        case ex_div4:
        case ex_div5:
        case ex_div6:
            {
                double valor = VarAtual->getDouble();
                switch (*FuncAtual->expr++)
                {
                case ex_div1: valor/=10; break;
                case ex_div2: valor/=100; break;
                case ex_div3: valor/=1000; break;
                case ex_div4: valor/=10000; break;
                case ex_div5: valor/=100000; break;
                default:      valor/=1000000; break;
                }
                if (VarAtual->defvar[2] != cReal)
                {
                    ApagarVar(VarAtual);
                    CriarVar(InstrDouble);
                }
                VarAtual->setDouble(valor);
            }

        default:
            assert(0);
        }
/*
  Se for um operador:
     Processa operador conforme as vari�veis do topo da pilha
     Os operadores & e | podem avan�ar na express�o num�rica
     Avan�a FuncAtual->expr
     Vai para (in�cio)
  *** A partir daqui processa fun��es e vari�veis
  *** Cria-se uma vari�vel cNomeVar, que cont�m parte do nome da vari�vel
  *** Cria-se uma vari�vel em seguida, que cont�m o resultado anterior
  *** Isso torna poss�vel processar v�rias vari�veis ligadas por ponto
  *** Exemplo: a.b.c
  Se for ex_varini ou ex_varabre:
     Cria vari�vel cNomeVar na pilha de vari�veis para armazenar o nome
     Cria vari�vel cVarInicio na pilha de vari�veis
     Avan�a FuncAtual->Expr
     Se era ex_varini:
       Copia o que puder do nome em (um texto) para �ltima vari�vel cNomeVar
     Vai para (in�cio)
  Se for ex_arg (argumento da fun��o) ou ex_abre (abre colchetes):
     Avan�a FuncAtual->expr
     Vai para (in�cio)
  Se for ex_fecha (fecha colchetes):
     Avan�a FuncAtual->expr
     Sendo que: X=�ltima vari�vel cNomeVar da pilha e Y=vari�vel ap�s X
     Transforma em texto as vari�veis ap�s Y e anoma em X
     Apaga vari�veis ap�s Y
     Copia o que puder do nome em FuncAtual->expr (um texto) para X
     Vai para (in�cio)
  Se for ex_varfim:
     Avan�a FuncAtual->expr
     Sendo que: X=�ltima vari�vel cNomeVar da pilha e Y=vari�vel ap�s X
     Se vari�vel Y for cVarInicio, cVarClasse ou cVarObjeto:
       Apaga vari�veis de X em diante
       Cria vari�vel cNulo
     Caso contr�rio:
       Apaga vari�veis ap�s Y
       Apaga vari�vel X
       Vari�vel Y passa a ocupar o lugar da vari�vel X
     Vai para (in�cio)
  Se for ex_doispontos:
     Procura �ltima vari�vel cNomeVar da pilha
     Se nome de classe em cNomeVar existe e pr�xima vari�vel for cVarInicio:
       Muda a pr�xima vari�vel para cVarClasse
       Anota o endere�o da classe
     Caso contr�rio:
       Apaga da vari�vel cNomeVar em diante
       Cria vari�vel cNulo na pilha de vari�veis
       Avan�a FuncAtual->expr at� ex_varfim
     Avan�a FuncAtual->expr
     Vai para (in�cio)
  Se for ex_ponto:
     Avan�a FuncAtual->expr
     Procura �ltima vari�vel cNomeVar da pilha
     Checa pr�xima vari�vel e texto em cNomeVar:
        cVarInicio -> 1. Verifica se � vari�vel/comando interno do programa
                      2. Verifica se � vari�vel local da fun��o
                      3. Verifica se � vari�vel ou fun��o do objeto
        cVarClasse -> Verifica se � fun��o da classe especificada
        cVarObjeto -> Verifica se � vari�vel ou fun��o do objeto em cVarObjeto
     Depois de decidido que a��o tomar:
       Limpa texto em cNomeVar
       Copia o que puder do nome em FuncAtual->expr (um texto) para cNomeVar
     Executa a��o, que pode ser:
     1. Processar fun��o interna do programa:
        Apaga vari�veis ap�s cNomeVar
        Cria vari�vel com o resultado
     2. Executar uma fun��o de uma classe:
        FuncAtual++
        FuncAtual->linha = primeira linha ap�s o nome da fun��o
        FuncAtual->este = endere�o do objeto "este", ou NULL
        FuncAtual->expr = 0
        FuncAtual->exprvar = 0
        FuncAtual->endvar = primeira vari�vel ap�s cNomeVar
        FuncAtual->numvar = 0
        FuncAtual->numarg = n�mero de vari�veis ap�s cNomeVar
     3. Vari�vel definida em uma classe:
        Apaga vari�veis a partir da anterior a cNomeVar
        Anota vari�vel no topo da pilha de vari�veis
     4. Uma vari�vel qualquer:
        Consulta a vari�vel sobre o que fazer; pode cair em 2 ou 3
  Nunca dever� chegar aqui; executar assert(0) */

    }
    return RetornoErro();
}

//----------------------------------------------------------------------------
/// Executa procedimentos ap�s ExecX()
void Instr::ExecFim()
{
    ApagarVar(VarAtual+1);


    // Se apagar alguma classe aqui, acertar tamb�m fun��o main()

}
