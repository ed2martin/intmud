/* Este arquivo � software livre; voc� pode redistribuir e/ou
 * modificar nos termos da licen�a LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "var-nomeobj.h"
#include "variavel.h"
#include "instr.h"
#include "misc.h"

//----------------------------------------------------------------------------
const TVarInfo * TVarNomeObj::Inicializa()
{
    static TVarInfo::FuncItem ListaFuncEnd[] = {
        { "ini",        &TVarNomeObj::FuncIni },
        { "nome",       &TVarNomeObj::FuncNome }  };
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        TVarInfo::FTipoOutros,
        FRedim,
        FMoverEnd,
        TVarInfo::FMoverDef0,
        FGetBool,
        FGetInt,
        FGetDouble,
        FGetTxt,
        TVarInfo::FGetObjNulo,
        TVarInfo::FOperadorAtribVazio,
        TVarInfo::FOperadorAddFalse,
        TVarInfo::FOperadorIgual2Var,
        TVarInfo::FOperadorComparaVar,
        TVarInfo::FFuncTextoFalse,
        TVarInfo::FFuncVetorFalse,
        ListaFuncEnd,
        sizeof(ListaFuncEnd) / sizeof(ListaFuncEnd[0]) - 1);
    return &var;
}

//------------------------------------------------------------------------------
bool TVarNomeObj::FuncNome(TVariavel * v)
{
    TVarNomeObj * ref = reinterpret_cast<TVarNomeObj*>(v->endvar) + v->indice;
    ref->Achou = 0;
    if (ref->Total <= 0 || Instr::VarAtual - v < 1)
        return Instr::CriarVarInt(v, 0);
// Prepara vari�veis
    if (*ref->NomeObj)
    {
        char mens[4096];
        const char * txt = v[1].getTxt();
        while (*txt == ' ')
            txt++;
        char * p = mens;
        for (; *txt && p < mens+sizeof(mens) - 1; p++,txt++)
            *p = tabNOMEOBJ[*(unsigned char*)(txt)];
        *p = 0;
        // Checa fim do nome
        if (*mens == 0)
            return Instr::CriarVarInt(v, 0);
// Verifica se � o nome procurado
// Todas as palavras de NomeObj devem estar em mens
        txt = ref->NomeObj;
        while (*txt)
        {
            const char * lista = mens;
            while (true)
            {
                const char * str = txt;
                while (*str && *str != ' ' && *str == *lista)
                    str++, lista++;
                if (*str == 0 || *str == ' ') // Encontrou
                    break;
                while (*lista && *lista != ' ')
                    lista++;
                while (*lista == ' ')
                    lista++;
                if (*lista == 0) // Fim da lista: n�o encontrou
                    return Instr::CriarVarInt(v, 0);
            }
            while (*txt && *txt != ' ')
                txt++;
            while (*txt == ' ')
                txt++;
        }
    }
// Checa se � o item inicial
    int valor = (Instr::VarAtual - v < 2 ? 1 : v[2].getInt());
    if (ref->Inicio >= valor)
    {
        ref->Inicio -= valor;
        return Instr::CriarVarInt(v, 0);
    }
    valor -= ref->Inicio;
    ref->Inicio = 0;
// Obt�m a quantidade de itens
    if (valor >= ref->Total)
        valor = ref->Total, ref->Total = 0;
    else
        ref->Total -= valor;
    ref->Achou = valor;
    return Instr::CriarVarInt(v, valor);
}

//------------------------------------------------------------------------------
bool TVarNomeObj::FuncIni(TVariavel * v)
{
    TVarNomeObj * ref = reinterpret_cast<TVarNomeObj*>(v->endvar) + v->indice;
    ref->Inicio = 0;
    ref->Total = 0;
// Checa o n�mero de vari�veis
    if (Instr::VarAtual - v < 1)
        return false;
// Obt�m o texto que se refere a todos os itens
    char nometudo[200];
    *nometudo = 0;
    if (Instr::VarAtual >= v + 3)
    {
        const char * o = v[3].getTxt();
        char * d = nometudo;
        for (; *o && d < nometudo + sizeof(nometudo) - 1; o++, d++)
            *d = tabNOMEOBJ[*(unsigned char*)o];
        *d = 0;
    }
// Obt�m o texto
    ref->Inicio = 0;
    const char * txt = v[1].getTxt();
    while (*txt == ' ')
        txt++;
    if (*nometudo)
    {
        const char * p1 = txt;
        const char * p2 = nometudo;
        while (*p1 && tabNOMEOBJ[*(unsigned char*)p1] == *p2)
            p1++, p2++;
        if (*p1 == '.' && *p2 == 0)
            ref->Total = 0x7FFFFFFF, txt = p1 + 1;
    }
    if (ref->Total == 0)
        while (*txt >= '0' && *txt <= '9')
        {
        // Obt�m o n�mero
            const char * p = txt;
            int num = TxtToInt(p);
            while (*p >= '0' && *p <= '9')
                p++;
        // O n�mero � zero
            if (num == 0)
                break;
        // � a quantidade de itens
            if (*p == ' ' && ref->Total == 0)
            {
                while (*p == ' ')
                    p++;
                if (*p == 0)  // Somente o n�mero n�o pode ser quantidade
                    break;
                txt = p;
                ref->Total = num;
                continue;
            }
        // � o n�mero do primeiro item
            if (*p == '.')
                ref->Inicio = num - 1, txt = p + 1;
        // � o nome do item
            break;
        }
// Copia o texto
    {
        while (*txt == ' ')
            txt++;
        char * p = ref->NomeObj;
        for (; *txt && p < ref->NomeObj + sizeof(ref->NomeObj) - 1; p++, txt++)
            *p = tabNOMEOBJ[*(unsigned char*)(txt)];
        *p = 0;
    }
// Checa se o texto � nulo
    if (*ref->NomeObj == 0)
    {
        ref->Total = 0;
        return false;
    }
// Checa se digitou a palavra que corresponde a todos os itens
    if (Instr::VarAtual >= v + 3)
        if (strcmp(ref->NomeObj, nometudo) == 0)
        {
            *ref->NomeObj = 0;
            if (ref->Total <= 0)
                ref->Total = 0x7FFFFFFF;
        }
// Obt�m Total
    if (ref->Total <= 0)
        ref->Total = 1;
    int x = (Instr::VarAtual - v < 2 ? 1 : v[2].getInt());
    if (ref->Total > x)
        ref->Total = x;
    return false;
}

//------------------------------------------------------------------------------
inline int TVarNomeObj::getValor()
{
    return Achou;
}

//------------------------------------------------------------------------------
int TVarNomeObj::FTamanho(const char * instr)
{
    return sizeof(TVarNomeObj);
}

//------------------------------------------------------------------------------
int TVarNomeObj::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TVarNomeObj);
}

//------------------------------------------------------------------------------
void TVarNomeObj::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TVarNomeObj * ref = reinterpret_cast<TVarNomeObj*>(v->endvar);
    for (; antes < depois; antes++)
    {
        ref[antes].NomeObj[0] = 0;
        ref[antes].Total = 0;
    }
}

//------------------------------------------------------------------------------
void TVarNomeObj::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    int total = (unsigned char)v->defvar[Instr::endVetor];
    memmove(destino, v->endvar, (total ? total : 1) * sizeof(TVarNomeObj));
}

//------------------------------------------------------------------------------
bool TVarNomeObj::FGetBool(TVariavel * v)
{
    return reinterpret_cast<TVarNomeObj*>(v->endvar)[ v->indice ].getValor();
}

//------------------------------------------------------------------------------
int TVarNomeObj::FGetInt(TVariavel * v)
{
    return reinterpret_cast<TVarNomeObj*>(v->endvar)[ v->indice ].getValor();
}

//------------------------------------------------------------------------------
double TVarNomeObj::FGetDouble(TVariavel * v)
{
    return reinterpret_cast<TVarNomeObj*>(v->endvar)[ v->indice ].getValor();
}

//------------------------------------------------------------------------------
const char * TVarNomeObj::FGetTxt(TVariavel * v)
{
    TVarNomeObj * ref = reinterpret_cast<TVarNomeObj*>(v->endvar) + v->indice;
    char * buf = TVarInfo::BufferTxt();
    sprintf(buf, "%d", ref->getValor());
    return buf;
}
