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

#include "arqmapa.h"
#include "misc.h"

//------------------------------------------------------------------------------
TArqMapa * TArqMapa::Inicio=0;
TArqMapa * TArqMapa::Fim=0;

//------------------------------------------------------------------------------
TArqMapa::TArqMapa(const char * classe)
{
    char * d=Arquivo;
    while (d<Arquivo+sizeof(Arquivo)-1 && *classe!=' ' && *classe!=0)
        *d++ = *classe++;
    *d=0;
    Anterior=Fim, Proximo=0;
    (Fim ? Fim->Proximo : Inicio)=this;
    Fim=this;
    Mudou=false;
}

//------------------------------------------------------------------------------
TArqMapa::~TArqMapa()
{
    (Anterior ? Anterior->Proximo : Inicio) = Proximo;
    (Proximo ? Proximo->Anterior : Fim) = Anterior;
}

//------------------------------------------------------------------------------
void TArqMapa::Salvar(const char * classe)
{
    if (Inicio && classe)
    {
        TArqMapa * obj;
        char mens[32];
        char * d=mens;
        while (d<mens+sizeof(mens)-1 && *classe!=' ' && *classe!=0)
            *d++ = *classe++;
        if (*classe==0)
            d=mens;
        *d=0;
        for (obj=Inicio; obj; obj=obj->Proximo)
            if (comparaZ(obj->Arquivo, mens)==0)
                break;
        if (obj==0)
            obj=new TArqMapa(mens);
        obj->Mudou=true;
    }
}
