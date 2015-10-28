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
#include "instr.h"
#include "variavel.h"
#include "var-log.h"
#include "classe.h"
#include "objeto.h"
#include "var-prog.h"
#include "var-debug.h"
#include "mudarclasse.h"
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
- cVarIniFunc -> Somente se OTIMIZAR_VAR estiver definido (em instr.h):
    - Internamente funciona como cVarInt (guarda um int)
    - Se o valor for >=0, é o número da função interna (ex_varfunc)
    - Se o valor for <0, é uma variável do objeto (ex_varlocal seguido de 0xFF)
    - Quando for variável local (ex_varlocal seguido de um valor != 0xFF),
      a variável local é colocada diretamente na pilha

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
     Os operadores && e || podem avançar na expressão numérica
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
        FuncAtual->nome = linha que contém o nome da função
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
        Consulta a variável sobre o que fazer; pode cair nos passos 2 ou 3
  Nunca deverá chegar aqui; executar assert(0)
@endverbatim
*/

//------------------------------------------------------------------------------
int Instr::VarExec = 0;
int Instr::VarExecIni = 5000;

// Para gerar evento para avisar sobre erros
static char * erromens = 0;

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
const char Instr::InstrNulo[] = { 9, 0, Instr::cConstNulo, (char)0xFF, 0, 0, 0, '+', 0 };
const char Instr::InstrDouble[] = { 9, 0, Instr::cReal2, (char)0xFF, 0, 0, 0, '+', 0 };
const char Instr::InstrSocket[] = { 9, 0, Instr::cSocket, (char)0xFF, 0, 0, 0, '+', 0 };
const char Instr::InstrTxtFixo[] = { 9, 0, Instr::cTxtFixo, (char)0xFF, 0, 0, 0, '+', 0 };
const char Instr::InstrVarNome[] = { 9, 0, Instr::cVarNome, (char)0xFF, 0, 0, 0, '+', 0 };
const char Instr::InstrVarInicio[] = { 9, 0, Instr::cVarInicio, (char)0xFF, 0, 0, 0, '+', 0 };
const char Instr::InstrVarIniFunc[] = { 9, 0, Instr::cVarIniFunc, (char)0xFF, 0, 0, 0, '+', 0 };
const char Instr::InstrVarClasse[] = { 9, 0, Instr::cVarClasse, (char)0xFF, 0, 0, 0, '+', 0 };
const char Instr::InstrVarObjeto[] = { 9, 0, Instr::cVarObjeto, (char)0xFF, 0, 0, 0, '+', 0 };
const char Instr::InstrVarInt[] = { 9, 0, Instr::cVarInt, (char)0xFF, 0, 0, 0, '+', 0 };
const char Instr::InstrVarListaItem[] = { 9, 0, Instr::cListaItem, (char)0xFF, 0, 0, 0, '+', 0 };
const char Instr::InstrVarTextoPos[] =  { 9, 0, Instr::cTextoPos, (char)0xFF, 0, 0, 0, '+', 0 };
const char Instr::InstrVarTextoVarSub[] =  { 9, 0, Instr::cTextoVarSub, (char)0xFF, 0, 0, 0, '+', 0 };
const char Instr::InstrVarTextoObjSub[] =  { 9, 0, Instr::cTextoObjSub, (char)0xFF, 0, 0, 0, '+', 0 };
const char Instr::InstrDebugFunc[] = { 9, 0, cFunc, (char)0xFF, 0, 0, 0, 'f', 0 };

static void ApagaVarEscopo()
{
    using namespace Instr;
    while ((unsigned char)FuncAtual->linha[endAlin] < FuncAtual->indent)
    {
        //printf("APAGOU %d\n", VarAtual-VarPilha); fflush(stdout);
        ApagarVar(VarAtual);
        FuncAtual->fimvar--;
        if (VarAtual < FuncAtual->inivar + FuncAtual->numarg)
            FuncAtual->indent = 0;
        else
            FuncAtual->indent = VarAtual->nomevar[endAlin];
    }
}

//----------------------------------------------------------------------------
/// Prepara para executar
/** @param classe Endereço da classe (objeto este é NULO)
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
    const char * nome = classe->InstrVar[indice];
    const char * instr = nome;
    const char * expr = 0; // Não está processando expressão numérica
    if (instr[0]==0 && instr[1]==0)
        return false;
    switch (instr[2])
    {
    case cFunc:
    case cVarFunc:
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
            case cConstVar:
                instr = vazio;
            }
        break;
    case cConstTxt:
    case cConstNum:
    case cConstExpr:
    case cConstVar:
        expr = instr + instr[endIndice];
        break;
    default:
        return false;
    }
    VarExec = VarExecIni;
// Acerta pilhas
    DadosTopo = DadosPilha; // Limpa pilha de dados
    VarAtual = VarPilha;    // Limpa pilha de variáveis
    VarAtual->Limpar();     // Variável atual é nulo
    VarAtual->defvar = InstrNulo;
    VarAtual->nomevar = InstrNulo;
    FuncAtual = FuncPilha;  // Limpa pilha de funções
    FuncAtual->nome = nome;
    FuncAtual->este = 0;    // Nenhum objeto
    FuncAtual->linha = instr;// Primeira instrução da função
    FuncAtual->expr = expr;
    FuncAtual->inivar = VarAtual + 1; // Endereço do primeiro argumento
    FuncAtual->fimvar = VarAtual + 1;
    FuncAtual->numarg = 0;  // Número de argumentos da função
    FuncAtual->tipo = 0;
    FuncAtual->indent = 0;
    return true;
}

//----------------------------------------------------------------------------
/// Prepara para executar
/** @param este Endereço do objeto "este"
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
/** @param txt Texto a adicionar
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
/** @param valor Número
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
/** @param def Definição da variável, conforme Instr::Codif()
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
 *  @param  motivo O motivo para chamar essa função:
 *              - 0 = VarExec chegou a zero
 *              - 1 = Memória insuficiente para chamar função
 *              - 2 = Memória insuficiente para criar variável
 *              - 3 = Erro desconhecido
 *  @return false
 */
static bool RetornoErro(int motivo)
{
// Anota erro
    if (erromens == 0)
    {
        char mens[BUF_MENS];
        char * p = mens;
        const char * pfim = mens + sizeof(mens);
        switch (motivo)
        {
        case 0: p = copiastr(p, "Exec chegou a 0"); break;
        case 1: p = copiastr(p, "Atingido limite de funções"); break;
        case 2: p = copiastr(p, "Atingido limite de variáveis"); break;
        default: p = copiastr(p, "Erro desconhecido"); break;
        }

        for (Instr::ExecFunc * f = Instr::FuncPilha; f<=Instr::FuncAtual; f++)
        {
        // Checa memória
            if (pfim-p < CLASSE_NOME_TAM + 30)
                break;
        // Nome da função
            *p++ = Instr::ex_barra_n;
            p = copiastr(p, "Função: ");
            Instr::Decod(p, f->nome, pfim-p);
            while (*p)
                p++;
            if (f->este)
                p = mprintf(p, pfim-p, " (objeto=%s)", f->este->Classe->Nome);
        // Checa memória
            if (pfim-p < CLASSE_NOME_TAM + 30)
                break;
        // Linha da função
            if (f->nome == f->linha)
                continue;
            *p++ = Instr::ex_barra_n;
            p = copiastr(p, "Linha:  ");
            Instr::Decod(p, f->linha, pfim-p);
            while (*p)
                p++;
        }

        erromens = new char[p+1-mens];
        memcpy(erromens, mens, p-mens);
        erromens[p-mens] = 0;
    }
// Apaga variáveis da pilha
    TVarProg::LimparVar(); // Apaga referências das variáveis prog
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
    const char * DebugInstr = 0;
    FuncAtual->funcdebug = 0;
    while (true)
    {
        if (FuncAtual->expr==0)
        {
    // Está processando uma linha
            if (FuncAtual->funcdebug && DebugInstr != FuncAtual->linha)
            {
                const char * l = FuncAtual->linha;
                if (l && (l[0] || l[1]) && l[2]!=cSenao1 && l[2]!=cSenao2)
                    TVarDebug::Exec();
                DebugInstr = FuncAtual->linha;
            }
            if (--VarExec < 0)
                return RetornoErro(0);

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
                case cConstVar:
                    FuncAtual->linha = 0;
                }
            if (FuncAtual->linha == 0)
            {
                bool b = (FuncAtual->funcdebug == 0);
                switch (FuncAtual->tipo)
                {
                case 1: // Ler varfunc
                    ApagarVar(FuncAtual->inivar);
                    if (VarAtual >= VarFim-1)
                        return RetornoErro(2);
                    VarAtual++;
                    VarAtual->Limpar();
                    VarAtual->defvar = InstrNulo;
                    if (!VarFuncFim())
                        return RetornoErro(1);
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
                case 4: // debug.cmd
                    ApagarVar(FuncAtual->inivar - 1);
                    VarAtual++;
                    VarAtual->Limpar();
                    VarAtual->defvar = InstrTxtFixo;
                    VarAtual->endfixo = "";
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
                {
                    if (b)
                        DebugInstr = FuncAtual->linha;
                    continue;
                }
                if (VarFuncIni(VarAtual))
                    continue;
                return true;
            }

        // Mostra o que vai executar
#ifdef DEBUG_INSTR
            {
                char mens[4096];
                printf(">>> %d %d  ",
                       static_cast<int>(DadosFim-DadosTopo),
                       static_cast<int>(VarFim-VarAtual));
                printf(" %d %d\n", (unsigned char)FuncAtual->linha[endAlin],
                                FuncAtual->indent);
                if (Instr::Decod(mens, FuncAtual->linha, sizeof(mens)))
                    printf("Exec: %s\n", mens);
                else
                    printf("Erro ao ler instrução\n");
                fflush(stdout);
            }
#endif

        // Apaga variáveis que saíram do escopo
            ApagaVarEscopo();

        // Variável da função
            if (FuncAtual->linha[2] > cVariaveis)
            {
                if (!CriarVar(FuncAtual->linha))
                    return RetornoErro(2);
                FuncAtual->fimvar++;
                FuncAtual->indent = VarAtual->defvar[endAlin];
                FuncAtual->linha += Num16(FuncAtual->linha);
                //printf("CRIOU %d\n", VarAtual-VarPilha); fflush(stdout);
                continue;
            }

        // Processa instrução
            switch (FuncAtual->linha[2])
            {
            case cRefVar: // Referência a uma variável
                {
                    const char * p = FuncAtual->linha + endNome;
                    while (*p++);
                    FuncAtual->expr = p;
                    break;
                }
            case cSe:   // Processa expressão numérica
            case cEnquanto:
            case cCasoVar:
            case cSair2:
            case cContinuar2:
                FuncAtual->expr = FuncAtual->linha + endVar + 2;
                break;
            case cEPara:
                FuncAtual->expr = FuncAtual->linha + endVar + 6;
                break;
            case cRet2: // Processa expressão numérica
            case cExpr:
                FuncAtual->expr = FuncAtual->linha + endVar;
                break;
            case cComent: // Comentário
                VarExec++;
                FuncAtual->linha += Num16(FuncAtual->linha);
                break;
            case cSenao1: // Avança
            case cSenao2:
            case cSair1:
                {
                    int desvio = Num16(FuncAtual->linha + endVar);
                    if (desvio==0)
                        FuncAtual->linha += Num16(FuncAtual->linha);
                    else
                        FuncAtual->linha += desvio;
                    break;
                }
            case cEFim:  // Recua
            case cContinuar1:
                {
                    const char * p = FuncAtual->linha;
                    int desvio = Num16(p + endVar);
                    if (desvio==0)
                        p += Num16(p);
                    else
                    {
                        p -= desvio;
                        if (p[2] == cEPara)
                            FuncAtual->expr = p + Num16(p + endVar + 4);
                    }
                    FuncAtual->linha = p;
                    ApagaVarEscopo();
                    break;
                }
            case cFimSe: // Passa para próxima instrução
            case cCasoSe:
            case cCasoSePadrao:
            case cCasoFim:
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
            if (v->nomevar)
                printf("[nome=%s] ", v->nomevar + Instr::endNome);
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
                   static_cast<int>(FuncPilha[i].inivar-VarPilha),
                   static_cast<int>(FuncPilha[i].fimvar-VarPilha));
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
#ifdef DEBUG_INSTR
            {
                char mens[4096];
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
            case cConstVar:
                FuncAtual->expr = 0;
                switch (FuncAtual->tipo)
                {
                case 1: // Ler varfunc
                    ApagarRet(FuncAtual->inivar);
                    if (!VarFuncFim())
                        return RetornoErro(2);
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
                case 4: // debug.cmd
                    {
                        assert(VarAtual >= FuncAtual->inivar);
                        char mens[BUF_MENS];
                        char * p = copiastr(mens, VarAtual->getTxt(), sizeof(mens));
                        ApagarVar(FuncAtual->inivar - 1);
                        if (!CriarVarTexto(mens, p-mens))
                        {
                            VarAtual++;
                            VarAtual->Limpar();
                            VarAtual->defvar = InstrNulo;
                        }
                        break;
                    }
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
            case cRefVar: // Referência a uma variável
                FuncAtual->expr = 0;
                assert(VarAtual >= FuncAtual->fimvar);
                ApagarVar(FuncAtual->fimvar + 1);
                VarAtual->nomevar = FuncAtual->linha;
                FuncAtual->fimvar++;
                FuncAtual->indent = VarAtual->nomevar[endAlin];
                FuncAtual->linha += Num16(FuncAtual->linha);
                break;
            case cSe:
            case cSenao2:
                if (!VarFuncIni(VarAtual))
                {
                    FuncAtual->expr = 0;
                    int desvio = 0;
                    if (VarAtual->getBool()==false)
                        desvio = Num16(FuncAtual->linha + endVar);
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
                    {
                        if (FuncAtual->funcdebug==0)
                            FuncAtual->linha += Num16(FuncAtual->linha);
                        else
                        {
                            // Executa função de debug e volta para
                            // case cSenao1 (abaixo)
                            static const char tmp[] = { ex_fim };
                            FuncAtual->expr = tmp;
                            TVarDebug::Exec();
                        }
                    }
                    else if (FuncAtual->linha[2]==cSenao2)
                    {
                        FuncAtual->expr = FuncAtual->linha + endVar + 2;
                        if (FuncAtual->funcdebug)
                            TVarDebug::Exec();
                    }
                }
                break;
            case cSair2:
                if (!VarFuncIni(VarAtual))
                {
                    FuncAtual->expr = 0;
                    int desvio = 0;
                    if (VarAtual->getBool())
                        desvio = Num16(FuncAtual->linha + endVar);
                    if (desvio==0)
                        FuncAtual->linha += Num16(FuncAtual->linha);
                    else
                        FuncAtual->linha += desvio;
                    ApagarVar(FuncAtual->fimvar);
                }
                break;
            case cContinuar2:
                if (!VarFuncIni(VarAtual))
                {
                    const char * p = FuncAtual->linha;
                    FuncAtual->expr = 0;
                    if (!VarAtual->getBool() ||
                            (p[endVar]==0 && p[endVar+1]==0))
                        p += Num16(p);
                    else
                    {
                        p -= Num16(p + endVar);
                        if (p[2] == cEPara)
                            FuncAtual->expr = p + Num16(p + endVar + 4);
                    }
                    FuncAtual->linha = p;
                    ApagarVar(FuncAtual->fimvar);
                    ApagaVarEscopo();
                }
                break;
            case cSenao1: // senão sem argumentos em modo debug
                FuncAtual->expr = 0;
                FuncAtual->linha += Num16(FuncAtual->linha);
                ApagarVar(FuncAtual->fimvar);
                break;
            case cEnquanto:
                if (!VarFuncIni(VarAtual))
                {
                    FuncAtual->expr = 0;
                    int desvio = 0;
                    if (VarAtual->getBool()==false)
                        desvio = Num16(FuncAtual->linha + endVar);
                    ApagarVar(FuncAtual->fimvar);
                    if (desvio==0)
                        FuncAtual->linha += Num16(FuncAtual->linha);
                    else
                        FuncAtual->linha += desvio;
                }
                break;
            case cEPara:
                {
                    const char * p = FuncAtual->linha;
                    FuncAtual->expr = p + Num16(p + endVar + 2);
                    break;
                }
            case cCasoVar:
                if (!VarFuncIni(VarAtual))
                {
                    FuncAtual->expr = 0;
                    if (FuncAtual->linha[endVar]==0 &&
                        FuncAtual->linha[endVar+1]==0)
                    {
                        FuncAtual->linha += Num16(FuncAtual->linha);
                        ApagarVar(FuncAtual->fimvar);
                        break;
                    }
                    char texto[512];
                    copiastr(texto, FuncAtual->fimvar->getTxt(), sizeof(texto));
                    ApagarVar(FuncAtual->fimvar);
                    FuncAtual->linha += Num16(FuncAtual->linha + endVar);
                    while (true)
                    {
                        if (FuncAtual->linha[0]==0 && FuncAtual->linha[1]==0)
                            break;
                        if (FuncAtual->linha[2]==cCasoSePadrao ||
                            FuncAtual->linha[2]==cCasoFim)
                        {
                            FuncAtual->linha += Num16(FuncAtual->linha);
                            break;
                        }
                        if (FuncAtual->linha[2]!=cCasoSe)
                            break;
                        int cmp = strcmp(texto, FuncAtual->linha + endVar + 4);
                        if (cmp==0)
                        {
                            FuncAtual->linha += Num16(FuncAtual->linha);
                            break;
                        }
                        const char * p = FuncAtual->linha + endVar;
                        if (cmp>0)
                            p+=2;
                        FuncAtual->linha += ((signed char)p[1] * 0x100)
                                            | (unsigned char)p[0];
                    }
                }
                break;
            default:
                FuncAtual->expr = 0;
                FuncAtual->linha += Num16(FuncAtual->linha);
                ApagarVar(FuncAtual->fimvar);
            }
            break;
        case ex_nulo:
            FuncAtual->expr++;
            if (VarAtual >= VarFim-1)
                return RetornoErro(2);
            VarAtual++;
            VarAtual->Limpar();
            VarAtual->defvar = InstrNulo;
            break;
        case ex_txt:
            FuncAtual->expr++;
            if (VarAtual >= VarFim-1)
                return RetornoErro(2);
            VarAtual++;
            VarAtual->Limpar();
            VarAtual->defvar = InstrTxtFixo;
            VarAtual->endfixo = FuncAtual->expr;
            while (*FuncAtual->expr++);
            break;
        case ex_num0:
            if (!CriarVarInt(0))
                return RetornoErro(2);
            FuncAtual->expr++;
            break;
        case ex_num1:
            if (!CriarVarInt(1))
                return RetornoErro(2);
            FuncAtual->expr++;
            break;
        case ex_num8p:
        case ex_num8hexp:
            if (!CriarVarInt((unsigned char)FuncAtual->expr[1]))
                return RetornoErro(2);
            FuncAtual->expr += 2;
            break;
        case ex_num16p:
        case ex_num16hexp:
            if (!CriarVarInt(Num16(FuncAtual->expr+1)))
                return RetornoErro(2);
            FuncAtual->expr += 3;
            break;
        case ex_num32p:
        case ex_num32hexp:
            {
                unsigned int x = Num32(FuncAtual->expr + 1);
                if (x & 0x80000000)
                {
                    if (!CriarVar(InstrDouble))
                        return RetornoErro(2);
                    VarAtual->setDouble(x);
                }
                else if (!CriarVarInt(x))
                    return RetornoErro(2);
                FuncAtual->expr += 5;
                break;
            }
        case ex_num8n:
        case ex_num8hexn:
            if (!CriarVarInt( -(int)(unsigned char)FuncAtual->expr[1] ))
                return RetornoErro(2);
            FuncAtual->expr += 2;
            break;
        case ex_num16n:
        case ex_num16hexn:
            if (!CriarVarInt( -(int)Num16(FuncAtual->expr+1) ))
                return RetornoErro(2);
            FuncAtual->expr += 3;
            break;
        case ex_num32n:
        case ex_num32hexn:
            if (*((unsigned char*)FuncAtual->expr+4) & 0x80)
            {
                if (!CriarVar(InstrDouble))
                    return RetornoErro(2);
                VarAtual->setDouble(-(double)Num32(FuncAtual->expr+1));
            }
            else if (!CriarVarInt( -(int)Num32(FuncAtual->expr+1) ))
                return RetornoErro(2);
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
                if (VarAtual->tamanho==0 || VarAtual->defvar[2] != cReal2)
                {
                    ApagarVar(VarAtual);
                    CriarVar(InstrDouble);
                }
                VarAtual->setDouble(valor);
                break;
            }
        case exo_virg_expr:  // Operador: Vírgula, para separar expressões
            switch (FuncAtual->linha[2])
            {
            case cRet2:
                if (VarFuncIni(VarAtual))
                    break;
                if (VarAtual->getBool())
                {
                    //ApagarVar(FuncAtual->fimvar);
                    ApagarVar(VarAtual);
                    FuncAtual->expr++;
                    break;
                }
                ApagarVar(FuncAtual->fimvar);
                FuncAtual->linha += Num16(FuncAtual->linha);
                FuncAtual->expr = 0;
                break;
            case cEPara:
                // Se completou o primeiro argumento, passa para o segundo
                if (FuncAtual->fimvar == VarAtual)
                {
                    FuncAtual->expr++;
                    //const char * p = FuncAtual->linha;
                    //FuncAtual->expr = p + Num16(p + endVar + 2);
                    break;
                }
                // Fecha o laço, exatamente como na instrução "enquanto"
                if (!VarFuncIni(VarAtual))
                {
                    FuncAtual->expr = 0;
                    int desvio = 0;
                    if (VarAtual->getBool()==false)
                        desvio = Num16(FuncAtual->linha + endVar);
                    ApagarVar(FuncAtual->fimvar);
                    if (desvio==0)
                        FuncAtual->linha += Num16(FuncAtual->linha);
                    else
                        FuncAtual->linha += desvio;
                }
                break;
            default:
                FuncAtual->expr++;
            }
            break;
        case exo_virgula:    // Operador: Vírgula, para separar argumentos
        case exo_ini:        // Operador: Marca o início dos operadores
        case exo_fim:        // Operador: Marca o fim dos operadores
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
                    if (!CriarVarInt(valor))
                        return RetornoErro(2);
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
                        return RetornoErro(2);
                    VarAtual->setDouble(valor);
                }
            }
            break;
        case exo_exclamacao: // Operador: !a
            if (VarFuncIni(VarAtual))
                break;
            FuncAtual->expr++;
            if (VarAtual->tamanho==0 || VarAtual->defvar!=InstrVarInt)
            {
                bool valor = !VarAtual->getBool();
                ApagarVar(VarAtual);
                if (!CriarVarInt(valor))
                    return RetornoErro(2);
            }
            else
                VarAtual->setInt(!VarAtual->getInt());
            break;
        case exo_b_comp: // Operador: ~a
            if (VarFuncIni(VarAtual))
                break;
            FuncAtual->expr++;
            if (VarAtual->tamanho==0 || VarAtual->defvar!=InstrVarInt)
            {
                int valor = ~VarAtual->getInt();
                ApagarVar(VarAtual);
                if (!CriarVarInt(valor))
                    return RetornoErro(2);
            }
            else
                VarAtual->setInt(~VarAtual->getInt());
            break;
        case exo_mul:        // Operador: a*b
            {
                if (VarFuncIni(VarAtual-1))
                    break;
                FuncAtual->expr++;
                double valor = VarAtual[-1].getDouble() *
                               VarAtual[0].getDouble();
                ApagarVar(VarAtual);
                if (VarAtual->tamanho==0 || VarAtual->defvar!=InstrDouble)
                {
                    ApagarVar(VarAtual);
                    if (!CriarVar(InstrDouble))
                        return RetornoErro(2);
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
                if (VarAtual->tamanho==0 || VarAtual->defvar!=InstrDouble)
                {
                    ApagarVar(VarAtual);
                    if (!CriarVar(InstrDouble))
                        return RetornoErro(2);
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
                    if (!CriarVarInt(0))
                        return RetornoErro(2);
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
                    if (VarAtual->tamanho==0 || VarAtual->defvar!=InstrDouble)
                    {
                        ApagarVar(VarAtual);
                        if (!CriarVar(InstrDouble))
                            return RetornoErro(2);
                    }
                    VarAtual->setDouble(valor);
                    break;
                }
            // Exemplo de comando no intmud.int que usa todos os tipos
            // de concatenação: ("abc" + ("x")) + ("def" + 1) + 4

            // Caso 2: Duas variáveis não locais
            // Transforma em: Primeira variável é cTxtFixo local
                if (VarAtual[-1].tamanho==0 && VarAtual[0].tamanho==0)
                {
                    const char * origem = VarAtual[-1].getTxt();
                    int total = 1 + strlen(origem);
                    if (DadosTopo + total > DadosFim)
                        return RetornoErro(2);
                    VarAtual[-1].defvar = InstrTxtFixo;
                    VarAtual[-1].nomevar = 0;
                    VarAtual[-1].endvar = DadosTopo;
                    VarAtual[-1].tamanho = total;
                    memcpy(DadosTopo, origem, total);
                    DadosTopo += total;
                }
            // Caso 3: Primeira variável não é cTxtFixo local
                if (VarAtual[-1].tamanho==0 ||
                    VarAtual[-1].defvar[2]!=cTxtFixo)
                {
                    char mens[BUF_MENS];
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
                        return RetornoErro(2);
                // Cria variável que conterá o texto
                    VarAtual++;
                    VarAtual->defvar = InstrTxtFixo;
                    VarAtual->nomevar = 0;
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
                        return RetornoErro(2);
                    memcpy(destino, origem, total);
                    VarAtual--;
                }
            // Caso 5: Segunda variável é cTxtFixo local
                else if (VarAtual->defvar[2]==cTxtFixo)
                {
                    total = VarAtual->tamanho;
                    VarAtual->MoverEnd(destino, 0, 0);
                    VarAtual--;
                }
            // Caso 6: Segunda variável é local
                else
                {
                    char mens[BUF_MENS];
                    const char * origem = VarAtual->getTxt();
                    total = 1 + strlen(origem);
                    if (total>BUF_MENS)
                        total=BUF_MENS;
                    if (destino + total >= DadosFim)
                        return RetornoErro(2);
                    memcpy(mens, origem, total);
                    mens[BUF_MENS-1]=0;
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
                if (VarAtual->tamanho==0 || VarAtual->defvar!=InstrDouble)
                {
                    ApagarVar(VarAtual);
                    if (!CriarVar(InstrDouble))
                        return RetornoErro(2);
                }
                VarAtual->setDouble(valor);
                break;
            }
        case exo_b_shl:  // Operador: a << b
            {
                if (VarFuncIni(VarAtual-1))
                    break;
                FuncAtual->expr++;
                int valor = VarAtual[-1].getInt() << VarAtual[0].getInt();
                ApagarVar(VarAtual);
                if (VarAtual->tamanho==0 || VarAtual->defvar!=InstrVarInt)
                {
                    ApagarVar(VarAtual);
                    if (!CriarVarInt(valor))
                        return RetornoErro(2);
                }
                else
                    VarAtual->setInt(valor);
                break;
            }
        case exo_b_shr:  // Operador: a >> b
            {
                if (VarFuncIni(VarAtual-1))
                    break;
                FuncAtual->expr++;
                int valor = VarAtual[-1].getInt() >> VarAtual[0].getInt();
                ApagarVar(VarAtual);
                if (VarAtual->tamanho==0 || VarAtual->defvar!=InstrVarInt)
                {
                    ApagarVar(VarAtual);
                    if (!CriarVarInt(valor))
                        return RetornoErro(2);
                }
                else
                    VarAtual->setInt(valor);
                break;
            }
        case exo_b_e:   // Operador: a&b
            {
                if (VarFuncIni(VarAtual-1))
                    break;
                FuncAtual->expr++;
                int valor = VarAtual[-1].getInt() & VarAtual[0].getInt();
                ApagarVar(VarAtual);
                if (VarAtual->tamanho==0 || VarAtual->defvar!=InstrVarInt)
                {
                    ApagarVar(VarAtual);
                    if (!CriarVarInt(valor))
                        return RetornoErro(2);
                }
                else
                    VarAtual->setInt(valor);
                break;
            }
        case exo_b_ouou: // Operador: a^b
            {
                if (VarFuncIni(VarAtual-1))
                    break;
                FuncAtual->expr++;
                int valor = VarAtual[-1].getInt() ^ VarAtual[0].getInt();
                ApagarVar(VarAtual);
                if (VarAtual->tamanho==0 || VarAtual->defvar!=InstrVarInt)
                {
                    ApagarVar(VarAtual);
                    if (!CriarVarInt(valor))
                        return RetornoErro(2);
                }
                else
                    VarAtual->setInt(valor);
                break;
            }
        case exo_b_ou:  // Operador: a|b
            {
                if (VarFuncIni(VarAtual-1))
                    break;
                FuncAtual->expr++;
                int valor = VarAtual[-1].getInt() | VarAtual[0].getInt();
                ApagarVar(VarAtual);
                if (VarAtual->tamanho==0 || VarAtual->defvar!=InstrVarInt)
                {
                    ApagarVar(VarAtual);
                    if (!CriarVarInt(valor))
                        return RetornoErro(2);
                }
                else
                    VarAtual->setInt(valor);
                break;
            }
        case exo_atrib:      // Operador de atribuição: a=b
            if (VarAtual[-1].defvar[2] == cVarFunc ||
                VarAtual[-1].defvar[2] == cConstVar)
            {
                if (FuncAtual+1 >= FuncFim)
                    return RetornoErro(2);
                FuncAtual->expr++;
                FuncAtual++;
                const char * defvar = VarAtual[-1].defvar;
                FuncAtual->nome = defvar;
                if (defvar[2] == cVarFunc)
                {
                    FuncAtual->linha = defvar + Num16(defvar);
                    FuncAtual->expr = 0;
                }
                else
                {
                    FuncAtual->linha = defvar;
                    FuncAtual->expr = defvar + defvar[endIndice];
                }
                FuncAtual->este = (TObjeto*)VarAtual[-1].endvar;
                FuncAtual->inivar = VarAtual;
                FuncAtual->fimvar = VarAtual + 1;
                FuncAtual->numarg = 1;
                FuncAtual->tipo = 2;
                FuncAtual->indent = 0;
                FuncAtual->objdebug = FuncAtual[-1].objdebug;
                FuncAtual->funcdebug = FuncAtual[-1].funcdebug;
                break;
            }
            if (VarFuncIni(VarAtual))
                break;
            FuncAtual->expr++;
            switch (VarAtual[-1].Tipo())
            {
            case varOutros:
                // Chama função Igual() se forem variáveis diferentes,
                // mas do mesmo tipo
                if (VarAtual[-1].defvar[2] == VarAtual[0].defvar[2] &&
                        (VarAtual[-1].endvar != VarAtual[0].endvar ||
                            VarAtual[-1].indice != VarAtual[0].indice) )
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
        case exo_menor:      // Operador: a<b
        case exo_menorigual: // Operador: a<=b
        case exo_maior:      // Operador: a>b
        case exo_maiorigual: // Operador: a>=b
        case exo_igual:      // Operador: a==b
        case exo_diferente:  // Operador: a!=b
            {
                if (VarFuncIni(VarAtual-1))
                    break;
            // Compara valores
                int tipo1 = VarAtual[-1].Tipo();
                int tipo2 = VarAtual[0].Tipo();
                    // Números
                switch (tipo1)
                {
                case varInt:
                    if (tipo2==varInt)
                    {
                        int v1 = VarAtual[-1].getInt();
                        int v2 = VarAtual[0].getInt();
                        tipo1 = (v1==v2 ? 0 : v1<v2 ? -1 : 1);
                        break;
                    }
                case varDouble:
                    {
                        double v1 = VarAtual[-1].getDouble();
                        double v2 = VarAtual[0].getDouble();
                        tipo1 = (v1==v2 ? 0 : v1<v2 ? -1 : 1);
                        break;
                    }
                case varTxt:
                    tipo1 = comparaZ(VarAtual[-1].getTxt(),
                                    VarAtual->getTxt());
                    break;
                case varObj:
                    if (tipo2 == varObj)
                    {
                        TObjeto * v1 = VarAtual[-1].getObj();
                        TObjeto * v2 = VarAtual[0].getObj();
                        tipo1 = (v1==v2 ? 0 : v1<v2 ? -1 : 1);
                        break;
                    }
                default:
                        // Mesmo tipo de variável
                    if (VarAtual[-1].defvar[2] == VarAtual[0].defvar[2])
                    {
                        tipo1 = VarAtual[-1].Compara(VarAtual);
                        break;
                    }
                        // Tipos diferentes de variáveis
                    tipo1 = (*FuncAtual->expr == exo_diferente);
                    FuncAtual->expr++;
                    ApagarVar(VarAtual-1);
                    if (!CriarVarInt(tipo1))
                        return RetornoErro(2);
                    goto exo_compara_sair;
                    //const void * v1 = VarAtual[-1].endvar;
                    //const void * v2 = VarAtual[0].endvar;
                    //tipo1 = (v1==v2 ? 0 : v1<v2 ? -1 : 1);
                    //break;
                }
            // Avança Func->expr, verifica operador
                switch (*FuncAtual->expr++)
                {
                case exo_menor:      tipo1 = (tipo1<0);  break;
                case exo_menorigual: tipo1 = (tipo1<=0); break;
                case exo_maior:      tipo1 = (tipo1>0);  break;
                case exo_maiorigual: tipo1 = (tipo1>=0); break;
                case exo_igual:      tipo1 = (tipo1==0); break;
                case exo_diferente:  tipo1 = (tipo1!=0); break;
                }
            // Apaga valores da pilha; cria int32 na pilha
                ApagarVar(VarAtual-1);
                if (!CriarVarInt(tipo1))
                    return RetornoErro(2);
exo_compara_sair:
                break;
            }
        case exo_igual2:     // Operador: a===b
        case exo_diferente2:  // Operador: a!==b
            {
                if (VarFuncIni(VarAtual-1))
                    break;
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
                if (*FuncAtual->expr++ == exo_diferente2)
                    tipo1 = !tipo1;
            // Apaga valores da pilha; cria int32 na pilha
                ApagarVar(VarAtual-1);
                if (!CriarVarInt(tipo1!=0))
                    return RetornoErro(2);
                break;
            }
        case exo_ee:         // Operador: Início do operador &&
            if (VarFuncIni(VarAtual))
                break;
            FuncAtual->expr++;
            if (VarAtual->getBool()==false)
            {
                const char * p = ProcuraExpr(FuncAtual->expr, exo_e);
                assert(p!=0);
                FuncAtual->expr = p + 1;
                ApagarVar(VarAtual);
                if (!CriarVarInt(0))
                    return RetornoErro(2);
                break;
            }
            ApagarVar(VarAtual);
            break;
        case exo_ouou:       // Operador: Início do operador ||
            if (VarFuncIni(VarAtual))
                break;
            FuncAtual->expr++;
            if (VarAtual->getBool())
            {
                const char * p = ProcuraExpr(FuncAtual->expr, exo_ou);
                assert(p!=0);
                FuncAtual->expr = p + 1;
                ApagarVar(VarAtual);
                if (!CriarVarInt(1))
                    return RetornoErro(2);
                break;
            }
            ApagarVar(VarAtual);
            break;
        case exo_e:          // Operador: a&&b
        case exo_ou:         // Operador: a||b
            {
                if (VarFuncIni(VarAtual))
                    break;
                FuncAtual->expr++;
                bool b = VarAtual->getBool();
                ApagarVar(VarAtual);
                if (!CriarVarInt(b))
                    return RetornoErro(2);
                break;
            }
        case exo_i_add:      // Operador: a += b
            if (VarAtual[-1].Tipo() == varTxt)
            {
                FuncAtual->expr += 3;
                VarAtual[-1].addTxt(VarAtual[0].getTxt());
                ApagarVar(VarAtual);
                break;
            }
        case exo_i_mul:      // Operador: a *= b
        case exo_i_div:      // Operador: a /= b
        case exo_i_porcent:  // Operador: a %= b
        case exo_i_sub:      // Operador: a -= b
        case exo_i_b_shl:    // Operador: a <<= b
        case exo_i_b_shr:    // Operador: a >>= b
        case exo_i_b_e:      // Operador: a &= b
        case exo_i_b_ouou:   // Operador: a ^= b
        case exo_i_b_ou:     // Operador: a |= b
            FuncAtual->expr++;
            if (VarAtual >= VarFim-1)
                return RetornoErro(2);
            VarAtual[1] = VarAtual[0];
            VarAtual[0] = VarAtual[-1];
            VarAtual[0].tamanho = 0;
            VarAtual++;
            break;
        case exo_int1:          // Operador: Início do operador "?"
            if (VarFuncIni(VarAtual))
                break;
            FuncAtual->expr++;
            if (VarAtual->getBool()==false)
            {
                const char * p = ProcuraExpr(FuncAtual->expr, exo_int2);
                assert(p!=0);
                FuncAtual->expr = p + 1;
                ApagarVar(VarAtual);
                if (!CriarVar(InstrNulo))
                    return RetornoErro(2);
                break;
            }
            ApagarVar(VarAtual);
            break;
        case exo_int2:          // Operador: Fim do operador "?"
            FuncAtual->expr++;
            while (true)
            {
                const char * p = FuncAtual->expr;
                if (*p == exo_int1)
                    p = ProcuraExpr(p + 1, exo_int2);
                else if (*p == exo_dponto1)
                    p = ProcuraExpr(p + 1, exo_dponto2);
                else
                    break;
                assert(p!=0);
                FuncAtual->expr = p + 1;
            }
            break;
        case exo_dponto1:       // Operador: Início do operador ":"
            FuncAtual->expr++;
            ApagarVar(VarAtual);
            break;
        case exo_dponto2:       // Operador: Fim do operador ":"
            FuncAtual->expr++;
            break;

    // A partir daqui processa funções e variáveis
    // Cria-se uma variável cVarNome, que contém parte do nome da variável
    // Cria-se uma variável em seguida, que contém o resultado anterior
    // Isso torna possível processar várias variáveis ligadas por ponto
    // Exemplo: a.b.c
        case ex_varini:     // Início do texto
        case ex_varabre:    // Início do texto + abre colchetes
            if (!CriarVar(InstrVarNome))
                return RetornoErro(2);
            if (!CriarVar(InstrVarInicio))
                return RetornoErro(2);
            if (*FuncAtual->expr++ == ex_varini)
                FuncAtual->expr = CopiaVarNome(VarAtual-1, FuncAtual->expr);
            break;
        case ex_varfunc:
            if (!CriarVar(InstrVarNome))
                return RetornoErro(2);
            if (!CriarVar(InstrVarIniFunc))
                return RetornoErro(2);
            VarAtual->valor_int = (unsigned char)FuncAtual->expr[1];
            FuncAtual->expr += 2;
            break;
        case ex_varlocal:
            {
                if (!CriarVar(InstrVarNome))
                    return RetornoErro(2);
                int valor = (unsigned char)FuncAtual->expr[1];
            // Variável do objeto
                if (valor == 0xFF)
                {
                    if (!CriarVar(InstrVarIniFunc))
                        return RetornoErro(2);
                    VarAtual->valor_int = -1;
                    FuncAtual->expr = CopiaVarNome(
                        VarAtual-1, FuncAtual->expr + 2);
                    break;
                }
            // Variável da função
                if (VarAtual >= VarFim-1)
                    return RetornoErro(2);
                // 2 bytes para ex_varlocal + X bytes para o nome da variável
                const char * p = FuncAtual->expr + 2;
                while (*(const unsigned char*)p >= ' ')
                    p++;
                // Avança para o próximo ex_ponto
                p = ProcuraExpr(p, ex_ponto);
                assert(p!=0);
                // Copia o nome depois do ex_ponto
                FuncAtual->expr = CopiaVarNome(VarAtual, p + 1);
                // Coloca a variável local na pilha
                TVariavel * var = FuncAtual->inivar +
                                  FuncAtual->numarg + valor;
                VarAtual++;
                *VarAtual = *var;
                VarAtual->tamanho = 0;
                break;
            }
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
                        return RetornoErro(2);
                    break;
                }
            // Outro valor
                ApagarVar(v+2);
                DadosTopo = (char*)v->endvar;
                if (VarAtual->tamanho)
                {
                    VarAtual->MoverEnd(v->endvar, 0, 0);
                    DadosTopo += VarAtual->tamanho;
                }
                *v = *VarAtual;
                VarAtual = v;
                break;
            }
        case ex_doispontos:
            {
                FuncAtual->expr++;
                TVariavel * v = EndVarNome();
                assert(v!=0);
                if (v[1].defvar[2] == cVarInicio)
                {
                    TClasse * c = TClasse::Procura(v->getTxt());
                    if (c)
                    {
                        v[1].defvar = InstrVarClasse;
                        v[1].nomevar = 0;
                        v[1].endvar = c;
                        v->setTxt("");
                        FuncAtual->expr = CopiaVarNome(
                                v, FuncAtual->expr);
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
                unsigned char defvar_cod = v[1].defvar[2];
                if (defvar_cod == cVarClasse)
                {
                    classe = (TClasse*)v[1].endvar;
                    objeto = FuncAtual->este;
                }
            // Verifica variável/função da classe
                else if (defvar_cod == cVarInicio)
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
                        VarAtual->Limpar();
                        VarAtual->defvar = InstrVarObjeto;
                        VarAtual->endvar = c->ObjetoIni;
                        v->setTxt("");
                        FuncAtual->expr = CopiaVarNome(v, FuncAtual->expr);
                        break;
                    }
                // Verifica se é variável/comando interno do programa
                    int NumFunc = InfoFunc(nome);
                    if (NumFunc >= 0)
                    {
                        const TListaFunc * func = &ListaFunc[NumFunc];
                            // Ler varfunc primeiro
                        ExecFunc * f = FuncAtual;
                        if (VarFuncIni(v+1))
                        {
                            f->expr--;
                            break;
                        }
                            // Processa função interna
                        if (func->Func(v+1, func->valor))
                        {
                            v->setTxt("");
                            f->expr = CopiaVarNome(v, f->expr);
                        }
                        else
                            VarInvalido();
                        break;
                    }
                // Verifica se é variável local da função
                    TVariavel * var = FuncAtual->inivar + FuncAtual->numarg;
                    for (; var < FuncAtual->fimvar; var++)
                      if (var->nomevar)
                        if (comparaVar(nome, var->nomevar + Instr::endNome)==0)
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
            // Verifica comando interno do programa (função predefinida)
                else if (defvar_cod == cVarIniFunc)
                {
                    int valor = v[1].valor_int;
                    if (valor >= 0) // Se é função predefinida...
                    {
                        const TListaFunc * func = &ListaFunc[ valor ];
                            // Ler varfunc primeiro
                        ExecFunc * f = FuncAtual;
                        if (VarFuncIni(v+1))
                        {
                            f->expr--;
                            break;
                        }
                            // Processa função interna
                        if (func->Func(v+1, func->valor))
                        {
                            v->setTxt("");
                            f->expr = CopiaVarNome(v, f->expr);
                        }
                        else
                            VarInvalido();
                        break;
                    }
                    // Variável/função do objeto
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
                        return RetornoErro(1);
                    FuncAtual++;
                    FuncAtual->nome = defvar;
                    FuncAtual->linha = defvar + Num16(defvar);
                    FuncAtual->este = objeto;
                    FuncAtual->expr = 0;
                    FuncAtual->inivar = v+2;
                    FuncAtual->fimvar = VarAtual + 1;
                    FuncAtual->numarg = FuncAtual->fimvar - FuncAtual->inivar;
                    FuncAtual->tipo = 0;
                    FuncAtual->indent = 0;
                    FuncAtual->objdebug = FuncAtual[-1].objdebug;
                    FuncAtual->funcdebug = FuncAtual[-1].funcdebug;
                    break;
                }
            // Função varfunc
                if (defvar[2]==cVarFunc || defvar[2]==cConstVar)
                {
                    ApagarVar(v+1);
                  //  if (objeto==0)
                  //  {
                  //      if (!CriarVar(InstrNulo))
                  //          return RetornoErro(2);
                  //      break;
                  //  }
                    VarAtual++;
                    VarAtual->Limpar();
                    VarAtual->defvar = defvar;
                    VarAtual->nomevar = defvar;
                    VarAtual->endvar = objeto;
                    break;
                }
            // Expressão numérica
                if (defvar[2]==cConstExpr)
                {
                    if (FuncAtual >= FuncFim - 1)
                        return RetornoErro(1);
                    FuncAtual++;
                    FuncAtual->nome = defvar;
                    FuncAtual->linha = defvar;
                    FuncAtual->este = objeto;
                    FuncAtual->expr = defvar + defvar[endIndice];
                    FuncAtual->inivar = v + 2;
                    FuncAtual->fimvar = VarAtual + 1;
                    FuncAtual->numarg = FuncAtual->fimvar - FuncAtual->inivar;
                    FuncAtual->tipo = 0;
                    FuncAtual->indent = 0;
                    FuncAtual->objdebug = FuncAtual[-1].objdebug;
                    FuncAtual->funcdebug = FuncAtual[-1].funcdebug;
                    break;
                }
            // Variável
                ApagarVar(v+1);
                VarAtual++;
                VarAtual->Limpar();
                VarAtual->defvar = defvar;
                VarAtual->nomevar = defvar;
                VarAtual->numbit = indvar >> 24;
                VarAtual->numfunc = 0;
                VarAtual->indice = (defvar[endVetor]==0 ? 0 : 0xFF);
                if (defvar[2]==cConstTxt || // Constante
                        defvar[2]==cConstNum)
                    VarAtual->endvar = 0;
                else if (indvar & 0x400000) // Variável da classe
                    VarAtual->endvar = classe->Vars +
                            (indvar & 0x3FFFFF);
                else if (objeto &&  // Variável do objeto
                            objeto->Classe == classe)
                        // Nota: se executar algo como "x:y = nulo",
                        // a classe vai ser "x", mas o objeto
                        // pode ser de outra classe
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
    return RetornoErro(3);
}

//----------------------------------------------------------------------------
/// Executa procedimentos após ExecX()
void Instr::ExecFim()
{
    static bool dentro = false;
    if (dentro)
    {
        TVarProg::LimparVar(); // Apaga referências das variáveis prog
        ApagarVar(VarPilha); // Apaga variáveis pendentes
        return;
    }
    dentro = true;
    while (true)
    {
        TVarProg::LimparVar(); // Apaga referências das variáveis prog
        ApagarVar(VarPilha); // Apaga variáveis pendentes

    // Apaga objetos marcados para exclusão
        while (true)
        {
            TObjeto * obj = TObjeto::IniApagar;
            if (obj==0)
                break;
            if (Instr::ExecIni(obj, "fim"))
            {
                Instr::ExecX();
                TVarProg::LimparVar(); // Apaga referências das variáveis prog
                ApagarVar(VarPilha); // Apaga variáveis pendentes
            }
            obj->Apagar();
        }

    // Altera o programa
        if (TMudarClasse::ExecPasso())
            continue;

    // Avisa sobre erros
        if (erromens)
        {
            TVarDebug::FuncEvento("erro", erromens);
            delete[] erromens;
            erromens = 0;
            continue;
        }

        break;
    }
    dentro = false;
}
