#ifndef VARIAVEL_H
#define VARIAVEL_H

class TClasse;
class TObjeto;

//----------------------------------------------------------------------------
/// Tipo de variável
enum TVarTipo
{
    varNulo,    ///< "NULO" ou desconhecido
    varInt,     ///< Variável int
    varDouble,  ///< Variável double
    varTxt,     ///< Texto (const char*)
    varObj      ///< Referência (TObjeto*)
};

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

// Dados da variável
    static int Tamanho(const char * instr);
        ///< Obtém o tamanho de uma variável na memória
        /**< @param instr Instrução codificada por Instr::Codif
             @return Tamanho da variável (0=não ocupa lugar na memória) */

    int Tamanho();
        ///< Obtém o tamanho de uma variável conforme TVariavel::defvar

    TVarTipo Tipo();
        ///< Obtém o tipo mais apropriado para expressões numéricas
        /**< Usa  TVariavel::defvar  e  TVariavel::endvar
             @return Tipo de variável */

// Construtor/destrutor/mover
    void Criar(TClasse * c, TObjeto * o);
        ///< Criar variável: acerta dados da variável na memória
        /**< Usa  TVariavel::defvar  e  TVariavel::endvar
             @note Criar uma variável significa:
                - Alocar memória para a variável
                - Chamar TVariavel::Criar()  */

    void Apagar();
        ///< Apagar variável: remove dados da variável na memória
        /**< Usa  TVariavel::defvar  e  TVariavel::endvar
             @note Não libera memória alocada (não executa delete) */

    void Mover(void * destino, TClasse * c, TObjeto * o);
        ///< Move a variável para outra região da memória
        /**< Usa:
             - TVariavel::defvar = definição da variável
             - TVariavel::endvar = endereço atual   */

// Funções get
    bool getBool();         ///< Obtém o valor "bool" da variável
    int getInt();           ///< Obtém o valor "int" da variável
    double getDouble();     ///< Obtém o valor "double" da variável
    const char * getTxt();  ///< Obtém o texto da variável
    TObjeto * getObj();     ///< Obtém a referência da variável

// Funções set
    void setInt(int valor); ///< Muda variável a partir de int
    void setDouble(double valor); ///< Muda variável a partir de double
    void setTxt(const char * txt); ///< Muda variável a partir de texto
    void addTxt(const char * txt); ///< Adiciona texto na variável
    void setObj(TObjeto * obj); ///< Muda variável a partir de referência

// Variáveis
    const char * defvar; ///< Instrução que define a variável
                         /**< @sa Instr::Comando */
    union {
        void * endvar;  ///< Endereço da variável na memória (0 se não for aplicável)
        const void * endfixo;
                    ///< Valor "const" de endvar
                    /**< Usar endfixo quando a variável não poderá ser mudada */
    };
    int  tamanho;   ///< Quantos bytes está usando na memória
                    /**< 0 significa que não está usando ou a variável está
                        sendo usada em outro lugar
                             @note Não é usado em TVariavel */
    unsigned char bit;  ///< Máscara do bit, se for variável de bit
};

//----------------------------------------------------------------------------
/** Trata das variáveis do tipo REF */
class TVarRef /// Variável REF
{
public:
    void MudarPont(TObjeto * obj);
    TObjeto * Pont;
    TVarRef * Antes;
    TVarRef * Depois;
};

//----------------------------------------------------------------------------

#endif
