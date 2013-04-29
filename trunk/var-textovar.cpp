#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "var-textovar.h"
#include "variavel.h"
#include "instr.h"
#include "misc.h"

//#define DEBUG_MEM // Mostra vari�veis criadas e apagadas

//----------------------------------------------------------------------------
bool TTextoVar::Func(TVariavel * v, const char * nome)
{
// Lista das fun��es de indiceitem
// Deve obrigatoriamente estar em letras min�sculas e ordem alfab�tica
    static const struct {
        const char * Nome;
        bool (TTextoVar::*Func)(TVariavel * v); } ExecFunc[] = {
        { "antes",        &TTextoVar::FuncAntes },
        { "depois",       &TTextoVar::FuncDepois },
        { "fim",          &TTextoVar::FuncFim },
        { "ini",          &TTextoVar::FuncIni },
        //{ "int",        &TTextoVar::FuncInt },
        { "limpar",       &TTextoVar::FuncLimpar },
        { "mudar",        &TTextoVar::FuncMudar },
        { "nomevar",      &TTextoVar::FuncNomeVar },
        { "total",        &TTextoVar::FuncTotal },
        { "valor",        &TTextoVar::FuncValor }  };
// Procura a fun��o correspondente e executa
    int ini = 0;
    int fim = sizeof(ExecFunc) / sizeof(ExecFunc[0]) - 1;
    char mens[80];
    copiastrmin(mens, nome, sizeof(mens));
    while (ini <= fim)
    {
        int meio = (ini+fim)/2;
        int resultado = strcmp(mens, ExecFunc[meio].Nome);
        if (resultado==0) // Se encontrou...
            return (this->*ExecFunc[meio].Func)(v);
        if (resultado<0) fim=meio-1; else ini=meio+1;
    }
// Outro nome de vari�vel
    TTextoVarSub sub1;
    int tipo = varTxt;
    sub1.Criar(this);
    char * p = copiastr(sub1.NomeVar, nome, sizeof(sub1.NomeVar));
    if (*nome && p[-1]=='_')
        p[-1]=0, tipo=varDouble;
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarTextoVarSub))
    {
        sub1.Apagar();
        return false;
    }
    Instr::VarAtual->numfunc = tipo;
    sub1.Mover(Instr::VarAtual->end_textovarsub);
    return true;
}

//----------------------------------------------------------------------------
// Vari�vel como texto
bool TTextoVar::FuncValor(TVariavel * v)
{
    TBlocoVar * bl = 0;
    if (Instr::VarAtual >= v+1)
        bl = Procura(v[1].getTxt());
    Instr::ApagarVar(v);
    if (bl==0)
        return Instr::CriarVarTexto("");
    const char * p = bl->Texto;
    while (*p++);
//printf("\nLEU DE: %p\n", p); fflush(stdout);
//printf("\n%s\n\n\n", p); fflush(stdout);
    return Instr::CriarVarTexto(p);
}

//----------------------------------------------------------------------------
// Vari�vel como valor num�rico
/* bool TTextoVar::FuncInt(TVariavel * v)
{
    TBlocoVar * bl = 0;
    if (Instr::VarAtual >= v+1)
        bl = Procura(v[1].getTxt());
    Instr::ApagarVar(v);
    if (bl==0)
        return Instr::CriarVarInt(0);
    const char * p = bl->Texto;
    while (*p++);
    if (!Instr::CriarVar(Instr::InstrDouble))
        return false;
    double num;
    errno=0, num=strtod(p+1, 0);
    Instr::VarAtual->setDouble(errno ? 0 : num);
    return true;
} */

//----------------------------------------------------------------------------
// Nome da vari�vel
bool TTextoVar::FuncNomeVar(TVariavel * v)
{
    TBlocoVar * bl = 0;
    if (Instr::VarAtual >= v+1)
        bl = Procura(v[1].getTxt());
    Instr::ApagarVar(v);
    if (bl==0)
        return Instr::CriarVarTexto("");
    return Instr::CriarVarTexto(bl->Texto);
}

//----------------------------------------------------------------------------
// Mudar vari�vel
bool TTextoVar::FuncMudar(TVariavel * v)
{
    for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
        Mudar(v1->getTxt());
    return false;
}

//----------------------------------------------------------------------------
// Vari�vel anterior
bool TTextoVar::FuncAntes(TVariavel * v)
{
    TBlocoVar * bl = 0;
    if (Instr::VarAtual >= v+1)
        bl = ProcAntes(v[1].getTxt());
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(bl ? bl->Texto : "");
}

//----------------------------------------------------------------------------
// Pr�xima vari�vel
bool TTextoVar::FuncDepois(TVariavel * v)
{
    TBlocoVar * bl = 0;
    if (Instr::VarAtual >= v+1)
        bl = ProcDepois(v[1].getTxt());
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(bl ? bl->Texto : "");
}

//----------------------------------------------------------------------------
// In�cio
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
    return Instr::CriarVarTexto(bl ? bl->Texto : "");
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
    return Instr::CriarVarTexto(bl ? bl->Texto : "");
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
            ini->Apagar();
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
        RBroot->Apagar();
    while (Inicio)
        Inicio->Apagar();
}

//----------------------------------------------------------------------------
void TTextoVar::Limpar()
{
    while (RBroot)
        RBroot->Apagar();
}

//----------------------------------------------------------------------------
void TTextoVar::Mover(TTextoVar * destino)
{
    if (RBroot)
        RBroot->MoveTextoVar(destino);
    for (TTextoVarSub * obj = Inicio; obj; obj=obj->Depois)
        obj->TextoVar = destino;
    memmove(destino, this, sizeof(TTextoVar));
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
        int i = comparaVar(texto, y->Texto);
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
        switch (comparaVar(texto, y->Texto))
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
    TBlocoVar * x = 0;
    TBlocoVar * y = RBroot;
    while (y)
    {
        switch (comparaVar(texto, y->Texto))
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
    TBlocoVar * x = 0;
    TBlocoVar * y = RBroot;
    while (y)
    {
        int i = comparaVar(texto, y->Texto);
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
        int i = comparaVar(texto, y->Texto);
        if (i >= 0)
            y = y->RBright;
        else
            x=y, y = y->RBleft;
    }
    return x;
}

//----------------------------------------------------------------------------
TBlocoVar * TTextoVar::Mudar(const char * texto)
{
// Obt�m nome e conte�do da vari�vel
    char nomevar[256];
    const char * txtvar = texto;
    while (*txtvar && *txtvar!='=')
        txtvar++;
    if (txtvar==texto || txtvar-texto >= (int)sizeof(nomevar))
        return 0;
    memcpy(nomevar, texto, txtvar-texto);
    nomevar[txtvar-texto] = 0;
    if (*txtvar=='=')
        txtvar++;
// Inserir texto (n�o est� no textovar)
    TBlocoVar * bl = Procura(nomevar);
    if (bl==0)
    {
        if (*txtvar==0) // Texto vazio: n�o tem como apagar
            return 0;
        bl = TBlocoVar::AlocarMem(texto);
        bl->TextoVar = this;
        bl->RBinsert();
        Total++;
        return 0;
    }
// Apagar texto
    if (*txtvar==0)
    {
        bl->Apagar();
        return 0;
    }
// Alterar texto
    unsigned int total = strlen(texto) + 1;
    if (total <= bl->Bytes && total+32 > bl->Bytes)
    {
        memcpy(bl->Texto, texto, total);
        char * p = bl->Texto;
        while (*p && *p!='=') p++;
        *p=0;
        return bl;
    }
    TBlocoVar * bl2 = TBlocoVar::AlocarMem(texto);
    bl2->TextoVar = this;
    bl->RBmove(bl2);
#ifdef DEBUG_MEM
    printf("Apagar %p(%s); moveu para %p(%s)\n", bl, bl->Texto,
           bl2, bl2->Texto); fflush(stdout);
#endif
    delete[] (char*)bl;
    return bl2;
}

//----------------------------------------------------------------------------
void TTextoVarSub::Criar(TTextoVar * var)
{
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
    if (TextoVar==0) return 0;
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
    if (bl==0) return 0;
    const char * p = bl->Texto;
    while (*p++);
    return (*p != 0);
}

//----------------------------------------------------------------------------
int TTextoVarSub::getInt()
{
    if (TextoVar==0) return 0;
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
    if (bl==0) return 0;
    const char * p = bl->Texto;
    while (*p++);
    long num;
    errno=0, num=strtol(p, 0, 10);
    return (errno ? 0 : num);
}

//----------------------------------------------------------------------------
double TTextoVarSub::getDouble()
{
    if (TextoVar==0) return 0;
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
    if (bl==0) return 0;
    const char * p = bl->Texto;
    while (*p++);
    double num;
    errno=0, num=strtod(p, 0);
    return (errno ? 0 : num);
}

//----------------------------------------------------------------------------
const char * TTextoVarSub::getTxt()
{
    if (TextoVar==0) return "";
    TBlocoVar * bl = TextoVar->Procura(NomeVar);
    if (bl==0) return "";
    const char * p = bl->Texto;
    while (*p++);
    return p;
}

//----------------------------------------------------------------------------
void TTextoVarSub::setTxt(const char * txt)
{
    if (TextoVar==0)
        return;
    char mens[BUF_MENS];
    mprintf(mens, sizeof(mens), "%s=%s", NomeVar, txt);
    TextoVar->Mudar(mens);
}

//----------------------------------------------------------------------------
void TTextoVarSub::addTxt(const char * txt)
{
    if (TextoVar==0)
        return;
    char mens[BUF_MENS];
    mprintf(mens, sizeof(mens), "%s=%s%s", NomeVar, getTxt(), txt);
    TextoVar->Mudar(mens);
}

//----------------------------------------------------------------------------
TBlocoVar * TBlocoVar::AlocarMem(const char * texto)
{
    TBlocoVar * bl = 0;
    int total1 = strlen(texto) + 1; // Tamanho do texto
    int total2 = total1 + (bl->Texto - (char*)bl); // Tamanho do bloco
    int total3 = (total2+7) & ~7;   // Quantidade de bytes alocados
    bl = reinterpret_cast<TBlocoVar*>(new char[total3]);
    bl->Bytes = total1 + total3 - total2;
    memcpy(bl->Texto, texto, total1);
    char * p = bl->Texto;
    while (*p && *p!='=') p++;
    *p=0;
//printf("\nANOTOU EM: %p\n", p+1); fflush(stdout);
//printf("\n%s\n", p+1); fflush(stdout);
#ifdef DEBUG_MEM
    printf("Criar %p: %s\n", bl, bl->Texto); fflush(stdout);
#endif
    return bl;
}

//----------------------------------------------------------------------------
void TBlocoVar::Apagar()
{
#ifdef DEBUG_MEM
    printf("Apagar %p: %s\n", this, Texto); fflush(stdout);
#endif
    TextoVar->Total--;
    RBremove();
    delete[] (char*)this;
}

//----------------------------------------------------------------------------
int TBlocoVar::RBcomp(TBlocoVar * x, TBlocoVar * y)
{
    return comparaVar(x->Texto, y->Texto);
}

//----------------------------------------------------------------------------
void TBlocoVar::RBmove(TBlocoVar * novoender)
{
    if (novoender==this)
        return;
    novoender->RBparent = RBparent;
    novoender->RBleft = RBleft;
    novoender->RBright = RBright;
    novoender->RBcolour = RBcolour;
    if (TextoVar->RBroot==this)
        TextoVar->RBroot=novoender;
    if (RBparent)
    {
        if (RBparent->RBleft==this)
            RBparent->RBleft=novoender;
        else if (RBparent->RBright==this)
            RBparent->RBright=novoender;
    }
    if (RBleft)
        RBleft->RBparent=novoender;
    if (RBright)
        RBright->RBparent=novoender;
}

//----------------------------------------------------------------------------
#define CLASS TBlocoVar          // Nome da classe
#define RBmask 1 // M�scara para bit 0
#define RBroot TextoVar->RBroot
#include "rbt.cpp.h"