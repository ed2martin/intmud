#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "var-textovar.h"
#include "variavel.h"
#include "instr.h"
#include "misc.h"

//#define DEBUG_MEM // Mostra variáveis criadas e apagadas

//----------------------------------------------------------------------------
bool TTextoVar::Func(TVariavel * v, const char * nome)
{
// Variável como texto
    if (comparaZ(nome, "txt")==0)
    {
        TBlocoVar * bl = 0;
        if (Instr::VarAtual >= v+1)
            bl = Procura(v[1].getTxt());
        Instr::ApagarVar(v);
        if (bl==0)
            return Instr::CriarVarTexto("");
        const char * p = bl->Texto;
        while (*p)
            p++;
        return Instr::CriarVarTexto(p+1);
    }
// Variável como valor numérico
    if (comparaZ(nome, "int")==0)
    {
        TBlocoVar * bl = 0;
        if (Instr::VarAtual >= v+1)
            bl = Procura(v[1].getTxt());
        Instr::ApagarVar(v);
        if (bl==0)
            return Instr::CriarVarInt(0);
        const char * p = bl->Texto;
        while (*p)
            p++;
        if (!Instr::CriarVar(Instr::InstrDouble))
            return false;
        double num;
        errno=0, num=strtod(p+1, 0);
        Instr::VarAtual->setDouble(errno ? 0 : num);
        return true;
    }
// Nome da variável
    if (comparaZ(nome, "var")==0)
    {
        TBlocoVar * bl = 0;
        if (Instr::VarAtual >= v+1)
            bl = Procura(v[1].getTxt());
        Instr::ApagarVar(v);
        if (bl==0)
            return Instr::CriarVarTexto("");
        return Instr::CriarVarTexto(bl->Texto);
    }
// Mudar variável
    if (comparaZ(nome, "add")==0)
    {
        for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
            Mudar(v1->getTxt());
        return false;
    }
// Variável anterior
    if (comparaZ(nome, "antes")==0)
    {
        TBlocoVar * bl = 0;
        if (Instr::VarAtual >= v+1)
            bl = ProcAntes(v[1].getTxt());
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(bl ? bl->Texto : "");
    }
// Próxima variável
    if (comparaZ(nome, "depois")==0)
    {
        TBlocoVar * bl = 0;
        if (Instr::VarAtual >= v+1)
            bl = ProcDepois(v[1].getTxt());
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(bl ? bl->Texto : "");
    }
// Início
    if (comparaZ(nome, "ini")==0)
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
// Fim
    if (comparaZ(nome, "fim")==0)
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
// Limpar
    if (comparaZ(nome, "limpar")==0)
    {
        Limpar();
        return false;
    }
    return false;
}

//----------------------------------------------------------------------------
void TTextoVar::Apagar()
{
    while (RBroot)
        RBroot->Apagar();
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
    for (TBlocoVar * obj = RBroot->RBfirst(); obj; obj=TBlocoVar::RBnext(obj))
        obj->TextoVar = destino;
    move_mem(destino, this, sizeof(TTextoVar));
}

//----------------------------------------------------------------------------
TBlocoVar * TTextoVar::Procura(const char * texto)
{
    TBlocoVar * y = RBroot;
    while (y)
    {
        int i = comparaZ(texto, y->Texto);
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
        switch (comparaZ(texto, y->Texto))
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
        switch (comparaZ(texto, y->Texto))
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
        int i = comparaZ(texto, y->Texto);
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
        int i = comparaZ(texto, y->Texto);
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
// Obtém nome e conteúdo da variável
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
// Inserir texto (não está no textovar)
    TBlocoVar * bl = Procura(nomevar);
    if (bl==0)
    {
        if (*txtvar==0) // Texto vazio: não tem como apagar
            return 0;
        bl = TBlocoVar::AlocarMem(texto);
        bl->TextoVar = this;
        bl->RBinsert();
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
    printf("Apagar %p(%s); moveu para %p(%s)\n", bl->Inicio, bl->Texto,
           bl2->Inicio, bl2->Texto); fflush(stdout);
#endif
    delete[] bl->Inicio;
    return bl2;
}

//----------------------------------------------------------------------------
TBlocoVar * TBlocoVar::AlocarMem(const char * texto)
{
    TBlocoVar * bl = 0;
    int total1 = strlen(texto) + 1; // Tamanho do texto
    int total2 = total1 + (bl->Texto - bl->Inicio); // Tamanho do bloco
    int total3 = (total2+7) & ~7;   // Quantidade de bytes alocados
    bl = reinterpret_cast<TBlocoVar*>(new char*[total3]);
    bl->Bytes = total1 + total3 - total2;
    memcpy(bl->Texto, texto, total1);
    char * p = bl->Texto;
    while (*p && *p!='=') p++;
    *p=0;
#ifdef DEBUG_MEM
    printf("Criar %p: %s\n", bl->Inicio, bl->Texto); fflush(stdout);
#endif
    return bl;
}

//----------------------------------------------------------------------------
void TBlocoVar::Apagar()
{
#ifdef DEBUG_MEM
    printf("Apagar %p: %s\n", Inicio, Texto); fflush(stdout);
#endif
    RBremove();
    delete[] Inicio;
}

//----------------------------------------------------------------------------
int TBlocoVar::RBcomp(TBlocoVar * x, TBlocoVar * y)
{
    return comparaZ(x->Texto, y->Texto);
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
#define RBmask 1 // Máscara para bit 0
#define RBroot TextoVar->RBroot
#include "rbt.cpp.h"
