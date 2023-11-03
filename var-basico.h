#ifndef VAR_BASICO_H
#define VAR_BASICO_H

class TVariavel;
class TVarInfo;

//----------------------------------------------------------------------------
/// Variáveis txt1 a txt256
const TVarInfo * VarBaseTxt1();

/// Variáveis txt257 a txt512
const TVarInfo * VarBaseTxt2();

/// Variáveis int1
const TVarInfo * VarBaseInt1();

/// Variáveis int8
const TVarInfo * VarBaseInt8();

/// Variáveis uint8
const TVarInfo * VarBaseUInt8();

/// Variáveis int16
const TVarInfo * VarBaseInt16();

/// Variáveis uint16
const TVarInfo * VarBaseUInt16();

/// Variáveis int32
const TVarInfo * VarBaseInt32();

/// Variáveis uint32
const TVarInfo * VarBaseUInt32();

/// Variáveis real
const TVarInfo * VarBaseReal();

/// Variáveis real2
const TVarInfo * VarBaseReal2();

/// Altera valor numérico de vetor de int1
void SetVetorInt1(TVariavel * v, int valor);

//----------------------------------------------------------------------------

#endif
