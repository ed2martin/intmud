#ifndef OBJETO_H
#define OBJETO_H

#include "classe.h"

//----------------------------------------------------------------------------
/** Objetos  */
class TObjeto /// Objetos
{
    TObjeto();  ///< Não deve ser chamado; usar TObjeto::Criar()
public:
    ~TObjeto(); ///< Não deve ser chamado; usar TObjeto::Apagar()
    static TObjeto * Criar(TClasse * c); ///< Cria objeto
    void Apagar();              ///< Apaga objeto

    TClasse * Classe;           ///< Classe ao qual o objeto pertence
    TObjeto * Antes;            ///< Lista ligada: objeto anterior
    TObjeto * Depois;           ///< Lista ligada: próximo objeto
    TObjeto * Excluir;          ///< Próximo objeto marcado para exclusão
    static TObjeto * IniExcluir;///< Primeiro objeto marcado para exclusão
    unsigned char Vars[4];      ///< Variáveis do objeto
        /**< O tamanho real de Vars, em bytes, é Classe->TamObj
         *
         *  Isso torna-se necessário porque o espaço ocupado pelas
         *  variáveis pode mudar de classe para classe. É mais eficiente
         *  alocar memória para o objeto junto com as variáveis.
         *  @sa TClasse::TamObj
         */
};

#endif
