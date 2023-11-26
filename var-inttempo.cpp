/* Este arquivo é software livre; você pode redistribuir e/ou
 * modificar nos termos da licença LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "variavel.h"
#include "variavel-def.h"
#include "instr.h"
#include "instr-enum.h"
#include "classe.h"
#include "objeto.h"
#include "var-inttempo.h"
#include "misc.h"

TVarIntTempo ** TVarIntTempo::VetMenos = nullptr;
TVarIntTempo ** TVarIntTempo::VetMais = nullptr;
unsigned int TVarIntTempo::TempoMenos = 0;
unsigned int TVarIntTempo::TempoMais = 0;

//#define DEBUG  // Mostrar e testar vetores de TVarIntTempo

//------------------------------------------------------------------------------
const TVarInfo * TVarIntTempo::Inicializa()
{
    if (TVarIntTempo::VetMenos == nullptr)
    {
        VetMenos = new TVarIntTempo*[INTTEMPO_MAX];
        VetMais = new TVarIntTempo*[INTTEMPO_MAX];
        memset(VetMenos, 0, sizeof(TVarIntTempo*) * INTTEMPO_MAX);
        memset(VetMais, 0, sizeof(TVarIntTempo*) * INTTEMPO_MAX);
    }
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        TVarInfo::FTipoInt,
        FRedim,
        FMoverEnd,
        FMoverDef,
        FGetBool,
        FGetInt,
        FGetDouble,
        FGetTxt,
        TVarInfo::FGetObjNulo,
        FOperadorAtrib,
        FuncVetor);
    return &var;
}

//------------------------------------------------------------------------------
int TVarIntTempo::TempoEspera()
{
#ifdef DEBUG
    DebugVet(true);
#endif
    int menos = TempoMenos;
    int total = 0;
    for (; menos < INTTEMPO_MAX && total < 600; menos++, total++)
        if (VetMenos[menos])
            return total;
    return total;
}

//------------------------------------------------------------------------------
void TVarIntTempo::ProcEventos(int TempoDecorrido)
{
#ifdef DEBUG
    printf("Tempo %d\n", TempoDecorrido); fflush(stdout);
#endif
    while (TempoDecorrido-- > 0)
    {
    // Avança TempoMenos
    // Move objetos de VetMais para VetMenos se necessário
        if (TempoMenos < INTTEMPO_MAX - 1)
            TempoMenos++;
        else
        {
            TempoMenos = 0;
            TempoMais = (TempoMais + 1) % INTTEMPO_MAX;
            while (true)
            {
                TVarIntTempo * obj = VetMais[TempoMais];
                if (obj == nullptr)
                    break;
            // Move objeto da lista ligada VetMais para VetMenos
                VetMais[TempoMais] = obj->Depois;
                int menos = obj->IndiceMenos;
                obj->Antes = nullptr;
                obj->Depois = VetMenos[menos];
                VetMenos[menos] = obj;
                if (obj->Depois)
                    obj->Depois->Antes = obj;
            }
        }
    // Inverte a ordem de VetMenos[TempoMenos]
    // Dessa forma, variáveis alteradas primeiro geram eventos primeiro
        TVarIntTempo * obj1 = VetMenos[TempoMenos];
        if (obj1)
        {
            while (true)
            {
                TVarIntTempo * x = obj1->Depois;
                obj1->Depois = obj1->Antes;
                obj1->Antes = x;
                if (x == nullptr)
                    break;
                obj1 = x;
            }
            VetMenos[TempoMenos] = obj1;
        }
    // Chama eventos dos objetos em VetMenos[TempoMenos]
        while (true)
        {
            TVarIntTempo * obj = VetMenos[TempoMenos];
            if (obj == nullptr)
                break;
        // Tira objeto da lista ligada
            VetMenos[TempoMenos] = obj->Depois;
            if (obj->Depois)
                obj->Depois->Antes = nullptr;
            obj->Depois = nullptr;
        // Gera evento
            bool prossegue = false;
            if (obj->b_objeto)
            {
                char mens[80];
                sprintf(mens, "%s_exec", obj->defvar + Instr::endNome);
                prossegue = Instr::ExecIni(obj->endobjeto, mens);
            }
            else if (obj->endclasse)
            {
                char mens[80];
                sprintf(mens, "%s_exec", obj->defvar + Instr::endNome);
                prossegue = Instr::ExecIni(obj->endclasse, mens);
            }
            if (prossegue == false)
                continue;
                // Cria argumento: índice
            Instr::ExecArg(obj->indice);
                // Executa
            Instr::ExecX();
            Instr::ExecFim();
        }
    }
}

//------------------------------------------------------------------------------
int TVarIntTempo::getValor(int numfunc)
{
    if (parado)
        return (numfunc ? -ValorParado : ValorParado);
    if (Antes == nullptr && VetMenos[IndiceMenos] != this &&
                    VetMais[IndiceMais] != this)
        return 0;
    int valor = ((IndiceMais - TempoMais) * INTTEMPO_MAX +
            IndiceMenos - TempoMenos);
    if (valor < 0)
        valor += INTTEMPO_MAX * INTTEMPO_MAX;
    return valor;
}

//------------------------------------------------------------------------------
void TVarIntTempo::setValor(int numfunc, int valor)
{
// Tira objeto das listas ligadas
#ifdef DEBUG
    DebugVet(false);
#endif
// Acerta o sinal se for função abs
    if (numfunc && (getValor(0) < 0) != (valor < 0))
        valor = -valor;
// Retira da lista
    if (!parado)
    {
        if (Antes)
            Antes->Depois = Depois;
        else if (VetMenos[IndiceMenos] == this)
            VetMenos[IndiceMenos] = Depois;
        else if (VetMais[IndiceMais] == this)
            VetMais[IndiceMais] = Depois;
        if (Depois)
            Depois->Antes = Antes, Depois = nullptr;
    }
    Antes = nullptr;
#ifdef DEBUG
    DebugVet(false);
#endif
// Valores negativos
    if (valor <= 0)
    {
        parado = true;
        if (valor <= -INTTEMPO_MAX * INTTEMPO_MAX)
            valor = -INTTEMPO_MAX * INTTEMPO_MAX + 1;
        ValorParado = valor;
        return;
    }
    parado = false;
// Valor máximo
    if (valor >= INTTEMPO_MAX * INTTEMPO_MAX)
        valor = INTTEMPO_MAX * INTTEMPO_MAX - 1;
// Acerta IndiceMenos e IndiceMais
    valor += TempoMais * INTTEMPO_MAX + TempoMenos;
    IndiceMenos = valor % INTTEMPO_MAX;
    IndiceMais = valor / INTTEMPO_MAX % INTTEMPO_MAX;
// Coloca na lista apropriada
    if (IndiceMais != TempoMais || IndiceMenos <= TempoMenos)
    {
        Depois = VetMais[IndiceMais];
        VetMais[IndiceMais] = this;
    }
    else
    {
        Depois = VetMenos[IndiceMenos];
        VetMenos[IndiceMenos] = this;
    }
    if (Depois)
        Depois->Antes = this;
#ifdef DEBUG
    DebugVet(false);
#endif
}

//------------------------------------------------------------------------------
void TVarIntTempo::Mover(TVarIntTempo * destino)
{
    if (!parado)
    {
        if (Depois)
            Depois->Antes = destino;
        if (Antes)
            Antes->Depois = destino;
        else if (VetMenos[IndiceMenos] == this)
            VetMenos[IndiceMenos] = destino;
        else if (VetMais[IndiceMais] == this)
            VetMais[IndiceMais] = destino;
#ifdef DEBUG
        DebugVet(false);
#endif
    }
    memmove(destino, this, sizeof(TVarIntTempo));
}

//------------------------------------------------------------------------------
void TVarIntTempo::EndObjeto(TClasse * c, TObjeto * o)
{
    if (o)
        endobjeto = o, b_objeto = true;
    else
        endclasse = c, b_objeto = false;
}

//------------------------------------------------------------------------------
bool TVarIntTempo::Func(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "abs") == 0)
    {
        Instr::ApagarVar(v + 1);
        Instr::VarAtual->numfunc = 1;
        return true;
    }
    if (comparaZ(nome, "pos") == 0)
    {
        int valor = getValor(0);
        if (valor < 0)
            setValor(0, -valor);
        return false;
    }
    if (comparaZ(nome, "neg") == 0)
    {
        int valor = getValor(0);
        if (valor > 0)
            setValor(0, -valor);
        return false;
    }
    return false;
}

//------------------------------------------------------------------------------
bool TVarIntTempo::FuncVetor(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar") != 0)
        return false;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    const int numero = (Instr::VarAtual <= v ? 0 : v[1].getInt());
    TVarIntTempo * ender = reinterpret_cast<TVarIntTempo*>(v->endvar);
    for (int x = 0; x < total; x++)
        ender[x].setValor(0, numero);
    return false;
}

//------------------------------------------------------------------------------
int TVarIntTempo::FTamanho(const char * instr)
{
    return sizeof(TVarIntTempo);
}

//------------------------------------------------------------------------------
int TVarIntTempo::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TVarIntTempo);
}

//------------------------------------------------------------------------------
void TVarIntTempo::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TVarIntTempo * ref = reinterpret_cast<TVarIntTempo*>(v->endvar);
    for (; antes < depois; antes++)
    {
        ref[antes].defvar = v->defvar;
        ref[antes].indice = antes;
        ref[antes].EndObjeto(c, o);
        ref[antes].parado = true;
        ref[antes].ValorParado = 0;
    }
    for (; depois < antes; depois++)
        if (!ref[depois].parado)
            ref[depois].setValor(0, 0);
}

//------------------------------------------------------------------------------
void TVarIntTempo::DebugVet(bool mostrar)
{
    if (mostrar)
        printf("VetMenos: ");
    for (int x = 0; x < INTTEMPO_MAX; x++)
        if (VetMenos[x])
        {
            int y = 0;
            TVarIntTempo * objini = nullptr, * obj=VetMenos[x];
            for (; obj; obj = obj->Depois)
            {
                assert(obj->Antes == objini);
                objini = obj, y++;
            }
            if (mostrar)
                printf("[%d,%d]", x, y);
        }
    if (mostrar)
        printf("\nVetMais: ");
    for (int x = 0; x < INTTEMPO_MAX; x++)
        if (VetMais[x])
        {
            int y = 0;
            TVarIntTempo * objini = nullptr, * obj=VetMais[x];
            for (; obj; obj = obj->Depois)
            {
                assert(obj->Antes == objini);
                objini = obj, y++;
            }
            if (mostrar)
                printf("[%d,%d]", x, y);
        }
    if (mostrar)
        { printf("\n"); fflush(stdout); }
}

//------------------------------------------------------------------------------
void TVarIntTempo::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    VARIAVEL_MOVER_COMPLETO(TVarIntTempo)
}

//------------------------------------------------------------------------------
void TVarIntTempo::FMoverDef(TVariavel * v)
{
    VARIAVEL_MOVERDEF(TVarIntTempo)
}

//------------------------------------------------------------------------------
bool TVarIntTempo::FGetBool(TVariavel * v) VARIAVEL_FGETINT1(TVarIntTempo)
int TVarIntTempo::FGetInt(TVariavel * v) VARIAVEL_FGETINT1(TVarIntTempo)
double TVarIntTempo::FGetDouble(TVariavel * v) VARIAVEL_FGETINT1(TVarIntTempo)
const char * TVarIntTempo::FGetTxt(TVariavel * v) VARIAVEL_FGETTXT1(TVarIntTempo)

//------------------------------------------------------------------------------
void TVarIntTempo::FOperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    TVarIntTempo * ref = reinterpret_cast<TVarIntTempo*>(v1->endvar) + v1->indice;
    ref->setValor(v1->numfunc, v2->getInt());
}
