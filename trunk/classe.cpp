/* Este programa é software livre; você pode redistribuir e/ou
 * modificar nos termos da GNU General Public License V2
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details at www.gnu.org
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "classe.h"
#include "instr.h"
#include "misc.h"

//----------------------------------------------------------------------------
TClasse::TClasse(const char * nome)
{
    Comandos=0;
    copiastr(Nome, nome, sizeof(Nome));
    ListaDeriv=0;
    NumDeriv=0;
    InstrVar=0;
    IndiceVar=0;
    NumVar=0;
    RBinsert();
}

//----------------------------------------------------------------------------
TClasse::~TClasse()
{
    RBremove();
    if (Comandos)     delete[] Comandos;
    if (ListaDeriv)   delete[] ListaDeriv;
    if (InstrVar)     delete[] InstrVar;
    if (IndiceVar)    delete[] IndiceVar;
}

//----------------------------------------------------------------------------
bool TClasse::NomeClasse(char * nome)
{
    char *o,*d;
// Verifica se tem algum caracter inválido
    for (o=nome; *o; o++)
        if (tabNOMES[*(unsigned char*)o]==0)
            return false;
// Avança para início do nome
    for (o=nome; *o==' '; o++);
    if (*o>='0' && *o<='9')
        return false;
// Retira espaços desnecessários
    d=nome;
    for (; *o; o++)
        if (*o!=' ' || (o[1]!=' ' && o[1]))
            *d++=*o;
    *d=0;
// Checa tamanho do nome
    TClasse * cl=0; // Apenas para sizeof(TClasse::Nome)
    if (d-nome >= (int)sizeof(cl->Nome))
        return false;
    return true;
}

//----------------------------------------------------------------------------
void TClasse::AcertaComandos()
{
    if (Comandos==0)
        return;

    char * x;
    int nivelse;

// Primeiro zera endereços de desvio
    for (char * p = Comandos; p[0] && p[1]; p+=Num16(p))
        switch (p[2])
        {
        case Instr::cSe:
        case Instr::cSenao1:
        case Instr::cSenao2:
        case Instr::cEnquanto:
        case Instr::cEFim:
        case Instr::cSair:
        case Instr::cContinuar:
            p[3] = 0;
            p[4] = 0;
        }

// Acerta endereços de desvio conforme instruções
    for (char * p = Comandos; p[0] && p[1]; p+=Num16(p))
        switch (p[2])
        {
        case Instr::cSe:
        case Instr::cSenao1:
        case Instr::cSenao2:
            nivelse=0;
            for (x=p+Num16(p); x[0] && x[1]; x+=Num16(x))
                if (x[2] >= Instr::cVariaveis)
                    break;
                else if (x[2]==Instr::cSe)
                    nivelse++;
                else if (nivelse==0 && (x[2]==Instr::cSenao1 ||
                                        x[2]==Instr::cSenao2))
                    break;
                else if (x[2]==Instr::cFimSe)
                    if (nivelse)
                        nivelse--;
                    else
                        break;
            p[3] = (x-p);
            p[4] = (x-p)>>8;
            break;
        case Instr::cEnquanto:
            nivelse=0;
            for (x=p+Num16(p); x[0] && x[1]; x+=Num16(x))
                if (x[2] >= Instr::cVariaveis)
                    break;
                else if (x[2]==Instr::cEnquanto)
                    nivelse++;
                else if (x[2]==Instr::cContinuar)
                {
                    x[3] = (x-p);
                    x[4] = (x-p) >> 8;
                }
                else if (x[2]==Instr::cEFim)
                    if (nivelse)
                        nivelse--;
                    else
                    {
                        x[3] = (x-p);
                        x[4] = (x-p) >> 8;
                        x+=Num16(x);
                        break;
                    }
            p[3] = (x-p);
            p[4] = (x-p)>>8;
            break;
        case Instr::cSair:
            nivelse=0;
            for (x=p+Num16(p); x[0] && x[1]; x+=Num16(x))
                if (x[2] >= Instr::cVariaveis)
                    break;
                else if (x[2]==Instr::cEnquanto)
                    nivelse++;
                else if (x[2]==Instr::cEFim)
                    if (nivelse)
                        nivelse--;
                    else
                    {
                        x+=Num16(x);
                        break;
                    }
            p[3] = (x-p);
            p[4] = (x-p)>>8;
            break;
        }
}

//----------------------------------------------------------------------------
void TClasse::AcertaVar()
{
// Limpa lista de variáveis/funções
    if (InstrVar)   delete[] InstrVar;
    if (IndiceVar)  delete[] IndiceVar;
    InstrVar=0;
    IndiceVar=0;
    NumVar=0;

// Nenhuma instrução: nada faz
    if (Comandos==0)
        return;

// Obtém número de variáveis/funções definidas na classe
    int total = 0;
    bool inifunc = false;
    const char * ComandosFim = Comandos;
    for (char * p = Comandos; p[0] && p[1]; p+=Num16(p))
    {
        ComandosFim = p;
        switch (p[2])
        {
        case Instr::cConstNulo:
        case Instr::cConstTxt:
        case Instr::cConstNum:
        case Instr::cConstExpr:
            total++;
            break;
        case Instr::cFunc:
        case Instr::cVarFunc:
            total++, inifunc=true;
            break;
        default:
            if (inifunc==false && p[2] > Instr::cVariaveis)
                total++;
        }
    }

// Soma com número de variáveis/funções herdadas
    if ((Comandos[0] || Comandos[1]) && Comandos[2]==Instr::cHerda)
    {
        const char * p = Comandos+4;
        int x = (unsigned char)Comandos[3];
        for (; x; x--)
        {
            TClasse * c = Procura(p);
            assert(c!=0);
            for (unsigned int y=0; y<c->NumVar; y++)
                if (c->IndiceVar[y] & 0x2000000)
                    total++;
            while (*p++);
        }
    }
    if (total==0)
        return;

// Aloca memória
    char ** var1 = new char*[total];
    total = 0;

// Adiciona funções/variáveis da própria classe
    inifunc = false;
    for (char * p = Comandos; p[0] && p[1]; p+=Num16(p))
        switch (p[2])
        {
        case Instr::cConstNulo:
        case Instr::cConstTxt:
        case Instr::cConstNum:
        case Instr::cConstExpr:
            var1[total++] = p;
            break;
        case Instr::cFunc:
        case Instr::cVarFunc:
            var1[total++] = p;
            inifunc=true;
            break;
        default:
            if (inifunc==false && p[2] > Instr::cVariaveis)
                var1[total++] = p;
        }

// Adiciona variáveis/funções herdadas
    if ((Comandos[0] || Comandos[1]) && Comandos[2]==Instr::cHerda)
    {
        const char * p = Comandos+4;
        int x = (unsigned char)Comandos[3];
        for (; x; x--)
        {
            TClasse * c = Procura(p);
            assert(c!=0);
            for (unsigned int y=0; y<c->NumVar; y++)
                if (c->IndiceVar[y] & 0x2000000)
                    var1[total++] = c->InstrVar[y];
            while (*p++);
        }
    }

// Organiza em ordem alfabética (aloca/libera memória - var2)
    char ** var2 = new char*[total];
    for (int a=1; a<total; a+=a)
    {
        char ** pont = var2;
        var2 = var1;
        var1 = pont;
        int lido=0;
        for (int b=0; b<total; b+=a*2)
        {
            int b1=b, b2=b+a;
            while (b1<b+a && b2<b+a*2 && b2<total)
            {
                if (comparaZ(var2[b1]+5, var2[b2]+5)>0)
                    var1[lido++] = var2[b2++];
                else
                    var1[lido++] = var2[b1++];
            }
            while (b1<b+a && b1<total)
                var1[lido++] = var2[b1++];
            while (b2<b+a*2 && b2<total)
                var1[lido++] = var2[b2++];
        }
    }
    delete[] var2;

// Obtém número de variáveis, detectando as repetidas
    NumVar = total;
    for (int x=total-2; x>=0; x--)
        if (comparaZ(var1[x]+5, var2[x+1]+5)==0)
        {
            var2[x+1]=0;
            NumVar--;
        }

// Acerta TClasse::InstrVar (nomes das variáveis)
    InstrVar = new char*[NumVar];
    for (int x=0,y=0; x<total; x++)
        if (var2[x])
            InstrVar[y++] = var2[x];

// Libera memória alocada
    delete[] var1;

// Acerta bits de controle de TClasse::IndiceVar
    IndiceVar = new int[NumVar];
    for (unsigned int x=0; x<NumVar; x++)
    {
        unsigned int valor = 0;
    // Verifica se está na própria classe
        if (InstrVar[x]>=Comandos && InstrVar[x]<=ComandosFim)
            valor |= 0x2000000;
    // Verifica se é variável "comum"
        if (InstrVar[x][3] & 1)
            valor |= 0x1000000;
        IndiceVar[x] = valor;
    }


}

//----------------------------------------------------------------------------
TClasse * TClasse::Procura(const char * nome)
{
    int i;
    TClasse * y = RBroot;
    while (y)
    {
        i=comparaZ(nome, y->Nome);
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
int TClasse::RBcomp(TClasse * x, TClasse * y)
{
    return comparaZ(x->Nome, y->Nome);
}

//----------------------------------------------------------------------------
TClasse * TClasse::RBroot=0;
#define CLASS TClasse          // Nome da classe
#define RBmask 1 // Máscara para bit 0
#include "rbt.cpp.h"
