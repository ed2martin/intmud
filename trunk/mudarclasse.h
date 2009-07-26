#ifndef TMUDARCLASSE_H
#define TMUDARCLASSE_H

#include "classe.h"

//----------------------------------------------------------------------------
/** Cont�m a lista de modifica��es que devem ser feitas nas classes
    do programa interpretado */
class TMudarClasse /// Classes que devem ser mudadas
{
public:
    TMudarClasse(const char * nome); ///< Construtor
    ~TMudarClasse(); ///< Destrutor

    static TMudarClasse * Procurar(const char * nome);
        ///< Procura um objeto TMudarClasse a partir do nome
        /**<
         *  @param nome Nome a pesquisar
         *  @return Endere�o do objeto, ou 0 se n�o foi encontrado */
    void MudarComandos(char * com); ///< Muda a vari�vel Comandos
        /**< Apaga o Comandos anterior se necess�rio */
    void MarcarApagar() { RBcolour |= 2; }
        ///< Marca classe para exclus�o
    void DesmarcarApagar() { RBcolour &= ~2; }
        ///< Desmarca classe para exclus�o

    static bool ExecPasso();
            ///< Executa um passo das altera��es no programa
            /**< @return true se deve voltar ao in�cio de Instr::ExecFim
                 @note Chamado em Instr::ExecFim */

    char * Comandos;    ///< Lista de comandos, =0 se n�o deve alterar classe
    char Nome[CLASSE_NOME_TAM]; ///< Nome da classe; n�o deve ser mudado

// Lista ligada - ordem em que as classes ser�o alteradas
    TMudarClasse * Antes;     ///< Objeto anterior
    TMudarClasse * Depois;    ///< Pr�ximo objeto
    static TMudarClasse * Inicio; ///< Primeiro item
    static TMudarClasse * Fim;    ///< �ltimo item

// �rvore organizada por TClasse::Nome
public:
    void RBinsert(void);        ///< Insere objeto na RBT
    void RBremove(void);        ///< Remove objeto da RBT
    static TMudarClasse * RBfirst(void); ///< Primeiro objeto da RBT
    static TMudarClasse * RBlast(void);  ///< �ltimo objeto da RBT
    static TMudarClasse * RBnext(TMudarClasse *); ///< Pr�ximo objeto da RBT
    static TMudarClasse * RBprevious(TMudarClasse *); ///< Objeto anterior da RBT
private:
    static TMudarClasse * RBroot;  ///< Objeto raiz
    TMudarClasse *RBleft,*RBright,*RBparent; ///< Objetos filhos e objeto pai
    void RBleft_rotate(void);
    void RBright_rotate(void);
    static int RBcomp(TMudarClasse * x, TMudarClasse * y); ///< Compara objetos
    unsigned char RBcolour; ///< Bit 0=cor, Bit 1=1 se deve apagar classe
};

//----------------------------------------------------------------------------

#endif
