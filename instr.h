#ifndef INSTR_H
#define INSTR_H

class TClasse;
class TObjeto;
class TVariavel;

/// Codificar, decodificar e executar instruções
/** Codificar, decodificar e executar instruções */
namespace Instr {

//----------------------------------------------------------------------------
// Funções
bool Codif(char * destino, const char * origem, int tamanho);
bool Decod(char * destino, const char * origem, int tamanho);
bool Mostra(char * destino, const char * origem, int tamanho);

bool CriarVarTexto(const char * mens, int tammens=-1);
bool CriarVar(const char * defvar);
void ApagarVar(TVariavel * v);
void ApagarRet(TVariavel * v);
bool VarFuncIni(TVariavel * varini);
bool VarFuncFim();
const char * ProcuraExpr(const char * expr, int valor);
const char * NomeInstr(const char * instr);
const char * NomeComando(int valor);
const char * NomeExpr(int valor);

bool ExecIni(TClasse * classe, const char * func);
bool ExecIni(TObjeto * este, const char * func);
void ExecArg(const char * txt);
void ExecArg(int valor);
void ExecArgCriar(const char * def);
bool ExecX();
void ExecFim();

bool FuncArg(TVariavel * v, int valor);
bool FuncArgs(TVariavel * v, int valor);
bool FuncCriar(TVariavel * v, int valor);
bool FuncApagar(TVariavel * v, int valor);
bool FuncEste(TVariavel * v, int valor);
bool FuncNumero(TVariavel * v, int valor);
bool FuncRef(TVariavel * v, int valor);
bool FuncTxtNum(TVariavel * v, int valor);
bool FuncTxt(TVariavel * v, int valor);
bool FuncTxt2(TVariavel * v, int valor);
bool FuncInt(TVariavel * v, int valor);
bool FuncTxtRemove(TVariavel * v, int valor);
bool FuncTxtProc(TVariavel * v, int valor);
bool FuncTxtTroca(TVariavel * v, int valor);
bool FuncAntesDepois(TVariavel * v, int valor);
bool FuncTotal(TVariavel * v, int valor);
bool FuncVarTroca(TVariavel * v, int valor);

bool ChecaHerda(const char * instr, const char * nomeclasse);
int  Prioridade(int operador);

//----------------------------------------------------------------------------
// Variáveis predefinidas
/// TVariavel::defvar para Instr::cNulo
extern const char InstrNulo[];
/// TVariavel::defvar para Instr::cReal
extern const char InstrDouble[];
/// TVariavel::defvar para Instr::cSocket
extern const char InstrSocket[];
/// TVariavel::defvar para Instr::cTxtFixo
extern const char InstrTxtFixo[];
/// TVariavel::defvar para Instr::cVarNome
extern const char InstrVarNome[];
/// TVariavel::defvar para Instr::cVarInicio
extern const char InstrVarInicio[];
/// TVariavel::defvar para Instr::cVarClasse
extern const char InstrVarClasse[];
/// TVariavel::defvar para Instr::cVarObjeto
extern const char InstrVarObjeto[];
/// TVariavel::defvar para Instr::cVarInt
extern const char InstrVarInt[];
/// Quantas instruções pode executar antes que o controle retorne ao programa
extern int VarExec;
/// Valor inicial de Instr::VarExec quando Instr::ExecIni é executado
extern int VarExecIni;

//----------------------------------------------------------------------------
/** Verifica se instruções de uma classe (codificadas por Instr::Codif)
    estão na ordem correta */
class ChecaLinha /// Verifica ordem das instruções de uma classe
{
public:
    ChecaLinha();
    void Inicio();
            /**< Indica que está no início da lista de instruções */
    const char * Instr(const char * instr);
            /**< Checa próxima instrução da lista
                @param instr Instrução codificada
                @return 0 se instrução está no lugar correto,
                        outro valor=mensagem de erro */
private:
    char esperando;
            /**< O que está esperando:
                - 0=início do arquivo; pode receber "herda"
                - 1=nenhuma função definida
                - 2=instruções e variáveis de uma função
                - 3=instruções de uma função
                - 4=após definição de função + definição de constante */
};

//----------------------------------------------------------------------------
/** Pilha de funções - usado ao executar funções */
class ExecFunc /// Pilha de funções
{
public:
    TObjeto * este;     ///< Objeto ao qual a função pertence
    const char * linha; ///< Instrução codificada sendo executada
                        ///< Mesmo formato de TClasse::Comandos
    const char * expr;  ///< Aonde parou na expressão numérica
                        ///< =0 se não estiver processando expressão numérica
    TVariavel * inivar; ///< Primeiro argumento da função
    TVariavel * fimvar; ///< Primeira variável após variáveis locais da função
    char  numarg;       ///< Número de argumentos arg0 a arg9
    char  tipo;         ///< 0=func 1=ler varfunc 2=mudar varfunc 3=criar()
    bool  igualcompara; ///< Se o sinal de igual compara ou atribui
};

//----------------------------------------------------------------------------
/** Processa funções predefinidas
    @note A lista de funções está definida em instr-exec.cpp
*/
class TListaFunc /// Funções internas
{
public:
        /// Nome da função predefinida
    const char * Nome;

        /// Função que processa a função predefinida
        /** Ao executar a função, a pilha de variáveis está assim:
        - cVarNome
        - cVarInicio  (v aponta para essa variável)
        - Argumentos da função interna (VarAtual aponta para a última)
        .
        Se retornar true, deve:
        - Apagar variáveis de v em diante
        - Criar uma variável com o resultado
        .
        @param v Endereço da variável cVarInicio
        @param valor Valor de TListaFunc::valor
        @retval true Se processou normalmente
        @retval false Se erro; o resultado será o tipo NULO */
    bool (*Func)(TVariavel * v, int valor);

        /// Valor passado à função Func
    int  valor;
};

//----------------------------------------------------------------------------
    /// Pilha de dados (64K)
extern char * const DadosPilha;
extern char * const DadosFim;
    /// Pilha de dados - início da área de dados disponível
extern char * DadosTopo;

    /// Pilha de variáveis
extern TVariavel * const VarPilha;
extern TVariavel * const VarFim;
    /// Pilha de variáveis - última variável da pilha
extern TVariavel * VarAtual;

    /// Pilha de funções
extern ExecFunc * const FuncPilha;
extern ExecFunc * const FuncFim;
    /// Pilha de funções - última função da pilha
extern ExecFunc * FuncAtual;

enum EndVar
{
    endProp=3,
    endIndice=4,
    endVetor=5,
    endNome=6
};

//----------------------------------------------------------------------------
/// Comandos
/**
    Cada linha de comando é codificada assim:
    - bytes 0,1 = tamanho em bytes; 0 significa fim da lista de comandos
    - byte 2 = comando (vide TComando)
    - X bytes = dados do comando (depende do comando)
    .

    A partir de cVariaveis vem as definições de variáveis:
    - byte 3 (Instr::endProp) = propriedades:
      - bit 0=1 se comum
      - bit 1=1 se sav
      .
    - byte 4 (Instr::endIndice)
      - tamanho do texto em cTxt1 e cTxt2
      - índice para os dados extras das variáveis Const
      .
    - byte 5 (Instr::endVetor)
      - número de elementos do vetor ou 0 se não for vetor
      .
    - X bytes (Instr::endNome) = nome da variável em ASCIIZ
    - Y bytes = expressão numérica, em cConstExp
      - texto em ASCIIZ, em cConstTxt
      - valor numérico em cConstInt e cConstReal
      .
    .

    Tipos de comandos:
    - Comum: Instruções comuns (não se encaixam em outra categoria)
    - Fluxo: Instruções de controle de fluxo
    - Var:   Definições de variáveis
    - Extra: Variáveis extras
    .

    Nos comandos que não têm expressões numéricas (exceto Herda),
    se houver algum comentário, ele virá após os dados do comando.

    Exemplo,  "fimse # teste" é codificado assim:
              0x08, 0x00, cFimSe, 't', 'e', 's', 't', 'e'
 */
enum Comando
{
// Instruções comuns
    cHerda,         /**< Comum: Instrução herda
                        - 1 byte = número de classes
                        - X bytes = nomes das classes em ASCIIZ
                        - Só pode estar no início da lista de comandos */
    cExpr,          ///< Comum: Expressão numérica pura
    cComent,        ///< Comum: Comentário

// Instruções de controle de fluxo
    cSe,            ///< Fluxo: ushort,expressão   se(expressão)
    cSenao1,        ///< Fluxo: ushort             senão
    cSenao2,        ///< Fluxo: ushort,expressão   senão(expressão)
    cFimSe,         ///< Fluxo:                    fimse
    cEnquanto,      ///< Fluxo: ushort,expressão   enquanto(expressão)
    cEFim,          ///< Fluxo: ushort             efim
    cRet1,          ///< Fluxo: ret sem argumentos
    cRet2,          ///< Fluxo: ret com expressão numérica
    cSair,          ///< Fluxo: ushort             sair
    cContinuar,     ///< Fluxo: ushort             continuar
    cTerminar,      ///< Fluxo:                    terminar

// Definições de variáveis
    cVariaveis,         ///< Var: Marca o início das variáveis
    cTxt1,              ///< Var: Texto de 1 a 256 caracteres
    cTxt2,              ///< Var: Texto de 257 a 512 caracteres
    cInt1,              ///< Var: 1 bit
    cInt8, cUInt8,      ///< Var: 8 bits com e sem sinal
    cInt16, cUInt16,    ///< Var: 16 bits com e sem sinal
    cInt32, cUInt32,    ///< Var: 32 bits com e sem sinal
    cReal,              ///< Var: real - "double"
    cConstNulo,         ///< Var: Constante = nulo
    cConstTxt,          ///< Var: Constante = texto
    cConstNum,          ///< Var: Constante = número
    cConstExpr,         ///< Var: Constante = expressão numérica
    cFunc,              ///< Var: Função
    cVarFunc,           ///< Var: Função
        /**< @note Para as variáveis após cVarFunc, deve-se
            obrigatoriamente usar TVariavel::Criar e TVariavel::Apagar,
            exceto quando TVariavel::Tamanho retorna 0 */
    cIntInc, cIntDec,   ///< Var: intinc e intdec
    cRef,               ///< Var: Referência para um objeto
                        /**< @sa TVarRef */

// Variáveis extras
    cListaObj,          ///< Extra: ListaObj
    cListaItem,         ///< Extra: ListaItem
    cListaTxt,          ///< Extra: cListaTxt
    cListaMsg,          ///< Extra: ListaMsg
    cNomeObj,           ///< Extra: NomeObj
    cArqLog,            ///< Extra: ArqLog
    cArqTxt,            ///< Extra: ArqTxt
    cIntTempo,          ///< Extra: IntTempo
    cSocket,            ///< Extra: Socket
    cServ,              ///< Extra: Serv
    cSalvar,            ///< Extra: Salvar
    cProg,              ///< Extra: Prog
    cIndiceObj,         ///< Extra: IndiceObj
    cIndiceItem,        ///< Extra: IndiceItem

// Usado internamente
    cTxtFixo,           ///< Aponta para um texto fixo
    cVarNome,           ///< Para obter nome da variável
    cVarInicio,         ///< Esperando texto logo após ex_varini
    cVarClasse,         ///< TVariavel::endvar = endereço do objeto TClasse
    cVarObjeto,         ///< TVariavel::endvar = endereço do objeto TObjeto
    cVarInt,            ///< int local; vide TVariavel::valor_int

    cTotalComandos      ///< Número de comandos - não usado
};

//----------------------------------------------------------------------------
/// Idenfificadores usados em expressões numéricas
/**
    Texto: Usado em textos

    Variável: Nome de variável ou função (um texto)
    - Cada conjunto de variáveis/funções (no estilo x.y.x):
      -    ex_varini + variáveis/funções + ex_varfim
      .
    - Cada variável/função (sem e com argumentos):
      -    nome da variável/função + ex_ponto
      -    nome da variável/função + ex_arg + expressões numéricas + ex_ponto
      .
    - Colchetes em nomes de variáveis/funções:
      -    ex_abre + expressões numéricas + ex_fecha
      .
    - Substituições:
      -    ex_varabre = ex_varini + ex_abre
      .
    .

    Operador: Operadores numéricos

    Interno: Usado internamente em Instr::Codif
*/
enum Expressao
{
    ex_fim,         ///< Fim da variável ou expressão numérica
    ex_coment,      ///< Marca início de comentário (encontrou #)

// Usado em textos
    ex_barra_n,     ///< Texto: \\n
    ex_barra_b,     ///< Texto: \\b
    ex_barra_c,     ///< Texto: \\c
    ex_barra_d,     ///< Texto: \\d

// Nome de variável ou função (um texto)
    ex_varini,      ///< Variável: Início do texto
    ex_varfim,      ///< Variável: Fim do texto
    ex_doispontos,  ///< Variável: ":", que separa nome da classe da variável
    ex_ponto,       ///< Variável: Fim do nome da variável
    ex_arg,         ///< Variável: Início da lista de argumentos
    ex_varabre,     ///< Variável: Início do texto + abre colchetes
    ex_abre,        ///< Variável: Abre colchetes; segue expressão numérica + ex_fecha
    ex_fecha,       ///< Variável: Fecha colchetes

// Valores fixos
    ex_nulo,        ///< Fixo: Valor NULO
    ex_txt,         ///< Fixo: Texto em ASCIIZ
    ex_num0,        ///< Fixo: número 0
    ex_num1,        ///< Fixo: Número 1
    ex_num8p,       ///< Fixo: + 1 byte = número 8 bits sem sinal
    ex_num16p,      ///< Fixo: + 2 bytes = número 16 bits sem sinal
    ex_num32p,      ///< Fixo: + 4 bytes = número 32 bits sem sinal
    ex_num8n,       ///< Fixo: + 1 byte = número 8 bits negativo
    ex_num16n,      ///< Fixo: + 2 bytes = número 16 bits negativo
    ex_num32n,      ///< Fixo: + 4 bytes = número 32 bits negativo
    ex_div1,        ///< Fixo: Divide por 10
    ex_div2,        ///< Fixo: Divide por 100
    ex_div3,        ///< Fixo: Divide por 1000
    ex_div4,        ///< Fixo: Divide por 10000
    ex_div5,        ///< Fixo: Divide por 100000
    ex_div6,        ///< Fixo: Divide por 1000000

// Operadores numéricos
    exo_ini,        ///< Operador: Marca o início dos operadores
    exo_virgula,    ///< Operador: Vírgula, para separar expressões
    exo_neg,        ///< Operador: -a
    exo_exclamacao, ///< Operador: !a
    exo_mul,        ///< Operador: a*b
    exo_div,        ///< Operador: a/b
    exo_porcent,    ///< Operador: a%b
    exo_add,        ///< Operador: a+b
    exo_sub,        ///< Operador: a-b
    exo_menor,      ///< Operador: a<b
    exo_menorigual, ///< Operador: a<=b
    exo_maior,      ///< Operador: a>b
    exo_maiorigual, ///< Operador: a>=b
    exo_igual,      ///< Operador: a=b
    exo_igual2,     ///< Operador: a==b
    exo_diferente,  ///< Operador: a!=b
    exo_e,          ///< Operador: a&b
    exo_ou,         ///< Operador: a|b
    exo_igualmul,   ///< Operador: a*=b   (segue exo_mul, exo_igual)
    exo_igualdiv,   ///< Operador: a/=b   (segue exo_div, exo_igual)
    exo_igualporcent,///< Operador: a%=b  (segue exo_porcent, exo_igual)
    exo_igualadd,   ///< Operador: a+=b   (segue exo_add, exo_igual)
    exo_igualsub,   ///< Operador: a-=b   (segue exo_sub, exo_igual)
    exo_fim,        ///< Operador: Marca o fim dos operadores
    exo_ee,         ///< Operador: Início do operador &
    exo_ouou,       ///< Operador: Início do operador |

// Usado ao codificar expressões
    ex_var1,        ///< Interno: Processando nome de variável; aceita dois pontos
    ex_var2,        ///< Interno: Processando nome de variável; não aceita dois pontos
    ex_colchetes,   ///< Interno: Processando colchetes em nome de variável
    ex_parenteses   ///< Interno: Processando parênteses
};

//----------------------------------------------------------------------------
}

#endif
