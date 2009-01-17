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
#include "instr.h"
#include "classe.h"
#include "objeto.h"
#include "variavel.h"
#include "random.h"
#include "misc.h"

//----------------------------------------------------------------------------
/// Argumento da função (arg0 a arg9)
bool Instr::FuncArg(TVariavel * v, int valor)
{
    if (valor >= FuncAtual->numarg)
        return false;
    ApagarVar(v);
    VarAtual++;
    *VarAtual = *(FuncAtual->inivar + valor);
    VarAtual->tamanho = 0;
    return true;
}

//----------------------------------------------------------------------------
/// Número de argumentos (args)
bool Instr::FuncArgs(TVariavel * v, int valor)
{
    ApagarVar(v);
    if (!CriarVar(InstrVarInt))
        return false;
    VarAtual->setInt(FuncAtual->numarg);
    return true;
}

//----------------------------------------------------------------------------
/// Criar objeto (criar)
bool Instr::FuncCriar(TVariavel * v, int valor)
{
    if (VarAtual <= v)
        return false;
// Procura a classe
    TClasse * c = TClasse::Procura(v[1].getTxt());
    if (c==0)
        return false;
// Cria objeto
    TObjeto * obj = TObjeto::Criar(c);
    while (true)
    {
    // Procura a função ini
        int indice = c->IndiceNome("ini");
        if (indice<0) // Variável/função inexistente
            break;
        char * defvar = c->InstrVar[indice];
    // Verifica se é função
        if (defvar[2] != cFunc)
            break;
    // Verifica se pode criar função
        if (FuncAtual >= FuncFim - 1)
            break;
    // Cria função
        FuncAtual++;
        FuncAtual->linha = defvar + Num16(defvar);
        FuncAtual->este = obj;
        FuncAtual->expr = 0;
        FuncAtual->inivar = v+2;
        FuncAtual->fimvar = VarAtual + 1;
        FuncAtual->numarg = FuncAtual->fimvar - FuncAtual->inivar;
        FuncAtual->tipo = 3;
        return true;
    }
// Retorna o endereço do objeto criado
    ApagarVar(v);
    VarAtual++;
    VarAtual->Limpar();
    VarAtual->defvar = InstrVarObjeto;
    VarAtual->endvar = obj;
    return true;
}

//----------------------------------------------------------------------------
/// Apagar objeto (apagar)
bool Instr::FuncApagar(TVariavel * v, int valor)
{
    for (TVariavel * var = v+1; var<=VarAtual; var++)
    {
        TObjeto * obj = var->getObj();
        if (obj)
            obj->MarcarApagar();
    }
    return false;
}

//----------------------------------------------------------------------------
/// Objeto "este"
bool Instr::FuncEste(TVariavel * v, int valor)
{
    ApagarVar(v);
    if (!CriarVar(InstrVarObjeto))
        return false;
    VarAtual->setObj(FuncAtual->este);
    return true;
}

//----------------------------------------------------------------------------
/// Funções que lidam com números (pos, abs, int e rand)
bool Instr::FuncNumero(TVariavel * v, int valor)
{
    double numero=0;
    for (TVariavel * var = v+1; var<=VarAtual; var++)
        numero += var->getDouble();
    ApagarVar(v);
    switch (valor)
    {
    case 0: // pos()
        if (!CriarVar(InstrDouble))
            return false;
        VarAtual->setDouble(numero<0 ? 0 : numero);
        return true;
    case 1: // abs()
        if (!CriarVar(InstrDouble))
            return false;
        VarAtual->setDouble(numero<0 ? -numero : numero);
        return true;
    case 2: // int()
        if (!CriarVar(InstrVarInt))
            return false;
        VarAtual->setInt((int)numero);
        return true;
    case 3: // rand()
        if (!CriarVar(InstrVarInt))
            return false;
        if (numero<1)
            return true;
        VarAtual->setInt(circle_random() % (int)numero);
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
/// Referência (ref)
bool Instr::FuncRef(TVariavel * v, int valor)
{
    TObjeto * obj = 0;
    for (TVariavel * var = v+1; var<=VarAtual && obj==0; var++)
        obj = var->getObj();
    if (obj==0)
        return false;
    ApagarVar(v);
    if (!CriarVar(InstrVarObjeto))
        return false;
    VarAtual->setObj(obj);
    return true;
}

//----------------------------------------------------------------------------
/// Texto (txt)
bool Instr::FuncTxt(TVariavel * v, int valor)
{
    int ini = 0;        // Início
    int tam = 0x10000;  // Tamanho
    const char * txt = "";  // Texto
    char mens[4096];    // Resultado
    char * destino = mens;
// Obtém ini, tam e txt conforme os argumentos
    if (VarAtual >= v+3)
        tam = v[3].getInt();
    if (tam>0)
    {
        if (VarAtual >= v+2)
        {
            ini = v[2].getInt();
            if (ini<0)
                ini=0;
        }
        if (VarAtual >= v+1)
            txt = v[1].getTxt();
    }
    if (tam>=(int)sizeof(mens))
        tam = sizeof(mens);
// Obtém o texto
        // Avança para início do texto
    for (const char * fim = txt+ini; *txt && txt<fim; )
        txt++;
        // Copia texto
    for (const char * fim = txt+tam; *txt && txt<fim; )
        *destino++ = *txt++;
        // Obtém: tam = tamanho do texto sem o zero no final
    tam = destino - mens;
// Acerta variáveis
    ApagarVar(v);
    if (!CriarVar(InstrTxtFixo))
        return false;
// Verifica espaço disponível (sem o 0 no final do texto)
    int disp = Instr::DadosFim - Instr::DadosTopo - 1;
    if (disp<0)
        return false;
    if (tam>disp)
        tam = disp;
// Copia texto
    if (tam>0)
        memcpy(Instr::DadosTopo, mens, tam);
    Instr::DadosTopo[tam] = 0;
// Acerta variáveis
    VarAtual->endvar = Instr::DadosTopo;
    VarAtual->tamanho = tam+1;
    Instr::DadosTopo += tam+1;
    return true;
}

//----------------------------------------------------------------------------
/// Funções de texto: txt1, txt2, txtcor
bool Instr::FuncTxt2(TVariavel * v, int valor)
{
    const char * txt = "";  // Texto
    char mens[4096];    // Resultado
    char * destino = mens;

    if (VarAtual >= v+1)
        txt = v[1].getTxt();
// Obtém o texto
    switch (valor)
    {
    case 0: // txt1
        while (*txt==' ')
            txt++;
        while (*txt && *txt!=' ' && destino<mens+sizeof(mens))
            *destino++ = *txt++;
        break;
    case 1: // txt2
        while (*txt==' ') txt++;
        while (*txt && *txt!=' ') txt++;
        while (*txt==' ') txt++;
        while (*txt && destino<mens+sizeof(mens))
            *destino++ = *txt++;
        break;
    case 2: // txtcor
        while (*txt && destino<mens+sizeof(mens))
            switch (*txt)
            {
            case ex_barra_b:
                txt++;
                break;
            case ex_barra_c:
                if (txt[1]>='0' && txt[1]<='9' ||
                        txt[1]>='A' && txt[1]<='F' ||
                        txt[1]>='a' && txt[1]<='f')
                    txt += 2;
                else
                    txt++;
                break;
            case ex_barra_d:
                if (txt[1]>='0' && txt[1]<='7')
                    txt += 2;
                else
                    txt++;
                break;
            default:
                *destino++ = *txt++;
            }
        break;
    default:
        return false;
    }
// Acerta variáveis
    ApagarVar(v);
    if (!CriarVar(InstrTxtFixo))
        return false;
// Verifica espaço disponível (sem o 0 no final do texto)
    int disp = Instr::DadosFim - Instr::DadosTopo - 1;
    if (disp<0)
        return false;
// Copia texto
    int tam = (destino-mens < disp ? destino-mens : disp);
    if (tam>0)
        memcpy(Instr::DadosTopo, mens, tam);
    Instr::DadosTopo[tam] = 0;
// Acerta variáveis
    VarAtual->endvar = Instr::DadosTopo;
    VarAtual->tamanho = tam+1;
    Instr::DadosTopo += tam+1;
    return true;
}

//----------------------------------------------------------------------------
/// Procura texto (txtproc)
// Usa algoritmo Boyer-Moore, mas com uma só tabela
bool Instr::FuncTxtProc(TVariavel * v, int valor)
{
    int posic = -1;     // Retorno - posição onde encontrou
    int ini = 0;        // Início

    while (VarAtual >= v+2)
    {
        if (VarAtual >= v+3)
            ini = v[3].getInt();
        if (ini<0)
            ini=0;
    // Padrão
        const char * p = v[2].getTxt();
        if (*p==0)
            break;
    // Procurar um caracter
        if (p[1]==0)
        {
            char ch = *p;
            const char * ini = v[1].getTxt();
            if (valor)
            {
                for (p=ini; *p; p++)
                    if (*p == ch)
                    {
                        posic = p - ini;
                        break;
                    }
            }
            else
            {
                ch = tabCOMPLETO[(unsigned char)ch];
                for (p=ini; *p; p++)
                    if (tabCOMPLETO[*(unsigned char*)p] == ch)
                    {
                        posic = p - ini;
                        break;
                    }
            }
            break;
        }
    // Variáveis
        char padrao[4096];  // Texto procurado
        int  tabela[0x100]; // Tabela de deslocamento
    // Obtém tamanho do padrão
        int tampadrao = strlen(p);
        if (tampadrao > (int)sizeof(padrao))
            tampadrao = (int)sizeof(padrao);
    // Inicializa tabela
        for (int x=0; x<0x100; x++)
            tabela[x] = -1;
        if (valor)
        {
    // Exato - Copia padrão e acerta tabela
            for (int x=0; x<tampadrao; x++,p++)
            {
                padrao[x] = *p;
                tabela[*(unsigned char*)p] = x;
            }
    // Exato - Obtém o texto
            p = v[1].getTxt();      // p = texto
            int tamp = strlen(p);   // tamp = tamanho do texto
            if (tampadrao > tamp)
                break;
    // Exato - procura
            int i=tampadrao-1, j=0;
            while (true)
            {
                char ch = p[i+j];
                if (padrao[i]==ch)
                {
                    if (i--)    // Passa para próximo caracter
                        continue;
                    posic = j;  // Encontrou
                    break;
                }
                i -= tabela[(unsigned char)ch]; // Obtém deslocamento
                j += (i>1 ? i : 1); // Desloca j
                i = tampadrao-1;    // Inicializa i
                if (i+j >= tamp)    // Verifica fim da string
                    break;
            }
        }
        else
        {
    // Não exato - Copia padrão e acerta tabela
            for (int x=0; x<tampadrao; x++,p++)
            {
                unsigned char ch = tabCOMPLETO[*(unsigned char*)p];
                padrao[x] = ch;
                tabela[ch] = x;
            }
    // Não exato - Obtém o texto
            p = v[1].getTxt();      // p = texto
            int tamp = strlen(p);   // tamp = tamanho do texto
            if (tampadrao > tamp)
                break;
    // Não exato - procura
            int i=tampadrao-1, j=0;
            while (true)
            {
                char ch = tabCOMPLETO[(unsigned char)p[i+j]];
                if (padrao[i]==ch)
                {
                    if (i--)    // Passa para próximo caracter
                        continue;
                    posic = j;  // Encontrou
                    break;
                }
                i -= tabela[(unsigned char)ch]; // Obtém deslocamento
                j += (i>1 ? i : 1); // Desloca j
                i = tampadrao-1;    // Inicializa i
                if (i+j >= tamp)    // Verifica fim da string
                    break;
            }
        }
        break;
    }
// Retorna o resultado
    ApagarVar(v);
    if (!CriarVar(InstrVarInt))
        return false;
    VarAtual->setInt(posic);
    return true;
}

//----------------------------------------------------------------------------
/// Funções objantes e objdepois
bool Instr::FuncAntesDepois(TVariavel * v, int valor)
{
    if (VarAtual < v+1)
        return false;
// Obtém objeto
    TObjeto * obj = v[1].getObj();
    if (obj==0)
        return false;
// Obtém objeto anterior ou próximo
    if (valor==0)
        obj = obj->Antes;
    else
        obj = obj->Depois;
// Anota objeto
    if (obj==0)
        return false;
    ApagarVar(v);
    if (!CriarVar(InstrVarObjeto))
        return false;
    VarAtual->setObj(obj);
    return true;
}

//----------------------------------------------------------------------------
/// Função total
bool Instr::FuncTotal(TVariavel * v, int valor)
{
    int tamanho = 0;
    TObjeto * obj = 0;
    for (TVariavel * var = v+1; var<=VarAtual && obj==0; var++)
        switch (var->Tipo())
        {
        case varNulo:
        case varInt:
        case varDouble:
            break;
        case varTxt:
            tamanho += strlen(var->getTxt());
            break;
        case varObj:
            obj = var->getObj();
            if (obj)
                tamanho += obj->Classe->NumObj;
            break;
        }
    ApagarVar(v);
    if (!CriarVar(InstrVarInt))
        return false;
    VarAtual->setInt(tamanho);
    return true;
}
