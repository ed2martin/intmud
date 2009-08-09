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
#include "mudarclasse.h"
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
void TVarProg::LimparVar()
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
        { "apagar",       &TVarProg::FuncApagar },
        { "apagarlin",    &TVarProg::FuncApagarLin },
        { "const",        &TVarProg::FuncConst },
        { "criar",        &TVarProg::FuncCriar },
        { "criarlin",     &TVarProg::FuncCriarLin },
        { "depois",       &TVarProg::FuncDepois },
        { "existe",       &TVarProg::FuncExiste },
        { "iniclasse",    &TVarProg::FuncIniClasse },
        { "inifunc",      &TVarProg::FuncIniFunc },
        { "inifunctudo",  &TVarProg::FuncIniFuncTudo },
        { "iniherda",     &TVarProg::FuncIniHerda },
        { "iniherdainv",  &TVarProg::FuncIniHerdaInv },
        { "iniherdatudo", &TVarProg::FuncIniHerdaTudo },
        { "inilinha",     &TVarProg::FuncIniLinha },
        { "lin",          &TVarProg::FuncLin },
        { "nivel",        &TVarProg::FuncNivel },
        { "texto",        &TVarProg::FuncTexto },
        { "varcomum",     &TVarProg::FuncVarComum },
        { "varlocal",     &TVarProg::FuncVarLocal },
        { "varnum",       &TVarProg::FuncVarNum },
        { "varsav",       &TVarProg::FuncVarSav },
        { "vartexto",     &TVarProg::FuncVarTexto },
        { "vartipo",      &TVarProg::FuncVarTipo },
        { "varvetor",     &TVarProg::FuncVarVetor }
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
bool TVarProg::FuncVarNum(TVariavel * v)
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
            valor = (cl->InstrVar[indice][2] == Instr::cConstNum);
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
bool TVarProg::FuncConst(TVariavel * v)
{
    while (true)
    {
        if (Instr::VarAtual < v+2)
            break;
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl==0)
            break;
        int indice = cl->IndiceNome(v[2].getTxt());
        if (indice<0)
            break;
        const char * txt = cl->InstrVar[indice];
    // Constante numérica
        if (txt[2]==Instr::cConstNum)
        {
            TVariavel var;
            var.defvar = txt;
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(var.getTxt());
        }
    // Constante de texto
        if (txt[2]!=Instr::cConstTxt)
            break;
        txt += txt[Instr::endIndice] + 1;
    // Cria variável
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrTxtFixo))
            return false;
        if (Instr::DadosTopo >= Instr::DadosFim)
            return false;
    // Anota o texto
        char * destino = Instr::DadosTopo;
        for (; *txt && destino <= Instr::DadosFim-3; txt++)
            switch (*txt)
            {
            case Instr::ex_barra_n:
                destino[0] = '\\';
                destino[1] = 'n';
                destino += 2;
                break;
            case Instr::ex_barra_b:
                destino[0] = '\\';
                destino[1] = 'b';
                destino += 2;
                break;
            case Instr::ex_barra_c:
                destino[0] = '\\';
                destino[1] = 'c';
                destino += 2;
                break;
            case Instr::ex_barra_d:
                destino[0] = '\\';
                destino[1] = 'd';
                destino += 2;
                break;
            case '\\':
                destino[0] = '\\';
                destino[1] = '\\';
                destino += 2;
                break;
            case '\"':
                destino[0] = '\\';
                destino[1] = '\"';
                destino += 2;
                break;
            default:
                *destino++ = *txt;
            }
        *destino++ = 0;
    // Acerta a variável
        Instr::VarAtual->endvar = Instr::DadosTopo;
        Instr::VarAtual->tamanho = destino - Instr::DadosTopo;
        Instr::DadosTopo = destino;
        return true;
    }
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrTxtFixo))
        return false;
    Instr::VarAtual->endfixo = "";
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
bool TVarProg::FuncIniFuncTudo(TVariavel * v)
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
bool TVarProg::FuncIniHerda(TVariavel * v)
{
    int valor = 0;
    while (Instr::VarAtual >= v+1)
    {
        Classe = TClasse::Procura(v[1].getTxt());
        if (Classe==0)
            break;
        const char * txt = Classe->Comandos;
        if (txt[0]==0 && txt[1]==0)
            break;
        if (txt[2]!=Instr::cHerda || txt[3]==0)
            break;
        TextoAtual = txt + 4;
        ValorFim = (unsigned char)txt[3];
        valor = 4;
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
bool TVarProg::FuncIniHerdaTudo(TVariavel * v)
{
    int valor = 0;
    while (Instr::VarAtual >= v+1)
    {
        Classe = TClasse::Procura(v[1].getTxt());
        if (Classe==0)
            break;
        int total = Classe->Heranca(ClasseHerda, HERDA_TAM);
        if (total<=0)
            break;
        if (total<HERDA_TAM)
            ClasseHerda[total]=0;
        ValorAtual = 0;
        valor = 5;
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
bool TVarProg::FuncIniHerdaInv(TVariavel * v)
{
    int valor = 0;
    while (Instr::VarAtual >= v+1)
    {
        Classe = TClasse::Procura(v[1].getTxt());
        if (Classe==0)
            break;
        if (Classe->NumDeriv==0)
            break;
        PontAtual = Classe->ListaDeriv;
        PontFim = PontAtual + Classe->NumDeriv;
        valor = 6;
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
bool TVarProg::FuncIniLinha(TVariavel * v)
{
    int valor = 0;
    while (Instr::VarAtual >= v+1)
    {
        Classe = TClasse::Procura(v[1].getTxt());
        if (Classe==0)
            break;
    // Linhas de uma classe
        if (Instr::VarAtual < v+2)
        {
            TextoAtual = Classe->Comandos;
            if (TextoAtual[0]==0 && TextoAtual[1]==0)
                break;
            ValorFim = 0;
            valor = 7;
            break;
        }
    // Linhas de uma função ou variável
        int indice = Classe->IndiceNome(v[2].getTxt());
        if (indice < 0)
            break;
        TextoAtual = Classe->InstrVar[indice];
        if (TextoAtual[2]==Instr::cFunc || TextoAtual[2]==Instr::cVarFunc)
            valor = 8;
        else
            valor = 9;
        ValorFim = 1;
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
    int total = 1;
    if (Instr::VarAtual >= v+1)
        total = v[1].getInt();
    switch (consulta)
    {
    case 1:  // iniclasse
        for (; total>0; total--)
        {
            if (ClasseAtual == ClasseFim)
            {
                MudaConsulta(0);
                break;
            }
            ClasseAtual = TClasse::RBnext(ClasseAtual);
            if (ClasseAtual==0)
                MudaConsulta(0);
        }
        break;
    case 2:  // inifunc
        while (total>0)
        {
            ValorAtual++;
            if (ValorAtual > ValorFim)
            {
                MudaConsulta(0);
                break;
            }
            if (Classe->IndiceVar[ValorAtual] & 0x800000)
                total--;
        }
        break;
    case 3:  // inifunctudo
        if (total <= 0)
            break;
        if (total >= 0xFFFF || ValorAtual + total > ValorFim)
            MudaConsulta(0);
        else
            ValorAtual += total;
        break;
    case 4:  // iniherda
        if (ValorFim<=total)
            MudaConsulta(0);
        else for (; total>0; total--)
        {
            ValorFim--;
            while (*TextoAtual++);
        }
        break;
    case 5:  // iniherdatudo
        if (total <= 0)
            break;
        ValorAtual+=total;
        if (total>=HERDA_TAM || ValorAtual>=HERDA_TAM)
            MudaConsulta(0);
        else if (ClasseHerda[ValorAtual]==0)
            MudaConsulta(0);
        break;
    case 6:  // iniherdainv
        if (total <= 0)
            break;
        PontAtual+=total;
        if (total>=0xFFFFFF || PontAtual>=PontFim)
            MudaConsulta(0);
        break;
    case 7:  // inilinha com classe
    case 8:  // inilinha com função
        for (; total>0 && consulta; total--)
        {
            switch (TextoAtual[2])
            {
            case Instr::cFunc:
            case Instr::cVarFunc:
            case Instr::cSe:
            case Instr::cEnquanto:
                if (ValorFim)
                    ValorFim++;
                break;
            }
            TextoAtual += Num16(TextoAtual);
            if (TextoAtual[0]==0 && TextoAtual[1]==0)
                MudaConsulta(0);
            else switch (TextoAtual[2])
            {
            case Instr::cConstNulo:
            case Instr::cConstTxt:
            case Instr::cConstNum:
            case Instr::cConstExpr:
                if (consulta==8)
                    MudaConsulta(0);
                else if (ValorFim>1)
                    ValorFim=1;
                break;
            case Instr::cFunc:
            case Instr::cVarFunc:
                if (consulta==8)
                    MudaConsulta(0);
                else
                    ValorFim = 1;
                break;
            case Instr::cFimSe:
            case Instr::cEFim:
                if (ValorFim>1)
                    ValorFim--;
                break;
            }
        }
        break;
    case 9:  // inilinha com variável
        MudaConsulta(0);
        break;
    }
    return false;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncTexto(TVariavel * v)
{
    const char * p = "";
    switch (consulta)
    {
    case 1:  // iniclasse
        p = ClasseAtual->Nome;
        break;
    case 2:  // inifunc
    case 3:  // inifunctudo
        p = Classe->InstrVar[ValorAtual] + Instr::endNome;
        break;
    case 4:  // iniherda
        p = TextoAtual;
        break;
    case 5:  // iniherdatudo
        p = ClasseHerda[ValorAtual]->Nome;
        break;
    case 6:  // iniherdainv
        p = PontAtual[0]->Nome;
        break;
    case 7:  // inilinha com classe
    case 8:  // inilinha com função
    case 9:  // inilinha com variável
        {
        // Cria variável
            const char * instr = TextoAtual;
            Instr::ApagarVar(v);
            if (!Instr::CriarVar(Instr::InstrTxtFixo))
                return false;
            if (Instr::DadosTopo >= Instr::DadosFim)
                return false;
        // Anota o texto
            char * destino = Instr::DadosTopo;
            Instr::Decod(destino, instr, Instr::DadosFim-destino);
            while (*destino++);
        // Acerta a variável
            Instr::VarAtual->endvar = Instr::DadosTopo;
            Instr::VarAtual->tamanho = destino - Instr::DadosTopo;
            Instr::DadosTopo = destino;
            return true;
        }
    }
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrTxtFixo))
        return false;
    Instr::VarAtual->endfixo = p;
    return true;
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

//------------------------------------------------------------------------------
bool TVarProg::FuncNivel(TVariavel * v)
{
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarInt))
        return false;
    if ((consulta==7 || consulta==8) && ValorFim>=2)
    {
        int valor = ValorFim-1;
        if (TextoAtual[2]==Instr::cSenao1 || TextoAtual[2]==Instr::cSenao2)
            valor--;
        if (valor>0)
            Instr::VarAtual->setInt(valor);
    }
    return true;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncApagar(TVariavel * v)
{
    int valor = 0;
    while (Instr::VarAtual >= v+1)
    {
        const char * nome = v[1].getTxt();
        TClasse * cl = TClasse::Procura(nome);
        TMudarClasse * obj = TMudarClasse::Procurar(nome);
    // Apagar uma classe
        if (Instr::VarAtual < v+2)
        {
            if (cl)
            {
                if (obj==0)
                    obj = new TMudarClasse(nome);
                obj->MudarComandos(0);
                obj->MarcarApagar();
            }
            else if (obj)
                delete obj;
            valor = 1;
            break;
        }
    // Apagar uma variável ou função
        char * texto1=0; // Endereço do primeiro bloco
        int tamanho1=0; // Tamanho do primeiro bloco
        char * texto2=0; // Endereço do segundo bloco
        int tamanho2=0; // Tamanho do segundo bloco
    // Obtém: texto1 = instruções da classe
        if (obj && obj->Comandos!=0) // Comandos definidos em TMudarClasse
            texto1 = obj->Comandos;
        else if (cl)    // Comandos definidos em TClasse
            texto1 = cl->Comandos;
        else            // Nenhum comando definido, retorna
            break;
    // Obtém bloco de instruções
        texto2 = TMudarAux::ProcuraInstr(texto1, v[2].getTxt());
        if (texto2[0]==0 && texto2[1]==0) // Não encontrou
            break;
        tamanho1 = texto2 - texto1;
        texto2 = TMudarAux::AvancaInstr(texto2);
        tamanho2 = TMudarAux::FimInstr(texto2) - texto2 + 2;
    // Anota as alterações em objeto TMudarClasse
        if (obj==0)
            obj = new TMudarClasse(v[1].getTxt());
        char * p = new char[tamanho1+tamanho2];
        memcpy(p, texto1, tamanho1);
        memcpy(p+tamanho1, texto2, tamanho2);
        obj->MudarComandos(p);
        valor=1;
        break;
    }
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarInt))
        return false;
    Instr::VarAtual->setInt(valor != 0);
    return true;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncCriar(TVariavel * v)
{
// Criar uma variável ou função
    if (Instr::VarAtual >= v+2)
    {
        //puts("Criar variável"); fflush(stdout);
        const char * nomeclasse = v[1].getTxt();
        TClasse * cl = TClasse::Procura(nomeclasse);
        TMudarClasse * obj = TMudarClasse::Procurar(nomeclasse);
        TMudarAux mudarcom; // Para mudar a lista de instruções
    // Obtém: texto1 = instruções da classe
        char * texto1 = 0;
        if (obj && obj->Comandos!=0) // Comandos definidos em TMudarClasse
            texto1 = obj->Comandos, nomeclasse=obj->Nome;
        else if (cl)    // Comandos definidos em TClasse
            texto1 = cl->Comandos, nomeclasse=cl->Nome;
        else            // Nenhum comando definido, retorna
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto("Classe não existe");
        }
    // Codifica as instruções
        char mens[65000];
        int  total = TMudarAux::CodifInstr(mens, v[2].getTxt(), sizeof(mens)-2);
        if (total<0) // Erro
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(mens);
        }
        if (total==0) // Nenhuma alteração
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto("");
        }
    // Obtém bloco de instruções
        mens[total]=0;  // Marca o fim das instruções
        mens[total+1]=0;
        char * p = TMudarAux::AvancaInstr(mens); // Passa para próxima função/variável
        if (p[0] || p[1])
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(
                    "Definição de mais de uma função/variável/herda");
        }
        if (mens[2]==Instr::cHerda)
        {
            mudarcom.AddBloco(mens, total);
            if ((texto1[0] || texto1[1]) && texto1[2]==Instr::cHerda)
                texto1 += Num16(texto1);
            mudarcom.AddBloco(texto1,
                                   TMudarAux::FimInstr(texto1) - texto1);
        }
        else if (mens[2] > Instr::cVariaveis)
        {
        // Obtém o lugar ideal:
        // 1=no início, 2=nas definições de função, 3=em qualquer lugar
            int lugar = 1;
            switch (mens[2])
            {
            case Instr::cFunc:
            case Instr::cVarFunc:
                lugar=2;
                break;
            case Instr::cConstNulo:
            case Instr::cConstTxt:
            case Instr::cConstNum:
            case Instr::cConstExpr:
                lugar=3;
                break;
            }
        // Obtém nova lista de instruções
            char * p = texto1;
            int l = 1; // Lugar atual
            while (p[0] || p[1])
            {
            // Não é variável: prossegue
                if (p[2] <= Instr::cVariaveis)
                {
                    p += Num16(p);
                    continue;
                }
            // Verifica se é início da lista de funções
                if (p[2]==Instr::cFunc || p[2]==Instr::cVarFunc)
                {
                    l=2;
                    if (lugar==1) // É variável: deve vir antes
                    {
                                    // Anota o bloco atual
                        mudarcom.AddBloco(texto1, p-texto1);
                                    // Anota a variável
                        mudarcom.AddBloco(mens, total);
                        lugar=0; // Indica que já anotou a variável
                        texto1 = p;
                    }
                }
            // Verifica se é a instrução procurada
                if (comparaZ(p + Instr::endNome,
                                    mens + Instr::endNome)==0)
                {
                    mudarcom.AddBloco(texto1, p-texto1);// Anota bloco atual
                    if ((l & lugar)!=0) // Anota variável se está no lugar certo
                    {
                        mudarcom.AddBloco(mens, total);
                        lugar = 0; // Indica que já anotou a variável
                    }
                    texto1 = p = TMudarAux::AvancaInstr(p); // Avança p, acerta texto1
                    continue;
                }
            // Passa para próxima instrução
                p = TMudarAux::AvancaInstr(p);
            } // while (p[0] || p[1])
            mudarcom.AddBloco(texto1, p-texto1);// Anota bloco atual
            if (lugar) // Anota variável
                mudarcom.AddBloco(mens, total);
        }
        else
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(
                    "Faltou o nome da função/variável");
        }
    // Verifica se bloco válido
        char err[200];
        if (!mudarcom.ChecaBloco(err, sizeof(err)))
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(err);
        }
    // Anota alterações em objeto TMudarClasse
        if (obj==0)
            obj = new TMudarClasse(nomeclasse);
        mudarcom.AnotaBloco(obj);
    }
// Criar uma classe
    else if (Instr::VarAtual >= v+1)
    {
        //puts("Criar classe"); fflush(stdout);
    // Obtém o nome da classe
        char nomeclasse[CLASSE_NOME_TAM+30];
        const char * nome = v[1].getTxt();
        char * p = nomeclasse;
        while (*nome && *nome!=Instr::ex_barra_n)
        {
            if (p >= nomeclasse+sizeof(nomeclasse)-2)
            {
                Instr::ApagarVar(v);
                return Instr::CriarVarTexto("Nome de classe inválido");
            }
            *p++ = *nome++;
        }
        *p=0;
    // Checa se nome válido / acerta o nome
        if (!TClasse::NomeValido(nomeclasse))
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto("Nome de classe inválido");
        }
    // Codifica as instruções
        if (*nome)
            nome++;
        char mens[65000];
        int  total = TMudarAux::CodifInstr(mens, nome, sizeof(mens)-2);
        if (total<0)
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(mens);
        }
        mens[total++] = 0;
        mens[total++] = 0;
    // Verifica se bloco válido
        int linha=1;
        char * com = mens;
        Instr::ChecaLinha checalinha;
        checalinha.Inicio();
        while (com[0] || com[1])
        {
            const char * p = checalinha.Instr(com);
            if (p)
            {
                sprintf(mens, "%d: %s\n", linha, p);
                Instr::ApagarVar(v);
                return Instr::CriarVarTexto(mens);
            }
            com+=Num16(com), linha++;
        }
    // Anota alterações em objeto TMudarClasse
        TMudarClasse * obj = TMudarClasse::Procurar(nomeclasse);
        if (obj==0)
            obj = new TMudarClasse(nomeclasse);
        com = new char[total];
        memcpy(com, mens, total);
        obj->MudarComandos(com);
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto("");
}

//------------------------------------------------------------------------------
bool TVarProg::FuncApagarLin(TVariavel * v)
{
    if (Instr::VarAtual < v+2)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto("");
    }
    const char * nome = v[1].getTxt();
    TClasse * cl = TClasse::Procura(nome);
    TMudarClasse * obj = TMudarClasse::Procurar(nome);
// Variáveis do bloco
    char * texto1=0; // Endereço do primeiro bloco
    int tamanho1=0; // Tamanho do primeiro bloco
    char * texto2=0; // Endereço do segundo bloco
    int tamanho2=0; // Tamanho do segundo bloco
// Obtém: texto1 = instruções da classe
    if (obj && obj->Comandos!=0) // Comandos definidos em TMudarClasse
        texto1 = obj->Comandos;
    else if (cl)    // Comandos definidos em TClasse
        texto1 = cl->Comandos;
    else            // Nenhum comando definido, retorna
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto("Classe inexistente");
    }
// Apagar linha da classe
    if (Instr::VarAtual == v+2)
    {
        int linha = v[2].getInt() - 1;
        texto2 = texto1;
        while (linha>0 && (texto2[0] || texto2[1]))
            linha--, texto2+=Num16(texto2);
    }
// Apagar uma linha da variável/função
    else
    {
        int linha = v[3].getInt() - 1;
        texto2 = TMudarAux::ProcuraInstr(texto1, v[2].getTxt());
        const char * fim = TMudarAux::AvancaInstr(texto2);
        while (linha>0 && texto2<fim)
            linha--, texto2+=Num16(texto2);
        if (texto2>=fim)
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto("");
        }
    }
// Verifica se alguma linha será apagada
    if (texto2[0]==0 && texto2[1]==0)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto("");
    }
// Verifica se bloco válido
    int linha=1;
    char * com = texto1;
    Instr::ChecaLinha checalinha;
    checalinha.Inicio();
    for (; com[0] || com[1]; com+=Num16(com), linha++)
    {
        if (com==texto2)
            continue;
        const char * p = checalinha.Instr(com);
        if (p)
        {
            char mens[80];
            sprintf(mens, "%d: %s\n", linha, p);
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(mens);
        }
    }
// Obtém bloco de instruções
    tamanho1 = texto2 - texto1;
    texto2 += Num16(texto2);
    tamanho2 = com - texto2 + 2;
// Anota as alterações em objeto TMudarClasse
    if (obj==0)
        obj = new TMudarClasse(cl->Nome);
    char * p = new char[tamanho1+tamanho2];
    memcpy(p, texto1, tamanho1);
    memcpy(p+tamanho1, texto2, tamanho2);
    obj->MudarComandos(p);
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto("");
}

//------------------------------------------------------------------------------
bool TVarProg::FuncCriarLin(TVariavel * v)
{
    if (Instr::VarAtual < v+3)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto("");
    }
    const char * nome = v[1].getTxt();
    TClasse * cl = TClasse::Procura(nome);
    TMudarClasse * obj = TMudarClasse::Procurar(nome);
    TMudarAux mudarcom; // Para mudar a lista de instruções
// Obtém: texto1 = instruções da classe
    char * texto1 = 0;
    if (obj && obj->Comandos!=0) // Comandos definidos em TMudarClasse
        texto1 = obj->Comandos;
    else if (cl)    // Comandos definidos em TClasse
        texto1 = cl->Comandos;
    else            // Nenhum comando definido, retorna
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto("Classe inexistente");
    }
// Instruções na classe
    char * p = 0;
    if (Instr::VarAtual == v+3)
    {
        int linha = v[2].getInt() - 1;
        p = texto1;
        while (linha>0 && (p[0] || p[1]))
            linha--, p+=Num16(p);
        nome = v[3].getTxt();
    }
// Instruções em variável/função
    else
    {
        int linha = v[3].getInt() - 1;
        p = TMudarAux::ProcuraInstr(texto1, v[2].getTxt());
        if (p[0]==0 && p[1]==0)
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto("Variável/função inexistente");
        }
        const char * fim = TMudarAux::AvancaInstr(p);
        while (linha>0 && p<fim)
            linha--, p+=Num16(p);
        nome = v[4].getTxt();
    }
    mudarcom.AddBloco(texto1, p-texto1);
    texto1 = p;
// Codifica as instruções
    char mens[65000];
    int  total = TMudarAux::CodifInstr(mens, nome, sizeof(mens)-2);
    if (total<=0) // 0 se nenhuma instrução, <0 se erro
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(total==0 ? "" : mens);
    }
    mudarcom.AddBloco(mens, total);
    mudarcom.AddBloco(texto1, TMudarAux::FimInstr(texto1) - texto1);
// Verifica se bloco válido
    char err[200];
    if (!mudarcom.ChecaBloco(err, sizeof(err)))
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(err);
    }
// Anota alterações em objeto TMudarClasse
    if (obj==0)
        obj = new TMudarClasse(cl->Nome);
    mudarcom.AnotaBloco(obj);
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto("");
}
