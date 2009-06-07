#ifndef VAR_TXT_H
#define VAR_TXT_H

#include <stdio.h>

//----------------------------------------------------------------------------
class TVariavel;
class TVarTxt /// Vari�vel arqtxt
{
public:
    void Criar();           ///< Chamado ao criar objeto
    void Apagar();          ///< Apaga objeto
    int  getValor();        ///< Ler valor num�rico da vari�vel
    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
private:
    void Fechar();      ///< Fecha arquivo
    FILE * arq;         ///< Para acessar o arquivo
};

//----------------------------------------------------------------------------

#endif
