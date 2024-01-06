#ifndef VARIAVEL_H
#define VARIAVEL_H

#include "instr-enum.h"
class TClasse;
class TObjeto;
class TVariavel;

// De misc.h
int comparaZ(const char * string1, const char * string2);

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
public:
    /// Um item, para executar funções de uma variável de TVariavel
    struct FuncItem
    {
        const char * Nome;
        bool (*Func)(TVariavel * v);
    };

    /// Construtor usado ao criar TVariavel::VarInfo
    TVarInfo();
    /// Construtor usado nas variáveis da linguagem
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
            FuncItem * FuncListaEnd, // Lista de funções em ordem alfabética
            int FuncListaFim); // Número de elementos de FuncListaEnd menos 1
    /// Retorna um buffer de 0x100 bytes para ser usado para retornar texto
    static inline char * BufferTxt()
    {
        NumBufferTxt = (NumBufferTxt + 0x100) & 0x300;
        return EndBufferTxt + NumBufferTxt;
    }
    /// Compara variável int e retorna como em TVariavel::OperadorCompara
    static unsigned char ComparaInt(int d1, TVariavel * v);
    /// Compara variável double e retorna como em TVariavel::OperadorCompara
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
    /// Um item, para executar funções
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
    /// Lista de funções em letras minúsculas e em ordem alfabética
    FuncItem * FFuncListaEnd;
    /// Número de elementos de FFuncListaEnd menos 1
    int FFuncListaFim;

    static char * EndBufferTxt;
    static unsigned short NumBufferTxt;

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
    /** @note Usa  TVariavel::defvar  e  TVariavel::endvar
     *  @return Tipo de variável */
    inline TVarTipo Tipo()
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff)
            return VarInfo[cmd].FTipo(this);
        return varOutros;
    }

    /// Retorna true se o tipo de variável for int ou double
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
#if 0
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
#endif
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos)
            VarInfo[cmd].FRedim(this, c, o, antes, depois);
        else
            TVarInfo::FRedim0(this, c, o, antes, depois);
    }

    /// Move a variável para outra região da memória, mas não acerta defvar
    /** Usa:
     *    - TVariavel::defvar = definição da variável
     *    - TVariavel::endvar = endereço atual  */
    inline void MoverEnd(void * destino, TClasse * c, TObjeto * o)
    {
        if (destino == endvar)
            return;
#if 0
        printf("Variável movida de %p para %p", endvar, destino);
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

    /// Acerta variável porque defvar mudou
    /** Usa:
     *    - TVariavel::defvar = definição da variável
     *    - TVariavel::endvar = endereço atual   */
    inline void MoverDef()
    {
        unsigned char cmd = (unsigned char)defvar[2];
#if 0
        printf("Variável mudou def end=%p", endvar);
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

// Funções get

    /// Obtém o valor "bool" da variável
    inline bool getBool()
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff)
            return VarInfo[cmd].FGetBool(this);
        return false;
    }

    /// Obtém o valor "int" da variável
    inline int getInt()
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff)
            return VarInfo[cmd].FGetInt(this);
        return 0;
    }

    /// Obtém o valor "double" da variável
    inline double getDouble()
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff)
            return VarInfo[cmd].FGetDouble(this);
        return 0;
    }

    /// Obtém o texto da variável
    inline const char * getTxt()
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff)
            return VarInfo[cmd].FGetTxt(this);
        return "";
    }

    /// Obtém a referência da variável
    inline TObjeto * getObj()
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff)
            return VarInfo[cmd].FGetObj(this);
        return nullptr;
    }

// Operadores

    /// Atribui o valor de uma variável a esta
    inline void OperadorAtrib(TVariavel * v)
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff &&
                v->indice != 0xff)
            VarInfo[cmd].FOperadorAtrib(this, v);
    }

    /// Adiciona uma variável a esta (operador mais igual)
    /** @param v Variável que será adicionada
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

    /// Checa se esta variável é exatamente igual à outra (tipo e valor)
    /** @param v Variável que será comparada
     *  @return true se exatamente igualis, false caso contrário */
    inline bool OperadorIgual2(TVariavel * v)
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff &&
                v->indice != 0xff)
            return VarInfo[cmd].FOperadorIgual2(this, v);
        return false;
    }

    /// Compara esta variável com outra
    /** @param v Variável que será comparada
     *  @retval 1 se esta variável vem antes
     *  @retval 2 se variáveis iguais
     *  @retval 4 se esta variável vem depois
     *  @retval 0 se são apenas diferentes porque não tem como comparar */
    inline unsigned char OperadorCompara(TVariavel * v)
    {
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos && indice != 0xff &&
                v->indice != 0xff)
            return VarInfo[cmd].FOperadorCompara(this, v);
        return 0;
    }

    /// Obtém o número da função (de qualquer variável) a partir do nome
    /** @param nome Nome da função
     *  @return Número da função ou -1 se não existir */
    static int FuncNum(const char * nome);

    /// Executa função da variável
    /** Deve verificar argumentos, após a variável
     *  @param nome Nome da função
     *  @retval true se processou
     *  @retval false se função inexistente ou se retornou false */
    bool FuncExec(int numero)
    {
        if (numero < 0 || numero > VarExecFim)
            return false;
        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd >= Instr::cTotalComandos || indice == 0xFF)
            return false;
        return VarExecEnd[numero].Func[cmd](this);
    }

    /// Executa função da variável
    /** Deve verificar argumentos, após a variável
     *  @param nome Nome da função
     *  @retval true se processou
     *  @retval false se função inexistente ou se retornou false */
    bool FuncExec(const char * nome);

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
                        /** - É nullptr se não for aplicável
                         *  - Em vetores, é o endereço da primeira variável */
        const void * endfixo;
                    ///< Valor "const" de endvar
                    /**< Usar endfixo quando a variável não poderá ser mudada */
        char * endchar; ///< endvar como char*
        int  valor_int;  ///< endvar como int
        double valor_double; ///< endvar como double (8 bytes)
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
    static TVarInfo::FuncExec * VarExecEnd;
    static int VarExecFim;
};

//----------------------------------------------------------------------------
inline unsigned char TVarInfo::ComparaInt(int d1, TVariavel * v)
{
    /*if (v->Tipo() == varInt) // Nos testes, isso é menos eficiente
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
