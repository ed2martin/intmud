#ifndef VAR_INDICEOBJ_H
#define VAR_INDICEOBJ_H

class TVariavel;
class TVarInfo;
class TClasse;
class TObjeto;
class TIndiceObj;

//----------------------------------------------------------------------------
class TIndiceItem /// Variável IndiceItem
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    void Apagar();          ///< Apaga objeto
    void Mover(TIndiceItem * destino); ///< Move para outro lugar
    int  getValor();        ///< Ler valor numérico da variável
    TIndiceObj * getIndiceObj(); ///< Retorna IndiceObj
    void MudarRef(TIndiceObj * indice); ///< Muda referência de ListaX
    void Igual(TIndiceItem * v);     ///< Operador de atribuição igual
    bool Func(TVariavel * v, const char * nome); ///< Função da variável

    bool FuncObj(TVariavel * v);
    bool FuncTxt(TVariavel * v);
    bool FuncAntes(TVariavel * v);
    bool FuncDepois(TVariavel * v);
    bool FuncIni(TVariavel * v);
    bool FuncFim(TVariavel * v);

private:
    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static void FRedim(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);

    TIndiceItem * Antes;    ///< Objeto anterior
    TIndiceItem * Depois;   ///< Próximo objeto
    TIndiceObj * IndiceObj; ///< Qual variável está apontando
    int  TamTxt;            ///< Tamanho do texto ao avançar/voltar objeto

    friend class TIndiceObj;
};

//----------------------------------------------------------------------------
class TIndiceObj /// Variável IndiceObj
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    void Apagar();          ///< Apaga objeto
    void Mover(TIndiceObj * destino); ///< Move para outro lugar
    const char * getNome();     ///< Lê o texto
    void setNome(const char * texto); ///< Muda o texto
    static TIndiceObj * Procura(const char * nome);
        ///< Procura um objeto TIndiceObj a partir do nome
        /**<
         *  @param nome Nome a pesquisar
         *  @return Endereço do objeto, ou 0 se não foi encontrado */
    static TIndiceObj * ProcIni(const char * nome);
        ///< Semelhante a Procura(), mas procura o primeiro texto
    static TIndiceObj * ProcFim(const char * nome);
        ///< Semelhante a Procura(), mas procura o último texto

    TObjeto * Objeto;       ///< Objeto que contém a variável

// Árvore organizada por TIndiceObj::Nome
// Se *Nome==0, objeto não está na árvore
    void RBinsert(void);        ///< Insere objeto na RBT
    void RBremove(void);        ///< Remove objeto da RBT
    static TIndiceObj * RBfirst(void); ///< Primeiro objeto da RBT
    static TIndiceObj * RBlast(void);  ///< Último objeto da RBT
    static TIndiceObj * RBnext(TIndiceObj *); ///< Próximo objeto da RBT
    static TIndiceObj * RBprevious(TIndiceObj *); ///< Objeto anterior da RBT
private:
    static TIndiceObj * RBroot;  ///< Objeto raiz
    TIndiceObj *RBleft,*RBright,*RBparent; ///< Objetos filhos e objeto pai
    void RBleft_rotate(void);
    void RBright_rotate(void);
    static int RBcomp(TIndiceObj * x, TIndiceObj * y); ///< Compara objetos
    unsigned char RBcolour;

    char Nome[65];          ///< Texto da variável
    TIndiceItem * IndiceItem; ///< Primeiro indiceitem apontando para esse objeto

    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static void FRedim(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);

    friend class TIndiceItem;
};

//----------------------------------------------------------------------------

#endif
