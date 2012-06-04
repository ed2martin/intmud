#ifndef VAR_INDICEOBJ_H
#define VAR_INDICEOBJ_H

//----------------------------------------------------------------------------
class TObjeto;
class TVariavel;
class TIndiceObj;
class TIndiceItem /// Vari�vel IndiceItem
{
public:
    void Apagar();          ///< Apaga objeto
    void Mover(TIndiceItem * destino); ///< Move para outro lugar
    int  getValor();        ///< Ler valor num�rico da vari�vel
    TIndiceObj * getIndiceObj(); ///< Retorna IndiceObj
    void MudarRef(TIndiceObj * indice); ///< Muda refer�ncia de ListaX
    void Igual(TIndiceItem * v);     ///< Operador de atribui��o igual
    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel

    bool FuncObj(TVariavel * v);
    bool FuncTxt(TVariavel * v);
    bool FuncAntes(TVariavel * v);
    bool FuncDepois(TVariavel * v);
    bool FuncIni(TVariavel * v);
    bool FuncFim(TVariavel * v);

private:
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
    void Apagar();          ///< Apaga objeto
    void Mover(TIndiceObj * destino); ///< Move para outro lugar
    const char * getNome();     ///< L� o texto
    void setNome(const char * texto); ///< Muda o texto
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
private:
    static TIndiceObj * RBroot;  ///< Objeto raiz
    TIndiceObj *RBleft,*RBright,*RBparent; ///< Objetos filhos e objeto pai
    void RBleft_rotate(void);
    void RBright_rotate(void);
    static int RBcomp(TIndiceObj * x, TIndiceObj * y); ///< Compara objetos
    unsigned char RBcolour;

    char Nome[65];          ///< Texto da vari�vel
    TIndiceItem * IndiceItem; ///< Primeiro indiceitem apontando para esse objeto

    friend class TIndiceItem;
};

//----------------------------------------------------------------------------

#endif
