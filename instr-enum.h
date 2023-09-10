#ifndef INSTR_ENUM_H
#define INSTR_ENUM_H

/// Emumerações de Instr
/** Emumerações de Instr */
namespace Instr {

//----------------------------------------------------------------------------
enum EndVar
{
    endAlin = 3,
    endProp = 4,
    endIndice = 5,
    endVetor = 6,
    endExtra = 7,
    endNome = 8,
    endVar = 4
};

//----------------------------------------------------------------------------
/// Comandos
/**
    Cada linha de comando é codificada assim:
    - bytes 0,1 = tamanho em bytes; 0 significa fim da lista de comandos
    - byte 2 = comando (vide TComando)
    - byte 3 (Instr::endAlin) = alinhamento (nível de identação)
    - X bytes (Instr::endVar) = dados do comando (depende do comando)
    .

    A partir de cVariaveis vem as definições de variáveis:
    - byte 4 (Instr::endProp) = propriedades:
      - bit 0=1 se comum
      - bit 1=1 se sav
      .
    - byte 5 (Instr::endIndice)
      - índice para os dados extras das variáveis (expressão)
      - Ou 0 se não há dados extras
      .
    - byte 6 (Instr::endVetor)
      - número de elementos do vetor ou 0 se não for vetor
      .
    - byte 7 (Instr::endExtra)
      - tamanho do texto em cTxt1 e cTxt2
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
    cRefVar,        ///< Comum: refvar (referência a uma variável)

// Instruções de controle de fluxo
    cSe,            ///< Fluxo: ushort,expressão   se(expressão)
    cSenao1,        ///< Fluxo: ushort             senão
    cSenao2,        ///< Fluxo: ushort,expressão   senão(expressão)
    cFimSe,         ///< Fluxo:                    fimse
    cEnquanto,      ///< Fluxo: ushort,expressão   enquanto(expressão)
    cEPara,         ///< Fluxo: ushort[3],expressão
    cEFim,          ///< Fluxo: ushort             efim
    cCasoVar,       ///< Fluxo: ushort,expressão   casovar
    cCasoSe,        ///< Fluxo: ushort[2],texto    casose com texto
    cCasoSePadrao,  ///< Fluxo:                    casose sem texto
    cCasoFim,       ///< Fluxo:                    casofim
    cRet1,          ///< Fluxo: ret sem argumentos
    cRet2,          ///< Fluxo: ret com expressão numérica
    cSair1,         ///< Fluxo: ushort             sair
    cSair2,         ///< Fluxo: ushort,expressão   sair
    cContinuar1,    ///< Fluxo: ushort             continuar
    cContinuar2,    ///< Fluxo: ushort,expressão   continuar
    cTerminar,      ///< Fluxo:                    terminar

// Definições de variáveis
    cVariaveis,         ///< Var: Marca o início das variáveis
    cTxt1,              ///< Var: Texto de 1 a 256 caracteres
    cTxt2,              ///< Var: Texto de 257 a 512 caracteres
    cInt1,              ///< Var: 1 bit
    cInt8, cUInt8,      ///< Var: 8 bits com e sem sinal
    cInt16, cUInt16,    ///< Var: 16 bits com e sem sinal
    cInt32, cUInt32,    ///< Var: 32 bits com e sem sinal
    cReal,              ///< Var: real - "float"
    cReal2,             ///< Var: real2 - "double"
    cConstNulo,         ///< Var: Constante = nulo
    cConstTxt,          ///< Var: Constante = texto
    cConstNum,          ///< Var: Constante = número
    cConstExpr,         ///< Var: Constante = expressão numérica
    cConstVar,          ///< Var: Constante do tipo constvar
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
    cTextoTxt,          ///< Extra: TextoTxt
    cTextoPos,          ///< Extra: TextoPos
    cTextoVar,          ///< Extra: TextoVar
    cTextoObj,          ///< Extra: TextoObj
    cNomeObj,           ///< Extra: NomeObj
    cArqDir,            ///< Extra: ArqDir
    cArqLog,            ///< Extra: ArqLog
    cArqProg,           ///< Extra: ArqProg
    cArqExec,           ///< Extra: ArqExec
    cArqSav,            ///< Extra: Salvar
    cArqTxt,            ///< Extra: ArqTxt
    cArqMem,            ///< Extra: ArqMem
    cIntTempo,          ///< Extra: IntTempo
    cIntExec,           ///< Extra: IntExec
    cTelaTxt,           ///< Extra: TelaTxt
    cSocket,            ///< Extra: Socket
    cServ,              ///< Extra: Serv
    cProg,              ///< Extra: Prog
    cDebug,             ///< Extra: Debug
    cIndiceObj,         ///< Extra: IndiceObj
    cIndiceItem,        ///< Extra: IndiceItem
    cDataHora,          ///< Extra: DataHora

// Usado internamente
    cTxtFixo,           ///< Aponta para um texto fixo
    cVarNome,           ///< Para obter nome da variável
    cVarInicio,         ///< Esperando texto logo após ex_varini
    cVarIniFunc,        ///< Mesmo que cVarInicio, porém para ex_varfunc
    cVarClasse,         ///< TVariavel::endvar = endereço do objeto TClasse
    cVarObjeto,         ///< TVariavel::endvar = endereço do objeto TObjeto
    cVarInt,            ///< int local; vide TVariavel::valor_int
    cTextoVarSub,       ///< Variável de TextoVar
    cTextoObjSub,       ///< Variável de TextoObj

    cTotalComandos      ///< Número de comandos - não usado
};

//----------------------------------------------------------------------------
/// Idenfificadores usados em expressões numéricas
/**
    Texto: Usado em textos

    Variável: Nome de variável ou função (um texto)
    - Cada conjunto de variáveis/funções (no estilo x.y.x):
      -  ex_varini + variáveis/funções + ex_varfim
      .
    - Cada variável/função (sem e com argumentos):
      -  nome da variável/função + ex_ponto
      -  nome da variável/função + ex_arg + expressões numéricas + ex_ponto
      .
    - Colchetes em nomes de variáveis/funções:
      -  ex_abre + expressões numéricas + ex_fecha
      .
    - Substituições:
      -  ex_varabre = ex_varini + ex_abre
      -  ex_varfunc + número da função = ex_varini + nome da função predefinida
      -  ex_varlocal + número da variável = ex_varini (para uma variável local)
      -  ex_varlocal + 255 = ex_varini (para uma variável da classe/objeto)
      -  Nota: variável local é uma variável definida na função
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
    ex_varfunc,     ///< Variável: próximo byte é o número da função predefinida
    ex_varlocal,    ///< Variável: (+1 byte) variável/função exceto predefinida

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
    ex_num8hexp,    ///< Fixo: + 1 byte = número hexadecimal 8 bits sem sinal
    ex_num16hexp,   ///< Fixo: + 2 bytes = número hexadecimal 16 bits sem sinal
    ex_num32hexp,   ///< Fixo: + 4 bytes = número hexadecimal 32 bits sem sinal
    ex_num8hexn,    ///< Fixo: + 1 byte = número hexadecimal 8 bits negativo
    ex_num16hexn,   ///< Fixo: + 2 bytes = número hexadecimal 16 bits negativo
    ex_num32hexn,   ///< Fixo: + 4 bytes = número hexadecimal 32 bits negativo
    ex_div1,        ///< Fixo: Divide por 10
    ex_div2,        ///< Fixo: Divide por 100
    ex_div3,        ///< Fixo: Divide por 1000
    ex_div4,        ///< Fixo: Divide por 10000
    ex_div5,        ///< Fixo: Divide por 100000
    ex_div6,        ///< Fixo: Divide por 1000000

// Operadores numéricos
    exo_ini,        ///< Operador: Marca o início dos operadores
    exo_virgula,    ///< Operador: Vírgula
    exo_virg_expr,  ///< Operador: Vírgula, separando expressões
    exo_neg,        ///< Operador: -a
    exo_exclamacao, ///< Operador: !a
    exo_b_comp,     ///< Operador: ~a
    exo_add_antes,  ///< Operador: ++a
    exo_sub_antes,  ///< Operador: --a
    exo_add_depois, ///< Operador: a++
    exo_sub_depois, ///< Operador: a--
    exo_add_sub1,   ///< Usado em ++ e --
    exo_add_sub2,   ///< Usado em ++ e --
    exo_mul,        ///< Operador: a * b
    exo_div,        ///< Operador: a / b
    exo_porcent,    ///< Operador: a % b
    exo_add,        ///< Operador: a + b
    exo_sub,        ///< Operador: a - b
    exo_b_shl,      ///< Operador: a << b
    exo_b_shr,      ///< Operador: a >> b
    exo_b_e,        ///< Operador: a & b
    exo_b_ouou,     ///< Operador: a ^ b
    exo_b_ou,       ///< Operador: a | b
    exo_menor,      ///< Operador: a < b
    exo_menorigual, ///< Operador: a <= b
    exo_maior,      ///< Operador: a > b
    exo_maiorigual, ///< Operador: a >= b
    exo_igual,      ///< Operador: a == b
    exo_igual2,     ///< Operador: a === b
    exo_diferente,  ///< Operador: a != b
    exo_diferente2, ///< Operador: a !== b
    exo_e,          ///< Operador: a && b
    exo_ou,         ///< Operador: a || b
    exo_atrib,      ///< Operador: a = b
    exo_i_mul,      ///< Operador: a *= b   (segue exo_mul, exo_atrib)
    exo_i_div,      ///< Operador: a /= b   (segue exo_div, exo_atrib)
    exo_i_porcent,  ///< Operador: a %= b   (segue exo_porcent, exo_atrib)
    exo_i_add,      ///< Operador: a += b   (segue exo_add, exo_atrib)
    exo_i_sub,      ///< Operador: a -= b   (segue exo_sub, exo_atrib)
    exo_i_b_shl,    ///< Operador: a <<= b  (segue exo_b_shl, exo_atrib)
    exo_i_b_shr,    ///< Operador: a >>= b  (segue exo_b_shr, exo_atrib)
    exo_i_b_e,      ///< Operador: a &= b   (segue exo_b_e, exo_atrib)
    exo_i_b_ouou,   ///< Operador: a ^= b   (segue exo_b_ouou, exo_atrib)
    exo_i_b_ou,     ///< Operador: a |= b   (segue exo_b_ou, exo_atrib)
    exo_int2,       ///< Operador: Fim de ?
    exo_intint2,    ///< Operador: Fim de ??
    exo_dponto2,    ///< Operador: Fim de :
    exo_fim,        ///< Operador: Marca o fim dos operadores
    exo_int1,       ///< Operador: Início de ?
    exo_dponto1,    ///< Operador: Início de :
    exo_intint1,    ///< Operador: Início de ??
    exo_ee,         ///< Operador: Início do operador &&
    exo_ouou,       ///< Operador: Início do operador ||

// Usado ao codificar expressões
    ex_var1,        ///< Interno: Processando nome de variável; aceita dois pontos
    ex_var2,        ///< Interno: Processando nome de variável; não aceita dois pontos
    ex_var3,        ///< Interno: Processando nome de variável; já anotou ponto
    ex_colchetes,   ///< Interno: Processando colchetes em nome de variável
    ex_parenteses   ///< Interno: Processando parênteses
};

//----------------------------------------------------------------------------
}

#endif
