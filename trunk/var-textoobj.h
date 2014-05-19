#ifndef VAR_TEXTOOBJ_H
#define VAR_TEXTOOBJ_H

//----------------------------------------------------------------------------
class TVariavel;
class TObjeto;
class TTextoObjSub;
class TBlocoObj;

class TTextoObj  /// Variáveis TextoObj
{
public:
    void Apagar();          ///< Apaga objeto
    void Limpar();          ///< Apaga o texto do objeto
    void Mover(TTextoObj * destino);
        ///< Move objeto para outro lugar
    TBlocoObj * Procura(const char * texto);
        ///< Procura um objeto TIndiceObj a partir do nome
        /**< @param texto Nome a pesquisar
         *   @return Endereço do objeto, ou 0 se não foi encontrado */
    TBlocoObj * ProcIni(const char * nome);
        ///< Semelhante a Procura(), mas procura o primeiro texto
    TBlocoObj * ProcFim(const char * nome);
        ///< Semelhante a Procura(), mas procura o último texto
    TBlocoObj * ProcAntes(const char * nome);
        ///< Semelhante a Procura(), mas procura o texto anterior
    TBlocoObj * ProcDepois(const char * nome);
        ///< Semelhante a Procura(), mas procura o próximo texto
    void Mudar(const char * nomevar, TObjeto * obj);
        ///< Adiciona/muda/apaga variável
    bool Func(TVariavel * v, const char * nome);
        ///< Função da variável

    bool FuncValor(TVariavel * v);
    bool FuncValorIni(TVariavel * v);
    bool FuncValorFim(TVariavel * v);
    bool FuncNomeVar(TVariavel * v);
    bool FuncMudar(TVariavel * v);
    bool FuncAntes(TVariavel * v);
    bool FuncDepois(TVariavel * v);
    bool FuncIni(TVariavel * v);
    bool FuncFim(TVariavel * v);
    bool FuncLimpar(TVariavel * v);
    bool FuncApagar(TVariavel * v);
    bool FuncTotal(TVariavel * v);

    TBlocoObj * RBroot;  ///< Objeto raiz da RBT
    TTextoObjSub * Inicio; ///< Primeiro objeto TTextoObjSub
    TObjeto * Objeto;       ///< Objeto em que o textoobj lista foi definido
    int Total;  ///< Quantidade de variáveis
};

//----------------------------------------------------------------------------
class TTextoObjSub /// Para acessar uma variável de textoobj
{
public:
    void Criar(TTextoObj * var); ///< Adiciona objeto em um textoobj
    void Apagar();      ///< Retira objeto de um textoobj
    void Mover(TTextoObjSub * destino);
        ///< Move bloco para outro lugar

    int getInt();               ///< Retorna valor como int
    TObjeto * getObj();         ///< Retorna valor como TObjeto
    void setObj(TObjeto * obj); ///< Muda o valor
    TTextoObj * TextoObj;       ///< A qual textoobj pertence
    TTextoObjSub * Antes;       ///< Objeto anterior
    TTextoObjSub * Depois;      ///< Próximo objeto
    char NomeVar[64];           ///< Nome da variável
};

//----------------------------------------------------------------------------
class TBlocoObj /// Bloco de texto de um objeto TTextoObj
{
public:
    void InsereLista(TObjeto * obj); ///< Adiciona objeto na lista ligada
    void RemoveLista();        ///< Remove objeto da lista ligada
    void Apagar();              ///< Apaga objeto
    void Mover(TBlocoObj * destino);
        ///< Move bloco para outro lugar
        /**< @param destino Endereço destino
         *   @note Usado por TGrupoVar */
    void MoveTextoObj(TTextoObj * textoobj);
        ///< Usado por TTextoObj::Mover, para mudar TBlocoObj::TextoObj
    void FuncApagarSub();
        ///< Usado por TTextoObj::FuncApagar, para marcar para exclusão

// Árvore organizada por TBlocoObj::Texto
    void RBinsert(void);        ///< Insere objeto na RBT
    void RBremove(void);        ///< Remove objeto da RBT
    TBlocoObj * RBfirst(void);  ///< Primeiro objeto da RBT
    TBlocoObj * RBlast(void);   ///< Último objeto da RBT
    static TBlocoObj * RBnext(TBlocoObj *); ///< Próximo objeto da RBT
    static TBlocoObj * RBprevious(TBlocoObj *); ///< Objeto anterior da RBT
    static int RBcomp(TBlocoObj * x, TBlocoObj * y); ///< Compara objetos
    void RBleft_rotate(void);
    void RBright_rotate(void);

    TObjeto * Objeto;       ///< Objeto que está na lista
    TBlocoObj * ObjAntes;   ///< Objeto anterior da lista
    TBlocoObj * ObjDepois;  ///< Próximo objeto da lista

    //static TBlocoObj * RBroot;  ///< Objeto raiz
    TBlocoObj *RBparent;    ///< Objeto objeto pai
    TBlocoObj *RBleft,*RBright; ///< Objetos filhos
    TTextoObj *TextoObj;    ///< Variável TextoObj que contém o bloco (RBT)
    unsigned char RBcolour; ///< Bit 0=cor, bit 1: 0=RBT, 1=somente texto

    char NomeVar[1];        ///< Nome da variável, em ASCIIZ
};

//----------------------------------------------------------------------------
#endif
