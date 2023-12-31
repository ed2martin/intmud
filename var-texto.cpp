/* Este arquivo é software livre; você pode redistribuir e/ou
 * modificar nos termos da licença LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "var-texto.h"
#include "variavel.h"
#include "variavel-def.h"
#include "instr.h"
#include "instr-enum.h"
#include "misc.h"

//#define DEBUG_MSG // Alocação de memória

//----------------------------------------------------------------------------
TTextoGrupo * TTextoGrupo::Disp = nullptr;
TTextoGrupo * TTextoGrupo::Usado = nullptr;
unsigned long TTextoGrupo::Tempo = 0;

//----------------------------------------------------------------------------
// Obtém o número de linhas correspondente a um texto
int TextoNumLin(const char * texto, int total)
{
    int linhas = 0;
    for (; total > 0; total--, texto++)
        if (*texto == Instr::ex_barra_n)
            linhas++;
    return linhas;
}

//----------------------------------------------------------------------------
// Copia texto e retorna número de linhas
int TextoAnotaLin(char * destino, const char * origem, int total)
{
    int linhas = 0;
    for (; total > 0; total--, origem++)
        if (*origem == 0 || *origem == Instr::ex_barra_n)
            *destino++ = Instr::ex_barra_n, linhas++;
        else
            *destino++ = *origem;
    return linhas;
}

//----------------------------------------------------------------------------
const TVarInfo * TTextoTxt::Inicializa()
{
    static TVarInfo::FuncItem ListaFuncEnd[] = {
        { "addfim",       &TTextoTxt::FuncAddFim },
        { "addini",       &TTextoTxt::FuncAddIni },
        { "bytes",        &TTextoTxt::FuncBytes },
        { "clipler",      &TTextoTxt::FuncClipLer },
        { "clipsalvar",   &TTextoTxt::FuncClipSalvar },
        { "dividelin",    &TTextoTxt::FuncDivideLin },
        { "dividelincor", &TTextoTxt::FuncDivideLinCor },
        { "fim",          &TTextoTxt::FuncFim },
        { "ini",          &TTextoTxt::FuncIni },
        { "juntalin",     &TTextoTxt::FuncJuntaLin },
        { "juntar",       &TTextoTxt::FuncJuntar },
        { "ler",          &TTextoTxt::FuncLer },
        { "limpar",       &TTextoTxt::FuncLimpar },
        { "linhas",       &TTextoTxt::FuncLinhas },
        { "ordena",       &TTextoTxt::FuncOrdena },
        { "ordenalin",    &TTextoTxt::FuncOrdenaLin },
        { "rand",         &TTextoTxt::FuncRand },
        { "remove",       &TTextoTxt::FuncRemove },
        { "salvar",       &TTextoTxt::FuncSalvar },
        { "txtremove",    &TTextoTxt::FuncTxtRemove }  };
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        TVarInfo::FTipoOutros,
        FRedim,
        FMoverEnd,
        TVarInfo::FMoverDef0,
        FGetBool,
        FGetInt,
        FGetDouble,
        FGetTxt,
        TVarInfo::FGetObjNulo,
        TVarInfo::FOperadorAtribVazio,
        TVarInfo::FOperadorAddFalse,
        TVarInfo::FOperadorIgual2Var,
        TVarInfo::FOperadorComparaVar,
        TVarInfo::FFuncTextoFalse,
        TVarInfo::FFuncVetorFalse,
        ListaFuncEnd,
        sizeof(ListaFuncEnd) / sizeof(ListaFuncEnd[0]) - 1);
    return &var;
}

//----------------------------------------------------------------------------
inline void TTextoTxt::Apagar()
{
// Otimização, no caso de precisar mover algum bloco que será apagado
    for (TTextoBloco * obj = Inicio; obj; obj = obj->Depois)
        obj->Bytes = 0;
// Acerta objetos TTextoPos
// Nota: mais eficiente que: while (Posic) Posic->MudarTxt(0);
    for (TTextoPos * obj = Posic; obj; obj = obj->Depois)
        obj->TextoTxt = nullptr, obj->Bloco = nullptr;
    Posic = nullptr;
// Apaga blocos
    while (Inicio)
        Inicio->Apagar();
}

//----------------------------------------------------------------------------
void TTextoTxt::Limpar()
{
// Otimização, no caso de precisar mover algum bloco que será apagado
    for (TTextoBloco * obj = Inicio; obj; obj = obj->Depois)
        obj->Bytes = 0;
// Acerta objetos TTextoPos
    for (TTextoPos * obj = Posic; obj; obj = obj->Depois)
    {
        obj->Bloco = nullptr;
        obj->PosicBloco = 0;
        obj->PosicTxt = 0;
        obj->LinhaTxt = 0;
    }
    Linhas = 0;
    Bytes = 0;
// Apaga blocos
    while (Inicio)
        Inicio->Apagar();
}

//----------------------------------------------------------------------------
inline void TTextoTxt::Mover(TTextoTxt * destino)
{
    for (TTextoPos * obj = Posic; obj; obj = obj->Depois)
        obj->TextoTxt = destino;
    for (TTextoBloco * obj = Inicio; obj; obj = obj->Depois)
        obj->TextoTxt = destino;
    memmove(destino, this, sizeof(TTextoTxt));
}

//----------------------------------------------------------------------------
void TTextoTxt::IniBloco()
{
    if (Inicio)
        return;
    TTextoBloco * bloco = TTextoGrupo::Criar();
    bloco->TextoTxt = this;
    bloco->Antes = nullptr;
    bloco->Depois = nullptr;
    bloco->Linhas = 0;
    bloco->Bytes = 0;
    Inicio = Fim = bloco;
    for (TTextoPos * obj = Posic; obj; obj = obj->Depois)
    {
        obj->Bloco = bloco;
        obj->PosicBloco = 0;
        obj->PosicTxt = 0;
        obj->LinhaTxt = 0;
    }
}

//----------------------------------------------------------------------------
void TTextoTxt::AddTexto(const char * texto, unsigned int tamtexto)
{
    const unsigned int bloco_tam = sizeof(TTextoBloco::Texto);
    if (Inicio == 0)
        IniBloco();
    Bytes += tamtexto;
    while (true)
    {
        unsigned int copiar = bloco_tam - Fim->Bytes;
        if (copiar > tamtexto)
            copiar = tamtexto, tamtexto = 0; // copiar todo o texto
        else
            tamtexto -= copiar; // copiar parte do texto
        int lin = TextoAnotaLin(Fim->Texto + Fim->Bytes, texto, copiar);
        Fim->Bytes += copiar;
        Fim->Linhas += lin;
        Linhas += lin;
        texto += copiar;
        if (tamtexto == 0)
            break;
        if (Fim->Depois)
            Fim = Fim->Depois;
        else
            Fim = Fim->CriarDepois();
    }
}

//------------------------------------------------------------------------------
int TTextoTxt::FTamanho(const char * instr)
{
    return sizeof(TTextoTxt);
}

//------------------------------------------------------------------------------
int TTextoTxt::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TTextoTxt);
}

//------------------------------------------------------------------------------
void TTextoTxt::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TTextoTxt * ref = reinterpret_cast<TTextoTxt*>(v->endvar);
    if (antes < depois)
        memset(ref + antes, 0, (depois - antes) * sizeof(ref[0]));
    for (; depois < antes; depois++)
        ref[depois].Apagar();
}

//------------------------------------------------------------------------------
void TTextoTxt::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    VARIAVEL_MOVER_SIMPLES(TTextoTxt)
}

//------------------------------------------------------------------------------
bool TTextoTxt::FGetBool(TVariavel * v) VARIAVEL_FGETINT0(TTextoTxt)
int TTextoTxt::FGetInt(TVariavel * v) VARIAVEL_FGETINT0(TTextoTxt)
double TTextoTxt::FGetDouble(TVariavel * v) VARIAVEL_FGETINT0(TTextoTxt)
const char * TTextoTxt::FGetTxt(TVariavel * v) VARIAVEL_FGETTXT0(TTextoTxt)

//----------------------------------------------------------------------------
const TVarInfo * TTextoPos::Inicializa()
{
    static TVarInfo::FuncItem ListaFuncEnd[] = {
        { "add",          &TTextoPos::FuncAdd },
        { "addpos",       &TTextoPos::FuncAddPos },
        { "antes",        &TTextoPos::FuncAntes },
        { "byte",         &TTextoPos::FuncByte },
        { "depois",       &TTextoPos::FuncDepois },
        { "juntar",       &TTextoPos::FuncJuntar },
        { "lin",          &TTextoPos::FuncLin },
        { "linha",        &TTextoPos::FuncLinha },
        { "mudar",        &TTextoPos::FuncMudar },
        { "remove",       &TTextoPos::FuncRemove },
        { "texto",        &TTextoPos::FuncTexto },
        { "textolin",     &TTextoPos::FuncTextoLin },
        { "txtmd5",       &TTextoPos::FuncMd5 },
        { "txtproc",      &TTextoPos::FuncTxtProc },
        { "txtprocdif",   &TTextoPos::FuncTxtProc },
        { "txtprocmai",   &TTextoPos::FuncTxtProc },
        { "txtsha1",      &TTextoPos::FuncSha1 } };
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        FTipo,
        FRedim,
        FMoverEnd,
        FMoverDef,
        FGetBool,
        FGetInt,
        FGetDouble,
        FGetTxt,
        TVarInfo::FGetObjNulo,
        FOperadorAtrib,
        TVarInfo::FOperadorAddFalse,
        FOperadorIgual2,
        FOperadorCompara,
        TVarInfo::FFuncTextoFalse,
        TVarInfo::FFuncVetorFalse,
        ListaFuncEnd,
        sizeof(ListaFuncEnd) / sizeof(ListaFuncEnd[0]) - 1);
    return &var;
}

//----------------------------------------------------------------------------
inline void TTextoPos::Apagar()
{
    if (TextoTxt == nullptr)
        return;
    (Antes ? Antes->Depois : TextoTxt->Posic) = Depois;
    if (Depois)
        Depois->Antes = Antes;
}

//----------------------------------------------------------------------------
inline void TTextoPos::Mover(TTextoPos * destino, TObjeto * o)
{
    Objeto = o;
    if (TextoTxt)
    {
        (Antes ? Antes->Depois : TextoTxt->Posic) = destino;
        if (Depois)
            Depois->Antes = destino;
    }
    memmove(destino, this, sizeof(TTextoPos));
}

//----------------------------------------------------------------------------
void TTextoPos::MudarTxt(TTextoTxt * obj)
{
// Verifica se o endereço vai mudar
    if (obj == TextoTxt)
        return;
// Retira da lista
    if (TextoTxt)
    {
        (Antes ? Antes->Depois : TextoTxt->Posic) = Depois;
        if (Depois)
            Depois->Antes = Antes;
    }
// Coloca na lista
    if (obj)
    {
        Antes = nullptr;
        Depois = obj->Posic;
        if (Depois)
            Depois->Antes = this;
        obj->Posic = this;
        //Bloco = TextoTxt->Inicio;
        //PosicBloco = 0;
        //PosicTxt = 0;
        //LinhaTxt = 0;
    }
// Atualiza ponteiro
    TextoTxt = obj;
    Bloco = nullptr;
}

//------------------------------------------------------------------------------
int TTextoPos::FTamanho(const char * instr)
{
    return sizeof(TTextoPos);
}

//------------------------------------------------------------------------------
int TTextoPos::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TTextoPos);
}

//------------------------------------------------------------------------------
void TTextoPos::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TTextoPos * ref = reinterpret_cast<TTextoPos*>(v->endvar);
    for (; antes < depois; antes++)
    {
        ref[antes].TextoTxt = nullptr;
        ref[antes].Objeto = o;
        ref[antes].defvar = v->defvar;
        ref[antes].indice = antes;
    }
    for (; depois < antes; depois++)
        ref[depois].Apagar();
}

//------------------------------------------------------------------------------
void TTextoPos::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    VARIAVEL_MOVER_OBJETO(TTextoPos)
}

//------------------------------------------------------------------------------
void TTextoPos::FMoverDef(TVariavel * v)
{
    VARIAVEL_MOVERDEF(TTextoPos)
}

//------------------------------------------------------------------------------
bool TTextoPos::FGetBool(TVariavel * v) VARIAVEL_FGETINT1(TTextoPos)
int TTextoPos::FGetInt(TVariavel * v) VARIAVEL_FGETINT1(TTextoPos)
double TTextoPos::FGetDouble(TVariavel * v) VARIAVEL_FGETINT1(TTextoPos)
const char * TTextoPos::FGetTxt(TVariavel * v) VARIAVEL_FGETTXT1(TTextoPos)

//----------------------------------------------------------------------------
void TTextoPos::FOperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    if (v1->defvar[2] != v2->defvar[2])
        return;
    TTextoPos * r1 = reinterpret_cast<TTextoPos*>(v1->endvar) + v1->indice;
    TTextoPos * r2 = reinterpret_cast<TTextoPos*>(v2->endvar) + v2->indice;
    if (r1 == r2)
        return;
    r1->MudarTxt(r2->TextoTxt);
    r1->Bloco = r2->Bloco;
    r1->PosicBloco = r2->PosicBloco;
    r1->PosicTxt = r2->PosicTxt;
    r1->LinhaTxt = r2->LinhaTxt;
}

//------------------------------------------------------------------------------
bool TTextoPos::FOperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    if (v1->defvar[2] != v2->defvar[2])
        return false;
    TTextoPos * ref1 = reinterpret_cast<TTextoPos*>(v1->endvar) + v1->indice;
    TTextoPos * ref2 = reinterpret_cast<TTextoPos*>(v2->endvar) + v2->indice;
    if (ref1->TextoTxt != ref2->TextoTxt)
        return false;
    return ref1->TextoTxt == nullptr || ref1->PosicTxt == ref2->PosicTxt;
}

//------------------------------------------------------------------------------
unsigned char TTextoPos::FOperadorCompara(TVariavel * v1, TVariavel * v2)
{
    if (v1->defvar[2] != v2->defvar[2])
        return 0;
    TTextoPos * ref1 = reinterpret_cast<TTextoPos*>(v1->endvar) + v1->indice;
    TTextoPos * ref2 = reinterpret_cast<TTextoPos*>(v2->endvar) + v2->indice;
    if (ref1->TextoTxt != ref2->TextoTxt)
        return ref1->TextoTxt < ref2->TextoTxt ? 1 : 4;
    if (ref1->TextoTxt == nullptr || ref1->PosicTxt == ref2->PosicTxt)
        return 2;
    return ref1->PosicTxt < ref2->PosicTxt ? 1 : 4;
}

//----------------------------------------------------------------------------
TTextoBloco * TTextoBloco::CriarAntes()
{
    TTextoBloco * obj = TTextoGrupo::Criar();
    obj->TextoTxt = TextoTxt;
    obj->Antes = Antes;
    obj->Depois = this;
    obj->Linhas = 0;
    obj->Bytes = 0;
    (Antes ? Antes->Depois : TextoTxt->Inicio) = obj;
    Antes = obj;
    return obj;
}

//----------------------------------------------------------------------------
TTextoBloco * TTextoBloco::CriarDepois()
{
    TTextoBloco * obj = TTextoGrupo::Criar();
    obj->TextoTxt = TextoTxt;
    obj->Antes = this;
    obj->Depois = Depois;
    obj->Linhas = 0;
    obj->Bytes = 0;
    (Depois ? Depois->Antes : TextoTxt->Fim) = obj;
    Depois = obj;
    return obj;
}

//----------------------------------------------------------------------------
TTextoBloco * TTextoBloco::Apagar()
{
// Acerta ponteiros
    (Antes ? Antes->Depois : TextoTxt->Inicio) = Depois;
    (Depois ? Depois->Antes : TextoTxt->Fim) = Antes;
    for (TTextoPos * obj = TextoTxt->Posic; obj; obj = obj->Depois)
        if (obj->Bloco == this)
            obj->Bloco = nullptr;
// Apaga objeto
    return TTextoGrupo::Apagar(this);
}

//----------------------------------------------------------------------------
void TTextoBloco::Mover(TTextoBloco * destino)
{
// Acerta ponteiros
    (Antes ? Antes->Depois : TextoTxt->Inicio) = destino;
    (Depois ? Depois->Antes : TextoTxt->Fim) = destino;
    for (TTextoPos * obj = TextoTxt->Posic; obj; obj = obj->Depois)
        if (obj->Bloco == this)
            obj->Bloco = destino;
// Move na memória
    int total = Texto + Bytes - (char*)this;
    assert(total <= (int)sizeof(TTextoBloco));
    memcpy(destino, this, total);
}

//----------------------------------------------------------------------------
int TTextoBloco::LinhasBytes(unsigned int posic, int numlinhas)
{
    TTextoBloco * obj = this;
    int total = 0;
    if (numlinhas <= 0)
        return 0;
// Avança até o início de algum bloco
    if (posic > 0)
    {
        if (posic<Bytes)
        {
            const int max = obj->Bytes;
            for (int x = posic; x < max; x++)
                if (obj->Texto[x] == Instr::ex_barra_n)
                    if (--numlinhas == 0)
                        return x+1-posic;
            total = max - posic;
        }
        obj = Depois;
        if (obj == nullptr)
            return total;
    }
// Avança blocos inteiros
    while (numlinhas > obj->Linhas)
    {
        numlinhas -= obj->Linhas;
        total += obj->Bytes;
        obj = obj->Depois;
        if (obj == nullptr)
            return total;
    }
// Avança em um bloco
    int max = obj->Bytes;
    for (int x = 0; x < max; x++)
        if (obj->Texto[x] == Instr::ex_barra_n)
            if (--numlinhas == 0)
                return total + x + 1;
    return total + max;
}

//----------------------------------------------------------------------------
int TTextoBloco::CopiarTxt(unsigned int posic, char * buffer, int tambuf)
{
    const char * bufini = buffer;
    if (tambuf <= 1)
    {
        *buffer = 0;
        return 1;
    }
    tambuf--;
    TTextoBloco * obj = this;
// Acerta posic no início ou no final do bloco
    if (posic >= Bytes)
    {
        obj = Depois, posic = 0;
        if (obj == 0)
        {
            *buffer=0;
            return 1;
        }
    }
// Copia bytes
    while (tambuf > (int)obj->Bytes - (int)posic)
    {
        int total = obj->Bytes - posic;
        memcpy(buffer, obj->Texto + posic, total);
        buffer += total;
        tambuf -= total;
        obj = obj->Depois, posic = 0;
        if (obj == nullptr)
        {
            *buffer = 0;
            return buffer - bufini + 1;
        }
    }
// Copia bytes do último bloco
    if (tambuf > 0)
    {
        memcpy(buffer, obj->Texto + posic, tambuf);
        buffer += tambuf;
    }
    *buffer = 0;
    return buffer - bufini + 1;
}

//----------------------------------------------------------------------------
void TBlocoPos::MoverPos(int numlinhas)
{
    if (Bloco == nullptr)
        return;
// Avançar
    if (numlinhas > 0)
    {
    // Avança até o início de algum bloco
        if (PosicBloco > 0)
        {
            if (PosicBloco < Bloco->Bytes)
            {
                int x = PosicBloco;
                int lin = numlinhas;
                const int max = Bloco->Bytes;
                while (x < max)
                    if (Bloco->Texto[x++] == Instr::ex_barra_n)
                        if (--numlinhas == 0)
                            break;
                LinhaTxt += lin - numlinhas;
                PosicTxt += x - PosicBloco;
                PosicBloco = x;
            }
            if (Bloco->Depois == nullptr || numlinhas == 0)
                return;
            Bloco = Bloco->Depois;
        }
    // Avança blocos inteiros
        while (true)
        {
            const int lin = Bloco->Linhas;
            if (numlinhas <= lin)
                break;
            numlinhas -= lin;
            LinhaTxt += lin;
            PosicTxt += Bloco->Bytes;
            if (Bloco->Depois)
                Bloco = Bloco->Depois;
            else
            {
                PosicBloco = Bloco->Bytes;
                return;
            }
        }
    // Avança em um bloco
        int x = 0;
        int lin = numlinhas;
        const int max = Bloco->Bytes;
        while (x<max)
            if (Bloco->Texto[x++] == Instr::ex_barra_n)
                if (--numlinhas == 0)
                    break;
        LinhaTxt += lin - numlinhas;
        PosicTxt += x;
        PosicBloco = x;
        return;
    }
// Recuar
    else
    {
        numlinhas = 1 - numlinhas;
    // Recua até o fim de algum bloco
        if (PosicBloco<Bloco->Bytes)
        {
            if (PosicBloco > 0)
            {
                int x = PosicBloco;
                int lin = numlinhas;
                while (x)
                    if (Bloco->Texto[--x] == Instr::ex_barra_n)
                        if (--numlinhas == 0)
                        {
                            x++, numlinhas++;
                            LinhaTxt -= lin - numlinhas;
                            PosicTxt -= PosicBloco - x;
                            PosicBloco = x;
                            return;
                        }
                LinhaTxt -= lin - numlinhas;
                PosicTxt -= PosicBloco;
                PosicBloco = 0;
            }
            if (Bloco->Antes == nullptr)
                return;
            Bloco = Bloco->Antes;
        }
    // Recua blocos inteiros
        while (true)
        {
            const int lin = Bloco->Linhas;
            if (numlinhas <= lin)
                break;
            numlinhas -= lin;
            LinhaTxt -= lin;
            PosicTxt -= Bloco->Bytes;
            if (Bloco->Antes)
                Bloco = Bloco->Antes;
            else
            {
                PosicBloco = 0;
                return;
            }
        }
    // Recua em um bloco
        int x = Bloco->Bytes;
        int lin = numlinhas;
        while (x)
            if (Bloco->Texto[--x] == Instr::ex_barra_n)
                if (--numlinhas == 0)
                {
                    x++, numlinhas++;
                    LinhaTxt -= lin - numlinhas;
                    PosicTxt -= Bloco->Bytes - x;
                    PosicBloco = x;
                    return;
                }
        LinhaTxt -= lin - numlinhas;
        PosicTxt -= Bloco->Bytes;
        PosicBloco = 0;
        return;
    }
}

//----------------------------------------------------------------------------
void TBlocoPos::Mudar(const char * texto, unsigned int tamtexto,
        unsigned int tamapagar)
{
    const unsigned int bloco_tam = sizeof(TTextoBloco::Texto);
    int add_bytes = tamtexto;   // Quantidade de bytes inseridos
    int sub_bytes = 0;          // Quantidade de bytes removidos
    int dif_linhas = 0;         // = linhas inseridas - linhas removidas
    TTextoTxt * TextoTxt = Bloco->TextoTxt;
    TTextoBloco * obj;          // Bloco atual
    int posic;                  // Posição no bloco atual

// Acerta posic no início ou no final do bloco
// Nota: PosicBloco só será 0 se estiver no começo do texto
    if (PosicBloco > Bloco->Bytes)
        PosicBloco = Bloco->Bytes;
    else if (PosicBloco == 0 && Bloco->Antes)
        Bloco = Bloco->Antes, PosicBloco = Bloco->Bytes;
    posic = PosicBloco;
    obj = Bloco;

// Limpa blocos; preenche com texto se possível
    while (true)
    {
        const unsigned int max = obj->Bytes;
        if (tamapagar + posic < max)
            break;
    // Apaga bytes do bloco a partir de posic
        tamapagar -= max - posic;
        if (posic == 0)
        {
            sub_bytes += max;
            dif_linhas -= obj->Linhas;
            obj->Bytes = 0, obj->Linhas = 0;
        }
        else
        {
            int lin = TextoNumLin(obj->Texto + posic, max - posic);
            sub_bytes += max - posic;
            dif_linhas -= lin;
            obj->Bytes = posic;
            obj->Linhas -= lin;
        }
    // Anota texto no bloco a partir de posic
        if (tamtexto)
        {
            unsigned int copiar = bloco_tam - posic;
                                // = quantos bytes cabem no bloco
            if (copiar > tamtexto)
                copiar = tamtexto, tamtexto = 0; // copiar todo o texto
            else
                tamtexto -= copiar; // copiar parte do texto
            int lin = TextoAnotaLin(obj->Texto + posic, texto, copiar);
            obj->Bytes += copiar;
            obj->Linhas += lin;
            dif_linhas += lin;
            texto += copiar;
        }
    // Passa para o próximo bloco
        if (obj->Depois)
        {
            posic = 0;
            obj = obj->Depois;
            continue;
        }
    // Encontrou o fim do texto (não há próximo bloco)
        tamapagar = 0;
        posic = obj->Bytes;
        break;
    }

// Apaga bytes do bloco
// Nota: tamapagar < obj->Bytes - posic
    if (tamapagar)
    {
        char * p = obj->Texto + posic;
        int lin = TextoNumLin(p, tamapagar);
        obj->Bytes -= tamapagar;
        obj->Linhas -= lin;
        sub_bytes += tamapagar;
        dif_linhas -= lin;
        memmove(p, p+tamapagar, obj->Bytes-posic);
        //tamapagar = 0;
    }

// Inserir bytes
    while (tamtexto)
    {
    // Apenas insere bytes no bloco se couber
        if (tamtexto + obj->Bytes <= bloco_tam)
        {
            char * p = obj->Texto + posic;
            memmove(p + tamtexto, p, obj->Bytes - posic);
            int lin = TextoAnotaLin(p, texto, tamtexto);
            obj->Bytes += tamtexto;
            obj->Linhas += lin;
            dif_linhas += lin;
            // tamtexto = 0;
            break;
        }
    // Separa final do bloco
        char buffer[0x100];
        unsigned int tambuf = obj->Bytes - posic;
        int  linhabuf = TextoAnotaLin(buffer, obj->Texto + posic, tambuf);
        obj->Linhas -= linhabuf;
        obj->Bytes = posic;
    // Insere bytes
        while (true)
        {
            unsigned int copiar = bloco_tam - obj->Bytes;
            if (copiar > tamtexto)
                copiar = tamtexto, tamtexto = 0; // copiar todo o texto
            else
                tamtexto -= copiar; // copiar parte do texto
            int lin = TextoAnotaLin(obj->Texto + obj->Bytes, texto, copiar);
            obj->Bytes += copiar;
            obj->Linhas += lin;
            dif_linhas += lin;
            texto += copiar;
            if (tamtexto == 0)
                break;
            obj = obj->CriarDepois();
        }
    // Insere final do bloco
        if (tambuf)
        {
            if (obj->Bytes + tambuf > bloco_tam)
                obj = obj->CriarDepois();
            memcpy(obj->Texto + obj->Bytes, buffer, tambuf);
            obj->Bytes += tambuf;
            obj->Linhas += linhabuf;
        }
        break;
    } // while (tamtexto)

// Apaga objetos vazios
    if (obj->Bytes && obj->Antes)
        obj = obj->Antes;
    while (obj->Bytes == 0)
    {
        TTextoBloco * outro = obj->Antes;
        if (outro == nullptr)
            outro = obj->Depois;
        if (outro != obj->Apagar())
            obj = outro;
        if (obj)
            continue;
    // Texto ficou vazio
        for (TTextoPos * pos = TextoTxt->Posic; pos; pos = pos->Depois)
        {
            pos->Bloco = nullptr;
            pos->PosicBloco = 0;
            pos->PosicTxt = 0;
            pos->LinhaTxt = 0;
        }
        TextoTxt->Bytes = 0;
        TextoTxt->Linhas = 0;
        return;
    }

// Acerta tamanho do texto
    TextoTxt->Bytes += add_bytes - sub_bytes;
    TextoTxt->Linhas += dif_linhas;

// Se PosicBloco==0, está no início do texto
// Caso contrário, objeto Bloco não foi apagado porque não estava vazio
    if (PosicBloco == 0)
        Bloco = TextoTxt->Inicio, PosicTxt = 0, LinhaTxt = 0;

// Obtém bloco/posição após o texto adicionado
// Posição no arquivo = PosicTxt + add_bytes
    obj = Bloco;
    posic = PosicBloco + add_bytes;
    while (posic > obj->Bytes)
        posic -= obj->Bytes, obj = obj->Depois;

// Acerta objetos TTextoPos
    for (TTextoPos * pos = TextoTxt->Posic; pos; pos = pos->Depois)
    {
    // Antes do texto modificado: nada faz
        if (pos->PosicTxt < PosicTxt)
            continue;

    // Na região do texto apagado: move para o início da modificação
        if (pos->PosicTxt <= PosicTxt + sub_bytes)
        {
            pos->Bloco = Bloco;
            pos->PosicBloco = PosicBloco;
            pos->PosicTxt = PosicTxt;
            pos->LinhaTxt = LinhaTxt;
            continue;
        }

    // Após a região do texto modificado: acerta posição no texto
        pos->PosicTxt += add_bytes - sub_bytes;
        pos->LinhaTxt += dif_linhas;

    // Verifica se deve acertar bloco e posição no bloco
    // Nota: para pos->PosicTxt == PosicTxt+add_bytes, mais=0
        unsigned int mais = pos->PosicTxt - PosicTxt - add_bytes;
        if (mais > bloco_tam)
            continue;

    // Acerta bloco e posição no bloco
        pos->Bloco = obj;
        mais += posic;
        while (mais > pos->Bloco->Bytes)
            mais -= pos->Bloco->Bytes, pos->Bloco = pos->Bloco->Depois;
        pos->PosicBloco = mais;
    }

// Obtém objeto inicial e número de bytes sobrando
    TTextoBloco * ini = obj;
    int sobrando = bloco_tam - ini->Bytes;
    if (ini->Antes)
    {
        ini=ini->Antes, sobrando += bloco_tam - ini->Bytes;
        if (ini->Antes)
            ini=ini->Antes, sobrando += bloco_tam - ini->Bytes;
    }
    if (obj->Depois)
    {
        obj = obj->Depois, sobrando += bloco_tam - obj->Bytes;
        if (obj->Depois)
            obj = obj->Depois, sobrando += bloco_tam - obj->Bytes;
    }

// Verifica se consegue apagar algum objeto
    if (sobrando < (int)bloco_tam)
        return;

// Compacta o texto, apagando algum objeto
    while (ini->Bytes == bloco_tam)
        ini = ini->Depois;
    while (true)
    {
        TTextoBloco * proximo = ini->Depois;
    // Obtém número de bytes disponíveis em ini
        unsigned int total = bloco_tam - ini->Bytes;
    // Se pode copiar o bloco inteiro...
        if (total >= proximo->Bytes)
        {
        // Acerta objetos TTextoPos
            for (TTextoPos * pos = TextoTxt->Posic; pos; pos = pos->Depois)
                if (pos->Bloco == proximo)
                {
                    pos->Bloco = ini;
                    pos->PosicBloco += ini->Bytes;
                }
        // Copia bloco
            memcpy(ini->Texto + ini->Bytes, proximo->Texto, proximo->Bytes);
            ini->Bytes += proximo->Bytes;
            ini->Linhas += proximo->Linhas;
        // Apaga objeto
            proximo->Apagar();
            return;
        }
    // Acerta objetos TTextoPos
        for (TTextoPos * pos = TextoTxt->Posic; pos; pos = pos->Depois)
            if (pos->Bloco == proximo)
            {
                if (pos->PosicBloco < total)
                    pos->PosicBloco += ini->Bytes, pos->Bloco = ini;
                else
                    pos->PosicBloco -= total;
            }
    // Copia parte do bloco
        int linhas = TextoNumLin(proximo->Texto, total);
        memcpy(ini->Texto + ini->Bytes, proximo->Texto, total);
        memmove(proximo->Texto, proximo->Texto + total, proximo->Bytes - total);
        ini->Bytes += total;
        ini->Linhas += linhas;
        proximo->Bytes -= total;
        proximo->Linhas -= linhas;
    // Passa para o próximo bloco
        ini = proximo;
    }
}

//----------------------------------------------------------------------------
TTextoBloco * TTextoGrupo::Criar()
{
#ifdef DEBUG_MSG
    printf("TTextoBloco::Criar\n"); fflush(stdout);
#endif
// Se tem objeto TListaX disponível...
    if (Usado && Usado->Total < TOTAL_TEXTOTXTX)
        return Usado->Lista + (Usado->Total++);
// Se não tem objeto disponível...
    TTextoGrupo * obj;
    if (Disp == nullptr)    // Não tem objeto TTextoGrupo disponível
    {
        obj = new TTextoGrupo;
#ifdef DEBUG_MSG
        printf("  TTextoGrupo::Criar(%p)\n", obj); fflush(stdout);
#endif
    }
    else            // Tem objeto TTextoGrupo disponível
        obj=Disp, Disp = Disp->Depois; // Retira da lista Disp
    obj->Total = 1;
    obj->Depois = Usado; // Coloca na lista Usado
    Usado = obj;
    return obj->Lista;
}

//----------------------------------------------------------------------------
TTextoBloco * TTextoGrupo::Apagar(TTextoBloco * lista)
{
#ifdef DEBUG_MSG
    printf("TTextoBloco::Apagar\n"); fflush(stdout);
#endif
    TTextoBloco * lfim = Usado->Lista + Usado->Total - 1;
    if (lista != lfim)
        lfim->Mover(lista);
    Usado->Total--;
    if (Usado->Total == 0)
    {
        if (Disp == nullptr)
            Tempo = TempoIni;
        TTextoGrupo * obj = Usado;
        Usado = Usado->Depois; // Retira da lista Usado
        obj->Depois = Disp;    // Coloca na lista Disp
        Disp = obj;
    }
    return lfim;
}

//----------------------------------------------------------------------------
void TTextoGrupo::ProcEventos()
{
    if (Disp && Tempo + 10 < TempoIni)
    {
        TTextoGrupo * obj = Disp;
        Disp = Disp->Depois; // Retira da lista Disp
#ifdef DEBUG_MSG
        printf("  TTextoGrupo::Apagar(%p)\n", obj); fflush(stdout);
#endif
        delete obj;
        Tempo = TempoIni;
    }
}
