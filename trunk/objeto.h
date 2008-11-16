#ifndef OBJETO_H
#define OBJETO_H

#include "classe.h"

//----------------------------------------------------------------------------
/** Objetos  */
class TObjeto /// Objetos
{
    TObjeto();  ///< N�o deve ser chamado; usar TObjeto::Criar()
public:
    ~TObjeto(); ///< N�o deve ser chamado; usar TObjeto::Apagar()
    static TObjeto * Criar(TClasse * c); ///< Cria objeto
    void Apagar();              ///< Apaga objeto

    TClasse * Classe;           ///< Classe ao qual o objeto pertence
    TObjeto * Antes;            ///< Lista ligada: objeto anterior
    TObjeto * Depois;           ///< Lista ligada: pr�ximo objeto
    TObjeto * Excluir;          ///< Pr�ximo objeto marcado para exclus�o
    static TObjeto * IniExcluir;///< Primeiro objeto marcado para exclus�o
    unsigned char Vars[4];      ///< Vari�veis do objeto
        /**< O tamanho real de Vars, em bytes, � Classe->TamObj
         *
         *  Isso torna-se necess�rio porque o espa�o ocupado pelas
         *  vari�veis pode mudar de classe para classe. � mais eficiente
         *  alocar mem�ria para o objeto junto com as vari�veis.
         *  @sa TClasse::TamObj
         */
};

#endif
