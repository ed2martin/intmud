#ifndef VAR_INDICEOBJ_H
#define VAR_INDICEOBJ_H

class TVariavel;
class TVarInfo;
class TClasse;
class TObjeto;
class TIndiceObj;

//----------------------------------------------------------------------------
class TIndiceItem /// Vari�vel IndiceItem
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa vari�vel e retorna informa��es
private:
    void Apagar();          ///< Apaga objeto
    void Mover(TIndiceItem * destino); ///< Move para outro lugar
    int  getValor();        ///< Ler valor num�rico da vari�vel
    TIndiceObj * getIndiceObj(); ///< Retorna IndiceObj
    void MudarRef(TIndiceObj * indice); ///< Muda refer�ncia de ListaX

    static bool FuncObj(TVariavel * v);
    static bool FuncTxt(TVariavel * v);
    static bool FuncAntes(TVariavel * v);
    static bool FuncDepois(TVariavel * v);
    static bool FuncIni(TVariavel * v);
    static bool FuncFim(TVariavel * v);

    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static void FRedim(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FMoverEnd(TVariavel * v, void * destino,
            TClasse * c, TObjeto * o);
    static bool FGetBool(TVariavel * v);
    static int FGetInt(TVariavel * v);
    static double FGetDouble(TVariavel * v);
    static const char * FGetTxt(TVariavel * v);
    static void FOperadorAtrib(TVariavel * v1, TVariavel * v2);
    static bool FOperadorIgual2(TVariavel * v1, TVariavel * v2);
    static unsigned char FOperadorCompara(TVariavel * v1, TVariavel * v2);

    TIndiceItem * Antes;    ///< Objeto anterior
    TIndiceItem * Depois;   ///< Pr�ximo objeto
    TIndiceObj * IndiceObj; ///< Qual vari�vel est� apontando
    int  TamTxt;            ///< Tamanho do texto ao avan�ar/voltar objeto

    friend class TIndiceObj;
};

//----------------------------------------------------------------------------
class TIndiceObj /// Vari�vel IndiceObj
{
public:
    static const TVarInfo * Inicializa();
private:
        ///< Inicializa vari�vel e retorna informa��es
    void Apagar();          ///< Apaga objeto
    void Mover(TIndiceObj * destino, TObjeto * obj); ///< Move para outro lugar
    static TIndiceObj * Procura(const char * nome);
        ///< Procura um objeto TIndiceObj a partir do nome
        /**<
         *  @param nome Nome a pesquisar
         *  @return Endere�o do objeto, ou 0 se n�o foi encontrado */
    static TIndiceObj * ProcIni(const char * nome);
        ///< Semelhante a Procura(), mas procura o primeiro texto
    static TIndiceObj * ProcFim(const char * nome);
        ///< Semelhante a Procura(), mas procura o �ltimo texto

    TObjeto * Objeto;       ///< Objeto que cont�m a vari�vel

// �rvore organizada por TIndiceObj::Nome
// Se *Nome==0, objeto n�o est� na �rvore
    void RBinsert(void);        ///< Insere objeto na RBT
    void RBremove(void);        ///< Remove objeto da RBT
    static TIndiceObj * RBfirst(void); ///< Primeiro objeto da RBT
    static TIndiceObj * RBlast(void);  ///< �ltimo objeto da RBT
    static TIndiceObj * RBnext(TIndiceObj *); ///< Pr�ximo objeto da RBT
    static TIndiceObj * RBprevious(TIndiceObj *); ///< Objeto anterior da RBT
    static TIndiceObj * RBroot;  ///< Objeto raiz
    TIndiceObj *RBleft,*RBright,*RBparent; ///< Objetos filhos e objeto pai
    void RBleft_rotate(void);
    void RBright_rotate(void);
    static int RBcomp(TIndiceObj * x, TIndiceObj * y); ///< Compara objetos
    unsigned char RBcolour;

    char Nome[65];          ///< Texto da vari�vel
    TIndiceItem * IndiceItem; ///< Primeiro indiceitem apontando para esse objeto

    void setNome(const char * texto); ///< Muda o texto

    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static void FRedim(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FMoverEnd(TVariavel * v, void * destino,
            TClasse * c, TObjeto * o);
    static bool FGetBool(TVariavel * v);
    static int FGetInt(TVariavel * v);
    static double FGetDouble(TVariavel * v);
    static const char * FGetTxt(TVariavel * v);
    static void FOperadorAtrib(TVariavel * v1, TVariavel * v2);
    static bool FOperadorAdd(TVariavel * v1, TVariavel * v2);
    static bool FOperadorIgual2(TVariavel * v1, TVariavel * v2);
    static unsigned char FOperadorCompara(TVariavel * v1, TVariavel * v2);

    friend class TIndiceItem;
};

//----------------------------------------------------------------------------

#endif
