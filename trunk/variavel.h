#ifndef VARIAVEL_H
#define VARIAVEL_H

// Acima desse valor, double � mostrado com nota��o cient�fica
#define DOUBLE_MAX 1000000000000000000.0

class TClasse;
class TObjeto;
class TVarRef;
class TVarIncDec;
class TVarIntTempo;
class TVarIntExec;
class TListaObj;
class TListaItem;
class TTextoTxt;
class TTextoPos;
class TTextoVar;
class TVarDir;
class TVarLog;
class TVarTxt;
class TVarNomeObj;
class TVarTelaTxt;
class TVarSocket;
class TVarServ;
class TVarProg;
class TIndiceObj;
class TIndiceItem;
class TVarDataHora;
class TTextoVarSub;

//----------------------------------------------------------------------------
/// Tipo de vari�vel
enum TVarTipo
{
    varOutros,  ///< Desconhecido
    varInt,     ///< Vari�vel int
    varDouble,  ///< Vari�vel double
    varTxt,     ///< Texto (const char*)
    varObj      ///< Refer�ncia (TObjeto*) e "NULO" (se TObjeto* = 0)
};

//----------------------------------------------------------------------------
/** Cont�m as informa��es necess�rias para acessar uma vari�vel
 *  - Usado ao acessar vari�veis
 *  - Usado tamb�m ao executar fun��es (a pilha de vari�veis)
 */
class TVariavel /// Acesso �s vari�veis
{
public:
    TVariavel();        ///< Construtor
    void Limpar();      ///< Limpa todos os campos do objeto

// Dados da vari�vel
    static int Tamanho(const char * instr);
        ///< Obt�m o tamanho de uma vari�vel na mem�ria
        /**< @param instr Instru��o codificada por Instr::Codif
             @return Tamanho da vari�vel (0=n�o ocupa lugar na mem�ria)
             @note  Se for vetor, retorna o tamanho do vetor na mem�ria */

    int Tamanho();
        ///< Obt�m o tamanho de uma vari�vel conforme TVariavel::defvar e TVariavel::vetor

    TVarTipo Tipo();
        ///< Obt�m o tipo mais apropriado para express�es num�ricas
        /**< Usa  TVariavel::defvar  e  TVariavel::endvar
             @return Tipo de vari�vel */

// Construtor/destrutor/mover
    void Criar(TClasse * c, TObjeto * o);
        ///< Criar vari�vel: acerta dados da vari�vel na mem�ria
        /**< Usa  TVariavel::defvar  e  TVariavel::endvar
             @note Criar uma vari�vel significa:
                - Alocar mem�ria para a vari�vel
                - Chamar TVariavel::Criar()  */

    void Apagar();
        ///< Apagar vari�vel: remove dados da vari�vel na mem�ria
        /**< Usa  TVariavel::defvar  e  TVariavel::endvar
             @note N�o libera mem�ria alocada (n�o executa delete) */

    void Redim(TClasse * c, TObjeto * o, unsigned int antes, unsigned int depois);
        ///< Redimensiona vetor na mem�ria
        /**< @param c Classe ao qual a vari�vel pertence, 0 se nenhuma classe
             @param o Objeto ao qual a vari�vel pertence, 0 se nenhum objeto
             @param antes Tamanho atual do vetor (quantidade de vari�veis)
             @param depois Novo tamanho do vetor (quantidade de vari�veis)
             @note Para diminuir o tamanho do vetor, c e o podem ser 0
             @note N�o libera mem�ria alocada (n�o executa delete) */

    void MoverEnd(void * destino, TClasse * c, TObjeto * o);
        ///< Move a vari�vel para outra regi�o da mem�ria, mas n�o acerta defvar
        /**< Usa:
             - TVariavel::defvar = defini��o da vari�vel
             - TVariavel::endvar = endere�o atual   */

    void MoverDef();
        ///< Acerta vari�vel porque defvar mudou
        /**< Usa:
             - TVariavel::defvar = defini��o da vari�vel
             - TVariavel::endvar = endere�o atual   */

// Fun��es get
    bool getBool();         ///< Obt�m o valor "bool" da vari�vel
    int getInt();           ///< Obt�m o valor "int" da vari�vel
    double getDouble();     ///< Obt�m o valor "double" da vari�vel
    const char * getTxt();  ///< Obt�m o texto da vari�vel
    TObjeto * getObj();     ///< Obt�m a refer�ncia da vari�vel

// Fun��es set
    void setInt(int valor); ///< Muda vari�vel a partir de int
    void setDouble(double valor); ///< Muda vari�vel a partir de double
    void setTxt(const char * txt); ///< Muda vari�vel a partir de texto
    void addTxt(const char * txt); ///< Adiciona texto na vari�vel
    void setObj(TObjeto * obj); ///< Muda vari�vel a partir de refer�ncia

// Operadores num�ricos
    int Compara(TVariavel * v); ///< Compara com outra vari�vel do mesmo tipo
                        /**< @return -1 se menor, 0 se igual, 1 se maior */
    void Igual(TVariavel * v); ///< Operador igual com vari�vel do mesmo tipo
    bool Func(const char * nome);///< Executa fun��o da vari�vel
                /**< Deve verificar argumentos, ap�s a vari�vel
                    @param nome Nome da fun��o
                    @retval true se processou
                    @retval false se fun��o inexistente */

// Vari�veis
    const char * defvar;
        ///< Instru��o que define a vari�vel
        /**< @sa Instr::Comando */
    const char * nomevar;
        ///< Mesmo que defvar, mas s� s�o usados nome da vari�vel e indenta��o
        /**< @note Usado para obter a vari�vel a partir do nome, nas fun��es,
         *         e para apagar vari�veis locais */
    union {
        void * endvar;  ///< Endere�o da vari�vel na mem�ria
                        /** - � 0 se n�o for aplic�vel
                         *  - Em vetores, endere�o da primeira vari�vel */
        const void * endfixo;
                    ///< Valor "const" de endvar
                    /**< Usar endfixo quando a vari�vel n�o poder� ser mudada */
        char * end_char; ///< endvar como char*
        TVarRef * end_varref;        ///< Instr::cRef
        signed   short * end_short;  ///< Instr::cInt16
        unsigned short * end_ushort; ///< Instr::cUInt16
        signed   int * end_int;      ///< Instr::cInt32
        unsigned int * end_uint;     ///< Instr::cUInt32
        TVarIncDec   * end_incdec;   ///< Instr::cIntInc e Instr::cIntDec
        double       * end_double;   ///< Instr::cReal
        TVarDir      * end_dir;      ///< Instr::cArqDir
        TVarLog      * end_log;      ///< Instr::cArqLog
        TVarTxt      * end_txt;      ///< Instr::cArqTxt
        TVarIntTempo * end_inttempo; ///< Instr::cIntTempo
        TVarIntExec  * end_intexec;  ///< Instr::cIntExec
        TListaObj    * end_listaobj; ///< Instr::cListaObj
        TListaItem   * end_listaitem;///< Instr::cListaItem
        TTextoTxt    * end_textotxt; ///< Instr::cTextoTxt
        TTextoPos    * end_textopos; ///< Instr::cTextoPos
        TTextoVar    * end_textovar; ///< Instr::cTextoVar
        TVarTelaTxt  * end_telatxt;  ///< Instr::cTelaTxt
        TVarSocket   * end_socket;   ///< Instr::cSocket
        TVarServ     * end_serv;     ///< Instr::cServ
        TVarNomeObj  * end_nomeobj;  ///< Instr::cNomeObj
        TVarProg     * end_prog;     ///< Instr::cProg
        TIndiceObj   * end_indiceobj; ///< Instr::cIndiceObj
        TIndiceItem  * end_indiceitem; ///< Instr::cIndiceItem
        TVarDataHora * end_datahora; ///< Instr::cDataHora
        int  valor_int;              ///< Instr::cVarInt - endvar como int
        TTextoVarSub * end_textovarsub; ///< Instr::cTextoVarSub
    };
    int  tamanho;   ///< Quantos bytes est� usando na mem�ria
                    /**< 0 significa que n�o est� usando ou a vari�vel est�
                         sendo usada em outro lugar
                             @note N�o � usado em TVariavel */
    unsigned char indice;
        ///< �ndice no vetor ou 0 se n�o for vetor ou 0xFF se for o vetor
    unsigned char bit;  ///< M�scara do bit, se for vari�vel de bit
    unsigned short numfunc; ///< Para uso da vari�vel; inicialmente � zero
};

//----------------------------------------------------------------------------

#endif
