/* Este arquivo é software livre; você pode redistribuir e/ou
 * modificar nos termos da licença LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "variavel.h"
#include "var-outros.h"
#include "objeto.h"
#include "classe.h"
#include "instr.h"
#include "misc.h"

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
static void VarOutrosVarNome_Redim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    char * ref = reinterpret_cast<char*>(v->endvar);
    if (antes < depois)
        ref[antes * VAR_NOME_TAM] = 0;
}

//----------------------------------------------------------------------------
static void VarOutrosVarNome_MoverEnd(TVariavel * v, void * destino,
        TClasse * c, TObjeto * o)
{
    memmove(destino, v->endvar, VarOutrosVarNome_TamanhoVetor(v->defvar));
}

//----------------------------------------------------------------------------
static void VarOutrosTxtFixo_MoverEnd(TVariavel * v, void * destino,
        TClasse * c, TObjeto * o)
{
    memmove(destino, v->endvar, strlen((char*)v->endvar) + 1);
}

//------------------------------------------------------------------------------
static bool VarOutrosConstTxt_GetBool(TVariavel * v)
{
    const char * ref = v->defvar + v->defvar[Instr::endIndice] + 1;
    return ref[0] != 0;
}
static bool VarOutrosConstNum_GetBool(TVariavel * v)
{
    const char * origem = v->defvar + v->defvar[Instr::endIndice];
    switch (*origem)
    {
    case Instr::ex_num1:
        return true;
    case Instr::ex_num0:
        return false;
    case Instr::ex_num8n:
    case Instr::ex_num8p:
    case Instr::ex_num8hexn:
    case Instr::ex_num8hexp:
        return origem[1] != 0;
    case Instr::ex_num16n:
    case Instr::ex_num16p:
    case Instr::ex_num16hexn:
    case Instr::ex_num16hexp:
        return origem[1] != 0 || origem[2] != 0;
    case Instr::ex_num32n:
    case Instr::ex_num32p:
    case Instr::ex_num32hexn:
    case Instr::ex_num32hexp:
        return origem[1] != 0 || origem[2] != 0 ||
               origem[3] != 0 || origem[4] != 0;
    default:
        assert(0);
    }
}
static bool VarOutrosTxtFixo_GetBool(TVariavel * v)
{
    return reinterpret_cast<const char*>(v->endvar)[0] != 0;
}
static bool VarOutrosVarObjeto_GetBool(TVariavel * v)
{
    return v->endvar != nullptr;
}
static bool VarOutrosVarInt_GetBool(TVariavel * v)
{
    return v->valor_int != 0;
}
static bool VarOutrosVarDouble_GetBool(TVariavel * v)
{
    return v->valor_double != 0;
}

//------------------------------------------------------------------------------
static int VarOutrosConstTxt_GetInt(TVariavel * v)
{
    return TxtToInt(v->defvar + v->defvar[Instr::endIndice] + 1);
}
static int VarOutrosConstNum_GetInt(TVariavel * v)
{
    unsigned int valor = 0;
    bool negativo = false;
    const char * origem = v->defvar + v->defvar[Instr::endIndice];
    switch (*origem)
    {
    case Instr::ex_num1:
        valor = 1;
    case Instr::ex_num0:
        origem++;
        break;
    case Instr::ex_num8n:
    case Instr::ex_num8hexn:
        negativo = true;
    case Instr::ex_num8p:
    case Instr::ex_num8hexp:
        valor = (unsigned char)origem[1];
        origem += 2;
        break;
    case Instr::ex_num16n:
    case Instr::ex_num16hexn:
        negativo = true;
    case Instr::ex_num16p:
    case Instr::ex_num16hexp:
        valor = Num16(origem + 1);
        origem += 3;
        break;
    case Instr::ex_num32n:
    case Instr::ex_num32hexn:
        negativo = true;
    case Instr::ex_num32p:
    case Instr::ex_num32hexp:
        valor = Num32(origem + 1);
        origem += 5;
        break;
    default:
        assert(0);
    }
    while (*origem >= Instr::ex_div1 && *origem <= Instr::ex_div6)
        switch (*origem++)
        {
        case Instr::ex_div1: valor /= 10; break;
        case Instr::ex_div2: valor /= 100; break;
        case Instr::ex_div3: valor /= 1000; break;
        case Instr::ex_div4: valor /= 10000; break;
        case Instr::ex_div5: valor /= 100000; break;
        case Instr::ex_div6: valor /= 1000000; break;
        }
    if (negativo)
        return (valor < 0x80000000LL ? -valor : -0x80000000);
    return (valor < 0x7FFFFFFFLL ? valor : 0x7FFFFFFF);
}
static int VarOutrosTxtFixo_GetInt(TVariavel * v)
{
    return TxtToInt(reinterpret_cast<const char*>(v->endvar));
}
static int VarOutrosVarObjeto_GetInt(TVariavel * v)
{
    return v->endvar != nullptr;
}
static int VarOutrosVarInt_GetInt(TVariavel * v)
{
    return v->valor_int;
}
static int VarOutrosVarDouble_GetInt(TVariavel * v)
{
    return DoubleToInt(v->valor_double);
}

//------------------------------------------------------------------------------
static double VarOutrosConstTxt_GetDouble(TVariavel * v)
{
    return TxtToDouble(v->defvar + v->defvar[Instr::endIndice] + 1);
}
static double VarOutrosConstNum_GetDouble(TVariavel * v)
{
    double valor = 0;
    bool negativo = false;
    const char * origem = v->defvar + v->defvar[Instr::endIndice];
    switch (*origem)
    {
    case Instr::ex_num1:
        valor = 1;
    case Instr::ex_num0:
        origem++;
        break;
    case Instr::ex_num8n:
    case Instr::ex_num8hexn:
        negativo = true;
    case Instr::ex_num8p:
    case Instr::ex_num8hexp:
        valor = (unsigned char)origem[1];
        origem += 2;
        break;
    case Instr::ex_num16n:
    case Instr::ex_num16hexn:
        negativo = true;
    case Instr::ex_num16p:
    case Instr::ex_num16hexp:
        valor=Num16(origem + 1);
        origem += 3;
        break;
    case Instr::ex_num32n:
    case Instr::ex_num32hexn:
        negativo = true;
    case Instr::ex_num32p:
    case Instr::ex_num32hexp:
        valor = Num32(origem + 1);
        origem += 5;
        break;
    default:
        assert(0);
    }
    while (*origem >= Instr::ex_div1 && *origem <= Instr::ex_div6)
        switch (*origem++)
        {
        case Instr::ex_div1: valor /= 10; break;
        case Instr::ex_div2: valor /= 100; break;
        case Instr::ex_div3: valor /= 1000; break;
        case Instr::ex_div4: valor /= 10000; break;
        case Instr::ex_div5: valor /= 100000; break;
        case Instr::ex_div6: valor /= 1000000; break;
        }
    return (negativo ? -valor : valor);
}
static double VarOutrosTxtFixo_GetDouble(TVariavel * v)
{
    return TxtToDouble(reinterpret_cast<const char*>(v->endvar));
}
static double VarOutrosVarObjeto_GetDouble(TVariavel * v)
{
    return v->endvar != nullptr;
}
static double VarOutrosVarInt_GetDouble(TVariavel * v)
{
    return v->valor_int;
}
static double VarOutrosVarDouble_GetDouble(TVariavel * v)
{
    return v->valor_double;
}

//------------------------------------------------------------------------------
static const char * VarOutrosConstTxt_GetTxt(TVariavel * v)
{
    return v->defvar + v->defvar[Instr::endIndice] + 1;
}
static const char * VarOutrosConstNum_GetTxt(TVariavel * v)
{
    unsigned int valor = 0; // Valor numérico sem sinal
    bool negativo = false; // Se é negativo
    int  virgula = 0;   // Casas após a vírgula
    const char * origem = v->defvar + v->defvar[Instr::endIndice];
// Acerta variáveis valor e negativo
    switch (*origem)
    {
    case Instr::ex_num1:
        valor = 1;
    case Instr::ex_num0:
        origem++;
        break;
    case Instr::ex_num8n:
    case Instr::ex_num8hexn:
        negativo = true;
    case Instr::ex_num8p:
    case Instr::ex_num8hexp:
        valor=(unsigned char)origem[1];
        origem += 2;
        break;
    case Instr::ex_num16n:
    case Instr::ex_num16hexn:
        negativo = true;
    case Instr::ex_num16p:
    case Instr::ex_num16hexp:
        valor = Num16(origem + 1);
        origem += 3;
        break;
    case Instr::ex_num32n:
    case Instr::ex_num32hexn:
        negativo = true;
    case Instr::ex_num32p:
    case Instr::ex_num32hexp:
        valor = Num32(origem + 1);
        origem += 5;
        break;
    }
// Acerta variável virgula
    while (*origem >= Instr::ex_div1 && *origem <= Instr::ex_div6)
        switch (*origem++)
        {
        case Instr::ex_div1: virgula++; break;
        case Instr::ex_div2: virgula += 2; break;
        case Instr::ex_div3: virgula += 3; break;
        case Instr::ex_div4: virgula += 4; break;
        case Instr::ex_div5: virgula += 5; break;
        case Instr::ex_div6: virgula += 6; break;
        }
    if (virgula > 60)
        virgula = 60;
// Zero é sempre zero
    if (valor == 0)
        return "0";
// Obtém em nome a string correspondente ao número
    char mens[80];
    char * d = mens;
    char * buf = TVarInfo::BufferTxt();
    char * destino = buf;
    while (valor > 0)
        *d++ = valor % 10 + '0', valor /= 10;
// Obtém o número de dígitos
    int digitos = d-mens;
    if (digitos <= virgula)
        digitos = virgula+1;
// Anota o número
    if (negativo)
        *destino++ = '-';
    while (digitos > 0)
    {
        if (digitos == virgula)
            *destino++ = '.';
        digitos--;
        *destino++ = (&mens[digitos] >= d ? '0' : mens[digitos]);
    }
    *destino = 0;
    return buf;
}
static const char * VarOutrosTxtFixo_GetTxt(TVariavel * v)
{
    return reinterpret_cast<const char*>(v->endvar);
}
static const char * VarOutrosVarNome_GetTxt(TVariavel * v)
{
    return reinterpret_cast<const char*>(v->endvar);
}
static const char * VarOutrosVarClasse_GetTxt(TVariavel * v)
{
    TClasse * obj = reinterpret_cast<TClasse*>(v->endvar);
    return (obj == nullptr ? "" : obj->Nome);
}
static const char * VarOutrosVarObjeto_GetTxt(TVariavel * v)
{
    TObjeto * obj = reinterpret_cast<TObjeto*>(v->endvar);
    return (obj == nullptr ? "" : obj->Classe->Nome);
}
static const char * VarOutrosVarInt_GetTxt(TVariavel * v)
{
    char * buf = TVarInfo::BufferTxt();
    sprintf(buf, "%d", v->valor_int);
    return buf;
}
static const char * VarOutrosVarDouble_GetTxt(TVariavel * v)
{
    char * buf = TVarInfo::BufferTxt();
    DoubleToTxt(buf, v->valor_double);
    return buf;
}

//------------------------------------------------------------------------------
static TObjeto * VarOutrosVarObjeto_GetObj(TVariavel * v)
{
    return reinterpret_cast<TObjeto*>(v->endvar);
}

//----------------------------------------------------------------------------
static void VarOutrosVarNome_OperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    char * ref = reinterpret_cast<char*>(v1->endvar) + v1->indice * VAR_NOME_TAM;
    const char * origem = v2->getTxt();
    if (ref != origem)
        copiastr(ref, origem, VAR_NOME_TAM);
}
static void VarOutrosVarIniFunc_OperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    v1->valor_int = v2->getInt();
}
static void VarOutrosVarObjeto_OperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    v1->endvar = v2->getObj();
}
static void VarOutrosVarInt_OperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    v1->valor_int = v2->getInt();
}
static void VarOutrosVarDouble_OperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    v1->valor_double = v2->getDouble();
}

//----------------------------------------------------------------------------
static bool VarOutrosVarNome_OperadorAddFalse(TVariavel * v1, TVariavel * v2)
{
    char * ref = reinterpret_cast<char*>(v1->endvar) + v1->indice * VAR_NOME_TAM;
    const char * origem = v2->getTxt();
    int desloc = strlen(ref);
    int tam = VAR_NOME_TAM - desloc;
    if (origem != ref)
        copiastr(ref + desloc, origem, tam);
    else
        copiastr(ref + desloc, origem, tam < desloc + 1 ? tam : desloc + 1);
    return true;
}

//----------------------------------------------------------------------------
static bool VarOutrosConstNulo_OperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    return v2->Tipo() == varObj && v2->getObj() == nullptr;
}
static bool VarOutrosConstTxt_OperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    const char * ref = v1->defvar + v1->defvar[Instr::endIndice] + 1;
    return v2->Tipo() == varTxt && strcmp(ref, v2->getTxt()) == 0;
}
static bool VarOutrosConstNum_OperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    return v2->TipoNumerico() &&
            VarOutrosConstNum_GetDouble(v1) == v2->getDouble();
}
static bool VarOutrosTxtFixo_OperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    const char * ref = reinterpret_cast<const char*>(v1->endvar);
    return v2->Tipo() == varTxt && strcmp(ref, v2->getTxt()) == 0;
}
static bool VarOutrosVarNome_OperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    const char * ref = reinterpret_cast<const char*>(v1->endvar);
    return v2->Tipo() == varTxt && strcmp(ref, v2->getTxt()) == 0;
}
static bool VarOutrosVarInt_OperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    return v2->TipoNumerico() && v1->valor_int == v2->getDouble();
}
static bool VarOutrosVarDouble_OperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    return v2->TipoNumerico() && v1->valor_double == v2->getDouble();
}
static bool VarOutrosVarClasse_OperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    TClasse * obj = reinterpret_cast<TClasse*>(v1->endvar);
    const char * ref = (obj == nullptr ? "" : obj->Nome);
    return v2->Tipo() == varTxt && strcmp(ref, v2->getTxt()) == 0;
}
static bool VarOutrosVarObjeto_OperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    if (v2->Tipo() != varObj)
        return 0;
    TObjeto * obj1 = reinterpret_cast<TObjeto*>(v1->endvar);
    return obj1 == v2->getObj();
}

//----------------------------------------------------------------------------
static unsigned char VarOutrosConstNulo_OperadorCompara(TVariavel * v1, TVariavel * v2)
{
    return v2->Tipo() == varObj && v2->getObj() == nullptr ? 2 : 0;
}
static unsigned char VarOutrosConstTxt_OperadorCompara(TVariavel * v1, TVariavel * v2)
{
    const char * ref = v1->defvar + v1->defvar[Instr::endIndice] + 1;
    return TVarInfo::ComparaTxt(ref, v2->getTxt());
}
static unsigned char VarOutrosConstNum_OperadorCompara(TVariavel * v1, TVariavel * v2)
{
    return TVarInfo::ComparaDouble(VarOutrosConstNum_GetDouble(v1), v2);
}
static unsigned char VarOutrosTxtFixo_OperadorCompara(TVariavel * v1, TVariavel * v2)
{
    const char * ref = reinterpret_cast<const char*>(v1->endvar);
    return TVarInfo::ComparaTxt(ref, v2->getTxt());
}
static unsigned char VarOutrosVarNome_OperadorCompara(TVariavel * v1, TVariavel * v2)
{
    const char * ref = reinterpret_cast<const char*>(v1->endvar);
    return TVarInfo::ComparaTxt(ref, v2->getTxt());
}
static unsigned char VarOutrosVarInt_OperadorCompara(TVariavel * v1, TVariavel * v2)
{
    return TVarInfo::ComparaInt(v1->valor_int, v2);
}
static unsigned char VarOutrosVarDouble_OperadorCompara(TVariavel * v1, TVariavel * v2)
{
    return TVarInfo::ComparaDouble(v1->valor_double, v2);
}
static unsigned char VarOutrosVarClasse_OperadorCompara(TVariavel * v1, TVariavel * v2)
{
    TClasse * obj = reinterpret_cast<TClasse*>(v1->endvar);
    const char * ref = (obj == nullptr ? "" : obj->Nome);
    return TVarInfo::ComparaTxt(ref, v2->getTxt());
}
static unsigned char VarOutrosVarObjeto_OperadorCompara(TVariavel * v1, TVariavel * v2)
{
    if (v2->Tipo() != varObj)
        return 0;
    TObjeto * obj1 = reinterpret_cast<TObjeto*>(v1->endvar);
    TObjeto * obj2 = v2->getObj();
    return obj1 == obj2 ? 2 : obj1 < obj2 ? 1 : 4;
}

//----------------------------------------------------------------------------
const TVarInfo * VarOutrosConstNulo()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoObj,
        TVarInfo::FRedim0,
        TVarInfo::FMoverEnd0,
        TVarInfo::FMoverDef0,
        TVarInfo::FGetBoolFalse,
        TVarInfo::FGetInt0,
        TVarInfo::FGetDouble0,
        TVarInfo::FGetTxtVazio,
        TVarInfo::FGetObjNulo,
        TVarInfo::FOperadorAtribVazio,
        TVarInfo::FOperadorAddFalse,
        VarOutrosConstNulo_OperadorIgual2,
        VarOutrosConstNulo_OperadorCompara,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosConstTxt()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoTxt,
        TVarInfo::FRedim0,
        TVarInfo::FMoverEnd0,
        TVarInfo::FMoverDef0,
        VarOutrosConstTxt_GetBool,
        VarOutrosConstTxt_GetInt,
        VarOutrosConstTxt_GetDouble,
        VarOutrosConstTxt_GetTxt,
        TVarInfo::FGetObjNulo,
        TVarInfo::FOperadorAtribVazio,
        TVarInfo::FOperadorAddFalse,
        VarOutrosConstTxt_OperadorIgual2,
        VarOutrosConstTxt_OperadorCompara,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosConstNum()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoDouble,
        TVarInfo::FRedim0,
        TVarInfo::FMoverEnd0,
        TVarInfo::FMoverDef0,
        VarOutrosConstNum_GetBool,
        VarOutrosConstNum_GetInt,
        VarOutrosConstNum_GetDouble,
        VarOutrosConstNum_GetTxt,
        TVarInfo::FGetObjNulo,
        TVarInfo::FOperadorAtribVazio,
        TVarInfo::FOperadorAddFalse,
        VarOutrosConstNum_OperadorIgual2,
        VarOutrosConstNum_OperadorCompara,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosConstExpr()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoOutros,
        TVarInfo::FRedim0,
        TVarInfo::FMoverEnd0,
        TVarInfo::FMoverDef0,
        TVarInfo::FGetBoolFalse,
        TVarInfo::FGetInt0,
        TVarInfo::FGetDouble0,
        TVarInfo::FGetTxtVazio,
        TVarInfo::FGetObjNulo,
        TVarInfo::FOperadorAtribVazio,
        TVarInfo::FOperadorAddFalse,
        TVarInfo::FOperadorIgual2Var,
        TVarInfo::FOperadorComparaVar,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosConstVar()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoOutros,
        TVarInfo::FRedim0,
        TVarInfo::FMoverEnd0,
        TVarInfo::FMoverDef0,
        TVarInfo::FGetBoolFalse,
        TVarInfo::FGetInt0,
        TVarInfo::FGetDouble0,
        TVarInfo::FGetTxtVazio,
        TVarInfo::FGetObjNulo,
        TVarInfo::FOperadorAtribVazio,
        TVarInfo::FOperadorAddFalse,
        TVarInfo::FOperadorIgual2Var,
        TVarInfo::FOperadorComparaVar,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosFunc()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoOutros,
        TVarInfo::FRedim0,
        TVarInfo::FMoverEnd0,
        TVarInfo::FMoverDef0,
        TVarInfo::FGetBoolFalse,
        TVarInfo::FGetInt0,
        TVarInfo::FGetDouble0,
        TVarInfo::FGetTxtVazio,
        TVarInfo::FGetObjNulo,
        TVarInfo::FOperadorAtribVazio,
        TVarInfo::FOperadorAddFalse,
        TVarInfo::FOperadorIgual2Var,
        TVarInfo::FOperadorComparaVar,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosVarFunc()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoOutros,
        TVarInfo::FRedim0,
        TVarInfo::FMoverEnd0,
        TVarInfo::FMoverDef0,
        TVarInfo::FGetBoolFalse,
        TVarInfo::FGetInt0,
        TVarInfo::FGetDouble0,
        TVarInfo::FGetTxtVazio,
        TVarInfo::FGetObjNulo,
        TVarInfo::FOperadorAtribVazio,
        TVarInfo::FOperadorAddFalse,
        TVarInfo::FOperadorIgual2Var,
        TVarInfo::FOperadorComparaVar,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosTxtFixo()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoTxt,
        TVarInfo::FRedim0,
        VarOutrosTxtFixo_MoverEnd,
        TVarInfo::FMoverDef0,
        VarOutrosTxtFixo_GetBool,
        VarOutrosTxtFixo_GetInt,
        VarOutrosTxtFixo_GetDouble,
        VarOutrosTxtFixo_GetTxt,
        TVarInfo::FGetObjNulo,
        TVarInfo::FOperadorAtribVazio,
        TVarInfo::FOperadorAddFalse,
        VarOutrosTxtFixo_OperadorIgual2,
        VarOutrosTxtFixo_OperadorCompara,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosVarNome()
{
    static const TVarInfo var(
        VarOutrosVarNome_Tamanho,
        VarOutrosVarNome_TamanhoVetor,
        TVarInfo::FTipoTxt,
        VarOutrosVarNome_Redim,
        VarOutrosVarNome_MoverEnd,
        TVarInfo::FMoverDef0,
        TVarInfo::FGetBoolFalse,
        TVarInfo::FGetInt0,
        TVarInfo::FGetDouble0,
        VarOutrosVarNome_GetTxt,
        TVarInfo::FGetObjNulo,
        VarOutrosVarNome_OperadorAtrib,
        VarOutrosVarNome_OperadorAddFalse,
        VarOutrosVarNome_OperadorIgual2,
        VarOutrosVarNome_OperadorCompara,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosVarInicio()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoOutros,
        TVarInfo::FRedim0,
        TVarInfo::FMoverEnd0,
        TVarInfo::FMoverDef0,
        TVarInfo::FGetBoolFalse,
        TVarInfo::FGetInt0,
        TVarInfo::FGetDouble0,
        TVarInfo::FGetTxtVazio,
        TVarInfo::FGetObjNulo,
        TVarInfo::FOperadorAtribVazio,
        TVarInfo::FOperadorAddFalse,
        TVarInfo::FOperadorIgual2Var,
        TVarInfo::FOperadorComparaVar,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosVarIniFunc()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoInt,
        TVarInfo::FRedim0,
        TVarInfo::FMoverEnd0,
        TVarInfo::FMoverDef0,
        VarOutrosVarInt_GetBool,
        VarOutrosVarInt_GetInt,
        VarOutrosVarInt_GetDouble,
        VarOutrosVarInt_GetTxt,
        TVarInfo::FGetObjNulo,
        VarOutrosVarIniFunc_OperadorAtrib,
        TVarInfo::FOperadorAddFalse,
        VarOutrosVarInt_OperadorIgual2,
        VarOutrosVarInt_OperadorCompara,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosVarClasse()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoTxt,
        TVarInfo::FRedim0,
        TVarInfo::FMoverEnd0,
        TVarInfo::FMoverDef0,
        TVarInfo::FGetBoolFalse,
        TVarInfo::FGetInt0,
        TVarInfo::FGetDouble0,
        VarOutrosVarClasse_GetTxt,
        TVarInfo::FGetObjNulo,
        TVarInfo::FOperadorAtribVazio,
        TVarInfo::FOperadorAddFalse,
        VarOutrosVarClasse_OperadorIgual2,
        VarOutrosVarClasse_OperadorCompara,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosVarObjeto()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoObj,
        TVarInfo::FRedim0,
        TVarInfo::FMoverEnd0,
        TVarInfo::FMoverDef0,
        VarOutrosVarObjeto_GetBool,
        VarOutrosVarObjeto_GetInt,
        VarOutrosVarObjeto_GetDouble,
        VarOutrosVarObjeto_GetTxt,
        VarOutrosVarObjeto_GetObj,
        VarOutrosVarObjeto_OperadorAtrib,
        TVarInfo::FOperadorAddFalse,
        VarOutrosVarObjeto_OperadorIgual2,
        VarOutrosVarObjeto_OperadorCompara,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosVarInt()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoInt,
        TVarInfo::FRedim0,
        TVarInfo::FMoverEnd0,
        TVarInfo::FMoverDef0,
        VarOutrosVarInt_GetBool,
        VarOutrosVarInt_GetInt,
        VarOutrosVarInt_GetDouble,
        VarOutrosVarInt_GetTxt,
        TVarInfo::FGetObjNulo,
        VarOutrosVarInt_OperadorAtrib,
        TVarInfo::FOperadorAddFalse,
        VarOutrosVarInt_OperadorIgual2,
        VarOutrosVarInt_OperadorCompara,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

const TVarInfo * VarOutrosVarDouble()
{
    static const TVarInfo var(
        TVarInfo::FTamanho0,
        TVarInfo::FTamanho0,
        TVarInfo::FTipoDouble,
        TVarInfo::FRedim0,
        TVarInfo::FMoverEnd0,
        TVarInfo::FMoverDef0,
        VarOutrosVarDouble_GetBool,
        VarOutrosVarDouble_GetInt,
        VarOutrosVarDouble_GetDouble,
        VarOutrosVarDouble_GetTxt,
        TVarInfo::FGetObjNulo,
        VarOutrosVarDouble_OperadorAtrib,
        TVarInfo::FOperadorAddFalse,
        VarOutrosVarDouble_OperadorIgual2,
        VarOutrosVarDouble_OperadorCompara,
        TVarInfo::FFuncVetorFalse);
    return &var;
}
