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
#include "var-arqmem.h"
#include "variavel.h"
#include "instr.h"
#include "misc.h"
#include "instr-enum.h"

//#define DEBUG  // Checar arqmem e usar blocos de tamanho pequeno

//----------------------------------------------------------------------------
#ifdef DEBUG
#define DEBUG1 Debug();
static const int tamanhobloco = 10;
#else
#define DEBUG1
static const int tamanhobloco = 1024;
#endif

//----------------------------------------------------------------------------
const TVarInfo * TVarArqMem::Inicializa()
{
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        TVarInfo::FTipoOutros,
        FRedim,
        FMoverEnd,
        TVarInfo::FMoverDef0,
        TVarInfo::FGetBoolFalse,
        TVarInfo::FGetInt0,
        TVarInfo::FGetDouble0,
        TVarInfo::FGetTxtVazio,
        TVarInfo::FGetObjNulo,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

//----------------------------------------------------------------------------
void TVarArqMem::Criar()
{
    Inicio = Fim = PosBloco = nullptr;
    PosByte = ArqByte = 0;
}

//----------------------------------------------------------------------------
void TVarArqMem::Apagar()
{
    while (Fim)
        ApagarBloco();
    DEBUG1
}

//----------------------------------------------------------------------------
bool TVarArqMem::CriarBloco()
{
    int tam = tamanhobloco, pos = 0;
    if (Fim)
    {
        tam = Fim->Tamanho;
        pos = Fim->Posicao + tam;
        if (tam < 16*tamanhobloco && pos >= tam * 4)
            tam *= 2;
    }
    if (pos + tam >= 0x7FFF0000)
        return false;
    char * p = new char[sizeof(TBloco) + tam];
    TBloco * bl = reinterpret_cast<TBloco*>(p);
    bl->Posicao = pos;
    bl->Tamanho = tam;
    bl->Antes = Fim;
    bl->Depois = nullptr;
    (Fim ? Fim->Depois : Inicio) = bl;
    Fim = bl;
    if (PosBloco == nullptr)
        PosBloco = bl;
    ArqByte = 0;
    return true;
}

//----------------------------------------------------------------------------
void TVarArqMem::ApagarBloco()
{
    TBloco * bl = Fim;
    if (bl == nullptr)
        return;
    (bl->Antes ? bl->Antes->Depois : Inicio) = nullptr;
    Fim = bl->Antes;
    if (PosBloco == bl)
        PosBloco = Fim;
    char * p = reinterpret_cast<char*>(bl);
    delete[] p;
}

//----------------------------------------------------------------------------
int TVarArqMem::Tamanho()
{
    return (Fim == nullptr ? 0 : Fim->Posicao + ArqByte);
}

//----------------------------------------------------------------------------
int TVarArqMem::Posicao()
{
    return (PosBloco == nullptr ? 0 : PosBloco->Posicao + PosByte);
}

//----------------------------------------------------------------------------
void TVarArqMem::Posicao(int novapos)
{
    if (Fim == nullptr) // Arquivo vazio
        return;
    if (novapos <= 0) // No início do arquivo
    {
        PosBloco = Inicio;
        PosByte = 0;
        DEBUG1
        return;
    }
    int posfim = Fim->Posicao;
    if (novapos >= posfim) // No último bloco do arquivo
    {
        PosBloco = Fim;
        novapos -= posfim;
        PosByte = (novapos <= ArqByte ? novapos : ArqByte);
        DEBUG1
        return;
    }
    TBloco * bl = PosBloco;
    if (novapos < bl->Posicao)
    {
        bl = bl->Antes;
        while (novapos < bl->Posicao)
            bl = bl->Antes;
    }
    else
        while (novapos >= bl->Posicao + bl->Tamanho)
            bl = bl->Depois;
    PosBloco = bl;
    PosByte = novapos - bl->Posicao;
    DEBUG1
}

//----------------------------------------------------------------------------
void TVarArqMem::TruncarZero()
{
    while (Fim)
        ApagarBloco();
    PosByte = ArqByte = 0;
}

//----------------------------------------------------------------------------
void TVarArqMem::TruncarPosicao()
{
    TBloco * bl = PosBloco;
    if (bl == nullptr) // Arquivo vazio
        return;
    if (PosByte > 0) // Posição não está no início do bloco
    {
        while (bl->Depois)
            ApagarBloco();
        ArqByte = PosByte;
        DEBUG1
        return;
    }
    bl = bl->Antes; // Vai para o fim do bloco anterior
    if (bl == nullptr) // Se não há bloco anterior: arquivo vazio
    {
         TruncarZero();
         return;
    }
    while (bl->Depois)
        ApagarBloco();
    PosBloco = bl;
    ArqByte = PosByte = bl->Tamanho;
    DEBUG1
}

//----------------------------------------------------------------------------
int TVarArqMem::Ler(char * buffer, int tamanho)
{
    if (tamanho <= 0)
        return 0;
    const char * bufini = buffer;
    TVarArqMem::TArqLer pont(this, false);
    while (true)
    {
        int t = pont.tamanho;
        if (t == 0)
        {
            pont.MudaPosicao(pont.buffer);
            DEBUG1
            return buffer - bufini;
        }
        else if (tamanho <= t)
        {
            memcpy(buffer, pont.buffer, tamanho);
            buffer += tamanho;
            pont.MudaPosicao(buffer);
            DEBUG1
            return buffer - bufini;
        }
        memcpy(buffer, pont.buffer, t);
        buffer += t;
        tamanho -= t;
        pont.Proximo();
    }
}

//----------------------------------------------------------------------------
int TVarArqMem::Escrever(char * buffer, int tamanho)
{
    if (tamanho <= 0)
        return 0;
    TBloco * bl = PosBloco;
    if (bl == nullptr)
    {
        CriarBloco();
        bl = PosBloco;
    }
    int pos = PosByte;
    const char * bufinicio = buffer;
// Copia bytes
    int copiar = bl->Tamanho - pos;
    while (copiar < tamanho)
    {
        memcpy(reinterpret_cast<char*>(bl) + sizeof(*bl) + pos, buffer, copiar);
        buffer += copiar;
        tamanho -= copiar;
        if (bl == Fim && !CriarBloco())
        {
            PosByte = ArqByte = bl->Tamanho;
            PosBloco = bl;
            DEBUG1
            return buffer - bufinicio;
        }
        bl = bl->Depois;
        copiar = bl->Tamanho;
        pos = 0;
    }
// Copia bytes do último bloco
    memcpy(reinterpret_cast<char*>(bl) + sizeof(*bl) + pos, buffer, tamanho);
    buffer += tamanho;
    pos += tamanho;
    if (bl != Fim) // Bloco intermediário: pode passar para o próximo
    {
        if (pos >= bl->Tamanho)
            bl = bl->Depois, pos = 0;
    }
    else if (ArqByte < pos) // Último bloco: acerta o tamanho do arquivo
        ArqByte = pos;
    PosBloco = bl;
    PosByte = pos;
    DEBUG1
    return buffer - bufinicio;
}

//------------------------------------------------------------------------------
bool TVarArqMem::Func(TVariavel * v, const char * nome)
{
// Lista das funções de varmem
// Deve obrigatoriamente estar em letras minúsculas e ordem alfabética
    static const struct {
        const char * Nome;
        bool (TVarArqMem::*Func)(TVariavel * v); } ExecFunc[] = {
        { "add",          &TVarArqMem::FuncAdd },
        { "addbin",       &TVarArqMem::FuncAddBin },
        { "eof",          &TVarArqMem::FuncEof },
        { "escr",         &TVarArqMem::FuncEscr },
        { "escrbin",      &TVarArqMem::FuncEscrBin },
        { "ler",          &TVarArqMem::FuncLer },
        { "lerbin",       &TVarArqMem::FuncLerBin },
        { "lerbinesp",    &TVarArqMem::FuncLerBinEsp },
        { "limpar",       &TVarArqMem::FuncLimpar },
        { "pos",          &TVarArqMem::FuncPos },
        { "tamanho",      &TVarArqMem::FuncTamanho },
        { "truncar",      &TVarArqMem::FuncTruncar }  };
// Procura a função correspondente e executa
    int ini = 0;
    int fim = sizeof(ExecFunc) / sizeof(ExecFunc[0]) - 1;
    char mens[80];
    copiastrmin(mens, nome, sizeof(mens));
    while (ini <= fim)
    {
        int meio = (ini + fim) / 2;
        int resultado = strcmp(mens, ExecFunc[meio].Nome);
        if (resultado == 0) // Se encontrou...
            return (this->*ExecFunc[meio].Func)(v);
        if (resultado < 0) fim = meio - 1; else ini = meio + 1;
    }
    return false;
}

//------------------------------------------------------------------------------
bool TVarArqMem::FuncAdd(TVariavel * v)
{
    PosBloco = Fim;
    PosByte = ArqByte;
    return FuncEscr(v);
}

//------------------------------------------------------------------------------
bool TVarArqMem::FuncAddBin(TVariavel * v)
{
    PosBloco = Fim;
    PosByte = ArqByte;
    return FuncEscrBin(v);
}

//------------------------------------------------------------------------------
bool TVarArqMem::FuncEof(TVariavel * v)
{
    int valor = (PosBloco == Fim && PosByte >= ArqByte);
    return Instr::CriarVarInt(v, valor);
}

//------------------------------------------------------------------------------
bool TVarArqMem::FuncEscr(TVariavel * v)
{
    for (TVariavel * v1 = v + 1; v1 <= Instr::VarAtual; v1++)
    {
        const char * txt = v1->getTxt();
        char mens[BUF_MENS];
        int pmens = 0;
        while (*txt)
        {
            if (pmens >= (int)sizeof(mens) - 2)
            {
                Escrever(mens, pmens);
                pmens = 0;
            }
            switch (*txt)
            {
            case Instr::ex_barra_b:
                txt++;
                break;
            case Instr::ex_barra_c:
                if ((txt[1] >= '0' && txt[1] <= '9') ||
                        (txt[1] >= 'A' && txt[1] <= 'J') ||
                        (txt[1] >= 'a' && txt[1] <= 'j'))
                    txt += 2;
                else
                    txt++;
                break;
            case Instr::ex_barra_d:
                if (txt[1] >= '0' && txt[1] <= '7')
                    txt += 2;
                else
                    txt++;
                break;
            case Instr::ex_barra_n:
                mens[pmens++] = '\n';
                txt++;
                break;
            default:
                mens[pmens++] = *txt++;
            }
        }
        if (pmens)
            Escrever(mens, pmens);
    }
    DEBUG1
    return false;
}

//------------------------------------------------------------------------------
bool TVarArqMem::FuncEscrBin(TVariavel * v)
{
    for (TVariavel * v1 = v + 1; v1 <= Instr::VarAtual; v1++)
    {
        const char * txt = v1->getTxt();
        char mens[BUF_MENS];
        int pmens = 0;
        int valor = 1;
        while (true)
        {
            unsigned char ch = *txt++;
            if (ch == 0)
                break;
            else if (ch >= '0' && ch <= '9')
                valor = valor * 16 + ch - '0';
            else if (ch >= 'A' && ch <= 'F')
                valor = valor * 16 + ch - 'A' + 10;
            else if (ch >= 'a' && ch <= 'f')
                valor = valor * 16 + ch - 'a' + 10;
            else
                continue;
            if (valor < 0x100)
                continue;
            mens[pmens++] = valor, valor = 1;
            if (pmens >= (int)sizeof(mens))
            {
                Escrever(mens, pmens);
                pmens = 0;
            }
        }
        if (pmens)
            Escrever(mens, pmens);
    }
    DEBUG1
    return false;
}

//------------------------------------------------------------------------------
bool TVarArqMem::FuncLer(TVariavel * v)
{
    char mens[BUF_MENS];
    int pmens = 0;
    TVarArqMem::TArqLer pont(this, false);
    if (Instr::VarAtual > v) // Ler o tamanho especificado
    {
        int total = v[1].getInt();
        if (total > (int)sizeof(mens))
            total = (int)sizeof(mens);
        for (; pmens < total && pont.tamanho; pont.Proximo())
        {
            const char * buf = pont.buffer;
            int tam = pont.tamanho;
            for (int pos = 0; pos < tam; pos++)
            {
                unsigned char lido = *buf++;
                if (lido == '\n')
                    mens[pmens++] = Instr::ex_barra_n;
                else if (lido >= ' ')
                    mens[pmens++] = lido;
                if (pmens >= total)
                {
                    pont.MudaPosicao(buf);
                    goto fim;
                }
            }
        }
    }
    else // Ler uma linha
    {
        bool cr = false;
        for (; pont.tamanho; pont.Proximo())
        {
            const char * buf = pont.buffer;
            int tam = pont.tamanho;
            for (int pos = 0; pos < tam; pos++)
            {
                unsigned char lido = *buf++;
                if (lido == '\n')
                {
                    pont.MudaPosicao(buf);
                    goto fim;
                }
                else if (cr)
                {
                    pont.MudaPosicao(buf - 1);
                    goto fim;
                }
                else if (lido != '\r')
                {
                    if (lido >= ' ')
                        mens[pmens++] = lido;
                    if (pmens >= (int)sizeof(mens))
                    {
                        pont.MudaPosicao(buf);
                        goto fim;
                    }
                }
                else
                    cr = true;
            }
        }
    }
    pont.MudaPosicao(pont.buffer);
fim:
    DEBUG1
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(mens, pmens);
}

//------------------------------------------------------------------------------
bool TVarArqMem::FuncLerBin(TVariavel * v)
{
    return FuncLerBinComum(v, false);
}

//------------------------------------------------------------------------------
bool TVarArqMem::FuncLerBinEsp(TVariavel * v)
{
    return FuncLerBinComum(v, true);
}

//------------------------------------------------------------------------------
bool TVarArqMem::FuncLerBinComum(TVariavel * v, bool espaco)
{
    char mens[BUF_MENS];
    int pmens = 0;
    TVarArqMem::TArqLer pont(this, false);
    if (Instr::VarAtual > v) // Ler o tamanho especificado
    {
        int total = v[1].getInt();
        int totalmens = (espaco ? sizeof(mens) / 3 : sizeof(mens) / 2);
        if (total > totalmens)
            total = totalmens;
        total *= (espaco ? 3 : 2);
        for (; pmens < total && pont.tamanho; pont.Proximo())
        {
            const char * buf = pont.buffer;
            int tam = pont.tamanho;
            for (int pos = 0; pos < tam; pos++)
            {
                unsigned char lido = *buf++;
                char ch = ((lido >> 4) & 15);
                if (espaco)
                    mens[pmens++] = ' ';
                mens[pmens++] = (ch < 10 ? ch + '0' : ch + 'a' - 10);
                ch = (lido & 15);
                mens[pmens++] = (ch < 10 ? ch + '0' : ch + 'a' - 10);
                if (pmens >= total)
                {
                    pont.MudaPosicao(buf);
                    goto fim;
                }
            }
        }
    }
    else // Ler uma linha
    {
        bool cr = false;
        for (; pont.tamanho; pont.Proximo())
        {
            const char * buf = pont.buffer;
            int tam = pont.tamanho;
            for (int pos = 0; pos < tam; pos++)
            {
                unsigned char lido = *buf++;
                char ch = ((lido >> 4) & 15);
                if (espaco)
                    mens[pmens++] = ' ';
                mens[pmens++] = (ch < 10 ? ch + '0' : ch + 'a' - 10);
                ch = (lido & 15);
                mens[pmens++] = (ch < 10 ? ch + '0' : ch + 'a' - 10);
                if (lido == '\n')
                {
                    pont.MudaPosicao(buf);
                    goto fim;
                }
                else if (cr)
                {
                    pont.MudaPosicao(buf - 1);
                    pmens -= (espaco ? 3 : 2);
                    goto fim;
                }
                else if (lido == '\r')
                    cr = true;
                if (pmens > (int)sizeof(mens) - 3)
                {
                    pont.MudaPosicao(buf);
                    goto fim;
                }
            }
        }
    }
    pont.MudaPosicao(pont.buffer);
fim:
    DEBUG1
    Instr::ApagarVar(v);
    int add = (espaco && pmens ? 1 : 0);
    return Instr::CriarVarTexto(mens + add, pmens - add);
}

//------------------------------------------------------------------------------
bool TVarArqMem::FuncLimpar(TVariavel * v)
{
    TruncarZero();
    return false;
}

//------------------------------------------------------------------------------
bool TVarArqMem::FuncPos(TVariavel * v)
{
    if (Instr::VarAtual >= v + 1)
        Posicao(v[1].getInt());
    int pos = Posicao();
    return Instr::CriarVarInt(v, pos);
}

//------------------------------------------------------------------------------
bool TVarArqMem::FuncTamanho(TVariavel * v)
{
    int valor = Tamanho();
    return Instr::CriarVarInt(v, valor);
}

//------------------------------------------------------------------------------
bool TVarArqMem::FuncTruncar(TVariavel * v)
{
    TruncarPosicao();
    return false;
}

//------------------------------------------------------------------------------
TVarArqMem::TArqLer::TArqLer(TVarArqMem * arqmem, bool inicio)
{
    this->arqmem = arqmem;
    if (arqmem->Inicio == nullptr) // Arquivo vazio
    {
        bloco = nullptr;
        buffer = nullptr;
        tamanho = 0;
    }
    else if (inicio) // A partir do começo do arquivo
    {
        bloco = arqmem->Inicio;
        buffer = reinterpret_cast<char*>(bloco) + sizeof(*bloco);
        tamanho = (bloco == arqmem->Fim ? arqmem->ArqByte : bloco->Tamanho);
    }
    else // A partir da posição atual
    {
        bloco = arqmem->PosBloco;
        buffer = reinterpret_cast<char*>(bloco) + sizeof(*bloco) + arqmem->PosByte;
        tamanho = (bloco == arqmem->Fim ? arqmem->ArqByte : bloco->Tamanho) -
                arqmem->PosByte;
    }
}

//------------------------------------------------------------------------------
void TVarArqMem::TArqLer::Proximo()
{
    if (bloco == nullptr)
        return;
    if ((bloco = bloco->Depois) != nullptr)
    {
        buffer = reinterpret_cast<char*>(bloco) + sizeof(*bloco);
        tamanho = (bloco == arqmem->Fim ? arqmem->ArqByte : bloco->Tamanho);
    }
    else
        tamanho = 0, buffer = nullptr;
}

//------------------------------------------------------------------------------
void TVarArqMem::TArqLer::MudaPosicao(const char * posicao)
{
    if (bloco == nullptr)
    {
        arqmem->PosBloco = arqmem->Fim;
        arqmem->PosByte = arqmem->ArqByte;
        return;
    }
    int tam = posicao - (reinterpret_cast<char*>(bloco) + sizeof(*bloco));
    if (tam <= 0)
        arqmem->PosBloco = bloco,
        arqmem->PosByte = 0;
    else if (bloco == arqmem->Fim)
        arqmem->PosBloco = bloco,
        arqmem->PosByte = (tam <= arqmem->ArqByte ? tam : arqmem->ArqByte);
    else if (tam >= bloco->Tamanho)
        arqmem->PosBloco = bloco->Depois,
        arqmem->PosByte = 0;
    else
        arqmem->PosBloco = bloco,
        arqmem->PosByte = tam;
}

//----------------------------------------------------------------------------
void TVarArqMem::Debug()
{
    if (Inicio == nullptr)
    {
        assert(Fim == nullptr);
        assert(PosBloco == nullptr);
        assert(PosByte == 0);
        assert(ArqByte == 0);
    }
    else
    {
        assert(PosByte >= 0);
        assert(ArqByte >= 0);
        assert(ArqByte <= Fim->Tamanho);
        if (PosBloco == Fim)
            assert(PosByte <= ArqByte);
        else
            assert(PosByte < PosBloco->Tamanho);
        TBloco * bl1 = nullptr, * bl2 = Inicio;
        while (bl2 != Fim)
        {
            assert(bl2->Antes == bl1);
            bl1 = bl2;
            bl2 = bl2->Depois;
        }
        assert(bl2->Depois == nullptr);
    }
}

//------------------------------------------------------------------------------
int TVarArqMem::FTamanho(const char * instr)
{
    return sizeof(TVarArqMem);
}

//------------------------------------------------------------------------------
int TVarArqMem::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TVarArqMem);
}

//------------------------------------------------------------------------------
void TVarArqMem::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TVarArqMem * ref = reinterpret_cast<TVarArqMem*>(v->endvar);
    for (; antes < depois; antes++)
        ref[antes].Criar();
    for (; depois < antes; depois++)
        ref[depois].Apagar();
}

//------------------------------------------------------------------------------
void TVarArqMem::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    int total = (unsigned char)v->defvar[Instr::endVetor];
    memmove(destino, v->endvar, (total ? total : 1) * sizeof(TVarArqMem));
}
