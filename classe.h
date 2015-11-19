#ifndef CLASSE_H
#define CLASSE_H

class TObjeto;
class TArqMapa;

#define HERDA_TAM 50 // Número máximo de classes herdadas por classe
#define CLASSE_NOME_TAM 48 // Tamanho máximo dos nomes das classes + 1

//----------------------------------------------------------------------------
/** Definição das classes dos arquivos .int

Na inicialização do programa:
-# Criar classes
-# Colocar os comandos em TClasse::Comandos das classes
-# AcertaDeriv() para acertar herança de todas as classes
-# AcertaVar() para cada classe
-# Executar a função iniclasse de cada classe
.

Para alterar uma classe:
-# Criar um novo TClasse::Comandos, mas ainda não apagar o antigo
-# Chamar AcertaDeriv(Comandos antigo) e AcertaVar(true), nessa ordem
-# Apagar o antigo TClasse::Comandos
.

Para criar uma classe:
-# Checar se nome de classe existe e é válido
-# Criar objeto TClasse
-# Acertar TClasse::Comandos
-# AcertaDeriv("\0\0")
-# AcertaVar(true)
.

Para apagar uma classe:
-# Se a classe tiver algum objeto:
   -# Marcar o primeiro objeto como pendente para apagar
   -# Chamar a função fim do primeiro objeto (será apagado aqui)
   -# Voltar ao item 1
   .
-# Apagar objeto TClasse (com delete)
.
*/
class TClasse /// Classes
{
public:
    TClasse(const char * nome, TArqMapa * arquivo);
        ///< Construtor
        /**< @param nome Nome da classe
         *   @param arquivo A qual arquivo pertence
         *   @note Indica que o arquivo da classe foi alterado */
    ~TClasse();
        ///< Destrutor
        /**< @note Antes de apagar, não deve haver nenhuma classe derivada
         *   @note Indica que o arquivo da classe foi alterado */

    static bool NomeValido(char * nome);
        ///< Verifica se nome é um nome válido para classe
        /**< @param nome Nome a pesquisar
         *   @return true se o nome é valido; nome original é alterado */
    static char * NomeDef(char * texto);
        ///< Obtém nome de classe a partir da linha que define uma classe
        /**< @param texto Linha contendo a definição da classe
         *   @return Nome da classe ou 0 se não for definição de classe */

    static TClasse * Procura(const char * nome);
        ///< Procura uma classe a partir do nome
        /**< @param nome Nome a pesquisar
         *   @return Endereço da classe, ou 0 se não foi encontrada */
    static TClasse * ProcuraIni(const char * nome);
        ///< Procura() que procura a primeira classe que começa com o nome
    static TClasse * ProcuraFim(const char * nome);
        ///< Procura() que procura a última classe que começa com o nome
    void MudaNome(const char * novonome);
        ///< Muda o nome da classe

    char * Comandos;    ///< Lista de comandos da classe
    char Nome[CLASSE_NOME_TAM]; ///< Nome da classe; não deve ser mudado

// Herança
    int Heranca(TClasse ** buffer, int tambuf);
        ///< Obtém classes herdadas de uma determinada classe
        /**< @param buffer Aonde colocar as classes obtidas,
         *            pelo menos HERDA_TAM entradas
         *   @param tambuf Número de elementos de buffer
         *   @return Número de classes herdadas */

    bool RetiraDeriv(TClasse * cl);
        ///< Retira classe de ListaDeriv/NumDeriv
        /**< @param cl Classe que será removida
         *   @return true se conseguiu remover (achou a classe) */

    void AdicionaDeriv(TClasse * cl);
        ///< Adicionaa classe em ListaDeriv/NumDeriv
        /**< @param cl Classe que será adicionada */

    static void AcertaDeriv();
        ///< Acerta ListaDeriv e NumDeriv de todas as classes

    void AcertaDeriv(char * comandos_antes);
        ///< Acerta ListaDeriv e NumDeriv quando Comandos[] de uma classe mudou
        /**< @param comandos_antes Conteúdo anterior de Comandos[] */

    void LimpaInstr();
        ///< Apaga todas as instruções e indica que a classe não é herdada
        /**< Chamado automaticamente no destrutor da classe */

    TClasse ** ListaDeriv;
        ///< Lista de classes derivadas dessa, ou NULL se NumDeriv=0
    unsigned int NumDeriv;
        ///< Número de elementos de TClasse::ListaDeriv

    static void AcertaComandos(char * comandos);
        ///< Chamado automaticamente por AcertaDeriv
        /**< Acerta as instruções que contém desvio implícito, como
             "se", "enquanto", "efim", etc. */

    static bool AcertaComandosFim(int valor);
        ///< Usado internamente em AcertaComandos() para detectar fim da função

// Variáveis
    int AcertaVar(bool acertaderiv);
        ///< Acerta as variáveis da classe e dos objetos da classe
        /**< @param acertaderiv Se deve acertar classes derivadas e indicar
         *                que o arquivo (que possui a classe) foi alterado
         *   @retval 0 Variáveis não mudaram
         *   @retval 1 Lista de variáveis mudou
         *   @retval 2 Lista de variáveis mudou e foi necessário
         *             criar/apagar variáveis na classe e/ou no objeto
         *   @note Acerta InstrVar, IndiceVar, NumVar, TamObj e TamVars.
         *   @note Acerta variáveis e objetos da classe, se necessário.
         *   @note Não acerta as classes derivadas. */

    int IndiceNome(const char * nome);
        ///< Obtém o índice de uma variável a partir do nome
        /**< @param nome Nome da variável
         *   @return Índice em InstrVar e IndiceVar ou -1 se não encontrou */
    int IndiceNomeIni(const char * nome);
        ///< IndiceNome() que procura a primeira variável que começa com o nome
    int IndiceNomeFim(const char * nome);
        ///< IndiceNome() que procura a última variável que começa com o nome

    char ** InstrVar;
        ///< Instruções que definem as variáveis (inclusive herdadas)
        /**<  @sa Instr::Comando */
    unsigned int * IndiceVar;
        ///< Localização das variáveis (inclusive herdadas) da classe
        /**<
         *  - Bits 21-0 = índice na memória
         *  - Bit 22 =0 -> índice em TObjeto::Vars
         *  - Bit 22 =1 -> índice em TClasse::Vars
         *  - Bit 23 =1 se variável definida na classe (não foi herdada)
         *  - Bits 31-24 = número do bit (de 0 a 7), se for int1 */
    unsigned int NumVar;
        ///< Número de elementos de TClasse::InstrVar e TClasse::IndiceVar

// Objetos
    TObjeto * ObjetoIni; ///< Primeiro objeto da classe
    TObjeto * ObjetoFim; ///< Último objeto da classe
    unsigned int NumObj; ///< Número de objetos
    unsigned int TamObj; ///< Tamanho de TObjeto::Vars (usado ao criar objetos)

    char * Vars;  ///< Variáveis da classe (do tipo comum)
    unsigned int TamVars; ///< Número de bytes usados em TClasse::Vars

    static TClasse * ClInic;
        ///< Usado em conjunto com main.cpp
        /**< Usado ao chamar funções iniclasse de todas as classes */

// Arquivo
    void MoveArquivo(TArqMapa * arquivo);
        ///< Muda a classe de arquivo
        /**< @param arquivo Novo arquivo
             @note Se necessário, indica que o arquivo foi alterado */
    void MoveArqIni(TArqMapa * arquivo);
        ///< Move classe para o início de um arquivo
    void MoveArqFim(TArqMapa * arquivo);
        ///< Move classe para o fim de um arquivo
    void MoveArqAntes(TClasse * cl);
        ///< Move classe no arquivo para antes de outra classe
    void MoveArqDepois(TClasse * cl);
        ///< Move classe no arquivo para depois de outra classe
    TClasse * ArqAntes; ///< Classe anterior no mesmo arquivo
    TClasse * ArqDepois; ///< Próxima classe no mesmo arquivo
    TArqMapa * ArqArquivo; ///< Em qual arquivo está

// Árvore organizada por TClasse::Nome
public:
    void RBinsert(void);        ///< Insere objeto na RBT
    void RBremove(void);        ///< Remove objeto da RBT
    static TClasse * RBfirst(void); ///< Primeiro objeto da RBT
    static TClasse * RBlast(void);  ///< Último objeto da RBT
    static TClasse * RBnext(TClasse *); ///< Próximo objeto da RBT
    static TClasse * RBprevious(TClasse *); ///< Objeto anterior da RBT
    static int RBcomp(TClasse * x, TClasse * y); ///< Compara objetos
private:
    static TClasse * RBroot;  ///< Objeto raiz
    TClasse *RBleft,*RBright,*RBparent; ///< Objetos filhos e objeto pai
    void RBleft_rotate(void);
    void RBright_rotate(void);
    unsigned char RBcolour;

    unsigned char Marca;      ///< Usado para acertar classes derivadas
};

//----------------------------------------------------------------------------

#endif
