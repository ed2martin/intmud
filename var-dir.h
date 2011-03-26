#ifndef VAR_DIR_H
#define VAR_DIR_H

#include <dirent.h>
#ifdef __WIN32__
#include <windows.h>
#endif

//----------------------------------------------------------------------------
class TVariavel;
class TVarDir /// Variável arqdir
{
public:
    void Criar();
            ///< Chamado ao criar objeto
    void Apagar();
            ///< Chamado ao apagar objeto
    bool Func(TVariavel * v, const char * nome);
            ///< Função da variável
    void Proximo(); ///< Passa para a próxima entrada
#ifdef __WIN32__
    HANDLE wdir; ///< Diretório sendo verificado, ou 0 se nenhum
#else
    DIR * dir; ///< Diretório sendo verificado, ou 0 se nenhum
#endif
    char arqdir[258]; ///< Nome da entrada encontrada, se dir!=0
    char arqtipo;     ///< Letra que indica o tipo de arquivo, se dir!=0
};

//----------------------------------------------------------------------------

#endif
