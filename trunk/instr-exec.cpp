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
#include "classe.h"
#include "objeto.h"
#include "misc.h"

/** @defgroup exec Instr::Exec - Algoritmo para executar funções

@par Algoritmo geral para executar uma função:

@verbatim
Localiza o objeto e a função
Coloca argumentos na pilha
Executa função
Para cada objeto marcado para exclusão:
    Executa função fim
    Exclui objeto
Para cada modificação agendada nas classes:
    Se for excluir classe:
        Enquanto a classe possuir algum objeto:
            Executa função fim do primeiro objeto da classe
            Exclui objeto
        Exclui classe
    Se for criar classe:
        Cria classe
        Executa função ini da classe
        Para cada objeto marcado para exclusão:
            Executa função fim
            Exclui objeto
    Se for modificar classe:
        Modifica a classe
    Se for salvar mapa:
        Salva o mapa em arquivo
    Retira modificação da lista
@endverbatim

@par Valores de Instr::Comando usados somente aqui:

- cVarInicio  -> Esperando texto logo após ex_varini
- cVarClasse  -> Leu ':', TVariavel::endvar = endereço do objeto TClasse
- cVarObjeto  -> TVariavel::endvar = endereço do objeto TObjeto

@par Algoritmo simplificado - apenas a parte de executar uma função:

@verbatim
Limpa pilha de variáveis
Cria variável cNulo no topo da pilha
Limpa pilha de funções
Cria função referente ao que deve ser executado
Cria argumentos da função: variáveis no topo da pilha

(início)
Se FuncAtual->expr == 0:
  *** Está processando uma linha:
  Se for fim da função ou instrução ret sem parâmetros:
     Apaga variáveis criadas na função (a partir de FuncAtual->endvar)
     Coloca NULO na pilha de variáveis
     Se não houver função anterior:
       Fim do algoritmo
     Apaga função atual (FuncAtual--)
     Vai para (início)
  Se for variável:
     Cria variável local da função atual
  Se linha possui expressão numérica:
     FuncAtual->expr = início da expressão numérica
     FuncAtual->exprvar = VarAtual + 1 (para desempilhar variáveis depois)
     Vai para (início)
  Processa instrução que não requer expressão numérica
  Passa para próxima instrução
  Vai para (início)
Se FuncAtual->expr != 0:
  *** Está processando uma expressão numérica
  *** FuncAtual->expr[0] contém a próxima instrução da expressão numérica
  Se for ex_fim:
     FuncAtual->expr = 0
     Processa instrução que depende de expressão numérica
     Passa para próxima instrução
     Apaga variáveis de FuncAtual->exprvar em diante
     Vai para (início)
  Se for um valor fixo (número, texto ou NULO):
     Coloca variável na pilha de variáveis
     Avança FuncAtual->expr
     Vai para (início)
  Se for um operador:
     Processa operador conforme as variáveis do topo da pilha
     Os operadores & e | podem avançar na expressão numérica
     Avança FuncAtual->expr
     Vai para (início)
  *** A partir daqui processa funções e variáveis
  *** Cria-se uma variável cNomeVar, que contém parte do nome da variável
  *** Cria-se uma variável em seguida, que contém o resultado anterior
  *** Isso torna possível processar várias variáveis ligadas por ponto
  *** Exemplo: a.b.c
  Se for ex_varini ou ex_varabre:
     Cria variável cNomeVar na pilha de variáveis para armazenar o nome
     Cria variável cVarInicio na pilha de variáveis
     Avança FuncAtual->Expr
     Se era ex_varini:
       Copia o que puder do nome em (um texto) para última variável cNomeVar
     Vai para (início)
  Se for ex_arg (argumento da função) ou ex_abre (abre colchetes):
     Avança FuncAtual->expr
     Vai para (início)
  Se for ex_fecha (fecha colchetes):
     Avança FuncAtual->expr
     Sendo que: X=última variável cNomeVar da pilha e Y=variável após X
     Transforma em texto as variáveis após Y e anoma em X
     Apaga variáveis após Y
     Copia o que puder do nome em FuncAtual->expr (um texto) para X
     Vai para (início)
  Se for ex_varfim:
     Avança FuncAtual->expr
     Sendo que: X=última variável cNomeVar da pilha e Y=variável após X
     Se variável Y for cVarInicio, cVarClasse ou cVarObjeto:
       Apaga variáveis de X em diante
       Cria variável cNulo
     Caso contrário:
       Apaga variáveis após Y
       Apaga variável X
       Variável Y passa a ocupar o lugar da variável X
     Vai para (início)
  Se for ex_doispontos:
     Procura última variável cNomeVar da pilha
     Se nome de classe em cNomeVar existe e próxima variável for cVarInicio:
       Muda a próxima variável para cVarClasse
       Anota o endereço da classe
     Caso contrário:
       Apaga da variável cNomeVar em diante
       Cria variável cNulo na pilha de variáveis
       Avança FuncAtual->expr até ex_varfim
     Avança FuncAtual->expr
     Vai para (início)
  Se for ex_ponto:
     Avança FuncAtual->expr
     Procura última variável cNomeVar da pilha
     Checa próxima variável e texto em cNomeVar:
        cVarInicio -> 1. Verifica se é variável/comando interno do programa
                      2. Verifica se é variável local da função
                      3. Verifica se é variável ou função do objeto
        cVarClasse -> Verifica se é função da classe especificada
        cVarObjeto -> Verifica se é variável ou função do objeto em cVarObjeto
     Depois de decidido que ação tomar:
       Limpa texto em cNomeVar
       Copia o que puder do nome em FuncAtual->expr (um texto) para cNomeVar
     Executa ação, que pode ser:
     1. Processar função interna do programa:
        Apaga variáveis após cNomeVar
        Cria variável com o resultado
     2. Executar uma função de uma classe:
        FuncAtual++
        FuncAtual->linha = primeira linha após o nome da função
        FuncAtual->este = endereço do objeto "este", ou NULL
        FuncAtual->expr = 0
        FuncAtual->exprvar = 0
        FuncAtual->endvar = primeira variável após cNomeVar
        FuncAtual->numvar = 0
        FuncAtual->numarg = número de variáveis após cNomeVar
     3. Variável definida em uma classe:
        Apaga variáveis a partir da anterior a cNomeVar
        Anota variável no topo da pilha de variáveis
     4. Uma variável qualquer:
        Consulta a variável sobre o que fazer; pode cair em 2 ou 3
  Nunca deverá chegar aqui; executar assert(0)
@endverbatim
*/

//------------------------------------------------------------------------------
// Pilha de dados - para criar/apagar variáveis
char * const Instr::DadosPilha = new char[0x10000];
char * const Instr::DadosFim   = Instr::DadosPilha + 0x10000;
char * Instr::DadosTopo  = Instr::DadosPilha;

// Pilha de variáveis
TVariavel * const Instr::VarPilha   = new TVariavel[500];
TVariavel * const Instr::VarFim     = Instr::VarPilha + 500;
TVariavel * Instr::VarAtual   = Instr::VarPilha;

// Pilha de funções
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
 *  @param classe Endereço da classe (objeto este é NULO)
 *  @param func Nome da função (string ASCIIZ)
 *  @retval true Função encontrada; pode executar
 *  @retval false Função não existe, não pode executar
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
    VarAtual = VarPilha;    // Limpa pilha de variáveis
    VarAtual->Limpar();     // Variável atual é nulo
    VarAtual->defvar = InstrNulo;
    FuncAtual = FuncPilha;  // Limpa pilha de funções
    FuncAtual->este = 0;    // Nenhum objeto
    FuncAtual->linha = instr;// Primeira instrução da função
    FuncAtual->expr = 0;    // Não está processando expressão numérica
    FuncAtual->exprvar = 0;
    FuncAtual->endvar = VarAtual + 1; // Endereço do primeiro argumento
    FuncAtual->numvar = 0;  // Número de variáveis da função
    FuncAtual->numarg = 0;  // Número de argumentos da função
    return true;
}

//----------------------------------------------------------------------------
/// Prepara para executar
/**
 *  @param este Endereço do objeto "este"
 *  @param func Nome da função (string ASCIIZ)
 *  @retval true Função encontrada; pode executar
 *  @retval false Função não existe, não pode executar
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
// Apaga variáveis da pilha
    Instr::ApagarVar(Instr::VarPilha+1);
// Cria NULO na pilha
    Instr::VarAtual++;
    Instr::VarAtual->Limpar();
    Instr::VarAtual->defvar = InstrNulo;
    return false;
}

//----------------------------------------------------------------------------
/// Executa função
/** @return true se executou normal, false se cancelou por falta de memória
 *  @note Independente de retornar true ou false, executar ExecFim() depois
 */
bool Instr::ExecX()
{
    while (true)
    {
        if (FuncAtual->expr==0)
        {
    // Está processando uma linha

        // Retorno sem parâmetros
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
            // Apaga variáveis da função
                ApagarVar(FuncAtual->endvar);
            // Cria NULO na pilha de variávels (retorno da função)
                if (VarAtual >= VarFim-1)
                    return RetornoErro();
                VarAtual++;
                VarAtual->Limpar();
                VarAtual->defvar = InstrNulo;
            // Apaga função
                if (FuncAtual==FuncPilha)
                    return true;
                FuncAtual--;
                continue;
            }

        // Variável da função
            if (FuncAtual->linha[2] > cVariaveis)
            {
                if (!CriarVar(FuncAtual->linha))
                    return RetornoErro();
                continue;
            }

        // Processa instrução
            switch (FuncAtual->linha[2])
            {
            case cSe:   // Processa expressão numérica
            case cSenao2:
            case cEnquanto:
                FuncAtual->expr = FuncAtual->linha + 5;
                FuncAtual->exprvar = VarAtual + 1;
                break;
            case cRet2: // Processa expressão numérica
            case cExpr:
                FuncAtual->expr = FuncAtual->linha + 3;
                FuncAtual->exprvar = VarAtual + 1;
                break;
            case cComent: // Comentário
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
            default:  // Instrução desconhecida
                assert(0);
            }
            continue;
        }

    // Está processando expressão numérica
    // FuncAtual->expr[0] contém a próxima instrução da expressão numérica
        switch (FuncAtual->expr[0])
        {
        case ex_fim:
            FuncAtual->expr = 0;
            // Processa instrução que depende de expressão numérica
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
     Processa operador conforme as variáveis do topo da pilha
     Os operadores & e | podem avançar na expressão numérica
     Avança FuncAtual->expr
     Vai para (início)
  *** A partir daqui processa funções e variáveis
  *** Cria-se uma variável cNomeVar, que contém parte do nome da variável
  *** Cria-se uma variável em seguida, que contém o resultado anterior
  *** Isso torna possível processar várias variáveis ligadas por ponto
  *** Exemplo: a.b.c
  Se for ex_varini ou ex_varabre:
     Cria variável cNomeVar na pilha de variáveis para armazenar o nome
     Cria variável cVarInicio na pilha de variáveis
     Avança FuncAtual->Expr
     Se era ex_varini:
       Copia o que puder do nome em (um texto) para última variável cNomeVar
     Vai para (início)
  Se for ex_arg (argumento da função) ou ex_abre (abre colchetes):
     Avança FuncAtual->expr
     Vai para (início)
  Se for ex_fecha (fecha colchetes):
     Avança FuncAtual->expr
     Sendo que: X=última variável cNomeVar da pilha e Y=variável após X
     Transforma em texto as variáveis após Y e anoma em X
     Apaga variáveis após Y
     Copia o que puder do nome em FuncAtual->expr (um texto) para X
     Vai para (início)
  Se for ex_varfim:
     Avança FuncAtual->expr
     Sendo que: X=última variável cNomeVar da pilha e Y=variável após X
     Se variável Y for cVarInicio, cVarClasse ou cVarObjeto:
       Apaga variáveis de X em diante
       Cria variável cNulo
     Caso contrário:
       Apaga variáveis após Y
       Apaga variável X
       Variável Y passa a ocupar o lugar da variável X
     Vai para (início)
  Se for ex_doispontos:
     Procura última variável cNomeVar da pilha
     Se nome de classe em cNomeVar existe e próxima variável for cVarInicio:
       Muda a próxima variável para cVarClasse
       Anota o endereço da classe
     Caso contrário:
       Apaga da variável cNomeVar em diante
       Cria variável cNulo na pilha de variáveis
       Avança FuncAtual->expr até ex_varfim
     Avança FuncAtual->expr
     Vai para (início)
  Se for ex_ponto:
     Avança FuncAtual->expr
     Procura última variável cNomeVar da pilha
     Checa próxima variável e texto em cNomeVar:
        cVarInicio -> 1. Verifica se é variável/comando interno do programa
                      2. Verifica se é variável local da função
                      3. Verifica se é variável ou função do objeto
        cVarClasse -> Verifica se é função da classe especificada
        cVarObjeto -> Verifica se é variável ou função do objeto em cVarObjeto
     Depois de decidido que ação tomar:
       Limpa texto em cNomeVar
       Copia o que puder do nome em FuncAtual->expr (um texto) para cNomeVar
     Executa ação, que pode ser:
     1. Processar função interna do programa:
        Apaga variáveis após cNomeVar
        Cria variável com o resultado
     2. Executar uma função de uma classe:
        FuncAtual++
        FuncAtual->linha = primeira linha após o nome da função
        FuncAtual->este = endereço do objeto "este", ou NULL
        FuncAtual->expr = 0
        FuncAtual->exprvar = 0
        FuncAtual->endvar = primeira variável após cNomeVar
        FuncAtual->numvar = 0
        FuncAtual->numarg = número de variáveis após cNomeVar
     3. Variável definida em uma classe:
        Apaga variáveis a partir da anterior a cNomeVar
        Anota variável no topo da pilha de variáveis
     4. Uma variável qualquer:
        Consulta a variável sobre o que fazer; pode cair em 2 ou 3
  Nunca deverá chegar aqui; executar assert(0) */

    }
    return RetornoErro();
}

//----------------------------------------------------------------------------
/// Executa procedimentos após ExecX()
void Instr::ExecFim()
{
    ApagarVar(VarAtual+1);


    // Se apagar alguma classe aqui, acertar também função main()

}
