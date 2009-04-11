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

#include "var-listaobj.h"
#include "variavel.h"
#include "objeto.h"
#include "instr.h"
#include "misc.h"

//----------------------------------------------------------------------------
//class GrupoX /// Um grupo de objetos TListaX
//{
//public:
//    int total; ///< Número de objetos usados
//    static GrupoX * disp;///< Variável GrupoX com TListaX disponível
//    static GrupoX * fim; ///< Última variável GrupoX, ou 0 se não houver
//    GrupoX * anterior;   ///< Variável GrupoX anterior, ou 0 se não houver
//    TListaX lista[1024];
//};

const char ListaItem1[] = { 7, 0, Instr::cListaItem, 0, 0, 0, '+', 0 };

//----------------------------------------------------------------------------
void TListaObj::Apagar()
{
    while (Inicio)
    {
        Inicio->Objeto->MarcarApagar();
        Inicio->Apagar();
    }
}

//----------------------------------------------------------------------------
void TListaObj::Mover(TListaObj * destino)
{
    // Acerta TListaX::Lista em todos os TListaX da lista
    for (TListaX * obj = Inicio; obj; obj=obj->ListaDepois)
        obj->Lista = destino;
}

//----------------------------------------------------------------------------
int TListaObj::getValor()
{
    return (Inicio!=0);
}

//----------------------------------------------------------------------------
bool TListaObj::Func(TVariavel * v, const char * nome)
{
// Adiciona objetos no topo da lista
    if (comparaZ(nome, "add1")==0)
    {
        for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
        {
            TObjeto * obj = v1->getObj();
            if (obj==0)
                continue;
        // Cria objeto
            TListaX * l1 = TListaX::Criar();
        // Acrescenta no topo da lista
            l1->Lista = this;
            l1->ListaAntes = 0;
            l1->ListaDepois = Inicio;
            if (Inicio)
                Inicio->ListaAntes = l1;
            Inicio = l1;
            if (Fim==0)
                Fim = l1;
        // Acrescenta objeto
            l1->Objeto = obj;
            l1->ObjAntes = 0;
            l1->ObjDepois = obj->VarListaX;
            if (obj->VarListaX)
                obj->VarListaX->ObjAntes = l1;
            obj->VarListaX = l1;
        // Acerta ListaItem
            l1->ListaItem = 0;
        }
        return false;
    }
// Adiciona objetos no final da lista
    if (comparaZ(nome, "add2")==0)
    {
        for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
        {
            TObjeto * obj = v1->getObj();
            if (obj==0)
                continue;
        // Cria objeto
            TListaX * l1 = TListaX::Criar();
        // Acrescenta no final da lista
            l1->Lista = this;
            l1->ListaAntes = Fim;
            l1->ListaDepois = 0;
            if (Fim)
                Fim->ListaDepois = l1;
            Fim = l1;
            if (Inicio==0)
                Inicio = l1;
        // Acrescenta objeto
            l1->Objeto = obj;
            l1->ObjAntes = 0;
            l1->ObjDepois = obj->VarListaX;
            if (obj->VarListaX)
                obj->VarListaX->ObjAntes = l1;
            obj->VarListaX = l1;
        // Acerta ListaItem
            l1->ListaItem = 0;
        }
        return false;
    }
// Primeiro item da lista
    if (comparaZ(nome, "ini")==0)
    {
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(ListaItem1))
            return false;
        Instr::VarAtual->end_listaitem->MudarRef(Inicio);
        return true;
    }
// Último item da lista
    if (comparaZ(nome, "fim")==0)
    {
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(ListaItem1))
            return false;
        Instr::VarAtual->end_listaitem->MudarRef(Fim);
        return true;
    }
// Remove todos os objetos
    if (comparaZ(nome, "limpar")==0)
    {
        while (Inicio)
            Inicio->Apagar();
        return false;
    }
// Remove objeto da lista
    if (comparaZ(nome, "remover")==0)
    {
        for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
        {
            TObjeto * obj = v1->getObj();
            if (obj==0)
                continue;
            for (TListaX * l1 = Inicio; l1; l1=l1->ListaDepois)
                if (l1->Objeto==obj)
                {
                    l1->Apagar();
                    break;
                }
        }
        return false;
    }
// Verifica se objeto está na lista
    if (comparaZ(nome, "possui")==0)
    {
        for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
        {
            TObjeto * obj = v1->getObj();
            if (obj==0)
                continue;
            for (TListaX * l1 = Inicio; l1; l1=l1->ListaDepois)
                if (l1->Objeto==obj)
                {
                    Instr::ApagarVar(v);
                    if (!Instr::CriarVar(Instr::InstrVarInt))
                        return false;
                    Instr::VarAtual->setInt(1);
                    return true;
                }
        }
        return false;
    }
    return false;
}

//----------------------------------------------------------------------------
void TListaItem::Apagar()
{
    if (ListaX)
    {
        (Antes ? Antes->Depois : ListaX->ListaItem) = Depois;
        if (Depois)
            Depois->Antes = Antes;
        ListaX = 0;
    }
}

//----------------------------------------------------------------------------
void TListaItem::Mover(TListaItem * destino)
{
    if (ListaX)
    {
        (Antes ? Antes->Depois : ListaX->ListaItem) = destino;
        if (Depois)
            Depois->Antes = destino;
    }
}

//----------------------------------------------------------------------------
int TListaItem::getValor()
{
    return (ListaX!=0);
}

//----------------------------------------------------------------------------
void TListaItem::MudarRef(TListaX * lista)
{
    if (lista==ListaX)
        return;
    if (ListaX)
    {
        (Antes ? Antes->Depois : ListaX->ListaItem) = Depois;
        if (Depois)
            Depois->Antes = Antes;
        ListaX = 0;
    }
    if (lista)
    {
        ListaX = lista;
        Antes = 0;
        Depois = lista->ListaItem;
        lista->ListaItem = this;
        if (Depois)
            Depois->Antes = this;
    }
}

//----------------------------------------------------------------------------
bool TListaItem::Func(TVariavel * v, const char * nome)
{
// Objeto
    if (comparaZ(nome, "obj")==0)
    {
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrVarObjeto))
            return false;
        if (ListaX)
            Instr::VarAtual->setObj(ListaX->Objeto);
        return true;
    }
// Vai para o objeto anterior
    if (comparaZ(nome, "antes")==0)
    {
        if (ListaX)
            MudarRef(ListaX->ListaAntes);
        return false;
    }
// Vai para o próximo objeto
    if (comparaZ(nome, "depois")==0)
    {
        if (ListaX)
            MudarRef(ListaX->ListaDepois);
        return false;
    }
// Remove objeto da lista
    if (comparaZ(nome, "remover")==0)
    {
        if (ListaX)
        {
            TListaX * l = ListaX;
            MudarRef(ListaX->ListaDepois);
            l->Apagar();
        }
        return false;
    }
// Adiciona objetos antes
    if (comparaZ(nome, "add1")==0)
    {
        if (ListaX==0)
            return false;
        for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
        {
            TObjeto * obj = v1->getObj();
            if (obj==0)
                continue;
        // Cria objeto
            TListaX * l1 = TListaX::Criar();
        // Acrescenta antes
            l1->Lista = ListaX->Lista;
            l1->ListaAntes = ListaX->ListaAntes;
            l1->ListaDepois = ListaX;
            (l1->ListaAntes ? l1->ListaAntes->ListaDepois :
                              l1->Lista->Inicio) = l1;
            ListaX->ListaAntes = l1;
        // Acrescenta objeto
            l1->Objeto = obj;
            l1->ObjAntes = 0;
            l1->ObjDepois = obj->VarListaX;
            if (obj->VarListaX)
                obj->VarListaX->ObjAntes = l1;
            obj->VarListaX = l1;
        // Acerta ListaItem
            l1->ListaItem = 0;
        }
        return false;
    }
// Adiciona objetos depois
    if (comparaZ(nome, "add2")==0)
    {
        if (ListaX==0)
            return false;
        for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
        {
            TObjeto * obj = v1->getObj();
            if (obj==0)
                continue;
        // Cria objeto
            TListaX * l1 = TListaX::Criar();
        // Acrescenta depois
            l1->Lista = ListaX->Lista;
            l1->ListaAntes = ListaX;
            l1->ListaDepois = ListaX->ListaDepois;
            (l1->ListaDepois ? l1->ListaDepois->ListaAntes :
                               l1->Lista->Fim) = l1;
            ListaX->ListaDepois = l1;
        // Acrescenta objeto
            l1->Objeto = obj;
            l1->ObjAntes = 0;
            l1->ObjDepois = obj->VarListaX;
            if (obj->VarListaX)
                obj->VarListaX->ObjAntes = l1;
            obj->VarListaX = l1;
        // Acerta ListaItem
            l1->ListaItem = 0;
        }
        return false;
    }
    return false;
}

//----------------------------------------------------------------------------
TListaX * TListaX::Criar()
{
    return new TListaX;
}

//----------------------------------------------------------------------------
void TListaX::Apagar()
{
// Retira de TListaObj
    (ListaAntes ? ListaAntes->ListaDepois : Lista->Inicio) = ListaDepois;
    (ListaDepois ? ListaDepois->ListaAntes : Lista->Fim) = ListaAntes;
// Retira de TObjeto
    (ObjAntes ? ObjAntes->ObjDepois : Objeto->VarListaX) = ObjDepois;
    if (ObjDepois)
        ObjDepois->ObjAntes = ObjAntes;
// Retira de TListaItem
    while (ListaItem)
        ListaItem->MudarRef(0);
// Apaga objeto
    delete this;
}
