/* Este arquivo � software livre; voc� pode redistribuir e/ou
 * modificar nos termos da licen�a LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "var-textovar.h"
#include "variavel.h"
#include "variavel-def.h"
#include "classe.h"
#include "objeto.h"
#include "instr.h"
#include "misc.h"

TTextoVar * TTextoVar::TextoAtual = nullptr;
TBlocoVarDec ** TBlocoVarDec::VetMenos = nullptr;
TBlocoVarDec ** TBlocoVarDec::VetMais = nullptr;
unsigned int TBlocoVarDec::TempoMenos = 0;
unsigned int TBlocoVarDec::TempoMais = 0;

//----------------------------------------------------------------------------
const TVarInfo * TTextoVar::Inicializa()
{
    TBlocoVarDec::PreparaIni();
    static TVarInfo::FuncItem ListaFuncEnd[] = {
        { "antes",        &TTextoVar::FuncAntes },
        { "depois",       &TTextoVar::FuncDepois },
        { "fim",          &TTextoVar::FuncFim },
        { "ini",          &TTextoVar::FuncIni },
        { "limpar",       &TTextoVar::FuncLimpar },
        { "mudar",        &TTextoVar::FuncMudar },
        { "nomevar",      &TTextoVar::FuncNomeVar },
        { "tipo",         &TTextoVar::FuncTipo },
        { "total",        &TTextoVar::FuncTotal },
        { "valor",        &TTextoVar::FuncValor },
        { "valorfim",     &TTextoVar::FuncValorFim },
        { "valorini",     &TTextoVar::FuncValorIni } };
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        TVarInfo::FTipoOutros,
        FRedim,
        FMoverEnd,
        TVarInfo::FMoverDef0,
        TVarInfo::FGetBoolFalse,
        TVarInfo::FGetInt0,
        TVarInfo::FGetDouble0,
        TVarInfo::FGetTxtVazio,
        TVarInfo::FGetObjNulo,
        FOperadorAtrib,
        TVarInfo::FOperadorAddFalse,
        FOperadorIgual2,
        FOperadorCompara,
        FFuncTexto,
        TVarInfo::FFuncVetorFalse,
        ListaFuncEnd,
        sizeof(ListaFuncEnd) / sizeof(ListaFuncEnd[0]) - 1);
    return &var;
}

//----------------------------------------------------------------------------
bool TTextoVar::FFuncTexto(TVariavel * v, const char * nome)
{
    if (nome[0] == 0)
        return false;
    TTextoVar * ref = reinterpret_cast<TTextoVar*>(v->endvar) + v->indice;
    ref->TextoAtual = ref;
    Instr::ApagarVar(v);
    if (ref->TextoAtual == nullptr)
        return false;
    return ref->CriarTextoVarSub(nome);
}

//----------------------------------------------------------------------------
bool TTextoVar::CriarTextoVarSub(const char * nome)
{
    if (!Instr::CriarVar(Instr::InstrVarTextoVarSub))
        return false;
    TTextoVarSub * sub = reinterpret_cast<TTextoVarSub*>(Instr::VarAtual->endvar);
    sub->Criar(this, nome, true);
    switch (sub->TipoVar)
    {
    case TextoVarTipoTxt: Instr::VarAtual->numfunc = varTxt; break;
    case TextoVarTipoNum: Instr::VarAtual->numfunc = varDouble; break;
    case TextoVarTipoDec: Instr::VarAtual->numfunc = varInt; break;
    case TextoVarTipoRef: Instr::VarAtual->numfunc = varObj; break;
    default: Instr::VarAtual->numfunc = varOutros;
    }
    return true;
}

//----------------------------------------------------------------------------
bool TTextoVar::CriarTextoVarSub(TBlocoVar * bl)
{
    if (!Instr::CriarVar(Instr::InstrVarTextoVarSub))
        return false;
    TTextoVarSub * sub = reinterpret_cast<TTextoVarSub*>(Instr::VarAtual->endvar);
    sub->Criar(this, bl->NomeVar, false);
    sub->TipoVar = bl->TipoVar();
    switch (sub->TipoVar)
    {
    case TextoVarTipoTxt: Instr::VarAtual->numfunc = varTxt; break;
    case TextoVarTipoNum: Instr::VarAtual->numfunc = varDouble; break;
    case TextoVarTipoDec: Instr::VarAtual->numfunc = varInt; break;
    case TextoVarTipoRef: Instr::VarAtual->numfunc = varObj; break;
    default: Instr::VarAtual->numfunc = varOutros;
    }
    return true;
}

//----------------------------------------------------------------------------
// Vari�vel
bool TTextoVar::FuncValor(TVariavel * v)
{
    if (Instr::VarAtual < v + 1)
        return false;
    TTextoVar * ref = reinterpret_cast<TTextoVar*>(v->endvar) + v->indice;
    TextoAtual = ref;
    char mens[VAR_NOME_TAM];
    copiastr(mens, v[1].getTxt(), sizeof(mens));
    if (mens[0] == 0)
        return false;
    Instr::ApagarVar(v);
    if (ref->TextoAtual == nullptr)
        return false;
    return ref->CriarTextoVarSub(mens);
}

//----------------------------------------------------------------------------
// Primeira vari�vel
bool TTextoVar::FuncValorIni(TVariavel * v)
{
    TTextoVar * ref = reinterpret_cast<TTextoVar*>(v->endvar) + v->indice;
    TBlocoVar * bl = nullptr;
    if (ref->RBroot)
    {
        if (Instr::VarAtual < v + 1)
            bl = ref->RBroot->RBfirst();
        else
            bl = ref->ProcIni(v[1].getTxt());
    }
    if (bl == nullptr)
        return false;
    ref->TextoAtual = ref;
    Instr::ApagarVar(v);
    if (ref->TextoAtual == nullptr)
        return false;
    return ref->CriarTextoVarSub(bl);
}

//----------------------------------------------------------------------------
// �ltima vari�vel
bool TTextoVar::FuncValorFim(TVariavel * v)
{
    TTextoVar * ref = reinterpret_cast<TTextoVar*>(v->endvar) + v->indice;
    TBlocoVar * bl = nullptr;
    if (ref->RBroot)
    {
        if (Instr::VarAtual < v + 1)
            bl = ref->RBroot->RBlast();
        else
            bl = ref->ProcFim(v[1].getTxt());
    }
    if (bl == nullptr)
        return false;
    ref->TextoAtual = ref;
    Instr::ApagarVar(v);
    if (ref->TextoAtual == nullptr)
        return false;
    return ref->CriarTextoVarSub(bl);
}

//----------------------------------------------------------------------------
// Mudar vari�vel
bool TTextoVar::FuncMudar(TVariavel * v)
{
    if (Instr::VarAtual < v + 1)
        return false;
    TTextoVar * ref = reinterpret_cast<TTextoVar*>(v->endvar) + v->indice;
    TTextoVarSub sub;
    if (Instr::VarAtual > v + 1)
    {
        sub.Criar(ref, v[1].getTxt(), true);
        switch (sub.TipoVar)
        {
        case TextoVarTipoNum:
            sub.setDouble(v[2].getDouble());
            break;
        case TextoVarTipoDec:
            sub.setInt(v[2].getInt());
            break;
        case TextoVarTipoRef:
            sub.setObj(v[2].getObj());
            break;
        case TextoVarTipoTxt:
        default:
            sub.setTxt(v[2].getTxt());
            break;
        }
    }
    else
    {
        char nome[VAR_NOME_TAM];
        const char * o = v[1].getTxt();
        char * d = nome;
        while (*o)
        {
            char ch = *o++;
            if (ch == '=')
                break;
            *d++ = ch;
            if (d >= nome + sizeof(nome) - 1)
                return false;
        }
        *d = 0;
        sub.Criar(ref, nome, true);
        sub.setTxt(o);
    }
    sub.Apagar();
    return false;
}

//----------------------------------------------------------------------------
// Nome da vari�vel
bool TTextoVar::FuncNomeVar(TVariavel * v)
{
    TTextoVar * ref = reinterpret_cast<TTextoVar*>(v->endvar) + v->indice;
    TBlocoVar * bl = nullptr;
    if (Instr::VarAtual >= v + 1)
        bl = ref->Procura(v[1].getTxt());
    ref->TextoAtual = ref;
    Instr::ApagarVar(v);
    if (ref->TextoAtual == nullptr)
        return false;
    return Instr::CriarVarTexto(bl ? bl->NomeVar : "");
}

//----------------------------------------------------------------------------
// Nome da vari�vel
bool TTextoVar::FuncTipo(TVariavel * v)
{
    const char * texto = "";
    TBlocoVar * bl = nullptr;
    if (Instr::VarAtual >= v + 1)
    {
        TTextoVar * ref = reinterpret_cast<TTextoVar*>(v->endvar) + v->indice;
        bl = ref->Procura(v[1].getTxt());
        if (bl)
            switch (bl->TipoVar())
            {
            case TextoVarTipoTxt: texto = " "; break;
            case TextoVarTipoNum: texto = "_"; break;
            case TextoVarTipoDec: texto = "@"; break;
            case TextoVarTipoRef: texto = "$"; break;
            }
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(texto);
}

//----------------------------------------------------------------------------
// Vari�vel anterior
bool TTextoVar::FuncAntes(TVariavel * v)
{
    TTextoVar * ref = reinterpret_cast<TTextoVar*>(v->endvar) + v->indice;
    TBlocoVar * bl = nullptr;
    if (Instr::VarAtual >= v + 1)
        bl = ref->ProcAntes(v[1].getTxt());
    if (Instr::VarAtual >= v + 2 && bl)
    {
        int cmp = comparaZ(bl->NomeVar, v[2].getTxt());
        if (cmp != 0 && cmp != 2) // 0=textos iguais, 2=texto 1 cont�m texto 2
            bl = nullptr;
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(bl ? bl->NomeVar : "");
}

//----------------------------------------------------------------------------
// Pr�xima vari�vel
bool TTextoVar::FuncDepois(TVariavel * v)
{
    TTextoVar * ref = reinterpret_cast<TTextoVar*>(v->endvar) + v->indice;
    TBlocoVar * bl = nullptr;
    if (Instr::VarAtual >= v + 1)
        bl = ref->ProcDepois(v[1].getTxt());
    if (Instr::VarAtual >= v + 2 && bl)
    {
        int cmp = comparaZ(bl->NomeVar, v[2].getTxt());
        if (cmp != 0 && cmp != 2) // 0=textos iguais, 2=texto 1 cont�m texto 2
            bl = nullptr;
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(bl ? bl->NomeVar : "");
}

//----------------------------------------------------------------------------
// In�cio
bool TTextoVar::FuncIni(TVariavel * v)
{
    TTextoVar * ref = reinterpret_cast<TTextoVar*>(v->endvar) + v->indice;
    TBlocoVar * bl = nullptr;
    if (ref->RBroot)
    {
        if (Instr::VarAtual < v + 1)
            bl = ref->RBroot->RBfirst();
        else
            bl = ref->ProcIni(v[1].getTxt());
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(bl ? bl->NomeVar : "");
}

//----------------------------------------------------------------------------
// Fim
bool TTextoVar::FuncFim(TVariavel * v)
{
    TTextoVar * ref = reinterpret_cast<TTextoVar*>(v->endvar) + v->indice;
    TBlocoVar * bl = nullptr;
    if (ref->RBroot)
    {
        if (Instr::VarAtual < v + 1)
            bl = ref->RBroot->RBlast();
        else
            bl = ref->ProcFim(v[1].getTxt());
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(bl ? bl->NomeVar : "");
}

//----------------------------------------------------------------------------
// Limpar
bool TTextoVar::FuncLimpar(TVariavel * v)
{
    TTextoVar * ref = reinterpret_cast<TTextoVar*>(v->endvar) + v->indice;
    if (Instr::VarAtual < v + 1)
    {
        ref->Limpar();
        return false;
    }
    for (TVariavel * v1 = v + 1; v1 <= Instr::VarAtual; v1++)
    {
        const char * p = v1->getTxt();
        TBlocoVar * ini = ref->ProcIni(p);
        if (ini == nullptr)
            continue;
        TBlocoVar * fim = TBlocoVar::RBnext(ref->ProcFim(p));
        while (ini && ini != fim)
        {
            TBlocoVar * bl = TBlocoVar::RBnext(ini);
            delete ini;
            ini = bl;
        }
    }
    return false;
}

//----------------------------------------------------------------------------
// Total
bool TTextoVar::FuncTotal(TVariavel * v)
{
    TTextoVar * ref = reinterpret_cast<TTextoVar*>(v->endvar) + v->indice;
    const char * txt = "";
    int total = 0;
    if (Instr::VarAtual >= v + 1)
        txt = v[1].getTxt();
    if (*txt == 0)
        return Instr::CriarVarInt(v, ref->Total);
    TBlocoVar * ini = ref->ProcIni(txt);
    if (ini)
    {
        TBlocoVar * fim = ref->ProcFim(txt);
        total = 1;
        while (ini && ini != fim)
            total++, ini = TBlocoVar::RBnext(ini);
    }
    return Instr::CriarVarInt(v, total);
}

//----------------------------------------------------------------------------
inline void TTextoVar::Apagar()
{
    while (RBroot)
        delete RBroot;
    while (Inicio)
        Inicio->Apagar();
    if (TextoAtual == this)
        TextoAtual = nullptr;
}

//----------------------------------------------------------------------------
void TTextoVar::Limpar()
{
    while (RBroot)
        delete RBroot;
}

//----------------------------------------------------------------------------
inline void TTextoVar::Mover(TTextoVar * destino)
{
    if (RBroot)
        RBroot->MoveTextoVar(destino);
    for (TTextoVarSub * obj = Inicio; obj; obj = obj->Depois)
        obj->TextoVar = destino;
    memmove(destino, this, sizeof(TTextoVar));
    if (TextoAtual == this)
        TextoAtual = destino;
}

//----------------------------------------------------------------------------
void TBlocoVar::MoveTextoVar(TTextoVar * textovar)
{
    TextoVar = textovar;
    if (RBleft)
        RBleft->MoveTextoVar(textovar);
    if (RBright)
        RBright->MoveTextoVar(textovar);
}

//------------------------------------------------------------------------------
int TTextoVar::FTamanho(const char * instr)
{
    return sizeof(TTextoVar);
}

//------------------------------------------------------------------------------
int TTextoVar::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TTextoVar);
}

//------------------------------------------------------------------------------
void TTextoVar::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TTextoVar * ref = reinterpret_cast<TTextoVar*>(v->endvar);
    for (; antes < depois; antes++)
    {
        ref[antes].RBroot = nullptr;
        ref[antes].Inicio = nullptr;
        ref[antes].Total = 0;
    }
    for (; depois < antes; depois++)
        ref[depois].Apagar();
}

//------------------------------------------------------------------------------
void TTextoVar::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    VARIAVEL_MOVER_SIMPLES(TTextoVar)
}

//------------------------------------------------------------------------------
void TTextoVar::FOperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    if (v1->defvar[2] != v2->defvar[2])
        return;
    TTextoVar * r1 = reinterpret_cast<TTextoVar*>(v1->endvar) + v1->indice;
    TTextoVar * r2 = reinterpret_cast<TTextoVar*>(v2->endvar) + v2->indice;
    if (r1 == r2)
        return;
    while (r1->RBroot)
        delete r1->RBroot;
    TBlocoVar * bl1 = (r2->RBroot ? r2->RBroot->RBfirst() : nullptr);
    for (; bl1; bl1 = TBlocoVar::RBnext(bl1))
    {
        const char * nome = bl1->NomeVar;
        switch (bl1->TipoVar())
        {
        case TextoVarTipoTxt:
            {
                const char * p = bl1->getTxt();
                if (*p)
                    new TBlocoVarTxt(r1, nome, p);
                break;
            }
        case TextoVarTipoNum:
            {
                double valor = bl1->getDouble();
                if (valor != 0)
                    new TBlocoVarNum(r1, nome, valor);
                break;
            }
        case TextoVarTipoDec:
            {
                int valor = bl1->getInt();
                if (valor != 0)
                    new TBlocoVarDec(r1, nome, valor);
                break;
            }
        case TextoVarTipoRef:
            {
                TObjeto * obj = bl1->getObj();
                if (obj)
                    new TBlocoVarRef(r1, nome, obj);
                break;
            }
        }
    }
}

//------------------------------------------------------------------------------
bool TTextoVar::FOperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    if (v1->defvar[2] != v2->defvar[2])
        return false;
    TTextoVar * ref1 = reinterpret_cast<TTextoVar*>(v1->endvar) + v1->indice;
    TTextoVar * ref2 = reinterpret_cast<TTextoVar*>(v2->endvar) + v2->indice;
    TBlocoVar * bl1 = (ref1->RBroot ? ref1->RBroot->RBfirst() : nullptr);
    TBlocoVar * bl2 = (ref2->RBroot ? ref2->RBroot->RBfirst() : nullptr);
    while (bl1 && bl2)
    {
    // Compara o nome da vari�vel
        if (strcmp(bl1->NomeVar, bl2->NomeVar) != 0)
            return false;
    // Compara o tipo de vari�vel
        if (strcmp(bl1->Tipo(), bl2->Tipo()) != 0)
            return false;
    // Compara o conte�do da vari�vel
        switch (bl1->TipoVar())
        {
        case TextoVarTipoTxt:
            if (strcmp(bl1->getTxt(), bl2->getTxt()) != 0)
                return false;
            break;
        case TextoVarTipoNum:
            if (bl1->getDouble() != bl2->getDouble())
                return false;
            break;
        case TextoVarTipoDec:
            if (bl1->getInt() != bl2->getInt())
                return false;
            break;
        case TextoVarTipoRef:
            if (bl1->getObj() != bl2->getObj())
                return false;
            break;
        }
    // Passa para a pr�xima vari�vel
        bl1 = TBlocoVar::RBnext(bl1);
        bl2 = TBlocoVar::RBnext(bl2);
    }
    return bl1 == bl2;
}

//------------------------------------------------------------------------------
unsigned char TTextoVar::FOperadorCompara(TVariavel * v1, TVariavel * v2)
{
    if (v1->defvar[2] != v2->defvar[2])
        return false;
    TTextoVar * ref1 = reinterpret_cast<TTextoVar*>(v1->endvar) + v1->indice;
    TTextoVar * ref2 = reinterpret_cast<TTextoVar*>(v2->endvar) + v2->indice;
    TBlocoVar * bl1 = (ref1->RBroot ? ref1->RBroot->RBfirst() : nullptr);
    TBlocoVar * bl2 = (ref2->RBroot ? ref2->RBroot->RBfirst() : nullptr);
    while (bl1 && bl2)
    {
    // Compara o nome da vari�vel
        int cmp1 = strcmp(bl1->NomeVar, bl2->NomeVar);
        if (cmp1 != 0)
            return cmp1 < 0 ? 1 : 4;
    // Compara o tipo de vari�vel
        int cmp2 = strcmp(bl1->Tipo(), bl2->Tipo());
        if (cmp2 != 0)
            return cmp2 < 0 ? 1 : 4;
    // Compara o conte�do da vari�vel
        switch (bl1->TipoVar())
        {
        case TextoVarTipoTxt:
            {
                int cmp2 = comparaZ(bl1->getTxt(), bl2->getTxt());
                if (cmp2 == 0)
                    break;
                return cmp2 < 0 ? 1 : 4;
            }
        case TextoVarTipoNum:
            {
                double x1 = bl1->getDouble();
                double x2 = bl2->getDouble();
                if (x1 == x2)
                    break;
                unsigned char result = 0;
                if (x1 < x2) result |= 1;
                if (x1 > x2) result |= 4;
                return (x1 < x2 ? 1 : 0) | (x1 > x2 ? 4 : 0);
            }
        case TextoVarTipoDec:
            {
                int x1 = bl1->getInt();
                int x2 = bl2->getInt();
                if (x1 == x2)
                    break;
                return x1 < x2 ? 1 : 4;
            }
        case TextoVarTipoRef:
            {
                TObjeto * x1 = bl1->getObj();
                TObjeto * x2 = bl2->getObj();
                if (x1 == x2)
                    break;
                return x1 < x2 ? 1 : 4;
            }
        }
    // Passa para a pr�xima vari�vel
        bl1 = TBlocoVar::RBnext(bl1);
        bl2 = TBlocoVar::RBnext(bl2);
    }
    return bl2 ? 1 : bl1 ? 4 : 0;
}

//----------------------------------------------------------------------------
TBlocoVar * TTextoVar::Procura(const char * texto)
{
    TBlocoVar * y = RBroot;
    while (y)
    {
        int i = comparaVar(texto, y->NomeVar);
        if (i == 0)
            return y;
        if (i < 0)
            y = y->RBleft;
        else
            y = y->RBright;
    }
    return nullptr;
}

//----------------------------------------------------------------------------
TBlocoVar * TTextoVar::ProcIni(const char * texto)
{
    TBlocoVar * x = nullptr;
    TBlocoVar * y = RBroot;
    while (y)
    {
        switch (comparaVar(texto, y->NomeVar))
        {
        case -2: // string 2 cont�m string 1
        case 0:  // encontrou
            x = y;
        case -1:
            y = y->RBleft;
            break;
        default:
            y = y->RBright;
        }
    }
    return x;
}

//----------------------------------------------------------------------------
TBlocoVar * TTextoVar::ProcFim(const char * texto)
{
    TBlocoVar * x = nullptr;
    TBlocoVar * y = RBroot;
    while (y)
    {
        switch (comparaVar(texto, y->NomeVar))
        {
        case -2: // string 2 cont�m string 1
        case 0:  // encontrou
            x = y;
            y = y->RBright;
            break;
        case -1:
            y = y->RBleft;
            break;
        default:
            y = y->RBright;
        }
    }
    return x;
}

//----------------------------------------------------------------------------
TBlocoVar * TTextoVar::ProcAntes(const char * texto)
{
    TBlocoVar * x = nullptr;
    TBlocoVar * y = RBroot;
    while (y)
    {
        int i = comparaVar(texto, y->NomeVar);
        if (i <= 0)
            y = y->RBleft;
        else
            x = y, y = y->RBright;
    }
    return x;
}

//----------------------------------------------------------------------------
TBlocoVar * TTextoVar::ProcDepois(const char * texto)
{
    TBlocoVar * x = nullptr;
    TBlocoVar * y = RBroot;
    while (y)
    {
        int i = comparaVar(texto, y->NomeVar);
        if (i >= 0)
            y = y->RBright;
        else
            x = y, y = y->RBleft;
    }
    return x;
}

//----------------------------------------------------------------------------
const TVarInfo * TTextoVarSub::Inicializa()
{
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        FTipo,
        FRedim,
        FMoverEnd,
        TVarInfo::FMoverDef0,
        FGetBool,
        FGetInt,
        FGetDouble,
        FGetTxt,
        FGetObj,
        FOperadorAtrib,
        FOperadorAdd,
        FOperadorIgual2,
        FOperadorCompara,
        TVarInfo::FFuncTextoFalse,
        TVarInfo::FFuncVetorFalse,
        nullptr, -1);
    return &var;
}

//----------------------------------------------------------------------------
void TTextoVarSub::Criar(TTextoVar * var, const char * nome, bool checatipo)
{
// Acerta vari�vel conforme o nome
    char * p = copiastr(NomeVar, nome, sizeof(NomeVar));
    if (!checatipo)
        TipoVar = TextoVarTipoTxt;
    else if (p > NomeVar)
    {
        p--;
        switch (*p)
        {
        case ' ': *p = 0; TipoVar = TextoVarTipoTxt; break;
        case '_': *p = 0; TipoVar = TextoVarTipoNum; break;
        case '@': *p = 0; TipoVar = TextoVarTipoDec; break;
        case '$': *p = 0; TipoVar = TextoVarTipoRef; break;
        default:          TipoVar = TextoVarTipoTxt; break;
        }
    }

// Checa se � nome v�lido para vari�vel (n�o pode ser um texto vazio)
    if (NomeVar[0] == 0)
    {
        TextoVar = nullptr;
        return;
    }

// Coloca na lista ligada
    TextoVar = var;
    Antes = nullptr;
    Depois = var->Inicio;
    if (var->Inicio)
        var->Inicio->Antes = this;
    var->Inicio = this;
}

//----------------------------------------------------------------------------
void TTextoVarSub::Apagar()
{
    if (TextoVar == nullptr)
        return;
    (Antes ? Antes->Depois : TextoVar->Inicio) = Depois;
    if (Depois)
        Depois->Antes = Antes;
    TextoVar = nullptr;
}

//----------------------------------------------------------------------------
void TTextoVarSub::Mover(TTextoVarSub * destino)
{
    if (TextoVar)
    {
        (Antes ? Antes->Depois : TextoVar->Inicio) = destino;
        if (Depois)
            Depois->Antes = destino;
    }
    memmove(destino, this, sizeof(TTextoVarSub));
}

//------------------------------------------------------------------------------
int TTextoVarSub::FTamanho(const char * instr)
{
    return sizeof(TTextoVarSub);
}

//------------------------------------------------------------------------------
int TTextoVarSub::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TTextoVarSub);
}

//------------------------------------------------------------------------------
void TTextoVarSub::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TTextoVarSub * ref = reinterpret_cast<TTextoVarSub*>(v->endvar);
    for (; antes < depois; antes++)
    {
        ref[antes].TextoVar = nullptr;
        ref[antes].NomeVar[0] = 0;
    }
    for (; depois < antes; depois++)
        ref[depois].Apagar();
}

//------------------------------------------------------------------------------
void TTextoVarSub::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    VARIAVEL_MOVER_SIMPLES(TTextoVarSub)
}

//----------------------------------------------------------------------------
TVarTipo TTextoVarSub::FTipo(TVariavel * v)
{
    return (TVarTipo)v->numfunc;
}

//----------------------------------------------------------------------------
bool TTextoVarSub::FGetBool(TVariavel * v)
{
    return reinterpret_cast<TTextoVarSub*>(v->endvar)[ v->indice ].getBool();
}

//----------------------------------------------------------------------------
int TTextoVarSub::FGetInt(TVariavel * v)
{
    return reinterpret_cast<TTextoVarSub*>(v->endvar)[ v->indice ].getInt();
}

//----------------------------------------------------------------------------
double TTextoVarSub::FGetDouble(TVariavel * v)
{
    return reinterpret_cast<TTextoVarSub*>(v->endvar)[ v->indice ].getDouble();
}

//----------------------------------------------------------------------------
const char * TTextoVarSub::FGetTxt(TVariavel * v)
{
    return reinterpret_cast<TTextoVarSub*>(v->endvar)[ v->indice ].getTxt();
}

//------------------------------------------------------------------------------
TObjeto * TTextoVarSub::FGetObj(TVariavel * v)
{
    return reinterpret_cast<TTextoVarSub*>(v->endvar)[ v->indice ].getObj();
}

//------------------------------------------------------------------------------
void TTextoVarSub::FOperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    TTextoVarSub * ref = reinterpret_cast<TTextoVarSub*>(v1->endvar) + v1->indice;
    if (ref->TextoVar == nullptr)
        return;
    TBlocoVar * bl = ref->TextoVar->Procura(ref->NomeVar);
    switch (ref->TipoVar)
    {
    case TextoVarTipoTxt:
        {
            const char * txt = v2->getTxt();
            if (bl != nullptr)
            {
                if (bl->TipoVar() == TextoVarTipoTxt)
                {
                    bl->setTxt(txt);
                    break;
                }
                delete bl;
            }
            if (*txt != 0)
                new TBlocoVarTxt(ref->TextoVar, ref->NomeVar, txt);
            break;
        }
    case TextoVarTipoNum:
        {
            double valor = v2->getDouble();
            if (bl != nullptr)
            {
                if (bl->TipoVar() == TextoVarTipoNum)
                {
                    bl->setDouble(valor);
                    break;
                }
                delete bl;
            }
            if (valor != 0)
                new TBlocoVarNum(ref->TextoVar, ref->NomeVar, valor);
            break;
        }
    case TextoVarTipoDec:
        {
            int valor = v2->getInt();
            if (bl != nullptr)
            {
                if (bl->TipoVar() == TextoVarTipoDec)
                {
                    bl->setInt(valor);
                    break;
                }
                delete bl;
            }
            if (valor > 0)
                new TBlocoVarDec(ref->TextoVar, ref->NomeVar, valor);
            break;
        }
    case TextoVarTipoRef:
        {
            TObjeto * obj = v2->getObj();
            if (bl != nullptr)
            {
                if (bl->TipoVar() == TextoVarTipoRef)
                {
                    bl->setObj(obj);
                    break;
                }
                delete bl;
            }
            if (obj)
                new TBlocoVarRef(ref->TextoVar, ref->NomeVar, obj);
            break;
        }
    }
}

//----------------------------------------------------------------------------
bool TTextoVarSub::FOperadorAdd(TVariavel * v1, TVariavel * v2)
{
    TTextoVarSub * ref = reinterpret_cast<TTextoVarSub*>(v1->endvar) + v1->indice;
    if (ref->TextoVar == nullptr || ref->TipoVar != TextoVarTipoTxt)
        return false;
    ref->addTxt(v2->getTxt());
    return true;
}

//------------------------------------------------------------------------------
bool TTextoVarSub::FOperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    TTextoVarSub * ref = reinterpret_cast<TTextoVarSub*>(v1->endvar) + v1->indice;
    TBlocoVar * bl = (ref->TextoVar ? ref->TextoVar->Procura(ref->NomeVar) : nullptr);
    switch (ref->TipoVar)
    {
    case TextoVarTipoTxt:
        return v2->Tipo() == varTxt && strcmp(bl ? bl->getTxt() : "", v2->getTxt()) == 0;
    case TextoVarTipoNum:
    case TextoVarTipoDec:
        return v2->TipoNumerico() && (bl ? bl->getDouble() : 0) == v2->getDouble();
    case TextoVarTipoRef:
        return v2->Tipo() == varObj && (bl ? bl->getObj() : nullptr) == v2->getObj();
    }
    return false;
}

//------------------------------------------------------------------------------
unsigned char TTextoVarSub::FOperadorCompara(TVariavel * v1, TVariavel * v2)
{
    TTextoVarSub * ref = reinterpret_cast<TTextoVarSub*>(v1->endvar) + v1->indice;
    TBlocoVar * bl = (ref->TextoVar ? ref->TextoVar->Procura(ref->NomeVar) : nullptr);
    switch (ref->TipoVar)
    {
    case TextoVarTipoTxt:
        return TVarInfo::ComparaTxt(bl ? bl->getTxt() : "", v2->getTxt());
    case TextoVarTipoNum:
        return TVarInfo::ComparaDouble(bl ? bl->getDouble() : 0, v2);
    case TextoVarTipoDec:
        return TVarInfo::ComparaInt(bl ? bl->getDouble() : 0, v2);
    case TextoVarTipoRef:
        if (v2->Tipo() == varObj)
        {
            TObjeto * obj1 = (bl ? bl->getObj() : nullptr);
            TObjeto * obj2 = v2->getObj();
            return (obj1 == obj2 ? 2 : obj1 < obj2 ? 1 : 4);
        }
        return 0;
    }
    return 0;
}

//----------------------------------------------------------------------------
bool TTextoVarSub::getBool()
{
    if (TextoVar == nullptr)
        return false;
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
    return (bl ? bl->getBool() : false);
}

//----------------------------------------------------------------------------
int TTextoVarSub::getInt()
{
    if (TextoVar == nullptr)
        return 0;
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
    return (bl ? bl->getInt() : 0);
}

//----------------------------------------------------------------------------
double TTextoVarSub::getDouble()
{
    if (TextoVar == nullptr)
        return 0;
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
    return (bl ? bl->getDouble() : 0);
}

//----------------------------------------------------------------------------
const char * TTextoVarSub::getTxt()
{
    if (TextoVar)
    {
        TBlocoVar * bl = TextoVar->Procura(NomeVar);
        if (bl)
            return bl->getTxt();
    }
    if (TipoVar == TextoVarTipoNum || TipoVar == TextoVarTipoDec)
        return "0";
    return "";
}

//----------------------------------------------------------------------------
TObjeto * TTextoVarSub::getObj()
{
    if (TextoVar == nullptr)
        return nullptr;
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
    return (bl ? bl->getObj() : nullptr);
}

//----------------------------------------------------------------------------
void TTextoVarSub::setInt(int valor)
{
    if (TextoVar == nullptr)
        return;
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
// Vari�vel existente: checa se deve mudar ou apagar/criar vari�vel
    if (bl != nullptr)
    {
        if (bl->TipoVar() == TipoVar)
        {
            bl->setInt(valor);
            return;
        }
        delete bl;
        bl = nullptr;
    }
// Criar vari�vel
    switch (TipoVar)
    {
    case TextoVarTipoTxt:
        {
            char mens[40];
            mprintf(mens, sizeof(mens), "%d", valor);
            new TBlocoVarTxt(TextoVar, NomeVar, mens);
            break;
        }
        break;
    case TextoVarTipoNum:
        if (valor != 0)
            new TBlocoVarNum(TextoVar, NomeVar, valor);
        break;
    case TextoVarTipoDec:
        if (valor > 0)
            new TBlocoVarDec(TextoVar, NomeVar, valor);
        break;
    case TextoVarTipoRef:
        break;
    }
}

//----------------------------------------------------------------------------
void TTextoVarSub::setDouble(double valor)
{
    if (TextoVar == nullptr)
        return;
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
// Vari�vel existente: checa se deve mudar ou apagar/criar vari�vel
    if (bl != nullptr)
    {
        if (bl->TipoVar() == TipoVar)
        {
            bl->setDouble(valor);
            return;
        }
        delete bl;
        bl = nullptr;
    }
// Criar vari�vel
    switch (TipoVar)
    {
    case TextoVarTipoTxt:
        {
            char mens[100];
            DoubleToTxt(mens, valor);
            new TBlocoVarTxt(TextoVar, NomeVar, mens);
            break;
        }
    case TextoVarTipoNum:
        if (valor != 0)
            new TBlocoVarNum(TextoVar, NomeVar, valor);
        break;
    case TextoVarTipoDec:
        {
            int i = DoubleToInt(valor);
            if (i > 0)
                new TBlocoVarDec(TextoVar, NomeVar, i);
            break;
        }
    case TextoVarTipoRef:
        break;
    }
}

//----------------------------------------------------------------------------
void TTextoVarSub::setTxt(const char * txt)
{
    if (TextoVar == nullptr)
        return;
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
// Vari�vel existente: checa se deve mudar ou apagar/criar vari�vel
    if (bl != nullptr)
    {
        if (bl->TipoVar() == TipoVar)
        {
            bl->setTxt(txt);
            return;
        }
        delete bl;
        bl = nullptr;
    }
// Criar vari�vel
    switch (TipoVar)
    {
    case TextoVarTipoTxt:
        if (*txt != 0)
            new TBlocoVarTxt(TextoVar, NomeVar, txt);
        break;
    case TextoVarTipoNum:
        {
            double d = TxtToDouble(txt);
            if (d != 0)
                new TBlocoVarNum(TextoVar, NomeVar, d);
            break;
        }
    case TextoVarTipoDec:
        {
            int i = TxtToInt(txt);
            if (i > 0)
                new TBlocoVarDec(TextoVar, NomeVar, i);
            break;
        }
    case TextoVarTipoRef:
        break;
    }
}

//----------------------------------------------------------------------------
void TTextoVarSub::addTxt(const char * txt)
{
    if (TextoVar == nullptr || *txt == 0)
        return;
// Vari�vel existente: checa se deve mudar ou apagar/criar vari�vel
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
    if (bl != nullptr)
    {
        if (bl->TipoVar() == TipoVar)
        {
            bl->addTxt(txt);
            return;
        }
        delete bl;
        bl = nullptr;
    }
// Criar vari�vel
    switch (TipoVar)
    {
    case TextoVarTipoTxt:
        new TBlocoVarTxt(TextoVar, NomeVar, txt);
        break;
    case TextoVarTipoNum:
        {
            double d = TxtToDouble(txt);
            if (d != 0)
                new TBlocoVarNum(TextoVar, NomeVar, d);
            break;
        }
    case TextoVarTipoDec:
        {
            int i = TxtToInt(txt);
            if (i > 0)
                new TBlocoVarDec(TextoVar, NomeVar, i);
            break;
        }
    case TextoVarTipoRef:
        break;
    }
}

//----------------------------------------------------------------------------
void TTextoVarSub::setObj(TObjeto * obj)
{
    if (TextoVar == nullptr)
        return;
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
// Nenhum objeto: apaga vari�vel
    if (obj == nullptr)
    {
        if (bl)
            delete bl;
        return;
    }
// Vari�vel existente: checa se deve mudar ou apagar/criar vari�vel
    if (bl != nullptr)
    {
        if (bl->TipoVar() == TipoVar)
        {
            bl->setObj(obj);
            return;
        }
        delete bl;
        bl = nullptr;
    }
// Criar vari�vel
    switch (TipoVar)
    {
    case TextoVarTipoTxt:
        new TBlocoVarTxt(TextoVar, NomeVar, obj->Classe->Nome);
        break;
    case TextoVarTipoNum:
    case TextoVarTipoDec:
        break;
    case TextoVarTipoRef:
        new TBlocoVarRef(TextoVar, NomeVar, obj);
        break;
    }
}

//----------------------------------------------------------------------------
TBlocoVar::TBlocoVar(TTextoVar * var, const char * nome, const char * texto)
{
    int tam1 = strlen(nome) + 1;
    int tam2 = (texto ? strlen(texto) + 1 : 0);
    char * p = new char[tam1 + tam2];
    NomeVar = p;
    memcpy(p, nome, tam1);
    if (tam2)
        memcpy(p+tam1, texto, tam2);
    Texto = tam1;
    TextoVar = var;
    RBinsert();
    var->Total++;
}

//----------------------------------------------------------------------------
TBlocoVar::~TBlocoVar()
{
    TextoVar->Total--;
    delete[] NomeVar;
    RBremove();
}

//----------------------------------------------------------------------------
int TBlocoVar::RBcomp(TBlocoVar * x, TBlocoVar * y)
{
    return comparaVar(x->NomeVar, y->NomeVar);
}

//----------------------------------------------------------------------------
#define CLASS TBlocoVar          // Nome da classe
#define RBmask 1 // M�scara para bit 0
#define RBroot TextoVar->RBroot
#include "rbt.cpp.h"

//----------------------------------------------------------------------------
TBlocoVarTxt::TBlocoVarTxt(TTextoVar * var, const char * nome, const char * texto) :
    TBlocoVar(var, nome, texto)
{
    assert(*texto != 0);
}

//----------------------------------------------------------------------------
TBlocoVarTxt::~TBlocoVarTxt()
{
}

//----------------------------------------------------------------------------
bool TBlocoVarTxt::getBool()
{
    return true; // Texto n�o est� vazio
    //return NomeVar[Texto] != 0;
}

//----------------------------------------------------------------------------
int TBlocoVarTxt::getInt()
{
    return TxtToInt(NomeVar + Texto);
}

//----------------------------------------------------------------------------
double TBlocoVarTxt::getDouble()
{
    return TxtToDouble(NomeVar + Texto);
}

//----------------------------------------------------------------------------
const char * TBlocoVarTxt::getTxt()
{
    return NomeVar + Texto;
}

//----------------------------------------------------------------------------
TObjeto * TBlocoVarTxt::getObj()
{
    return nullptr;
}

//----------------------------------------------------------------------------
void TBlocoVarTxt::setInt(int valor)
{
    char mens[80];
    sprintf(mens, "%d", valor);
    setTxt(mens);
}

//----------------------------------------------------------------------------
void TBlocoVarTxt::setDouble(double valor)
{
    char mens[100];
    DoubleToTxt(mens, valor);
    setTxt(mens);
}

//----------------------------------------------------------------------------
void TBlocoVarTxt::setTxt(const char * txt)
{
    if (*txt == 0)
    {
        delete this;
        return;
    }
    int tot1 = Texto;
    int tot2 = strlen(txt) + 1;
    char * p = new char[tot1 + tot2];
    memcpy(p, NomeVar, tot1);
    memcpy(p + tot1, txt, tot2);
    delete[] NomeVar;
    NomeVar = p;
}

//----------------------------------------------------------------------------
void TBlocoVarTxt::addTxt(const char * txt)
{
    if (*txt == 0)
        return;
    int tot1 = Texto + strlen(NomeVar + Texto);
    int tot2 = strlen(txt) + 1;
    char * p = new char[tot1 + tot2];
    memcpy(p, NomeVar, tot1);
    memcpy(p + tot1, txt, tot2);
    delete[] NomeVar;
    NomeVar = p;
}

//----------------------------------------------------------------------------
void TBlocoVarTxt::setObj(TObjeto * obj)
{
    delete this;
}

//------------------------------------------------------------------------------
TBlocoVarNum::TBlocoVarNum(TTextoVar * var, const char * nome, double valor) :
    TBlocoVar(var, nome)
{
    ValorDouble = valor;
}

//------------------------------------------------------------------------------
TBlocoVarNum::~TBlocoVarNum()
{
}

//------------------------------------------------------------------------------
bool TBlocoVarNum::getBool()
{
    return ValorDouble != 0;
}

//------------------------------------------------------------------------------
int TBlocoVarNum::getInt()
{
    return DoubleToInt(ValorDouble);
}

//------------------------------------------------------------------------------
double TBlocoVarNum::getDouble()
{
    return ValorDouble;
}
//------------------------------------------------------------------------------
const char * TBlocoVarNum::getTxt()
{
    char * txtnum = TVarInfo::BufferTxt();
    DoubleToTxt(txtnum, ValorDouble);
    return txtnum;
}

//------------------------------------------------------------------------------
TObjeto * TBlocoVarNum::getObj()
{
    return nullptr;
}

//------------------------------------------------------------------------------
void TBlocoVarNum::setInt(int valor)
{
    if (valor == 0)
        delete this;
    else
        ValorDouble = valor;
}

//------------------------------------------------------------------------------
void TBlocoVarNum::setDouble(double valor)
{
    if (valor == 0)
        delete this;
    else
        ValorDouble = valor;
}

//------------------------------------------------------------------------------
void TBlocoVarNum::setTxt(const char * txt)
{
    ValorDouble = TxtToDouble(txt);
}

//------------------------------------------------------------------------------
void TBlocoVarNum::addTxt(const char * txt)
{
    ValorDouble = TxtToDouble(txt);
}

//------------------------------------------------------------------------------
void TBlocoVarNum::setObj(TObjeto * obj)
{
    delete this;
}

//------------------------------------------------------------------------------
void TBlocoVarDec::PreparaIni()
{
    if (TBlocoVarDec::VetMenos)
        return;
    VetMenos = new TBlocoVarDec*[INTTEMPO_MAX];
    VetMais = new TBlocoVarDec*[INTTEMPO_MAX];
    memset(VetMenos, 0, sizeof(TBlocoVarDec*)*INTTEMPO_MAX);
    memset(VetMais, 0, sizeof(TBlocoVarDec*)*INTTEMPO_MAX);
}

//------------------------------------------------------------------------------
void TBlocoVarDec::ProcEventos(int TempoDecorrido)
{
    while (TempoDecorrido-- > 0)
    {
    // Avan�a TempoMenos
    // Move objetos de VetMais para VetMenos se necess�rio
        if (TempoMenos < INTTEMPO_MAX-1)
            TempoMenos++;
        else
        {
            TempoMenos = 0;
            TempoMais = (TempoMais + 1) % INTTEMPO_MAX;
            while (true)
            {
                TBlocoVarDec * obj = VetMais[TempoMais];
                if (obj == nullptr)
                    break;
            // Move objeto da lista ligada VetMais para VetMenos
                VetMais[TempoMais] = obj->Depois;
                int menos = obj->IndiceMenos;
                obj->Antes = nullptr;
                obj->Depois = VetMenos[menos];
                VetMenos[menos] = obj;
                if (obj->Depois)
                    obj->Depois->Antes = obj;
            }
        }
    // Apaga objetos de VetMenos[TempoMenos]
        while (true)
        {
            TBlocoVarDec * obj = VetMenos[TempoMenos];
            if (obj == nullptr)
                break;
            delete obj;
        }
    }
}

//----------------------------------------------------------------------------
TBlocoVarDec::TBlocoVarDec(TTextoVar * var, const char * nome, int valor) :
    TBlocoVar(var, nome)
{
    InsereLista(valor);
}

//----------------------------------------------------------------------------
TBlocoVarDec::~TBlocoVarDec()
{
    RemoveLista();
}

//----------------------------------------------------------------------------
bool TBlocoVarDec::getBool()
{
    return true;
}

//----------------------------------------------------------------------------
int TBlocoVarDec::getInt()
{
    int valor = ((IndiceMais - TempoMais) * INTTEMPO_MAX +
            IndiceMenos - TempoMenos);
    if (valor < 0)
        valor += INTTEMPO_MAX * INTTEMPO_MAX;
    return valor;
}

//----------------------------------------------------------------------------
double TBlocoVarDec::getDouble()
{
    return getInt();
}

//----------------------------------------------------------------------------
const char * TBlocoVarDec::getTxt()
{
    char * txtnum = TVarInfo::BufferTxt();
    sprintf(txtnum, "%d", getInt());
    return txtnum;
}

//----------------------------------------------------------------------------
TObjeto * TBlocoVarDec::getObj()
{
    return nullptr;
}

//----------------------------------------------------------------------------
void TBlocoVarDec::setInt(int valor)
{
    if (TextoVar == nullptr)
        return;
    if (valor <= 0)
        delete this;
    else
    {
        RemoveLista();
        InsereLista(valor);
    }
}

//----------------------------------------------------------------------------
void TBlocoVarDec::setDouble(double valor)
{
    if (TextoVar != nullptr)
        setInt(DoubleToInt(valor));
}

//----------------------------------------------------------------------------
void TBlocoVarDec::setTxt(const char * txt)
{
    if (TextoVar != nullptr)
        setInt(TxtToInt(txt));
}

//----------------------------------------------------------------------------
void TBlocoVarDec::addTxt(const char * txt)
{
    if (TextoVar != nullptr)
        setInt(TxtToInt(txt));
}

//----------------------------------------------------------------------------
void TBlocoVarDec::setObj(TObjeto * obj)
{
    delete this;
}

//----------------------------------------------------------------------------
void TBlocoVarDec::InsereLista(int valor)
{
// Acerta os valores m�nimo e m�ximo
    if (valor <= 0)
        valor = 1;
    if (valor >= INTTEMPO_MAX * INTTEMPO_MAX)
        valor = INTTEMPO_MAX * INTTEMPO_MAX - 1;
// Acerta IndiceMenos e IndiceMais
    valor += TempoMais * INTTEMPO_MAX + TempoMenos;
    IndiceMenos = valor % INTTEMPO_MAX;
    IndiceMais = valor / INTTEMPO_MAX % INTTEMPO_MAX;
// Coloca na lista apropriada
    if (IndiceMais != TempoMais || IndiceMenos <= TempoMenos)
    {
        Depois = VetMais[IndiceMais];
        VetMais[IndiceMais] = this;
    }
    else
    {
        Depois = VetMenos[IndiceMenos];
        VetMenos[IndiceMenos] = this;
    }
    if (Depois)
        Depois->Antes = this;
    Antes = nullptr;
}

//----------------------------------------------------------------------------
void TBlocoVarDec::RemoveLista()
{
    if (Antes)
        Antes->Depois = Depois;
    else if (VetMenos[IndiceMenos] == this)
        VetMenos[IndiceMenos] = Depois;
    else if (VetMais[IndiceMais] == this)
        VetMais[IndiceMais] = Depois;
    if (Depois)
        Depois->Antes = Antes, Depois = nullptr;
}

//----------------------------------------------------------------------------
TBlocoVarRef::TBlocoVarRef(TTextoVar * var, const char * nome, TObjeto * obj) :
    TBlocoVar(var, nome)
{
    InsereLista(obj);
}

//----------------------------------------------------------------------------
TBlocoVarRef::~TBlocoVarRef()
{
    RemoveLista();
}

//----------------------------------------------------------------------------
bool TBlocoVarRef::getBool()
{
    return true;
}

//----------------------------------------------------------------------------
int TBlocoVarRef::getInt()
{
    return 0;
}

//----------------------------------------------------------------------------
double TBlocoVarRef::getDouble()
{
    return 0;
}

//----------------------------------------------------------------------------
const char * TBlocoVarRef::getTxt()
{
    return Objeto->Classe->Nome;
}

//----------------------------------------------------------------------------
TObjeto * TBlocoVarRef::getObj()
{
    return Objeto;
}

//----------------------------------------------------------------------------
void TBlocoVarRef::setInt(int valor)
{
    delete this;
}

//----------------------------------------------------------------------------
void TBlocoVarRef::setDouble(double valor)
{
    delete this;
}

//----------------------------------------------------------------------------
void TBlocoVarRef::setTxt(const char * txt)
{
    delete this;
}

//----------------------------------------------------------------------------
void TBlocoVarRef::addTxt(const char * txt)
{
    delete this;
}

//----------------------------------------------------------------------------
void TBlocoVarRef::setObj(TObjeto * obj)
{
    if (obj == nullptr)
        delete this;
    else if (obj != Objeto)
    {
        RemoveLista();
        InsereLista(obj);
    }
}

//----------------------------------------------------------------------------
void TBlocoVarRef::InsereLista(TObjeto * obj)
{
    Objeto = obj;
    ObjAntes = nullptr;
    ObjDepois = obj->VarBlocoRef;
    if (ObjDepois)
        ObjDepois->ObjAntes = this;
    obj->VarBlocoRef = this;
}

//----------------------------------------------------------------------------
void TBlocoVarRef::RemoveLista()
{
    (ObjAntes ? ObjAntes->ObjDepois : Objeto->VarBlocoRef) = ObjDepois;
    if (ObjDepois)
        ObjDepois->ObjAntes = ObjAntes;
}
