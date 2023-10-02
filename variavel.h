#ifndef VARIAVEL_H
#define VARIAVEL_H

#include "instr-enum.h"
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
class TTextoObj;
class TVarArqDir;
class TVarArqProg;
class TVarArqExec;
class TVarArqLog;
class TVarArqTxt;
class TVarArqMem;
class TVarNomeObj;
class TVarTelaTxt;
class TVarSocket;
class TVarServ;
class TVarProg;
class TVarDebug;
class TIndiceObj;
class TIndiceItem;
class TVarDataHora;
class TTextoVarSub;
class TTextoObjSub;
class TVariavel;

//----------------------------------------------------------------------------
/// Tipo de variável
enum TVarTipo : unsigned char
{
    varOutros,  ///< Desconhecido
    varInt,     ///< Variável int
    varDouble,  ///< Variável double
    varTxt,     ///< Texto (const char*)
    varObj      ///< Referência (TObjeto*) e "NULO" (se TObjeto* = 0)
};

//----------------------------------------------------------------------------
/// Informações e funções de cada variável
class TVarInfo
{
private:
    int (*FTamanho)(const char * instr);
    int (*FTamanhoVetor)(const char * instr);
    TVarTipo (*FTipo)(TVariavel * v);
    void (*FRedim)(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    bool (*FFuncVetor)(TVariavel * v, const char * nome);
public:
    /// Construtor usado ao criar TVariavel::VarInfo
    TVarInfo();
    /// Construtor usado nas variáveis da linguagem
    TVarInfo(int (*fTamanho)(const char * instr),
            int (*fTamanhoVetor)(const char * instr),
            TVarTipo (*fTipo)(TVariavel * v),
            void (*fRedim)(TVariavel * v, TClasse * c, TObjeto * o,
                    unsigned int antes, unsigned int depois),
            bool (*fFuncVetor)(TVariavel * v, const char * nome));

    static int FTamanho0(const char * instr);
    static TVarTipo FTipoOutros(TVariavel * v);
    static TVarTipo FTipoInt(TVariavel * v);
    static TVarTipo FTipoDouble(TVariavel * v);
    static TVarTipo FTipoTxt(TVariavel * v);
    static TVarTipo FTipoObj(TVariavel * v);
    static void FRedim0(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static bool FFuncVetorFalse(TVariavel * v, const char * nome);

    friend TVariavel;
};

//----------------------------------------------------------------------------
/** Contém as informações necessárias para acessar uma variável
 *  - Usado ao acessar variáveis
 *  - Usado também ao executar funções (a pilha de variáveis)
 */
class TVariavel /// Acesso às variáveis
{
public:
    static void Inicializa(); ///< Inicializa as variáveis; chamado em main.cpp
    TVariavel();        ///< Construtor
    void Limpar();      ///< Limpa todos os campos do objeto

// Dados da variável

    ///< Obtém o tamanho de uma variável na memória
    /**< @param instr Instrução codificada por Instr::Codif
     *   @return Tamanho da variável (0=não ocupa lugar na memória)
     *   @note  Se for vetor, retorna o tamanho do vetor na memória */
    static inline int Tamanho(const char * instr)
    {
        unsigned char cmd = (unsigned char)instr[2];
        if (cmd < Instr::cTotalComandos)
            return VarInfo[cmd].FTamanho(instr);
        return 0;
    }

    /// Obtém o tamanho do vetor de variáveis conforme TVariavel::defvar
    inline int TamanhoVetor()
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos)
            return VarInfo[cmd].FTamanhoVetor(defvar);
        return 0;
    }

    /// Obtém o tipo mais apropriado para expressões numéricas
    /** Usa  TVariavel::defvar  e  TVariavel::endvar
     *  @return Tipo de variável */
    inline TVarTipo Tipo()
    {
        if (indice == 0xFF) // Vetor
            return varOutros;
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos)
            return VarInfo[cmd].FTipo(this);
        return varOutros;
    }

// Construtor/destrutor/mover

    /// Criar variável: acerta dados da variável na memória
    /** Usa  TVariavel::defvar  e  TVariavel::endvar
     *  @note Criar uma variável significa:
     *    - Alocar memória para a variável
     *    - Chamar TVariavel::Criar()  */
    inline void Criar(TClasse * c, TObjeto * o)
    {
        Redim(c, o, 0, defvar[Instr::endVetor] ?
                (unsigned char)defvar[Instr::endVetor] : 1);
    }

    /// Apagar variável: remove dados da variável na memória
    /** Usa  TVariavel::defvar  e  TVariavel::endvar
     *  @note Não libera memória alocada (não executa delete) */
    inline void Apagar()
    {
        Redim(0, 0, defvar[Instr::endVetor] ?
                (unsigned char)defvar[Instr::endVetor] : 1, 0);
    }

    /// Redimensiona vetor na memória
    /** @param c Classe ao qual a variável pertence, 0 se nenhuma classe
     *  @param o Objeto ao qual a variável pertence, 0 se nenhum objeto
     *  @param antes Tamanho atual do vetor (quantidade de variáveis)
     *  @param depois Novo tamanho do vetor (quantidade de variáveis)
     *  @note Para diminuir o tamanho do vetor, c e o podem ser 0
     *  @note Não libera memória alocada (não executa delete) */
    inline void Redim(TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois)
    {
        if (antes == depois)
            return;
        // Mostra o que vai fazer
        /*
        if (depois > antes)
            printf("Variável criada  (%d a %d) end=%p", antes, depois-1, endvar);
        else
            printf("Variável apagada (%d a %d) end=%p", depois, antes-1, endvar);
        char mens[BUF_MENS];
        if (Instr::Decod(mens, defvar, sizeof(mens)))
            printf(" def=%p %s\n", defvar, mens);
        else
            printf(" ERRO: %s\n", mens);
        fflush(stdout);
        */
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos)
            VarInfo[cmd].FRedim(this, c, o, antes, depois);
        else
            TVarInfo::FRedim0(this, c, o, antes, depois);
    }

    void MoverEnd(void * destino, TClasse * c, TObjeto * o);
        ///< Move a variável para outra região da memória, mas não acerta defvar
        /**< Usa:
             - TVariavel::defvar = definição da variável
             - TVariavel::endvar = endereço atual   */

    void MoverDef();
        ///< Acerta variável porque defvar mudou
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
    int Compara(TVariavel * v);
        ///< Compara com outra variável do mesmo tipo
        /**< @return -1 se menor, 0 se igual, 1 se maior */
    void Igual(TVariavel * v);
        ///< Operador igual com variável do mesmo tipo
    bool Func(const char * nome);
        ///< Executa função da variável
        /**< Deve verificar argumentos, após a variável
         *   @param nome Nome da função
         *   @retval true se processou
         *   @retval false se função inexistente */

// Variáveis
    const char * defvar;
        ///< Instrução que define a variável
        /**< @sa Instr::Comando */
    const char * nomevar;
        ///< Mesmo que defvar, mas só são usados nome da variável e indentação
        /**< @note Usado para obter a variável a partir do nome, nas funções,
         *         e para apagar variáveis locais */
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
        float        * end_float;    ///< Instr::cReal
        double       * end_double;   ///< Instr::cReal2
        TVarArqDir   * end_arqdir;   ///< Instr::cArqDir
        TVarArqLog   * end_arqlog;   ///< Instr::cArqLog
        TVarArqProg  * end_arqprog;  ///< Instr::cArqProg
        TVarArqExec  * end_arqexec;  ///< Instr::cArqExec
        TVarArqTxt   * end_arqtxt;   ///< Instr::cArqTxt
        TVarArqMem   * end_arqmem;   ///< Instr::cArqMem
        TVarIntTempo * end_inttempo; ///< Instr::cIntTempo
        TVarIntExec  * end_intexec;  ///< Instr::cIntExec
        TListaObj    * end_listaobj; ///< Instr::cListaObj
        TListaItem   * end_listaitem;///< Instr::cListaItem
        TTextoTxt    * end_textotxt; ///< Instr::cTextoTxt
        TTextoPos    * end_textopos; ///< Instr::cTextoPos
        TTextoVar    * end_textovar; ///< Instr::cTextoVar
        TTextoObj    * end_textoobj; ///< Instr::cTextoObj
        TVarTelaTxt  * end_telatxt;  ///< Instr::cTelaTxt
        TVarSocket   * end_socket;   ///< Instr::cSocket
        TVarServ     * end_serv;     ///< Instr::cServ
        TVarNomeObj  * end_nomeobj;  ///< Instr::cNomeObj
        TVarProg     * end_prog;     ///< Instr::cProg
        TVarDebug    * end_debug;    ///< Instr::cDebug
        TIndiceObj   * end_indiceobj; ///< Instr::cIndiceObj
        TIndiceItem  * end_indiceitem; ///< Instr::cIndiceItem
        TVarDataHora * end_datahora; ///< Instr::cDataHora
        int  valor_int;              ///< Instr::cVarInt - endvar como int
        TTextoVarSub * end_textovarsub; ///< Instr::cTextoVarSub
        TTextoObjSub * end_textoobjsub; ///< Instr::cTextoObjSub
    };
    int  tamanho;   ///< Quantos bytes está usando na memória
                    /**< 0 significa que não está usando ou a variável está
                         sendo usada em outro lugar
                             @note Não é usado em TVariavel */
    unsigned char indice;
        ///< Índice no vetor ou 0 se não for vetor ou 0xFF se for o vetor
    unsigned char numbit;  ///< Número do primeiro bit de 0 a 7, se int1
    unsigned short numfunc; ///< Para uso da variável; inicialmente é zero

private:
    static TVarInfo * VarInfo;
};

//----------------------------------------------------------------------------

#endif
