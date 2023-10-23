#ifndef VAR_ARQPROG_H
#define VAR_ARQPROG_H

#include <dirent.h>
#ifdef __WIN32__
#include <windows.h>
#endif

#define ARQINCLUIR_TAM 0x200 // Tamanho máximo dos nomes dos arquivos

class TVariavel;
class TVarInfo;
class TClasse;
class TObjeto;

//----------------------------------------------------------------------------
class TArqIncluir
{
public:
    TArqIncluir(const char * nome);
    ~TArqIncluir();
    static bool ProcPadrao(const char * nome);
        ///< Checa se nome é um padrão da lista
    static bool ProcArq(const char * nome);
        ///< Checa se nome pertence a um padrão da lista
    static void ArqNome(const char * nome);
        ///< Muda o nome do arquivo principal
    static const char * ArqNome();
        ///< Retorna o nome do arquivo principal
    static TArqIncluir * IncluirIni() { return Inicio; }
        ///< Retorna o primeiro objeto TIncluir
    TArqIncluir * IncluirProximo() { return Proximo; }
        ///< Retorna o próximo objeto TIncluir ou 0 se não houver
    const char * IncluirNome() { return Padrao; }
        ///< Retorna o nome para início de arquivo

private:
    static char * arqnome;      ///< Nome do arquivo principal
    char Padrao[ARQINCLUIR_TAM];///< Nome para início de arquivo
    TArqIncluir *Anterior;      ///< Lista ligada; objeto anterior
    TArqIncluir *Proximo;       ///< Lista ligada; próximo objeto
    static TArqIncluir *Inicio; ///< Lista ligada; primeiro objeto
    static TArqIncluir *Fim;    ///< Lista ligada; último objeto

    friend class TVarArqProg;
};

//----------------------------------------------------------------------------
class TVarArqProg
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    TVarArqProg() { Criar(); }
    ~TVarArqProg() { Apagar(); }
    void Criar();
            ///< Chamado ao criar objeto
    void Apagar();
            ///< Chamado ao apagar objeto
    void Abrir();  ///< Inicia a busca de arquivos
    void Fechar(); ///< Encerra a busca de arquivos
    void Proximo(); ///< Passa para a próxima entrada
    bool Func(TVariavel * v, const char * nome);
            ///< Função da variável

    char Arquivo[ARQINCLUIR_TAM]; ///< Nome da entrada encontrada, ou "" se nenhuma

private:
#ifdef __WIN32__
    HANDLE wdir; ///< Diretório sendo verificado, ou 0 se nenhum
#else
    DIR * dir; ///< Diretório sendo verificado, ou 0 se nenhum
#endif
    TArqIncluir * Incluir; // Próximo diretório a buscar
    int ArqBarra;       // Primeiro caracter a partir da última "/" no arquivo
    int ArqPadrao;      // Quantos caracteres compõem o padrão do arquivo

    bool FuncAbrir(TVariavel * v);  ///< Processa função Abrir
    bool FuncFechar(TVariavel * v); ///< Processa função Fechar
    bool FuncDepois(TVariavel * v); ///< Processa função Depois
    bool FuncLin(TVariavel * v);    ///< Processa função Lin
    bool FuncTexto(TVariavel * v);  ///< Processa função Texto

    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static void FRedim(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FMoverEnd(TVariavel * v, void * destino,
            TClasse * c, TObjeto * o);
};

//----------------------------------------------------------------------------

#endif
