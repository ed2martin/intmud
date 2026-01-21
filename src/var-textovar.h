#ifndef VAR_TEXTOVAR_H
#define VAR_TEXTOVAR_H

#include "instr.h"

class TVariavel;
class TVarInfo;
enum TVarTipo : unsigned char;
class TTextoVarSub;
class TBlocoVar;

//----------------------------------------------------------------------------
enum TextoVarTipo ///< Tipo de variável
{
    TextoVarTipoTxt, ///< TBlocoVarTxt
    TextoVarTipoNum, ///< TBlocoVarNum
    TextoVarTipoDec, ///< TBlocoVarDec
    TextoVarTipoRef  ///< TBlocoVarRef
};

//----------------------------------------------------------------------------
class TTextoVar  /// Variáveis TextoVar
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    void Apagar();          ///< Apaga objeto
    void Limpar();          ///< Apaga o texto do objeto
    void Mover(TTextoVar * destino);
        ///< Move objeto para outro lugar
    bool CriarTextoVarSub(const char * nome);
        ///< Cria uma variável TTextoVarSub na pilha
    bool CriarTextoVarSub(TBlocoVar * bl);
        ///< Cria uma variável TTextoVarSub na pilha
    TBlocoVar * Procura(const char * texto);
        ///< Procura um objeto TIndiceObj a partir do nome
        /**< @param texto Nome a pesquisar
         *   @return Endereço do objeto, ou 0 se não foi encontrado */
    TBlocoVar * ProcIni(const char * nome);
        ///< Semelhante a Procura(), mas procura o primeiro texto
    TBlocoVar * ProcFim(const char * nome);
        ///< Semelhante a Procura(), mas procura o último texto
    TBlocoVar * ProcAntes(const char * nome);
        ///< Semelhante a Procura(), mas procura o texto anterior
    TBlocoVar * ProcDepois(const char * nome);
        ///< Semelhante a Procura(), mas procura o próximo texto

private:
    static bool FuncValor(TVariavel * v);
    static bool FuncValorIni(TVariavel * v);
    static bool FuncValorFim(TVariavel * v);
    static bool FuncMudar(TVariavel * v);
    static bool FuncNomeVar(TVariavel * v);
    static bool FuncTipo(TVariavel * v);
    static bool FuncAntes(TVariavel * v);
    static bool FuncDepois(TVariavel * v);
    static bool FuncIni(TVariavel * v);
    static bool FuncFim(TVariavel * v);
    static bool FuncLimpar(TVariavel * v);
    static bool FuncTotal(TVariavel * v);

    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static void FRedim(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FMoverEnd(TVariavel * v, void * destino,
            TClasse * c, TObjeto * o);
    static void FOperadorAtrib(TVariavel * v1, TVariavel * v2);
    static bool FOperadorIgual2(TVariavel * v1, TVariavel * v2);
    static unsigned char FOperadorCompara(TVariavel * v1, TVariavel * v2);
    static bool FFuncTexto(TVariavel * v,const char * nome);

public:
    TBlocoVar * RBroot;  ///< Objeto raiz da RBT
    TTextoVarSub * Inicio; ///< Primeiro objeto TTextoVarSub
    int Total;  ///< Quantidade de variáveis

    static TTextoVar * TextoAtual;
                ///< Para obter o endereço da variável mesmo que seja movida
};

//----------------------------------------------------------------------------
class TTextoVarSub /// Para acessar uma variável de TTextoVar
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    void Criar(TTextoVar * var, const char * nome, bool checatipo);
        ///< Adiciona objeto em um TTextoVar
        /**< @param var Variável TextoVar
         *   @param nome Nome da variável de TextoVar
         *   @param checatipo Se deve obter o tipo a partir do nome
         *           da variável */
    void Apagar();
        ///< Retira objeto de um textovar
    void Mover(TTextoVarSub * destino);
        ///< Move bloco para outro lugar

// Lista ligada
    TTextoVar * TextoVar;       ///< A qual textovar pertence
                                /**< Se 0, a variável não é válida */
    TTextoVarSub * Antes;       ///< Objeto anterior
    TTextoVarSub * Depois;      ///< Próximo objeto
    char   NomeVar[VAR_NOME_TAM]; ///< Nome da variável
    TextoVarTipo TipoVar;       ///< Tipo da variável

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

private:
    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static TVarTipo FTipo(TVariavel * v);
    static void FRedim(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FMoverEnd(TVariavel * v, void * destino,
            TClasse * c, TObjeto * o);
    static bool FGetBool(TVariavel * v);
    static int FGetInt(TVariavel * v);
    static double FGetDouble(TVariavel * v);
    static const char * FGetTxt(TVariavel * v);
    static TObjeto * FGetObj(TVariavel * v);
    static void FOperadorAtrib(TVariavel * v1, TVariavel * v2);
    static bool FOperadorAdd(TVariavel * v1, TVariavel * v2);
    static bool FOperadorIgual2(TVariavel * v1, TVariavel * v2);
    static unsigned char FOperadorCompara(TVariavel * v1, TVariavel * v2);
};

//----------------------------------------------------------------------------
class TBlocoVar /// Variável de um objeto TTextoVar
{
public:
    TBlocoVar(TTextoVar * var, const char * nome, const char * texto=0);
        ///< Construtor
        /**< @param var Objeto TTextoVar que contém essa variável
         *   @param nome Nome da variável
         *   @param texto Texto que será colocado após o nome da variável */
    virtual ~TBlocoVar();
        ///< Destrutor
    void MoveTextoVar(TTextoVar * textovar);
        ///< Usado por TTextoVar::Mover, para mudar TBlocoVar::TextoVar
    virtual const char * Tipo() = 0;
        ///< Caracter após o nome da variável que indica o tipo
    virtual TextoVarTipo TipoVar() = 0;
        ///< Tipo de variável

// Funções get
    virtual bool   getBool()=0;           ///< Retorna valor como bool
    virtual int    getInt()=0;            ///< Retorna valor como int
    virtual double getDouble()=0;         ///< Retorna valor como double
    virtual const char * getTxt()=0;      ///< Retorna valor como texto
    virtual TObjeto * getObj()=0;         ///< Obtém a referência da variável

// Funções set
    virtual void setInt(int valor)=0;   ///< Muda valor como int
    virtual void setDouble(double valor)=0; ///< Muda valor como double
    virtual void setTxt(const char * txt)=0; ///< Muda valor como texto
    virtual void addTxt(const char * txt)=0; ///< Adiciona texto
    virtual void setObj(TObjeto * obj)=0; ///< Muda variável a partir de referência

// Árvore organizada por TBlocoVar::NomeVar
    void RBinsert(void);       ///< Insere objeto na RBT
    void RBremove(void);       ///< Remove objeto da RBT
    TBlocoVar * RBfirst(void); ///< Primeiro objeto da RBT
    TBlocoVar * RBlast(void);  ///< Último objeto da RBT
    static TBlocoVar * RBnext(TBlocoVar *); ///< Próximo objeto da RBT
    static TBlocoVar * RBprevious(TBlocoVar *); ///< Objeto anterior da RBT
    static int RBcomp(TBlocoVar * x, TBlocoVar * y); ///< Compara objetos
    void RBleft_rotate(void);
    void RBright_rotate(void);

    //static TBlocoVar * RBroot;  ///< Objeto raiz
    TBlocoVar *RBparent;    ///< Objeto objeto pai
    TBlocoVar *RBleft,*RBright; ///< Objetos filhos
    TTextoVar *TextoVar;    ///< Variável TextoVar que contém o bloco (RBT)
    const char * NomeVar;   ///< Nome da variável
    unsigned char RBcolour; ///< Bit 0=cor, bit 1: 0=RBT, 1=somente texto
    unsigned short Texto;   ///< NomeVar+Texto = texto após o nome da variável
};

//----------------------------------------------------------------------------
class TBlocoVarTxt : public TBlocoVar // Variável de texto
{
public:
    TBlocoVarTxt(TTextoVar * var, const char * nome, const char * texto);
    virtual ~TBlocoVarTxt();
    const char * Tipo() { return " "; }
    TextoVarTipo TipoVar() { return TextoVarTipoTxt; }

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
};

//----------------------------------------------------------------------------
class TBlocoVarNum : public TBlocoVar // Variável numérica
{
public:
    TBlocoVarNum(TTextoVar * var, const char * nome, double valor);
    virtual ~TBlocoVarNum();
    const char * Tipo() { return "_"; }
    TextoVarTipo TipoVar() { return TextoVarTipoNum; }

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
private:
    double ValorDouble;
};

//----------------------------------------------------------------------------
class TBlocoVarDec : public TBlocoVar // Variável intdec
{
public:
    static void PreparaIni();
    static void ProcEventos(int TempoDecorrido);
    TBlocoVarDec(TTextoVar * var, const char * nome, int valor);
    virtual ~TBlocoVarDec();
    const char * Tipo() { return "@"; }
    TextoVarTipo TipoVar() { return TextoVarTipoDec; }

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

private:
    void InsereLista(int valor);///< Adiciona na lista ligada
    void RemoveLista();           ///< Remove da lista ligada
    unsigned short IndiceMenos;   ///< Índice em VetMenos
    unsigned short IndiceMais;    ///< Índice em VetMais
    TBlocoVarDec * Antes;         ///< Objeto anterior na lista ligada
    TBlocoVarDec * Depois;        ///< Próximo objeto na lista ligada

    static unsigned int TempoMenos;  ///< Contagem de tempo com VetMenos
    static unsigned int TempoMais;   ///< Contagem de tempo com VetMais
    static TBlocoVarDec ** VetMenos; ///< Primeira lista ligada
    static TBlocoVarDec ** VetMais;  ///< Segunda lista ligada
};

//----------------------------------------------------------------------------
class TBlocoVarRef : public TBlocoVar // Variável ref
{
public:
    TBlocoVarRef(TTextoVar * var, const char * nome, TObjeto * obj);
    virtual ~TBlocoVarRef();
    const char * Tipo() { return "$"; }
    TextoVarTipo TipoVar() { return TextoVarTipoRef; }

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

    void InsereLista(TObjeto * obj); ///< Adiciona objeto na lista ligada
    void RemoveLista();        ///< Remove objeto da lista ligada
    TObjeto * Objeto;          ///< Objeto que está na lista
    TBlocoVarRef * ObjAntes;   ///< Objeto anterior da lista
    TBlocoVarRef * ObjDepois;  ///< Próximo objeto da lista
};

//----------------------------------------------------------------------------
#endif
