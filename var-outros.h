#ifndef VAR_OUTROS_H
#define VAR_OUTROS_H

class TVariavel;

//----------------------------------------------------------------------------
/// Processa funções de vetores de txt1 a txt512
bool FuncVetorTxt(TVariavel * v, const char * nome);

/// Processa funções de vetores de variáveis int1
bool FuncVetorInt1(TVariavel * v, const char * nome);

/// Lê valor numérico de vetor de int1
int GetVetorInt1(TVariavel * v);

/// Altera valor numérico de vetor de int1
void SetVetorInt1(TVariavel * v, int valor);

/// Processa funções de vetores de variáveis int8
bool FuncVetorInt8(TVariavel * v, const char * nome);

/// Processa funções de vetores de variáveis uint8
bool FuncVetorUInt8(TVariavel * v, const char * nome);

/// Processa funções de vetores de variáveis int16
bool FuncVetorInt16(TVariavel * v, const char * nome);

/// Processa funções de vetores de variáveis uint16
bool FuncVetorUInt16(TVariavel * v, const char * nome);

/// Processa funções de vetores de variáveis int32
bool FuncVetorInt32(TVariavel * v, const char * nome);

/// Processa funções de vetores de variáveis uint32
bool FuncVetorUInt32(TVariavel * v, const char * nome);

/// Processa funções de vetores de variáveis real
bool FuncVetorReal(TVariavel * v, const char * nome);

/// Processa funções de vetores de variáveis real2
bool FuncVetorReal2(TVariavel * v, const char * nome);

//----------------------------------------------------------------------------

#endif
