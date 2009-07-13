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
#include "var-log.h"
#include "classe.h"
#include "objeto.h"
#include "socket.h"
#include "misc.h"

void Termina(); // Encerra o programa

//#define DEBUG_INSTR // Mostra instruções que serão executadas
//#define DEBUG_EXPR  // Mostra valores de Instr::Expressao encontrados
//#define DEBUG_VAR   // Mostra variáveis no topo da pilha

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

- cVarNome    -> Para obter nome da variável
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
FuncAtual->fimvar = endereço da primeira variável após VarAtual

(início)
Se FuncAtual->expr == 0:
  *** Está processando uma linha:
  Se for fim da função ou instrução ret sem parâmetros:
     Apaga variáveis criadas na função (a partir de FuncAtual->inivar)
     Coloca NULO na pilha de variáveis
     Se não houver função anterior:
       Fim do algoritmo
     Apaga função atual (FuncAtual--)
     Vai para (início)
  Se for variável:
     Cria variável local da função atual
     FuncAtual->fimvar++
  Se linha possui expressão numérica:
     FuncAtual->expr = início da expressão numérica
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
     Apaga variáveis de FuncAtual->fimvar em diante
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
  *** Cria-se uma variável cVarNome, que contém parte do nome da variável
  *** Cria-se uma variável em seguida, que contém o resultado anterior
  *** Isso torna possível processar várias variáveis ligadas por ponto
  *** Exemplo: a.b.c
  Se for ex_varini ou ex_varabre:
     Cria variável cVarNome na pilha de variáveis para armazenar o nome
     Cria variável cVarInicio na pilha de variáveis
     Avança FuncAtual->Expr
     Se era ex_varini:
       Copia o que puder do nome em (um texto) para última variável cVarNome
     Vai para (início)
  Se for ex_arg (argumento da função) ou ex_abre (abre colchetes):
     Avança FuncAtual->expr
     Vai para (início)
  Se for ex_fecha (fecha colchetes):
     Avança FuncAtual->expr
     Sendo que: X=última variável cVarNome da pilha e Y=variável após X
     Transforma em texto as variáveis após Y e anota em X
     Apaga variáveis após Y
     Copia o que puder do nome em FuncAtual->expr (um texto) para X
     Vai para (início)
  Se for ex_varfim:
     Avança FuncAtual->expr
     Sendo que: X=última variável cVarNome da pilha e Y=variável após X
     Se variável Y for cVarInicio ou cVarClasse:
       Apaga variáveis de X em diante
       Cria variável cNulo
     Caso contrário:
       Apaga variáveis após Y
       Apaga variável X
       Variável Y passa a ocupar o lugar da variável X
     Vai para (início)
  Se for ex_doispontos:
     Procura última variável cVarNome da pilha
     Se nome de classe em cVarNome existe e próxima variável for cVarInicio:
       Muda a próxima variável para cVarClasse
       Anota o endereço da classe
     Caso contrário:
       Apaga da variável cVarNome em diante
       Cria variável cNulo na pilha de variáveis
       Avança FuncAtual->expr até ex_varfim
     Avança FuncAtual->expr
     Vai para (início)
  Se for ex_ponto:
     Avança FuncAtual->expr
     Procura última variável cVarNome da pilha
     Checa próxima variável e texto em cVarNome:
        cVarInicio -> 1. Verifica se é variável/comando interno do programa
                      2. Verifica se é variável local da função
                      3. Verifica se é variável ou função do objeto
        cVarClasse -> Verifica se é função da classe especificada
        cVarObjeto -> Verifica se é variável ou função do objeto em cVarObjeto
     Depois de decidido que ação tomar:
       Limpa texto em cVarNome
       Copia o que puder do nome em FuncAtual->expr (um texto) para cVarNome
     Executa ação, que pode ser:
     1. Processar função interna do programa:
        Apaga variáveis após cVarNome
        Cria variável com o resultado
     2. Executar uma função de uma classe:
        FuncAtual++
        FuncAtual->linha = primeira linha após o nome da função
        FuncAtual->este = endereço do objeto "este", ou NULL
        FuncAtual->expr = 0
        FuncAtual->inivar = primeira variável após cVarNome
        FuncAtual->fimvar = endereço da primeira variável após VarAtual
        FuncAtual->numarg = número de variáveis após cVarNome
     3. Variável definida em uma classe:
        Apaga variáveis a partir da anterior a cVarNome
        Anota variável no topo da pilha de variáveis
     4. Uma variável qualquer:
        Consulta a variável sobre o que fazer; pode cair em 2 ou 3
  Nunca deverá chegar aqui; executar assert(0)
@endverbatim
*/

//------------------------------------------------------------------------------
int Instr::VarExec = 0;
int Instr::VarExecIni = 5000;

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
const char Instr::InstrNulo[] = { 7, 0, Instr::cConstNulo, 0, 0, 0, '+', 0 };
const char Instr::InstrDouble[] = { 7, 0, Instr::cReal, 0, 0, 0, '+', 0 };
const char Instr::InstrSocket[] = { 7, 0, Instr::cSocket, 0, 0, 0, '+', 0 };
const char Instr::InstrTxtFixo[] = { 7, 0, Instr::cTxtFixo, 0, 0, 0, '+', 0 };
const char Instr::InstrVarNome[] = { 7, 0, Instr::cVarNome, 0, 0, 0, '+', 0 };
const char Instr::InstrVarInicio[] = { 7, 0, Instr::cVarInicio, 0, 0, 0, '+', 0 };
const char Instr::InstrVarClasse[] = { 7, 0, Instr::cVarClasse, 0, 0, 0, '+', 0 };
const char Instr::InstrVarObjeto[] = { 7, 0, Instr::cVarObjeto, 0, 0, 0, '+', 0 };
const char Instr::InstrVarInt[] = { 7, 0, Instr::cVarInt, 0, 0, 0, '+', 0 };

//------------------------------------------------------------------------------
// Lista de funções predefinidas
// Deve obrigatoriamente estar em ordem alfabética
static const Instr::TListaFunc ListaFunc[] = {
    { "apagar",     Instr::FuncApagar, 0 },
    { "arg0",       Instr::FuncArg, 0 },
    { "arg1",       Instr::FuncArg, 1 },
    { "arg2",       Instr::FuncArg, 2 },
    { "arg3",       Instr::FuncArg, 3 },
    { "arg4",       Instr::FuncArg, 4 },
    { "arg5",       Instr::FuncArg, 5 },
    { "arg6",       Instr::FuncArg, 6 },
    { "arg7",       Instr::FuncArg, 7 },
    { "arg8",       Instr::FuncArg, 8 },
    { "arg9",       Instr::FuncArg, 9 },
    { "args",       Instr::FuncArgs, 0 },
    { "criar",      Instr::FuncCriar, 0 },
    { "este",       Instr::FuncEste, 0 },
    { "int",        Instr::FuncNumero, 2 },
    { "intabs",     Instr::FuncNumero, 1 },
    { "intnome",    Instr::FuncInt, 0 },
    { "intpos",     Instr::FuncNumero, 0 },
    { "intsenha",   Instr::FuncInt, 1 },
    { "inttotal",   Instr::FuncTotal, 0 },
    { "objantes",   Instr::FuncAntesDepois, 0 },
    { "objdepois",  Instr::FuncAntesDepois, 1 },
    { "rand",       Instr::FuncNumero, 3 },
    { "ref",        Instr::FuncRef, 0 },
    { "txt",        Instr::FuncTxt, 0 },
    { "txt1",       Instr::FuncTxt2, 0 },
    { "txt2",       Instr::FuncTxt2, 1 },
    { "txtcor",     Instr::FuncTxt2, 2 },
    { "txtesp",     Instr::FuncEsp, 0 },
    { "txtfiltro",  Instr::FuncTxt2, 7 },
    { "txtmai",     Instr::FuncTxt2, 3 },
    { "txtmai2",    Instr::FuncTxt2, 4 },
    { "txtmaimin",  Instr::FuncTxt2, 6 },
    { "txtmin",     Instr::FuncTxt2, 5 },
    { "txtnome",    Instr::FuncTxt2, 9 },
    { "txtnum",     Instr::FuncTxtNum, 0 },
    { "txtproc",    Instr::FuncTxtProc, 0 },
    { "txtprocdif", Instr::FuncTxtProc, 1 },
    { "txtremove",  Instr::FuncTxtRemove, 0 },
    { "txtshs",     Instr::FuncTxt2, 8 },
    { "txttroca",   Instr::FuncTxtTroca, 0 },
    { "txttrocadif",Instr::FuncTxtTroca, 1 },
    { "vartroca",   Instr::FuncVarTroca, 0 }
};

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
    VarExec = VarExecIni;
// Acerta pilhas
    DadosTopo = DadosPilha; // Limpa pilha de dados
    VarAtual = VarPilha;    // Limpa pilha de variáveis
    VarAtual->Limpar();     // Variável atual é nulo
    VarAtual->defvar = InstrNulo;
    FuncAtual = FuncPilha;  // Limpa pilha de funções
    FuncAtual->este = 0;    // Nenhum objeto
    FuncAtual->linha = instr;// Primeira instrução da função
    FuncAtual->expr = 0;    // Não está processando expressão numérica
    FuncAtual->inivar = VarAtual + 1; // Endereço do primeiro argumento
    FuncAtual->fimvar = VarAtual + 1;
    FuncAtual->numarg = 0;  // Número de argumentos da função
    FuncAtual->tipo = 0;
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
void Instr::ExecArg(const char * txt)
{
    VarAtual++;
    VarAtual->Limpar();
    VarAtual->defvar = InstrTxtFixo;
    VarAtual->endfixo = txt;
    FuncAtual->fimvar = VarAtual + 1;
    FuncAtual->numarg++;
}

//----------------------------------------------------------------------------
/// Adiciona argumento antes de executar
/**
 *  @param valor Número
 *  @sa exec
 */
void Instr::ExecArg(int valor)
{
    VarAtual++;
    VarAtual->Limpar();
    VarAtual->defvar = InstrVarInt;
    VarAtual->setInt(valor);
    FuncAtual->fimvar = VarAtual + 1;
    FuncAtual->numarg++;
}

//----------------------------------------------------------------------------
/// Cria uma variável como argumento antes de executar
/**
 *  @param def Definição da variável, conforme Instr::Codif()
 *  @sa exec
 */
void Instr::ExecArgCriar(const char * def)
{
    // Nota: CriarVar só retorna false no caso de memória insuficiente
    // Nesse caso, isso não deve acontecer
    CriarVar(def);
    FuncAtual->fimvar = VarAtual + 1;
    FuncAtual->numarg++;
}

//----------------------------------------------------------------------------
/// Usado internamente em Instr::ExecX()
/** Executa procedimentos em caso de memória insuficiente
 *  @return false
 */
static bool RetornoErro(void)
{
// Apaga variáveis da pilha
    Instr::ApagarVar(Instr::VarPilha);
// Cria NULO na pilha
    Instr::VarAtual++;
    Instr::VarAtual->Limpar();
    Instr::VarAtual->defvar = Instr::InstrNulo;
    return false;
}

//----------------------------------------------------------------------------
/// Usado internamente em Instr::ExecX()
/** Adiciona texto em variável cVarNome
 */
static const char * CopiaVarNome(TVariavel * v, const char * txt)
{
    if (v->defvar[2] != Instr::cVarNome)
        return txt;
    char * dest = (char*)v->endvar;
    const char * fim = dest + TVariavel::Tamanho(v->defvar) - 1;
    while (*dest)
        dest++;
    for (; *txt; txt++)
    {
        if (*(unsigned char*)txt < Instr::ex_varini)
            continue;
        else if (*(unsigned char*)txt < ' ')
            break;
        else if (dest<fim)
            *dest++ = *txt;
    }
    *dest=0;
    return txt;
}

//----------------------------------------------------------------------------
/// Usado internamente em Instr::ExecX()
/** @return Última variável cVarNome da pilha
 */
static TVariavel * EndVarNome(void)
{
    for (TVariavel * v = Instr::VarAtual; v>=Instr::VarPilha; v--)
        if (v->defvar[2] == Instr::cVarNome)
            return v;
    return 0;
}

//----------------------------------------------------------------------------
/// Usado internamente em Instr::ExecX()
/** Avança para fim do nome da variável
 */
static void VarInvalido(void)
{
// Obtém última variável cVarNome da pilha
    TVariavel * v = EndVarNome();
    assert(v!=0);
// Apaga última variável cVarNome em diante
    Instr::ApagarVar(v);
// Cria NULO na pilha
    Instr::CriarVar(Instr::InstrNulo);
// Avança para o fim do nome da variável
    const char * p = ProcuraExpr(Instr::FuncAtual->expr-1, Instr::ex_varfim);
    assert(p!=0);
    Instr::FuncAtual->expr = p + 1;
}

//----------------------------------------------------------------------------
/// Executa função
/** VarAtual contém a variável retornada pela função
 *  @return true se executou normal, false se cancelou por falta de memória
 *  @note Independente de retornar true ou false, executar ExecFim() depois
 */
bool Instr::ExecX()
{
    while (true)
    {
        if (FuncAtual->expr==0)
        {
    // Está processando uma linha
            if (--VarExec < 0)
                return RetornoErro();

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
                switch (FuncAtual->tipo)
                {
                case 1: // Ler varfunc
                    ApagarVar(FuncAtual->inivar);
                    if (VarAtual >= VarFim-1)
                        return RetornoErro();
                    VarAtual++;
                    VarAtual->Limpar();
                    VarAtual->defvar = InstrNulo;
                    if (!VarFuncFim())
                        return RetornoErro();
                    break;
                case 2: // Mudar varfunc
                    ApagarVar(FuncAtual->inivar);
                    break;
                case 3: // criar()
                    ApagarVar(FuncAtual->inivar - 2);
                    VarAtual++;
                    VarAtual->Limpar();
                    VarAtual->defvar = InstrVarObjeto;
                    VarAtual->endvar = FuncAtual->este;
                    break;
                default: // Função normal
                    ApagarVar(FuncAtual->inivar - 1);
                    VarAtual++;
                    VarAtual->Limpar();
                    VarAtual->defvar = InstrNulo;
                }
            // Apaga função
                FuncAtual--;
                if (FuncAtual>=FuncPilha)
                    continue;
                if (VarFuncIni(VarAtual))
                    continue;
                return true;
            }

        // Variável da função
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
            if (FuncAtual->linha[2] > cVariaveis)
            {
                if (!CriarVar(FuncAtual->linha))
                    return RetornoErro();
                FuncAtual->fimvar++;
                FuncAtual->linha += Num16(FuncAtual->linha);
                continue;
            }

        // Processa instrução
            switch (FuncAtual->linha[2])
            {
            case cSe:   // Processa expressão numérica
            case cEnquanto:
                FuncAtual->expr = FuncAtual->linha + 5;
                FuncAtual->igualcompara = true;
                break;
            case cRet2: // Processa expressão numérica
                FuncAtual->expr = FuncAtual->linha + 3;
                FuncAtual->igualcompara = true;
                break;
            case cExpr:
                FuncAtual->expr = FuncAtual->linha + 3;
                FuncAtual->igualcompara = false;
                break;
            case cComent: // Comentário
                VarExec++;
                FuncAtual->linha += Num16(FuncAtual->linha);
                break;
            case cSenao1: // Avança
            case cSenao2:
            case cSair:
                {
                    int desvio = Num16(FuncAtual->linha+3);
                    if (desvio==0)
                        FuncAtual->linha += Num16(FuncAtual->linha);
                    else
                        FuncAtual->linha += desvio;
                    break;
                }
            case cEFim:  // Recua
            case cContinuar:
                {
                    int desvio = Num16(FuncAtual->linha+3);
                    if (desvio==0)
                        FuncAtual->linha += Num16(FuncAtual->linha);
                    else
                        FuncAtual->linha -= desvio;
                    break;
                }
            case cFimSe: // Passa para próxima instrução
                FuncAtual->linha += Num16(FuncAtual->linha);
                break;
            case cTerminar: // Encerra o programa
                TSocket::SairPend();
                TVarLog::TempoEspera(1000);
                Termina();
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
            printf("    v%d %p m%d/%d l%d ", v-VarPilha, v->endvar,
                   v->indice, (unsigned char)v->defvar[endVetor], v->tamanho);
            const char * p = NomeComando(v->defvar[2]);
            if (p)
                printf("%s ", p);
            else
                printf("???_%d ", (unsigned char)v->defvar[2]);
            switch (v->Tipo())
            {
            case varOutros: printf("outros"); break;
            case varInt: printf("int=%d", v->getInt()); break;
            case varDouble: printf("double=%f", v->getDouble()); break;
            case varTxt: printf("txt=%s", v->getTxt()); break;
            case varObj: printf("ref=%s", v->getTxt()); break;
            }
            putchar('\n');
        }
        for (int i=0; i<3; i++)
            printf("   %sf%d %d/%d\n",
                   i==FuncAtual-FuncPilha ? ">>" : "  ", i,
                   FuncPilha[i].inivar-VarPilha,
                   FuncPilha[i].fimvar-VarPilha);
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
            switch (FuncAtual->linha[2])
            {
            case cRet2:
            case cConstExpr:
                switch (FuncAtual->tipo)
                {
                case 1: // Ler varfunc
                    ApagarRet(FuncAtual->inivar);
                    if (!VarFuncFim())
                        return RetornoErro();
                    break;
                case 2: // Mudar varfunc
                    ApagarVar(FuncAtual->inivar);
                    break;
                case 3: // criar()
                    ApagarVar(FuncAtual->inivar - 2);
                    VarAtual++;
                    VarAtual->Limpar();
                    VarAtual->defvar = InstrVarObjeto;
                    VarAtual->endvar = FuncAtual->este;
                    break;
                default: // Função normal
                    assert(VarAtual >= FuncAtual->inivar);
                    ApagarRet(FuncAtual->inivar-1);
                }
            // Apaga função
                FuncAtual--;
                if (FuncAtual>=FuncPilha)
                    break;
                if (VarFuncIni(VarAtual))
                    break;
                return true;
            case cSe:
            case cSenao2:
                {
                    int desvio = 0;
                    for (TVariavel * v=FuncAtual->fimvar; v<=VarAtual; v++)
                        if (v->getBool()==false)
                        {
                            desvio = Num16(FuncAtual->linha+3);
                            break;
                        }
                    ApagarVar(FuncAtual->fimvar);
                    if (desvio==0)
                    {
                        FuncAtual->linha += Num16(FuncAtual->linha);
                        break;
                    }
                    FuncAtual->linha += desvio;
                    if (FuncAtual->linha[0]==0 && FuncAtual->linha[1]==0)
                        break;
                    if (FuncAtual->linha[2]==cSenao1)
                        FuncAtual->linha += Num16(FuncAtual->linha);
                    else if (FuncAtual->linha[2]==cSenao2)
                    {
                        FuncAtual->expr = FuncAtual->linha + 5;
                        FuncAtual->igualcompara = true;
                    }
                    break;
                }
            case cEnquanto:
                {
                    int desvio = 0;
                    for (TVariavel * v=FuncAtual->fimvar; v<=VarAtual; v++)
                        if (v->getBool()==false)
                        {
                            desvio = Num16(FuncAtual->linha+3);
                            break;
                        }
                    ApagarVar(FuncAtual->fimvar);
                    if (desvio==0)
                        FuncAtual->linha += Num16(FuncAtual->linha);
                    else
                        FuncAtual->linha += desvio;
                    break;
                }
            default:
                FuncAtual->linha += Num16(FuncAtual->linha);
                ApagarVar(FuncAtual->fimvar);
            }
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
            VarAtual->endfixo = FuncAtual->expr;
            while (*FuncAtual->expr++);
            break;
        case ex_num0:
            if (!CriarVar(InstrVarInt))
                return RetornoErro();
            FuncAtual->expr++;
            break;
        case ex_num1:
            if (!CriarVar(InstrVarInt))
                return RetornoErro();
            VarAtual->setInt(1);
            FuncAtual->expr++;
            break;
        case ex_num8p:
            if (!CriarVar(InstrVarInt))
                return RetornoErro();
            VarAtual->setInt((unsigned char)FuncAtual->expr[1]);
            FuncAtual->expr += 2;
            break;
        case ex_num16p:
            if (!CriarVar(InstrVarInt))
                return RetornoErro();
            VarAtual->setInt(Num16(FuncAtual->expr+1));
            FuncAtual->expr += 3;
            break;
        case ex_num32p:
            {
                unsigned int x = Num32(FuncAtual->expr + 1);
                if (x & 0x80000000)
                {
                    if (!CriarVar(InstrDouble))
                        return RetornoErro();
                    VarAtual->setDouble(x);
                }
                else
                {
                    if (!CriarVar(InstrVarInt))
                        return RetornoErro();
                    VarAtual->setInt(x);
                }
                FuncAtual->expr += 5;
                break;
            }
        case ex_num8n:
            if (!CriarVar(InstrVarInt))
                return RetornoErro();
            VarAtual->setInt(-(int)(unsigned char)FuncAtual->expr[1]);
            FuncAtual->expr += 2;
            break;
        case ex_num16n:
            if (!CriarVar(InstrVarInt))
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
                if (!CriarVar(InstrVarInt))
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
                if (VarAtual->tamanho==0 || VarAtual->defvar[2] != cReal)
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
            if (VarFuncIni(VarAtual))
                break;
            FuncAtual->expr++;
            switch (VarAtual->Tipo())
            {
            case varOutros:
            case varObj:
                break;
            case varInt:
                if (VarAtual->tamanho)
                    VarAtual->setInt(-VarAtual->getInt());
                else
                {
                    int valor = -VarAtual->getInt();
                    ApagarVar(VarAtual);
                    if (!CriarVar(InstrVarInt))
                        return RetornoErro();
                    VarAtual->setInt(valor);
                }
                break;
            case varDouble:
                if (VarAtual->tamanho)
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
            if (VarFuncIni(VarAtual))
                break;
            FuncAtual->expr++;
            if (VarAtual->Tipo() != varInt || VarAtual->tamanho==0)
            {
                bool valor = !VarAtual->getBool();
                ApagarVar(VarAtual);
                if (!CriarVar(InstrVarInt))
                    return RetornoErro();
                VarAtual->setInt(valor);
            }
            else
                VarAtual->setInt(!VarAtual->getInt());
            break;
        case exo_mul:        // Operador: a*b
            {
                if (VarFuncIni(VarAtual-1))
                    break;
                FuncAtual->expr++;
                double valor = VarAtual[-1].getDouble() *
                               VarAtual[0].getDouble();
                ApagarVar(VarAtual);
                if (VarAtual->tamanho==0 || VarAtual->Tipo()!=varDouble)
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
                if (VarFuncIni(VarAtual-1))
                    break;
                FuncAtual->expr++;
                double valor1 = VarAtual[-1].getDouble();
                double valor2 = VarAtual[0].getDouble();
                if (valor2)
                    valor1 /= valor2;
                else
                    valor1 *= 1.0e1000;
                ApagarVar(VarAtual);
                if (VarAtual->tamanho==0 || VarAtual->Tipo()!=varDouble)
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
                if (VarFuncIni(VarAtual-1))
                    break;
                FuncAtual->expr++;
                int valor = VarAtual[0].getInt();
                if (valor)
                    valor = VarAtual[-1].getInt() % valor;
                else
                    valor = 0;
                ApagarVar(VarAtual);
                if (VarAtual->tamanho==0 || VarAtual->Tipo()!=varInt)
                {
                    ApagarVar(VarAtual);
                    if (!CriarVar(InstrVarInt))
                        return RetornoErro();
                }
                VarAtual->setInt(valor);
                break;
            }
        case exo_add:        // Operador: a+b
            {
                if (VarFuncIni(VarAtual-1))
                    break;
                FuncAtual->expr++;
                int tipo = VarAtual[-1].Tipo();
            // Caso 1: Não é texto: executa soma numérica
                if (tipo!=varTxt)
                {
                    double valor = VarAtual[-1].getDouble() +
                                   VarAtual[0].getDouble();
                    ApagarVar(VarAtual);
                    if (VarAtual->tamanho==0 || tipo!=varDouble)
                    {
                        ApagarVar(VarAtual);
                        if (!CriarVar(InstrDouble))
                            return RetornoErro();
                    }
                    VarAtual->setDouble(valor);
                    break;
                }
            // Exemplo de comando no intmud.map que usa todos os tipos
            // de concatenação: ("abc" + ("x")) + ("def" + 1) + 4

            // Caso 2: Duas variáveis não locais
            // Transforma em: Primeira variável é cTxtFixo local
                if (VarAtual[-1].tamanho==0 && VarAtual[0].tamanho==0)
                {
                    const char * origem = VarAtual[-1].getTxt();
                    int total = 1 + strlen(origem);
                    if (DadosTopo + total > DadosFim)
                        return RetornoErro();
                    VarAtual[-1].defvar = InstrTxtFixo;
                    VarAtual[-1].endvar = DadosTopo;
                    VarAtual[-1].tamanho = total;
                    memcpy(DadosTopo, origem, total);
                    DadosTopo += total;
                }
            // Caso 3: Primeira variável não é cTxtFixo local
                if (VarAtual[-1].tamanho==0 ||
                    VarAtual[-1].defvar[2]!=cTxtFixo)
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
                // Apaga variáveis locais
                    ApagarVar(VarAtual-1);
                    int total = destino - mens;
                    if (DadosTopo + total > DadosFim)
                        return RetornoErro();
                // Cria variável que conterá o texto
                    VarAtual++;
                    VarAtual->defvar = InstrTxtFixo;
                    VarAtual->endvar = DadosTopo;
                    VarAtual->tamanho = total;
                // Copia texto para variável
                    memcpy(DadosTopo, mens, total);
                    DadosTopo += total;
                    break;
                }
            // Primeira variável é cTxtFixo local
                char * destino = (char*)VarAtual[-1].endvar +
                        VarAtual[-1].tamanho - 1;
                int total = 0;
            // Caso 4: Segunda variável não é local
                if (VarAtual->tamanho==0)
                {
                    const char * origem = VarAtual->getTxt();
                    total = 1 + strlen(origem);
                    if (destino + total >= DadosFim)
                        return RetornoErro();
                    memcpy(destino, origem, total);
                    VarAtual--;
                }
            // Caso 5: Segunda variável é cTxtFixo local
                else if (VarAtual->defvar[2]==cTxtFixo)
                {
                    total = VarAtual->tamanho;
                    VarAtual->Mover(destino, 0, 0);
                    VarAtual--;
                }
            // Caso 6: Segunda variável é local
                else
                {
                    char mens[4096];
                    const char * origem = VarAtual->getTxt();
                    total = 1 + strlen(origem);
                    if (total>4096)
                        total=4096;
                    if (destino + total >= DadosFim)
                        return RetornoErro();
                    memcpy(mens, origem, total);
                    mens[4095]=0;
                    ApagarVar(VarAtual);
                    memcpy(destino, mens, total);
                }
                destino += total;
                VarAtual->tamanho = destino - (char*)VarAtual->endvar;
                DadosTopo = destino;
                break;
            }
        case exo_sub:        // Operador: a-b
            {
                if (VarFuncIni(VarAtual-1))
                    break;
                FuncAtual->expr++;
                double valor = VarAtual[-1].getDouble() -
                                VarAtual[0].getDouble();
                ApagarVar(VarAtual);
                if (VarAtual->tamanho==0 || VarAtual->Tipo()!=varDouble)
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
                if (VarAtual[-1].defvar[2] == cVarFunc)
                {
                    if (FuncAtual+1 >= FuncFim)
                        return RetornoErro();
                    FuncAtual->expr++;
                    FuncAtual++;
                    FuncAtual->linha = VarAtual[-1].defvar +
                            Num16(VarAtual[-1].defvar);
                    FuncAtual->este = (TObjeto*)VarAtual[-1].endvar;
                    FuncAtual->expr = 0;
                    FuncAtual->inivar = VarAtual;
                    FuncAtual->fimvar = VarAtual + 1;
                    FuncAtual->numarg = 1;
                    FuncAtual->tipo = 2;
                    break;
                }
                if (VarFuncIni(VarAtual))
                    break;
                FuncAtual->expr++;
                switch (VarAtual[-1].Tipo())
                {
                case varOutros:
                    if (VarAtual[-1].defvar[2] == VarAtual[0].defvar[2] &&
                            VarAtual[-1].endvar != VarAtual[0].endvar)
                        VarAtual[-1].Igual(VarAtual);
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
                if (VarFuncIni(VarAtual-1))
                    break;
            // Compara valores
                int tipo1 = VarAtual[-1].Tipo();
                int tipo2 = VarAtual[0].Tipo();
                    // Textos
                if (tipo1==varTxt || tipo2==varTxt)
                    tipo1 = comparaZ(VarAtual[-1].getTxt(),
                                    VarAtual->getTxt());
                    // Ref
                else if (tipo1==varObj || tipo2==varObj)
                {
                    if (tipo1 != tipo2)
                        tipo1 -= tipo2;
                    else
                    {
                        TObjeto * v1 = VarAtual[-1].getObj();
                        TObjeto * v2 = VarAtual[0].getObj();
                        tipo1 = (v1==v2 ? 0 : v1<v2 ? -1 : 1);
                    }
                }
                    // Valores numéricos
                else if (tipo1!=varOutros && tipo2!=varOutros)
                {
                    if (tipo1==varInt && tipo2==varInt)
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
                }
                    // Mesmo tipo de variável
                else if (VarAtual[-1].defvar[2] == VarAtual[0].defvar[2])
                    tipo1 = VarAtual[-1].Compara(VarAtual);
                    // Tipos diferentes de variáveis
                else
                {
                    const void * v1 = VarAtual[-1].endvar;
                    const void * v2 = VarAtual[0].endvar;
                    tipo1 = (v1==v2 ? 0 : v1<v2 ? -1 : 1);
                }
            // Apaga valores da pilha; cria int32 na pilha
                ApagarVar(VarAtual-1);
                if (!CriarVar(InstrVarInt))
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
        case exo_igual2:     // Operador: a==b
            {
                if (VarFuncIni(VarAtual-1))
                    break;
                FuncAtual->expr++;
            // Compara valores
                int tipo1 = VarAtual[-1].Tipo();
                int tipo2 = VarAtual[0].Tipo();
                    // Números
                switch (tipo1)
                {
                case varInt:
                    tipo1 = 0;
                    if (tipo2==varInt)
                        tipo1 = (VarAtual[-1].getInt() ==
                                 VarAtual[0].getInt());
                    else if (tipo2==varDouble)
                        tipo1 = (VarAtual[-1].getDouble() ==
                                 VarAtual[0].getDouble());
                    break;
                case varDouble:
                    tipo1 = 0;
                    if (tipo2==varInt || tipo2==varDouble)
                        tipo1 = (VarAtual[-1].getDouble() ==
                                 VarAtual[0].getDouble());
                    break;
                case varTxt:
                    tipo1 = 0;
                    if (tipo2 == varTxt)
                        tipo1 = (strcmp(VarAtual[-1].getTxt(),
                                        VarAtual[0].getTxt())==0);
                    break;
                case varObj:
                    tipo1 = 0;
                    if (tipo2 == varObj)
                        tipo1 = (VarAtual[-1].getObj() ==
                                 VarAtual[0].getObj());
                    break;
                default:
                    if (tipo1 != tipo2)
                        tipo1 = 0;
                    else if (VarAtual[-1].defvar[2] == VarAtual[0].defvar[2])
                        tipo1 = (VarAtual[-1].Compara(VarAtual) == 0);
                    else
                        tipo1 = 0;
                }
            // Apaga valores da pilha; cria int32 na pilha
                ApagarVar(VarAtual-1);
                if (!CriarVar(InstrVarInt))
                    return RetornoErro();
            // Avança Func->expr, verifica operador
                if (tipo1)
                    VarAtual->setInt(1);
                break;
            }
        case exo_ee:         // Operador: Início do operador &
            if (VarFuncIni(VarAtual))
                break;
            FuncAtual->expr++;
            if (VarAtual->getBool()==false)
            {
                const char * p = ProcuraExpr(FuncAtual->expr, exo_e);
                assert(p!=0);
                FuncAtual->expr = p + 1;
                ApagarVar(VarAtual);
                if (!CriarVar(InstrVarInt))
                    return RetornoErro();
                break;
            }
            ApagarVar(VarAtual);
            break;
        case exo_ouou:       // Operador: Início do operador |
            if (VarFuncIni(VarAtual))
                break;
            FuncAtual->expr++;
            if (VarAtual->getBool())
            {
                const char * p = ProcuraExpr(FuncAtual->expr, exo_ou);
                assert(p!=0);
                FuncAtual->expr = p + 1;
                ApagarVar(VarAtual);
                if (!CriarVar(InstrVarInt))
                    return RetornoErro();
                VarAtual->setInt(1);
                break;
            }
            ApagarVar(VarAtual);
            break;
        case exo_e:          // Operador: a&b
        case exo_ou:         // Operador: a|b
            {
                if (VarFuncIni(VarAtual))
                    break;
                FuncAtual->expr++;
                bool b = VarAtual->getBool();
                ApagarVar(VarAtual);
                if (!CriarVar(InstrVarInt))
                    return RetornoErro();
                if (b)
                    VarAtual->setInt(1);
                break;
            }
        case exo_igualadd:   // Operador: a+=b
            if (VarAtual[-1].Tipo() == varTxt)
            {
                FuncAtual->expr += 3;
                VarAtual[-1].addTxt(VarAtual[0].getTxt());
                ApagarVar(VarAtual);
                break;
            }
        case exo_igualmul:   // Operador: a*=b
        case exo_igualdiv:   // Operador: a/=b
        case exo_igualporcent: // Operador: a%=b
        case exo_igualsub:   // Operador: a-=b
            FuncAtual->expr++;
            if (VarAtual >= VarFim-1)
                return RetornoErro();
            VarAtual[1] = VarAtual[0];
            VarAtual[0] = VarAtual[-1];
            VarAtual[0].tamanho = 0;
            VarAtual++;
            break;

    // A partir daqui processa funções e variáveis
    // Cria-se uma variável cVarNome, que contém parte do nome da variável
    // Cria-se uma variável em seguida, que contém o resultado anterior
    // Isso torna possível processar várias variáveis ligadas por ponto
    // Exemplo: a.b.c
        case ex_varini:     // Início do texto
        case ex_varabre:    // Início do texto + abre colchetes
            if (!CriarVar(InstrVarNome))
                return RetornoErro();
            if (!CriarVar(InstrVarInicio))
                return RetornoErro();
            if (*FuncAtual->expr++ == ex_varini)
                FuncAtual->expr = CopiaVarNome(VarAtual-1, FuncAtual->expr);
            break;
        case ex_arg:        // Início da lista de argumentos
        case ex_abre:       // Abre colchetes
            FuncAtual->expr++;
            break;
        case ex_fecha:      // Fecha colchetes
            {
                TVariavel * v = EndVarNome();
                if (VarFuncIni(v+2))
                    break;
                FuncAtual->expr++;
                char mens[VAR_NOME_TAM];
                char * p = mens;
                char * pfim = p + sizeof(mens);
                *mens=0;
                assert(v!=0);
                for (TVariavel * x = v+2; x<=VarAtual; x++)
                    p = copiastr(p, x->getTxt(), pfim-p);
                ApagarVar(v+2);
                CopiaVarNome(v, mens);
                FuncAtual->expr = CopiaVarNome(v, FuncAtual->expr);
                break;
            }
        case ex_varfim:     // Fim do texto
            {
                FuncAtual->expr++;
                TVariavel * v = EndVarNome();
                assert(v!=0);
                int tipo = v[1].defvar[2];
            // cVarInicio ou cVarClasse: resultado nulo
                if (tipo==cVarInicio || tipo==cVarClasse)
                {
                    ApagarVar(v);
                    if (!CriarVar(InstrNulo))
                        return RetornoErro();
                    break;
                }
            // Outro valor
                ApagarVar(v+2);
                DadosTopo = (char*)v->endvar;
                if (VarAtual->tamanho)
                {
                    VarAtual->Mover(v->endvar, 0, 0);
                    DadosTopo += VarAtual->tamanho;
                }
                *v = *VarAtual;
                VarAtual = v;
                break;
            }
        case ex_doispontos:
            {
                TVariavel * v = EndVarNome();
                assert(v!=0);
                if (v[1].defvar[2] == cVarInicio)
                {
                    TClasse * c = TClasse::Procura(v->getTxt());
                    if (c)
                    {
                        v[1].defvar = InstrVarClasse;
                        v[1].endvar = c;
                        v->setTxt("");
                        FuncAtual->expr = CopiaVarNome(
                                v, FuncAtual->expr+1);
                        break;
                    }
                }
                VarInvalido();
                break;
            }
        case ex_ponto:
            {
                FuncAtual->expr++;
                TVariavel * v = EndVarNome();
                const char * nome = v->getTxt(); // Nome encontrado
                TClasse * classe = 0; // Procurar variável de classe/objeto
                TObjeto * objeto = 0; // Procurar variável de objeto
                assert(v!=0);

            // Verifica variável/função da classe
                if (v[1].defvar[2]==cVarClasse)
                {
                    classe = (TClasse*)v[1].endvar;
                    objeto = FuncAtual->este;
                }
            // Verifica variável/função da classe
                else if (v[1].defvar[2]==cVarInicio)
                {
                // Se começa com $, verifica se objeto existe
                    if (nome[0]=='$')
                    {
                        TClasse * c = TClasse::Procura(nome+1);
                        if (c==0 || c->ObjetoIni==0)
                        {
                            VarInvalido();
                            break;
                        }
                        ApagarVar(v+1);
                        VarAtual++;
                        VarAtual->defvar = InstrVarObjeto;
                        VarAtual->endvar = c->ObjetoIni;
                        VarAtual->tamanho = 0;
                        v->setTxt("");
                        FuncAtual->expr = CopiaVarNome(v, FuncAtual->expr);
                        break;
                    }
                // Verifica se é variável/comando interno do programa
                    int ini = 0;
                    int fim = sizeof(ListaFunc) / sizeof(ListaFunc[0]) - 1;
                    int resultado = 1;
                    while (ini <= fim)
                    {
                        int meio = (ini+fim)/2;
                        resultado = comparaZ(nome, ListaFunc[meio].Nome);
                    // Verifica se encontrou
                        if (resultado==0)  // Se encontrou
                        {
                                // Ler varfunc primeiro
                            ExecFunc * f = FuncAtual;
                            if (VarFuncIni(v+1))
                            {
                                f->expr--;
                                break;
                            }
                                // Processa função interna
                            if (ListaFunc[meio].Func(
                                    v+1, ListaFunc[meio].valor))
                            {
                                v->setTxt("");
                                f->expr = CopiaVarNome(v, f->expr);
                            }
                            else
                                VarInvalido();
                            break;
                        }
                        if (resultado<0)
                            fim = meio-1;
                        else
                            ini = meio+1;
                    }
                    if (resultado==0)
                        break;
                // Verifica se é variável local da função
                    TVariavel * var = FuncAtual->inivar + FuncAtual->numarg;
                    for (; var < FuncAtual->fimvar; var++)
                        if (comparaZ(nome, var->defvar+endNome)==0)
                            break;
                    if (var < FuncAtual->fimvar)
                    {
                        ApagarVar(v+1);
                        VarAtual++;
                        *VarAtual = *var;
                        VarAtual->tamanho = 0;
                        v->setTxt("");
                        FuncAtual->expr = CopiaVarNome(v, FuncAtual->expr);
                        break;
                    }
                // Verifica se é variável/função do objeto
                    objeto = FuncAtual->este;
                    if (objeto)
                        classe = objeto->Classe;
                }
            // Verifica variável/função de objeto
                else if (v[1].Tipo()==varObj)
                {
                    objeto = v[1].getObj();
                    if (objeto==0) // Objeto inexistente
                    {
                        VarInvalido();
                        break;
                    }
                    classe = objeto->Classe;
                }
            // Consulta a variável sobre o que fazer
                else
                {
                // Ler varfunc primeiro
                    ExecFunc * f = FuncAtual;
                    if (VarFuncIni(v+1))
                    {
                        f->expr--;
                        break;
                    }
                // Executa função
                    if (v[1].Func(nome))
                    {
                        ApagarVar(v+2);
                        FuncAtual = f;
                        v->setTxt("");
                        f->expr = CopiaVarNome(v, f->expr);
                    }
                    else
                        VarInvalido();
                    break;
                }

            // Processa classe e objeto
                if (classe==0)
                {
                    VarInvalido();
                    break;
                }
                int indice = classe->IndiceNome(nome);
                if (indice<0) // Variável/função inexistente
                {
                    VarInvalido();
                    break;
                }
                char * defvar = classe->InstrVar[indice];
                int indvar = classe->IndiceVar[indice];
                v->setTxt("");
                FuncAtual->expr = CopiaVarNome(v, FuncAtual->expr);
            // Função
                if (defvar[2]==cFunc)
                {
                    if (FuncAtual >= FuncFim - 1)
                        return RetornoErro();
                    FuncAtual++;
                    FuncAtual->linha = defvar + Num16(defvar);
                    FuncAtual->este = objeto;
                    FuncAtual->expr = 0;
                    FuncAtual->inivar = v+2;
                    FuncAtual->fimvar = VarAtual + 1;
                    FuncAtual->numarg = FuncAtual->fimvar - FuncAtual->inivar;
                    FuncAtual->tipo = 0;
                    break;
                }
            // Função varfunc
                if (defvar[2]==cVarFunc)
                {
                    ApagarVar(v+1);
                  //  if (objeto==0)
                  //  {
                  //      if (!CriarVar(InstrNulo))
                  //          return RetornoErro();
                  //      break;
                  //  }
                    VarAtual++;
                    VarAtual->defvar = defvar;
                    VarAtual->endvar = objeto;
                    VarAtual->tamanho = 0;
                    VarAtual->indice = 0;
                    break;
                }
            // Expressão numérica
                if (defvar[2]==cConstExpr)
                {
                    if (FuncAtual >= FuncFim - 1)
                        return RetornoErro();
                    FuncAtual++;
                    FuncAtual->linha = defvar;
                    FuncAtual->este = objeto;
                    FuncAtual->expr = defvar + defvar[endIndice];
                    FuncAtual->inivar = v + 2;
                    FuncAtual->fimvar = VarAtual + 1;
                    FuncAtual->numarg = FuncAtual->fimvar - FuncAtual->inivar;
                    FuncAtual->tipo = 0;
                    break;
                }
            // Variável
                ApagarVar(v+1);
                VarAtual++;
                VarAtual->defvar = defvar;
                VarAtual->tamanho = 0;
                VarAtual->bit = indvar >> 24;
                VarAtual->indice = (defvar[endVetor]==0 ? 0 : 0xFF);
                if (defvar[2]==cConstTxt || // Constante
                        defvar[2]==cConstNum)
                    VarAtual->endvar = 0;
                else if (indvar & 0x400000) // Variável da classe
                    VarAtual->endvar = classe->Vars +
                            (indvar & 0x3FFFFF);
                else if (objeto) // Variável do objeto
                    VarAtual->endvar = objeto->Vars +
                            (indvar & 0x3FFFFF);
                else // Objeto inexistente
                    VarInvalido();
                break;
            }
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
// Apaga variáveis pendentes
    ApagarVar(VarPilha);

// Apaga objetos marcados para exclusão
    while (true)
    {
        TObjeto * obj = TObjeto::IniApagar;
        if (obj==0)
            break;
        if (Instr::ExecIni(obj, "fim"))
        {
            Instr::ExecX();
            ApagarVar(VarPilha);
        }
        TObjeto::DesmarcarApagar();
        obj->Apagar();
    }


    // Se apagar alguma classe aqui, acertar também função main()

}
