#ifndef TMUDARCLASSE_H
#define TMUDARCLASSE_H

#include "classe.h"

//----------------------------------------------------------------------------
/** Contém a lista de modificações que devem ser feitas nas classes
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
         *  @return Endereço do objeto, ou 0 se não foi encontrado */
    void MudarComandos(char * com); ///< Muda a variável Comandos
        /**< Apaga o Comandos anterior se necessário */
    void MarcarApagar() { RBcolour |= 2; }
        ///< Marca classe para exclusão
    void DesmarcarApagar() { RBcolour &= ~2; }
        ///< Desmarca classe para exclusão

    static bool ExecPasso();
            ///< Executa um passo das alterações no programa
            /**< @return true se deve voltar ao início de Instr::ExecFim
                 @note Chamado em Instr::ExecFim */

    char * Comandos;    ///< Lista de comandos, =0 se não deve alterar classe
    char Nome[CLASSE_NOME_TAM]; ///< Nome da classe; não deve ser mudado

// Lista ligada - ordem em que as classes serão alteradas
    TMudarClasse * Antes;     ///< Objeto anterior
    TMudarClasse * Depois;    ///< Próximo objeto
    static TMudarClasse * Inicio; ///< Primeiro item
    static TMudarClasse * Fim;    ///< Último item

// Árvore organizada por TClasse::Nome
public:
    void RBinsert(void);        ///< Insere objeto na RBT
    void RBremove(void);        ///< Remove objeto da RBT
    static TMudarClasse * RBfirst(void); ///< Primeiro objeto da RBT
    static TMudarClasse * RBlast(void);  ///< Último objeto da RBT
    static TMudarClasse * RBnext(TMudarClasse *); ///< Próximo objeto da RBT
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
