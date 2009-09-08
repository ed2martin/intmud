#ifndef OBJETO_H
#define OBJETO_H

#include "classe.h"

class TVarRef;
class TListaX;

//----------------------------------------------------------------------------
/** Objetos  */
class TObjeto /// Objetos
{
    TObjeto();  ///< Não deve ser chamado; usar TObjeto::Criar()
public:
    ~TObjeto(); ///< Não deve ser chamado; usar TObjeto::MarcarApagar()
    static TObjeto * Criar(TClasse * c, bool criavar=true);
        ///< Cria objeto
        /**< @param c Classe ao qual o objeto pertence
             @param criavar Se deve chamar construtores das variáveis
             @return Endereço do objeto criado */
    void Apagar();
        ///< Apaga objeto @sa TObjeto::MarcarApagar
    void Apagar(TObjeto * obj);
        ///< Apaga objeto mudando referências obj
        /**< Não chama destrutores das variáveis.
         *   É usado por TClasse::AcertaVar()
         *   @param obj Novo objeto */
    void MarcarApagar();        ///< Marca objeto para apagar
    static void DesmarcarApagar(); ///< Desmarca objeto IniExcluir

    TClasse * Classe;           ///< Classe ao qual o objeto pertence
    TObjeto * Antes;            ///< Lista ligada: objeto anterior
    TObjeto * Depois;           ///< Lista ligada: próximo objeto
    TObjeto * PontApagar;       ///< Próximo objeto marcado para Apagar
    static TObjeto * IniApagar; ///< Primeiro objeto marcado para apagar
    static TObjeto * FimApagar; ///< Último objeto marcado para apagar
    TVarRef * VarRefIni;        ///< Primeira TVarRef apontando para o objeto
    TListaX * VarListaX;        ///< Primeiro TListaX apontando para o objeto
    unsigned short NumeroSav;   ///< Número; usado em var-sav.cpp, em "salvar"
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
