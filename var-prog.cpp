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
#include <assert.h>
#include "var-prog.h"
#include "variavel.h"
#include "classe.h"
#include "mudarclasse.h"
#include "arqmapa.h"
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
// Lista das funções de prog
// Deve obrigatoriamente estar em letras minúsculas e ordem alfabética
    static const struct {
        const char * Nome;
        bool (TVarProg::*Func)(TVariavel * v); } ExecFunc[] = {
        { "apagar",       &TVarProg::FuncApagar },
        { "apagarlin",    &TVarProg::FuncApagarLin },
        { "arquivo",      &TVarProg::FuncArquivo },
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
        { "salvar",       &TVarProg::FuncSalvar },
        { "salvartudo",   &TVarProg::FuncSalvarTudo },
        { "texto",        &TVarProg::FuncTexto },
        { "varcomum",     &TVarProg::FuncVarComum },
        { "varnum",       &TVarProg::FuncVarNum },
        { "varsav",       &TVarProg::FuncVarSav },
        { "vartexto",     &TVarProg::FuncVarTexto },
        { "vartipo",      &TVarProg::FuncVarTipo },
        { "varvetor",     &TVarProg::FuncVarVetor }  };
// Procura a função correspondente e executa
    int ini = 0;
    int fim = sizeof(ExecFunc) / sizeof(ExecFunc[0]) - 1;
    char mens[80];
    copiastrmin(mens, nome, sizeof(mens));
    while (ini <= fim)
    {
        int meio = (ini+fim)/2;
        int resultado = strcmp(mens, ExecFunc[meio].Nome);
        if (resultado==0) // Se encontrou...
            return (this->*ExecFunc[meio].Func)(v);
        if (resultado<0) fim=meio-1; else ini=meio+1;
    }
    return false;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncExiste(TVariavel * v)
{
    int existe = 0;
    if (Instr::VarAtual >= v+1)
    {
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl)
        {
            if (Instr::VarAtual == v+1)
                existe = 1;
            else
            {
                int x = cl->IndiceNome(v[2].getTxt());
                if (x >= 0)
                    existe = (cl->IndiceVar[x] & 0x800000 ? 1 : 2);
            }
        }
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarInt(existe);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncArquivo(TVariavel * v)
{
    const char * mens = "";
    while (true)
    {
        if (Instr::VarAtual < v+1)
            break;
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl)
            mens = cl->ArqArquivo->Arquivo;
        break;
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(mens);
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
    return Instr::CriarVarInt(valor);
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
    return Instr::CriarVarInt(valor);
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
    return Instr::CriarVarInt(valor);
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
    return Instr::CriarVarInt(valor);
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
    return Instr::CriarVarInt(valor);
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
    return Instr::CriarVarInt(valor != 0);
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
    return Instr::CriarVarInt(valor != 0);
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
    return Instr::CriarVarInt(valor != 0);
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
        if (txt[2]!=Instr::cHerda || txt[Instr::endVar]==0)
            break;
        TextoAtual = txt + Instr::endVar + 1;
        ValorFim = (unsigned char)txt[Instr::endVar];
        valor = 4;
        break;
    }
    MudaConsulta(valor);
    Instr::ApagarVar(v);
    return Instr::CriarVarInt(valor != 0);
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
    return Instr::CriarVarInt(valor != 0);
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
    return Instr::CriarVarInt(valor != 0);
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
    return Instr::CriarVarInt(valor != 0);
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
            TextoAtual += Num16(TextoAtual);
            if (TextoAtual[0]==0 && TextoAtual[1]==0)
                MudaConsulta(0);
            else switch (TextoAtual[2])
            {
            case Instr::cConstNulo:
            case Instr::cConstTxt:
            case Instr::cConstNum:
            case Instr::cConstExpr:
            case Instr::cConstVar:
            case Instr::cFunc:
            case Instr::cVarFunc:
                if (consulta==8)
                    MudaConsulta(0);
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
    return Instr::CriarVarInt(consulta != 0);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncNivel(TVariavel * v)
{
    Instr::ApagarVar(v);
    if (!Instr::CriarVarInt(0))
        return false;
    if (consulta==7 || consulta==8)
        Instr::VarAtual->setInt((unsigned char)TextoAtual[Instr::endAlin]);
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
                else if (obj->InfoApagar())
                    break;
                obj->MudarComandos(0);
                obj->MarcarApagar();
            }
            else if (obj)
                delete obj;
            else
                break;
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
    return Instr::CriarVarInt(valor != 0);
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
        TAddBuffer mens;
        bool codifok = TMudarAux::CodifInstr(&mens, v[2].getTxt());
        if (mens.Total==0) // Nenhuma alteração
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto("");
        }
        mens.Add("\x00\x00", 2); // Marca o fim das instruções
        mens.AnotarBuf();        // Anota resultado em mens.Buf
        if (!codifok) // Erro
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(mens.Buf);
        }
        char * p = TMudarAux::AvancaInstr(mens.Buf); // Passa para próxima função/variável
        if (p[0] || p[1])
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(
                    "Definição de mais de uma função/variável/herda");
        }
        if (mens.Buf[2]==Instr::cHerda)
        {
            mudarcom.AddBloco(mens.Buf, mens.Total-2);
            if ((texto1[0] || texto1[1]) && texto1[2]==Instr::cHerda)
                texto1 += Num16(texto1);
            mudarcom.AddBloco(texto1, TMudarAux::FimInstr(texto1) - texto1);
        }
        else if (mens.Buf[2] > Instr::cVariaveis)
        {
        // Obtém o lugar ideal:
        // 1=no início, 2=nas definições de função, 3=em qualquer lugar
            int lugar = 1;
            switch (mens.Buf[2])
            {
            case Instr::cFunc:
            case Instr::cVarFunc:
                lugar=2;
                break;
            case Instr::cConstNulo:
            case Instr::cConstTxt:
            case Instr::cConstNum:
            case Instr::cConstExpr:
            case Instr::cConstVar:
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
                        mudarcom.AddBloco(mens.Buf, mens.Total-2);
                        lugar=0; // Indica que já anotou a variável
                        texto1 = p;
                    }
                }
            // Verifica se é a instrução procurada
                if (comparaVar(p + Instr::endNome,
                                    mens.Buf + Instr::endNome)==0)
                {
                    mudarcom.AddBloco(texto1, p-texto1);// Anota bloco atual
                    if ((l & lugar)!=0) // Anota variável se está no lugar certo
                    {
                        mudarcom.AddBloco(mens.Buf, mens.Total-2);
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
                mudarcom.AddBloco(mens.Buf, mens.Total-2);
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
        char nomearq[INT_NOME_TAM+30];
        char nomeclasse[CLASSE_NOME_TAM+30];
        const char * nome = v[1].getTxt();
    // Obtém o nome da classe
        char * p = nomeclasse;
        while (*nome && *nome!=Instr::ex_barra_n &&
                p < nomeclasse+sizeof(nomeclasse)-1)
            *p++ = *nome++;
        *p=0;
        if (*nome==Instr::ex_barra_n)
            nome++;
    // Obtém o nome do arquivo
        p = nomearq;
        while (*nome && *nome!=Instr::ex_barra_n &&
                p < nomearq+sizeof(nomearq)-1)
            *p++ = *nome++;
        *p=0;
        if (*nome==Instr::ex_barra_n)
            nome++;
    // Checa se nome de arquivo válido
        if (!TArqMapa::NomeValido(nomearq))
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto("Nome de arquivo inválido");
        }
    // Checa se nome de classe válido
        if (!TClasse::NomeValido(nomeclasse))
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto("Nome de classe inválido");
        }
    // Codifica as instruções
        TAddBuffer mens;
        bool codifok = TMudarAux::CodifInstr(&mens, nome);
        mens.Add("\x00\x00", 2); // Marca o fim das instruções
        mens.AnotarBuf();        // Anota resultado em mens.Buf
        if (!codifok) // Erro
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(mens.Buf);
        }
    // Verifica se bloco válido
        int linha=1;
        char * com = mens.Buf;
        Instr::ChecaLinha checalinha;
        checalinha.Inicio();
        while (com[0] || com[1])
        {
            const char * p = checalinha.Instr(com);
            if (p)
            {
                char txt1[1024];
                mprintf(txt1, sizeof(txt1), "%d: %s\n", linha, p);
                Instr::ApagarVar(v);
                return Instr::CriarVarTexto(txt1);
            }
            com+=Num16(com), linha++;
        }
    // Anota alterações em objeto TMudarClasse
        TMudarClasse * obj = TMudarClasse::Procurar(nomeclasse);
        if (obj==0)
            obj = new TMudarClasse(nomeclasse);
        obj->MudarComandos(mens.Buf);  // Transfere a memória alocada
        mens.Buf = 0; // Para não desalocar memória em mens.Buf
    // Anota o arquivo correspondente à classe
        TArqMapa * arq = TArqMapa::Procura(nomearq);
        if (arq==0)
            arq = new TArqMapa(nomearq);
        obj->Arquivo = arq;
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
        int linha = v[3].getInt();
        if (linha<1)
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto("Nada foi apagado");
        }
        texto2 = TMudarAux::ProcuraInstr(texto1, v[2].getTxt());
        if (texto2[0]==0 && texto2[1]==0)
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto("Função inexistente");
        }
        const char * fim = TMudarAux::AvancaInstr(texto2);
        while (linha>0 && texto2<fim)
            linha--, texto2+=Num16(texto2);
        if (texto2>=fim)
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto("Nada foi apagado");
        }
    }
// Verifica se alguma linha será apagada
    if (texto2[0]==0 && texto2[1]==0)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto("Nada foi apagado");
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
    char * texto2 = 0;
    if (Instr::VarAtual == v+3)
    {
        int linha = v[2].getInt() - 1;
        texto2 = texto1;
        while (linha>0 && (texto2[0] || texto2[1]))
            linha--, texto2 += Num16(texto2);
        nome = v[3].getTxt();
    }
// Instruções em variável/função
    else
    {
        int linha = v[3].getInt();
        if (linha<1) linha=1;
        texto2 = TMudarAux::ProcuraInstr(texto1, v[2].getTxt());
        if (texto2[0]==0 && texto2[1]==0)
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto("Função inexistente");
        }
        if (texto2[2]!=Instr::cFunc && texto2[2]!=Instr::cVarFunc)
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto("Não é função");
        }
        const char * fim = TMudarAux::AvancaInstr(texto2);
        while (linha>0 && texto2<fim)
            linha--, texto2 += Num16(texto2);
        nome = v[4].getTxt();
    }
// Codifica as instruções
    TAddBuffer mens;
    if (!TMudarAux::CodifInstr(&mens, nome)) // Checa se erro
    {
        mens.Add("\x00", 1); // Zero no fim da mensagem
        mens.AnotarBuf();    // Anota resultado em mens.Buf
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(mens.Buf);
    }
    if (mens.Total==0) // Nenhuma instrução
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto("");
    }
    mens.AnotarBuf();  // Anota resultado em mens.Buf
// Instruções em função: não permite definições de constantes e funções
    if (Instr::VarAtual > v+3)
    {
        for (const char * p = mens.Buf; p < mens.Buf+mens.Total; p += Num16(p))
            switch (p[2])
            {
            case Instr::cConstNulo:
            case Instr::cConstTxt:
            case Instr::cConstNum:
            case Instr::cConstExpr:
            case Instr::cConstVar:
                Instr::ApagarVar(v);
                return Instr::CriarVarTexto(
                        "Definição de constante de função");
            case Instr::cFunc:
            case Instr::cVarFunc:
                Instr::ApagarVar(v);
                return Instr::CriarVarTexto(
                        "Definição de função dentro de função");
            }
    }
// Prepara bloco
    mudarcom.AddBloco(texto1, texto2-texto1);
    mudarcom.AddBloco(mens.Buf, mens.Total);
    mudarcom.AddBloco(texto2, TMudarAux::FimInstr(texto2) - texto2);
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

//------------------------------------------------------------------------------
bool TVarProg::FuncSalvar(TVariavel * v)
{
// Indica que deve salvar
    if (TMudarClasse::Salvar==0)
        TMudarClasse::Salvar=1;
// Acerta valor padrão das variáveis
    TArqMapa::ParamLinha = 4000;
    TArqMapa::ParamN = 0;
    TArqMapa::ParamIndent = 2;
    TArqMapa::ParamClasse = 1;
    TArqMapa::ParamFunc = 1;
    TArqMapa::ParamVar = 0;
// Obtém variáveis de acordo com texto
    if (Instr::VarAtual < v+1)
        return false;
    char comando = 0;
    int  valor = 0;
    for (const char * p = v[1].getTxt();; p++)
    {
        if (*p>='0' && *p<='9')
        {
            valor = valor * 10 + *p - '0';
            if (valor>10000)
                valor=10000;
            continue;
        }
        switch (comando | 0x20)
        {
        case 'l':
            TArqMapa::ParamLinha = (valor<70 ? 70 : valor>4000 ? 4000 : valor);
            break;
        case 'n':
            TArqMapa::ParamN = (valor<2 ? valor : 2);
            break;
        case 'i':
            TArqMapa::ParamIndent = (valor<8 ? valor : 8);
            break;
        case 'c':
            TArqMapa::ParamClasse = (valor<10 ? valor : 10);
            break;
        case 'f':
            TArqMapa::ParamFunc = (valor<10 ? valor : 10);
            break;
        case 'v':
            TArqMapa::ParamVar = (valor<10 ? valor : 10);
            break;
        }
        valor=0;
        comando = *p;
        if (comando==0)
            break;
    }
    return false;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncSalvarTudo(TVariavel * v)
{
    bool ret = FuncSalvar(v);
    TMudarClasse::Salvar=2;
    return ret;
}
