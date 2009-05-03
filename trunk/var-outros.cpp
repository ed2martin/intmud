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
#include <errno.h>
#include <assert.h>
#include "variavel.h"
#include "instr.h"
#include "classe.h"
#include "objeto.h"
#include "var-outros.h"
#include "misc.h"

//#define DEBUG  // Mostrar e testar vetores de TVarIntTempo

TVarIntTempo ** TVarIntTempo::VetMenos = 0;
TVarIntTempo ** TVarIntTempo::VetMais = 0;
unsigned int TVarIntTempo::TempoMenos = 0;
unsigned int TVarIntTempo::TempoMais = 0;

//------------------------------------------------------------------------------
/*
void DebugRef()
{
 for (TClasse * cl = TClasse::RBfirst(); cl; cl=TClasse::RBnext(cl))
 {
  for (TObjeto * obj = cl->ObjetoIni; obj; obj=obj->Depois)
  {
   TVarRef * vantes = 0;
   for (TVarRef * v1 = obj->VarRefIni; v1; v1=v1->Depois)
   {
    assert(v1->Pont == obj);
    assert(v1->Antes == vantes);
    vantes = v1;
   }
  }
 }
}

//------------------------------------------------------------------------------
void MostraRef(TVarRef * r)
{
    printf("  this = %p\n", r);
    if (r==0)
        return;
    printf("  Antes = %p\n", r->Antes);
    printf("  Depois = %p\n", r->Depois);
    printf("  Pont = %p\n", r->Pont);
    if (r->Pont)
        printf("  Pont->VarRefIni = %p\n", r->Pont->VarRefIni);
    if (r->Antes)
        printf("  Antes->Depois = %p\n", r->Antes->Depois);
    if (r->Depois)
        printf("  Depois->Antes = %p\n", r->Depois->Antes);
    fflush(stdout);
}
*/

//------------------------------------------------------------------------------
void TVarRef::MudarPont(TObjeto * obj)
{
// Verifica se o endereço vai mudar
    if (obj == Pont)
        return;
    //printf("ANTES\n"); MostraRef(this); fflush(stdout);
// Retira da lista
    if (Pont)
    {
        (Antes ? Antes->Depois : Pont->VarRefIni) = Depois;
        if (Depois)
            Depois->Antes = Antes;
    }
// Coloca na lista
    if (obj)
    {
        Antes = 0;
        Depois = obj->VarRefIni;
        if (Depois)
            Depois->Antes = this;
        obj->VarRefIni = this;
    }
// Atualiza ponteiro
    Pont = obj;
    //printf("DEPOIS\n"); MostraRef(this); fflush(stdout);
}

//------------------------------------------------------------------------------
void TVarRef::Mover(TVarRef * destino)
{
    if (Pont)
    {
        (Antes ? Antes->Depois : Pont->VarRefIni) = destino;
        if (Depois)
            Depois->Antes = destino;
    }
    move_mem(destino, this, sizeof(TVarRef));
}

//------------------------------------------------------------------------------
int TVarIncDec::getInc()
{
    unsigned int v = TempoIni - valor;
    return (v>0xFFFF ? 0xFFFF : v);
}

//------------------------------------------------------------------------------
int TVarIncDec::getDec()
{
    unsigned int v = valor - TempoIni;
    return (v>0xFFFF ? 0 : v);
}

//------------------------------------------------------------------------------
void TVarIncDec::setInc(int v)
{
    if (v<0)  v=0;
    if (v>0xFFFF)  v=0xFFFF;
    valor = TempoIni - v;
}

//------------------------------------------------------------------------------
void TVarIncDec::setDec(int v)
{
    if (v<0)  v=0;
    if (v>0xFFFF)  v=0xFFFF;
    valor = TempoIni + v;
}

//------------------------------------------------------------------------------
void TVarIntTempo::PreparaIni()
{
    if (TVarIntTempo::VetMenos)
        return;
    VetMenos = new TVarIntTempo*[0x100];
    VetMais = new TVarIntTempo*[0x100];
    memset(VetMenos, 0, sizeof(TVarIntTempo*)*0x100);
    memset(VetMais, 0, sizeof(TVarIntTempo*)*0x100);
}

//------------------------------------------------------------------------------
int TVarIntTempo::TempoEspera()
{
#ifdef DEBUG
    DebugVet(true);
#endif
    int menos = TempoMenos;
    int total = 0;
    for (; menos<0x100 && total<10; menos++,total++)
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
        if (TempoMenos < 0xFF)
            TempoMenos++;
        else
        {
            TempoMenos = 0;
            TempoMais = (TempoMais + 1) & 0xFF;
            while (true)
            {
                TVarIntTempo * obj = VetMais[TempoMais];
                if (obj==0)
                    break;
            // Move objeto da lista ligada VetMais para VetMenos
                VetMais[TempoMais] = obj->Depois;
                int menos = obj->IndiceMenos;
                obj->Antes = 0;
                obj->Depois = VetMenos[menos];
                VetMenos[menos] = obj;
                if (obj->Depois)
                    obj->Depois->Antes = obj;
            }
        }
    // Chama eventos dos objetos em VetMenos[TempoMenos]
        while (true)
        {
            TVarIntTempo * obj = VetMenos[TempoMenos];
            if (obj==0)
                break;
        // Tira objeto da lista ligada
            VetMenos[TempoMenos] = obj->Depois;
            if (obj->Depois)
                obj->Depois->Antes = 0;
            obj->Depois = 0;
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
            if (prossegue==false)
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
int TVarIntTempo::getValor()
{
    if (Antes==0 && VetMenos[IndiceMenos]!=this &&
                    VetMais[IndiceMais]!=this)
        return 0;
    return (unsigned short)
        ((IndiceMais - TempoMais) * 0x100 + IndiceMenos - TempoMenos);
}

//------------------------------------------------------------------------------
void TVarIntTempo::setValor(int valor)
{
// Tira objeto das listas ligadas
#ifdef DEBUG
    DebugVet(false);
#endif
    if (Antes)
        Antes->Depois = Depois;
    else if (VetMenos[IndiceMenos]==this)
        VetMenos[IndiceMenos] = Depois;
    else if (VetMais[IndiceMais]==this)
        VetMais[IndiceMais] = Depois;
    if (Depois)
        Depois->Antes = Antes, Depois=0;
    Antes = 0;
#ifdef DEBUG
    DebugVet(false);
#endif
// Menos que zero: não está em nenhuma lista
    if (valor<=0)
        return;
// Máximo 0xFFFF
    if (valor > 0xFFFF)
        valor = 0xFFFF;
// Acerta IndiceMenos e IndiceMais
    valor += TempoMais * 0x100 + TempoMenos;
    IndiceMenos = valor;
    IndiceMais = valor / 0x100;
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
    if (Depois)
        Depois->Antes = destino;
    if (Antes)
        Antes->Depois = destino;
    else if (VetMenos[IndiceMenos]==this)
        VetMenos[IndiceMenos] = destino;
    else if (VetMais[IndiceMais]==this)
        VetMais[IndiceMais] = destino;
#ifdef DEBUG
    DebugVet(false);
#endif
    move_mem(destino, this, sizeof(TVarIntTempo));
}

//------------------------------------------------------------------------------
void TVarIntTempo::EndObjeto(TClasse * c, TObjeto * o)
{
    if (o)
        endobjeto=o, b_objeto=true;
    else
        endclasse=c, b_objeto=false;
}

//------------------------------------------------------------------------------
void TVarIntTempo::DebugVet(bool mostrar)
{
    if (mostrar)
        printf("VetMenos: ");
    for (int x=0; x<0x100; x++)
        if (VetMenos[x])
        {
            int y=0;
            TVarIntTempo * objini=0, * obj=VetMenos[x];
            for (; obj; obj=obj->Depois)
            {
                assert(obj->Antes == objini);
                objini = obj, y++;
            }
            if (mostrar)
                printf("[%d,%d]", x, y);
        }
    if (mostrar)
        printf("\nVetMais: ");
    for (int x=0; x<0x100; x++)
        if (VetMais[x])
        {
            int y=0;
            TVarIntTempo * objini=0, * obj=VetMais[x];
            for (; obj; obj=obj->Depois)
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
