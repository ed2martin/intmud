#ifndef VAR_ARQPROG_H
#define VAR_ARQPROG_H

#include <dirent.h>
#ifdef __WIN32__
#include <windows.h>
#endif

#define ARQINCLUIR_TAM 0x200 // Tamanho m�ximo dos nomes dos arquivos

//----------------------------------------------------------------------------
class TVariavel;
class TArqIncluir
{
public:
    TArqIncluir(const char * nome);
    ~TArqIncluir();
    static bool ProcPadrao(const char * nome);
        ///< Checa se nome � um padr�o da lista
    static bool ProcArq(const char * nome);
        ///< Checa se nome pertence a um padr�o da lista
    static void ArqNome(const char * nome);
        ///< Muda o nome do arquivo principal
    static const char * ArqNome();
        ///< Retorna o nome do arquivo principal
    static TArqIncluir * IncluirIni() { return Inicio; }
        ///< Retorna o primeiro objeto TIncluir
    TArqIncluir * IncluirProximo() { return Proximo; }
        ///< Retorna o pr�ximo objeto TIncluir ou 0 se n�o houver
    const char * IncluirNome() { return Padrao; }
        ///< Retorna o nome para in�cio de arquivo

private:
    static char * arqnome;      ///< Nome do arquivo principal
    char Padrao[ARQINCLUIR_TAM];///< Nome para in�cio de arquivo
    TArqIncluir *Anterior;      ///< Lista ligada; objeto anterior
    TArqIncluir *Proximo;       ///< Lista ligada; pr�ximo objeto
    static TArqIncluir *Inicio; ///< Lista ligada; primeiro objeto
    static TArqIncluir *Fim;    ///< Lista ligada; �ltimo objeto

    friend class TVarArqProg;
};

//----------------------------------------------------------------------------
class TVarArqProg
{
public:
    TVarArqProg() { Criar(); }
    ~TVarArqProg() { Apagar(); }
    void Criar();
            ///< Chamado ao criar objeto
    void Apagar();
            ///< Chamado ao apagar objeto
    void Abrir();  ///< Inicia a busca de arquivos
    void Fechar(); ///< Encerra a busca de arquivos
    void Proximo(); ///< Passa para a pr�xima entrada
    bool Func(TVariavel * v, const char * nome);
            ///< Fun��o da vari�vel

    char Arquivo[ARQINCLUIR_TAM]; ///< Nome da entrada encontrada, ou "" se nenhuma

private:
#ifdef __WIN32__
    HANDLE wdir; ///< Diret�rio sendo verificado, ou 0 se nenhum
#else
    DIR * dir; ///< Diret�rio sendo verificado, ou 0 se nenhum
#endif
    TArqIncluir * Incluir; // Pr�ximo diret�rio a buscar
    int ArqBarra;       // Primeiro caracter a partir da �ltima "/" no arquivo
    int ArqPadrao;      // Quantos caracteres comp�em o padr�o do arquivo
};

//----------------------------------------------------------------------------

#endif
