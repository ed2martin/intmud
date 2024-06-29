#ifndef VARIAVEL_H
#define VARIAVEL_H

#include "instr-enum.h"
class TClasse;
class TObjeto;
class TVariavel;

// De misc.h
int comparaZ(const char * string1, const char * string2);

//----------------------------------------------------------------------------
/// Tipo de vari�vel
enum TVarTipo : unsigned char
{
    varOutros,  ///< Desconhecido
    varInt,     ///< Vari�vel int
    varDouble,  ///< Vari�vel double
    varTxt,     ///< Texto (const char*)
    varObj      ///< Refer�ncia (TObjeto*) e "NULO" (se TObjeto* = 0)
};

//----------------------------------------------------------------------------
/// Informa��es e fun��es de cada vari�vel
class TVarInfo
{
public:
    /// Um item, para executar fun��es de uma vari�vel de TVariavel
    struct FuncItem
    {
        const char * Nome;
        bool (*Func)(TVariavel * v);
    };

    /// Construtor usado ao criar TVariavel::VarInfo
    TVarInfo();
    /// Construtor usado nas vari�veis da linguagem
    TVarInfo(int (*fTamanho)(const char * instr),
            int (*fTamanhoVetor)(const char * instr),
            TVarTipo (*fTipo)(TVariavel * v),
            void (*fRedim)(TVariavel * v, TClasse * c, TObjeto * o,
                    unsigned int antes, unsigned int depois),
            void (*fMoverEnd)(TVariavel * v, void * destino,
                    TClasse * c, TObjeto * o),
            void (*fMoverDef)(TVariavel * v),
            bool (*fGetBool)(TVariavel * v),
            int (*fGetInt)(TVariavel * v),
            double (*fGetDouble)(TVariavel * v),
            const char * (*fGetTxt)(TVariavel * v),
            TObjeto * (*fGetObj)(TVariavel * v),
            void (*fOperadorAtrib)(TVariavel * v1, TVariavel * v2),
            bool (*fOperadorAdd)(TVariavel * v1, TVariavel * v2),
            bool (*fOperadorIgual2)(TVariavel * v1, TVariavel * v2),
            unsigned char (*fOperadorCompara)(TVariavel * v1, TVariavel * v2),
            bool (*fFuncTexto)(TVariavel * v, const char * nome),
            bool (*fFuncVetor)(TVariavel * v, const char * nome),
            FuncItem * FuncListaEnd, // Lista de fun��es em ordem alfab�tica
            int FuncListaFim); // N�mero de elementos de FuncListaEnd menos 1
    /// Retorna um buffer de 0x100 bytes para ser usado para retornar texto
    static inline char * BufferTxt()
    {
        NumBufferTxt = (NumBufferTxt + 0x100) & 0x300;
        return EndBufferTxt + NumBufferTxt;
    }
    /// Compara vari�vel int e retorna como em TVariavel::OperadorCompara
    static unsigned char ComparaInt(int d1, TVariavel * v);
    /// Compara vari�vel double e retorna como em TVariavel::OperadorCompara
    static unsigned char ComparaDouble(double d1, TVariavel * v);
    /// Compara dois textos e retorna como em TVariavel::OperadorCompara
    static unsigned char ComparaTxt(const char * t1, const char * t2);

    static int FTamanho0(const char * instr);
    static TVarTipo FTipoOutros(TVariavel * v);
    static TVarTipo FTipoInt(TVariavel * v);
    static TVarTipo FTipoDouble(TVariavel * v);
    static TVarTipo FTipoTxt(TVariavel * v);
    static TVarTipo FTipoObj(TVariavel * v);
    static void FRedim0(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FMoverEnd0(TVariavel * v, void * destino,
            TClasse * c, TObjeto * o);
    static void FMoverDef0(TVariavel * v);
    static bool FGetBoolFalse(TVariavel * v);
    static int FGetInt0(TVariavel * v);
    static double FGetDouble0(TVariavel * v);
    static const char * FGetTxtVazio(TVariavel * v);
    static TObjeto * FGetObjNulo(TVariavel * v);
    static void FOperadorAtribVazio(TVariavel * v1, TVariavel * v2);
    static bool FOperadorAddFalse(TVariavel * v1, TVariavel * v2);
    static bool FOperadorIgual2Var(TVariavel * v1, TVariavel * v2);
    static unsigned char FOperadorComparaVar(TVariavel * v1, TVariavel * v2);
    static bool FFuncTextoFalse(TVariavel * v, const char * nome);
    static bool FFuncVetorFalse(TVariavel * v, const char * nome);
    static bool FFuncFalse(TVariavel * v);

private:
    /// Um item, para executar fun��es
    struct FuncExec
    {
        const char * Nome;
        bool (*Func[Instr::cTotalComandos])(TVariavel * v);
    };

    int (*FTamanho)(const char * instr);
    int (*FTamanhoVetor)(const char * instr);
    TVarTipo (*FTipo)(TVariavel * v);
    void (*FRedim)(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    void (*FMoverEnd)(TVariavel * v, void * destino, TClasse * c, TObjeto * o);
    void (*FMoverDef)(TVariavel * v);
    bool (*FGetBool)(TVariavel * v);
    int (*FGetInt)(TVariavel * v);
    double (*FGetDouble)(TVariavel * v);
    const char * (*FGetTxt)(TVariavel * v);
    TObjeto * (*FGetObj)(TVariavel * v);
    void (*FOperadorAtrib)(TVariavel * v1, TVariavel * v2);
    bool (*FOperadorAdd)(TVariavel * v1, TVariavel * v2);
    bool (*FOperadorIgual2)(TVariavel * v1, TVariavel * v2);
    unsigned char (*FOperadorCompara)(TVariavel * v1, TVariavel * v2);
    bool (*FFuncTexto)(TVariavel * v, const char * nome);
    bool (*FFuncVetor)(TVariavel * v, const char * nome);
    /// Lista de fun��es em letras min�sculas e em ordem alfab�tica
    FuncItem * FFuncListaEnd;
    /// N�mero de elementos de FFuncListaEnd menos 1
    int FFuncListaFim;

    static char * EndBufferTxt;
    static unsigned short NumBufferTxt;

    friend class TVariavel;
};

//----------------------------------------------------------------------------
/** Cont�m as informa��es necess�rias para acessar uma vari�vel
 *  - Usado ao acessar vari�veis
 *  - Usado tamb�m ao executar fun��es (a pilha de vari�veis)
 */
class TVariavel /// Acesso �s vari�veis
{
public:
    static void Inicializa(); ///< Inicializa as vari�veis; chamado em main.cpp
    TVariavel();        ///< Construtor
    void Limpar();      ///< Limpa todos os campos do objeto

// Dados da vari�vel

    ///< Obt�m o tamanho de uma vari�vel na mem�ria
    /**< @param instr Instru��o codificada por Instr::Codif
     *   @return Tamanho da vari�vel (0=n�o ocupa lugar na mem�ria)
     *   @note  Se for vetor, retorna o tamanho do vetor na mem�ria */
    static inline int Tamanho(const char * instr)
    {
        unsigned char cmd = (unsigned char)instr[2];
        if (cmd < Instr::cTotalComandos)
            return VarInfo[cmd].FTamanho(instr);
        return 0;
    }

    /// Obt�m o tamanho do vetor de vari�veis conforme TVariavel::defvar
    inline int TamanhoVetor()
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos)
            return VarInfo[cmd].FTamanhoVetor(defvar);
        return 0;
    }

    /// Obt�m o tipo mais apropriado para express�es num�ricas
    /** @note Usa  TVariavel::defvar  e  TVariavel::endvar
     *  @return Tipo de vari�vel */
    inline TVarTipo Tipo()
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff)
            return VarInfo[cmd].FTipo(this);
        return varOutros;
    }

    /// Retorna true se o tipo de vari�vel for int ou double
    inline bool TipoNumerico()
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff)
        {
            TVarTipo tipo = VarInfo[cmd].FTipo(this);
            return tipo == varInt || tipo == varDouble;
        }
        return false;
    }

// Construtor/destrutor/mover

    /// Criar vari�vel: acerta dados da vari�vel na mem�ria
    /** Usa  TVariavel::defvar  e  TVariavel::endvar
     *  @note Criar uma vari�vel significa:
     *    - Alocar mem�ria para a vari�vel
     *    - Chamar TVariavel::Criar()  */
    inline void Criar(TClasse * c, TObjeto * o)
    {
        Redim(c, o, 0, defvar[Instr::endVetor] ?
                (unsigned char)defvar[Instr::endVetor] : 1);
    }

    /// Apagar vari�vel: remove dados da vari�vel na mem�ria
    /** Usa  TVariavel::defvar  e  TVariavel::endvar
     *  @note N�o libera mem�ria alocada (n�o executa delete) */
    inline void Apagar()
    {
        Redim(0, 0, defvar[Instr::endVetor] ?
                (unsigned char)defvar[Instr::endVetor] : 1, 0);
    }

    /// Redimensiona vetor na mem�ria
    /** @param c Classe ao qual a vari�vel pertence, 0 se nenhuma classe
     *  @param o Objeto ao qual a vari�vel pertence, 0 se nenhum objeto
     *  @param antes Tamanho atual do vetor (quantidade de vari�veis)
     *  @param depois Novo tamanho do vetor (quantidade de vari�veis)
     *  @note Para diminuir o tamanho do vetor, c e o podem ser 0
     *  @note N�o libera mem�ria alocada (n�o executa delete) */
    inline void Redim(TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois)
    {
        if (antes == depois)
            return;
        // Mostra o que vai fazer
#if 0
        if (depois > antes)
            printf("Vari�vel criada  (%d a %d) end=%p", antes, depois-1, endvar);
        else
            printf("Vari�vel apagada (%d a %d) end=%p", depois, antes-1, endvar);
        char mens[BUF_MENS];
        if (Instr::Decod(mens, defvar, sizeof(mens)))
            printf(" def=%p %s\n", defvar, mens);
        else
            printf(" ERRO: %s\n", mens);
        fflush(stdout);
#endif
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos)
            VarInfo[cmd].FRedim(this, c, o, antes, depois);
        else
            TVarInfo::FRedim0(this, c, o, antes, depois);
    }

    /// Move a vari�vel para outra regi�o da mem�ria, mas n�o acerta defvar
    /** Usa:
     *    - TVariavel::defvar = defini��o da vari�vel
     *    - TVariavel::endvar = endere�o atual  */
    inline void MoverEnd(void * destino, TClasse * c, TObjeto * o)
    {
        if (destino == endvar)
            return;
#if 0
        printf("Vari�vel movida de %p para %p", endvar, destino);
        char mens1[BUF_MENS];
        if (Instr::Decod(mens1, defvar, sizeof(mens1)))
            printf(" def=%p %s\n", defvar, mens1);
        else
            printf(" ERRO: %s\n", mens1);
        fflush(stdout);
#endif
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos)
            VarInfo[cmd].FMoverEnd(this, destino, c, o);
        endvar = destino;
    }

    /// Acerta vari�vel porque defvar mudou
    /** Usa:
     *    - TVariavel::defvar = defini��o da vari�vel
     *    - TVariavel::endvar = endere�o atual   */
    inline void MoverDef()
    {
        unsigned char cmd = (unsigned char)defvar[2];
#if 0
        printf("Vari�vel mudou def end=%p", endvar);
        char mens1[BUF_MENS];
        if (Instr::Decod(mens1, defvar, sizeof(mens1)))
            printf(" def=%p %s\n", defvar, mens1);
        else
            printf(" ERRO: %s\n", mens1);
        fflush(stdout);
#endif
        if (cmd < Instr::cTotalComandos)
            VarInfo[cmd].FMoverDef(this);
    }

// Fun��es get

    /// Obt�m o valor "bool" da vari�vel
    inline bool getBool()
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff)
            return VarInfo[cmd].FGetBool(this);
        return false;
    }

    /// Obt�m o valor "int" da vari�vel
    inline int getInt()
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff)
            return VarInfo[cmd].FGetInt(this);
        return 0;
    }

    /// Obt�m o valor "double" da vari�vel
    inline double getDouble()
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff)
            return VarInfo[cmd].FGetDouble(this);
        return 0;
    }

    /// Obt�m o texto da vari�vel
    inline const char * getTxt()
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff)
            return VarInfo[cmd].FGetTxt(this);
        return "";
    }

    /// Obt�m a refer�ncia da vari�vel
    inline TObjeto * getObj()
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff)
            return VarInfo[cmd].FGetObj(this);
        return nullptr;
    }

// Operadores

    /// Atribui o valor de uma vari�vel a esta
    inline void OperadorAtrib(TVariavel * v)
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff &&
                v->indice != 0xff)
            VarInfo[cmd].FOperadorAtrib(this, v);
    }

    /// Adiciona uma vari�vel a esta (operador mais igual)
    /** @param v Vari�vel que ser� adicionada
     *  @return true se adicionou, false se deve somar da forma convencional
     *  @note Costuma ser usado somente para adicionar texto */
    inline bool OperadorAdd(TVariavel * v)
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff &&
                v->indice != 0xff)
            return VarInfo[cmd].FOperadorAdd(this, v);
        return false;
    }

    /// Checa se esta vari�vel � exatamente igual � outra (tipo e valor)
    /** @param v Vari�vel que ser� comparada
     *  @return true se exatamente igualis, false caso contr�rio */
    inline bool OperadorIgual2(TVariavel * v)
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff &&
                v->indice != 0xff)
            return VarInfo[cmd].FOperadorIgual2(this, v);
        return false;
    }

    /// Compara esta vari�vel com outra
    /** @param v Vari�vel que ser� comparada
     *  @retval 1 se esta vari�vel vem antes
     *  @retval 2 se vari�veis iguais
     *  @retval 4 se esta vari�vel vem depois
     *  @retval 0 se s�o apenas diferentes porque n�o tem como comparar */
    inline unsigned char OperadorCompara(TVariavel * v)
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff &&
                v->indice != 0xff)
            return VarInfo[cmd].FOperadorCompara(this, v);
        return 0;
    }

    /// Obt�m o n�mero da fun��o (de qualquer vari�vel) a partir do nome
    /** @param nome Nome da fun��o
     *  @return N�mero da fun��o ou -1 se n�o existir */
    static int FuncNum(const char * nome);

    /// Executa fun��o da vari�vel
    /** Deve verificar argumentos, ap�s a vari�vel
     *  @param nome Nome da fun��o
     *  @retval true se processou
     *  @retval false se fun��o inexistente ou se retornou false */
    bool FuncExec(int numero)
    {
        if (numero < 0 || numero > VarExecFim)
            return false;
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd >= Instr::cTotalComandos || indice == 0xFF)
            return false;
        return VarExecEnd[numero].Func[cmd](this);
    }

    /// Executa fun��o da vari�vel
    /** Deve verificar argumentos, ap�s a vari�vel
     *  @param nome Nome da fun��o
     *  @retval true se processou
     *  @retval false se fun��o inexistente ou se retornou false */
    bool FuncExec(const char * nome);

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
                        /** - � nullptr se n�o for aplic�vel
                         *  - Em vetores, � o endere�o da primeira vari�vel */
        const void * endfixo;
                    ///< Valor "const" de endvar
                    /**< Usar endfixo quando a vari�vel n�o poder� ser mudada */
        char * endchar; ///< endvar como char*
        int  valor_int;  ///< endvar como int
        double valor_double; ///< endvar como double (8 bytes)
    };
    int  tamanho;   ///< Quantos bytes est� usando na mem�ria
                    /**< 0 significa que n�o est� usando ou a vari�vel est�
                         sendo usada em outro lugar
                             @note N�o � usado em TVariavel */
    unsigned char indice;
        ///< �ndice no vetor ou 0 se n�o for vetor ou 0xFF se for o vetor
    unsigned char numbit;  ///< N�mero do primeiro bit de 0 a 7, se int1
    unsigned short numfunc; ///< Para uso da vari�vel; inicialmente � zero

private:
    static TVarInfo * VarInfo;
    static TVarInfo::FuncExec * VarExecEnd;
    static int VarExecFim;
};

//----------------------------------------------------------------------------
inline unsigned char TVarInfo::ComparaInt(int d1, TVariavel * v)
{
    /*if (v->Tipo() == varInt) // Nos testes, isso � menos eficiente
    {
        int d2 = v->getInt();
        return d1 == d2 ? 2 : d1 < d2 ? 1 : 4;
    }*/
    double d2 = v->getDouble();
    unsigned char result = 0;
    if (d1 < d2) result |= 1;
    if (d1 == d2) result |= 2;
    if (d1 > d2) result |= 4;
    return result;
}
inline unsigned char TVarInfo::ComparaDouble(double d1, TVariavel * v)
{
    double d2 = v->getDouble();
    unsigned char result = 0;
    if (d1 < d2) result |= 1;
    if (d1 == d2) result |= 2;
    if (d1 > d2) result |= 4;
    return result;
}
inline unsigned char TVarInfo::ComparaTxt(const char * t1, const char * t2)
{
    int cmp = comparaZ(t1, t2);
    return (cmp == 0 ? 2 : cmp < 0 ? 1 : 4);
}

//----------------------------------------------------------------------------

#endif
