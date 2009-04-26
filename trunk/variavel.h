#ifndef VARIAVEL_H
#define VARIAVEL_H

// Acima desse valor, double é mostrado com notação científica
#define DOUBLE_MAX 1000000000000000000.0

class TClasse;
class TObjeto;
class TVarRef;
class TVarIncDec;
class TVarIntTempo;
class TListaObj;
class TListaItem;
class TVarNomeObj;
class TVarSocket;
class TVarServ;
class TIndiceObj;
class TIndiceItem;

//----------------------------------------------------------------------------
/// Tipo de variável
enum TVarTipo
{
    varOutros,  ///< Desconhecido
    varInt,     ///< Variável int
    varDouble,  ///< Variável double
    varTxt,     ///< Texto (const char*)
    varObj      ///< Referência (TObjeto*) e "NULO" (se TObjeto* = 0)
};

//----------------------------------------------------------------------------
/** Contém as informações necessárias para acessar uma variável
 *  - Usado ao acessar variáveis
 *  - Usado também ao executar funções (a pilha de variáveis)
 */
class TVariavel /// Acesso às variáveis
{
public:
    TVariavel();        ///< Construtor
    void Limpar();      ///< Limpa todos os campos do objeto

// Dados da variável
    static int Tamanho(const char * instr);
        ///< Obtém o tamanho de uma variável na memória
        /**< @param instr Instrução codificada por Instr::Codif
             @return Tamanho da variável (0=não ocupa lugar na memória)
             @note  Se for vetor, retorna o tamanho do vetor na memória */

    int Tamanho();
        ///< Obtém o tamanho de uma variável conforme TVariavel::defvar e TVariavel::vetor

    TVarTipo Tipo();
        ///< Obtém o tipo mais apropriado para expressões numéricas
        /**< Usa  TVariavel::defvar  e  TVariavel::endvar
             @return Tipo de variável */

// Construtor/destrutor/mover
    void Criar(TClasse * c, TObjeto * o);
        ///< Criar variável: acerta dados da variável na memória
        /**< Usa  TVariavel::defvar  e  TVariavel::endvar
             @note Criar uma variável significa:
                - Alocar memória para a variável
                - Chamar TVariavel::Criar()  */

    void Apagar();
        ///< Apagar variável: remove dados da variável na memória
        /**< Usa  TVariavel::defvar  e  TVariavel::endvar
             @note Não libera memória alocada (não executa delete) */

    void Mover(void * destino, TClasse * c, TObjeto * o);
        ///< Move a variável para outra região da memória
        /**< Usa:
             - TVariavel::defvar = definição da variável
             - TVariavel::endvar = endereço atual   */

// Funções get
    bool getBool();         ///< Obtém o valor "bool" da variável
    int getInt();           ///< Obtém o valor "int" da variável
    double getDouble();     ///< Obtém o valor "double" da variável
    const char * getTxt();  ///< Obtém o texto da variável
    TObjeto * getObj();     ///< Obtém a referência da variável

// Funções set
    void setInt(int valor); ///< Muda variável a partir de int
    void setDouble(double valor); ///< Muda variável a partir de double
    void setTxt(const char * txt); ///< Muda variável a partir de texto
    void addTxt(const char * txt); ///< Adiciona texto na variável
    void setObj(TObjeto * obj); ///< Muda variável a partir de referência

// Operadores numéricos
    int Compara(TVariavel * v); ///< Compara com outra variável do mesmo tipo
                        /**< @return -1 se menor, 0 se igual, 1 se maior */
    void Igual(TVariavel * v); ///< Operador igual com variável do mesmo tipo
    bool Func(const char * nome);///< Executa função da variável
                /**< Deve verificar argumentos, após a variável
                    @param nome Nome da função
                    @retval true se processou
                    @retval false se função inexistente */

// Variáveis
    const char * defvar; ///< Instrução que define a variável
                         /**< @sa Instr::Comando */
    union {
        void * endvar;  ///< Endereço da variável na memória
                        /** - É 0 se não for aplicável
                         *  - Em vetores, endereço da primeira variável */
        const void * endfixo;
                    ///< Valor "const" de endvar
                    /**< Usar endfixo quando a variável não poderá ser mudada */
        char * end_char; ///< endvar como char*
        TVarRef * end_varref;        ///< Instr::cRef
        signed   short * end_short;  ///< Instr::cInt16
        unsigned short * end_ushort; ///< Instr::cUInt16
        signed   int * end_int;      ///< Instr::cInt32
        unsigned int * end_uint;     ///< Instr::cUInt32
        TVarIncDec   * end_incdec;   ///< Instr::cIntInc e Instr::cIntDec
        double       * end_double;   ///< Instr::cReal
        TVarIntTempo * end_inttempo; ///< Instr::cIntTempo
        TListaObj    * end_listaobj; ///< Instr::cListaObj
        TListaItem   * end_listaitem;///< Instr::cListaItem
        TVarSocket   * end_socket;   ///< Instr::cSocket
        TVarServ     * end_serv;     ///< Instr::cServ
        TVarNomeObj  * end_nomeobj;  ///< Instr::cNomeObj
        TIndiceObj   * end_indiceobj; ///< Instr::cIndiceObj
        TIndiceItem  * end_indiceitem; ///< Instr::cIndiceItem
        int  valor_int;              ///< Instr::cVarInt - endvar como int
    };
    int  tamanho;   ///< Quantos bytes está usando na memória
                    /**< 0 significa que não está usando ou a variável está
                        sendo usada em outro lugar
                             @note Não é usado em TVariavel */
    unsigned char indice;
        ///< Índice no vetor ou 0 se não for vetor ou 0xFF se for o vetor
    unsigned char bit;  ///< Máscara do bit, se for variável de bit
};

//----------------------------------------------------------------------------

#endif
