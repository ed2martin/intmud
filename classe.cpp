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

//#define MOSTRA_HERANCA // Mostra classes herdadas
//#define MOSTRA_MEM // Mostrar como alocou variáveis na memória
//#define MOSTRA_MUDOU // Mostrar as variáveis que mudaram, em AcertaVar()

//----------------------------------------------------------------------------
class TClasseAloca : public TVariavel /// Usado em TClasse::AcertaVar
{
public:
    int indvar;     ///< Valor de IndiceVar
    int tam;        ///< Tamanho do vetor em variáveis
    int comando;    ///< O que fazer
                    /**<
                     - 0=nada
                     - 1=apagar (antiga)
                     - 2=criar (nova)
                     - De 3 em diante, próximo objeto = nova variável
                     - 3=mover; variável não mudou
                     - De 4 em diante, tam=menor tam entre este e o próximo
                     - 4=copiar como int
                     - 5=copiar como double
                     - 6=copiar como texto
                     - 7=copiar como referência */
};

//----------------------------------------------------------------------------
class TClasseVar /// Usado em TClasse::AcertaVar, para acertar as variáveis
{
public:
    TClasseVar();             ///< Construtor
    ~TClasseVar();            ///< Destrutor
    char ** InstrVar;         ///< Valor anterior de TClasse::InstrVar
    unsigned int * IndiceVar; ///< Valor anterior de TClasse::IndiceVar
    unsigned int NumVar;      ///< Valor anterior de TClasse::NumVar
    char * Vars;              ///< Valor anterior de TClasse::Vars
    unsigned int TamVars;     ///< Valor anterior de TClasse::TamVars
    TClasseAloca * al;        ///< Para desalocar memória de TClasseAloca
};

//----------------------------------------------------------------------------
TClasse * TClasse::ClInic=0;

//----------------------------------------------------------------------------
TClasseVar::TClasseVar()
{
    InstrVar=0;
    IndiceVar=0;
    NumVar=0;
    Vars=0;
    TamVars=0;
    al=0;
}

//----------------------------------------------------------------------------
TClasseVar::~TClasseVar()
{
    if (InstrVar) delete[] InstrVar;
    if (IndiceVar) delete[] IndiceVar;
    if (Vars) delete[] Vars;
    if (al) delete[] al;
}

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
    ObjetoIni=0;
    ObjetoFim=0;
    NumObj=0;
    TamObj=0;
    TamVars=0;
    Vars=0;
    RBinsert();
}

//----------------------------------------------------------------------------
TClasse::~TClasse()
{
    if (ClInic==this)
        ClInic=RBnext(this);
    while (ObjetoIni)
        ObjetoIni->Apagar();
    if (Vars)
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
        delete[] Vars;
        Vars=0;
    }
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
                {
                    if (nivelse)
                        nivelse--;
                    else
                        break;
                }
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
                {
                    if (nivelse)
                        nivelse--;
                    else
                    {
                        x[3] = (x-p);
                        x[4] = (x-p) >> 8;
                        x+=Num16(x);
                        break;
                    }
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
                {
                    if (nivelse)
                        nivelse--;
                    else
                    {
                        x+=Num16(x);
                        break;
                    }
                }
            p[3] = (x-p);
            p[4] = (x-p)>>8;
            break;
        }
}

//----------------------------------------------------------------------------
// Obtém classes herdadas de uma determinada classe
int TClasse::Heranca(TClasse ** buffer, int tambuf)
{
    TClasse ** bufler = buffer; // Próxima classe que será verificada
    TClasse ** bufescr = buffer; // Próxima classe que será anotada
    TClasse * atual = this; // Classe que está sendo verificada

    while (tambuf>0)
    {
    // Se encontrou herança na classe...
        if ((atual->Comandos[0] || atual->Comandos[1]) &&
             atual->Comandos[2]==Instr::cHerda)
        {
            const char * p = atual->Comandos+4;
            int x = (unsigned char)atual->Comandos[3];
            for (; x; x--)
            {
            // Obtém a classe herdada
                TClasse * c = Procura(p);
                while (*p++);
                assert(c!=0);
            // A classe "this" não entra na lista
                if (c==this)
                    continue;
            // Verifica se já anotou a classe
                TClasse ** cpont = buffer;
                for (; cpont<bufescr; cpont++)
                    if (*cpont == c)
                        break;
                if (cpont<bufescr)
                    continue;
            // Anota a classe
                *bufescr++ = c;
                tambuf--;
                if (tambuf<=0)
                    break;
            }
        }
    // Lê a próxima classe
        if (bufler >= bufescr)
            break;
        atual = *bufler++;
    }
    return bufescr - buffer;
}

//----------------------------------------------------------------------------
// Acerta ListaDeriv e NumDeriv de todas as classes
void TClasse::AcertaDeriv()
{
    TClasse * buf[HERDA_TAM];
    TClasse * cl;

// Limpa ListaDeriv e NumDeriv
    for (cl = TClasse::RBfirst(); cl; cl = TClasse::RBnext(cl))
    {
        cl->AcertaComandos();
        cl->NumDeriv = 0;
        if (cl->ListaDeriv==0)
            continue;
        delete[] cl->ListaDeriv;
        cl->ListaDeriv = 0;
    }

// Mostra herança
#ifdef MOSTRA_HERANCA
    for (cl = TClasse::RBfirst(); cl; cl = TClasse::RBnext(cl))
    {
        int total = cl->Heranca(buf, HERDA_TAM);
        printf("herda(%s) ", cl->Nome);
        for (int x=0; x<total; x++)
            printf("%c %s", x==0 ? '=' : ',', buf[x]->Nome);
        putchar('\n'); fflush(stdout);
    }
#endif

// Obtém número de classes derivadas
    for (cl = TClasse::RBfirst(); cl; cl = TClasse::RBnext(cl))
        for (int total = cl->Heranca(buf, HERDA_TAM); total>0; )
            buf[--total]->NumDeriv++;

// Aloca memória em TClasse::ListaDeriv
    for (TClasse * cl = TClasse::RBfirst(); cl; cl = TClasse::RBnext(cl))
        if (cl->NumDeriv)
        {
            cl->ListaDeriv = new TClasse* [cl->NumDeriv];
            cl->NumDeriv = 0;
        }

// Acerta ListaDeriv e NumDeriv
    for (TClasse * cl = TClasse::RBfirst(); cl; cl = TClasse::RBnext(cl))
        for (int total = cl->Heranca(buf, HERDA_TAM); total>0; )
        {
            total--;
            buf[total]->ListaDeriv[ buf[total]->NumDeriv++ ] = cl;
        }
}

//----------------------------------------------------------------------------
// Acerta ListaDeriv e NumDeriv quando Comandos[] de uma classe mudou
void TClasse::AcertaDeriv(char * comandos_antes)
{
    TClasse * buf1[HERDA_TAM];
    TClasse * buf2[HERDA_TAM];
    int total1, total2;

    AcertaComandos();

// Obtém herança antes e depois da mudança
    char * p = Comandos;
    Comandos = comandos_antes;
    total1 = Heranca(buf1, HERDA_TAM);
    Comandos = p;
    total2 = Heranca(buf2, HERDA_TAM);

// Verifica se a herança não mudou
    while (total1>0 && total2>0)
        if (buf1[total1-1]==buf2[total2-1])
            total1--, total2--;
    if (total1==0 && total2==0)
        return;

// Retira herança
    for (int x1=0; x1<total1; x1++)
        for (int x2=0;; x2++)
        {
            if (x2 >= total2) // Retira herança
            {
                int &total = buf1[x1]->NumDeriv;
                TClasse ** lista = buf1[x1]->ListaDeriv;
                TClasse ** lfim = lista + total;
                for (;; lista++)
                {
                    assert(lista < lfim);
                    if (*lista == this)
                        break;
                }
                lista[0] = lfim[-1];
                if (--total==0)
                {
                    delete[] buf1[x1]->ListaDeriv;
                    buf1[x1]->ListaDeriv = 0;
                    break;
                }
                lista = new TClasse*[total];
                memcpy(lista, buf1[x1]->ListaDeriv, total*sizeof(TClasse*));
                delete[] buf1[x1]->ListaDeriv;
                buf1[x1]->ListaDeriv = lista;
                break;
            }
            if (buf1[x1] == buf2[x2]) // Se herança não mudou
                break;
        }

// Coloca herança
    for (int x2=0; x2<total2; x2++)
        for (int x1=0;; x1++)
        {
            if (x1 >= total1) // Coloca herança
            {
                int total = buf2[x2]->NumDeriv;
                buf2[x2]->NumDeriv++;
                TClasse ** lista = new TClasse*[total+1];
                if (total>0)
                    memcpy(lista, buf2[x2]->ListaDeriv, total*sizeof(TClasse*));
                if (buf2[x2]->ListaDeriv)
                    delete[] buf2[x2]->ListaDeriv;
                lista[total] = buf2[x2];
                buf2[x2]->ListaDeriv = lista;
                break;
            }
            if (buf1[x1] == buf2[x2]) // Se herança não mudou
                break;
        }
}

//----------------------------------------------------------------------------
void TClasse::AcertaVarSub()
{
// Obtém faixa de endereço das variáveis
    const char * ini=0;
    const char * fim=0;
    if (NumVar)
    {
        ini = fim = InstrVar[0];
        for (unsigned int x=1; x<NumVar; x++)
        {
            if (ini > InstrVar[x])
                ini = InstrVar[x];
            if (fim < InstrVar[x])
                fim = InstrVar[x];
        }
    }
// Acerta variáveis
    int mudou = AcertaVar();
// Lista de variáveis mudou: acerta classes derivadas
    if (mudou)
    {
        for (int x=0; x<NumDeriv; x++)
            ListaDeriv[x]->AcertaVar();
        return;
    }
// Lista não mudou: apenas acerta os endereços das variáveis
    for (int x=0; x<NumDeriv; x++)
    {
        TClasse * cl = ListaDeriv[x];
        for (unsigned int y=0; y<cl->NumVar; y++)
            if (cl->InstrVar[y] >= ini && cl->InstrVar[y] <= fim)
            {
                int ind = IndiceNome(cl->InstrVar[y] + Instr::endNome);
                assert(ind>=0);
                cl->InstrVar[y] = InstrVar[ind];
                //puts(InstrVar[ind] + Instr::endNome); fflush(stdout);
            }
    }
}

//----------------------------------------------------------------------------
int TClasse::AcertaVar()
{
    TClasseVar antes;
    antes.InstrVar = InstrVar;
    antes.IndiceVar = IndiceVar;
    antes.NumVar = NumVar;
    antes.Vars = Vars;
    antes.TamVars = TamVars;
    InstrVar = 0;
    IndiceVar = 0;
    NumVar = 0;
    Vars = 0;
    TamVars = 0;

// Obtém classes herdadas
    TClasse * herda[HERDA_TAM+1];
    int numherda = Heranca(herda+1, HERDA_TAM) + 1;
    herda[0] = this;

// Obtém número de variáveis/funções
    unsigned int total = 0;
    const char * ComandosFim = Comandos;
    for (int cont=0; cont<numherda; cont++)
    {
        bool inifunc = false;
        char * p = herda[cont]->Comandos;
        while (p[0] || p[1])
        {
            switch (p[2])
            {
            case Instr::cConstNulo:
            case Instr::cConstTxt:
            case Instr::cConstNum:
            case Instr::cConstExpr:
                total++, p+=Num16(p);
                break;
            case Instr::cFunc:
            case Instr::cVarFunc:
                total++, p+=Num16(p), inifunc=true;
                break;
            case Instr::cEnquanto:
            case Instr::cSe:
            case Instr::cSenao1:
            case Instr::cSenao2:
                assert(p[3] || p[4]);
                p += Num16(p+3);
                break;
            case Instr::cFimSe:
            default:
                if (inifunc==false && p[2] > Instr::cVariaveis)
                    total++;
                p+=Num16(p);
            }
        }
        if (cont==0)
            ComandosFim = p;
    }

// Aloca memória
    char ** var1=0;
    if (total) var1=new char*[total];
    total = 0;

// Adiciona funções/variáveis
    for (int cont=0; cont<numherda; cont++)
    {
        bool inifunc = false;
        char * p = herda[cont]->Comandos;
        while (p[0] || p[1])
        {
            switch (p[2])
            {
            case Instr::cConstNulo:
            case Instr::cConstTxt:
            case Instr::cConstNum:
            case Instr::cConstExpr:
                var1[total++] = p;
                p+=Num16(p);
                break;
            case Instr::cFunc:
            case Instr::cVarFunc:
                var1[total++] = p;
                p+=Num16(p), inifunc=true;
                break;
            case Instr::cEnquanto:
            case Instr::cSe:
            case Instr::cSenao1:
            case Instr::cSenao2:
                p += Num16(p+3);
                break;
            case Instr::cFimSe:
            default:
                if (inifunc==false && p[2] > Instr::cVariaveis)
                    var1[total++] = p;
                p+=Num16(p);
            }
        }
    }

// Organiza em ordem alfabética (aloca/libera memória - var2)
    if (total)
    {
        char ** var2 = new char*[total];
        for (unsigned int a=1; a<total; a+=a)
        {
            char ** pont = var2;
            var2 = var1;
            var1 = pont;
            int lido=0;
            for (unsigned int b=0; b<total; )
            {
                unsigned int b1=b, b2=b+a;
                b = b2;
                while (b1<b && b2<b+a && b2<total)
                {
                    if (comparaZ(var2[b1] + Instr::endNome,
                            var2[b2] + Instr::endNome) > 0)
                        var1[lido++] = var2[b2++];
                    else
                        var1[lido++] = var2[b1++];
                }
                while (b1<b && b1<total)
                    var1[lido++] = var2[b1++];
                b += a;
                while (b2<b && b2<total)
                    var1[lido++] = var2[b2++];
            }
        }
        delete[] var2;
    }

// Obtém número de variáveis, detectando as repetidas
    NumVar = total;
    for (int x=total-2; x>=0; x--)
        if (comparaZ(var1[x]+Instr::endNome, var1[x+1]+Instr::endNome)==0)
        {
            var1[x+1]=0;
            NumVar--;
        }

// Acerta TClasse::InstrVar (nomes das variáveis)
// Acerta memória alocada em var1
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
    int indclasse=0, bitclasse=1;
    int indobjeto=0, bitobjeto=1;
    IndiceVar = new unsigned int[NumVar];
    for (unsigned int x=0; x<NumVar; x++)
    {
        unsigned int valor = 0;
    // Verifica se está na própria classe
        if (InstrVar[x]>=Comandos && InstrVar[x]<=ComandosFim)
            valor |= 0x800000;
    // Verifica se é variável "comum"
        if (InstrVar[x][Instr::endProp] & 1)
            valor |= 0x400000;
    // Verifica cBit1
        if (InstrVar[x][2] == Instr::cInt1)
        {
            int total = (unsigned char)InstrVar[x][Instr::endVetor];
            if (total==0)
                total++;
            if (valor & 0x400000) // Classe
            {
                valor += indclasse + (bitclasse << 24);
                indclasse += total/8;
                for (; total&7; total--)
                    if (bitclasse==0x80)
                        bitclasse=1, indclasse++;
                    else
                        bitclasse <<= 1;
            }
            else // Objeto
            {
                valor += indobjeto + (bitobjeto << 24);
                indobjeto += total/8;
                for (; total&7; total--)
                    if (bitobjeto==0x80)
                        bitobjeto=1, indobjeto++;
                    else
                        bitobjeto <<= 1;
            }
        }
    // Anota o resultado
        IndiceVar[x] = valor;
    }
    if (bitclasse!=1) indclasse++;
    if (bitobjeto!=1) indobjeto++;

// Acerta variáveis com alinhamento de 1 byte
    for (unsigned int x=0; x<NumVar; x++)
    {
        if (InstrVar[x][2] == Instr::cInt1)
            continue;
        int tamanho = TVariavel::Tamanho(InstrVar[x]);
        if (tamanho&1)
        {
            int total = (unsigned char)InstrVar[x][Instr::endVetor];
            if (total)
                tamanho *= total;
            if (IndiceVar[x] & 0x400000) // Classe
                IndiceVar[x] += indclasse, indclasse += tamanho;
            else // Objeto
                IndiceVar[x] += indobjeto, indobjeto += tamanho;
        }
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
        {
            int total = (unsigned char)InstrVar[x][Instr::endVetor];
            if (total)
                tamanho *= total;
            if (IndiceVar[x] & 0x400000) // Classe
                IndiceVar[x] += indclasse, indclasse += tamanho;
            else // Objeto
                IndiceVar[x] += indobjeto, indobjeto += tamanho;
        }
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
        {
            int total = (unsigned char)InstrVar[x][Instr::endVetor];
            if (total)
                tamanho *= total;
            if (IndiceVar[x] & 0x400000) // Classe
                IndiceVar[x] += indclasse, indclasse += tamanho;
            else // Objeto
                IndiceVar[x] += indobjeto, indobjeto += tamanho;
        }
    }

// Acerta variáveis
    Vars = 0;
    TamVars = indclasse;
    TamObj = indobjeto;

// Mostra o resultado
#ifdef MOSTRA_MEM
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

// Nenhuma variável: nada para ser feito
    if (NumVar==0 && antes.NumVar==0)
        return 0;

// Prepara lista de variáveis que mudaram
    antes.al =  new TClasseAloca[NumVar + antes.NumVar];
    TClasseAloca * al = antes.al;
    indobjeto=0; // Número de variáveis de objetos
    indclasse=0; // Número de variáveis de objetos + classe
    bitobjeto=0; // Bit 1 =1 se alguma variável da classe mudou
                 // Bit 2 =1 se alguma variável de objeto mudou
                 // !=0 se a lista de variáveis mudou
    total = 0;   // Para obter o número de variáveis

        // passo=0 -> obtém variáveis de objetos
        // passo=1 -> obtém variáveis da classe (definida com "comum")
    for (int passo=0; passo<2; passo++)
    {
        unsigned int c1=0,c2=0;
        TVariavel v;
        v.Limpar();
    // Checa variáveis que mudaram
        while (c1<antes.NumVar && c2<NumVar)
        {
        // Pula variáveis que não correspondem ao passo
            if ((antes.InstrVar[c1][Instr::endProp] & 1) != passo)
                { c1++; continue; }
            if ((InstrVar[c2][Instr::endProp] & 1) != passo)
                { c2++; continue; }
        // Compara variáveis antes e depois
            int cmp = comparaZ(antes.InstrVar[c1] + Instr::endNome,
                               InstrVar[c2] + Instr::endNome);
            if (cmp!=0)
                bitobjeto |= 0x10;
        // Pula variáveis com tamanho zero
            bool tamzero=false;
            if (TVariavel::Tamanho(antes.InstrVar[c1])==0)
                c1++, tamzero=true;
            if (TVariavel::Tamanho(InstrVar[c2])==0)
                c2++, tamzero=true;
            if (tamzero)
                continue;
        // Variável apagada
            if (cmp<0)
            {
                if (antes.InstrVar[c1][2] > Instr::cVarFunc)
                {
                    al[total].comando = 1;
                    al[total].defvar = antes.InstrVar[c1];
                    al[total].indvar = antes.IndiceVar[c1];
                    total++;
                }
                c1++, bitobjeto |= 1;
                continue;
            }
        // Variável criada
            if (cmp>0)
            {
                if (InstrVar[c2][2] > Instr::cVarFunc)
                {
                    al[total].comando = 2;
                    al[total].defvar = InstrVar[c2];
                    al[total].indvar = IndiceVar[c2];
                    total++;
                }
                c2++, bitobjeto |= 1;
                continue;
            }
        // Tipo de variável não mudou: mover
            if (antes.InstrVar[c1][2] == InstrVar[c2][2] &&
                antes.InstrVar[c1][Instr::endIndice] ==
                      InstrVar[c2][Instr::endIndice])
            {
                unsigned char tam1 = antes.InstrVar[c1][Instr::endVetor];
                unsigned char tam2 = InstrVar[c2][Instr::endVetor];
                if (tam1==0) tam1++;
                if (tam2==0) tam2++;
                if (tam1!=tam2)
                    bitobjeto |= 1;
                al[total].comando = 3;
                if (InstrVar[c2][2]==Instr::cInt1) // Bits: devem ser copiados
                    al[total].comando = 4;
                al[total].defvar = antes.InstrVar[c1];
                al[total].indvar = antes.IndiceVar[c1];
                total++;
                al[total].comando = 0;
                al[total].defvar = InstrVar[c2];
                al[total].indvar = IndiceVar[c2];
                total++, c1++, c2++;
                continue;
            }
        // Tipo mudou: verifica como deve ser feita a cópia
            v.defvar = InstrVar[c2];
            switch (v.Tipo())
            {
            case varInt:    al[total].comando = 4; break;
            case varDouble: al[total].comando = 5; break;
            case varTxt:    al[total].comando = 6; break;
            case varObj:    al[total].comando = 7; break;
            default:        al[total].comando = 0;
            }
            if (al[total].comando)
            {
                al[total].defvar = antes.InstrVar[c1];
                al[total].indvar = antes.IndiceVar[c1];
                total++;
                al[total].comando = 0;
                al[total].defvar = InstrVar[c2];
                al[total].indvar = IndiceVar[c2];
                total++;
            }
            else
            {
                if (antes.InstrVar[c1][2] > Instr::cVarFunc)
                {
                    al[total].comando = 1;
                    al[total].defvar = antes.InstrVar[c1];
                    al[total].indvar = antes.IndiceVar[c1];
                    total++;
                }
                if (InstrVar[c2][2] > Instr::cVarFunc)
                {
                    al[total].comando = 2;
                    al[total].defvar = InstrVar[c2];
                    al[total].indvar = IndiceVar[c2];
                    total++;
                }
            }
            c1++, c2++, bitobjeto |= 1;
        }
    // Variáveis que serão apagadas
        for (; c1<antes.NumVar; c1++)
            if ((antes.InstrVar[c1][Instr::endProp] & 1) == passo)
            {
                bitobjeto |= 0x10;
                if (TVariavel::Tamanho(antes.InstrVar[c1])==0)
                    continue;
                bitobjeto |= 1;
                if (antes.InstrVar[c1][2] <= Instr::cVarFunc)
                    continue;
                al[total].comando = 1;
                al[total].defvar = antes.InstrVar[c1];
                al[total].indvar = antes.IndiceVar[c1];
                total++;
            }
    // Variáveis que serão criadas
        for (; c2<NumVar; c2++)
            if ((InstrVar[c2][Instr::endProp] & 1) == passo)
            {
                bitobjeto |= 0x10;
                if (TVariavel::Tamanho(InstrVar[c2])==0)
                    continue;
                bitobjeto |= 1;
                if (InstrVar[c2][2] <= Instr::cVarFunc)
                    continue;
                al[total].comando = 2;
                al[total].defvar = InstrVar[c2];
                al[total].indvar = IndiceVar[c2];
                total++;
            }
    // Acerta indobjeto e indclasse
        indobjeto = indclasse;
        indclasse = total;
        bitobjeto <<= 1;
    }

// Acerta al[n].tam e al[n].bit
    for (unsigned int x=0; x<total; x++)
    {
        al[x].tam = (unsigned char)al[x].defvar[Instr::endVetor];
        if (al[x].tam==0)
            al[x].tam=1;
        al[x].bit = al[x].indvar >> 24;
    }

// Acerta al[n].tam para comandos 4 em diante
    for (unsigned int x=0; x<total; x++)
        if (al[x].comando >= 4)
        {
            if (al[x].tam > al[x+1].tam)
                al[x].tam = al[x+1].tam;
            if (al[x].tam > 0)
                al[x].tam--;
        }

// Mostra a lista de alterações
#ifdef MOSTRA_MUDOU
    printf("Classe=\"%s\"  classe_mudou=%s  objeto_mudou=%s  "
           "variáveis_mudaram=%s\n",
           Nome, bitobjeto&2 ? "sim" : "não",
                 bitobjeto&4 ? "sim" : "não",
                 bitobjeto   ? "sim" : "não");
    for (int x=0; x<indclasse; x++)
    {
        int param = 0;
        char mens[2048];
        printf("  %s  ", x<indobjeto ? "objeto" : "classe");
        switch (al[x].comando)
        {
        case 1: printf("apagar("); param=1; break;
        case 2: printf("criar("); param=1; break;
        case 3: printf("mover("); param=2; break;
        case 4: printf("copiar_int("); param=2; break;
        case 5: printf("copiar_double("); param=2; break;
        case 6: printf("copiar_txt("); param=2; break;
        case 7: printf("copiar_ref("); param=2; break;
        default: printf("??? (");
        }
        if (param>=1)
            if (Instr::Decod(mens, al[x].defvar, sizeof(mens)))
                printf("%s", mens);
        if (param>=2)
        {
            if (Instr::Decod(mens, al[x+1].defvar, sizeof(mens)))
                printf(")  (%s", mens);
            x++;
        }
        printf(")\n");
    }
    putchar('\n'); fflush(stdout);
#endif

// Acerta variáveis da classe
    if (bitobjeto & 2) // Alguma variável mudou
    {
        if (TamVars)
            Vars = new char[TamVars];
        for (int x=indobjeto; x<indclasse; x++)
        {
            if (al[x].comando >= 3)
            {
                al[x].endvar = antes.Vars + (al[x].indvar & 0x3FFFFF);
                al[x+1].endvar = Vars + (al[x+1].indvar & 0x3FFFFF);
            }
            switch (al[x].comando)
            {
            case 1: // Apagar antiga
                al[x].endvar = antes.Vars + (al[x].indvar & 0x3FFFFF);
                al[x].Apagar();
                break;
            case 2: // Criar nova
                al[x].endvar = Vars + (al[x].indvar & 0x3FFFFF);
                al[x].Criar(this, 0);
                break;
            case 3: // Mover; variável não mudou
                if (al[x].tam == al[x+1].tam)
                    al[x].Mover(al[x+1].endvar, this, 0);
                else if (al[x].tam > al[x+1].tam)
                {
                    const char * def = al[x].defvar;
                    al[x].Redim(0, 0, al[x].tam, al[x+1].tam);
                    al[x].defvar = al[x+1].defvar;
                    al[x].Mover(al[x+1].endvar, this, 0);
                    al[x].defvar = def;
                }
                else
                {
                    al[x].Mover(al[x+1].endvar, this, 0);
                    al[x+1].Redim(this, 0, al[x].tam, al[x+1].tam);
                }
                x++;
                break;
            case 4:
                if (al[x+1].defvar[2] > Instr::cVarFunc)
                    al[x+1].Criar(this, 0);
                al[x].indice = al[x+1].indice = al[x].tam;
                for (; al[x].indice; al[x].indice--,al[x+1].indice--)
                    al[x+1].setInt( al[x].getInt() );
                al[x+1].setInt( al[x].getInt() );
                if (al[x].defvar[2] > Instr::cVarFunc)
                    al[x].Apagar();
                x++;
                break;
            case 5:
                if (al[x+1].defvar[2] > Instr::cVarFunc)
                    al[x+1].Criar(this, 0);
                al[x].indice = al[x+1].indice = al[x].tam;
                for (; al[x].indice; al[x].indice--,al[x+1].indice--)
                    al[x+1].setDouble( al[x].getDouble() );
                al[x+1].setDouble( al[x].getDouble() );
                if (al[x].defvar[2] > Instr::cVarFunc)
                    al[x].Apagar();
                x++;
                break;
            case 6:
                if (al[x+1].defvar[2] > Instr::cVarFunc)
                    al[x+1].Criar(this, 0);
                al[x].indice = al[x+1].indice = al[x].tam;
                for (; al[x].indice; al[x].indice--,al[x+1].indice--)
                    al[x+1].setTxt( al[x].getTxt() );
                al[x+1].setTxt( al[x].getTxt() );
                if (al[x].defvar[2] > Instr::cVarFunc)
                    al[x].Apagar();
                x++;
                break;
            case 7:
                if (al[x+1].defvar[2] > Instr::cVarFunc)
                    al[x+1].Criar(this, 0);
                al[x].indice = al[x+1].indice = al[x].tam;
                for (; al[x].indice; al[x].indice--,al[x+1].indice--)
                    al[x+1].setObj( al[x].getObj() );
                al[x+1].setObj( al[x].getObj() );
                if (al[x].defvar[2] > Instr::cVarFunc)
                    al[x].Apagar();
                x++;
                break;
            } // switch
        } // for
    } // if

// Acerta variáveis dos objetos
    if (bitobjeto & 4) // Alguma variável mudou
    {
        for (int nobj=NumObj; nobj>0; nobj--)
        {
        // Cria objeto no final da lista ligada
            TObjeto * obj = TObjeto::Criar(this, false);
        // Copia variáveis do primeiro objeto para o objeto criado
            for (int x=0; x<indobjeto; x++)
            {
                if (al[x].comando >= 3)
                {
                    al[x].endvar = ObjetoIni->Vars + (al[x].indvar & 0x3FFFFF);
                    al[x+1].endvar = obj->Vars + (al[x+1].indvar & 0x3FFFFF);
                }
                switch (al[x].comando)
                {
                case 1: // Apagar antiga
                    al[x].endvar = ObjetoIni->Vars + (al[x].indvar & 0x3FFFFF);
                    al[x].Apagar();
                    break;
                case 2: // Criar nova
                    al[x].endvar = obj->Vars + (al[x].indvar & 0x3FFFFF);
                    al[x].Criar(this, obj);
                    break;
                case 3: // Mover; variável não mudou
                    if (al[x].tam == al[x+1].tam)
                        al[x].Mover(al[x+1].endvar, this, obj);
                    else if (al[x].tam > al[x+1].tam)
                    {
                        const char * def = al[x].defvar;
                        al[x].Redim(0, 0, al[x].tam, al[x+1].tam);
                        al[x].defvar = al[x+1].defvar;
                        al[x].Mover(al[x+1].endvar, this, obj);
                        al[x].defvar = def;
                    }
                    else
                    {
                        al[x].Mover(al[x+1].endvar, this, obj);
                        al[x+1].Redim(this, obj, al[x].tam, al[x+1].tam);
                    }
                    x++;
                    break;
                case 4:
                    if (al[x+1].defvar[2] > Instr::cVarFunc)
                        al[x+1].Criar(this, obj);
                    al[x].indice = al[x+1].indice = al[x].tam;
                    for (; al[x].indice; al[x].indice--,al[x+1].indice--)
                        al[x+1].setInt( al[x].getInt() );
                    al[x+1].setInt( al[x].getInt() );
                    if (al[x].defvar[2] > Instr::cVarFunc)
                        al[x].Apagar();
                    x++;
                    break;
                case 5:
                    if (al[x+1].defvar[2] > Instr::cVarFunc)
                        al[x+1].Criar(this, obj);
                    al[x].indice = al[x+1].indice = al[x].tam;
                    for (; al[x].indice; al[x].indice--,al[x+1].indice--)
                        al[x+1].setDouble( al[x].getDouble() );
                    al[x+1].setDouble( al[x].getDouble() );
                    if (al[x].defvar[2] > Instr::cVarFunc)
                        al[x].Apagar();
                    x++;
                    break;
                case 6:
                    if (al[x+1].defvar[2] > Instr::cVarFunc)
                        al[x+1].Criar(this, obj);
                    al[x].indice = al[x+1].indice = al[x].tam;
                    for (; al[x].indice; al[x].indice--,al[x+1].indice--)
                        al[x+1].setTxt( al[x].getTxt() );
                    al[x+1].setTxt( al[x].getTxt() );
                    if (al[x].defvar[2] > Instr::cVarFunc)
                        al[x].Apagar();
                    x++;
                    break;
                case 7:
                    if (al[x+1].defvar[2] > Instr::cVarFunc)
                        al[x+1].Criar(this, obj);
                    al[x].indice = al[x+1].indice = al[x].tam;
                    for (; al[x].indice; al[x].indice--,al[x+1].indice--)
                        al[x+1].setObj( al[x].getObj() );
                    al[x+1].setObj( al[x].getObj() );
                    if (al[x].defvar[2] > Instr::cVarFunc)
                        al[x].Apagar();
                    x++;
                    break;
                } // switch
            } // for
        // Apaga objeto mudando as referências do objeto
            ObjetoIni->Apagar(obj);
        }
    }
    return (bitobjeto==0 ? 0 : (bitobjeto&15)==0 ? 1 : 2);
}

//----------------------------------------------------------------------------
TClasse * TClasse::Procura(const char * nome)
{
    TClasse * y = RBroot;
    while (y)
    {
        int i = comparaZ(nome, y->Nome);
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
TClasse * TClasse::ProcuraIni(const char * nome)
{
    TClasse * x = 0;
    TClasse * y = RBroot;
    while (y)
    {
        int i = comparaZ(nome, y->Nome);
        if (i==0)
            return y;
        if (i==-2)
            x = y;
        if (i<0)
            y = y->RBleft;
        else
            y = y->RBright;
    }
    return x;
}

//----------------------------------------------------------------------------
TClasse * TClasse::ProcuraFim(const char * nome)
{
    TClasse * x = 0;
    TClasse * y = RBroot;
    while (y)
    {
        int i = comparaZ(nome, y->Nome);
        if (i==0 || i==-2)
            x = y;
        if (i==-1)
            y = y->RBleft;
        else
            y = y->RBright;
    }
    return x;
}

//----------------------------------------------------------------------------
int TClasse::IndiceNome(const char * nome)
{
    int ini = 0; // Índice inicial
    int fim = NumVar - 1; // Índice final
    while (ini<=fim)
    {
        int meio = (ini+fim)/2;
        switch (comparaZ(nome, InstrVar[meio] + Instr::endNome))
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
int TClasse::IndiceNomeIni(const char * nome)
{
    int ini = 0; // Índice inicial
    int fim = NumVar - 1; // Índice final
    int ind = -1;
    while (ini<=fim)
    {
        int meio = (ini+fim)/2;
        switch (comparaZ(nome, InstrVar[meio] + Instr::endNome))
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
int TClasse::IndiceNomeFim(const char * nome)
{
    int ini = 0; // Índice inicial
    int fim = NumVar - 1; // Índice final
    int ind = -1;
    while (ini<=fim)
    {
        int meio = (ini+fim)/2;
        switch (comparaZ(nome, InstrVar[meio] + Instr::endNome))
        {
        case 2:
        case 1:
            ini = meio+1;
            break;
        case -1:
            fim = meio-1;
            break;
        case -2:
        case 0:
            ind = meio;
            ini = meio+1;
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
