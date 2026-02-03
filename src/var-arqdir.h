#ifndef VAR_ARQDIR_H
#define VAR_ARQDIR_H

#include <dirent.h>
#ifdef _WIN32
#include <windows.h>
#endif

class TVariavel;
class TVarInfo;
class TClasse;
class TObjeto;

//----------------------------------------------------------------------------
class TVarArqDir /// Variável arqdir
{
public:
    static const TVarInfo * Inicializa();
            ///< Inicializa variável e retorna informações
    void Criar();
            ///< Chamado ao criar objeto
    void Apagar();
            ///< Chamado ao apagar objeto
    void Proximo(); ///< Passa para a próxima entrada
#ifdef _WIN32
    HANDLE wdir; ///< Diretório sendo verificado, ou 0 se nenhum
#else
    DIR * dir; ///< Diretório sendo verificado, ou 0 se nenhum
#endif
    char arqdir[258]; ///< Nome da entrada encontrada, se dir!=0
    char arqtipo;     ///< Letra que indica o tipo de arquivo, se dir!=0

private:
    static bool FuncLin(TVariavel * v);
    static bool FuncTexto(TVariavel * v);
    static bool FuncDepois(TVariavel * v);
    static bool FuncBarra(TVariavel * v);
    static bool FuncAbrir(TVariavel * v);
    static bool FuncFechar(TVariavel * v);
    static bool FuncTipo(TVariavel * v);
    static bool FuncTamanho(TVariavel * v);
    static bool FuncMtempo(TVariavel * v);
    static bool FuncAtempo(TVariavel * v);
    static bool FuncApagarDir(TVariavel * v);
    static bool FuncCriarDir(TVariavel * v);
    static bool FuncApagar(TVariavel * v);
    static bool FuncRenomear(TVariavel * v);

    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static void FRedim(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FMoverEnd(TVariavel * v, void * destino,
            TClasse * c, TObjeto * o);
};

//----------------------------------------------------------------------------

#endif
