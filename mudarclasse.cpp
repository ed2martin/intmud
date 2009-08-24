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
#include "mudarclasse.h"
#include "classe.h"
#include "objeto.h"
#include "instr.h"
#include "arqmapa.h"
#include "misc.h"

//#define SIMUL // Mostra o que vai mudar, mas não muda

TMudarClasse * TMudarClasse::Inicio=0;
TMudarClasse * TMudarClasse::Fim=0;
char TMudarClasse::Salvar=0;

//----------------------------------------------------------------------------
TMudarAux::TMudarAux()
{
    numbloco=0;
}

//------------------------------------------------------------------------------
void TMudarAux::AddBloco(char * ender, int tamanho)
{
    if (tamanho<=0 || numbloco >= sizeof(tambloco) / sizeof(tambloco[0]))
        return;
    endbloco[numbloco] = ender;
    tambloco[numbloco] = tamanho;
    numbloco++;
}

//------------------------------------------------------------------------------
bool TMudarAux::ChecaBloco(char * mensagem, int tamanho)
{
// Checa o tamanho do bloco
    int total=0;
    for (unsigned int bloco=0; bloco<numbloco; bloco++)
        total += tambloco[bloco];
    if (total > 65000)
    {
        copiastr(mensagem, "Classe muito grande", tamanho);
        return false;
    }
// Verifica se bloco válido
    int linha=1;
    Instr::ChecaLinha checalinha;
    checalinha.Inicio();
    for (unsigned int bloco=0; bloco<numbloco; bloco++)
    {
        char * com = endbloco[bloco];
        char * fim = com + tambloco[bloco];
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
            if (comparaZ(comando + Instr::endNome, nomevar)==0)
                return comando;
            comando += Num16(comando);
            break;
        default:
            if (inifunc==false && comando[2] > Instr::cVariaveis)
                if (comparaZ(comando + Instr::endNome, nomevar)==0)
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
int TMudarAux::CodifInstr(char * destino, const char * origem, int tamanho)
{
    char mens[2048];
    int linhanum = 1;
    char * linhaend = mens;
    int pdestino = 0;
    bool erro = false;

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
                sprintf(destino, "%d: Instrução muito grande", linhanum);
                return -1;
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
        if (!Instr::Codif(destino+pdestino, mens, tamanho-pdestino))
        {
            sprintf(mens, "%c%d: %s",
                    Instr::ex_barra_n, linhanum, destino+pdestino);
            if (erro==false)
            {
                pdestino = copiastr(destino, mens+1, tamanho) - destino;
                erro=true;
            }
            else
            {
                pdestino = copiastr(destino+pdestino, mens,
                                    tamanho-pdestino) - destino;
                if (pdestino < 10)
                    return -1;
            }
        }
        if (!erro)
            pdestino += Num16(destino+pdestino);
        if (ch==0)
            break;
        linhanum++;
    }
    return (erro ? -1 : pdestino);
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
    while (Inicio)
    {
    // Obtém a classe
        TClasse * cl = TClasse::Procura(Inicio->Nome);
    // Apagar classe
        if (Inicio->RBcolour & 2)
        {
        // Verifica se pode apagar
            if (cl==0 || cl->NumDeriv>0)
                Inicio->RBcolour &= ~2;
        // Verifica se tem objetos da classe
            else if (cl->ObjetoIni)
            {
                for (TObjeto * obj = cl->ObjetoIni; obj; obj=obj->Depois)
                    obj->MarcarApagar();
                return true;
            }
        // Apaga classe
            else
            {
                Inicio->RBcolour &= ~2;
                if (Inicio->Arquivo==0)
                    Inicio->Arquivo = cl->ArqArquivo;
#ifdef SIMUL
                printf("Apagar classe: %s\n", cl->Nome); fflush(stdout);
#else
                delete cl;
#endif
                cl=0;
            }
        }
    // Verifica se deve criar/alterar classe
        if (Inicio->Comandos==0)
        {
            delete Inicio;
            continue;
        }
    // Instrução herda: verifica se instrução ainda contém classes válidas
        if ((Inicio->Comandos[0] || Inicio->Comandos[1]) &&
            Inicio->Comandos[2]==Instr::cHerda)
        {
            const char * p = Inicio->Comandos + 4;
            for (unsigned char x = Inicio->Comandos[3]; x; x--)
            {
                if (TClasse::Procura(p)==0)
                {
                    p=0;
                    delete Inicio;
                    break;
                }
                while (*p++);
            }
            if (p==0)
                continue;
        }
    // Mostra alterações na classe
#ifdef SIMUL
        if (cl)
            printf("Mudar classe: %s\n", cl->Nome);
        else if (TClasse::NomeValido(Inicio->Nome))
            printf("Criar classe: %s\n", Inicio->Nome);
        else
        {
            printf("Nome inválido de classe: %s:\n", Inicio->Nome);
            delete Inicio;
            continue;
        }
        for (const char * p = Inicio->Comandos; Num16(p); p+=Num16(p))
        {
            char mens[2048];
            if (Instr::Decod(mens, p, sizeof(mens)))
                printf("  %s\n", mens);
            else
            {
                printf("  **** Erro\n");
                exit(EXIT_FAILURE);
            }
        }
        fflush(stdout);
        delete Inicio;
        continue;
#endif
    // Alterar classe
        if (cl)
        {
            char * antigo_com = cl->Comandos;
            if (Inicio->Arquivo)
                cl->Arquivo(Inicio->Arquivo);
            cl->Comandos = Inicio->Comandos;
            Inicio->Comandos = 0;
            delete Inicio;
            cl->AcertaDeriv();
            cl->AcertaVarSub();
            delete[] antigo_com;
            continue;
        }
    // Criar classe
        if (TClasse::NomeValido(Inicio->Nome))
        {
            char mens[2] = { 0, 0 };
            cl = new TClasse(Inicio->Nome, Inicio->Arquivo);
            cl->Comandos = Inicio->Comandos;
            Inicio->Comandos = 0;
            delete Inicio;
            cl->AcertaDeriv(mens);
            cl->AcertaVarSub();
            if (TClasse::ClInic)
                if (TClasse::RBcomp(cl, TClasse::ClInic) >= 0)
                    continue;
            if (Instr::ExecIni(cl, "iniclasse"))
            {
                Instr::ExecArg(cl->Nome);
                Instr::ExecX();
                return true;
            }
            continue;
        }
        delete Inicio;
    }
    // Salvar classes
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
int TMudarClasse::RBcomp(TMudarClasse * x, TMudarClasse * y)
{
    return comparaZ(x->Nome, y->Nome);
}

//----------------------------------------------------------------------------
TMudarClasse * TMudarClasse::RBroot=0;
#define CLASS TMudarClasse          // Nome da classe
#define RBmask 1 // Máscara para bit 0
#include "rbt.cpp.h"
