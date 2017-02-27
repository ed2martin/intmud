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
#include "var-textovar.h"
#include "variavel.h"
#include "classe.h"
#include "objeto.h"
#include "instr.h"
#include "misc.h"

TTextoVar * TTextoVar::TextoAtual = 0;
TBlocoVarDec ** TBlocoVarDec::VetMenos = 0;
TBlocoVarDec ** TBlocoVarDec::VetMais = 0;
unsigned int TBlocoVarDec::TempoMenos = 0;
unsigned int TBlocoVarDec::TempoMais = 0;
char TBlocoVar::txtnum[80] = "";

//----------------------------------------------------------------------------
bool TTextoVar::Func(TVariavel * v, const char * nome)
{
// Lista das funções de indiceitem
// Deve obrigatoriamente estar em letras minúsculas e ordem alfabética
    static const struct {
        const char * Nome;
        bool (TTextoVar::*Func)(TVariavel * v); } ExecFunc[] = {
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
// Procura a função correspondente e executa
    int ini = 0;
    int fim = sizeof(ExecFunc) / sizeof(ExecFunc[0]) - 1;
    char mens[VAR_NOME_TAM];
    copiastrmin(mens, nome, sizeof(mens));
    while (ini <= fim)
    {
        int meio = (ini+fim)/2;
        int resultado = strcmp(mens, ExecFunc[meio].Nome);
        if (resultado==0) // Se encontrou...
            return (this->*ExecFunc[meio].Func)(v);
        if (resultado<0) fim=meio-1; else ini=meio+1;
    }
// Outro nome de variável
    if (nome[0] == 0)
        return false;
    TextoAtual = this;
    Instr::ApagarVar(v);
    if (TextoAtual == 0)
        return false;
    return CriarTextoVarSub(nome);
}

//----------------------------------------------------------------------------
bool TTextoVar::CriarTextoVarSub(const char * nome)
{
    if (!Instr::CriarVar(Instr::InstrVarTextoVarSub))
        return false;
    TTextoVarSub * sub = Instr::VarAtual->end_textovarsub;
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
    TTextoVarSub * sub = Instr::VarAtual->end_textovarsub;
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

//------------------------------------------------------------------------------
int TTextoVar::Compara(TTextoVar * v)
{
    TBlocoVar * bl1 = (RBroot ? RBroot->RBfirst() : 0);
    TBlocoVar * bl2 = (v->RBroot ? v->RBroot->RBfirst() : 0);
    while (bl1 && bl2)
    {
    // Compara o nome da variável
        int cmp1 = strcmp(bl1->NomeVar, bl2->NomeVar);
        if (cmp1 != 0)
            return cmp1;
    // Compara o tipo de variável
        const char * tipo1 = bl1->Tipo();
        const char * tipo2 = bl2->Tipo();
        int cmp2 = strcmp(tipo1, tipo2);
        if (cmp2 != 0)
            return cmp1;
    // Compara o conteúdo da variável
        switch (bl1->TipoVar())
        {
        case TextoVarTipoTxt:
            {
                int cmp2 = strcmp(bl1->getTxt(), bl2->getTxt());
                if (cmp2 == 0)
                    break;
                return (cmp2 < 0 ? -1 : 1);
            }
        case TextoVarTipoNum:
            {
                double v1 = bl1->getDouble();
                double v2 = bl2->getDouble();
                if (v1 == v2)
                    break;
                return (v1 < v2 ? -1 : 1);
            }
        case TextoVarTipoDec:
            {
                int v1 = bl1->getDouble();
                int v2 = bl2->getDouble();
                if (v1 == v2)
                    break;
                return (v1 < v2 ? -1 : 1);
            }
        case TextoVarTipoRef:
            {
                TObjeto * v1 = bl1->getObj();
                TObjeto * v2 = bl2->getObj();
                if (v1 == v2)
                    break;
                return (v1 < v2 ? -1 : 1);
            }
        }
    // Passa para a próxima variável
        bl1 = TBlocoVar::RBnext(bl1);
        bl2 = TBlocoVar::RBnext(bl2);
    }
    return (bl1 ? -1 : bl2 ? 1 : 0);
}

//------------------------------------------------------------------------------
void TTextoVar::Igual(TTextoVar * v)
{
    if (v == this)
        return;
    while (RBroot)
        delete RBroot;
    TBlocoVar * bl1 = (v->RBroot ? v->RBroot->RBfirst() : 0);
    for (; bl1; bl1 = TBlocoVar::RBnext(bl1))
    {
        const char * nome = bl1->NomeVar;
        switch (bl1->TipoVar())
        {
        case TextoVarTipoTxt:
            {
                const char * p = bl1->getTxt();
                if (*p)
                    new TBlocoVarTxt(this, nome, p);
                break;
            }
        case TextoVarTipoNum:
            {
                double valor = bl1->getDouble();
                if (valor != 0)
                    new TBlocoVarNum(this, nome, valor);
                break;
            }
        case TextoVarTipoDec:
            {
                int valor = bl1->getInt();
                if (valor != 0)
                    new TBlocoVarDec(this, nome, valor);
                break;
            }
        case TextoVarTipoRef:
            {
                TObjeto * obj = bl1->getObj();
                if (obj)
                    new TBlocoVarRef(this, nome, obj);
                break;
            }
        }
    }
}

//----------------------------------------------------------------------------
// Variável
bool TTextoVar::FuncValor(TVariavel * v)
{
    if (Instr::VarAtual < v+1)
        return false;
    TextoAtual = this;
    char mens[VAR_NOME_TAM];
    copiastr(mens, v[1].getTxt(), sizeof(mens));
    if (mens[0] == 0)
        return false;
    Instr::ApagarVar(v);
    if (TextoAtual == 0)
        return false;
    return CriarTextoVarSub(mens);
}

//----------------------------------------------------------------------------
// Primeira variável
bool TTextoVar::FuncValorIni(TVariavel * v)
{
    TBlocoVar * bl = 0;
    if (RBroot)
    {
        if (Instr::VarAtual < v+1)
            bl = RBroot->RBfirst();
        else
            bl = ProcIni(v[1].getTxt());
    }
    if (bl == 0)
        return false;
    TextoAtual = this;
    Instr::ApagarVar(v);
    if (TextoAtual == 0)
        return false;
    return CriarTextoVarSub(bl);
}

//----------------------------------------------------------------------------
// Última variável
bool TTextoVar::FuncValorFim(TVariavel * v)
{
    TBlocoVar * bl = 0;
    if (RBroot)
    {
        if (Instr::VarAtual < v+1)
            bl = RBroot->RBlast();
        else
            bl = ProcFim(v[1].getTxt());
    }
    if (bl == 0)
        return false;
    TextoAtual = this;
    Instr::ApagarVar(v);
    if (TextoAtual == 0)
        return false;
    return CriarTextoVarSub(bl);
}

//----------------------------------------------------------------------------
// Mudar variável
bool TTextoVar::FuncMudar(TVariavel * v)
{
    if (Instr::VarAtual < v+1)
        return false;
    TTextoVarSub sub;
    if (Instr::VarAtual > v+1)
    {
        sub.Criar(this, v[1].getTxt(), true);
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
        sub.Criar(this, nome, true);
        sub.setTxt(o);
    }
    sub.Apagar();
    return false;
}

//----------------------------------------------------------------------------
// Nome da variável
bool TTextoVar::FuncNomeVar(TVariavel * v)
{
    TBlocoVar * bl = 0;
    if (Instr::VarAtual >= v+1)
        bl = Procura(v[1].getTxt());
    TextoAtual = this;
    Instr::ApagarVar(v);
    if (TextoAtual == 0)
        return false;
    return Instr::CriarVarTexto(bl ? bl->NomeVar : "");
}

//----------------------------------------------------------------------------
// Nome da variável
bool TTextoVar::FuncTipo(TVariavel * v)
{
    const char * texto = "";
    TBlocoVar * bl = 0;
    if (Instr::VarAtual >= v+1)
    {
        bl = Procura(v[1].getTxt());
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
// Variável anterior
bool TTextoVar::FuncAntes(TVariavel * v)
{
    TBlocoVar * bl = 0;
    if (Instr::VarAtual >= v+1)
        bl = ProcAntes(v[1].getTxt());
    if (Instr::VarAtual >= v+2 && bl)
    {
        int cmp = comparaZ(bl->NomeVar, v[2].getTxt());
        if (cmp!=0 && cmp!=2) // 0=textos iguais, 2=texto 1 contém texto 2
            bl = 0;
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(bl ? bl->NomeVar : "");
}

//----------------------------------------------------------------------------
// Próxima variável
bool TTextoVar::FuncDepois(TVariavel * v)
{
    TBlocoVar * bl = 0;
    if (Instr::VarAtual >= v+1)
        bl = ProcDepois(v[1].getTxt());
    if (Instr::VarAtual >= v+2 && bl)
    {
        int cmp = comparaZ(bl->NomeVar, v[2].getTxt());
        if (cmp!=0 && cmp!=2) // 0=textos iguais, 2=texto 1 contém texto 2
            bl = 0;
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(bl ? bl->NomeVar : "");
}

//----------------------------------------------------------------------------
// Início
bool TTextoVar::FuncIni(TVariavel * v)
{
    TBlocoVar * bl = 0;
    if (RBroot)
    {
        if (Instr::VarAtual < v+1)
            bl = RBroot->RBfirst();
        else
            bl = ProcIni(v[1].getTxt());
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(bl ? bl->NomeVar : "");
}

//----------------------------------------------------------------------------
// Fim
bool TTextoVar::FuncFim(TVariavel * v)
{
    TBlocoVar * bl = 0;
    if (RBroot)
    {
        if (Instr::VarAtual < v+1)
            bl = RBroot->RBlast();
        else
            bl = ProcFim(v[1].getTxt());
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(bl ? bl->NomeVar : "");
}

//----------------------------------------------------------------------------
// Limpar
bool TTextoVar::FuncLimpar(TVariavel * v)
{
    if (Instr::VarAtual < v+1)
    {
        Limpar();
        return false;
    }
    for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
    {
        const char * p = v1->getTxt();
        TBlocoVar * ini = ProcIni(p);
        if (ini==0)
            continue;
        TBlocoVar * fim = TBlocoVar::RBnext(ProcFim(p));
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
    const char * txt = "";
    int total = 0;
    if (Instr::VarAtual >= v+1)
        txt = v[1].getTxt();
    if (*txt==0)
    {
        total = Total;
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(total);
    }
    TBlocoVar * ini = ProcIni(txt);
    if (ini)
    {
        TBlocoVar * fim = ProcFim(txt);
        total=1;
        while (ini && ini != fim)
            total++, ini=TBlocoVar::RBnext(ini);
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarInt(total);
}

//----------------------------------------------------------------------------
void TTextoVar::Apagar()
{
    while (RBroot)
        delete RBroot;
    while (Inicio)
        Inicio->Apagar();
    if (TextoAtual == this)
        TextoAtual = 0;
}

//----------------------------------------------------------------------------
void TTextoVar::Limpar()
{
    while (RBroot)
        delete RBroot;
}

//----------------------------------------------------------------------------
void TTextoVar::Mover(TTextoVar * destino)
{
    if (RBroot)
        RBroot->MoveTextoVar(destino);
    for (TTextoVarSub * obj = Inicio; obj; obj=obj->Depois)
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

//----------------------------------------------------------------------------
TBlocoVar * TTextoVar::Procura(const char * texto)
{
    TBlocoVar * y = RBroot;
    while (y)
    {
        int i = comparaVar(texto, y->NomeVar);
        if (i==0)
            return y;
        if (i<0)
            y = y->RBleft;
        else
            y = y->RBright;
    }
    return 0;
}

//----------------------------------------------------------------------------
TBlocoVar * TTextoVar::ProcIni(const char * texto)
{
    TBlocoVar * x = 0;
    TBlocoVar * y = RBroot;
    while (y)
    {
        switch (comparaVar(texto, y->NomeVar))
        {
        case -2: // string 2 contém string 1
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
    TBlocoVar * x = 0;
    TBlocoVar * y = RBroot;
    while (y)
    {
        switch (comparaVar(texto, y->NomeVar))
        {
        case -2: // string 2 contém string 1
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
    TBlocoVar * x = 0;
    TBlocoVar * y = RBroot;
    while (y)
    {
        int i = comparaVar(texto, y->NomeVar);
        if (i <= 0)
            y = y->RBleft;
        else
            x=y, y = y->RBright;
    }
    return x;
}

//----------------------------------------------------------------------------
TBlocoVar * TTextoVar::ProcDepois(const char * texto)
{
    TBlocoVar * x = 0;
    TBlocoVar * y = RBroot;
    while (y)
    {
        int i = comparaVar(texto, y->NomeVar);
        if (i >= 0)
            y = y->RBright;
        else
            x=y, y = y->RBleft;
    }
    return x;
}

//----------------------------------------------------------------------------
void TTextoVarSub::Criar(TTextoVar * var, const char * nome, bool checatipo)
{
// Acerta variável conforme o nome
    char * p = copiastr(NomeVar, nome, sizeof(NomeVar));
    if (!checatipo)
        TipoVar = TextoVarTipoTxt;
    else if (p > NomeVar)
    {
        p--;
        switch (*p)
        {
        case ' ': *p=0; TipoVar=TextoVarTipoTxt; break;
        case '_': *p=0; TipoVar=TextoVarTipoNum; break;
        case '@': *p=0; TipoVar=TextoVarTipoDec; break;
        case '$': *p=0; TipoVar=TextoVarTipoRef; break;
        default:        TipoVar=TextoVarTipoTxt; break;
        }
    }

// Checa se é nome válido para variável (não pode ser um texto vazio)
    if (NomeVar[0] == 0)
    {
        TextoVar = 0;
        return;
    }

// Coloca na lista ligada
    TextoVar = var;
    Antes = 0;
    Depois = var->Inicio;
    if (var->Inicio)
        var->Inicio->Antes = this;
    var->Inicio = this;
}

//----------------------------------------------------------------------------
void TTextoVarSub::Apagar()
{
    if (TextoVar == 0)
        return;
    (Antes ? Antes->Depois : TextoVar->Inicio) = Depois;
    if (Depois)
        Depois->Antes = Antes;
    TextoVar = 0;
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

//----------------------------------------------------------------------------
bool TTextoVarSub::getBool()
{
    if (TextoVar==0)
        return false;
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
    return (bl ? bl->getBool() : false);
}

//----------------------------------------------------------------------------
int TTextoVarSub::getInt()
{
    if (TextoVar==0)
        return 0;
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
    return (bl ? bl->getInt() : 0);
}

//----------------------------------------------------------------------------
double TTextoVarSub::getDouble()
{
    if (TextoVar==0)
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
    if (TextoVar==0)
        return 0;
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
    return (bl ? bl->getObj() : 0);
}

//----------------------------------------------------------------------------
void TTextoVarSub::setInt(int valor)
{
    if (TextoVar==0)
        return;
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
// Variável existente: checa se deve mudar ou apagar/criar variável
    if (bl != 0)
    {
        if (bl->TipoVar() == TipoVar)
        {
            bl->setInt(valor);
            return;
        }
        delete bl;
        bl = 0;
    }
// Criar variável
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
    if (TextoVar==0)
        return;
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
// Variável existente: checa se deve mudar ou apagar/criar variável
    if (bl != 0)
    {
        if (bl->TipoVar() == TipoVar)
        {
            bl->setDouble(valor);
            return;
        }
        delete bl;
        bl = 0;
    }
// Criar variável
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
    if (TextoVar==0)
        return;
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
// Variável existente: checa se deve mudar ou apagar/criar variável
    if (bl != 0)
    {
        if (bl->TipoVar() == TipoVar)
        {
            bl->setTxt(txt);
            return;
        }
        delete bl;
        bl = 0;
    }
// Criar variável
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
    if (TextoVar==0 || *txt==0)
        return;
// Variável existente: checa se deve mudar ou apagar/criar variável
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
    if (bl != 0)
    {
        if (bl->TipoVar() == TipoVar)
        {
            bl->addTxt(txt);
            return;
        }
        delete bl;
        bl = 0;
    }
// Criar variável
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
    if (TextoVar==0)
        return;
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
// Nenhum objeto: apaga variável
    if (obj == 0)
    {
        if (bl)
            delete bl;
        return;
    }
// Variável existente: checa se deve mudar ou apagar/criar variável
    if (bl != 0)
    {
        if (bl->TipoVar() == TipoVar)
        {
            bl->setObj(obj);
            return;
        }
        delete bl;
        bl = 0;
    }
// Criar variável
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
#define RBmask 1 // Máscara para bit 0
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
    return true; // Texto não está vazio
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
    return 0;
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
    memcpy(p+tot1, txt, tot2);
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
    memcpy(p+tot1, txt, tot2);
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
    DoubleToTxt(txtnum, ValorDouble);
    return txtnum;
}

//------------------------------------------------------------------------------
TObjeto * TBlocoVarNum::getObj()
{
    return 0;
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
    // Avança TempoMenos
    // Move objetos de VetMais para VetMenos se necessário
        if (TempoMenos < INTTEMPO_MAX-1)
            TempoMenos++;
        else
        {
            TempoMenos = 0;
            TempoMais = (TempoMais + 1) % INTTEMPO_MAX;
            while (true)
            {
                TBlocoVarDec * obj = VetMais[TempoMais];
                if (obj==0)
                    break;
            // Move objeto da lista ligada VetMais para VetMenos
                VetMais[TempoMais] = obj->Depois;
                int menos = obj->IndiceMenos;
                obj->Antes = 0;
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
            if (obj==0)
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
    if (valor<0)
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
    sprintf(txtnum, "%d", getInt());
    return txtnum;
}

//----------------------------------------------------------------------------
TObjeto * TBlocoVarDec::getObj()
{
    return 0;
}

//----------------------------------------------------------------------------
void TBlocoVarDec::setInt(int valor)
{
    if (TextoVar == 0)
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
    if (TextoVar != 0)
        setInt(DoubleToInt(valor));
}

//----------------------------------------------------------------------------
void TBlocoVarDec::setTxt(const char * txt)
{
    if (TextoVar != 0)
        setInt(TxtToInt(txt));
}

//----------------------------------------------------------------------------
void TBlocoVarDec::addTxt(const char * txt)
{
    if (TextoVar != 0)
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
// Acerta os valores mínimo e máximo
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
    Antes = 0;
}

//----------------------------------------------------------------------------
void TBlocoVarDec::RemoveLista()
{
    if (Antes)
        Antes->Depois = Depois;
    else if (VetMenos[IndiceMenos]==this)
        VetMenos[IndiceMenos] = Depois;
    else if (VetMais[IndiceMais]==this)
        VetMais[IndiceMais] = Depois;
    if (Depois)
        Depois->Antes = Antes, Depois=0;
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
    if (obj == 0)
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
    ObjAntes = 0;
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
