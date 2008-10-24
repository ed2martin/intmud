#ifndef EXPR_H
#define EXPR_H

namespace Instr {

//----------------------------------------------------------------------------
// Funções
bool Codif(char * destino, const char * origem, int tamanho);
bool Decod(char * destino, const char * origem, int tamanho);
bool Mostra(char * destino, const char * origem, int tamanho);
bool ChecaHerda(const char * instr, const char * nomeclasse);
int  Prioridade(int operador);

//----------------------------------------------------------------------------
class ChecaLinha
{
public:
    ChecaLinha();
    void Inicio();      // Indica que está no início da lista de instruções
    const char * Instr(const char * instr);
                        // Checa próxima instrução da lista
                        // Retorna 0 se instrução está no lugar correto
                        // Caso contrário, retorna mensagem de erro
private:
    char esperando;     // 0=início do arquivo; pode receber "herda"
                        // 1=nenhuma função definida
                        // 2=instruções e variáveis de uma função
                        // 3=instruções de uma função
                        // 4=após definição de função + definição de constante
};

//----------------------------------------------------------------------------
// Cada linha de comando:
// bytes 0,1 = tamanho em bytes; 0 significa fim da lista de comandos
// byte 2 = comando (vide TComando)
// X bytes = dados do comando (depende do comando)

// Nos comandos que não têm expressões numéricas (exceto Herda),
// se houver algum comentário, ele virá após os dados do comando.
// Exemplo,  "fimse # teste" é codificado assim:
//           0x08, 0x00, cFimSe, 't', 'e', 's', 't', 'e'

// Comandos
enum Comando
{
    cHerda,         // 1 byte = número de classes
                    // X bytes = nomes das classes em ASCIIZ
                    // Só pode estar no início da lista de comandos
    cExpr,          // Expressão numérica pura
    cComent,        // Comentário

// Instruções de controle de fluxo
    cSe,            // ushort,expressão   se(expressão)
    cSenao1,        //                    senão
    cSenao2,        // ushort,expressão   senão(expressão)
    cFimSe,         //                    fimse
    cEnquanto,      // ushort,expressão   enquanto(expressão)
    cEFim,          // ushort             efim
    cRet1,          // ret sem argumentos
    cRet2,          // ret com expressão numérica
    cSair,          // ushort             sair
    cContinuar,     // ushort             continuar
    cTerminar,      //                    terminar

// Definições de variáveis:
// byte 3 = propriedades:
//      bit 0=1 se comum
//      bit 1=1 se sav
// byte 4 = tamanho do texto em cTxt1 e cTxt2
//        = índice para os dados extras das variáveis Const
// X bytes = nome da variável em ASCIIZ
// Y bytes = expressão numérica, em cConstExp
//         = texto em ASCIIZ, em cConstTxt
//         = valor numérico em cConstInt e cConstReal
    cVariaveis,         // Marca o início das variáveis
    cTxt1,              // Texto de 1 a 256 caracteres
    cTxt2,              // Texto de 257 a 512 caracteres
    cIntb0, cIntb1, cIntb2, cIntb3, // 1 bit
    cIntb4, cIntb5, cIntb6, cIntb7, // 1 bit
    cInt8, cUInt8,      // 8 bits com e sem sinal
    cInt16, cUInt16,    // 16 bits com e sem sinal
    cInt32, cUInt32,    // 32 bits com e sem sinal
    cIntInc, cIntDec,   // intinc e intdec
    cReal,              // real - "double"
    cRef,               // Referência para um objeto
    cConstNulo,         // Constante = nulo
    cConstTxt,          // Constante = texto
    cConstNum,          // Constante = número
    cConstExpr,         // Constante = expressão numérica
    cFunc,              // Função

// Variáveis extras
    cListaObj,
    cListaTxt,
    cListaMsg,
    cNomeObj,
    cLog,
    cVarTempo,
    cSocket,
    cServ,
    cSalvar,
    cProg,
    cIndice,

    cTotalComandos      // Número de comandos - não usado
};

//----------------------------------------------------------------------------
// Em expressões numéricas:
enum Expressao
{
    ex_fim,         // Fim da variável ou expressão numérica
    ex_coment,      // Marca início de comentário (encontrou #)

// Usado em textos
    ex_barra_n,     // \n
    ex_barra_b,     // \b
    ex_barra_c,     // \c
    ex_barra_d,     // \d

// Nome de variável ou função (um texto)
    // Cada conjunto de variáveis/funções (no estilo x.y.x):
    //     ex_varini + variáveis/funções + ex_varfim
    // Cada variável/função (sem e com argumentos):
    //     nome da variável/função + ex_ponto
    //     nome da variável/função + ex_arg + expressões numéricas + ex_ponto
    // Colchetes em nomes de variáveis/funções:
    //     ex_abre + expressões numéricas + ex_fecha
    // Substituições:
    //     ex_varabre = ex_varini + ex_abre
    ex_varini,      // Início do texto
    ex_varfim,      // Fim do texto
    ex_ponto,       // Fim do nome da variável
    ex_arg,         // Início da lista de argumentos
    ex_varabre,     // Início do texto + abre colchetes
    ex_abre,        // Abre colchetes; segue expressão numérica + ex_fecha
    ex_fecha,       // Fecha colchetes

// Valores fixos
    ex_nulo,        // Valor NULO
    ex_txt,         // Texto em ASCIIZ
    ex_num0,        // número 0
    ex_num1,        // Número 1
    ex_num8p,       // + 1 byte = número 8 bits sem sinal
    ex_num16p,      // + 2 bytes = número 16 bits sem sinal
    ex_num32p,      // + 4 bytes = número 32 bits sem sinal
    ex_num8n,       // + 1 byte = número 8 bits negativo
    ex_num16n,      // + 2 bytes = número 16 bits negativo
    ex_num32n,      // + 4 bytes = número 32 bits negativo
    ex_div1,        // Divide por 10
    ex_div2,        // Divide por 100
    ex_div3,        // Divide por 1000
    ex_div4,        // Divide por 10000
    ex_div5,        // Divide por 100000
    ex_div6,        // Divide por 1000000

// Operadores numéricos
    exo_ini,        // Marca o início dos operadores
    exo_virgula,    // Vírgula, para separar expressões
    exo_neg,        // -a
    exo_exclamacao, // !a
    exo_mul,        // a*b
    exo_div,        // a/b
    exo_porcent,    // a%b
    exo_add,        // a+b
    exo_sub,        // a-b
    exo_menor,      // a<b
    exo_menorigual, // a<=b
    exo_maior,      // a>b
    exo_maiorigual, // a>=b
    exo_igual,      // a=b
    exo_diferente,  // a!=b
    exo_e,          // a&b
    exo_ou,         // a|b
    exo_igualmul,   // a*=b
    exo_igualdiv,   // a/=b
    exo_igualporcent, // a%=b
    exo_igualadd,   // a+=b
    exo_igualsub,   // a-=b
    exo_fim,        // Marca o fim dos operadores
    exo_ee,         // Início do operador &
    exo_ouou,       // Início do operador |

// Usado ao codificar expressões
    ex_var1,        // Processando nome de variável; aceita dois pontos
    ex_var2,        // Processando nome de variável; não aceita dois pontos
    ex_colchetes,   // Processando colchetes em nome de variável
    ex_parenteses   // Processando parênteses
};

//----------------------------------------------------------------------------
}

#endif
