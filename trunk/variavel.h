#ifndef VARIAVEL_H
#define VARIAVEL_H

//----------------------------------------------------------------------------
/** Contém as informações necessárias para acessar uma variável
 *  - Usado ao acessar variáveis
 *  - Usado também ao executar funções (a pilha de variáveis)
 */
class TVariavel /// Acesso às variáveis
{
public:
    TVariavel();        ///< Construtor
    void Limpar();      ///< Limpa todos os campos do objeto

    char * defvar;  ///< Instrução que define a variável
                    /**< @sa Instr::Comando */
    void * endvar;  ///< Endereço da variável na memória (0 se não for aplicável)
    unsigned char local;///< Se deve apagar variável quando terminar a função
                        /**< @note Não é usado em TVariavel */
    unsigned char bit;  ///< Máscara do bit, se for variável de bit
};

//----------------------------------------------------------------------------

#endif
