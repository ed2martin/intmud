/* Este arquivo � software livre; voc� pode redistribuir e/ou
 * modificar nos termos da licen�a LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "classe.h"
#include "objeto.h"
#include "variavel.h"
#include "instr.h"
#include "instr-enum.h"
#include "arqmapa.h"
#include "misc.h"

//#define DEBUG_CRIAR // Mostra classes criadas e apagadas
//#define MOSTRA_HERANCA // Mostra classes herdadas
//#define MOSTRA_MEM // Mostrar como alocou vari�veis na mem�ria
//#define MOSTRA_MUDOU // Mostrar as vari�veis que mudaram, em AcertaVar()

//----------------------------------------------------------------------------
class TClasseAloca : public TVariavel /// Usado em TClasse::AcertaVar
{
public:
    int indvar;     ///< Valor de IndiceVar
    int tam;        ///< Tamanho do vetor em vari�veis
    int comando;    ///< O que fazer
                    /**<
                     - 0=nada
                     - 1=apagar (antiga)
                     - 2=criar (nova)
                     - De 3 em diante, pr�ximo objeto = nova vari�vel
                     - 3=mover; vari�vel n�o mudou
                     - De 4 em diante, tam=menor tam entre este e o pr�ximo
                     - 4=copiar como int
                     - 5=copiar como double
                     - 6=copiar como texto
                     - 7=copiar como refer�ncia */
};

//----------------------------------------------------------------------------
class TClasseVar /// Usado em TClasse::AcertaVar, para acertar as vari�veis
{
public:
    TClasseVar();             ///< Construtor
    ~TClasseVar();            ///< Destrutor
    char ** InstrVar;         ///< Valor anterior de TClasse::InstrVar
    unsigned int * IndiceVar; ///< Valor anterior de TClasse::IndiceVar
    unsigned int NumVar;      ///< Valor anterior de TClasse::NumVar
    char * Vars;              ///< Valor anterior de TClasse::Vars
    unsigned int TamVars;     ///< Valor anterior de TClasse::TamVars
    TClasseAloca * al;        ///< Para desalocar mem�ria de TClasseAloca
};

//----------------------------------------------------------------------------
TClasse * TClasse::ClInic = nullptr;

//----------------------------------------------------------------------------
TClasseVar::TClasseVar()
{
    InstrVar = nullptr;
    IndiceVar = nullptr;
    NumVar = 0;
    Vars = nullptr;
    TamVars = 0;
    al = nullptr;
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
TClasse::TClasse(const char * nome, TArqMapa * arquivo)
{
    Comandos = nullptr;
    copiastr(Nome, nome, sizeof(Nome));
    ListaDeriv = nullptr;
    NumDeriv = 0;
    InstrVar = nullptr;
    IndiceVar = nullptr;
    NumVar = 0;
    ObjetoIni = nullptr;
    ObjetoFim = nullptr;
    NumObj = 0;
    TamObj = 0;
    TamVars = 0;
    Vars = nullptr;
    ArqArquivo = nullptr;
    RBinsert();
    ArqArquivo = arquivo;
    ArqAntes = arquivo->ClFim;
    ArqDepois = nullptr;
    arquivo->ClFim = this;
    (ArqAntes ? ArqAntes->ArqDepois : arquivo->ClInicio) = this;
    arquivo->Mudou = true;
#ifdef DEBUG_CRIAR
    printf("TClasse( %s , %s , %p )\n", Nome, ArqArquivo->Arquivo, this);
    fflush(stdout);
#endif
}

//----------------------------------------------------------------------------
TClasse::~TClasse()
{
#ifdef DEBUG_CRIAR
    printf("~TClasse( %s , %s , %p )\n", Nome,
           ArqArquivo ? ArqArquivo->Arquivo : "???", this); fflush(stdout);
#endif
// Apaga objetos
    while (ObjetoIni)
        ObjetoIni->Apagar();
// Limpa as instru��es
    LimpaInstr();
    if (ClInic==this)
        ClInic=RBnext(this);
// Apaga vari�veis
    if (Vars)
    {
    // Chama destrutores das vari�veis
        TVariavel v;
        for (int x= (int)NumVar - 1; x >= 0; x--)
            if (InstrVar[x][2] > Instr::cVarFunc &&
                    (IndiceVar[x] & 0x400000))
            {
                v.endvar = Vars + (IndiceVar[x] & 0x3FFFFF);
                v.defvar = InstrVar[x];
                v.Apagar();
            }
    // Libera mem�ria
        delete[] Vars;
        Vars = nullptr;
    }
// Retira classe de ListaDeriv das outras classes
    TClasse * buf[HERDA_TAM];
    int total = Heranca(buf, HERDA_TAM);
    for (int x = 0; x < total; x++)
        buf[x]->RetiraDeriv(this);
// Remove da RBT
    RBremove();
// Desaloca mem�ria
    if (Comandos)     delete[] Comandos;
    if (ListaDeriv)   delete[] ListaDeriv;
    if (InstrVar)     delete[] InstrVar;
    if (IndiceVar)    delete[] IndiceVar;
// Retira da lista ligada de arquivo
    if (ArqArquivo)
    {
        (ArqAntes ? ArqAntes->ArqDepois : ArqArquivo->ClInicio) = ArqDepois;
        (ArqDepois ? ArqDepois->ArqAntes : ArqArquivo->ClFim) = ArqAntes;
        ArqArquivo->Mudou = true;
    }
}

//----------------------------------------------------------------------------
bool TClasse::NomeValido(char * nome)
{
    char *o,*d;
// Verifica se tem algum caracter inv�lido
    for (o=nome; *o; o++)
        if (tabNOMES1[*(unsigned char*)o]==0)
            return false;
// Avan�a para in�cio do nome
    for (o = nome; *o == ' '; o++);
    if (*o >= '0' && *o <= '9')
        return false;
// Retira espa�os desnecess�rios
    d=nome;
    for (; *o; o++)
        if (*o != ' ' || (o[1] != ' ' && o[1]))
            *d++ = *o;
    *d = 0;
// Checa tamanho do nome
    if (d == nome || d - nome >= CLASSE_NOME_TAM)
        return false;
    return true;
}

//----------------------------------------------------------------------------
char * TClasse::NomeDef(char * texto)
{
    while (*texto == ' ') texto++;
    if (compara(texto, "classe ", 7) != 0)
        return 0;
    texto += 6;
    while (*texto == ' ') texto++;
    return (tabNOMES1[*(unsigned char*)texto] ? texto : 0);
}

//----------------------------------------------------------------------------
void TClasse::MudaNome(const char * novonome)
{
    char nomeantes[CLASSE_NOME_TAM]; // Nome anterior da classe

// Se os nomes s�o iguais na busca, atualiza apenas o nome
    if (comparaVar(Nome, novonome) == 0 && strlen(Nome) == strlen(novonome))
    {
        if (strcmp(Nome, novonome) == 0)
            return;
        ArqArquivo->Mudou = true;
        strcpy(Nome, novonome);
        return;
    }

// Acerta o nome da classe e guarda o nome anterior
    ArqArquivo->Mudou = true;
    RBremove();
    copiastr(nomeantes, Nome, sizeof(nomeantes));
    copiastr(Nome, novonome, sizeof(Nome));
    RBinsert();
    //printf("\nRenomeando classe %s para %s\n", nomeantes, Nome); fflush(stdout);

// Acerta classes que herdam essa
    for (unsigned int contderiv = 0; contderiv < NumDeriv; contderiv++)
    {
        char txtherda[HERDA_TAM * CLASSE_NOME_TAM + 10];
        TClasse * cl = ListaDeriv[contderiv];

    // Monta a instru��o herda com o novo nome da classe
        assert(cl->Comandos[2] == Instr::cHerda);
        memcpy(txtherda, cl->Comandos, Instr::endVar + 1);
        const char * o = cl->Comandos + Instr::endVar + 1;
        char * d = txtherda + Instr::endVar + 1;
        bool mudounome = false;
        for (int total = (unsigned char)txtherda[Instr::endVar]; total; total--)
        {
            if (strcmp(o, nomeantes) == 0)
            {
                strcpy(d, Nome);
                mudounome = true;
            }
            else
                strcpy(d, o);
            while (*o++);
            while (*d++);
        }
        unsigned int tamherda = d - txtherda;
        txtherda[0] = tamherda;
        txtherda[1] = tamherda >> 8;
        assert(tamherda <= sizeof(txtherda));
        /*{
          char mens[BUF_MENS];
          if (!mudounome)
            printf("Classe %s n�o herda %s diretamente\n", cl->Nome, Nome);
          else if (Instr::Decod(mens, txtherda, sizeof(mens)))
            printf("Classe %s: %s\n", cl->Nome, mens);
          fflush(stdout);
        }*/
        if (!mudounome) // Classe n�o � herdada diretamente
            continue;

    // Obt�m faixa de endere�os das instru��es da classe
        const char * ini = cl->Comandos + Num16(cl->Comandos);
        const char * fim = ini;
        while (fim[0] || fim[1])
            fim += Num16(fim);
        fim += 2;

    // Acerta instru��es da classe
        char * pnovo = new char[fim - ini + tamherda];
        memcpy(pnovo, txtherda, tamherda);
        memcpy(pnovo + tamherda, ini, fim - ini);
        delete[] cl->Comandos;
        cl->Comandos = pnovo;
        pnovo += tamherda;
        cl->ArqArquivo->Mudou = true;

    // Acerta endere�os das vari�veis na classe herdada
        long long enddif = pnovo - ini;
        assert(ini + enddif == pnovo);
        for (unsigned int y = 0; y < cl->NumVar; y++)
            if (cl->InstrVar[y] >= ini &&
                        cl->InstrVar[y] < fim)
                cl->InstrVar[y] += enddif;

    // Acerta endere�os das vari�veis nas outras classes
        for (unsigned int numd = 0; numd < cl->NumDeriv; numd++)
        {
            TClasse * deriv = cl->ListaDeriv[numd];
            //printf("  Ajustando classe %s\n", deriv->Nome); fflush(stdout);
            for (unsigned int y = 0; y < deriv->NumVar; y++)
                if (deriv->InstrVar[y] >= ini &&
                        deriv->InstrVar[y] < fim)
                    deriv->InstrVar[y] += enddif;
        }
    }
}

//----------------------------------------------------------------------------
#define REMOVE_ARQ \
    if (ArqArquivo) \
    { \
        (ArqAntes ? ArqAntes->ArqDepois : ArqArquivo->ClInicio) = ArqDepois; \
        (ArqDepois ? ArqDepois->ArqAntes : ArqArquivo->ClFim) = ArqAntes; \
        ArqArquivo->Mudou = true; \
    } \
    ArqArquivo = arquivo; \
    if (arquivo == nullptr) \
        return; \
    arquivo->Mudou = true;

//----------------------------------------------------------------------------
void TClasse::MoveArquivo(TArqMapa * arquivo)
{
    if (ArqArquivo == arquivo)
        return;
    REMOVE_ARQ
    ArqAntes = arquivo->ClFim;
    ArqDepois = nullptr;
    arquivo->ClFim = this;
    (ArqAntes ? ArqAntes->ArqDepois : arquivo->ClInicio) = this;
}

//----------------------------------------------------------------------------
void TClasse::MoveArqIni(TArqMapa * arquivo)
{
    if (arquivo->ClInicio == this)
        return;
    REMOVE_ARQ;
    ArqAntes = nullptr;
    ArqDepois = arquivo->ClInicio;
    arquivo->ClInicio = this;
    (ArqDepois ? ArqDepois->ArqAntes : arquivo->ClFim) = this;
}

//----------------------------------------------------------------------------
void TClasse::MoveArqFim(TArqMapa * arquivo)
{
    if (arquivo->ClInicio == this)
        return;
    REMOVE_ARQ;
    ArqAntes = arquivo->ClFim;
    ArqDepois = nullptr;
    arquivo->ClFim = this;
    (ArqAntes ? ArqAntes->ArqDepois : arquivo->ClInicio) = this;
}

//----------------------------------------------------------------------------
void TClasse::MoveArqAntes(TClasse * cl)
{
    if (cl == nullptr || cl == this || cl->ArqAntes == this)
        return;
    TArqMapa * arquivo = cl->ArqArquivo;
    REMOVE_ARQ;
    ArqAntes = cl->ArqAntes;
    ArqDepois = cl;
    cl->ArqAntes = this;
    (ArqAntes ? ArqAntes->ArqDepois : arquivo->ClInicio) = this;
}

//----------------------------------------------------------------------------
void TClasse::MoveArqDepois(TClasse * cl)
{
    if (cl == nullptr || cl == this || cl->ArqDepois == this)
        return;
    TArqMapa * arquivo = cl->ArqArquivo;
    REMOVE_ARQ;
    ArqDepois = cl->ArqDepois;
    ArqAntes = cl;
    cl->ArqDepois = this;
    (ArqDepois ? ArqDepois->ArqAntes : arquivo->ClFim) = this;
}

//----------------------------------------------------------------------------
bool TClasse::AcertaComandosFim(int valor)
{
    switch (valor)
    {
    case Instr::cConstNulo:
    case Instr::cConstTxt:
    case Instr::cConstNum:
    case Instr::cConstExpr:
    case Instr::cConstVar:
    case Instr::cFunc:
    case Instr::cVarFunc:
        return 1;
    }
    return 0;
}

//----------------------------------------------------------------------------
void TClasse::AcertaComandos(char * comandos)
{
    if (comandos == nullptr)
        return;

    char * x;
    int nivelse;
    char * casopadrao;
    char * caso1[1024];
    char * caso2[1024];
    unsigned int casonum; // Quantidade de vari�veis usadas em caso1

// Primeiro zera endere�os de desvio e acerta alinhamento
    int indent = 0;
    for (char * p = comandos; p[0] || p[1]; p += Num16(p))
    {
        switch (p[2]) // Alinhamento
        {
        case Instr::cConstNulo:
        case Instr::cConstTxt:
        case Instr::cConstNum:
        case Instr::cConstExpr:
        case Instr::cConstVar:
            p[Instr::endAlin] = 0;
            indent = 0;
            break;
        case Instr::cFunc:
        case Instr::cVarFunc:
            p[Instr::endAlin] = 0;
            indent = 1;
            break;
        case Instr::cFimSe:
        case Instr::cEFim:
        case Instr::cCasoFim:
            if (indent > 1)
                indent--;
            p[Instr::endAlin] = indent;
            break;
        case Instr::cSe:
        case Instr::cEnquanto:
        case Instr::cEPara:
        case Instr::cCasoVar:
            p[Instr::endAlin] = indent;
            if (indent < 0xFF)
                indent++;
            break;
        case Instr::cSenao1:
        case Instr::cSenao2:
        case Instr::cCasoSe:
        case Instr::cCasoSePadrao:
            p[Instr::endAlin] = (indent < 2 ? 1 : indent - 1);
            break;
        default:
            p[Instr::endAlin] = indent;
        }
        switch (p[2]) // Endere�os de desvio
        {
        case Instr::cSe:
        case Instr::cSenao1:
        case Instr::cSenao2:
        case Instr::cEnquanto:
        case Instr::cEPara:
        case Instr::cEFim:
        case Instr::cCasoVar:
        case Instr::cSair1:
        case Instr::cSair2:
        case Instr::cContinuar1:
        case Instr::cContinuar2:
            memset(p+Instr::endVar, 0, 2);
            break;
        case Instr::cCasoSe:
            memset(p+Instr::endVar, 0, 4);
        }
    }

// Acerta endere�os de desvio conforme instru��es
    for (char * p = comandos; p[0] || p[1]; p += Num16(p))
        switch (p[2])
        {
        case Instr::cSe:
        case Instr::cSenao1:
        case Instr::cSenao2:
            nivelse=0;
            for (x=p+Num16(p); x[0] || x[1]; x += Num16(x))
            {
                if (AcertaComandosFim(x[2]))
                    break;
                if (x[2]==Instr::cSe)
                    nivelse++;
                else if (nivelse == 0 && (x[2] == Instr::cSenao1 ||
                                          x[2] == Instr::cSenao2))
                    break;
                else if (x[2]==Instr::cFimSe)
                {
                    if (nivelse==0)
                        break;
                    nivelse--;
                }
            }
            if (x - p >= 0x10000)
                x = p + Num16(p);
            p[Instr::endVar]     = (x-p);
            p[Instr::endVar + 1] = (x-p)>>8;
            break;
        case Instr::cEnquanto:
        case Instr::cEPara:
            nivelse=0;
            for (x = p+Num16(p); x[0] || x[1]; x += Num16(x))
            {
                if (AcertaComandosFim(x[2]))
                    break;
                if (x[2] == Instr::cEnquanto || x[2] == Instr::cEPara)
                    nivelse++;
                else if (x[2] == Instr::cContinuar1 ||
                         x[2] == Instr::cContinuar2)
                {
                    int valor = x-p;
                    if (valor > 0x10000) valor=0;
                    x[Instr::endVar]     = valor;
                    x[Instr::endVar + 1] = valor >> 8;
                }
                else if (x[2] == Instr::cEFim)
                {
                    if (nivelse)
                    {
                        nivelse--;
                        continue;
                    }
                    int valor = x - p;
                    if (valor > 0x10000) valor=0;
                    x[Instr::endVar]     = valor;
                    x[Instr::endVar + 1] = valor >> 8;
                    x+=Num16(x);
                    break;
                }
            }
            if (x - p >= 0x10000)
                x = p + Num16(p);
            p[Instr::endVar]     = (x - p);
            p[Instr::endVar + 1] = (x - p) >> 8;
            break;
        case Instr::cCasoVar:
            nivelse = 0;
            casopadrao = nullptr;
            casonum = 0;
            for (x = p + Num16(p); x[0] || x[1]; x += Num16(x))
            {
                if (AcertaComandosFim(x[2]))
                    break;
                if (x[2] == Instr::cCasoVar)
                    nivelse++;
                else if (x[2] == Instr::cCasoSePadrao)
                {
                    if (nivelse == 0)
                        casopadrao = x;
                }
                else if (x[2] == Instr::cCasoSe)
                {
                    if (nivelse == 0 && casonum<sizeof(caso1)/sizeof(caso1[0]))
                        caso1[casonum++] = x;
                }
                else if (x[2] == Instr::cCasoFim)
                {
                    if (nivelse)
                    {
                        nivelse--;
                        continue;
                    }
                // Acerta casopadrao
                    if (casopadrao == nullptr)
                        casopadrao = x;
                // Verifica se existe algum casose
                    if (casonum == 0)
                    {
                        p[Instr::endVar]     = (casopadrao - p);
                        p[Instr::endVar + 1] = (casopadrao - p) >> 8;
                        break;
                    }
                // Organiza caso1 em ordem alfab�tica; resultado em var1
                    char ** var1 = caso1;
                    char ** var2 = caso2;
                    for (unsigned int a = 1; a < casonum; a += a)
                    {
                        char ** pont = var2;
                        var2 = var1;
                        var1 = pont;
                        int lido = 0;
                        for (unsigned int b = 0; b<casonum; )
                        {
                            unsigned int b1 = b, b2 = b + a;
                            b = b2;
                            while (b1 < b && b2 < b + a && b2 < casonum)
                            {
                                if (strcmp(var2[b1] + Instr::endVar + 4,
                                           var2[b2] + Instr::endVar + 4) > 0)
                                    var1[lido++] = var2[b2++];
                                else
                                    var1[lido++] = var2[b1++];
                            }
                            while (b1 < b && b1 < casonum)
                                var1[lido++] = var2[b1++];
                            b += a;
                            while (b2 < b && b2 < casonum)
                                var1[lido++] = var2[b2++];
                        }
                    }
                // Obt�m e anota a ordem de busca
                    int meio = casonum/2;
                    p[Instr::endVar]     = (var1[meio] - p);
                    p[Instr::endVar + 1] = (var1[meio] - p) >> 8;
                    int pont[20][3]; // vari�vel + m�nimo + m�ximo
                    int total = 1;   // Quantidade de vari�veis em pont
                    pont[0][0] = meio; // Aonde anotar
                    pont[0][1] = 0;     // Menor �ndice
                    pont[0][2] = casonum - 1; // Maior �ndice
                    while (total>0)
                    {
                        total--;
                        int var = pont[total][0];
                        int min = pont[total][1];
                        int max = pont[total][2];
                        int soma;
                    // Checa antes (valor m�nimo)
                        if (min >= var)
                            soma = casopadrao - var1[var];
                        else
                        {
                            pont[total][0] = (min + var) / 2;
                            pont[total][1] = min;
                            pont[total][2] = var - 1;
                            soma = var1[pont[total][0]] - var1[var];
                            total++;
                        }
                        var1[var][Instr::endVar]     = soma;
                        var1[var][Instr::endVar + 1] = soma >> 8;
                    // Checa depois (valor m�ximo)
                        if (var >= max)
                            soma = casopadrao - var1[var];
                        else
                        {
                            pont[total][0] = (var + max + 1) / 2;
                            pont[total][1] = var + 1;
                            pont[total][2] = max;
                            soma = var1[pont[total][0]] - var1[var];
                            total++;
                        }
                        var1[var][Instr::endVar + 2] = soma;
                        var1[var][Instr::endVar + 3] = soma >> 8;
                    }
                    break;
                }
            }
            break;
        case Instr::cSair1:
        case Instr::cSair2:
            nivelse=0;
            for (x = p + Num16(p); x[0] || x[1]; x += Num16(x))
                if (AcertaComandosFim(x[2]))
                    break;
                else if (x[2] == Instr::cEnquanto ||
                         x[2] == Instr::cEPara ||
                         x[2] == Instr::cCasoVar)
                    nivelse++;
                else if (x[2] == Instr::cEFim ||
                         x[2] == Instr::cCasoFim)
                {
                    if (nivelse)
                        nivelse--;
                    else
                    {
                        x += Num16(x);
                        break;
                    }
                }
            if (x - p >= 0x10000)
                x = p + Num16(p);
            p[Instr::endVar]     = (x - p);
            p[Instr::endVar + 1] = (x - p) >> 8;
            break;
        }

// Marca quais vari�veis s�o locais (vari�veis definidas na fun��o)
#ifdef OTIMIZAR_VAR
    const char * varlocal[255]; // Aonde est�o definidas as vari�veis locais
    int totallocal = 0;  // Quantas vari�veis locais

    for (char * p = comandos; p[0] || p[1]; p += Num16(p))
    {
        // Retira vari�veis locais de varlocal
        int alin = (unsigned char)p[Instr::endAlin];
        if (alin == 0)
            totallocal = 0;
        while (totallocal)
        {
            if ( alin >=
                 (unsigned char)varlocal[totallocal - 1][Instr::endAlin] )
                break;
            totallocal--;
        }

        // Acrescenta vari�veis locais em varlocal
        char * expr = nullptr;
        if (alin && (unsigned char)p[2] > Instr::cVariaveis)
        {
            if (totallocal < 255)
                varlocal[totallocal++] = p;
            int ind = (unsigned char)p[Instr::endIndice];
            if (ind == 0)
                continue;
            expr = p + ind;
        }
        // Checa se instru��o cont�m express�o num�rica
        else
            switch ((unsigned char)p[2])
            {
            case Instr::cRefVar:
                expr = p + (unsigned char)p[Instr::endIndice];
                break;
            case Instr::cExpr:        expr = p + Instr::endVar; break;
            case Instr::cSe:          expr = p + Instr::endVar + 2; break;
            case Instr::cSenao2:      expr = p + Instr::endVar + 2; break;
            case Instr::cEnquanto:    expr = p + Instr::endVar + 2; break;
            case Instr::cEPara:       expr = p + Instr::endVar + 6; break;
            case Instr::cCasoVar:     expr = p + Instr::endVar + 2; break;
            case Instr::cRet2:        expr = p + Instr::endVar; break;
            case Instr::cSair2:       expr = p + Instr::endVar + 2; break;
            case Instr::cContinuar2:  expr = p + Instr::endVar + 2; break;
            }
        if (expr == nullptr)
            continue;

        while (*expr != Instr::ex_fim && *expr != Instr::ex_coment)
        {
            switch (*expr++)
            {
            //case ex_arg:    // In�cio da lista de argumentos
            //case ex_abre:   // Abre colchetes; segue express�o num�rica + ex_fecha
            case Instr::ex_varabre:
                break;
            case Instr::ex_varlocal:
                {
                    // Obt�m o nome da vari�vel
                    char nome[VAR_NOME_TAM + 1];
                    const char * o = expr + 1;
                    char * d = nome;
                    while (*(unsigned char*)o >= ' ' &&
                           d < nome + sizeof(nome) - 1)
                        *d++ = *o++;
                    *d = 0;

                    // Procura a vari�vel entre as vari�veis locais
                    expr[0] = 255;
                    for (int x = 0; x < totallocal; x++)
                        if (strcmp(nome, varlocal[x] + Instr::endNome) == 0)
                        {
                            expr[0] = x;
                            break;
                        }
                }
            case Instr::ex_varfunc:
                expr++;
            case Instr::ex_varini: // Ser� fechado com ex_varfim
            case Instr::ex_fecha:  // Aberto com ex_abre
            case Instr::ex_ponto:  // Aberto com ex_arg
            case Instr::ex_doispontos:
                while (*expr != Instr::ex_arg &&
                       *expr != Instr::ex_abre &&
                       *expr != Instr::ex_varfim)
                {
                    assert(*expr != 0);
                    expr++;
                }
                break;
            case Instr::ex_varfim:
            case Instr::ex_nulo:
                break;
            case Instr::ex_txt:
                while (*expr)
                    expr++;
                expr++;
                break;
            case Instr::ex_num32p:
            case Instr::ex_num32n:
            case Instr::ex_num32hexp:
            case Instr::ex_num32hexn:
                expr += 2;
            case Instr::ex_num16p:
            case Instr::ex_num16n:
            case Instr::ex_num16hexp:
            case Instr::ex_num16hexn:
                expr++;
            case Instr::ex_num8p:
            case Instr::ex_num8n:
            case Instr::ex_num8hexp:
            case Instr::ex_num8hexn:
                expr++;
            case Instr::ex_num0:
            case Instr::ex_num1:
                while (*expr >= Instr::ex_div1 &&
                       *expr <= Instr::ex_div6)
                    expr++;
                break;
            }
        }

        // Refvar: acrescenta vari�vel local em varlocal
        if (p[2] == Instr::cRefVar)
            if (totallocal < 255)
                varlocal[totallocal++] = p;
    }
#endif

#if 0
// Mostra instru��es com os endere�os de desvio
    for (char * p = comandos; p[0] || p[1]; p += Num16(p))
    {
        int pos = p - comandos;
        char mens[BUF_MENS];
        if (Instr::Decod(mens, p, sizeof(mens)))
        {
            printf("%4d  ", pos);
            for (int x=0; x<p[Instr::endAlin]; x++)
                printf("  ");
            printf("%s", mens);
        }
        else
            printf("ERRO  %s", mens);
        switch (p[2])
        {
        case Instr::cSe:
        case Instr::cSenao1:
        case Instr::cSenao2:
        case Instr::cEnquanto:
        case Instr::cEPara:
        case Instr::cSair1:
            printf("  *** %d", pos+Num16(p+Instr::endVar)); // Para frente
            break;
        case Instr::cEFim:
        case Instr::cContinuar1:
            printf("  *** %d", pos-Num16(p+Instr::endVar)); // Para tr�s
            break;
        case Instr::cCasoVar: // Para frente ou para tr�s
            printf("  *** %d", pos+(signed short)Num16(p + Instr::endVar));
            break;
        case Instr::cCasoSe:
            printf("  *** %d  %d", pos+(signed short)Num16(p+Instr::endVar),
                                   pos+(signed short)Num16(p+Instr::endVar+2));
        }
        putchar('\n');
    }
    fflush(stdout);
#endif
}

//----------------------------------------------------------------------------
// Obt�m classes herdadas de uma determinada classe
int TClasse::Heranca(TClasse ** buffer, int tambuf)
{
    TClasse ** bufler = buffer; // Pr�xima classe que ser� verificada
    TClasse ** bufescr = buffer; // Pr�xima classe que ser� anotada
    TClasse * atual = this; // Classe que est� sendo verificada

    while (tambuf>0)
    {
    // Se encontrou heran�a na classe...
        if ((atual->Comandos[0] || atual->Comandos[1]) &&
             atual->Comandos[2] == Instr::cHerda)
        {
            const char * p = atual->Comandos + Instr::endVar + 1;
            int x = (unsigned char)atual->Comandos[Instr::endVar];
            for (; x; x--)
            {
            // Obt�m a classe herdada
                TClasse * c = Procura(p);
                while (*p++);
                assert(c != nullptr);
            // A classe "this" n�o entra na lista
                if (c == this)
                    continue;
            // Verifica se j� anotou a classe
                TClasse ** cpont = buffer;
                for (; cpont<bufescr; cpont++)
                    if (*cpont == c)
                        break;
                if (cpont<bufescr)
                    continue;
            // Anota a classe
                *bufescr++ = c;
                tambuf--;
                if (tambuf <= 0)
                    break;
            }
        }
    // L� a pr�xima classe
        if (bufler >= bufescr)
            break;
        atual = *bufler++;
    }
    return bufescr - buffer;
}

//----------------------------------------------------------------------------
bool TClasse::RetiraDeriv(TClasse * cl)
{
#if 0
    printf("\nRetiraDeriv %s de %s\n", cl->Nome, Nome); fflush(stdout);
#endif
// Checa se tem classes derivadas
    if (NumDeriv == 0)
        return false;

// Marca todas as classes que ser�o verificadas
    {
        TClasse ** lista1 = ListaDeriv;
        for (int total = NumDeriv; total > 0; total--,lista1++)
            lista1[0]->Marca = 0;
        TClasse ** lista2 = cl->ListaDeriv;
        for (int total = cl->NumDeriv; total > 0; total--,lista2++)
            lista2[0]->Marca = 1;
        cl->Marca = 1;
    }

// Verifica se as classes marcadas ainda herdam essa classe
    unsigned int removeu = 0;
    TClasse ** lista3 = ListaDeriv;
    for (int total = NumDeriv; total > 0; total--,lista3++)
    {
        TClasse * c = *lista3;
        if (c->Marca == 0)
            continue;
        TClasse * buf[HERDA_TAM];
        int tot = c->Heranca(buf, HERDA_TAM);
        for (tot--; tot >= 0; tot--)
            if (buf[tot] == this)
                break;
#if 0
        printf("%s %s\n", tot >= 0 ? "Manter" : "Remover", cl->Nome);
        fflush(stdout);
#endif

        if (tot >= 0)
            continue;
        removeu++;
        *lista3 = 0;
    }

// Acerta ListaDeriv
    if (removeu == 0)
        return false;
    if (removeu >= NumDeriv)
    {
        delete[] ListaDeriv;
        ListaDeriv = nullptr;
        NumDeriv = 0;
    }
    else
    {
        const int ltam = NumDeriv - removeu;
        TClasse ** lnovo = new TClasse*[ltam];
        TClasse ** p = ListaDeriv;
        int total1 = NumDeriv;
        int total2 = 0;
        for (; total1 > 0; total1--, p++)
            if (*p)
                lnovo[total2++] = *p;
        assert(total2 == ltam);
        NumDeriv = ltam;
        delete[] ListaDeriv;
        ListaDeriv = lnovo;
    }
    return true;
}

//----------------------------------------------------------------------------
void TClasse::AdicionaDeriv(TClasse * cl)
{
// Marca todas as classes que ser�o adicionadas
    {
        TClasse ** lista1 = cl->ListaDeriv;
        for (int total = cl->NumDeriv; total > 0; total--,lista1++)
            lista1[0]->Marca = 1;
        cl->Marca = 1;
        TClasse ** lista2 = ListaDeriv;
        for (int total = NumDeriv; total > 0; total--,lista2++)
            lista2[0]->Marca = 0;
    }

// Obt�m quantas classes ser�o adicionadas
    int add = (cl->Marca ? 1 : 0);
    {
        TClasse ** lista3 = cl->ListaDeriv;
        for (int total = cl->NumDeriv; total > 0; total--,lista3++)
            if (lista3[0]->Marca)
                add++;
    }

// Acerta o tamanho de ListaDeriv
    TClasse ** lnovo = new TClasse*[NumDeriv + add];
    if (NumDeriv)
        memcpy(lnovo, ListaDeriv, NumDeriv * sizeof(TClasse*));
    if (ListaDeriv)
        delete[] ListaDeriv;
    ListaDeriv = lnovo;

// Mostra o que ser� adicionado
#if 0
    printf("\nAdd %d\n", add);
    if (cl->Marca)
        printf("Add %s\n", cl->Nome);
    for (unsigned int x=0; x<cl->NumDeriv; x++)
        if (cl->ListaDeriv[x]->Marca)
            printf("Add %s\n", cl->ListaDeriv[x]->Nome);
    fflush(stdout);
#endif

// Adiciona novas classes
    unsigned int indice = NumDeriv;
    TClasse ** lista4 = cl->ListaDeriv;
    if (cl->Marca)
        lnovo[indice++] = cl;
    for (int total = cl->NumDeriv; total > 0; total--, lista4++)
        if (lista4[0]->Marca)
            lnovo[indice++] = lista4[0];
    assert(indice == NumDeriv + add);
    NumDeriv = indice;
//for (int x=0; x<NumDeriv; x++)
//    printf(">%s\n", ListaDeriv[x]->Nome);
//fflush(stdout);
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
        AcertaComandos(cl->Comandos);
        cl->NumDeriv = 0;
        if (cl->ListaDeriv == nullptr)
            continue;
        delete[] cl->ListaDeriv;
        cl->ListaDeriv = nullptr;
    }

// Mostra heran�a
#ifdef MOSTRA_HERANCA
    for (cl = TClasse::RBfirst(); cl; cl = TClasse::RBnext(cl))
    {
        int total = cl->Heranca(buf, HERDA_TAM);
        printf("herda(%s) ", cl->Nome);
        for (int x = 0; x < total; x++)
            printf("%c %s", x == 0 ? '=' : ',', buf[x]->Nome);
        putchar('\n'); fflush(stdout);
    }
#endif

// Obt�m n�mero de classes derivadas
    for (cl = TClasse::RBfirst(); cl; cl = TClasse::RBnext(cl))
        for (int total = cl->Heranca(buf, HERDA_TAM); total > 0; )
            buf[--total]->NumDeriv++;

// Aloca mem�ria em TClasse::ListaDeriv
    for (TClasse * cl = TClasse::RBfirst(); cl; cl = TClasse::RBnext(cl))
        if (cl->NumDeriv)
        {
            cl->ListaDeriv = new TClasse* [cl->NumDeriv];
            cl->NumDeriv = 0;
        }

// Acerta ListaDeriv e NumDeriv
    for (TClasse * cl = TClasse::RBfirst(); cl; cl = TClasse::RBnext(cl))
        for (int total = cl->Heranca(buf, HERDA_TAM); total > 0; )
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

    AcertaComandos(Comandos);

// Obt�m heran�a antes e depois da mudan�a
    char * p = Comandos;
    Comandos = comandos_antes;
    total1 = Heranca(buf1, HERDA_TAM);
    Comandos = p;
    total2 = Heranca(buf2, HERDA_TAM);

// Verifica se a heran�a n�o mudou
    for (; total1 > 0 && total2 > 0; total1--, total2--)
        if (buf1[total1-1] != buf2[total2-1])
            break;
    if (total1 == 0 && total2 == 0)
        return;

// Retira heran�a
    for (int x1 = 0; x1 < total1; x1++)
        for (int x2 = 0;; x2++)
        {
            if (x2 >= total2) // Retira heran�a
            {
                // Essa classe n�o herda mais buf1[x1]
                buf1[x1]->RetiraDeriv(this);
                break;
            }
            if (buf1[x1] == buf2[x2]) // Se heran�a n�o mudou
                break;
        }

// Coloca heran�a
    for (int x2 = 0; x2 < total2; x2++)
        for (int x1 = 0;; x1++)
        {
            if (x1 >= total1) // Coloca heran�a
            {
                // Classe buf2[x2] herda essa classe
                buf2[x2]->AdicionaDeriv(this);
                break;
            }
            if (buf1[x1] == buf2[x2]) // Se heran�a n�o mudou
                break;
        }
}

//----------------------------------------------------------------------------
void TClasse::LimpaInstr()
{
// Apaga as instru��es da classe
    char * antigo_com = Comandos;
    Comandos = new char[2];
    memset(Comandos, 0, 2);
    AcertaDeriv(antigo_com);
    AcertaVar(true);
    delete[] antigo_com;
// Se n�o � herdada, nada faz
    if (NumDeriv == 0)
        return;
// Acerta heran�a das outras classes - retira essa classe
    for (unsigned int numcl = 0; numcl < NumDeriv; numcl++)
    {
    // Obt�m a classe derivada
        TClasse * cl = ListaDeriv[numcl];
        //printf("Classe %s\n", cl->Nome); fflush(stdout);
    // A primeira instru��o deve ser "Herda" obrigatoriamente
        assert(cl->Comandos[0] || cl->Comandos[1]);
        assert(cl->Comandos[2] == Instr::cHerda);
    // Obt�m o nome dessa classe na instru��o herda
        char * p = cl->Comandos + Instr::endVar + 1;
        int x = (unsigned char)cl->Comandos[Instr::endVar];
        for (; x; x--)
        {
            TClasse * c = Procura(p);
            assert(c != nullptr);
            if (c == this)
                break;
            while (*p++);
        }
    // Se n�o achou: nada faz
        if (x==0)
            continue;
    // Obt�m faixa de endere�o das instru��es
        char * ini = cl->Comandos;
        char * fim = cl->Comandos;
        while (fim[0] || fim[1])
            fim += Num16(fim);
        fim += 2;
    // Retira nome da classe na instru��o herda
        int tam_herda;
        cl->Comandos[Instr::endVar]--;
        if (cl->Comandos[Instr::endVar])
        {
            tam_herda = strlen(p)+1;
            x = Num16(cl->Comandos) - tam_herda;
            cl->Comandos[0] = x;
            cl->Comandos[1] = x >> 8;
        }
        else
            p=cl->Comandos, tam_herda = Num16(p);
        memmove(p, p+tam_herda, fim-p-tam_herda);
#if 0
        printf(">>>> %s\n", cl->Nome);
        for (const char * p = cl->Comandos; Num16(p); p+=Num16(p))
        {
            char mens[BUF_MENS];
            assert(Instr::Decod(mens, p, sizeof(mens)));
            printf(">  %s\n", mens);
        }
        fflush(stdout);
#endif
    // Acerta endere�os das vari�veis na classe herdada
        cl->LimpaInstrSub(ini, fim, -tam_herda);
    // Acerta endere�os das vari�veis nas outras classes
        for (unsigned int numd = 0; numd < cl->NumDeriv; numd++)
            cl->ListaDeriv[numd]->LimpaInstrSub(ini, fim, -tam_herda);
    }
// Indica que a classe n�o est� sendo herdada
    delete[] ListaDeriv;
    ListaDeriv = nullptr;
    NumDeriv = 0;
}

//----------------------------------------------------------------------------
void TClasse::LimpaInstrSub(const char * ini, const char * fim, int desloc)
{
    for (unsigned int y = 0; y < NumVar; y++)
    {
        char * defvar = InstrVar[y];
        if (defvar < ini || defvar >= fim)
            continue;
        defvar += desloc;
        InstrVar[y] = defvar;
        int indvar = IndiceVar[y];
        TVariavel v;
        v.defvar = defvar;
        if (indvar & 0x400000) // Vari�vel da classe
        {
            v.endvar = Vars + (indvar & 0x3FFFFF);
            v.MoverDef();
        }
        else // Vari�vel do objeto
        {
            for (TObjeto * obj = ObjetoIni; obj; obj = obj->Depois)
            {
                v.endvar = obj->Vars + (indvar & 0x3FFFFF);
                v.MoverDef();
            }
        }
    }
}

//----------------------------------------------------------------------------
int TClasse::AcertaVar(bool acertaderiv)
{
    TClasseVar antes;
    antes.InstrVar = InstrVar;
    antes.IndiceVar = IndiceVar;
    antes.NumVar = NumVar;
    antes.Vars = Vars;
    antes.TamVars = TamVars;
    InstrVar = nullptr;
    IndiceVar = nullptr;
    NumVar = 0;
    Vars = nullptr;
    TamVars = 0;

// Obt�m classes herdadas
    TClasse * herda[HERDA_TAM + 1];
    int numherda = Heranca(herda + 1, HERDA_TAM) + 1;
    herda[0] = this;

// Obt�m n�mero de vari�veis/fun��es
    unsigned int total = 0;
    const char * ComandosFim = Comandos;
    for (int cont = 0; cont < numherda; cont++)
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
            case Instr::cConstVar:
                total++, p += Num16(p);
                break;
            case Instr::cFunc:
            case Instr::cVarFunc:
                total++, p += Num16(p), inifunc = true;
                break;
            case Instr::cEnquanto:
            case Instr::cEPara:
            case Instr::cSe:
            case Instr::cSenao1:
            case Instr::cSenao2:
                assert(p[Instr::endVar] || p[Instr::endVar + 1]);
                p += Num16(p+Instr::endVar);
                break;
            case Instr::cFimSe:
            default:
                if (inifunc == false && p[2] > Instr::cVariaveis)
                    total++;
                p += Num16(p);
            }
        }
        if (cont == 0)
            ComandosFim = p;
    }

// Aloca mem�ria
    char ** var1 = 0;
    if (total) var1=new char*[total];
    total = 0;

// Adiciona fun��es/vari�veis
    for (int cont = 0; cont < numherda; cont++)
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
            case Instr::cConstVar:
                var1[total++] = p;
                p += Num16(p);
                break;
            case Instr::cFunc:
            case Instr::cVarFunc:
                var1[total++] = p;
                p += Num16(p), inifunc = true;
                break;
            case Instr::cEnquanto:
            case Instr::cEPara:
            case Instr::cSe:
            case Instr::cSenao1:
            case Instr::cSenao2:
                p += Num16(p+Instr::endVar);
                break;
            case Instr::cFimSe:
            default:
                if (inifunc == false && p[2] > Instr::cVariaveis)
                    var1[total++] = p;
                p += Num16(p);
            }
        }
    }

// Organiza em ordem alfab�tica (aloca/libera mem�ria - var2)
    if (total)
    {
        char ** var2 = new char*[total];
        for (unsigned int a = 1; a < total; a += a)
        {
            char ** pont = var2;
            var2 = var1;
            var1 = pont;
            int lido = 0;
            for (unsigned int b = 0; b < total; )
            {
                unsigned int b1 = b, b2 = b + a;
                b = b2;
                while (b1 < b && b2 < b + a && b2 < total)
                {
                    if (comparaVar(var2[b1] + Instr::endNome,
                            var2[b2] + Instr::endNome) > 0)
                        var1[lido++] = var2[b2++];
                    else
                        var1[lido++] = var2[b1++];
                }
                while (b1 < b && b1 < total)
                    var1[lido++] = var2[b1++];
                b += a;
                while (b2 < b && b2 < total)
                    var1[lido++] = var2[b2++];
            }
        }
        delete[] var2;
    }

// Obt�m n�mero de vari�veis, detectando as repetidas
    NumVar = total;
    for (int x = total - 2; x >= 0; x--)
        if (comparaVar(var1[x]+Instr::endNome, var1[x+1] + Instr::endNome) == 0)
        {
            var1[x + 1]=0;
            NumVar--;
        }

// Acerta TClasse::InstrVar (nomes das vari�veis)
// Acerta mem�ria alocada em var1
    if (NumVar == total)
        InstrVar = var1;
    else
    {
        InstrVar = new char*[NumVar];
        for (unsigned int x = 0, y = 0; x < total; x++)
            if (var1[x])
                InstrVar[y++] = var1[x];
        delete[] var1;
    }
    var1=0;

// Acerta bits de controle de TClasse::IndiceVar
// Acerta vari�veis cInt1 (alinhamento de bit)
    int indclasse = 0, bitclasse = 0;
    int indobjeto = 0, bitobjeto = 0;
    IndiceVar = new unsigned int[NumVar];
    for (unsigned int x = 0; x < NumVar; x++)
    {
        unsigned int valor = 0;
    // Verifica se est� na pr�pria classe
        if (InstrVar[x]>=Comandos && InstrVar[x] <= ComandosFim)
            valor |= 0x800000;
    // Verifica se � vari�vel "comum"
        if (InstrVar[x][Instr::endProp] & 1)
            valor |= 0x400000;
    // Verifica cBit1
        if (InstrVar[x][2] == Instr::cInt1)
        {
            int total = (unsigned char)InstrVar[x][Instr::endVetor];
            if (total == 0)
                total++;
            if (valor & 0x400000) // Classe
            {
                valor += indclasse + (bitclasse << 24);
                bitclasse += total;
                indclasse += bitclasse/8;
                bitclasse &= 7;
            }
            else // Objeto
            {
                valor += indobjeto + (bitobjeto << 24);
                bitobjeto += total;
                indobjeto += bitobjeto/8;
                bitobjeto &= 7;
            }
        }
    // Anota o resultado
        IndiceVar[x] = valor;
    }
    if (bitclasse) indclasse++;
    if (bitobjeto) indobjeto++;

// Acerta vari�veis com alinhamento de 1 byte
    for (unsigned int x = 0; x<NumVar; x++)
    {
        if (InstrVar[x][2] == Instr::cInt1)
            continue;
        int tamanho = TVariavel::Tamanho(InstrVar[x]);
        if (tamanho & 1)
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
    if (indclasse & 1) indclasse++;
    if (indobjeto & 1) indobjeto++;

// Acerta vari�veis com alinhamento de 2 bytes
    for (unsigned int x = 0; x < NumVar; x++)
    {
        if (InstrVar[x][2] == Instr::cInt1)
            continue;
        int tamanho = TVariavel::Tamanho(InstrVar[x]);
        if ((tamanho & 3) == 2)
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
    if (indclasse & 2) indclasse += 2;
    if (indobjeto & 2) indobjeto += 2;

// Acerta outras vari�veis (alinhamento de 4 bytes)
    for (unsigned int x = 0; x < NumVar; x++)
    {
        if (InstrVar[x][2] == Instr::cInt1)
            continue;
        int tamanho = TVariavel::Tamanho(InstrVar[x]);
        if (tamanho && (tamanho & 3) == 0)
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

// Acerta vari�veis
    Vars = nullptr;
    TamVars = indclasse;
    TamObj = indobjeto;

// Mostra o resultado
#ifdef MOSTRA_MEM
    printf("Classe=\"%s\"   Vari�veis classe=%d   Vari�veis Objeto=%d\n",
           Nome, indclasse, indobjeto);
    puts("  lugar  endere�o:bit  tamanho  instru��o");
    for (int passo = 0; passo < 2; passo++)
        for (unsigned int x = 0; x < NumVar; x++)
        {
            unsigned int valor = IndiceVar[x];
            if (((valor & 0x400000) == 0) == (passo == 0))
                continue;
            int tam = TVariavel::Tamanho(InstrVar[x]);
            char mens[BUF_MENS];
            if (!Instr::Decod(mens, InstrVar[x], sizeof(mens)))
                strcpy(mens, "-----");
            printf("  %s  %d",
                    IndiceVar[x] & 0x400000 ? "classe" : "objeto",
                    valor & 0x3FFFFF);
            if ((valor>>24)!=0)
                printf(":%02x", valor >> 24);
            printf("  %d  %s\n", tam, mens);
        }
    putchar('\n');
#endif

// Nenhuma vari�vel: nada para ser feito
    if (NumVar == 0 && antes.NumVar == 0)
        return 0;

// Prepara lista de vari�veis que mudaram
    antes.al =  new TClasseAloca[NumVar + antes.NumVar];
    TClasseAloca * al = antes.al;
    indobjeto = 0; // N�mero de vari�veis de objetos
    indclasse = 0; // N�mero de vari�veis de objetos + classe
    bitobjeto = 0; // Bit 1 =1 se alguma vari�vel da classe mudou
                   // Bit 2 =1 se alguma vari�vel de objeto mudou
                   // Diferente de 0 se a lista de vari�veis mudou
    total = 0;     // Para obter o n�mero de vari�veis

        // passo=0 -> obt�m vari�veis de objetos
        // passo=1 -> obt�m vari�veis da classe (definida com "comum")
    for (int passo = 0; passo < 2; passo++)
    {
        unsigned int c1 = 0, c2 = 0;
        TVariavel v;
        v.Limpar();
    // Checa vari�veis que mudaram
        while (c1 < antes.NumVar && c2 < NumVar)
        {
        // Pula vari�veis que n�o correspondem ao passo
            if ((antes.InstrVar[c1][Instr::endProp] & 1) != passo)
            {
                c1++;
                continue;
            }
            if ((InstrVar[c2][Instr::endProp] & 1) != passo)
            {
                c2++;
                continue;
            }
        // Compara vari�veis antes e depois
        // Checa tamb�m se heran�a da vari�vel mudou
            int cmp = comparaVar(antes.InstrVar[c1] + Instr::endNome,
                                 InstrVar[c2] + Instr::endNome);
            if (cmp != 0 || antes.IndiceVar[c1] != IndiceVar[c2])
                bitobjeto |= 0x10;
        // Pula vari�veis com tamanho zero
            bool tamzero = false;
            if (TVariavel::Tamanho(antes.InstrVar[c1]) == 0)
                c1++, tamzero=true;
            if (TVariavel::Tamanho(InstrVar[c2]) == 0)
                c2++, tamzero=true;
            if (tamzero)
                continue;
        // Vari�vel apagada
            if (cmp < 0)
            {
                if (antes.InstrVar[c1][2] > Instr::cVarFunc)
                {
                    al[total].comando = 1;
                    al[total].defvar = antes.InstrVar[c1];
                    al[total].indvar = antes.IndiceVar[c1];
                    total++;
                }
                c1++, bitobjeto |= 1;
            }
        // Vari�vel criada
            else if (cmp > 0)
            {
                if (InstrVar[c2][2] > Instr::cVarFunc)
                {
                    al[total].comando = 2;
                    al[total].defvar = InstrVar[c2];
                    al[total].indvar = IndiceVar[c2];
                    total++;
                }
                c2++, bitobjeto |= 1;
            }
        // Tipo de vari�vel n�o mudou: mover
            else if (antes.InstrVar[c1][2] == InstrVar[c2][2] &&
                antes.InstrVar[c1][Instr::endIndice] ==
                      InstrVar[c2][Instr::endIndice])
            {
                if ((InstrVar[c2][2] == Instr::cTxt1 ||
                    InstrVar[c2][2] == Instr::cTxt2) &&
                    antes.InstrVar[c1][Instr::endExtra] !=
                          InstrVar[c2][Instr::endExtra])
                {
                    al[total].comando = 4; // Tamanho de txt diferente: copiar
                    bitobjeto |= 1;
                }
                else if (InstrVar[c2][2] == Instr::cInt1)
                    al[total].comando = 4;  // Bits: devem ser copiados
                else
                    al[total].comando = 3; // Mover
                unsigned char tam1 = antes.InstrVar[c1][Instr::endVetor];
                unsigned char tam2 = InstrVar[c2][Instr::endVetor];
                if ((tam1 ? tam1 : 1) != (tam2 ? tam2 : 1))
                    bitobjeto |= 1;
                al[total].defvar = antes.InstrVar[c1];
                al[total].indvar = antes.IndiceVar[c1];
                total++;
                al[total].comando = 0;
                al[total].defvar = InstrVar[c2];
                al[total].indvar = IndiceVar[c2];
                total++, c1++, c2++;
            }
        // Tipo mudou: copia vari�vel
            else
            {
                v.defvar = InstrVar[c2];
                al[total].comando = 4;
                al[total].defvar = antes.InstrVar[c1];
                al[total].indvar = antes.IndiceVar[c1];
                total++;
                al[total].comando = 0;
                al[total].defvar = InstrVar[c2];
                al[total].indvar = IndiceVar[c2];
                bitobjeto |= 1;
                total++, c1++, c2++;
            }
        }
    // Vari�veis que ser�o apagadas
        for (; c1 < antes.NumVar; c1++)
            if ((antes.InstrVar[c1][Instr::endProp] & 1) == passo)
            {
                bitobjeto |= 0x10;
                if (TVariavel::Tamanho(antes.InstrVar[c1]) == 0)
                    continue;
                bitobjeto |= 1;
                if (antes.InstrVar[c1][2] <= Instr::cVarFunc)
                    continue;
                al[total].comando = 1;
                al[total].defvar = antes.InstrVar[c1];
                al[total].indvar = antes.IndiceVar[c1];
                total++;
            }
    // Vari�veis que ser�o criadas
        for (; c2 < NumVar; c2++)
            if ((InstrVar[c2][Instr::endProp] & 1) == passo)
            {
                bitobjeto |= 0x10;
                if (TVariavel::Tamanho(InstrVar[c2]) == 0)
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
    for (unsigned int x = 0; x < total; x++)
    {
        al[x].tam = (unsigned char)al[x].defvar[Instr::endVetor];
        if (al[x].tam == 0)
            al[x].tam = 1;
        al[x].numbit = al[x].indvar >> 24;
    }

// Acerta al[n].tam para comandos 4
    for (unsigned int x = 0; x < total; x++)
        if (al[x].comando == 4)
        {
            if (al[x].tam > al[x+1].tam)
                al[x].tam = al[x+1].tam;
            if (al[x].tam > 0)
                al[x].tam--;
        }

// Mostra a lista de altera��es
#ifdef MOSTRA_MUDOU
    printf("Classe=\"%s\"  classe_mudou=%s  objeto_mudou=%s  "
           "vari�veis_mudaram=%s\n",
           Nome, bitobjeto & 2 ? "sim" : "n�o",
                 bitobjeto & 4 ? "sim" : "n�o",
                 bitobjeto     ? "sim" : "n�o");
    for (int x = 0; x < indclasse; x++)
    {
        int param = 0;
        char mens[BUF_MENS];
        printf("  %s  ", x<indobjeto ? "objeto" : "classe");
        switch (al[x].comando)
        {
        case 1: printf("apagar("); param = 1; break;
        case 2: printf("criar("); param = 1; break;
        case 3: printf("mover("); param = 2; break;
        case 4: printf("copiar("); param = 2; break;
        default: printf("??? (");
        }
        if (param >= 1)
            if (Instr::Decod(mens, al[x].defvar, sizeof(mens)))
                printf("%s", mens);
        if (param >= 2)
        {
            if (Instr::Decod(mens, al[x+1].defvar, sizeof(mens)))
                printf(")  (%s", mens);
            x++;
        }
        printf(")\n");
    }
    putchar('\n'); fflush(stdout);
#endif

// Acerta vari�veis da classe
    if ((bitobjeto & 2) == 0) // Nenhuma vari�vel mudou
    {
        Vars = antes.Vars, antes.Vars = nullptr;
        for (int x = indobjeto; x < indclasse; x++)
            if (al[x].comando == 3) // Mover
            {
                x++; // Porque o novo endere�o est� em al[x+1]
                al[x].endvar = Vars + (al[x].indvar & 0x3FFFFF);
                al[x].MoverDef();
            }
    }
    else // Alguma vari�vel mudou
    {
        if (TamVars)
        {
            Vars = new char[TamVars];
            memset(Vars, 0, TamVars);
        }
        for (int x = indobjeto; x < indclasse; x++)
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
            case 3: // Mover; vari�vel n�o mudou
                if (al[x].tam == al[x+1].tam)
                    al[x].MoverEnd(al[x+1].endvar, this, 0);
                else if (al[x].tam > al[x+1].tam)
                {
                    const char * def = al[x].defvar;
                    al[x].Redim(0, 0, al[x].tam, al[x+1].tam);
                    al[x].defvar = al[x+1].defvar;
                    al[x].MoverEnd(al[x+1].endvar, this, 0);
                    al[x].defvar = def;
                }
                else
                {
                    al[x].MoverEnd(al[x+1].endvar, this, 0);
                    al[x+1].Redim(this, 0, al[x].tam, al[x+1].tam);
                }
                al[x+1].MoverDef();
                x++;
                break;
            case 4:
                if (al[x+1].defvar[2] > Instr::cVarFunc)
                    al[x+1].Criar(this, nullptr);
                al[x].indice = al[x+1].indice = al[x].tam;
                for (; al[x].indice; al[x].indice--, al[x+1].indice--)
                    al[x+1].OperadorAtrib( &al[x] );
                al[x+1].OperadorAtrib( &al[x] );
                if (al[x].defvar[2] > Instr::cVarFunc)
                    al[x].Apagar();
                x++;
                break;
            } // switch
        } // for
    } // if

// Acerta vari�veis dos objetos
    if ((bitobjeto & 4) == 0) // Nenhuma vari�vel mudou
    {
        for (TObjeto * obj = ObjetoIni; obj; obj=obj->Depois)
            for (int x = 0; x < indobjeto; x++)
                if (al[x].comando == 3) // Mover
                {
                    x++; // Porque o novo endere�o est� em al[x+1]
                    al[x].endvar = obj->Vars + (al[x].indvar & 0x3FFFFF);
                    al[x].MoverDef();
                }
    }
    else // Alguma vari�vel mudou
    {
        for (int nobj = NumObj; nobj > 0; nobj--)
        {
        // Cria objeto no final da lista ligada
            TObjeto * obj = TObjeto::Criar(this, false);
        // Copia vari�veis do primeiro objeto para o objeto criado
            for (int x = 0; x < indobjeto; x++)
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
                case 3: // Mover; vari�vel n�o mudou
                    if (al[x].tam == al[x+1].tam)
                        al[x].MoverEnd(al[x+1].endvar, this, obj);
                    else if (al[x].tam > al[x+1].tam)
                    {
                        const char * def = al[x].defvar;
                        al[x].Redim(0, 0, al[x].tam, al[x+1].tam);
                        al[x].defvar = al[x+1].defvar;
                        al[x].MoverEnd(al[x+1].endvar, this, obj);
                        al[x].defvar = def;
                    }
                    else
                    {
                        al[x].MoverEnd(al[x+1].endvar, this, obj);
                        al[x+1].Redim(this, obj, al[x].tam, al[x+1].tam);
                    }
                    al[x+1].MoverDef();
                    x++;
                    break;
                case 4:
                    if (al[x+1].defvar[2] > Instr::cVarFunc)
                        al[x+1].Criar(this, obj);
                    al[x].indice = al[x+1].indice = al[x].tam;
                    for (; al[x].indice; al[x].indice--, al[x+1].indice--)
                        al[x+1].OperadorAtrib( &al[x] );
                    al[x+1].OperadorAtrib( &al[x] );
                    if (al[x].defvar[2] > Instr::cVarFunc)
                        al[x].Apagar();
                    x++;
                    break;
                } // switch
            } // for
        // Apaga objeto mudando as refer�ncias do objeto
            ObjetoIni->Apagar(obj);
        }
    }

// Acerta classes derivadas
    while (acertaderiv)
    {
    // Indica que o arquivo foi alterado
        if (ArqArquivo)
            ArqArquivo->Mudou = true;
    // Se lista de vari�veis mudou: acerta classes derivadas
        if (bitobjeto)
        {
            for (unsigned int x = 0; x < NumDeriv; x++)
                ListaDeriv[x]->AcertaVar(false);
            break;
        }
    // Obt�m faixa de endere�o das vari�veis
        const char * ini = nullptr;
        const char * fim = nullptr;
        for (unsigned int x = 0; x < antes.NumVar; x++)
            if (antes.IndiceVar[x] & 0x800000)
            {
                if (ini == nullptr)
                    ini = fim = antes.InstrVar[x];
                else if (ini > antes.InstrVar[x])
                    ini = antes.InstrVar[x];
                else if (fim < antes.InstrVar[x])
                    fim = antes.InstrVar[x];
            }
    // Lista n�o mudou: apenas acerta os endere�os das vari�veis
        for (unsigned int x = 0; x < NumDeriv; x++)
        {
            TClasse * cl = ListaDeriv[x];
            for (unsigned int y = 0; y < cl->NumVar; y++)
                if (cl->InstrVar[y] >= ini && cl->InstrVar[y] <= fim)
                {
                    TVariavel v;
                    int ind = IndiceNome(cl->InstrVar[y] + Instr::endNome);
                    assert(ind >= 0);
                    //printf("[%s]:%p -> [%s]:%p\n",
                    //       cl->InstrVar[y] + Instr::endNome, cl->InstrVar[y],
                    //       InstrVar[ind] + Instr::endNome, InstrVar[ind]);
                    //fflush(stdout);
                    v.defvar = cl->InstrVar[y] = InstrVar[ind];
                    ind = cl->IndiceVar[y];
                    if (ind & 0x400000) // Vari�vel da classe
                    {
                        v.endvar = cl->Vars + (ind & 0x3FFFFF);
                        v.MoverDef();
                    }
                    else // Vari�vel do objeto
                        for (TObjeto * obj = cl->ObjetoIni; obj; obj = obj->Depois)
                        {
                            v.endvar = obj->Vars + (ind & 0x3FFFFF);
                            v.MoverDef();
                        }
                    //puts(InstrVar[ind] + Instr::endNome); fflush(stdout);
                }
        }
        break;
    }

    return (bitobjeto == 0 ? 0 : (bitobjeto & 15) == 0 ? 1 : 2);
}

//----------------------------------------------------------------------------
TClasse * TClasse::Procura(const char * nome)
{
    TClasse * y = RBroot;
    while (y)
    {
        int i = comparaVar(nome, y->Nome);
        if (i == 0)
            return y;
        if (i < 0)
            y = y->RBleft;
        else
            y = y->RBright;
    }
    return 0;
}

//----------------------------------------------------------------------------
TClasse * TClasse::ProcuraIni(const char * nome)
{
    TClasse * x = nullptr;
    TClasse * y = RBroot;
    while (y)
    {
        int i = comparaVar(nome, y->Nome);
        if (i == 0)
            return y;
        if (i == -2)
            x = y;
        if (i < 0)
            y = y->RBleft;
        else
            y = y->RBright;
    }
    return x;
}

//----------------------------------------------------------------------------
TClasse * TClasse::ProcuraFim(const char * nome)
{
    TClasse * x = nullptr;
    TClasse * y = RBroot;
    while (y)
    {
        int i = comparaVar(nome, y->Nome);
        if (i == 0 || i == -2)
            x = y;
        if (i == -1)
            y = y->RBleft;
        else
            y = y->RBright;
    }
    return x;
}

//----------------------------------------------------------------------------
int TClasse::IndiceNome(const char * nome)
{
    int ini = 0; // �ndice inicial
    int fim = NumVar - 1; // �ndice final
    while (ini<=fim)
    {
        int meio = (ini + fim) / 2;
        switch (comparaVar(nome, InstrVar[meio] + Instr::endNome))
        {
        case 2:
        case 1:
            ini = meio + 1;
            break;
        case 0:
            return meio;
        case -1:
        case -2:
            fim = meio - 1;
        }
    }
    return -1;
}

//----------------------------------------------------------------------------
int TClasse::IndiceNomeIni(const char * nome)
{
    int ini = 0; // �ndice inicial
    int fim = NumVar - 1; // �ndice final
    int ind = -1;
    while (ini<=fim)
    {
        int meio = (ini + fim) / 2;
        switch (comparaVar(nome, InstrVar[meio] + Instr::endNome))
        {
        case 2:
        case 1:
            ini = meio + 1;
            break;
        case 0:
            return meio;
        case -2:
            ind = meio;
        default:
            fim = meio - 1;
        }
    }
    return ind;
}

//----------------------------------------------------------------------------
int TClasse::IndiceNomeFim(const char * nome)
{
    int ini = 0; // �ndice inicial
    int fim = NumVar - 1; // �ndice final
    int ind = -1;
    while (ini<=fim)
    {
        int meio = (ini + fim) / 2;
        switch (comparaVar(nome, InstrVar[meio] + Instr::endNome))
        {
        case 2:
        case 1:
            ini = meio + 1;
            break;
        case -1:
            fim = meio - 1;
            break;
        case -2:
        case 0:
            ind = meio;
            ini = meio + 1;
        }
    }
    return ind;
}

//----------------------------------------------------------------------------
int TClasse::RBcomp(TClasse * x, TClasse * y)
{
    return comparaVar(x->Nome, y->Nome);
}

//----------------------------------------------------------------------------
TClasse * TClasse::RBroot = nullptr;
#define CLASS TClasse          // Nome da classe
#define RBmask 1 // M�scara para bit 0
#include "rbt.cpp.h"
