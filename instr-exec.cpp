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

#define DEBUG_INSTR // Mostra instruções que serão executadas
#define DEBUG_EXPR  // Mostra valores de Instr::Expressao encontrados
#define DEBUG_VAR   // Mostra variáveis no topo da pilha

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
static const char InstrInt[] = { 7, 0, Instr::cInt32, 0, 0, '+', 0 };
static const char InstrUInt[] = { 7, 0, Instr::cUInt32, 0, 0, '+', 0 };
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
/// Adiciona argumento antes de executar
/**
 *  @param txt Texto a adicionar
 *  @note  O texto deve existir quando Instr::ExecX() for executado
 *  @sa exec
 */
void Instr::ExecArg(char * txt)
{
    VarAtual++;
    VarAtual->Limpar();
    VarAtual->defvar = InstrTxtFixo;
    VarAtual->endvar = txt;
}

//----------------------------------------------------------------------------
/// Usado internamente em Instr::ExecX()
/** Executa procedimentos em caso de memória insuficiente
 *  @return false
 */
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
                FuncAtual->linha += Num16(FuncAtual->linha);
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
                FuncAtual->igualcompara = true;
                break;
            case cRet2: // Processa expressão numérica
                FuncAtual->expr = FuncAtual->linha + 3;
                FuncAtual->exprvar = VarAtual + 1;
                FuncAtual->igualcompara = true;
                break;
            case cExpr:
                FuncAtual->expr = FuncAtual->linha + 3;
                FuncAtual->exprvar = VarAtual + 1;
                FuncAtual->igualcompara = false;
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
#ifdef DEBUG_INSTR
            {
                char mens[512];
                if (Instr::Decod(mens, FuncAtual->linha, sizeof(mens)))
                    printf("Exec: %s\n", mens);
                else
                    printf("Erro ao ler instrução\n");
                fflush(stdout);
            }
#endif

  //*********** Aqui deve processar a instrução

                FuncAtual->linha += Num16(FuncAtual->linha);
                break;
            default:  // Instrução desconhecida
                assert(0);
            }
            continue;
        }

    // Está processando expressão numérica
    // FuncAtual->expr[0] contém a próxima instrução da expressão numérica
#ifdef DEBUG_VAR
        printf("       %p\n", DadosTopo);
        for (int x=0; x<5; x++)
        {
            TVariavel * v = VarAtual - x;
            if (v<VarPilha)
                continue;
            printf("    v%d %p l%d ", v-VarPilha, v->endvar, v->local);
            const char * p = NomeComando(v->defvar[2]);
            if (p)
                printf("%s ", p);
            else
                printf("???_%d ", (unsigned char)v->defvar[2]);
            switch (v->Tipo())
            {
            case varNulo: printf("NULL"); break;
            case varInt: printf("int=%d", v->getInt()); break;
            case varDouble: printf("double=%f", v->getDouble()); break;
            case varTxt: printf("txt=%s", v->getTxt()); break;
            case varObj: printf("ref=%s", v->getTxt()); break;
            }
            putchar('\n');
        }
        fflush(stdout);
#endif
#ifdef DEBUG_EXPR
        {
            const char * p = NomeExpr(FuncAtual->expr[0]);
            if (p)
                printf("  %s\n", p);
            else
                printf("  ??? %d\n", (unsigned char)FuncAtual->expr[0]);
            fflush(stdout);
        }
#endif
        switch (FuncAtual->expr[0])
        {
        case ex_fim:
        case ex_coment:
            FuncAtual->expr = 0;
#ifdef DEBUG_INSTR
            {
                char mens[512];
                if (Instr::Decod(mens, FuncAtual->linha, sizeof(mens)))
                    printf("Exec_arg: %s\n", mens);
                else
                    printf("Erro ao ler instrução\n");
                fflush(stdout);
            }
#endif

  //*********** Aqui deve processar a instrução

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
            while (*FuncAtual->expr)
                FuncAtual->expr++;
            FuncAtual->expr++;
            break;
        case ex_num0:
            if (!CriarVar(InstrInt))
                return RetornoErro();
            FuncAtual->expr++;
            break;
        case ex_num1:
            if (!CriarVar(InstrInt))
                return RetornoErro();
            *(unsigned char*)VarAtual->endvar=1;
            FuncAtual->expr++;
            break;
        case ex_num8p:
            if (!CriarVar(InstrInt))
                return RetornoErro();
            *(unsigned char*)VarAtual->endvar = FuncAtual->expr[1];
            FuncAtual->expr += 2;
            break;
        case ex_num16p:
            if (!CriarVar(InstrInt))
                return RetornoErro();
            *(unsigned char*)VarAtual->endvar = FuncAtual->expr[1];
            *((unsigned char*)VarAtual->endvar+1) = FuncAtual->expr[2];
            FuncAtual->expr += 3;
            break;
        case ex_num32p:
            if (!CriarVar( *((unsigned char*)FuncAtual->expr+4) & 0x80 ?
                    InstrUInt : InstrInt))
                return RetornoErro();
            memcpy(VarAtual->endvar, FuncAtual->expr+1, 4);
            FuncAtual->expr += 5;
            break;
        case ex_num8n:
            if (!CriarVar(InstrInt))
                return RetornoErro();
            VarAtual->setInt(-(int)(unsigned char)FuncAtual->expr[1]);
            FuncAtual->expr += 2;
            break;
        case ex_num16n:
            if (!CriarVar(InstrInt))
                return RetornoErro();
            VarAtual->setInt(-(int)Num16(FuncAtual->expr+1));
            FuncAtual->expr += 3;
            break;
        case ex_num32n:
            if (*((unsigned char*)FuncAtual->expr+4) & 0x80)
            {
                if (!CriarVar(InstrDouble))
                    return RetornoErro();
                VarAtual->setDouble(-(double)Num32(FuncAtual->expr+1));
            }
            else
            {
                if (!CriarVar(InstrInt))
                    return RetornoErro();
                VarAtual->setInt(-(int)Num32(FuncAtual->expr+1));
            }
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
                if (VarAtual->local==0 || VarAtual->defvar[2] != cReal)
                {
                    ApagarVar(VarAtual);
                    CriarVar(InstrDouble);
                }
                VarAtual->setDouble(valor);
                break;
            }
        case exo_ini:        // Operador: Marca o início dos operadores
        case exo_fim:        // Operador: Marca o fim dos operadores
        case exo_virgula:    // Operador: Vírgula, para separar expressões
            FuncAtual->expr++;
            break;
        case exo_neg:        // Operador: -a
            FuncAtual->expr++;
            switch (VarAtual->Tipo())
            {
            case varNulo:
                break;
            case varInt:
                if (VarAtual->local)
                    VarAtual->setInt(-VarAtual->getInt());
                else
                {
                    int valor = -VarAtual->getInt();
                    ApagarVar(VarAtual);
                    if (!CriarVar(InstrInt))
                        return RetornoErro();
                    VarAtual->setInt(valor);
                }
                break;
            case varDouble:
                if (VarAtual->local)
                {
                    VarAtual->setDouble(-VarAtual->getDouble());
                    break;
                }
            default:
                {
                    double valor = -VarAtual->getDouble();
                    ApagarVar(VarAtual);
                    if (!CriarVar(InstrDouble))
                        return RetornoErro();
                    VarAtual->setDouble(valor);
                }
            }
            break;
        case exo_exclamacao: // Operador: !a
            FuncAtual->expr++;
            if (VarAtual->Tipo() != varInt || VarAtual->local==0)
            {
                bool valor = !VarAtual->getBool();
                ApagarVar(VarAtual);
                if (!CriarVar(InstrInt))
                    return RetornoErro();
                VarAtual->setInt(valor);
            }
            else
                VarAtual->setInt(!VarAtual->getInt());
            break;
        case exo_mul:        // Operador: a*b
            {
                FuncAtual->expr++;
                double valor = VarAtual[-1].getDouble() *
                               VarAtual[0].getDouble();
                ApagarVar(VarAtual);
                if (VarAtual->local==0 || VarAtual->Tipo()!=varDouble)
                {
                    ApagarVar(VarAtual);
                    if (!CriarVar(InstrDouble))
                        return RetornoErro();
                }
                VarAtual->setDouble(valor);
                break;
            }
        case exo_div:        // Operador: a/b
            {
                FuncAtual->expr++;
                double valor1 = VarAtual[-1].getDouble();
                double valor2 = VarAtual[0].getDouble();
                if (valor2)
                    valor1 /= valor2;
                else
                    valor1 *= 1.0e1000;
                ApagarVar(VarAtual);
                if (VarAtual->local==0 || VarAtual->Tipo()!=varDouble)
                {
                    ApagarVar(VarAtual);
                    if (!CriarVar(InstrDouble))
                        return RetornoErro();
                }
                VarAtual->setDouble(valor1);
                break;
            }
        case exo_porcent:    // Operador: a%b
            {
                FuncAtual->expr++;
                int valor = VarAtual[0].getInt();
                if (valor)
                    valor = VarAtual[-1].getInt() % valor;
                else
                    valor = 0;
                ApagarVar(VarAtual);
                if (VarAtual->local==0 || VarAtual->Tipo()!=varInt)
                {
                    ApagarVar(VarAtual);
                    if (!CriarVar(InstrInt))
                        return RetornoErro();
                }
                VarAtual->setInt(valor);
                break;
            }
        case exo_add:        // Operador: a+b
            {
                FuncAtual->expr++;
                int tipo = VarAtual[-1].Tipo();
            // Não é texto: executa soma numérica
                if (tipo!=varTxt)
                {
                    double valor = VarAtual[-1].getDouble() +
                                   VarAtual[0].getDouble();
                    ApagarVar(VarAtual);
                    if (VarAtual->local==0 || tipo!=varDouble)
                    {
                        ApagarVar(VarAtual);
                        if (!CriarVar(InstrDouble))
                            return RetornoErro();
                    }
                    VarAtual->setDouble(valor);
                    break;
                }
            // Exemplo de comando no intmud.map que usa os 3 tipos
            // de concatenação:  "bom"+1+("dia"+2)+4

            // Primeiro texto não é cTxtFixo ou não está na pilha
                if (VarAtual[-1].local == 0 ||
                        VarAtual[-1].defvar[2] != cTxtFixo)
                {
                    char mens[4096];
                // Monta texto em mens[]
                    char * destino = mens;
                    const char * origem = VarAtual[-1].getTxt();
                    while (*origem && destino<mens+sizeof(mens)-1)
                        *destino++ = *origem++;
                    origem = VarAtual[0].getTxt();
                    while (*origem && destino<mens+sizeof(mens)-1)
                        *destino++ = *origem++;
                    *destino++=0;
                // Cria variável que conterá o texto
                    ApagarVar(VarAtual-1);
                    if (VarAtual >= VarFim-1)
                        return RetornoErro();
                    VarAtual++;
                    VarAtual->Limpar();
                    VarAtual->defvar = InstrTxtFixo;
                    VarAtual->endvar = DadosTopo;
                    VarAtual->local = 1;
                // Copia texto para variável
                    int total = destino-mens;
                    if (DadosTopo + total > DadosFim)
                        return RetornoErro();
                    memcpy(DadosTopo, mens, total);
                    DadosTopo += total;
                    break;
                }
            // Segundo texto está na pilha (depois do primeiro)
            // e segundo texto não é cTxtFixo
                if (VarAtual->local &&
                        VarAtual->defvar[2] != cTxtFixo)
                {
                    char mens[4096];
                // Monta texto em mens[]
                    char * destino = mens;
                    const char * origem = VarAtual->getTxt();
                    while (*origem && destino<mens+sizeof(mens)-1)
                        *destino++ = *origem++;
                    *destino++ = 0;
                    int total = destino-mens;
                // Apaga segunda variável
                    ApagarVar(VarAtual);
                // Obtém o final do primeiro texto, na pilha
                    destino = DadosTopo-4;
                    if (destino < (char*)VarAtual->endvar)
                        destino = (char*)VarAtual->endvar;
                    while (*destino)
                        destino++;
                // Acrescenta texto na primeira variável
                    if (destino+total > DadosFim)
                        return RetornoErro();
                    memcpy(destino, mens, total);
                    DadosTopo = destino + total;
                    break;
                }
            // Concatena dois textos copiando o segundo após o primeiro
                    const char * origem = VarAtual->getTxt();
                // Obtém o final do primeiro texto, na pilha
                    char * destino = DadosTopo-4;
                    if (VarAtual->local)
                        destino = (char*)VarAtual->endvar - 4;
                    if (destino < (char*)VarAtual[-1].endvar)
                        destino = (char*)VarAtual[-1].endvar;
                    while (*destino)
                        destino++;
                // Copia texto
                    while (*origem && destino<DadosFim-1)
                        *destino++ = *origem++;
                    *destino++=0;
                // Apaga segunda variável
                    ApagarVar(VarAtual);
                // Acerta o tamanho da primeira variável
                    DadosTopo = destino;
                break;
            }
        case exo_sub:        // Operador: a-b
            {
                FuncAtual->expr++;
                double valor = VarAtual[-1].getDouble() -
                                VarAtual[0].getDouble();
                ApagarVar(VarAtual);
                if (VarAtual->local==0 || VarAtual->Tipo()!=varDouble)
                {
                    ApagarVar(VarAtual);
                    if (!CriarVar(InstrDouble))
                        return RetornoErro();
                }
                VarAtual->setDouble(valor);
                break;
            }
        case exo_igual:      // Operador: a=b
            if (FuncAtual->igualcompara==false)
            {
                FuncAtual->expr++;
                switch (VarAtual[-1].Tipo())
                {
                case varNulo:
                    break;
                case varInt:
                    VarAtual[-1].setInt( VarAtual->getInt() );
                    break;
                case varDouble:
                    VarAtual[-1].setDouble( VarAtual->getDouble() );
                    break;
                case varTxt:
                    VarAtual[-1].setTxt( VarAtual->getTxt() );
                    break;
                case varObj:
                    VarAtual[-1].setObj( VarAtual->getObj() );
                    break;
                }
                ApagarVar(VarAtual);
                break;
            }
        case exo_menor:      // Operador: a<b
        case exo_menorigual: // Operador: a<=b
        case exo_maior:      // Operador: a>b
        case exo_maiorigual: // Operador: a>=b
        case exo_diferente:  // Operador: a!=b
            {
            // Compara valores
                int tipo1 = VarAtual[-1].Tipo();
                int tipo2 = VarAtual[0].Tipo();
                if (tipo1==varTxt || tipo2==varTxt)
                    tipo1 = comparaZ(VarAtual[-1].getTxt(),
                                    VarAtual->getTxt());
                else if (tipo1==varInt && tipo2==varInt)
                {
                    int v1 = VarAtual[-1].getInt();
                    int v2 = VarAtual[0].getInt();
                    tipo1 = (v1==v2 ? 0 : v1<v2 ? -1 : 1);
                }
                else
                {
                    double v1 = VarAtual[-1].getDouble();
                    double v2 = VarAtual[0].getDouble();
                    tipo1 = (v1==v2 ? 0 : v1<v2 ? -1 : 1);
                }
            // Apaga valores da pilha; cria int32 na pilha
                ApagarVar(VarAtual-1);
                if (!CriarVar(InstrInt))
                    return RetornoErro();
            // Avança Func->expr, verifica operador
                switch (*FuncAtual->expr++)
                {
                case exo_menor:      if (tipo1<0)  VarAtual->setInt(1); break;
                case exo_menorigual: if (tipo1<=0) VarAtual->setInt(1); break;
                case exo_maior:      if (tipo1>0)  VarAtual->setInt(1); break;
                case exo_maiorigual: if (tipo1>=0) VarAtual->setInt(1); break;
                case exo_igual:      if (tipo1==0) VarAtual->setInt(1); break;
                case exo_diferente:  if (tipo1!=0) VarAtual->setInt(1); break;
                }
                break;
            }
        case exo_ee:         // Operador: Início do operador &
            FuncAtual->expr++;
            if (VarAtual->getBool()==false)
            {
                char * p = ProcuraExpr(FuncAtual->expr, exo_e);
                assert(p!=0);
                FuncAtual->expr=p;
                ApagarVar(VarAtual);
                if (!CriarVar(InstrInt))
                    return RetornoErro();
                break;
            }
            ApagarVar(VarAtual);
            break;
        case exo_ouou:       // Operador: Início do operador |
            FuncAtual->expr++;
            if (VarAtual->getBool())
            {
                char * p = ProcuraExpr(FuncAtual->expr, exo_ou);
                assert(p!=0);
                FuncAtual->expr = p + 1;
                ApagarVar(VarAtual);
                if (!CriarVar(InstrInt))
                    return RetornoErro();
                VarAtual->setInt(1);
                break;
            }
            ApagarVar(VarAtual);
            break;
        case exo_e:          // Operador: a&b
        case exo_ou:         // Operador: a|b
            {
                FuncAtual->expr++;
                bool b = VarAtual->getBool();
                ApagarVar(VarAtual);
                if (!CriarVar(InstrInt))
                    return RetornoErro();
                if (b)
                    VarAtual->setInt(1);
                break;
            }


     /*   case exo_igualmul:   // Operador: a*=b
        case exo_igualdiv:   // Operador: a/=b
        case exo_igualporcent: // Operador: a%=b
        case exo_igualadd:   // Operador: a+=b
        case exo_igualsub:   // Operador: a-=b
      */

    // A partir daqui processa funções e variáveis
    // Cria-se uma variável cNomeVar, que contém parte do nome da variável
    // Cria-se uma variável em seguida, que contém o resultado anterior
    // Isso torna possível processar várias variáveis ligadas por ponto
    // Exemplo: a.b.c

/*
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

        default:
            assert(0);
        }
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
