#ifndef INSTR_H
#define INSTR_H

class TClasse;
class TObjeto;
class TVariavel;

/// Codificar, decodificar e executar instru��es
/** Codificar, decodificar e executar instru��es */
namespace Instr {

//----------------------------------------------------------------------------
// Fun��es
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
// Vari�veis predefinidas
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
/// Quantas instru��es pode executar antes que o controle retorne ao programa
extern int VarExec;
/// Valor inicial de Instr::VarExec quando Instr::ExecIni � executado
extern int VarExecIni;

//----------------------------------------------------------------------------
/** Verifica se instru��es de uma classe (codificadas por Instr::Codif)
    est�o na ordem correta */
class ChecaLinha /// Verifica ordem das instru��es de uma classe
{
public:
    ChecaLinha();
    void Inicio();
            /**< Indica que est� no in�cio da lista de instru��es */
    const char * Instr(const char * instr);
            /**< Checa pr�xima instru��o da lista
                @param instr Instru��o codificada
                @return 0 se instru��o est� no lugar correto,
                        outro valor=mensagem de erro */
private:
    char esperando;
            /**< O que est� esperando:
                - 0=in�cio do arquivo; pode receber "herda"
                - 1=nenhuma fun��o definida
                - 2=instru��es e vari�veis de uma fun��o
                - 3=instru��es de uma fun��o
                - 4=ap�s defini��o de fun��o + defini��o de constante */
};

//----------------------------------------------------------------------------
/** Pilha de fun��es - usado ao executar fun��es */
class ExecFunc /// Pilha de fun��es
{
public:
    TObjeto * este;     ///< Objeto ao qual a fun��o pertence
    const char * linha; ///< Instru��o codificada sendo executada
                        ///< Mesmo formato de TClasse::Comandos
    const char * expr;  ///< Aonde parou na express�o num�rica
                        ///< =0 se n�o estiver processando express�o num�rica
    TVariavel * inivar; ///< Primeiro argumento da fun��o
    TVariavel * fimvar; ///< Primeira vari�vel ap�s vari�veis locais da fun��o
    char  numarg;       ///< N�mero de argumentos arg0 a arg9
    char  tipo;         ///< 0=func 1=ler varfunc 2=mudar varfunc 3=criar()
    bool  igualcompara; ///< Se o sinal de igual compara ou atribui
};

//----------------------------------------------------------------------------
/** Processa fun��es predefinidas
    @note A lista de fun��es est� definida em instr-exec.cpp
*/
class TListaFunc /// Fun��es internas
{
public:
        /// Nome da fun��o predefinida
    const char * Nome;

        /// Fun��o que processa a fun��o predefinida
        /** Ao executar a fun��o, a pilha de vari�veis est� assim:
        - cVarNome
        - cVarInicio  (v aponta para essa vari�vel)
        - Argumentos da fun��o interna (VarAtual aponta para a �ltima)
        .
        Se retornar true, deve:
        - Apagar vari�veis de v em diante
        - Criar uma vari�vel com o resultado
        .
        @param v Endere�o da vari�vel cVarInicio
        @param valor Valor de TListaFunc::valor
        @retval true Se processou normalmente
        @retval false Se erro; o resultado ser� o tipo NULO */
    bool (*Func)(TVariavel * v, int valor);

        /// Valor passado � fun��o Func
    int  valor;
};

//----------------------------------------------------------------------------
    /// Pilha de dados (64K)
extern char * const DadosPilha;
extern char * const DadosFim;
    /// Pilha de dados - in�cio da �rea de dados dispon�vel
extern char * DadosTopo;

    /// Pilha de vari�veis
extern TVariavel * const VarPilha;
extern TVariavel * const VarFim;
    /// Pilha de vari�veis - �ltima vari�vel da pilha
extern TVariavel * VarAtual;

    /// Pilha de fun��es
extern ExecFunc * const FuncPilha;
extern ExecFunc * const FuncFim;
    /// Pilha de fun��es - �ltima fun��o da pilha
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
    Cada linha de comando � codificada assim:
    - bytes 0,1 = tamanho em bytes; 0 significa fim da lista de comandos
    - byte 2 = comando (vide TComando)
    - X bytes = dados do comando (depende do comando)
    .

    A partir de cVariaveis vem as defini��es de vari�veis:
    - byte 3 (Instr::endProp) = propriedades:
      - bit 0=1 se comum
      - bit 1=1 se sav
      .
    - byte 4 (Instr::endIndice)
      - tamanho do texto em cTxt1 e cTxt2
      - �ndice para os dados extras das vari�veis Const
      .
    - byte 5 (Instr::endVetor)
      - n�mero de elementos do vetor ou 0 se n�o for vetor
      .
    - X bytes (Instr::endNome) = nome da vari�vel em ASCIIZ
    - Y bytes = express�o num�rica, em cConstExp
      - texto em ASCIIZ, em cConstTxt
      - valor num�rico em cConstInt e cConstReal
      .
    .

    Tipos de comandos:
    - Comum: Instru��es comuns (n�o se encaixam em outra categoria)
    - Fluxo: Instru��es de controle de fluxo
    - Var:   Defini��es de vari�veis
    - Extra: Vari�veis extras
    .

    Nos comandos que n�o t�m express�es num�ricas (exceto Herda),
    se houver algum coment�rio, ele vir� ap�s os dados do comando.

    Exemplo,  "fimse # teste" � codificado assim:
              0x08, 0x00, cFimSe, 't', 'e', 's', 't', 'e'
 */
enum Comando
{
// Instru��es comuns
    cHerda,         /**< Comum: Instru��o herda
                        - 1 byte = n�mero de classes
                        - X bytes = nomes das classes em ASCIIZ
                        - S� pode estar no in�cio da lista de comandos */
    cExpr,          ///< Comum: Express�o num�rica pura
    cComent,        ///< Comum: Coment�rio

// Instru��es de controle de fluxo
    cSe,            ///< Fluxo: ushort,express�o   se(express�o)
    cSenao1,        ///< Fluxo: ushort             sen�o
    cSenao2,        ///< Fluxo: ushort,express�o   sen�o(express�o)
    cFimSe,         ///< Fluxo:                    fimse
    cEnquanto,      ///< Fluxo: ushort,express�o   enquanto(express�o)
    cEFim,          ///< Fluxo: ushort             efim
    cRet1,          ///< Fluxo: ret sem argumentos
    cRet2,          ///< Fluxo: ret com express�o num�rica
    cSair,          ///< Fluxo: ushort             sair
    cContinuar,     ///< Fluxo: ushort             continuar
    cTerminar,      ///< Fluxo:                    terminar

// Defini��es de vari�veis
    cVariaveis,         ///< Var: Marca o in�cio das vari�veis
    cTxt1,              ///< Var: Texto de 1 a 256 caracteres
    cTxt2,              ///< Var: Texto de 257 a 512 caracteres
    cInt1,              ///< Var: 1 bit
    cInt8, cUInt8,      ///< Var: 8 bits com e sem sinal
    cInt16, cUInt16,    ///< Var: 16 bits com e sem sinal
    cInt32, cUInt32,    ///< Var: 32 bits com e sem sinal
    cReal,              ///< Var: real - "double"
    cConstNulo,         ///< Var: Constante = nulo
    cConstTxt,          ///< Var: Constante = texto
    cConstNum,          ///< Var: Constante = n�mero
    cConstExpr,         ///< Var: Constante = express�o num�rica
    cFunc,              ///< Var: Fun��o
    cVarFunc,           ///< Var: Fun��o
        /**< @note Para as vari�veis ap�s cVarFunc, deve-se
            obrigatoriamente usar TVariavel::Criar e TVariavel::Apagar,
            exceto quando TVariavel::Tamanho retorna 0 */
    cIntInc, cIntDec,   ///< Var: intinc e intdec
    cRef,               ///< Var: Refer�ncia para um objeto
                        /**< @sa TVarRef */

// Vari�veis extras
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
    cVarNome,           ///< Para obter nome da vari�vel
    cVarInicio,         ///< Esperando texto logo ap�s ex_varini
    cVarClasse,         ///< TVariavel::endvar = endere�o do objeto TClasse
    cVarObjeto,         ///< TVariavel::endvar = endere�o do objeto TObjeto
    cVarInt,            ///< int local; vide TVariavel::valor_int

    cTotalComandos      ///< N�mero de comandos - n�o usado
};

//----------------------------------------------------------------------------
/// Idenfificadores usados em express�es num�ricas
/**
    Texto: Usado em textos

    Vari�vel: Nome de vari�vel ou fun��o (um texto)
    - Cada conjunto de vari�veis/fun��es (no estilo x.y.x):
      -    ex_varini + vari�veis/fun��es + ex_varfim
      .
    - Cada vari�vel/fun��o (sem e com argumentos):
      -    nome da vari�vel/fun��o + ex_ponto
      -    nome da vari�vel/fun��o + ex_arg + express�es num�ricas + ex_ponto
      .
    - Colchetes em nomes de vari�veis/fun��es:
      -    ex_abre + express�es num�ricas + ex_fecha
      .
    - Substitui��es:
      -    ex_varabre = ex_varini + ex_abre
      .
    .

    Operador: Operadores num�ricos

    Interno: Usado internamente em Instr::Codif
*/
enum Expressao
{
    ex_fim,         ///< Fim da vari�vel ou express�o num�rica
    ex_coment,      ///< Marca in�cio de coment�rio (encontrou #)

// Usado em textos
    ex_barra_n,     ///< Texto: \\n
    ex_barra_b,     ///< Texto: \\b
    ex_barra_c,     ///< Texto: \\c
    ex_barra_d,     ///< Texto: \\d

// Nome de vari�vel ou fun��o (um texto)
    ex_varini,      ///< Vari�vel: In�cio do texto
    ex_varfim,      ///< Vari�vel: Fim do texto
    ex_doispontos,  ///< Vari�vel: ":", que separa nome da classe da vari�vel
    ex_ponto,       ///< Vari�vel: Fim do nome da vari�vel
    ex_arg,         ///< Vari�vel: In�cio da lista de argumentos
    ex_varabre,     ///< Vari�vel: In�cio do texto + abre colchetes
    ex_abre,        ///< Vari�vel: Abre colchetes; segue express�o num�rica + ex_fecha
    ex_fecha,       ///< Vari�vel: Fecha colchetes

// Valores fixos
    ex_nulo,        ///< Fixo: Valor NULO
    ex_txt,         ///< Fixo: Texto em ASCIIZ
    ex_num0,        ///< Fixo: n�mero 0
    ex_num1,        ///< Fixo: N�mero 1
    ex_num8p,       ///< Fixo: + 1 byte = n�mero 8 bits sem sinal
    ex_num16p,      ///< Fixo: + 2 bytes = n�mero 16 bits sem sinal
    ex_num32p,      ///< Fixo: + 4 bytes = n�mero 32 bits sem sinal
    ex_num8n,       ///< Fixo: + 1 byte = n�mero 8 bits negativo
    ex_num16n,      ///< Fixo: + 2 bytes = n�mero 16 bits negativo
    ex_num32n,      ///< Fixo: + 4 bytes = n�mero 32 bits negativo
    ex_div1,        ///< Fixo: Divide por 10
    ex_div2,        ///< Fixo: Divide por 100
    ex_div3,        ///< Fixo: Divide por 1000
    ex_div4,        ///< Fixo: Divide por 10000
    ex_div5,        ///< Fixo: Divide por 100000
    ex_div6,        ///< Fixo: Divide por 1000000

// Operadores num�ricos
    exo_ini,        ///< Operador: Marca o in�cio dos operadores
    exo_virgula,    ///< Operador: V�rgula, para separar express�es
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
    exo_ee,         ///< Operador: In�cio do operador &
    exo_ouou,       ///< Operador: In�cio do operador |

// Usado ao codificar express�es
    ex_var1,        ///< Interno: Processando nome de vari�vel; aceita dois pontos
    ex_var2,        ///< Interno: Processando nome de vari�vel; n�o aceita dois pontos
    ex_colchetes,   ///< Interno: Processando colchetes em nome de vari�vel
    ex_parenteses   ///< Interno: Processando par�nteses
};

//----------------------------------------------------------------------------
}

#endif
