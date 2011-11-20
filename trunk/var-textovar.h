#ifndef VAR_TEXTOVAR_H
#define VAR_TEXTOVAR_H

//----------------------------------------------------------------------------
class TVariavel;
class TBlocoVar;

class TTextoVar  /// Vari�veis TextoVar
{
public:
    void Apagar();          ///< Apaga objeto
    void Limpar();          ///< Apaga o texto do objeto
    void Mover(TTextoVar * destino);
        ///< Move objeto para outro lugar
    TBlocoVar * Procura(const char * texto);
        ///< Procura um objeto TIndiceObj a partir do nome
        /**< @param texto Nome a pesquisar
         *   @return Endere�o do objeto, ou 0 se n�o foi encontrado */
    TBlocoVar * ProcIni(const char * nome);
        ///< Semelhante a Procura(), mas procura o primeiro texto
    TBlocoVar * ProcFim(const char * nome);
        ///< Semelhante a Procura(), mas procura o �ltimo texto
    TBlocoVar * ProcAntes(const char * nome);
        ///< Semelhante a Procura(), mas procura o texto anterior
    TBlocoVar * ProcDepois(const char * nome);
        ///< Semelhante a Procura(), mas procura o pr�ximo texto
    TBlocoVar * Mudar(const char * texto);
        ///< Adiciona/muda/apaga texto
    bool Func(TVariavel * v, const char * nome);
        ///< Fun��o da vari�vel
    TBlocoVar * RBroot;  ///< Objeto raiz da RBT
};

//----------------------------------------------------------------------------
class TBlocoVar /// Bloco de texto de um objeto TTextoVar
{
public:
    static TBlocoVar * AlocarMem(const char * texto); ///< Aloca mem�ria para objeto
    void Apagar(); ///< Apaga objeto
    void Mover(TBlocoVar * destino);
        ///< Move bloco para outro lugar
        /**< @param destino Endere�o destino
         *   @note Usado por TGrupoVar */

// �rvore organizada por TBlocoVar::Texto
    void RBinsert(void);        ///< Insere objeto na RBT
    void RBremove(void);        ///< Remove objeto da RBT
    TBlocoVar * RBfirst(void); ///< Primeiro objeto da RBT
    TBlocoVar * RBlast(void);  ///< �ltimo objeto da RBT
    static TBlocoVar * RBnext(TBlocoVar *); ///< Pr�ximo objeto da RBT
    static TBlocoVar * RBprevious(TBlocoVar *); ///< Objeto anterior da RBT
    static int RBcomp(TBlocoVar * x, TBlocoVar * y); ///< Compara objetos
    void RBleft_rotate(void);
    void RBright_rotate(void);
    void RBmove(TBlocoVar * novoender);
        ///< Muda o endere�o de um objeto j� inserido na RBT
        /**< @note Os dois objetos devem ter as mesmas informa��es.
         *   O novo objeto n�o deve estar inserido na RBT */

    //static TBlocoVar * RBroot;  ///< Objeto raiz
    union {
        char Inicio[1];     ///< Marca o in�cio da classe
        TBlocoVar *RBparent;    ///< Objeto objeto pai
    };
    TBlocoVar *RBleft,*RBright; ///< Objetos filhos
    TTextoVar *TextoVar;    ///< Vari�vel TextoVar que cont�m o bloco (RBT)
    unsigned short Bytes;   ///< N�mero de bytes dispon�veis em Texto
    unsigned char RBcolour; ///< Bit 0=cor, bit 1: 0=RBT, 1=somente texto

    char Texto[1];
    ///< A partir daqui, dois textos texto em ASCIIZ: nome e valor da vari�vel
};

//----------------------------------------------------------------------------
#endif