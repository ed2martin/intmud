#ifndef VARIAVEL_H
#define VARIAVEL_H

//----------------------------------------------------------------------------
/** Cont�m as informa��es necess�rias para acessar uma vari�vel
 *  - Usado ao acessar vari�veis
 *  - Usado tamb�m ao executar fun��es (a pilha de vari�veis)
 */
class TVariavel /// Acesso �s vari�veis
{
public:
    TVariavel();        ///< Construtor
    void Limpar();      ///< Limpa todos os campos do objeto

    char * defvar;  ///< Instru��o que define a vari�vel
                    /**< @sa Instr::Comando */
    void * endvar;  ///< Endere�o da vari�vel na mem�ria (0 se n�o for aplic�vel)
    unsigned char local;///< Se deve apagar vari�vel quando terminar a fun��o
                        /**< @note N�o � usado em TVariavel */
    unsigned char bit;  ///< M�scara do bit, se for vari�vel de bit
};

//----------------------------------------------------------------------------

#endif
