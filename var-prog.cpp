/* Este arquivo � software livre; voc� pode redistribuir e/ou
 * modificar nos termos da licen�a LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "var-prog.h"
#include "var-arqprog.h"
#include "variavel.h"
#include "variavel-def.h"
#include "classe.h"
#include "mudarclasse.h"
#include "arqmapa.h"
#include "instr.h"
#include "instr-enum.h"
#include "misc.h"

//------------------------------------------------------------------------------
TVarProg * TVarProg::Inicio = nullptr;

//----------------------------------------------------------------------------
const TVarInfo * TVarProg::Inicializa()
{
    static TVarInfo::FuncItem ListaFuncEnd[] = {
        { "apagar",       &TVarProg::FuncApagar },
        { "apagarlin",    &TVarProg::FuncApagarLin },
        { "arqnome",      &TVarProg::FuncArqNome },
        { "arquivo",      &TVarProg::FuncArquivo },
        { "clantes",      &TVarProg::FuncClAntes },
        { "classe",       &TVarProg::FuncClasse },
        { "cldepois",     &TVarProg::FuncClDepois },
        { "clfim",        &TVarProg::FuncClFim },
        { "clini",        &TVarProg::FuncClIni },
        { "const",        &TVarProg::FuncConst },
        { "criar",        &TVarProg::FuncCriar },
        { "criarlin",     &TVarProg::FuncCriarLin },
        { "depois",       &TVarProg::FuncDepois },
        { "existe",       &TVarProg::FuncExiste },
        { "fantes",       &TVarProg::FuncFAntes },
        { "fdepois",      &TVarProg::FuncFDepois },
        { "iniarq",       &TVarProg::FuncIniArq },
        { "iniclasse",    &TVarProg::FuncIniClasse },
        { "inifunc",      &TVarProg::FuncIniFunc },
        { "inifunccl",    &TVarProg::FuncIniFuncCl },
        { "inifunctudo",  &TVarProg::FuncIniFuncTudo },
        { "iniherda",     &TVarProg::FuncIniHerda },
        { "iniherdainv",  &TVarProg::FuncIniHerdaInv },
        { "iniherdatudo", &TVarProg::FuncIniHerdaTudo },
        { "inilinha",     &TVarProg::FuncIniLinha },
        { "lin",          &TVarProg::FuncLin },
        { "nivel",        &TVarProg::FuncNivel },
        { "renomear",     &TVarProg::FuncRenomear },
        { "salvar",       &TVarProg::FuncSalvar },
        { "salvartudo",   &TVarProg::FuncSalvarTudo },
        { "texto",        &TVarProg::FuncTexto },
        { "varcomum",     &TVarProg::FuncVarComum },
        { "varlugar",     &TVarProg::FuncVarLugar },
        { "varnum",       &TVarProg::FuncVarNum },
        { "varsav",       &TVarProg::FuncVarSav },
        { "vartexto",     &TVarProg::FuncVarTexto },
        { "vartipo",      &TVarProg::FuncVarTipo },
        { "varvetor",     &TVarProg::FuncVarVetor }  };
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

//------------------------------------------------------------------------------
inline void TVarProg::Criar()
{
    consulta = prNada;
}

//------------------------------------------------------------------------------
inline void TVarProg::Apagar()
{
    if (consulta == prNada)
        return;
    (Antes ? Antes->Depois : Inicio) = Depois;
    if (Depois)
        Depois->Antes = Antes;
    consulta = prNada;
}

//------------------------------------------------------------------------------
inline void TVarProg::Mover(TVarProg * destino)
{
    if (consulta == prNada)
    {
        destino->consulta = prNada;
        return;
    }
    (Antes ? Antes->Depois : Inicio) = destino;
    if (Depois)
        Depois->Antes = destino;
    memmove(destino, this, sizeof(TVarProg));
}

//------------------------------------------------------------------------------
inline int TVarProg::getValor()
{
    return 0;
}

//------------------------------------------------------------------------------
void TVarProg::LimparVar()
{
// Retira todos os objetos da lista ligada
    for (TVarProg * obj = Inicio; obj; obj = obj->Depois)
        obj->consulta = prNada;
    Inicio = nullptr;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncExiste(TVariavel * v)
{
    int existe = 0;
    if (Instr::VarAtual >= v + 1)
    {
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl)
        {
            if (Instr::VarAtual == v + 1)
                existe = 1;
            else
            {
                int x = cl->IndiceNome(v[2].getTxt());
                if (x >= 0)
                    existe = (cl->IndiceVar[x] & 0x800000 ? 1 : 2);
            }
        }
    }
    return Instr::CriarVarInt(v, existe);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncArquivo(TVariavel * v)
{
    const char * mens = "";
    while (true)
    {
        if (Instr::VarAtual < v + 1)
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
bool TVarProg::FuncArqNome(TVariavel * v)
{
    const char * p = "";
    if (Instr::VarAtual >= v + 1)
        p = v[1].getTxt();
    if (*p == 0)
        p = TArqIncluir::ArqNome();
    mprintf(arqinicio, INT_NOME_TAM, "%s." INT_EXT, p);
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(arqinicio);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncVarComum(TVariavel * v)
{
    bool valor=false;
    while (true)
    {
        if (Instr::VarAtual < v + 2)
            break;
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl == nullptr)
            break;
        int indice = cl->IndiceNome(v[2].getTxt());
        if (indice >= 0)
            valor = cl->InstrVar[indice][Instr::endProp] & 1;
        break;
    }
    return Instr::CriarVarInt(v, valor);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncVarSav(TVariavel * v)
{
    bool valor = false;
    while (true)
    {
        if (Instr::VarAtual < v + 2)
            break;
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl == nullptr)
            break;
        int indice = cl->IndiceNome(v[2].getTxt());
        if (indice >= 0)
            valor = cl->InstrVar[indice][Instr::endProp] & 2;
        break;
    }
    return Instr::CriarVarInt(v, valor);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncVarNum(TVariavel * v)
{
    bool valor=false;
    while (true)
    {
        if (Instr::VarAtual < v + 2)
            break;
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl == nullptr)
            break;
        int indice = cl->IndiceNome(v[2].getTxt());
        if (indice >= 0)
            valor = (cl->InstrVar[indice][2] == Instr::cConstNum);
        break;
    }
    return Instr::CriarVarInt(v, valor);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncVarTexto(TVariavel * v)
{
    bool valor=false;
    while (true)
    {
        if (Instr::VarAtual < v + 2)
            break;
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl == nullptr)
            break;
        int indice = cl->IndiceNome(v[2].getTxt());
        if (indice >= 0)
            valor = (cl->InstrVar[indice][2] == Instr::cConstTxt);
        break;
    }
    return Instr::CriarVarInt(v, valor);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncVarTipo(TVariavel * v)
{
    char mens[32] = "";
    while (true)
    {
        if (Instr::VarAtual < v + 2)
            break;
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl == nullptr)
            break;
        int indice = cl->IndiceNome(v[2].getTxt());
        if (indice >= 0)
            copiastr(mens, Instr::NomeInstr(cl->InstrVar[indice]),
                     sizeof(mens));
        break;
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(mens);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncVarLugar(TVariavel * v)
{
    int valor = 0;
    while (true)
    {
        if (Instr::VarAtual < v + 2)
            break;
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl == nullptr)
            break;
        int indice = cl->IndiceNome(v[2].getTxt());
        if (indice >= 0)
        {
            switch (cl->InstrVar[indice][2])
            {
            case Instr::cConstNulo:
            case Instr::cConstTxt:
            case Instr::cConstNum:
            case Instr::cConstExpr:
            case Instr::cConstVar:
                valor = 2;
                break;
            case Instr::cFunc:
            case Instr::cVarFunc:
                valor = 3;
                break;
            default:
                valor = 1;
            }
        }
        break;
    }
    return Instr::CriarVarInt(v, valor);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncVarVetor(TVariavel * v)
{
    int valor=0;
    while (true)
    {
        if (Instr::VarAtual < v + 2)
            break;
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl == nullptr)
            break;
        int indice = cl->IndiceNome(v[2].getTxt());
        if (indice >= 0)
            valor = (unsigned char)cl->InstrVar[indice][Instr::endVetor];
        break;
    }
    return Instr::CriarVarInt(v, valor);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncConst(TVariavel * v)
{
    while (true)
    {
        if (Instr::VarAtual < v + 2)
            break;
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl == nullptr)
            break;
        int indice = cl->IndiceNome(v[2].getTxt());
        if (indice < 0)
            break;
        const char * txt = cl->InstrVar[indice];
    // Constante num�rica
        if (txt[2] == Instr::cConstNum)
        {
            TVariavel var;
            var.defvar = txt;
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(var.getTxt());
        }
    // Constante de texto
        if (txt[2] != Instr::cConstTxt)
            break;
        txt += txt[Instr::endIndice] + 1;
    // Cria vari�vel
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrTxtFixo))
            return false;
        if (Instr::DadosTopo >= Instr::DadosFim)
            return false;
    // Anota o texto
        char * destino = Instr::DadosTopo;
        for (; *txt && destino <= Instr::DadosFim - 3; txt++)
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
    // Acerta a vari�vel
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
bool TVarProg::FuncClasse(TVariavel * v)
{
    if (Instr::VarAtual < v + 2)
        return false;
    TClasse * cl = TClasse::Procura(v[1].getTxt());

// Codifica as instru��es
    TAddBuffer mens;
    bool codifok = TMudarAux::CodifInstr(&mens, v[2].getTxt());
    Instr::ApagarVar(v);
    mens.Add("\x00\x00", 2); // Marca o fim das instru��es
    mens.AnotarBuf();        // Anota resultado em mens.Buf
    if (!codifok) // Erro
        return Instr::CriarVarTexto(mens.Buf);

// Verifica se bloco v�lido
    int linha = 1;
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
            return Instr::CriarVarTexto(txt1);
        }
        com += Num16(com), linha++;
    }

// Checa se est� igual � classe
    if (!cl)
        return Instr::CriarVarTxtFixo("0");
    const char * p1 = mens.Buf;
    const char * p2 = cl->Comandos;
    while (p1[0] || p1[1])
    {
        if (!Instr::ComparaInstr(p1, p2))
            return Instr::CriarVarTxtFixo("0");
        int total = Num16(p1);
        p1 += total, p2 += total;
    }
    return Instr::CriarVarTexto(p2[0] || p2[1] ? "0" : "1");
}

//------------------------------------------------------------------------------
void TVarProg::MudaConsulta(TProgConsulta valor)
{
    if (valor == prNada)
    {
        if (consulta == prNada)
            return;
        consulta = prNada;
        (Antes ? Antes->Depois : Inicio) = Depois;
        if (Depois)
            Depois->Antes = Antes;
        return;
    }
    if (consulta == prNada)
    {
        Antes = nullptr;
        Depois = Inicio;
        Inicio = this;
        if (Depois)
            Depois->Antes = this;
    }
    consulta = valor;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncIniArq(TVariavel * v)
{
    TVarProg * ref = reinterpret_cast<TVarProg*>(v->endvar) + v->indice;
    TProgConsulta valor = prNada;
    if (Instr::VarAtual < v + 1)
    {
        ref->ArqAtual = TArqMapa::Inicio;
        if (ref->ArqAtual)
            valor = prArquivo;
    }
    else
    {
        const char * texto = v[1].getTxt();
        TArqMapa * arq = TArqMapa::Inicio;
        ref->ClasseAtual = nullptr;
        for (; arq; arq = arq->Proximo)
            if (comparaZ(texto, arq->Arquivo) == 0)
            {
                ref->ClasseAtual = arq->ClInicio;
                if (ref->ClasseAtual)
                    valor = prArqCl;
                break;
            }
    }
    ref->MudaConsulta(valor);
    return Instr::CriarVarInt(v, valor != prNada);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncIniClasse(TVariavel * v)
{
    TVarProg * ref = reinterpret_cast<TVarProg*>(v->endvar) + v->indice;
    TProgConsulta valor = prNada;
    const char * texto = "";
    if (Instr::VarAtual >= v + 1)
        texto = v[1].getTxt();
    if (*texto == 0)
    {
        ref->ClasseAtual = TClasse::RBfirst();
        ref->ClasseFim = TClasse::RBlast();
        if (ref->ClasseAtual)
            valor = prClasse;
    }
    else
    {
        ref->ClasseAtual = TClasse::ProcuraIni(texto);
        if (ref->ClasseAtual)
            valor = prClasse, ref->ClasseFim = TClasse::ProcuraFim(texto);
    }
    ref->MudaConsulta(valor);
    return Instr::CriarVarInt(v, valor != prNada);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncIniFunc(TVariavel * v)
{
    TVarProg * ref = reinterpret_cast<TVarProg*>(v->endvar) + v->indice;
    TProgConsulta valor = prNada;
    while (Instr::VarAtual >= v + 1)
    {
        ref->Classe = TClasse::Procura(v[1].getTxt());
        if (ref->Classe == nullptr)
            break;
        const char * texto = "";
        if (Instr::VarAtual >= v + 2)
            texto = v[2].getTxt();
        if (*texto)
        {
            ref->ValorAtual = ref->Classe->IndiceNomeIni(texto);
            if (ref->ValorAtual < 0)
                break;
            ref->ValorFim = ref->Classe->IndiceNomeFim(texto);
        }
        else
        {
            if (ref->Classe->NumVar == 0)
                break;
            ref->ValorAtual = 0, ref->ValorFim = ref->Classe->NumVar - 1;
        }
        while ((ref->Classe->IndiceVar[ref->ValorAtual] & 0x800000) == 0)
            if (++ref->ValorAtual > ref->ValorFim)
                break;
        while ((ref->Classe->IndiceVar[ref->ValorFim] & 0x800000) == 0)
            if (--ref->ValorFim < ref->ValorAtual)
                break;
        if (ref->ValorAtual <= ref->ValorFim)
            valor = prFunc;
        break;
    }
    ref->MudaConsulta(valor);
    return Instr::CriarVarInt(v, valor != 0);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncIniFuncTudo(TVariavel * v)
{
    TVarProg * ref = reinterpret_cast<TVarProg*>(v->endvar) + v->indice;
    TProgConsulta valor = prNada;
    while (Instr::VarAtual >= v + 1)
    {
        ref->Classe = TClasse::Procura(v[1].getTxt());
        if (ref->Classe == nullptr)
            break;
        const char * texto = "";
        if (Instr::VarAtual >= v + 2)
            texto = v[2].getTxt();
        if (*texto)
        {
            ref->ValorAtual = ref->Classe->IndiceNomeIni(texto);
            if (ref->ValorAtual >= 0)
            {
                valor = prFuncTudo;
                ref->ValorFim = ref->Classe->IndiceNomeFim(texto);
            }
        }
        else if (ref->Classe->NumVar)
        {
            valor = prFuncTudo;
            ref->ValorAtual = 0;
            ref->ValorFim = ref->Classe->NumVar - 1;
        }
        break;
    }
    ref->MudaConsulta(valor);
    return Instr::CriarVarInt(v, valor != 0);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncIniHerda(TVariavel * v)
{
    TVarProg * ref = reinterpret_cast<TVarProg*>(v->endvar) + v->indice;
    TProgConsulta valor = prNada;
    while (Instr::VarAtual >= v + 1)
    {
        ref->Classe = TClasse::Procura(v[1].getTxt());
        if (ref->Classe == nullptr)
            break;
        const char * txt = ref->Classe->Comandos;
        if (txt[0] == 0 && txt[1] == 0)
            break;
        if (txt[2] != Instr::cHerda || txt[Instr::endVar] == 0)
            break;
        ref->TextoAtual = txt + Instr::endVar + 1;
        ref->ValorFim = (unsigned char)txt[Instr::endVar];
        valor = prHerda;
        break;
    }
    ref->MudaConsulta(valor);
    return Instr::CriarVarInt(v, valor != 0);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncIniHerdaTudo(TVariavel * v)
{
    TVarProg * ref = reinterpret_cast<TVarProg*>(v->endvar) + v->indice;
    TProgConsulta valor = prNada;
    while (Instr::VarAtual >= v + 1)
    {
        ref->Classe = TClasse::Procura(v[1].getTxt());
        if (ref->Classe == nullptr)
            break;
        int total = ref->Classe->Heranca(ref->ClasseHerda, HERDA_TAM);
        if (total <= 0)
            break;
        if (total<HERDA_TAM)
            ref->ClasseHerda[total] = nullptr;
        ref->ValorAtual = 0;
        valor = prHerdaTudo;
        break;
    }
    ref->MudaConsulta(valor);
    return Instr::CriarVarInt(v, valor != 0);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncIniHerdaInv(TVariavel * v)
{
    TVarProg * ref = reinterpret_cast<TVarProg*>(v->endvar) + v->indice;
    TProgConsulta valor = prNada;
    while (Instr::VarAtual >= v + 1)
    {
        ref->Classe = TClasse::Procura(v[1].getTxt());
        if (ref->Classe == nullptr)
            break;
        if (ref->Classe->NumDeriv == 0)
            break;
        ref->PontAtual = ref->Classe->ListaDeriv;
        ref->PontFim = ref->PontAtual + ref->Classe->NumDeriv;
        valor = prHerdaInv;
        break;
    }
    ref->MudaConsulta(valor);
    return Instr::CriarVarInt(v, valor != 0);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncIniLinha(TVariavel * v)
{
    TVarProg * ref = reinterpret_cast<TVarProg*>(v->endvar) + v->indice;
    TProgConsulta valor = prNada;
    while (Instr::VarAtual >= v + 1)
    {
        ref->Classe = TClasse::Procura(v[1].getTxt());
        if (ref->Classe == nullptr)
            break;
    // Linhas de uma classe
        if (Instr::VarAtual < v + 2)
        {
            ref->TextoAtual = ref->Classe->Comandos;
            if (ref->TextoAtual[0] == 0 && ref->TextoAtual[1] == 0)
                break;
            ref->ValorFim = 0;
            valor = prLinhaCl;
            break;
        }
    // Linhas de uma fun��o ou vari�vel
        int indice = ref->Classe->IndiceNome(v[2].getTxt());
        if (indice < 0)
            break;
        ref->TextoAtual = ref->Classe->InstrVar[indice];
        if (ref->TextoAtual[2] == Instr::cFunc ||
                ref->TextoAtual[2] == Instr::cVarFunc)
            valor = prLinhaFunc;
        else
            valor = prLinhaVar;
        ref->ValorFim = 1;
        break;
    }
    ref->MudaConsulta(valor);
    return Instr::CriarVarInt(v, valor != 0);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncIniFuncCl(TVariavel * v)
{
    TVarProg * ref = reinterpret_cast<TVarProg*>(v->endvar) + v->indice;
    TProgConsulta valor = prNada;
    const char * p = nullptr;
    while (Instr::VarAtual >= v + 1)
    {
        ref->Classe = TClasse::Procura(v[1].getTxt());
        if (ref->Classe == nullptr)
            break;
        for (p = ref->Classe->Comandos; p[0] || p[1]; p += Num16(p))
            if (p[2] >= Instr::cVariaveis)
            {
                valor = prFuncCl;
                break;
            }
        if (valor == prNada)
            break;
        const char * texto = "";
        if (Instr::VarAtual >= v + 2)
            texto = v[2].getTxt();
        if (*texto == 0)
        {
            ref->ValorFim = 0;
            break;
        }
        ref->ValorFim = strlen(texto);
        if (p[2] <= Instr::cVariaveis ||
                compara(texto, p + Instr::endNome, ref->ValorFim) != 0)
            p = Instr::ProximaInstr(p, texto, ref->ValorFim);
        if (p == nullptr)
            valor = prNada;
        break;
    }
    ref->TextoAtual = p;
    ref->MudaConsulta(valor);
    return Instr::CriarVarInt(v, valor != 0);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncDepois(TVariavel * v)
{
    TVarProg * ref = reinterpret_cast<TVarProg*>(v->endvar) + v->indice;
    int total = 1;
    if (Instr::VarAtual >= v + 1)
        total = v[1].getInt();
    switch (ref->consulta)
    {
    case prNada:
        break;
    case prArquivo: // iniarq com nome de arquivo
        for (; total > 0; total--)
        {
            ref->ArqAtual = ref->ArqAtual->Proximo;
            if (ref->ArqAtual == 0)
            {
                ref->MudaConsulta(prNada);
                break;
            }
        }
        break;
    case prArqCl:   // iniarq com nome de classe
        for (; total > 0; total--)
        {
            ref->ClasseAtual = ref->ClasseAtual->ArqDepois;
            if (ref->ClasseAtual == nullptr)
            {
                ref->MudaConsulta(prNada);
                break;
            }
        }
        break;
    case prClasse:  // iniclasse
        for (; total > 0; total--)
        {
            if (ref->ClasseAtual == ref->ClasseFim)
            {
                ref->MudaConsulta(prNada);
                break;
            }
            ref->ClasseAtual = TClasse::RBnext(ref->ClasseAtual);
            if (ref->ClasseAtual == nullptr)
            {
                ref->MudaConsulta(prNada);
                break;
            }
        }
        break;
    case prFunc:  // inifunc
        while (total > 0)
        {
            ref->ValorAtual++;
            if (ref->ValorAtual > ref->ValorFim)
            {
                ref->MudaConsulta(prNada);
                break;
            }
            if (ref->Classe->IndiceVar[ref->ValorAtual] & 0x800000)
                total--;
        }
        break;
    case prFuncTudo:  // inifunctudo
        if (total <= 0)
            break;
        if (total >= 0xFFFF || ref->ValorAtual + total > ref->ValorFim)
            ref->MudaConsulta(prNada);
        else
            ref->ValorAtual += total;
        break;
    case prHerda:  // iniherda
        if (ref->ValorFim <= total)
            ref->MudaConsulta(prNada);
        else for (; total > 0; total--)
        {
            ref->ValorFim--;
            while (*ref->TextoAtual++);
        }
        break;
    case prHerdaTudo:  // iniherdatudo
        if (total <= 0)
            break;
        ref->ValorAtual += total;
        if (total >= HERDA_TAM || ref->ValorAtual >= HERDA_TAM)
            ref->MudaConsulta(prNada);
        else if (ref->ClasseHerda[ref->ValorAtual] == 0)
            ref->MudaConsulta(prNada);
        break;
    case prHerdaInv:  // iniherdainv
        if (total <= 0)
            break;
        ref->PontAtual += total;
        if (total >= 0xFFFFFF || ref->PontAtual >= ref->PontFim)
            ref->MudaConsulta(prNada);
        break;
    case prLinhaCl:     // inilinha com classe
    case prLinhaFunc:   // inilinha com fun��o
        for (; total > 0 && ref->consulta != prNada; total--)
        {
            ref->TextoAtual += Num16(ref->TextoAtual);
            if (ref->TextoAtual[0] == 0 && ref->TextoAtual[1] == 0)
                ref->MudaConsulta(prNada);
            else switch (ref->TextoAtual[2])
            {
            case Instr::cConstNulo:
            case Instr::cConstTxt:
            case Instr::cConstNum:
            case Instr::cConstExpr:
            case Instr::cConstVar:
            case Instr::cFunc:
            case Instr::cVarFunc:
                if (ref->consulta == prLinhaFunc)
                    ref->MudaConsulta(prNada);
            }
        }
        break;
    case prLinhaVar:  // inilinha com vari�vel
        ref->MudaConsulta(prNada);
        break;
    case prFuncCl:    // inifunccl
        for (; total > 0 && ref->consulta != prNada; total--)
        {
            const char * p = ref->TextoAtual;
            p = Instr::ProximaInstr(p, p + Instr::endNome, ref->ValorFim);
            if (p == nullptr)
                ref->MudaConsulta(prNada);
            else
                ref->TextoAtual = p;
            break;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncTexto(TVariavel * v)
{
    TVarProg * ref = reinterpret_cast<TVarProg*>(v->endvar) + v->indice;
    const char * p = "";
    switch (ref->consulta)
    {
    case prNada:
        break;
    case prArquivo: // iniarq com nome de arquivo
        p = ref->ArqAtual->Arquivo;
        break;
    case prArqCl:   // iniarq com nome de classe
    case prClasse:  // iniclasse
        p = ref->ClasseAtual->Nome;
        break;
    case prFunc:     // inifunc
    case prFuncTudo: // inifunctudo
        p = ref->Classe->InstrVar[ref->ValorAtual] + Instr::endNome;
        break;
    case prHerda:  // iniherda
        p = ref->TextoAtual;
        break;
    case prHerdaTudo:  // iniherdatudo
        p = ref->ClasseHerda[ref->ValorAtual]->Nome;
        break;
    case prHerdaInv:  // iniherdainv
        p = ref->PontAtual[0]->Nome;
        break;
    case prLinhaCl:   // inilinha com classe
    case prLinhaFunc: // inilinha com fun��o
    case prLinhaVar:  // inilinha com vari�vel
        {
        // Cria vari�vel
            const char * instr = ref->TextoAtual;
            Instr::ApagarVar(v);
            if (!Instr::CriarVar(Instr::InstrTxtFixo))
                return false;
            if (Instr::DadosTopo >= Instr::DadosFim)
                return false;
        // Anota o texto
            char * destino = Instr::DadosTopo;
            Instr::Decod(destino, instr, Instr::DadosFim-destino);
            while (*destino++);
        // Acerta a vari�vel
            Instr::VarAtual->endvar = Instr::DadosTopo;
            Instr::VarAtual->tamanho = destino - Instr::DadosTopo;
            Instr::DadosTopo = destino;
            return true;
        }
    case prFuncCl:  // inifunccl
        p = ref->TextoAtual + Instr::endNome;
        break;
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
    TVarProg * ref = reinterpret_cast<TVarProg*>(v->endvar) + v->indice;
    return Instr::CriarVarInt(v, ref->consulta != prNada);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncNivel(TVariavel * v)
{
    TVarProg * ref = reinterpret_cast<TVarProg*>(v->endvar) + v->indice;
    int valor = prNada;
    if (ref->consulta == prLinhaCl || ref->consulta == prLinhaFunc)
        valor = (unsigned char)ref->TextoAtual[Instr::endAlin];
    return Instr::CriarVarInt(v, valor);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncApagar(TVariavel * v)
{
    int valor = 0;
    while (Instr::VarAtual >= v + 1)
    {
        const char * nome = v[1].getTxt();
        TClasse * cl = TClasse::Procura(nome);
        TMudarClasse * obj = TMudarClasse::Procurar(nome);
    // Apagar uma classe
        if (Instr::VarAtual < v + 2)
        {
            if (cl)
            {
                if (obj == nullptr)
                    obj = new TMudarClasse(nome);
                else if (obj->InfoApagar())
                    break;
                obj->MudarComandos(nullptr);
                obj->MarcarApagar();
            }
            else if (obj)
                delete obj;
            else
                break;
            valor = 1;
            break;
        }
    // Apagar uma vari�vel ou fun��o
        char * texto1 = nullptr; // Endere�o do primeiro bloco
        int tamanho1 = 0; // Tamanho do primeiro bloco
        char * texto2 = nullptr; // Endere�o do segundo bloco
        int tamanho2 = 0; // Tamanho do segundo bloco
    // Obt�m: texto1 = instru��es da classe
        if (obj && obj->Comandos != nullptr) // Comandos definidos em TMudarClasse
            texto1 = obj->Comandos;
        else if (cl)    // Comandos definidos em TClasse
            texto1 = cl->Comandos;
        else            // Nenhum comando definido, retorna
            break;
    // Obt�m bloco de instru��es
        texto2 = TMudarAux::ProcuraInstr(texto1, v[2].getTxt());
        if (texto2[0] == 0 && texto2[1] == 0) // N�o encontrou
            break;
        tamanho1 = texto2 - texto1;
        texto2 = TMudarAux::AvancaInstr(texto2);
        tamanho2 = TMudarAux::FimInstr(texto2) - texto2 + 2;
    // Anota as altera��es em objeto TMudarClasse
        if (obj == nullptr)
            obj = new TMudarClasse(v[1].getTxt());
        char * p = new char[tamanho1+tamanho2];
        memcpy(p, texto1, tamanho1);
        memcpy(p+tamanho1, texto2, tamanho2);
        obj->MudarComandos(p);
        valor = 1;
        break;
    }
    return Instr::CriarVarInt(v, valor != 0);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncCriar(TVariavel * v)
{
// Criar uma vari�vel ou fun��o
    if (Instr::VarAtual >= v + 2)
    {
        //puts("Criar vari�vel"); fflush(stdout);
        const char * nomeclasse = v[1].getTxt();
        TClasse * cl = TClasse::Procura(nomeclasse);
        TMudarClasse * obj = TMudarClasse::Procurar(nomeclasse);
        TMudarAux mudarcom; // Para mudar a lista de instru��es
    // Obt�m: texto1 = instru��es da classe
        char * texto1 = nullptr;
        if (obj && obj->Comandos != nullptr) // Comandos definidos em TMudarClasse
            texto1 = obj->Comandos, nomeclasse = obj->Nome;
        else if (cl)    // Comandos definidos em TClasse
            texto1 = cl->Comandos, nomeclasse = cl->Nome;
        else            // Nenhum comando definido, retorna
            return Instr::CriarVarTxtFixo(v, "Classe n�o existe");
    // Codifica as instru��es
        TAddBuffer mens;
        bool codifok = TMudarAux::CodifInstr(&mens, v[2].getTxt());
        if (mens.Total == 0) // Nenhuma altera��o
            return Instr::CriarVarTxtFixo(v, "");
        mens.Add("\x00\x00", 2); // Marca o fim das instru��es
        mens.AnotarBuf();        // Anota resultado em mens.Buf
        if (!codifok) // Erro
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(mens.Buf);
        }
        char * p = TMudarAux::AvancaInstr(mens.Buf); // Passa para pr�xima fun��o/vari�vel
        if (p[0] || p[1])
            return Instr::CriarVarTxtFixo(v,
                    "Defini��o de mais de uma fun��o/vari�vel/herda");
        if (mens.Buf[2] == Instr::cHerda)
        {
            mudarcom.AddBloco(mens.Buf, mens.Total-2);
            if ((texto1[0] || texto1[1]) && texto1[2] == Instr::cHerda)
                texto1 += Num16(texto1);
            mudarcom.AddBloco(texto1, TMudarAux::FimInstr(texto1) - texto1);
        }
        else if (mens.Buf[2] > Instr::cVariaveis)
        {
        // Obt�m o lugar ideal:
        // 1=no in�cio, 2=nas defini��es de fun��o, 3=em qualquer lugar
            int lugar = 1;
            switch (mens.Buf[2])
            {
            case Instr::cFunc:
            case Instr::cVarFunc:
                lugar = 2;
                break;
            case Instr::cConstNulo:
            case Instr::cConstTxt:
            case Instr::cConstNum:
            case Instr::cConstExpr:
            case Instr::cConstVar:
                lugar = 3;
                break;
            }
        // Obt�m nova lista de instru��es
            char * p = texto1;
            int l = 1; // Lugar atual
            while (p[0] || p[1])
            {
            // N�o � vari�vel: prossegue
                if (p[2] <= Instr::cVariaveis)
                {
                    p += Num16(p);
                    continue;
                }
            // Verifica se � in�cio da lista de fun��es
                if (p[2] == Instr::cFunc || p[2] == Instr::cVarFunc)
                {
                    l = 2;
                    if (lugar == 1) // � vari�vel: deve vir antes
                    {
                                    // Anota o bloco atual
                        mudarcom.AddBloco(texto1, p-texto1);
                                    // Anota a vari�vel
                        mudarcom.AddBloco(mens.Buf, mens.Total - 2);
                        lugar=0; // Indica que j� anotou a vari�vel
                        texto1 = p;
                    }
                }
            // Verifica se � a instru��o procurada
                if (comparaVar(p + Instr::endNome,
                                    mens.Buf + Instr::endNome) == 0)
                {
                    mudarcom.AddBloco(texto1, p-texto1);// Anota bloco atual
                    if ((l & lugar) != 0) // Anota vari�vel se est� no lugar certo
                    {
                        mudarcom.AddBloco(mens.Buf, mens.Total - 2);
                        lugar = 0; // Indica que j� anotou a vari�vel
                    }
                    texto1 = p = TMudarAux::AvancaInstr(p); // Avan�a p, acerta texto1
                    continue;
                }
            // Passa para pr�xima instru��o
                p = TMudarAux::AvancaInstr(p);
            } // while (p[0] || p[1])
            mudarcom.AddBloco(texto1, p - texto1);// Anota bloco atual
            if (lugar) // Anota vari�vel
                mudarcom.AddBloco(mens.Buf, mens.Total - 2);
        }
        else
            return Instr::CriarVarTxtFixo(v, "Faltou o nome da fun��o/vari�vel");
    // Verifica se bloco v�lido
        char err[200];
        if (!mudarcom.ChecaBloco(err, sizeof(err)))
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(err);
        }
    // Anota altera��es em objeto TMudarClasse
        if (obj == nullptr)
            obj = new TMudarClasse(nomeclasse);
        mudarcom.AnotaBloco(obj);
    }
// Criar uma classe
    else if (Instr::VarAtual >= v + 1)
    {
        //puts("Criar classe"); fflush(stdout);
        char nomearq[INT_NOME_TAM + 30];
        char nomeclasse[CLASSE_NOME_TAM + 30];
        const char * nome = v[1].getTxt();
    // Obt�m o nome da classe
        char * p = nomeclasse;
        while (*nome && *nome != Instr::ex_barra_n &&
                p < nomeclasse + sizeof(nomeclasse) - 1)
            *p++ = *nome++;
        *p = 0;
        if (*nome == Instr::ex_barra_n)
            nome++;
    // Obt�m o nome do arquivo
        p = nomearq;
        while (*nome && *nome != Instr::ex_barra_n &&
                p < nomearq + sizeof(nomearq) - 1)
            *p++ = *nome++;
        *p = 0;
        if (*nome == Instr::ex_barra_n)
            nome++;
    // Checa se nome de arquivo v�lido
        if (*nomearq && !TArqMapa::NomeValido(nomearq))
            return Instr::CriarVarTxtFixo(v, "Nome de arquivo inv�lido");
        if (*nomearq && !TArqIncluir::ProcArq(nomearq))
            return Instr::CriarVarTxtFixo(v, "Arquivo n�o pertence ao programa");
    // Checa se nome de classe v�lido
        if (!TClasse::NomeValido(nomeclasse))
            return Instr::CriarVarTxtFixo(v, "Nome de classe inv�lido");
    // Codifica as instru��es
        TAddBuffer mens;
        bool codifok = TMudarAux::CodifInstr(&mens, nome);
        mens.Add("\x00\x00", 2); // Marca o fim das instru��es
        mens.AnotarBuf();        // Anota resultado em mens.Buf
        if (!codifok) // Erro
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(mens.Buf);
        }
    // Verifica se bloco v�lido
        int linha = 1;
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
            com += Num16(com), linha++;
        }
        const char * pfim = checalinha.Fim();
        if (pfim)
        {
            char txt1[1024];
            mprintf(txt1, sizeof(txt1), "%d: %s\n", linha, pfim);
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(txt1);
        }
    // Anota altera��es em objeto TMudarClasse
        TMudarClasse * obj = TMudarClasse::Procurar(nomeclasse);
        if (obj == nullptr)
            obj = new TMudarClasse(nomeclasse);
        obj->MudarComandos(mens.Buf);  // Transfere a mem�ria alocada
        mens.Buf = nullptr; // Para n�o desalocar mem�ria em mens.Buf
    // Anota o arquivo correspondente � classe
        TArqMapa * arq = TArqMapa::Procura(nomearq);
        if (arq == nullptr)
            arq = new TArqMapa(nomearq);
        obj->Arquivo = arq;
    }
    return Instr::CriarVarTxtFixo(v, "");
}

//------------------------------------------------------------------------------
bool TVarProg::FuncApagarLin(TVariavel * v)
{
    if (Instr::VarAtual < v + 2)
        return Instr::CriarVarTxtFixo(v, "");
    const char * nome = v[1].getTxt();
    TClasse * cl = TClasse::Procura(nome);
    TMudarClasse * obj = TMudarClasse::Procurar(nome);
// Vari�veis do bloco
    char * texto1 = nullptr; // Endere�o do primeiro bloco
    int tamanho1 = 0; // Tamanho do primeiro bloco
    char * texto2 = nullptr; // Endere�o do segundo bloco
    int tamanho2 = 0; // Tamanho do segundo bloco
// Obt�m: texto1 = instru��es da classe
    if (obj && obj->Comandos != nullptr) // Comandos definidos em TMudarClasse
        texto1 = obj->Comandos;
    else if (cl)    // Comandos definidos em TClasse
        texto1 = cl->Comandos;
    else            // Nenhum comando definido, retorna
        return Instr::CriarVarTxtFixo(v, "Classe inexistente");
// Apagar linha da classe
    if (Instr::VarAtual == v + 2)
    {
        int linha = v[2].getInt() - 1;
        texto2 = texto1;
        while (linha > 0 && (texto2[0] || texto2[1]))
            linha--, texto2 += Num16(texto2);
    }
// Apagar uma linha da vari�vel/fun��o
    else
    {
        int linha = v[3].getInt();
        if (linha < 1)
            return Instr::CriarVarTxtFixo(v, "Nada foi apagado");
        texto2 = TMudarAux::ProcuraInstr(texto1, v[2].getTxt());
        if (texto2[0] == 0 && texto2[1] == 0)
            return Instr::CriarVarTxtFixo(v, "Fun��o inexistente");
        const char * fim = TMudarAux::AvancaInstr(texto2);
        while (linha > 0 && texto2 < fim)
            linha--, texto2 += Num16(texto2);
        if (texto2 >= fim)
            return Instr::CriarVarTxtFixo(v, "Nada foi apagado");
    }
// Verifica se alguma linha ser� apagada
    if (texto2[0] == 0 && texto2[1] == 0)
        return Instr::CriarVarTxtFixo(v, "Nada foi apagado");
// Verifica se bloco v�lido
    int linha = 1;
    char * com = texto1;
    Instr::ChecaLinha checalinha;
    checalinha.Inicio();
    for (; com[0] || com[1]; com += Num16(com), linha++)
    {
        if (com == texto2)
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
// Obt�m bloco de instru��es
    tamanho1 = texto2 - texto1;
    texto2 += Num16(texto2);
    tamanho2 = com - texto2 + 2;
// Anota as altera��es em objeto TMudarClasse
    if (obj == nullptr)
        obj = new TMudarClasse(cl->Nome);
    char * p = new char[tamanho1+tamanho2];
    memcpy(p, texto1, tamanho1);
    memcpy(p + tamanho1, texto2, tamanho2);
    obj->MudarComandos(p);
    return Instr::CriarVarTxtFixo(v, "");
}

//------------------------------------------------------------------------------
bool TVarProg::FuncCriarLin(TVariavel * v)
{
    if (Instr::VarAtual < v + 3)
        return Instr::CriarVarTxtFixo(v, "");
    const char * nome = v[1].getTxt();
    TClasse * cl = TClasse::Procura(nome);
    TMudarClasse * obj = TMudarClasse::Procurar(nome);
    TMudarAux mudarcom; // Para mudar a lista de instru��es
// Obt�m: texto1 = instru��es da classe
    char * texto1 = nullptr;
    if (obj && obj->Comandos != nullptr) // Comandos definidos em TMudarClasse
        texto1 = obj->Comandos;
    else if (cl)    // Comandos definidos em TClasse
        texto1 = cl->Comandos;
    else            // Nenhum comando definido, retorna
        return Instr::CriarVarTxtFixo(v, "Classe inexistente");
// Instru��es na classe
    char * texto2 = nullptr;
    if (Instr::VarAtual == v + 3)
    {
        int linha = v[2].getInt() - 1;
        texto2 = texto1;
        while (linha > 0 && (texto2[0] || texto2[1]))
            linha--, texto2 += Num16(texto2);
        nome = v[3].getTxt();
    }
// Instru��es em vari�vel/fun��o
    else
    {
        int linha = v[3].getInt();
        if (linha < 1)
            linha = 1;
        texto2 = TMudarAux::ProcuraInstr(texto1, v[2].getTxt());
        if (texto2[0] == 0 && texto2[1] == 0)
            return Instr::CriarVarTxtFixo(v, "Fun��o inexistente");
        if (texto2[2] != Instr::cFunc && texto2[2] != Instr::cVarFunc)
            return Instr::CriarVarTxtFixo(v, "N�o � fun��o");
        const char * fim = TMudarAux::AvancaInstr(texto2);
        while (linha > 0 && texto2 < fim)
            linha--, texto2 += Num16(texto2);
        nome = v[4].getTxt();
    }
// Codifica as instru��es
    TAddBuffer mens;
    if (!TMudarAux::CodifInstr(&mens, nome)) // Checa se erro
    {
        mens.Add("\x00", 1); // Zero no fim da mensagem
        mens.AnotarBuf();    // Anota resultado em mens.Buf
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(mens.Buf);
    }
    if (mens.Total == 0) // Nenhuma instru��o
        return Instr::CriarVarTxtFixo(v, "");
    mens.AnotarBuf();  // Anota resultado em mens.Buf
// Instru��es em fun��o: n�o permite defini��es de constantes e fun��es
    if (Instr::VarAtual > v + 3)
    {
        for (const char * p = mens.Buf; p < mens.Buf + mens.Total; p += Num16(p))
            switch (p[2])
            {
            case Instr::cConstNulo:
            case Instr::cConstTxt:
            case Instr::cConstNum:
            case Instr::cConstExpr:
            case Instr::cConstVar:
                return Instr::CriarVarTxtFixo(v,
                        "Defini��o de constante de fun��o");
            case Instr::cFunc:
            case Instr::cVarFunc:
                return Instr::CriarVarTxtFixo(v,
                        "Defini��o de fun��o dentro de fun��o");
            }
    }
// Prepara bloco
    mudarcom.AddBloco(texto1, texto2-texto1);
    mudarcom.AddBloco(mens.Buf, mens.Total);
    mudarcom.AddBloco(texto2, TMudarAux::FimInstr(texto2) - texto2);
// Verifica se bloco v�lido
    char err[200];
    if (!mudarcom.ChecaBloco(err, sizeof(err)))
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(err);
    }
// Anota altera��es em objeto TMudarClasse
    if (obj == nullptr)
        obj = new TMudarClasse(cl->Nome);
    mudarcom.AnotaBloco(obj);
    return Instr::CriarVarTxtFixo(v, "");
}

//------------------------------------------------------------------------------
bool TVarProg::FuncFAntes(TVariavel * v)
{
    return FuncMudar(v, 0);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncFDepois(TVariavel * v)
{
    return FuncMudar(v, 1);
}

//------------------------------------------------------------------------------
/// Obt�m o endere�o ap�s o fim de uma fun��o ou vari�vel
/** @note Usado em TVarProg::FuncMudar */
const char * FuncMudarProximo(const char * p)
{
    if (p[0] == 0 && p[1] == 0)
        return p;
    if (p[2] != Instr::cFunc && p[2] != Instr::cVarFunc)
        return p + Num16(p);
    for (p += Num16(p); p[0] || p[1]; p += Num16(p))
        switch (p[2])
        {
        case Instr::cConstNulo:
        case Instr::cConstTxt:
        case Instr::cConstNum:
        case Instr::cConstExpr:
        case Instr::cConstVar:
        case Instr::cFunc:
        case Instr::cVarFunc:
            return p;
        }
    return p;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncMudar(TVariavel * v, int lugar)
{
    if (Instr::VarAtual < v + 2)
        return false;
// Obt�m a classe
    const char * nomeclasse = v[1].getTxt();
    TClasse * cl = TClasse::Procura(nomeclasse);
    TMudarClasse * obj = TMudarClasse::Procurar(nomeclasse);
// Obt�m: texto1 = instru��es da classe
    char * texto1 = nullptr;
    if (obj && obj->Comandos != nullptr) // Comandos definidos em TMudarClasse
        texto1 = obj->Comandos, nomeclasse = obj->Nome;
    else if (cl)    // Comandos definidos em TClasse
        texto1 = cl->Comandos, nomeclasse = cl->Nome;
    else            // Nenhum comando definido, retorna
    {
        char mens[100];
        mprintf(mens, sizeof(mens), "Classe n�o existe: %s", nomeclasse);
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(mens);
    }
// Obt�m: p1 = in�cio da primeira fun��o
    const char * nome1 = v[2].getTxt();
    const char * p1 = nullptr;
    for (const char * p = texto1; p[0] || p[1]; p += Num16(p))
        if (p[2] > Instr::cVariaveis)
            if (comparaVar(p + Instr::endNome, nome1) == 0)
            {
                p1 = p;
                break;
            }
    if (p1 == nullptr)
    {
        char mens[100];
        mprintf(mens, sizeof(mens), "Fun��o n�o existe: %s", nome1);
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(mens);
    }
// Obt�m: p2 = primeiro byte ap�s a primeira fun��o
    const char * p2 = FuncMudarProximo(p1);
// Obt�m: p3 = fim das instru��es da classe
    const char * p3 = p2;
    while (p3[0] || p3[1])
        p3 += Num16(p3);
// Obt�m: d1 = segunda fun��o (aonde colocar a primeira fun��o)
    const char * d1 = nullptr;
    if (Instr::VarAtual >= v + 3)
    {
        const char * nomefunc = v[3].getTxt();
        for (const char * p = texto1; p[0] || p[1]; p += Num16(p))
            if (p[2] > Instr::cVariaveis)
                if (comparaVar(p + Instr::endNome, nomefunc) == 0)
                {
                    d1 = p;
                    break;
                }
        if (d1 == nullptr)
        {
            char mens[100];
            mprintf(mens, sizeof(mens), "Fun��o n�o existe: %s", nomefunc);
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(mens);
        }
        if (lugar == 1)
            d1 = FuncMudarProximo(d1);
    }
    else if (lugar == 1) // Mover para o final
        d1 = p3;
    else  // Mover para o in�cio
        for (d1 = texto1; d1[0] || d1[1]; d1 += Num16(d1))
            if (d1[2] >= Instr::cVariaveis)
                break;
// Checa se n�o est� colocando fun��o antes da �ltima vari�vel
// ou vari�vel depois da primeira fun��o
    switch (p1[2])
    {
    case Instr::cConstNulo: // Constantes podem ir em qualquer lugar
    case Instr::cConstTxt:
    case Instr::cConstNum:
    case Instr::cConstExpr:
    case Instr::cConstVar:
        break;
    case Instr::cFunc: // Fun��es depois da �ltima vari�vel
    case Instr::cVarFunc:
        {
        // Procura a primeira fun��o
            const char * p = texto1;
            for (; p < p3; p += Num16(p))
                if (p[2] == Instr::cFunc || p[2] == Instr::cVarFunc)
                    break;
        // Colocar depois das vari�veis
            if (d1 < p)
                d1 = p;
            break;
        }
    default: // Vari�veis antes da primeira fun��o
        {
        // Procura a primeira fun��o
            const char * p = texto1;
            for (; p < p3; p += Num16(p))
                if (p[2] == Instr::cFunc || p[2] == Instr::cVarFunc)
                    break;
        // Colocar antes das fun��es
            if (d1 > p)
                d1 = p;
        }
    }
// Checa se a posi��o da fun��o n�o vai mudar
    if (d1 >= p1 && d1 <= p2)
        return Instr::CriarVarTxtFixo(v, "");
// Prepara as instru��es
    TMudarAux mudarcom; // Para mudar a lista de instru��es
    if (d1 < p2)
    {
        mudarcom.AddBloco(texto1, d1 - texto1); // texto1 a d1
        mudarcom.AddBloco(p1, p2 - p1);       // fun��o que ser� movida
        mudarcom.AddBloco(d1, p1 - d1);       // d1 a p1
        mudarcom.AddBloco(p2, p3 - p2);       // p2 a p3
    }
    else
    {
        mudarcom.AddBloco(texto1, p1 - texto1); // texto1 a p1
        mudarcom.AddBloco(p2, d1 - p2);       // p2 a d1
        mudarcom.AddBloco(p1, p2 - p1);       // fun��o que ser� movida
        mudarcom.AddBloco(d1, p3 - d1);       // d1 a p3
    }
// Verifica se bloco v�lido
    char err[200];
    if (!mudarcom.ChecaBloco(err, sizeof(err)))
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(err);
    }
// Anota altera��es em objeto TMudarClasse
    if (obj == nullptr)
        obj = new TMudarClasse(nomeclasse);
    mudarcom.AnotaBloco(obj);
    return Instr::CriarVarTxtFixo(v, "");
}

//------------------------------------------------------------------------------
bool TVarProg::FuncRenomear(TVariavel * v)
{
    if (Instr::VarAtual < v + 2)
        return false;
    char antes[CLASSE_NOME_TAM+100];
    char depois[CLASSE_NOME_TAM+100];
    copiastr(antes, v[1].getTxt(), sizeof(antes));
    copiastr(depois, v[2].getTxt(), sizeof(depois));
    if (TClasse::NomeValido(antes)==false ||
        TClasse::NomeValido(depois)==false)
        return false;
    if (strcmp(antes, depois) == 0)
        return false;
    new TRenomeiaClasse(antes, depois);
    return false;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncSalvar(TVariavel * v)
{
// Indica que deve salvar
    if (TMudarClasse::Salvar == 0)
        TMudarClasse::Salvar = 1;
// Acerta valor padr�o das vari�veis
    TArqMapa::ParamLinha = 4000;
    TArqMapa::ParamN = 0;
    TArqMapa::ParamIndent = 2;
    TArqMapa::ParamClasse = 1;
    TArqMapa::ParamFunc = 1;
    TArqMapa::ParamVar = 0;
// Obt�m vari�veis de acordo com texto
    if (Instr::VarAtual < v + 1)
        return false;
    char comando = 0;
    int  valor = 0;
    for (const char * p = v[1].getTxt();; p++)
    {
        if (*p >= '0' && *p <= '9')
        {
            valor = valor * 10 + *p - '0';
            if (valor > 10000)
                valor = 10000;
            continue;
        }
        switch (comando | 0x20)
        {
        case 'l':
            TArqMapa::ParamLinha = (valor < 70 ? 70 : valor > 4000 ? 4000 : valor);
            break;
        case 'n':
            TArqMapa::ParamN = (valor<3 ? valor : 3);
            break;
        case 'i':
            TArqMapa::ParamIndent = (valor < 8 ? valor : 8);
            break;
        case 'c':
            TArqMapa::ParamClasse = (valor < 10 ? valor : 10);
            break;
        case 'f':
            TArqMapa::ParamFunc = (valor < 10 ? valor : 10);
            break;
        case 'v':
            TArqMapa::ParamVar = (valor < 10 ? valor : 10);
            break;
        }
        valor = 0;
        comando = *p;
        if (comando == 0)
            break;
    }
    return false;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncSalvarTudo(TVariavel * v)
{
    bool ret = FuncSalvar(v);
    TMudarClasse::Salvar = 2;
    return ret;
}

//------------------------------------------------------------------------------
bool TVarProg::FuncClIni(TVariavel * v)
{
    if (Instr::VarAtual < v + 1)
        return false;
    char nomearq[100];
    copiastr(nomearq, v[1].getTxt(), sizeof(nomearq));
    TArqMapa * m = TArqMapa::Procura(nomearq);
    while (Instr::VarAtual > v + 1)
    {
        TClasse * cl = TClasse::Procura(v[2].getTxt());
        if (cl == nullptr)
            break;
        if (m == nullptr)
        {
            if (!TArqMapa::NomeValido(nomearq))
                break;
            m = new TArqMapa(nomearq);
        }
        cl->MoveArqIni(m);
        break;
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(m == nullptr ? "" :
                                m->ClInicio == nullptr ? "" :
                                m->ClInicio->Nome);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncClFim(TVariavel * v)
{
    if (Instr::VarAtual < v + 1)
        return false;
    char nomearq[100];
    copiastr(nomearq, v[1].getTxt(), sizeof(nomearq));
    TArqMapa * m = TArqMapa::Procura(nomearq);
    while (Instr::VarAtual > v + 1)
    {
        TClasse * cl = TClasse::Procura(v[2].getTxt());
        if (cl == nullptr)
            break;
        if (m == nullptr)
        {
            if (!TArqMapa::NomeValido(nomearq))
                break;
            m = new TArqMapa(nomearq);
        }
        cl->MoveArqFim(m);
        break;
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(m == nullptr ? "" :
                                m->ClFim == nullptr ? "" :
                                m->ClFim->Nome);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncClAntes(TVariavel * v)
{
    if (Instr::VarAtual < v + 1)
        return false;
    TClasse * cl = TClasse::Procura(v[1].getTxt());
    if (cl == nullptr)
        return Instr::CriarVarTxtFixo(v, "");
    if (Instr::VarAtual > v + 1)
    {
        TClasse * cl2 = TClasse::Procura(v[2].getTxt());
        if (cl2 && cl2 != cl)
            cl2->MoveArqAntes(cl);
    }
    Instr::ApagarVar(v);
    cl = cl->ArqAntes;
    return Instr::CriarVarTexto(cl == nullptr ? "" : cl->Nome);
}

//------------------------------------------------------------------------------
bool TVarProg::FuncClDepois(TVariavel * v)
{
    if (Instr::VarAtual < v + 1)
        return false;
    TClasse * cl = TClasse::Procura(v[1].getTxt());
    if (cl == nullptr)
        return Instr::CriarVarTxtFixo(v, "");
    if (Instr::VarAtual > v + 1)
    {
        TClasse * cl2 = TClasse::Procura(v[2].getTxt());
        if (cl2 && cl2 != cl)
            cl2->MoveArqDepois(cl);
    }
    Instr::ApagarVar(v);
    cl = cl->ArqDepois;
    return Instr::CriarVarTexto(cl == nullptr ? "" : cl->Nome);
}

//------------------------------------------------------------------------------
int TVarProg::FTamanho(const char * instr)
{
    return sizeof(TVarProg);
}

//------------------------------------------------------------------------------
int TVarProg::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TVarProg);
}

//------------------------------------------------------------------------------
void TVarProg::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TVarProg * ref = reinterpret_cast<TVarProg*>(v->endvar);
    for (; antes < depois; antes++)
        ref[antes].Criar();
    for (; depois < antes; depois++)
        ref[depois].Apagar();
}

//------------------------------------------------------------------------------
void TVarProg::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    VARIAVEL_MOVER_SIMPLES(TVarProg)
}

//------------------------------------------------------------------------------
bool TVarProg::FGetBool(TVariavel * v) VARIAVEL_FGETINT0(TVarProg)
int TVarProg::FGetInt(TVariavel * v) VARIAVEL_FGETINT0(TVarProg)
double TVarProg::FGetDouble(TVariavel * v) VARIAVEL_FGETINT0(TVarProg)
const char * TVarProg::FGetTxt(TVariavel * v) VARIAVEL_FGETTXT0(TVarProg)
