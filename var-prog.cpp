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
#include "var-prog.h"
#include "variavel.h"
#include "classe.h"
#include "instr.h"
#include "misc.h"

//------------------------------------------------------------------------------
TVarProg * TVarProg::Inicio = 0;

//------------------------------------------------------------------------------
void TVarProg::Criar()
{
    consulta = 0;
}

//------------------------------------------------------------------------------
void TVarProg::Apagar()
{
    if (consulta==0)
        return;
    (Antes ? Antes->Depois : Inicio) = Depois;
    if (Depois)
        Depois->Antes = Antes;
    consulta=0;
}

//------------------------------------------------------------------------------
void TVarProg::Mover(TVarProg * destino)
{
    if (consulta==0)
    {
        destino->consulta=0;
        return;
    }
    (Antes ? Antes->Depois : Inicio) = destino;
    if (Depois)
        Depois->Antes = destino;
    move_mem(destino, this, sizeof(TVarProg));
}

//------------------------------------------------------------------------------
int TVarProg::getValor()
{
    return 0;
}

//------------------------------------------------------------------------------
void TVarProg::ProcEventos()
{
// Retira todos os objetos da lista ligada
    for (TVarProg * obj = Inicio; obj; obj=obj->Depois)
        obj->consulta=0;
    Inicio=0;
}

//------------------------------------------------------------------------------
bool TVarProg::Func(TVariavel * v, const char * nome)
{
    typedef class {
    public:
        const char * Nome; ///< Nome da função predefinida
        bool (TVarProg::*Func)(TVariavel * v); ///< Função
    } TProgFunc;
// Lista das funções de prog
// Deve obrigatoriamente estar em ordem alfabética
    const TProgFunc ProgFunc[] = {
        { "depois",     &TVarProg::FuncDepois },
        { "existe",     &TVarProg::FuncExiste },
        { "iniclasse",  &TVarProg::FuncIniClasse },
        { "inifunc",    &TVarProg::FuncIniFunc },
        { "inifunc2",   &TVarProg::FuncIniFunc2 },
        { "lin",        &TVarProg::FuncLin },
        { "texto",      &TVarProg::FuncTexto },
        { "varcomum",   &TVarProg::FuncVarComum },
        { "varlocal",   &TVarProg::FuncVarLocal },
        { "varsav",     &TVarProg::FuncVarSav },
        { "vartexto",   &TVarProg::FuncVarTexto },
        { "vartipo",    &TVarProg::FuncVarTipo },
        { "varvetor",   &TVarProg::FuncVarVetor }
    };
// Procura a função correspondente e executa
    int ini = 0;
    int fim = sizeof(ProgFunc) / sizeof(ProgFunc[0]) - 1;
    while (ini <= fim)
    {
        int meio = (ini+fim)/2;
        int resultado = comparaZ(nome, ProgFunc[meio].Nome);
        if (resultado==0) // Se encontrou...
            return (this->*ProgFunc[meio].Func)(v);
        if (resultado<0)
            fim = meio-1;
        else
            ini = meio+1;
    }
    return false;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncExiste(TVariavel * v)
{
    bool existe = false;
    if (Instr::VarAtual >= v+1)
    {
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl)
        {
            if (Instr::VarAtual == v+1)
                existe = true;
            else if (cl->IndiceNome(v[2].getTxt())>=0)
                existe = true;
        }
    }
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarInt))
        return false;
    if (existe)
        Instr::VarAtual->setInt(1);
    return true;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncVarComum(TVariavel * v)
{
    bool valor=false;
    while (true)
    {
        if (Instr::VarAtual < v+2)
            break;
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl==0)
            break;
        int indice = cl->IndiceNome(v[2].getTxt());
        if (indice>=0)
            valor = cl->InstrVar[indice][Instr::endProp] & 1;
        break;
    }
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarInt))
        return false;
    if (valor)
        Instr::VarAtual->setInt(1);
    return true;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncVarLocal(TVariavel * v)
{
    bool valor=false;
    while (true)
    {
        if (Instr::VarAtual < v+2)
            break;
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl==0)
            break;
        int indice = cl->IndiceNome(v[2].getTxt());
        if (indice>=0)
            valor = cl->IndiceVar[indice] & 0x800000;
        break;
    }
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarInt))
        return false;
    if (valor)
        Instr::VarAtual->setInt(1);
    return true;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncVarSav(TVariavel * v)
{
    bool valor=false;
    while (true)
    {
        if (Instr::VarAtual < v+2)
            break;
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl==0)
            break;
        int indice = cl->IndiceNome(v[2].getTxt());
        if (indice>=0)
            valor = cl->InstrVar[indice][Instr::endProp] & 2;
        break;
    }
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarInt))
        return false;
    if (valor)
        Instr::VarAtual->setInt(1);
    return true;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncVarTexto(TVariavel * v)
{
    bool valor=false;
    while (true)
    {
        if (Instr::VarAtual < v+2)
            break;
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl==0)
            break;
        int indice = cl->IndiceNome(v[2].getTxt());
        if (indice>=0)
            valor = (cl->InstrVar[indice][2] == Instr::cConstTxt);
        break;
    }
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarInt))
        return false;
    if (valor)
        Instr::VarAtual->setInt(1);
    return true;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncVarTipo(TVariavel * v)
{
    char mens[32] = "";
    while (true)
    {
        if (Instr::VarAtual < v+2)
            break;
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl==0)
            break;
        int indice = cl->IndiceNome(v[2].getTxt());
        if (indice>=0)
            copiastr(mens, Instr::NomeInstr(cl->InstrVar[indice]),
                     sizeof(mens));
        break;
    }
// Acerta variáveis
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(mens);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncVarVetor(TVariavel * v)
{
    int valor=0;
    while (true)
    {
        if (Instr::VarAtual < v+2)
            break;
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl==0)
            break;
        int indice = cl->IndiceNome(v[2].getTxt());
        if (indice>=0)
            valor = (unsigned char)cl->InstrVar[indice][Instr::endVetor];
        break;
    }
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarInt))
        return false;
    if (valor)
        Instr::VarAtual->setInt(valor);
    return true;
}

//------------------------------------------------------------------------------
void TVarProg::MudaConsulta(int valor)
{
    if (valor==0)
    {
        if (consulta==0)
            return;
        consulta = 0;
        (Antes ? Antes->Depois : Inicio) = Depois;
        if (Depois)
            Depois->Antes = Antes;
        return;
    }
    if (consulta==0)
    {
        Antes = 0;
        Depois = Inicio;
        Inicio = this;
        if (Depois)
            Depois->Antes = this;
    }
    consulta = valor;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncIniClasse(TVariavel * v)
{
    int valor = 0;
    const char * texto = "";
    if (Instr::VarAtual >= v+1)
        texto = v[1].getTxt();
    if (*texto==0)
    {
        ClasseAtual = TClasse::RBfirst();
        ClasseFim = TClasse::RBlast();
        if (ClasseAtual)
            valor=1;
    }
    else
    {
        ClasseAtual = TClasse::ProcuraIni(texto);
        if (ClasseAtual)
            valor=1, ClasseFim = TClasse::ProcuraFim(texto);
    }
    MudaConsulta(valor);
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarInt))
        return false;
    Instr::VarAtual->setInt(valor != 0);
    return true;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncIniFunc(TVariavel * v)
{
    int valor = 0;
    while (Instr::VarAtual >= v+1)
    {
        Classe = TClasse::Procura(v[1].getTxt());
        if (Classe==0)
            break;
        const char * texto = "";
        if (Instr::VarAtual >= v+2)
            texto = v[2].getTxt();
        if (*texto)
        {
            ValorAtual = Classe->IndiceNomeIni(texto);
            if (ValorAtual<0)
                break;
            ValorFim = Classe->IndiceNomeFim(texto);
        }
        else
        {
            if (Classe->NumVar==0)
                break;
            ValorAtual = 0, ValorFim = Classe->NumVar-1;
        }
        while ((Classe->IndiceVar[ValorAtual] & 0x800000)==0)
            if (++ValorAtual > ValorFim)
                break;
        while ((Classe->IndiceVar[ValorFim] & 0x800000)==0)
            if (--ValorFim < ValorAtual)
                break;
        if (ValorAtual <= ValorFim)
            valor = 2;
        break;
    }
    MudaConsulta(valor);
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarInt))
        return false;
    Instr::VarAtual->setInt(valor != 0);
    return true;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncIniFunc2(TVariavel * v)
{
    int valor = 0;
    while (Instr::VarAtual >= v+1)
    {
        Classe = TClasse::Procura(v[1].getTxt());
        if (Classe==0)
            break;
        const char * texto = "";
        if (Instr::VarAtual >= v+2)
            texto = v[2].getTxt();
        if (*texto)
        {
            ValorAtual = Classe->IndiceNomeIni(texto);
            if (ValorAtual>=0)
                valor=3, ValorFim = Classe->IndiceNomeFim(texto);
        }
        else if (Classe->NumVar)
            valor=3, ValorAtual=0, ValorFim=Classe->NumVar-1;
        break;
    }
    MudaConsulta(valor);
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarInt))
        return false;
    Instr::VarAtual->setInt(valor != 0);
    return true;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncDepois(TVariavel * v)
{
    switch (consulta)
    {
    case 1:  // iniclasse
        if (ClasseAtual == ClasseFim)
        {
            MudaConsulta(0);
            break;
        }
        ClasseAtual = TClasse::RBnext(ClasseAtual);
        if (ClasseAtual==0)
            MudaConsulta(0);
        break;
    case 2:  // inifunc
        while (true)
        {
            ValorAtual++;
            if (ValorAtual > ValorFim)
            {
                MudaConsulta(0);
                break;
            }
            if (Classe->IndiceVar[ValorAtual] & 0x800000)
                break;
        }
        break;
    case 3:  // inifunc2
        if (ValorAtual >= ValorFim)
            MudaConsulta(0);
        else
            ValorAtual++;
        break;

    }
    return false;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncTexto(TVariavel * v)
{
    switch (consulta)
    {
    case 1:  // iniclasse
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrTxtFixo))
            return false;
        Instr::VarAtual->endvar = ClasseAtual->Nome;
        return true;
    case 2:  // inifunc
    case 3:  // inifunc2
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrTxtFixo))
            return false;
        Instr::VarAtual->endvar =
                Classe->InstrVar[ValorAtual] + Instr::endNome;
        return true;
    }
    //char mens[4096];
    //*mens=0;
    //Instr::ApagarVar(v);
    //return Instr::CriarVarTexto(mens);
    return false;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncLin(TVariavel * v)
{
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarInt))
        return false;
    if (consulta)
        Instr::VarAtual->setInt(1);
    return true;
}
