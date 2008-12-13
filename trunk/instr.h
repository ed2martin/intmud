#ifndef INSTR_H
#define INSTR_H

class TClasse;
class TObjeto;
class TVariavel;
class TObjeto;

/// Codificar, decodificar e executar instruções
/** Codificar, decodificar e executar instruções */
namespace Instr {

//----------------------------------------------------------------------------
// Funções
bool Codif(char * destino, const char * origem, int tamanho);
bool Decod(char * destino, const char * origem, int tamanho);
bool Mostra(char * destino, const char * origem, int tamanho);

void ApagarVar(TVariavel * v);
bool CriarVar(const char * defvar);
char * ProcuraExpr(char * expr, int valor);
const char * NomeComando(int valor);
const char * NomeExpr(int valor);

bool ExecIni(TClasse * classe, const char * func);
bool ExecIni(TObjeto * este, const char * func);
void ExecArg(char * txt);
bool ExecX();
void ExecFim();

bool ChecaHerda(const char * instr, const char * nomeclasse);
int  Prioridade(int operador);

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
    char * linha;       ///< Instrução codificada sendo executada
                        ///< Mesmo formato de TClasse::Comandos
    char * expr;        ///< Aonde parou na expressão numérica
                        ///< =0 se não estiver processando expressão numérica
    TVariavel * exprvar;///< Valor de VarAtual ao fazer expr!=0
    TVariavel * endvar; ///< Primeiro argumento da função
    char  numarg;       ///< Número de argumentos arg0 a arg9
    char  numvar;       ///< Número de variáveis definidas na função
    bool  igualcompara; ///< Se o sinal de igual compara ou atribui
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

//----------------------------------------------------------------------------
/// Comandos
/**
    Cada linha de comando é codificada assim:
    - bytes 0,1 = tamanho em bytes; 0 significa fim da lista de comandos
    - byte 2 = comando (vide TComando)
    - X bytes = dados do comando (depende do comando)
    .

    A partir de cVariaveis vem as definições de variáveis:
    - byte 3 = propriedades:
      - bit 0=1 se comum
      - bit 1=1 se sav
      .
    - byte 4
      - tamanho do texto em cTxt1 e cTxt2
      - índice para os dados extras das variáveis Const
      .
    - X bytes = nome da variável em ASCIIZ
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
    cIntInc, cIntDec,   ///< Var: intinc e intdec
    cReal,              ///< Var: real - "double"
    cRef,               ///< Var: Referência para um objeto
    cConstNulo,         ///< Var: Constante = nulo
    cConstTxt,          ///< Var: Constante = texto
    cConstNum,          ///< Var: Constante = número
    cConstExpr,         ///< Var: Constante = expressão numérica
    cFunc,              ///< Var: Função
    cVarFunc,           ///< Var: Função

// Variáveis extras
    cListaObj,          ///< Extra: ListaObj
    cListaTxt,          ///< Extra: cListaTxt
    cListaMsg,          ///< Extra: cListaMsg
    cNomeObj,           ///< Extra: cNomeObj
    cLog,               ///< Extra: cLog
    cVarTempo,          ///< Extra: cVarTempo
    cSocket,            ///< Extra: cSocket
    cServ,              ///< Extra: cServ
    cSalvar,            ///< Extra: cSalvar
    cProg,              ///< Extra: cProg
    cIndice,            ///< Extra: cIndice

// Usado internamente
    cTxtFixo,           ///< Aponta para um texto fixo

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
