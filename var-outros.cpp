/* Este arquivo é software livre; você pode redistribuir e/ou
 * modificar nos termos das licenças GPL ou LGPL. Vide arquivos
 * COPYING e COPYING2.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GPL or the LGP licenses.
 * See files COPYING e COPYING2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "variavel.h"
#include "var-outros.h"
#include "instr.h"

//----------------------------------------------------------------------------
static int VarOutrosVarNome_Tamanho(const char * instr)
{
    return VAR_NOME_TAM;
}
static int VarOutrosVarNome_TamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * VAR_NOME_TAM;
}

//----------------------------------------------------------------------------
const TVarInfo * VarOutrosConstNulo()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoObj,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosConstTxt()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoTxt,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosConstNum()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoDouble,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosConstExpr()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoOutros,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosConstVar()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoOutros,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosFunc()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoOutros,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosVarFunc()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoOutros,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosTxtFixo()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoTxt,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosVarNome()
{
    static const TVarInfo var(
        VarOutrosVarNome_Tamanho,
        VarOutrosVarNome_TamanhoVetor,
        TVarInfo::FTipoTxt,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosVarInicio()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoOutros,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosVarIniFunc()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoInt,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosVarClasse()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoTxt,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosVarObjeto()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoObj,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosVarInt()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoInt,
        TVarInfo::FFuncVetorFalse);
    return &var;
}
