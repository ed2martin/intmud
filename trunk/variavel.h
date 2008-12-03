#ifndef VARIAVEL_H
#define VARIAVEL_H

class TClasse;
class TObjeto;

//----------------------------------------------------------------------------
/// Tipo de vari�vel
enum TVarTipo
{
    varNulo,    ///< "NULO" ou desconhecido
    varInt,     ///< Vari�vel int
    varDouble,  ///< Vari�vel double
    varTxt,     ///< Texto (const char*)
    varObj      ///< Refer�ncia (TObjeto*)
};

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

// Dados da vari�vel
    static int Tamanho(const char * instr);
        ///< Obt�m o tamanho de uma vari�vel na mem�ria
        /**< @param instr Instru��o codificada por Instr::Codif
             @return Tamanho da vari�vel (0=n�o ocupa lugar na mem�ria) */

    int Tamanho();
        ///< Obt�m o tamanho de uma vari�vel conforme TVariavel::defvar

    TVarTipo Tipo();
        ///< Obt�m o tipo mais apropriado para express�es num�ricas
        /**< Usa  TVariavel::defvar  e  TVariavel::endvar
             @return Tipo de vari�vel */

// Construtor/destrutor/mover
    void Criar(TClasse * c, TObjeto * o);
        ///< Criar vari�vel: acerta dados da vari�vel na mem�ria
        /**< Usa  TVariavel::defvar  e  TVariavel::endvar
             @note Criar uma vari�vel significa:
                - Alocar mem�ria para a vari�vel
                - Chamar TVariavel::Criar()  */

    void Apagar();
        ///< Apagar vari�vel: remove dados da vari�vel na mem�ria
        /**< Usa  TVariavel::defvar  e  TVariavel::endvar */

    void Mover(void * destino, TClasse * c, TObjeto * o);
        ///< Move a vari�vel para outra regi�o da mem�ria
        /**< Usa  TVariavel::defvar  e  TVariavel::endvar */

// Fun��es get
    int getInt();           ///< Obt�m o valor "int" da vari�vel
    double getDouble();     ///< Obt�m o valor "double" da vari�vel
    const char * getTxt();  ///< Obt�m o texto da vari�vel
    TObjeto * getObj();     ///< Obt�m a refer�ncia da vari�vel

// Fun��es set
    void setInt(int valor); ///< Muda vari�vel a partir de int
    void setDouble(double valor); ///< Muda vari�vel a partir de double
    void setTxt(const char * txt); ///< Muda vari�vel a partir de texto
    void addTxt(const char * txt); ///< Adiciona texto na vari�vel
    void setObj(TObjeto * obj); ///< Muda vari�vel a partir de refer�ncia

// Vari�veis
    char * defvar;  ///< Instru��o que define a vari�vel
                    /**< @sa Instr::Comando */
    void * endvar;  ///< Endere�o da vari�vel na mem�ria (0 se n�o for aplic�vel)
    unsigned char local;///< Se deve apagar vari�vel quando terminar a fun��o
                        /**< @note N�o � usado em TVariavel */
    unsigned char bit;  ///< M�scara do bit, se for vari�vel de bit
};

//----------------------------------------------------------------------------

#endif