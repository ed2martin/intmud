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

void Termina(); // Encerra o programa

#define DEBUG_INSTR // Mostra instru��es que ser�o executadas
#define DEBUG_EXPR  // Mostra valores de Instr::Expressao encontrados
#define DEBUG_VAR   // Mostra vari�veis no topo da pilha

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

- cVarNome    -> Para obter nome da vari�vel
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
FuncAtual->fimvar = endere�o da primeira vari�vel ap�s VarAtual

(in�cio)
Se FuncAtual->expr == 0:
  *** Est� processando uma linha:
  Se for fim da fun��o ou instru��o ret sem par�metros:
     Apaga vari�veis criadas na fun��o (a partir de FuncAtual->inivar)
     Coloca NULO na pilha de vari�veis
     Se n�o houver fun��o anterior:
       Fim do algoritmo
     Apaga fun��o atual (FuncAtual--)
     Vai para (in�cio)
  Se for vari�vel:
     Cria vari�vel local da fun��o atual
     FuncAtual->fimvar++
  Se linha possui express�o num�rica:
     FuncAtual->expr = in�cio da express�o num�rica
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
     Apaga vari�veis de FuncAtual->fimvar em diante
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
  *** Cria-se uma vari�vel cVarNome, que cont�m parte do nome da vari�vel
  *** Cria-se uma vari�vel em seguida, que cont�m o resultado anterior
  *** Isso torna poss�vel processar v�rias vari�veis ligadas por ponto
  *** Exemplo: a.b.c
  Se for ex_varini ou ex_varabre:
     Cria vari�vel cVarNome na pilha de vari�veis para armazenar o nome
     Cria vari�vel cVarInicio na pilha de vari�veis
     Avan�a FuncAtual->Expr
     Se era ex_varini:
       Copia o que puder do nome em (um texto) para �ltima vari�vel cVarNome
     Vai para (in�cio)
  Se for ex_arg (argumento da fun��o) ou ex_abre (abre colchetes):
     Avan�a FuncAtual->expr
     Vai para (in�cio)
  Se for ex_fecha (fecha colchetes):
     Avan�a FuncAtual->expr
     Sendo que: X=�ltima vari�vel cVarNome da pilha e Y=vari�vel ap�s X
     Transforma em texto as vari�veis ap�s Y e anota em X
     Apaga vari�veis ap�s Y
     Copia o que puder do nome em FuncAtual->expr (um texto) para X
     Vai para (in�cio)
  Se for ex_varfim:
     Avan�a FuncAtual->expr
     Sendo que: X=�ltima vari�vel cVarNome da pilha e Y=vari�vel ap�s X
     Se vari�vel Y for cVarInicio ou cVarClasse:
       Apaga vari�veis de X em diante
       Cria vari�vel cNulo
     Caso contr�rio:
       Apaga vari�veis ap�s Y
       Apaga vari�vel X
       Vari�vel Y passa a ocupar o lugar da vari�vel X
     Vai para (in�cio)
  Se for ex_doispontos:
     Procura �ltima vari�vel cVarNome da pilha
     Se nome de classe em cVarNome existe e pr�xima vari�vel for cVarInicio:
       Muda a pr�xima vari�vel para cVarClasse
       Anota o endere�o da classe
     Caso contr�rio:
       Apaga da vari�vel cVarNome em diante
       Cria vari�vel cNulo na pilha de vari�veis
       Avan�a FuncAtual->expr at� ex_varfim
     Avan�a FuncAtual->expr
     Vai para (in�cio)
  Se for ex_ponto:
     Avan�a FuncAtual->expr
     Procura �ltima vari�vel cVarNome da pilha
     Checa pr�xima vari�vel e texto em cVarNome:
        cVarInicio -> 1. Verifica se � vari�vel/comando interno do programa
                      2. Verifica se � vari�vel local da fun��o
                      3. Verifica se � vari�vel ou fun��o do objeto
        cVarClasse -> Verifica se � fun��o da classe especificada
        cVarObjeto -> Verifica se � vari�vel ou fun��o do objeto em cVarObjeto
     Depois de decidido que a��o tomar:
       Limpa texto em cVarNome
       Copia o que puder do nome em FuncAtual->expr (um texto) para cVarNome
     Executa a��o, que pode ser:
     1. Processar fun��o interna do programa:
        Apaga vari�veis ap�s cVarNome
        Cria vari�vel com o resultado
     2. Executar uma fun��o de uma classe:
        FuncAtual++
        FuncAtual->linha = primeira linha ap�s o nome da fun��o
        FuncAtual->este = endere�o do objeto "este", ou NULL
        FuncAtual->expr = 0
        FuncAtual->inivar = primeira vari�vel ap�s cVarNome
        FuncAtual->fimvar = endere�o da primeira vari�vel ap�s VarAtual
        FuncAtual->numarg = n�mero de vari�veis ap�s cVarNome
     3. Vari�vel definida em uma classe:
        Apaga vari�veis a partir da anterior a cVarNome
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
static const char InstrInt[] = { 7, 0, Instr::cInt32, 0, 0, '+', 0 };
static const char InstrUInt[] = { 7, 0, Instr::cUInt32, 0, 0, '+', 0 };
static const char InstrTxtFixo[] = { 7, 0, Instr::cTxtFixo, 0, 0, '+', 0 };
static const char InstrVarNome[] = { 7, 0, Instr::cVarNome, 0, 0, '+', 0 };
static const char InstrVarInicio[] = { 7, 0, Instr::cVarInicio, 0, 0, '+', 0 };
static const char InstrVarClasse[] = { 7, 0, Instr::cVarClasse, 0, 0, '+', 0 };
static const char InstrVarObjeto[] = { 7, 0, Instr::cVarObjeto, 0, 0, '+', 0 };

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
    FuncAtual->inivar = VarAtual + 1; // Endere�o do primeiro argumento
    FuncAtual->fimvar = VarAtual + 1;
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
    FuncAtual->fimvar = VarAtual + 1;
}

//----------------------------------------------------------------------------
/// Usado internamente em Instr::ExecX()
/** Executa procedimentos em caso de mem�ria insuficiente
 *  @return false
 */
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
/// Usado internamente em Instr::ExecX()
/** Adiciona texto em vari�vel cVarNome
 */
static char * CopiaVarNome(TVariavel * v, char * txt)
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
/** @return �ltima vari�vel cVarNome da pilha
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
/** Avan�a para fim do nome da vari�vel
 */
static void VarInvalido(void)
{
// Obt�m �ltima vari�vel cVarNome da pilha
    TVariavel * v = EndVarNome();
    assert(v!=0);
// Apaga �ltima vari�vel cVarNome em diante
    Instr::ApagarVar(v);
// Cria NULO na pilha
    Instr::CriarVar(InstrNulo);
// Avan�a para o fim do nome da vari�vel
    char * p = ProcuraExpr(Instr::FuncAtual->expr, Instr::ex_varfim);
    assert(p!=0);
    Instr::FuncAtual->expr = p + 1;
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
                ApagarVar(FuncAtual->inivar - 1);
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
                const char * x = FuncAtual->linha;
                if (x[2]==cRef)
                    x=InstrVarObjeto;
                if (!CriarVar(x))
                    return RetornoErro();
                FuncAtual->fimvar++;
                FuncAtual->linha += Num16(FuncAtual->linha);
                continue;
            }

        // Processa instru��o
#ifdef DEBUG_INSTR
            {
                char mens[512];
                if (Instr::Decod(mens, FuncAtual->linha, sizeof(mens)))
                    printf("Exec: %s\n", mens);
                else
                    printf("Erro ao ler instru��o\n");
                fflush(stdout);
            }
#endif
            switch (FuncAtual->linha[2])
            {
            case cSe:   // Processa express�o num�rica
            case cEnquanto:
                FuncAtual->expr = FuncAtual->linha + 5;
                FuncAtual->igualcompara = true;
                break;
            case cRet2: // Processa express�o num�rica
                FuncAtual->expr = FuncAtual->linha + 3;
                FuncAtual->igualcompara = true;
                break;
            case cExpr:
                FuncAtual->expr = FuncAtual->linha + 3;
                FuncAtual->igualcompara = false;
                break;
            case cComent: // Coment�rio
                FuncAtual->linha += Num16(FuncAtual->linha);
                break;
            case cSenao1: // Avan�a
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
            case cFimSe: // Passa para pr�xima instru��o
                FuncAtual->linha += Num16(FuncAtual->linha);
                break;
            case cTerminar: // Encerra o programa
                Termina();
            default:  // Instru��o desconhecida
                assert(0);
            }
            continue;
        }

    // Est� processando express�o num�rica
    // FuncAtual->expr[0] cont�m a pr�xima instru��o da express�o num�rica
#ifdef DEBUG_VAR
        printf("       %p\n", DadosTopo);
        for (int x=0; x<5; x++)
        {
            TVariavel * v = VarAtual - x;
            if (v<VarPilha)
                continue;
            printf("    v%d %p l%d ", v-VarPilha, v->endvar, v->tamanho);
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
                    printf("Erro ao ler instru��o\n");
                fflush(stdout);
            }
#endif
            switch (FuncAtual->linha[2])
            {
            case cRet2:
            case cConstExpr:
                assert(VarAtual >= FuncAtual->inivar);
                ApagarRet(FuncAtual->inivar-1);
                if (FuncAtual==FuncPilha)
                    return true;
                FuncAtual--;
                break;
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
                if (VarAtual->tamanho==0 || VarAtual->defvar[2] != cReal)
                {
                    ApagarVar(VarAtual);
                    CriarVar(InstrDouble);
                }
                VarAtual->setDouble(valor);
                break;
            }
        case exo_ini:        // Operador: Marca o in�cio dos operadores
        case exo_fim:        // Operador: Marca o fim dos operadores
        case exo_virgula:    // Operador: V�rgula, para separar express�es
            FuncAtual->expr++;
            break;
        case exo_neg:        // Operador: -a
            FuncAtual->expr++;
            switch (VarAtual->Tipo())
            {
            case varNulo:
                break;
            case varInt:
                if (VarAtual->tamanho)
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
            FuncAtual->expr++;
            if (VarAtual->Tipo() != varInt || VarAtual->tamanho==0)
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
            // Caso 1: N�o � texto: executa soma num�rica
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
            // de concatena��o: ("abc" + ("x")) + ("def" + 1) + 4

            // Caso 2: Duas vari�veis n�o locais
            // Transforma em: Primeira vari�vel � cTxtFixo local
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
            // Caso 3: Primeira vari�vel n�o � cTxtFixo local
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
                // Apaga vari�veis locais
                    ApagarVar(VarAtual-1);
                    int total = destino - mens;
                    if (DadosTopo + total > DadosFim)
                        return RetornoErro();
                // Cria vari�vel que conter� o texto
                    VarAtual++;
                    VarAtual->defvar = InstrTxtFixo;
                    VarAtual->endvar = DadosTopo;
                    VarAtual->tamanho = total;
                // Copia texto para vari�vel
                    memcpy(DadosTopo, mens, total);
                    DadosTopo += total;
                    break;
                }
            // Primeira vari�vel � cTxtFixo local
                char * destino = (char*)VarAtual[-1].endvar +
                        VarAtual[-1].tamanho - 1;
                int total = 0;
            // Caso 4: Segunda vari�vel n�o � local
                if (VarAtual->tamanho==0)
                {
                    const char * origem = VarAtual->getTxt();
                    total = 1 + strlen(origem);
                    if (destino + total >= DadosFim)
                        return RetornoErro();
                    memcpy(destino, origem, total);
                    VarAtual--;
                }
            // Caso 5: Segunda vari�vel � cTxtFixo local
                else if (VarAtual->defvar[2]==cTxtFixo)
                {
                    total = VarAtual->tamanho;
                    VarAtual->Mover(destino, 0, 0);
                    VarAtual--;
                }
            // Caso 6: Segunda vari�vel � local
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
            // Avan�a Func->expr, verifica operador
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
        case exo_ee:         // Operador: In�cio do operador &
            FuncAtual->expr++;
            if (VarAtual->getBool()==false)
            {
                char * p = ProcuraExpr(FuncAtual->expr, exo_e);
                assert(p!=0);
                FuncAtual->expr = p + 1;
                ApagarVar(VarAtual);
                if (!CriarVar(InstrInt))
                    return RetornoErro();
                break;
            }
            ApagarVar(VarAtual);
            break;
        case exo_ouou:       // Operador: In�cio do operador |
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

    // A partir daqui processa fun��es e vari�veis
    // Cria-se uma vari�vel cVarNome, que cont�m parte do nome da vari�vel
    // Cria-se uma vari�vel em seguida, que cont�m o resultado anterior
    // Isso torna poss�vel processar v�rias vari�veis ligadas por ponto
    // Exemplo: a.b.c
        case ex_varini:     // In�cio do texto
        case ex_varabre:    // In�cio do texto + abre colchetes
            if (!CriarVar(InstrVarNome))
                return RetornoErro();
            if (!CriarVar(InstrVarInicio))
                return RetornoErro();
            if (*FuncAtual->expr++ == ex_varini)
                FuncAtual->expr = CopiaVarNome(VarAtual-1, FuncAtual->expr);
            break;
        case ex_arg:        // In�cio da lista de argumentos
        case ex_abre:       // Abre colchetes
            FuncAtual->expr++;
            break;
        case ex_fecha:      // Fecha colchetes
            {
                FuncAtual->expr++;
                TVariavel * v = EndVarNome();
                char mens[64];
                char * p = mens;
                char * pfim = p + sizeof(mens);
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
                TClasse * classe = 0; // Procurar vari�vel de classe/objeto
                TObjeto * objeto = 0; // Procurar vari�vel de objeto
                assert(v!=0);

            // Verifica vari�vel/fun��o da classe
                if (v[1].defvar[2]==cVarClasse)
                    classe = (TClasse*)v[1].endvar;
            // Verifica vari�vel/fun��o da classe
                else if (v[1].defvar[2]==cVarInicio)
                {
                // Verifica se � vari�vel/comando interno do programa


    //*****************************


                // Se come�a com $, verifica se objeto existe
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
                        break;
                    }
                // Verifica se � argumento da fun��o
                    if ((nome[0]|0x20)=='a' && (nome[1]|0x20)=='r' &&
                            (nome[2]|0x20)=='g' && nome[3]>='0' &&
                            nome[3]<='9' && nome[4]==0)
                    {
                        int arg = nome[3] - '0';
                        if (arg >= FuncAtual->numarg)
                            VarInvalido();
                        else
                        {
                            ApagarVar(v+1);
                            VarAtual++;
                            *VarAtual = *(FuncAtual->inivar + arg);
                            VarAtual->tamanho = 0;
                        }
                        break;
                    }
                // Verifica se � vari�vel local da fun��o
                    TVariavel * var = FuncAtual->inivar + FuncAtual->numarg;
                    for (; var < FuncAtual->fimvar; var++)
                        if (comparaZ(nome, var->defvar+5)==0)
                            break;
                    if (var < FuncAtual->fimvar)
                    {
                        ApagarVar(v+1);
                        VarAtual++;
                        *VarAtual = *var;
                        VarAtual->tamanho = 0;
                        break;
                    }
                // Verifica se � vari�vel/fun��o do objeto
                    objeto = FuncAtual->este;
                    if (objeto)
                        classe = objeto->Classe;
                }
            // Verifica vari�vel/fun��o de objeto
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


    // else ...
    //******* Consulta a vari�vel sobre o que fazer


            // Processa classe e objeto
                if (classe==0)
                {
                    VarInvalido();
                    break;
                }
                int indice = classe->IndiceNome(nome);
                if (indice<0) // Vari�vel/fun��o inexistente
                {
                    VarInvalido();
                    break;
                }
                char * defvar = classe->InstrVar[indice];
                int indvar = classe->IndiceVar[indice];
                v->setTxt("");
            // Fun��o
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
                    break;
                }
            // Fun��o varfunc
                if (defvar[2]==cVarFunc)
                {
                    if (objeto==0)
                    {
                        VarInvalido();
                        break;
                    }
                    VarAtual++;
                    VarAtual->defvar = defvar;
                    VarAtual->endvar = objeto;
                    VarAtual->tamanho = 0;
                    break;
                }
            // Express�o num�rica
                if (defvar[2]==cConstExpr)
                {
                    if (FuncAtual >= FuncFim - 1)
                        return RetornoErro();
                    FuncAtual++;
                    FuncAtual->linha = defvar;
                    FuncAtual->este = objeto;
                    FuncAtual->expr = defvar + defvar[4];
                    FuncAtual->inivar = v + 2;
                    FuncAtual->fimvar = VarAtual + 1;
                    FuncAtual->numarg = FuncAtual->fimvar - FuncAtual->inivar;
                    break;
                }
            // Vari�vel
                ApagarVar(v+1);
                VarAtual++;
                VarAtual->defvar = defvar;
                VarAtual->tamanho = 0;
                VarAtual->bit = indvar >> 24;
                if (defvar[2]==cConstTxt || // Constante
                        defvar[2]==cConstNum)
                    VarAtual->endvar = 0;
                else if (indvar & 0x400000) // Vari�vel da classe
                    VarAtual->endvar = classe->Vars +
                            (indvar & 0x3FFFFF);
                else if (objeto) // Vari�vel do objeto
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
/// Executa procedimentos ap�s ExecX()
void Instr::ExecFim()
{
    ApagarVar(VarAtual+1);


    // Se apagar alguma classe aqui, acertar tamb�m fun��o main()

}
