#ifndef VAR_TXT_H
#define VAR_TXT_H

#include <stdio.h>

//----------------------------------------------------------------------------
class TVariavel;
class TVarTxt /// Variável arqtxt
{
public:
    void Criar();           ///< Chamado ao criar objeto
    void Apagar();          ///< Apaga objeto
    int  getValor();        ///< Ler valor numérico da variável
    bool Func(TVariavel * v, const char * nome); ///< Função da variável
private:
    void Fechar();      ///< Fecha arquivo
    FILE * arq;         ///< Para acessar o arquivo
    bool ModoBinario;   ///< Se está acessando o arquivo em modo binário
};

//----------------------------------------------------------------------------

#endif
