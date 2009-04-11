#ifndef OBJETO_H
#define OBJETO_H

#include "classe.h"

class TVarRef;
class TListaX;

//----------------------------------------------------------------------------
/** Objetos  */
class TObjeto /// Objetos
{
    TObjeto();  ///< N�o deve ser chamado; usar TObjeto::Criar()
public:
    ~TObjeto(); ///< N�o deve ser chamado; usar TObjeto::MarcarApagar()
    static TObjeto * Criar(TClasse * c); ///< Cria objeto
    void Apagar();              ///< Apaga objeto @sa TObjeto::MarcarApagar
    void MarcarApagar();        ///< Marca objeto para apagar
    static void DesmarcarApagar(); ///< Desmarca objeto IniExcluir

    TClasse * Classe;           ///< Classe ao qual o objeto pertence
    TObjeto * Antes;            ///< Lista ligada: objeto anterior
    TObjeto * Depois;           ///< Lista ligada: pr�ximo objeto
    TObjeto * PontApagar;       ///< Pr�ximo objeto marcado para Apagar
    static TObjeto * IniApagar; ///< Primeiro objeto marcado para apagar
    static TObjeto * FimApagar; ///< �ltimo objeto marcado para apagar
    TVarRef * VarRefIni;        ///< Primeira TVarRef apontando para o objeto
    TListaX * VarListaX;        ///< Primeiro TListaX apontando para o objeto
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
