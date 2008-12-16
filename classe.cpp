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
#include <string.h>
#include <assert.h>
#include "classe.h"
#include "objeto.h"
#include "variavel.h"
#include "instr.h"
#include "misc.h"

//----------------------------------------------------------------------------
/// Construtor
/**
 *  @param nome nome da classe
 *
 *  Para criar uma classe:
 *  - Criar um objeto com new TClasse()
 *  - Colocar as instruções em TClasse::Comandos (codificadas com Instr::Codif)
 *  - Chamar TClasse::AcertaComandos para acertar as instruções
 *  - Chamar TClasse::AcertaVar para levantar as variáveis existentes
 *  - Chamar TClasse::CriaVars para criar as variáveis da classe
 */
TClasse::TClasse(const char * nome)
{
    Comandos=0;
    copiastr(Nome, nome, sizeof(Nome));
    ListaDeriv=0;
    NumDeriv=0;
    InstrVar=0;
    IndiceVar=0;
    NumVar=0;
    ObjetoIni=0;
    ObjetoFim=0;
    NumObj=0;
    TamObj=0;
    TamVars=0;
    Vars=0;
    RBinsert();
}

//----------------------------------------------------------------------------
/// Destrutor
/** Para apagar uma classe:
 *  - Certificar-se que nenhuma classe herda a classe a ser apagada
 *  - Cuidado com herança circular (duas classes, uma herdando a outra)
 *  - Apagar todos os objetos da classe
 *  - Apagar a classe
 *  .
 *  @note Antes de apagar, não deve haver nenhuma classe derivada
 */
TClasse::~TClasse()
{
    while (ObjetoIni)
        ObjetoIni->Apagar();
    ApagaVars();
    RBremove();
    if (Comandos)     delete[] Comandos;
    if (ListaDeriv)   delete[] ListaDeriv;
    if (InstrVar)     delete[] InstrVar;
    if (IndiceVar)    delete[] IndiceVar;
}

//----------------------------------------------------------------------------
bool TClasse::NomeValido(char * nome)
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
    for (char * p = Comandos; p[0] || p[1]; p+=Num16(p))
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
    for (char * p = Comandos; p[0] || p[1]; p+=Num16(p))
        switch (p[2])
        {
        case Instr::cSe:
        case Instr::cSenao1:
        case Instr::cSenao2:
            nivelse=0;
            for (x=p+Num16(p); x[0] || x[1]; x+=Num16(x))
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
            for (x=p+Num16(p); x[0] || x[1]; x+=Num16(x))
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
            for (x=p+Num16(p); x[0] || x[1]; x+=Num16(x))
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
    ApagaVars();
    if (InstrVar)   delete[] InstrVar;
    if (IndiceVar)  delete[] IndiceVar;
    InstrVar=0;
    IndiceVar=0;
    NumVar=0;
    TamObj=0;
    TamVars=0;

// Nenhuma instrução: nada faz
    if (Comandos==0)
        return;

// Obtém número de variáveis/funções definidas na classe
    unsigned int total = 0;
    bool inifunc = false;
    const char * ComandosFim = Comandos;
    for (char * p = Comandos;; p+=Num16(p))
    {
        if (p[0]==0 && p[1]==0)
        {
            ComandosFim = p;
            break;
        }
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
                if (c->IndiceVar[y] & 0x800000)
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
    for (char * p = Comandos; p[0] || p[1]; p+=Num16(p))
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
                if (c->IndiceVar[y] & 0x800000)
                    var1[total++] = c->InstrVar[y];
            while (*p++);
        }
    }

// Organiza em ordem alfabética (aloca/libera memória - var2)
    char ** var2 = new char*[total];
    for (unsigned int a=1; a<total; a+=a)
    {
        char ** pont = var2;
        var2 = var1;
        var1 = pont;
        int lido=0;
        for (unsigned int b=0; b<total; b+=a*2)
        {
            unsigned int b1=b, b2=b+a;
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
    var2=0;

// Obtém número de variáveis, detectando as repetidas
    NumVar = total;
    for (int x=total-2; x>=0; x--)
        if (comparaZ(var1[x]+5, var1[x+1]+5)==0)
        {
            var1[x+1]=0;
            NumVar--;
        }

// Acerta TClasse::InstrVar (nomes das variáveis)
// Libera memória alocada
    if (NumVar == total)
        InstrVar = var1;
    else
    {
        InstrVar = new char*[NumVar];
        for (unsigned int x=0,y=0; x<total; x++)
            if (var1[x])
                InstrVar[y++] = var1[x];
        delete[] var1;
    }
    var1=0;

// Acerta bits de controle de TClasse::IndiceVar
// Acerta variáveis cInt1 (alinhamento de bit)
    int indclasse=-1, bitclasse=0x80;
    int indobjeto=-1, bitobjeto=0x80;
    IndiceVar = new unsigned int[NumVar];
    for (unsigned int x=0; x<NumVar; x++)
    {
        unsigned int valor = 0;
    // Verifica se está na própria classe
        if (InstrVar[x]>=Comandos && InstrVar[x]<=ComandosFim)
            valor |= 0x800000;
    // Verifica se é variável "comum"
        if (InstrVar[x][3] & 1)
            valor |= 0x400000;
    // Verifica cBit1
        if (InstrVar[x][2] == Instr::cInt1)
        {
            if (valor & 0x400000) // Classe
            {
                if (bitclasse==0x80)
                    bitclasse=1, indclasse++;
                else
                    bitclasse <<= 1;
                valor += indclasse + (bitclasse << 24);
            }
            else // Objeto
            {
                if (bitobjeto==0x80)
                    bitobjeto=1, indobjeto++;
                else
                    bitobjeto <<= 1;
                valor += indobjeto + (bitobjeto << 24);
            }
        }
    // Anota o resultado
        IndiceVar[x] = valor;
    }
    indclasse++, indobjeto++;

// Acerta variáveis com alinhamento de 1 byte
    for (unsigned int x=0; x<NumVar; x++)
    {
        if (InstrVar[x][2] == Instr::cInt1)
            continue;
        int tamanho = TVariavel::Tamanho(InstrVar[x]);
        if (tamanho&1)
            if (IndiceVar[x] & 0x400000) // Classe
                IndiceVar[x] += indclasse, indclasse += tamanho;
            else // Objeto
                IndiceVar[x] += indobjeto, indobjeto += tamanho;
    }
    if (indclasse&1) indclasse++;
    if (indobjeto&1) indobjeto++;

// Acerta variáveis com alinhamento de 2 bytes
    for (unsigned int x=0; x<NumVar; x++)
    {
        if (InstrVar[x][2] == Instr::cInt1)
            continue;
        int tamanho = TVariavel::Tamanho(InstrVar[x]);
        if ((tamanho&3)==2)
            if (IndiceVar[x] & 0x400000) // Classe
                IndiceVar[x] += indclasse, indclasse += tamanho;
            else // Objeto
                IndiceVar[x] += indobjeto, indobjeto += tamanho;
    }
    if (indclasse&2) indclasse+=2;
    if (indobjeto&2) indobjeto+=2;

// Acerta outras variáveis (alinhamento de 4 bytes)
    for (unsigned int x=0; x<NumVar; x++)
    {
        if (InstrVar[x][2] == Instr::cInt1)
            continue;
        int tamanho = TVariavel::Tamanho(InstrVar[x]);
        if (tamanho && (tamanho&3)==0)
            if (IndiceVar[x] & 0x400000) // Classe
                IndiceVar[x] += indclasse, indclasse += tamanho;
            else // Objeto
                IndiceVar[x] += indobjeto, indobjeto += tamanho;
    }

// Acerta TClasse::Vars
    if (indclasse)
    {
        Vars = new char[indclasse];
        memset(Vars, 0, indclasse);
    // Chama construtores das variáveis
        TVariavel v;
        for (int x=(int)NumVar-1; x>=0; x--)
            if (InstrVar[x][2] > Instr::cVarFunc &&
                    (IndiceVar[x] & 0x400000))
            {
                v.endvar = Vars + (IndiceVar[x] & 0x3FFFFF);
                v.defvar = InstrVar[x];
                v.Criar(this, 0);
            }
    }
    TamVars = indclasse;
    TamObj = indobjeto;

// Mostra o resultado
#if 1
    printf("Classe=\"%s\"   Variáveis classe=%d   Variáveis Objeto=%d\n",
           Nome, indclasse, indobjeto);
    puts("  lugar  endereço:bit  tamanho  instrução");
    for (int passo=0; passo<2; passo++)
        for (unsigned int x=0; x<NumVar; x++)
        {
            unsigned int valor = IndiceVar[x];
            if (((valor & 0x400000)==0) == (passo==0))
                continue;
            int tam = TVariavel::Tamanho(InstrVar[x]);
            char mens[500];
            if (!Instr::Decod(mens, InstrVar[x], sizeof(mens)))
                strcpy(mens, "-----");
            printf("  %s  %d",
                    IndiceVar[x] & 0x400000 ? "classe" : "objeto",
                    valor & 0x3FFFFF);
            if ((valor>>24)!=0)
                printf(":%02x", valor>>24);
            printf("  %d  %s\n", tam, mens);
        }
    putchar('\n');
#endif
}

//----------------------------------------------------------------------------
void TClasse::CriaVars()
{
    ApagaVars();
    if (TamVars==0)
        return;
// Acerta TClasse::Vars
    Vars = new char[TamVars];
    memset(Vars, 0, TamVars);
// Chama construtores das variáveis
    TVariavel v;
    for (int x=(int)NumVar-1; x>=0; x--)
        if (InstrVar[x][2] > Instr::cVarFunc &&
                (IndiceVar[x] & 0x400000))
        {
            v.endvar = Vars + (IndiceVar[x] & 0x3FFFFF);
            v.defvar = InstrVar[x];
            v.Criar(this, 0);
        }
}

//----------------------------------------------------------------------------
void TClasse::ApagaVars()
{
// Chama destrutores das variáveis
    TVariavel v;
    for (int x=(int)NumVar-1; x>=0; x--)
        if (InstrVar[x][2] > Instr::cVarFunc &&
                (IndiceVar[x] & 0x400000))
        {
            v.endvar = Vars + (IndiceVar[x] & 0x3FFFFF);
            v.defvar = InstrVar[x];
            v.Apagar();
        }
// Libera memória
    if (Vars)       delete[] Vars;
    Vars=0;
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
int TClasse::IndiceNome(const char * nome)
{
    int ini = 0; // Índice inicial
    int fim = NumVar - 1; // Índice final
    while (ini<=fim)
    {
        int meio = (ini+fim)/2;
        switch (comparaZ(nome, InstrVar[meio] + 5))
        {
        case 2:
        case 1:
            ini = meio+1;
            break;
        case 0:
            return meio;
        case -1:
        case -2:
            fim = meio-1;
        }
    }
    return -1;
}

//----------------------------------------------------------------------------
int TClasse::IndiceNome2(const char * nome)
{
    int ini = 0; // Índice inicial
    int fim = NumVar - 1; // Índice final
    int ind = -1;
    while (ini<=fim)
    {
        int meio = (ini+fim)/2;
        switch (comparaZ(nome, InstrVar[meio] + 5))
        {
        case 2:
        case 1:
            ini = meio+1;
            break;
        case 0:
            return meio;
        case -2:
            ind = meio;
        default:
            fim = meio-1;
        }
    }
    return ind;
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
