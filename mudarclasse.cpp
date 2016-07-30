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
#include <assert.h>
#include "mudarclasse.h"
#include "classe.h"
#include "objeto.h"
#include "instr.h"
#include "arqmapa.h"
#include "misc.h"

//#define DEBUG

TMudarClasse * TMudarClasse::Inicio=0;
TMudarClasse * TMudarClasse::Fim=0;
char TMudarClasse::Salvar=0;
TRenomeiaClasse * TRenomeiaClasse::Inicio = 0;
TRenomeiaClasse * TRenomeiaClasse::Fim = 0;

//----------------------------------------------------------------------------
TMudarAux::TMudarAux()
{
    numbloco=0;
}

//------------------------------------------------------------------------------
void TMudarAux::AddBloco(const char * ender, int tamanho)
{
    if (tamanho<=0 || numbloco >= sizeof(tambloco) / sizeof(tambloco[0]))
        return;
    endbloco[numbloco] = ender;
    tambloco[numbloco] = tamanho;
    numbloco++;
#if 0
    puts("Bloco");
    while (tamanho>0)
    {
        char mens[4096];
        Instr::Decod(mens, ender, sizeof(mens));
        printf(">>>%s\n", mens);
        tamanho -= Num16(ender), ender += Num16(ender);
    }
#endif
}

//------------------------------------------------------------------------------
bool TMudarAux::ChecaBloco(char * mensagem, int tamanho)
{
// Checa o tamanho do bloco
    int total=0;
    for (unsigned int bloco=0; bloco<numbloco; bloco++)
        total += tambloco[bloco];
// Verifica se bloco válido
    int linha=1;
    Instr::ChecaLinha checalinha;
    checalinha.Inicio();
    for (unsigned int bloco=0; bloco<numbloco; bloco++)
    {
        const char * com = endbloco[bloco];
        const char * fim = com + tambloco[bloco];
        while (com<fim)
        {
            const char * p = checalinha.Instr(com);
            if (p)
            {
                mprintf(mensagem, tamanho, "%d: %s\n", linha, p);
                return false;
            }
            com+=Num16(com), linha++;
        }
    }
    const char * p = checalinha.Fim();
    if (p)
    {
        mprintf(mensagem, tamanho, "%d: %s\n", linha, p);
        return false;
    }
    *mensagem=0;
    return true;
}

//------------------------------------------------------------------------------
void TMudarAux::AnotaBloco(TMudarClasse * obj)
{
    int total=0;
    for (unsigned int bloco=0; bloco<numbloco; bloco++)
        total += tambloco[bloco];
    char * com = new char[total+2];
    total=0;
    for (unsigned int bloco=0; bloco<numbloco; bloco++)
    {
        memcpy(com+total, endbloco[bloco], tambloco[bloco]);
        total += tambloco[bloco];
    }
    com[total]=0;
    com[total+1]=0;
    obj->MudarComandos(com);
}

//------------------------------------------------------------------------------
char * TMudarAux::ProcuraInstr(char * comando, const char * nomevar)
{
    bool inifunc = false;
    while (comando[0] || comando[1])
    {
        switch (comando[2])
        {
        case Instr::cFunc:
        case Instr::cVarFunc:
            inifunc=true;
        case Instr::cConstNulo:
        case Instr::cConstTxt:
        case Instr::cConstNum:
        case Instr::cConstExpr:
        case Instr::cConstVar:
            if (comparaVar(comando + Instr::endNome, nomevar)==0)
                return comando;
            comando += Num16(comando);
            break;
        default:
            if (inifunc==false && comando[2] > Instr::cVariaveis)
                if (comparaVar(comando + Instr::endNome, nomevar)==0)
                    return comando;
            comando += Num16(comando);
        }
    }
    return comando;
}

//------------------------------------------------------------------------------
char * TMudarAux::AvancaInstr(char * comando)
{
    if (comando[0]==0 && comando[1]==0)
        return comando;
    if (comando[2] != Instr::cFunc && comando[2] != Instr::cVarFunc)
        return comando + Num16(comando);
    comando += Num16(comando);
    while (comando[0] || comando[1])
    {
        switch (comando[2])
        {
        case Instr::cFunc:
        case Instr::cVarFunc:
        case Instr::cConstNulo:
        case Instr::cConstTxt:
        case Instr::cConstNum:
        case Instr::cConstExpr:
        case Instr::cConstVar:
            return comando;
        }
        comando += Num16(comando);
    }
    return comando;
}

//------------------------------------------------------------------------------
char * TMudarAux::FimInstr(char * comando)
{
    while (comando[0] || comando[1])
        comando += Num16(comando);
    return comando;
}

//------------------------------------------------------------------------------
bool TMudarAux::CodifInstr(TAddBuffer * destino, const char * origem)
{
    char mens[4096];
    char menscod[4096];
    int linhanum = 1;
    char * linhaend = mens;
    bool codifok = true;

    while (true)
    {
    // Lê uma linha
        unsigned char ch = *origem++;
        if (ch && ch != Instr::ex_barra_n)
        {
            if (ch==' ' && linhaend==mens)
                continue;
            if (linhaend >= mens+sizeof(mens)-1)
            {
                sprintf(mens, "%d: Instrução muito grande", linhanum);
                destino->Limpar();
                destino->Add(mens, strlen(mens));
                return false;
            }
            *linhaend++ = ch;
            continue;
        }
    // Checa linha vazia
        if (linhaend==mens)
        {
            if (ch==0)
                break;
            linhanum++;
            continue;
        }
    // Checa \ seguido de \n
        if (ch==Instr::ex_barra_n && origem[-2]=='\\')
        {
            linhaend--;
            linhanum++;
            continue;
        }
    // Processa instrução
        *linhaend=0;
        linhaend=mens;
        if (Instr::Codif(menscod, mens, sizeof(menscod)))
        {
            if (codifok)
                destino->Add(menscod, Num16(menscod));
        }
        else
        {
            //printf("\nAntes: %s\n", mens);
            sprintf(mens, "%c%d: %s", Instr::ex_barra_n, linhanum, menscod);
            //printf("Depois: %s\n", mens); fflush(stdout);
            if (codifok)
            {
                destino->Limpar();
                destino->Add(mens+1, strlen(mens+1));
                codifok=false;
            }
            else
                destino->Add(mens, strlen(mens));
        }
        if (ch==0)
            break;
        linhanum++;
    }
    return codifok;
}

//----------------------------------------------------------------------------
TMudarClasse::TMudarClasse(const char * nome)
{
    RBcolour=0;
    Comandos=0;
    copiastr(Nome, nome, sizeof(Nome));
    Arquivo=0;
    RBinsert();
    Antes=Fim;
    Depois=0;
    (Antes ? Antes->Depois : Inicio) = this;
    Fim = this;
}

//----------------------------------------------------------------------------
TMudarClasse::~TMudarClasse()
{
    if (Comandos)
        delete[] Comandos;
    RBremove();
    (Antes ? Antes->Depois : Inicio) = Depois;
    (Depois ? Depois->Antes : Fim) = Antes;
}

//----------------------------------------------------------------------------
void TMudarClasse::MudarComandos(char * com)
{
    if (com==Comandos)
        return;
    if (Comandos)
        delete[] Comandos;
    Comandos = com;
}

//----------------------------------------------------------------------------
bool TMudarClasse::ExecPasso()
{
// Executa funções iniclasse se possível
    while (Inicio)
    {
        if (Inicio->Comandos || // Se classe vai ser alterada
            (Inicio->RBcolour & 2)) // Se classe vai ser apagada
            break;
        if ((Inicio->RBcolour & 4)==0) // Se não foi mudada
        {
            delete Inicio;
            continue;
        }
            // Foi mudada
        TClasse * cl = TClasse::Procura(Inicio->Nome);
        delete Inicio;
        if (cl==0)
            continue;
#ifdef DEBUG
        printf("iniclasse %s\n", cl->Nome); fflush(stdout);
#endif
        if (Instr::ExecIni(cl, "iniclasse"))
        {
            Instr::ExecArg(cl->Nome);
            Instr::ExecX();
            return true;
        }
    }
// Apaga classes
    bool pendente = false;
    for (TMudarClasse * mudar = Inicio; mudar; )
    {
        TMudarClasse * m = mudar;
        mudar = mudar->Depois;
    // Verifica se classe deve ser apagada
        if ((m->RBcolour & 2)==0)
            continue;
    // Obtém a classe
        TClasse * cl = TClasse::Procura(m->Nome);
        if (cl==0)
        {
            m->RBcolour &= ~2;
            if (m->Comandos==0)
                delete m;
            continue;
        }
    // Verifica se existem objetos da classe
        if (cl->ObjetoIni)
        {
#ifdef DEBUG
            printf("apagando objetos %s\n", cl->Nome); fflush(stdout);
#endif
            for (TObjeto * obj = cl->ObjetoIni; obj; obj=obj->Depois)
                obj->MarcarApagar();
            pendente = true;
            continue;
        }
    // Apaga a classe
        m->RBcolour &= ~2;
        if (m->Comandos==0) // Nenhuma alteração: apaga TMudarClasse
            delete m;
        else if (m->Arquivo==0) // Anota o arquivo
            m->Arquivo = cl->ArqArquivo;
#ifdef DEBUG
        printf("delete %s\n", cl->Nome); fflush(stdout);
#endif
        delete cl;
    }
// Retorna se tem objetos para apagar
    if (pendente)
        return true;
// Cria classes
    for (TMudarClasse * mudar = Inicio; mudar; mudar=mudar->Depois)
    {
    // Verifica se classe vai ser mudada
        if (mudar->Comandos==0)
            continue;
    // Verifica se a classe já existe
        if (TClasse::Procura(mudar->Nome) != 0)
            continue;
    // Cria a classe vazia
        TClasse * cl = new TClasse(mudar->Nome, mudar->Arquivo);
        cl->Comandos = new char[2];
        memset(cl->Comandos, 0, 2);
        //cl->AcertaDeriv(mens);
        //cl->AcertaVarSub();
#ifdef DEBUG
        printf("new %s\n", cl->Nome); fflush(stdout);
#endif
    // Indica que precisa executar função iniclasse
        if (TClasse::ClInic == 0)
            mudar->RBcolour |= 4;
        else if (TClasse::RBcomp(cl, TClasse::ClInic) < 0)
            mudar->RBcolour |= 4;
    }
// Altera instruções das classes
    for (TMudarClasse * mudar = Inicio; mudar; mudar=mudar->Depois)
    {
    // Verifica se classe vai ser mudada
        if (mudar->Comandos==0)
            continue;
    // Retira da instrução herda as classes que não existem (foram apagadas)
        char * const instr = mudar->Comandos;
        if ((instr[0] || instr[1]) && instr[2]==Instr::cHerda)
        {
            char * o = instr+Instr::endVar+1, * d = instr+Instr::endVar+1;
            for (int x = (unsigned char)instr[Instr::endVar]; x; x--)
            {
                TClasse * c = TClasse::Procura(o);
                if (c)
                {
                    while (*o)
                        *d++ = *o++;
                    *d++ = *o++;
                    continue;
                }
#ifdef DEBUG
                printf("Retirando herda %s\n", o); fflush(stdout);
#endif
                instr[Instr::endVar]--;
                while (*o++);
            }
            if (o != d)
            {
                char * fim = instr;
                while (fim[0] || fim[1])
                    fim += Num16(fim);
                fim += 2;
                if (instr[Instr::endVar])
                {
                    memcpy(d, o, fim-o);
                    int x = Num16(instr) + d - o;
                    instr[0] = x;
                    instr[1] = x >> 8;
                }
                else
                    memcpy(instr, o, fim-o);
            }
        }
    // Altera a classe
        TClasse * cl = TClasse::Procura(mudar->Nome);
#ifdef DEBUG
        printf("Alterando %s\n", cl->Nome);
        for (const char * p = instr; Num16(p); p+=Num16(p))
        {
            char mens[2048];
            assert(Instr::Decod(mens, p, sizeof(mens)));
            printf("  %s\n", mens);
        }
        fflush(stdout);
#endif
        char * antigo_com = cl->Comandos;
        if (mudar->Arquivo)
            cl->MoveArquivo(mudar->Arquivo);
        cl->Comandos = instr;
        mudar->Comandos = 0;
        cl->AcertaDeriv(antigo_com);
        cl->AcertaVar(true);
        delete[] antigo_com;
    }
// Executa funções iniclasse das classes que foram criadas
    while (Inicio)
    {
        if ((Inicio->RBcolour & 4)==0)
        {
            delete Inicio;
            continue;
        }
        TClasse * cl = TClasse::Procura(Inicio->Nome);
        delete Inicio;
        if (cl==0)
            continue;
#ifdef DEBUG
        printf("iniclasse %s\n", cl->Nome); fflush(stdout);
#endif
        if (Instr::ExecIni(cl, "iniclasse"))
        {
            Instr::ExecArg(cl->Nome);
            Instr::ExecX();
            return true;
        }
    }
// Renomeia classes
    TRenomeiaClasse::Processa();
// Salva classes
    if (Salvar)
    {
        TArqMapa::SalvarArq(Salvar >= 2);
        Salvar=0;
    }
    return false;
}

//----------------------------------------------------------------------------
TMudarClasse * TMudarClasse::Procurar(const char * nome)
{
    TMudarClasse * y = RBroot;
    while (y)
    {
        int i = comparaVar(nome, y->Nome);
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
int TMudarClasse::RBcomp(TMudarClasse * x, TMudarClasse * y)
{
    return comparaVar(x->Nome, y->Nome);
}

//----------------------------------------------------------------------------
TMudarClasse * TMudarClasse::RBroot=0;
#define CLASS TMudarClasse          // Nome da classe
#define RBmask 1 // Máscara para bit 0
#include "rbt.cpp.h"

//----------------------------------------------------------------------------
TRenomeiaClasse::TRenomeiaClasse(const char * antes, const char * depois)
{
    copiastr(NomeAntes, antes, sizeof(NomeAntes));
    copiastr(NomeDepois, depois, sizeof(NomeDepois));
    Antes = Fim;
    Depois = 0;
    Fim = this;
    (Antes ? Antes->Depois : Inicio) = this;
}

//----------------------------------------------------------------------------
TRenomeiaClasse::~TRenomeiaClasse()
{
    (Antes ? Antes->Depois : Inicio) = Depois;
    (Depois ? Depois->Antes : Fim) = Antes;
}

//----------------------------------------------------------------------------
void TRenomeiaClasse::Processa()
{
    while (Inicio)
    {
        TClasse * cl = TClasse::Procura(Inicio->NomeAntes);
        if (cl && TClasse::Procura(Inicio->NomeDepois)==0)
            cl->MudaNome(Inicio->NomeDepois);
        delete Inicio;
    }
}
