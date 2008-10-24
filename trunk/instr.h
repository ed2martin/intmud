#ifndef EXPR_H
#define EXPR_H

namespace Instr {

//----------------------------------------------------------------------------
// Fun��es
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
    void Inicio();      // Indica que est� no in�cio da lista de instru��es
    const char * Instr(const char * instr);
                        // Checa pr�xima instru��o da lista
                        // Retorna 0 se instru��o est� no lugar correto
                        // Caso contr�rio, retorna mensagem de erro
private:
    char esperando;     // 0=in�cio do arquivo; pode receber "herda"
                        // 1=nenhuma fun��o definida
                        // 2=instru��es e vari�veis de uma fun��o
                        // 3=instru��es de uma fun��o
                        // 4=ap�s defini��o de fun��o + defini��o de constante
};

//----------------------------------------------------------------------------
// Cada linha de comando:
// bytes 0,1 = tamanho em bytes; 0 significa fim da lista de comandos
// byte 2 = comando (vide TComando)
// X bytes = dados do comando (depende do comando)

// Nos comandos que n�o t�m express�es num�ricas (exceto Herda),
// se houver algum coment�rio, ele vir� ap�s os dados do comando.
// Exemplo,  "fimse # teste" � codificado assim:
//           0x08, 0x00, cFimSe, 't', 'e', 's', 't', 'e'

// Comandos
enum Comando
{
    cHerda,         // 1 byte = n�mero de classes
                    // X bytes = nomes das classes em ASCIIZ
                    // S� pode estar no in�cio da lista de comandos
    cExpr,          // Express�o num�rica pura
    cComent,        // Coment�rio

// Instru��es de controle de fluxo
    cSe,            // ushort,express�o   se(express�o)
    cSenao1,        //                    sen�o
    cSenao2,        // ushort,express�o   sen�o(express�o)
    cFimSe,         //                    fimse
    cEnquanto,      // ushort,express�o   enquanto(express�o)
    cEFim,          // ushort             efim
    cRet1,          // ret sem argumentos
    cRet2,          // ret com express�o num�rica
    cSair,          // ushort             sair
    cContinuar,     // ushort             continuar
    cTerminar,      //                    terminar

// Defini��es de vari�veis:
// byte 3 = propriedades:
//      bit 0=1 se comum
//      bit 1=1 se sav
// byte 4 = tamanho do texto em cTxt1 e cTxt2
//        = �ndice para os dados extras das vari�veis Const
// X bytes = nome da vari�vel em ASCIIZ
// Y bytes = express�o num�rica, em cConstExp
//         = texto em ASCIIZ, em cConstTxt
//         = valor num�rico em cConstInt e cConstReal
    cVariaveis,         // Marca o in�cio das vari�veis
    cTxt1,              // Texto de 1 a 256 caracteres
    cTxt2,              // Texto de 257 a 512 caracteres
    cIntb0, cIntb1, cIntb2, cIntb3, // 1 bit
    cIntb4, cIntb5, cIntb6, cIntb7, // 1 bit
    cInt8, cUInt8,      // 8 bits com e sem sinal
    cInt16, cUInt16,    // 16 bits com e sem sinal
    cInt32, cUInt32,    // 32 bits com e sem sinal
    cIntInc, cIntDec,   // intinc e intdec
    cReal,              // real - "double"
    cRef,               // Refer�ncia para um objeto
    cConstNulo,         // Constante = nulo
    cConstTxt,          // Constante = texto
    cConstNum,          // Constante = n�mero
    cConstExpr,         // Constante = express�o num�rica
    cFunc,              // Fun��o

// Vari�veis extras
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

    cTotalComandos      // N�mero de comandos - n�o usado
};

//----------------------------------------------------------------------------
// Em express�es num�ricas:
enum Expressao
{
    ex_fim,         // Fim da vari�vel ou express�o num�rica
    ex_coment,      // Marca in�cio de coment�rio (encontrou #)

// Usado em textos
    ex_barra_n,     // \n
    ex_barra_b,     // \b
    ex_barra_c,     // \c
    ex_barra_d,     // \d

// Nome de vari�vel ou fun��o (um texto)
    // Cada conjunto de vari�veis/fun��es (no estilo x.y.x):
    //     ex_varini + vari�veis/fun��es + ex_varfim
    // Cada vari�vel/fun��o (sem e com argumentos):
    //     nome da vari�vel/fun��o + ex_ponto
    //     nome da vari�vel/fun��o + ex_arg + express�es num�ricas + ex_ponto
    // Colchetes em nomes de vari�veis/fun��es:
    //     ex_abre + express�es num�ricas + ex_fecha
    // Substitui��es:
    //     ex_varabre = ex_varini + ex_abre
    ex_varini,      // In�cio do texto
    ex_varfim,      // Fim do texto
    ex_ponto,       // Fim do nome da vari�vel
    ex_arg,         // In�cio da lista de argumentos
    ex_varabre,     // In�cio do texto + abre colchetes
    ex_abre,        // Abre colchetes; segue express�o num�rica + ex_fecha
    ex_fecha,       // Fecha colchetes

// Valores fixos
    ex_nulo,        // Valor NULO
    ex_txt,         // Texto em ASCIIZ
    ex_num0,        // n�mero 0
    ex_num1,        // N�mero 1
    ex_num8p,       // + 1 byte = n�mero 8 bits sem sinal
    ex_num16p,      // + 2 bytes = n�mero 16 bits sem sinal
    ex_num32p,      // + 4 bytes = n�mero 32 bits sem sinal
    ex_num8n,       // + 1 byte = n�mero 8 bits negativo
    ex_num16n,      // + 2 bytes = n�mero 16 bits negativo
    ex_num32n,      // + 4 bytes = n�mero 32 bits negativo
    ex_div1,        // Divide por 10
    ex_div2,        // Divide por 100
    ex_div3,        // Divide por 1000
    ex_div4,        // Divide por 10000
    ex_div5,        // Divide por 100000
    ex_div6,        // Divide por 1000000

// Operadores num�ricos
    exo_ini,        // Marca o in�cio dos operadores
    exo_virgula,    // V�rgula, para separar express�es
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
    exo_ee,         // In�cio do operador &
    exo_ouou,       // In�cio do operador |

// Usado ao codificar express�es
    ex_var1,        // Processando nome de vari�vel; aceita dois pontos
    ex_var2,        // Processando nome de vari�vel; n�o aceita dois pontos
    ex_colchetes,   // Processando colchetes em nome de vari�vel
    ex_parenteses   // Processando par�nteses
};

//----------------------------------------------------------------------------
}

#endif
