#ifndef TMUDARCLASSE_H
#define TMUDARCLASSE_H

#include "classe.h"

class TAddBuffer;

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
    bool InfoApagar() { return (RBcolour&2) && Comandos==0; }
        ///< Retorna true se classe vai ser apagada

    static bool ExecPasso();
        ///< Executa um passo das alterações no programa
        /**< @return true se deve voltar ao início de Instr::ExecFim
             @note Chamado em Instr::ExecFim */

    char * Comandos;    ///< Lista de comandos, =0 se não deve alterar classe
    char Nome[CLASSE_NOME_TAM]; ///< Nome da classe; não deve ser mudado
    TArqMapa * Arquivo; ///< Em qual arquivo criar classe
    static char Salvar;
        ///< Se deve salvar ou não as classes em arquivo
        /**< - 0 = não salvar
             - 1 = salvar só classes que mudaram
             - 2 = salvar tudo */

// Lista ligada - ordem em que as classes serão alteradas
    TMudarClasse * Antes;     ///< Objeto anterior
    TMudarClasse * Depois;    ///< Próximo objeto
    static TMudarClasse * Inicio; ///< Primeiro item
    static TMudarClasse * Fim;    ///< Último item

// Árvore organizada por TMudarClasse::Nome
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
    unsigned char RBcolour;
            ///< Cor da RBT e bits indicativos
            /**< - Bit 0 = cor da RBT
             *   - Bit 1 = 1 se deve apagar classe
             *   - Bit 2 = 1 se deve executar função iniclasse */
};

//----------------------------------------------------------------------------
class TMudarAux /// Funções auxiliares para mudar a lista de instruções de uma classe
{
public:
    TMudarAux();

    static char * ProcuraInstr(char * comando, const char * nomevar);
        ///< Procura variável/função em lista de instruções
        /**< @return Endereço da instrução ou endereço do fim da lista
                 de instruções (dois bytes =0) se não encontrou */
    static char * AvancaInstr(char * comando);
        ///< Avança para próxima variável/função em lista de instruções
    static char * FimInstr(char * comando);
        ///< Avança para o fim da lista de instruções (dois bytes =0)
    static bool CodifInstr(TAddBuffer * destino, const char * origem);
        ///< Codifica instruções
        /**< @param destino Aonde anotar as instruções codificadas
             @param origem Endereço origem (texto)
             @retval true Sucesso; destino contém as instruções codificadas
             @retval false Falha; destino contém as mensagens de erro */
    void AddBloco(const char * ender, int tamanho);
        ///< Adiciona um bloco de instruções na lista
        /**< @param ender Endereço inicial do bloco
             @param tamanho Tamanho do bloco sem os dois bytes =0 no final
             @note É adicionado apenas o endereço e o tamanho do bloco,
                   não o conteúdo */
    bool ChecaBloco(char * mensagem, int tamanho);
        ///< Checa instruções adicionadas com AddBloco
        /**< Verifica se a ordem das instruções está correta
             @param mensagem Aonde colocar a mensagem de erro
             @param tamanho Tamanho do buffer em mensagem
             @return true se não há nenhum erro (buffer não foi modificado),
                     false se há algum erro */
    void AnotaBloco(TMudarClasse * obj);
        ///< Anota alterações em objeto TMudarClasse
        /**< Anota em um objeto TMudarClasse a lista de instruções
             adicionadas com AddBloco */

private:
    const char * endbloco[30]; ///< Endereços dos blocos de instruções
    int  tambloco[30];   ///< Tamanhos dos blocos de instruções
    unsigned int numbloco;  ///< Número de blocos
};

//----------------------------------------------------------------------------
class TRenomeiaClasse /// Classes que devem ser renomeadas
{
public:
    TRenomeiaClasse(const char * antes, const char * depois);
        ///< Construtor
        /**< @param antes Nome da classe que será renomeada
         *   @param depois Novo nome da classe */
    ~TRenomeiaClasse();              ///< Destrutor
    static void Processa();          ///< Renomeia as classes

private:
    char NomeAntes[CLASSE_NOME_TAM]; ///< Nome da classe
    char NomeDepois[CLASSE_NOME_TAM];///< Novo nome da classe

    static TRenomeiaClasse * Inicio; ///< Primeiro objeto
    static TRenomeiaClasse * Fim;    ///< Último objeto
    TRenomeiaClasse * Antes;         ///< Objeto anterior
    TRenomeiaClasse * Depois;        ///< Próximo objeto
};
//----------------------------------------------------------------------------

#endif
