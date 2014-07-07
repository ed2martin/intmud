#ifndef TMUDARCLASSE_H
#define TMUDARCLASSE_H

#include "classe.h"

class TAddBuffer;

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
    bool InfoApagar() { return (RBcolour&2) && Comandos==0; }
        ///< Retorna true se classe vai ser apagada

    static bool ExecPasso();
        ///< Executa um passo das altera��es no programa
        /**< @return true se deve voltar ao in�cio de Instr::ExecFim
             @note Chamado em Instr::ExecFim */

    char * Comandos;    ///< Lista de comandos, =0 se n�o deve alterar classe
    char Nome[CLASSE_NOME_TAM]; ///< Nome da classe; n�o deve ser mudado
    TArqMapa * Arquivo; ///< Em qual arquivo criar classe
    static char Salvar;
        ///< Se deve salvar ou n�o as classes em arquivo
        /**< - 0 = n�o salvar
             - 1 = salvar s� classes que mudaram
             - 2 = salvar tudo */

// Lista ligada - ordem em que as classes ser�o alteradas
    TMudarClasse * Antes;     ///< Objeto anterior
    TMudarClasse * Depois;    ///< Pr�ximo objeto
    static TMudarClasse * Inicio; ///< Primeiro item
    static TMudarClasse * Fim;    ///< �ltimo item

// �rvore organizada por TMudarClasse::Nome
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
    unsigned char RBcolour;
            ///< Cor da RBT e bits indicativos
            /**< - Bit 0 = cor da RBT
             *   - Bit 1 = 1 se deve apagar classe
             *   - Bit 2 = 1 se deve executar fun��o iniclasse */
};

//----------------------------------------------------------------------------
class TMudarAux /// Fun��es auxiliares para mudar a lista de instru��es de uma classe
{
public:
    TMudarAux();

    static char * ProcuraInstr(char * comando, const char * nomevar);
        ///< Procura vari�vel/fun��o em lista de instru��es
        /**< @return Endere�o da instru��o ou endere�o do fim da lista
                 de instru��es (dois bytes =0) se n�o encontrou */
    static char * AvancaInstr(char * comando);
        ///< Avan�a para pr�xima vari�vel/fun��o em lista de instru��es
    static char * FimInstr(char * comando);
        ///< Avan�a para o fim da lista de instru��es (dois bytes =0)
    static bool CodifInstr(TAddBuffer * destino, const char * origem);
        ///< Codifica instru��es
        /**< @param destino Aonde anotar as instru��es codificadas
             @param origem Endere�o origem (texto)
             @retval true Sucesso; destino cont�m as instru��es codificadas
             @retval false Falha; destino cont�m as mensagens de erro */
    void AddBloco(const char * ender, int tamanho);
        ///< Adiciona um bloco de instru��es na lista
        /**< @param ender Endere�o inicial do bloco
             @param tamanho Tamanho do bloco sem os dois bytes =0 no final
             @note � adicionado apenas o endere�o e o tamanho do bloco,
                   n�o o conte�do */
    bool ChecaBloco(char * mensagem, int tamanho);
        ///< Checa instru��es adicionadas com AddBloco
        /**< Verifica se a ordem das instru��es est� correta
             @param mensagem Aonde colocar a mensagem de erro
             @param tamanho Tamanho do buffer em mensagem
             @return true se n�o h� nenhum erro (buffer n�o foi modificado),
                     false se h� algum erro */
    void AnotaBloco(TMudarClasse * obj);
        ///< Anota altera��es em objeto TMudarClasse
        /**< Anota em um objeto TMudarClasse a lista de instru��es
             adicionadas com AddBloco */

private:
    const char * endbloco[30]; ///< Endere�os dos blocos de instru��es
    int  tambloco[30];   ///< Tamanhos dos blocos de instru��es
    unsigned int numbloco;  ///< N�mero de blocos
};

//----------------------------------------------------------------------------
class TRenomeiaClasse /// Classes que devem ser renomeadas
{
public:
    TRenomeiaClasse(const char * antes, const char * depois);
        ///< Construtor
        /**< @param antes Nome da classe que ser� renomeada
         *   @param depois Novo nome da classe */
    ~TRenomeiaClasse();              ///< Destrutor
    static void Processa();          ///< Renomeia as classes

private:
    char NomeAntes[CLASSE_NOME_TAM]; ///< Nome da classe
    char NomeDepois[CLASSE_NOME_TAM];///< Novo nome da classe

    static TRenomeiaClasse * Inicio; ///< Primeiro objeto
    static TRenomeiaClasse * Fim;    ///< �ltimo objeto
    TRenomeiaClasse * Antes;         ///< Objeto anterior
    TRenomeiaClasse * Depois;        ///< Pr�ximo objeto
};
//----------------------------------------------------------------------------

#endif
