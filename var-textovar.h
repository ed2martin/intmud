#ifndef VAR_TEXTOVAR_H
#define VAR_TEXTOVAR_H

//----------------------------------------------------------------------------
class TVariavel;
class TBlocoVar;

class TTextoVar  /// Variáveis TextoVar
{
public:
    void Apagar();          ///< Apaga objeto
    void Limpar();          ///< Apaga o texto do objeto
    void Mover(TTextoVar * destino);
        ///< Move objeto para outro lugar
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
    TBlocoVar * Mudar(const char * texto);
        ///< Adiciona/muda/apaga texto
    bool Func(TVariavel * v, const char * nome);
        ///< Função da variável
    TBlocoVar * RBroot;  ///< Objeto raiz da RBT
};

//----------------------------------------------------------------------------
class TBlocoVar /// Bloco de texto de um objeto TTextoVar
{
public:
    static TBlocoVar * AlocarMem(const char * texto); ///< Aloca memória para objeto
    void Apagar(); ///< Apaga objeto
    void Mover(TBlocoVar * destino);
        ///< Move bloco para outro lugar
        /**< @param destino Endereço destino
         *   @note Usado por TGrupoVar */

// Árvore organizada por TBlocoVar::Texto
    void RBinsert(void);        ///< Insere objeto na RBT
    void RBremove(void);        ///< Remove objeto da RBT
    TBlocoVar * RBfirst(void); ///< Primeiro objeto da RBT
    TBlocoVar * RBlast(void);  ///< Último objeto da RBT
    static TBlocoVar * RBnext(TBlocoVar *); ///< Próximo objeto da RBT
    static TBlocoVar * RBprevious(TBlocoVar *); ///< Objeto anterior da RBT
    static int RBcomp(TBlocoVar * x, TBlocoVar * y); ///< Compara objetos
    void RBleft_rotate(void);
    void RBright_rotate(void);
    void RBmove(TBlocoVar * novoender);
        ///< Muda o endereço de um objeto já inserido na RBT
        /**< @note Os dois objetos devem ter as mesmas informações.
         *   O novo objeto não deve estar inserido na RBT */

    //static TBlocoVar * RBroot;  ///< Objeto raiz
    union {
        char Inicio[1];     ///< Marca o início da classe
        TBlocoVar *RBparent;    ///< Objeto objeto pai
    };
    TBlocoVar *RBleft,*RBright; ///< Objetos filhos
    TTextoVar *TextoVar;    ///< Variável TextoVar que contém o bloco (RBT)
    unsigned short Bytes;   ///< Número de bytes disponíveis em Texto
    unsigned char RBcolour; ///< Bit 0=cor, bit 1: 0=RBT, 1=somente texto

    char Texto[1];
    ///< A partir daqui, dois textos texto em ASCIIZ: nome e valor da variável
};

//----------------------------------------------------------------------------
#endif
