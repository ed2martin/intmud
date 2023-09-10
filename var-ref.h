#ifndef VAR_REF_H
#define VAR_REF_H

class TObjeto;

//----------------------------------------------------------------------------
/** Trata das variáveis do tipo REF */
class TVarRef /// Variáveis REF
{
public:
    void MudarPont(TObjeto * obj); ///< Muda a variável TVarRef::Pont
    void Mover(TVarRef * destino); ///< Move TVarRef para outro lugar
    TObjeto * Pont;     ///< Objeto atual
    TVarRef * Antes;
    TVarRef * Depois;
};

//----------------------------------------------------------------------------

#endif
