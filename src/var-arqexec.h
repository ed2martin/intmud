#ifndef VAR_ARQEXEC_H
#define VAR_ARQEXEC_H

#ifdef _WIN32
 #include <winsock2.h>
 #include <windows.h>
#else
 #include <sys/types.h>
 #include <netinet/in.h>
#endif
#include "exec.h"

#define EXEC_ENV 2048

//------------------------------------------------------------------------------
class TArqExec /// Opções ARQEXEC do arquivo INT principal
{
public:
    TArqExec(const char * nome);
    ~TArqExec();

    static TArqExec * ExecIni() { return Inicio; }
        ///< Retorna o primeiro objeto TArqExec
    TArqExec * ExecProximo() { return Proximo; }
        ///< Retorna o próximo objeto TArqExec ou 0 se não houver
    const char * ExecNome() { return Nome; }
        ///< Retorna o nome

private:
    char * Nome;            ///< Texto da opção arqexec do arquivo int principal

    TArqExec *Anterior;     ///< Lista ligada; objeto anterior
    TArqExec *Proximo;      ///< Lista ligada; próximo objeto
    static TArqExec *Inicio;///< Lista ligada; primeiro objeto
};

//------------------------------------------------------------------------------
class TClasse;
class TObjeto;
class TVariavel;
class TVarInfo;
class TVarArqExec;
class TObjExec : public TExec /// Um programa sendo executado por ArqExec
{
public:
    TObjExec(TVarArqExec * var);   ///< Construtor
    virtual ~TObjExec();        ///< Destrutor
    bool Enviar(const char * mensagem);
        ///< Envia mensagem
        /**< @return true se conseguiu enviar, false se buffer cheio */
    static void Fd_Set(fd_set * set_entrada, fd_set * set_saida);
    static void ProcEventos(fd_set * set_entrada, fd_set * set_saida);

private:
    TVarArqExec * VarArqExec;      ///< Objeto TVarArqExec associado a este objeto

// Para enviar mensagens
    void EnvPend();             ///< Envia dados pendentes
    char bufEnv[EXEC_ENV];      ///< Contém a mensagem que será enviada
    unsigned int pontEnv;       ///< Número de bytes pendentes em bufEnv
    static bool boolenvpend;    ///< Verdadeiro se tem algum dado pendente

// Para receber mensagens
    void Receber();
        ///< Recebe as mensagens pendentes
    char dadoRecebido;
        ///< Para controle da mensagem recebida
        /**< - 0 = comportamento padrão
         *   - 0x0D, 0x0A = para detectar nova linha */

// Lista ligada
    static TObjExec * Inicio; ///< Primeiro objeto da lista ligada
    TObjExec * Antes;  ///< Objeto anterior da lista ligada
    TObjExec * Depois; ///< Próximo objeto da lista ligada

    friend class TVarArqExec;
};

//------------------------------------------------------------------------------
class TVarArqExec /// Uma variável ArqExec
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    void Mover(TVarArqExec * destino);  ///< Move TVarArqExec para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);

    const char * defvar;    ///< Como foi definida a variável
    union {
        TClasse * endclasse;///< Em que classe está definido
        TObjeto * endobjeto;///< Em que objeto está definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< Índice no vetor

private:
    void GeraEvento(const char * evento, const char * texto, int valor);
        ///< Executa uma função
        /**< @param evento Nome do evento (ex. "msg")
         *   @param texto Texto do primeiro argumento, nullptr = nenhum texto
         *   @param valor Segundo argumento, menor que 0 = nenhum valor
         *   @note O objeto pode ser apagado nessa função */

    static bool FuncMsg(TVariavel * v);    ///< Processa função Msg
    static bool FuncAbrir(TVariavel * v);  ///< Processa função Abrir
    static bool FuncFechar(TVariavel * v); ///< Processa função Fechar
    static bool FuncAberto(TVariavel * v); ///< Processa função Aberto

    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static void FRedim(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FMoverEnd(TVariavel * v, void * destino,
            TClasse * c, TObjeto * o);
    static void FMoverDef(TVariavel * v);

    TObjExec * ObjExec;     ///< Programa atual

    friend class TObjExec;
};

//------------------------------------------------------------------------------

#endif
