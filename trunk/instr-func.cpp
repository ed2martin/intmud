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
#include "procurar.h"
#include "random.h"
#include "misc.h"

//----------------------------------------------------------------------------
// Usado em bool Instr::FuncTxt2() para copiar caracteres de cores
#define FUNCTXT_CORES \
    case ex_barra_b:  \
        *destino++ = *txt++;  \
        break;                \
    case ex_barra_c:          \
        if ((txt[1]>='0' && txt[1]<='9') ||      \
                (txt[1]>='A' && txt[1]<='F') ||  \
                (txt[1]>='a' && txt[1]<='f'))    \
            *destino++ = *txt++; \
        *destino++ = *txt++;     \
        break;        \
    case ex_barra_d:  \
        if (txt[1]>='0' && txt[1]<='7') \
            *destino++ = *txt++; \
    case ex_barra_n:             \
        *destino++ = *txt++;     \
        break;

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
/// Funções que lidam com números (intpos, intabs, int e rand)
bool Instr::FuncNumero(TVariavel * v, int valor)
{
    double numero=0;
    for (TVariavel * var = v+1; var<=VarAtual; var++)
        numero += var->getDouble();
    ApagarVar(v);
    switch (valor)
    {
    case 0: // intpos()
        if (!CriarVar(InstrDouble))
            return false;
        VarAtual->setDouble(numero<0 ? 0 : numero);
        return true;
    case 1: // intabs()
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
/// Função txtnum
bool Instr::FuncTxtNum(TVariavel * v, int valor)
{
    char mens[80];      // Resultado
    int  flags = 0;     // bit 0=1 -> usar notação científica
                        // bit 1=1 -> separar com pontos
                        // bit 2=1 -> separar com vírgulas
    int  digitos = -1;  // Dígitos após a vírgula, 0-9 ou -1=automático

    if (VarAtual < v+2)
        return false;

// Obtém o tipo de conversão
    for (const char * p = v[2].getTxt(); *p; p++)
        switch (*p)
        {
        case 'E':
        case 'e': flags |= 1; break;
        case '.': flags |= 2; break;
        case ',': flags |= 4; break;
        default:
            if (*p>='0' && *p<='9')
                digitos = *p - '0';
        }

// Obtém o texto
    while (true)
    {
        int tipo = v[1].Tipo();
    // Inteiro
        if (tipo == varInt)
        {
            const char * zeros = "000000000";
            if (flags&1)
            {
                sprintf(mens, "%E", v[1].getDouble());
                flags=0;
            }
            else if (digitos<1 || digitos>9)
                sprintf(mens, "%d", v[1].getInt());
            else
                sprintf(mens, "%d.%s", v[1].getInt(), zeros+9-digitos);
            break;
        }
    // Não é numérico
        if (tipo != varDouble)
            return false;
    // Double
        double d = v[1].getDouble();
    // Double fora dos limites
        if ((flags&1) || d >= DOUBLE_MAX || d <= -DOUBLE_MAX)
        {
            sprintf(mens, "%E", d);
            flags=0;
            break;
        }
    // Quantidade fixa de dígitos
        if (digitos >= 0)
        {
            char mens2[10];
            sprintf(mens2, "%%.%df", digitos);
            sprintf(mens, mens2, d);
            break;
        }
    // Quantidade de dígitos não foi especificada
        sprintf(mens, "%.9f", d);
        char * p = mens;
        while (*p)
            p++;
        while (p>mens && p[-1]=='0')
            p--;
        if (p>mens && p[-1]=='.')
            p--;
        *p=0;
        break;
    }

// Acrescenta pontos ou vírgulas, se necessário
    if (flags&6)
    {
        char * p = mens;
        int total;
        while (*p && *p!='.')
            p++;
        if (*p && (flags&2))
            *p=',';
        total = (p - mens - (*mens=='-') - 1) / 3;
        char * dest = p;
        while (*dest)
            dest++;
        while (total > 0)
        {
            p -= 3;
            assert(p>mens);
            for (; dest >= p; dest--)
                dest[total] = *dest;
            dest[total--] = (flags&2 ? '.' : ',');
        }
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
    int tam = strlen(mens);
    if (tam>disp)
        tam=disp;
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
/// Funções de texto: txt1, txt2, txtcor, etc.
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
        {
            switch (*txt)
            {
            case ex_barra_b:
                txt++;
                break;
            case ex_barra_c:
                if ((txt[1]>='0' && txt[1]<='9') ||
                        (txt[1]>='A' && txt[1]<='F') ||
                        (txt[1]>='a' && txt[1]<='f'))
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
        }
        break;
    case 3: // txtmai
        while (*txt && destino<mens+sizeof(mens)-1)
            switch (*txt)
            {
            FUNCTXT_CORES
            default: *destino++ = tabMAI[*(unsigned char*)txt++];
            }
        break;
    case 4: // txtmai2
        while (*txt && destino<mens+sizeof(mens)-1)
        {
            while (*txt && *(unsigned char*)txt<'0' &&
                    destino<mens+sizeof(mens)-1)
                switch (*txt)
                {
                FUNCTXT_CORES
                default: *destino++ = *txt++;
                }
            if (*txt && destino<mens+sizeof(mens)-1)
                switch (*txt)
                {
                FUNCTXT_CORES
                default: *destino++ = tabMAI[*(unsigned char*)txt++];
                }
            while (*txt && *txt!='.' && destino<mens+sizeof(mens)-1)
                *destino++ = *txt++;
        }
        break;
    case 5: // txtmin
        while (*txt && destino<mens+sizeof(mens)-1)
            switch (*txt)
            {
            FUNCTXT_CORES
            default: *destino++ = tabMIN[*(unsigned char*)txt++];
            }
        break;
    case 6: // txtmaimin
        while (*txt && destino<mens+sizeof(mens)-1)
        {
            while (*txt && *(unsigned char*)txt<'0' &&
                    destino<mens+sizeof(mens)-1)
                switch (*txt)
                {
                FUNCTXT_CORES
                default: *destino++ = *txt++;
                }
            if (*txt && destino<mens+sizeof(mens)-1)
                switch (*txt)
                {
                FUNCTXT_CORES
                default: *destino++ = tabMAI[*(unsigned char*)txt++];
                }
            while (*txt && *txt!='.' && destino<mens+sizeof(mens)-1)
                switch (*txt)
                {
                FUNCTXT_CORES
                default: *destino++ = tabMIN[*(unsigned char*)txt++];
                }
        }
        break;
    case 7: // txtfiltro
        destino = txtFiltro(destino, txt, sizeof(mens));
        break;
    case 8: // txtshs
        {
            unsigned long codif[5];
            gerasenha(txt, codif);
            for (int x=0; x<5; x++)
                for (int y=0; y<5; y++)
                {
                    *destino++ = (codif[x] & 0x3F) + 0x21;
                    codif[x] >>= 6;
                }
            *destino++ = (codif[0]&3) + (codif[1]&3)*4 +
                         (codif[2]&3)*16 + 0x21;
            *destino++ = (codif[3]&3) + (codif[4]&3)*4 + 0x21;
            break;
        }
    case 9: // txtnome
        destino = txtNome(destino, txt, sizeof(mens));
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
/// Funções intnome e intsenha
bool Instr::FuncInt(TVariavel * v, int valor)
{
    const char * txt = "";  // Texto
    if (VarAtual >= v+1)
        txt = v[1].getTxt();
// Obtém o valor
    switch (valor)
    {
    case 0: // intnome
        valor = verifNome(txt);
        break;
    case 1: // intsenha
        valor = verifSenha(txt);
        break;
    default:
        valor=0;
    }
// Retorna o valor
    ApagarVar(v);
    if (!CriarVar(InstrVarInt))
        return false;
    VarAtual->setInt(valor);
    return true;
}

//----------------------------------------------------------------------------
/// Função txtremove
bool Instr::FuncTxtRemove(TVariavel * v, int valor)
{
    const char * txt;   // Texto
    char mens[4096];    // Resultado
    int  remove;        // O que deve remover
    char * destino = mens;

// Obtém variáveis
    if (VarAtual < v+2)
        return false;
    remove = v[2].getInt();
    if (remove<0 || remove>15)
        return false;
    txt = v[1].getTxt();

// Copia texto conforme variável remove
    int ini = 1; // quando encontra algo diferente de espaço vai para 2
    int esp = 0; // quantos espaços acumulados

    while (*txt)
        switch (*txt)
        {
        case ex_barra_b:
            if ((remove&8)==0)
                goto copia;
            txt++;
            break;
        case ex_barra_c:
            if ((remove&8)==0)
                goto copia;
            if ((txt[1]>='0' && txt[1]<='9') ||
                    (txt[1]>='A' && txt[1]<='F') ||
                    (txt[1]>='a' && txt[1]<='f'))
                txt += 2;
            else
                txt++;
            break;
        case ex_barra_d:
            if ((remove&8)==0)
                goto copia;
            if (txt[1]>='0' && txt[1]<='7')
                txt += 2;
            else
                txt++;
            break;
        case ' ':
            esp++, txt++;
            break;
        default:
        copia:
            if (esp)
            {
                if (remove & ini)
                    esp = ini-1;
                while (esp && destino<mens+sizeof(mens))
                    *destino++=' ', esp--;
            }
            if (destino<mens+sizeof(mens))
                *destino++ = *txt;
            txt++;
            ini=2;
            break;
        }
    if ((remove&4)==0)
        while (esp && destino<mens+sizeof(mens))
            *destino++=' ', esp--;

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
bool Instr::FuncTxtProc(TVariavel * v, int valor)
{
    int posic = -1;     // Retorno - posição onde encontrou
    int ini = 0;        // Início
    while (VarAtual >= v+2)
    {
    // Obtém ini
        if (VarAtual >= v+3)
            ini = v[3].getInt();
        if (ini<0)
            ini=0;
    // Obtém o padrão
        TProcurar proc;
        if (!proc.Padrao(v[2].getTxt(), valor))
            break;
    // Obtém o texto a ser procurado
        const char * p = v[1].getTxt();
        int tam = strlen(p);
        if (ini>tam)
            break;
    // Procura
        posic = proc.Proc(p+ini, tam-ini);
        if (posic>=0)
            posic += ini;
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
/// Troca texto (txttroca)
bool Instr::FuncTxtTroca(TVariavel * v, int valor)
{
    if (VarAtual < v+3)
        return false;
// Obtém o padrão
    TProcurar proc;
    if (DadosTopo+10 >= DadosFim ||
        !proc.Padrao(v[2].getTxt(), valor))
    {
        ApagarVar(v+2);
        ApagarRet(v);
        return true;
    }
// Obtém o novo texto
    copiastr(proc.troca, v[3].getTxt(), sizeof(proc.troca));
// Prepara variáveis
    ApagarVar(v+2);
    if (!CriarVar(InstrTxtFixo))
        return false;
    VarAtual->endvar = DadosTopo;
    proc.dest = DadosTopo;
    proc.tamdest = DadosFim - DadosTopo;
// Troca
    const char * p = v[1].getTxt();
    proc.Proc(p, strlen(p));
    VarAtual->tamanho = strlen(DadosTopo) + 1;
    DadosTopo += VarAtual->tamanho;
    ApagarRet(v);
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
/// Função inttotal
bool Instr::FuncTotal(TVariavel * v, int valor)
{
    int tamanho = 0;
    TObjeto * obj = 0;
    for (TVariavel * var = v+1; var<=VarAtual && obj==0; var++)
        switch (var->Tipo())
        {
        case varOutros:
            if (var->defvar[endVetor])
                tamanho += (unsigned char)var->defvar[endVetor];
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

//----------------------------------------------------------------------------
/// Função vartroca
bool Instr::FuncVarTroca(TVariavel * v, int valor)
{
    if (VarAtual < v+3)
        return false;
    if (FuncAtual >= FuncFim - 2 || FuncAtual->este==0)
        return false;

// Variáveis
    TClasse * c = FuncAtual->este->Classe;
    const char * origem; // Primeiro argumento - texto original
    char mens[4096]; // Aonde jogar o texto codificado

// Obtém argumento - padrão que deve procurar no texto
    char txtpadrao[40]; // Texto
    int  tampadrao=0;   // Tamanho do texto sem o zero
    origem = v[2].getTxt();
    while (*origem && tampadrao<(int)sizeof(txtpadrao))
        txtpadrao[tampadrao++] = tabCOMPLETO[*(unsigned char*)origem++];
#if 0
    printf("Padrão = [");
    for (int x=0; x<tampadrao; x++)
        putchar(txtpadrao[x]);
    puts("]"); fflush(stdout);
#endif

// Obtém índices das variáveis que serão procuradas
    origem = v[3].getTxt();
    int tamvar = strlen(origem); // Tamanho do texto sem o zero
    int indini = 0;              // Índice inicial
    int indfim = c->NumVar-1;    // Índice final
    if (tamvar)
    {
        indini = c->IndiceNomeIni(origem);
        indfim = c->IndiceNomeFim(origem);
    }

// Verifica se índices válidos
    if (indini<0)   // Índice inválido
        indini=1, indfim=0;
    else if (tampadrao==0) // Texto a procurar é nulo
        if (c->InstrVar[indini][tamvar+Instr::endNome]==0)
                        // Se primeira variável é nula
            indini++; // Passa para próxima variável
#if 0
    printf("Variáveis(%d): ", tamvar);
    for (int x=indini; x<=indfim; x++)
        printf("[%s]", c->InstrVar[x]+5);
    putchar('\n'); fflush(stdout);
#endif

// Cabeçalho da instrução
    mens[2] = cConstExpr; // Tipo de instrução
    mens[Instr::endProp] = 0;
    mens[Instr::endIndice] = Instr::endNome+2; // Aonde começam os dados da constante
    mens[Instr::endVetor] = 0; // Não é vetor
    mens[Instr::endNome] = '+'; // Nome da variável
    mens[Instr::endNome+1] = 0;
    mens[Instr::endNome+2] = ex_txt;
    char * destino = mens + Instr::endNome + 3;
    char * dest_ini = 0;// Endereço do início da variável ex_txt em destino
                        // 0=não anotou nenhuma variável

// Monta instrução
    tamvar += Instr::endNome;  // Para comparação
    origem = v[1].getTxt();
    while (*origem)
    {
        int x;
    // Verifica padrão
        for (x=0; x<tampadrao; x++)
            if (tabCOMPLETO[(unsigned char)origem[x]] != txtpadrao[x])
                break;
    // Não achou - anota caracter
        if (x<tampadrao)
        {
            if (destino >= mens+sizeof(mens)-4)
                break;
            *destino++ = *origem++;
            continue;
        }
    // Achou - procura variável
        int ini = indini; // Índice inicial
        int fim = indfim; // Índice final
        x = -1;
        while (ini<=fim)
        {
            int meio = (ini+fim)/2;
            int comp = comparaZ(origem+tampadrao,
                    c->InstrVar[meio] + tamvar);
#if 0
            printf("cmp [%s] [%s] = %d\n", origem+tampadrao,
                    c->InstrVar[meio] + tamvar, comp); fflush(stdout);
#endif
            switch (comp)
            {
            case 2:
            case 0:
                x = meio;
            case 1:
                ini = meio+1;
                break;
            case -1:
            case -2:
                fim = meio-1;
                break;
            }
        }
    // Não achou variável - anota caracter
        if (x<0)
        {
            if (destino >= mens+sizeof(mens)-4)
                break;
            *destino++ = *origem++;
            continue;
        }
    // Achou variável
    // Acerta origem
        const char * defvar = c->InstrVar[x];
        int tamtxt = strlen(defvar + tamvar);
        origem += tampadrao + tamtxt;
        //printf("Variável [%s]\n", defvar+5); fflush(stdout);
    // Se for variável, copia texto
        if (defvar[2]!=cFunc && defvar[2]!=cVarFunc &&
                defvar[2]!=cConstExpr)
        {
            TVariavel v;
            x = c->IndiceVar[x];
            v.defvar = defvar;
            v.tamanho = 0;
            v.bit = x >> 24;
            if (defvar[2]==cConstTxt || // Constante
                    defvar[2]==cConstNum)
                v.endvar = 0;
            else if (x & 0x400000) // Variável da classe
                v.endvar = c->Vars + (x & 0x3FFFFF);
            else    // Variável do objeto
                v.endvar = FuncAtual->este->Vars + (x & 0x3FFFFF);
            const char * o = v.getTxt();
            while (*o && destino<mens+sizeof(mens)-4)
                *destino++ = *o++;
            continue;
        }
    // Verifica se espaço suficiente
        if (destino + strlen(defvar + Instr::endNome) + tamtxt + 15
                >= mens+sizeof(mens))
            break;
    // Anota fim do texto
        if (dest_ini == 0) // Início
            *destino++ = 0;
        else if (destino == dest_ini) // Texto vazio
            destino--;
        else   // Não é texto vazio
        {
            *destino++ = 0;
            *destino++ = exo_add;
        }
    // Anota variável
        destino[0] = ex_varini;
        destino = copiastr(destino+1, defvar + Instr::endNome);
        destino[0] = ex_arg;
        destino[1] = ex_txt;
        destino += 2;
        if (tamtxt)
        {
            memcpy(destino, origem - tamtxt, tamtxt);
            destino += tamtxt;
        }
        destino[0] = 0;
        destino[1] = ex_ponto;
        destino[2] = ex_varfim;
        destino[3] = exo_add;
        destino[4] = ex_txt;
        destino += 5;
        dest_ini = destino;
    }

    destino[0] = 0;
    destino[1] = exo_add;
    destino[2] = ex_fim;
    destino += 3;

// Texto puro, sem substituições
    if (dest_ini==0)
    {
        int tam = destino - (mens + Instr::endNome + 3) - 3;
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
            memcpy(Instr::DadosTopo, mens+Instr::endNome+3, tam);
        Instr::DadosTopo[tam] = 0;
    // Acerta variáveis
        VarAtual->endvar = Instr::DadosTopo;
        VarAtual->tamanho = tam+1;
        Instr::DadosTopo += tam+1;
#if 0
        printf("vartxt txt: %s\n", (char*)VarAtual->endvar);
        fflush(stdout);
#endif
        return true;
    }

// Acerta tamanho da instrução
    int total = destino - mens;
    mens[0] = total;    // Tamanho da instrução
    mens[1] = total>>8;

// Acerta variáveis
    ApagarVar(v);
    if (DadosFim - DadosTopo < total)
        return false;
    VarAtual++;
    VarAtual->Limpar();
    VarAtual->defvar = DadosTopo;
    VarAtual->endvar = DadosTopo;
    VarAtual->tamanho = total;
    memcpy(DadosTopo, mens, total);
    DadosTopo += total;

// Mostra o que codificou
#if 0
    Instr::Decod(mens, VarAtual->defvar, sizeof(mens));
    printf("vartxt exec: %s\n", mens); fflush(stdout);
#endif

// Acerta função
    FuncAtual++;
    FuncAtual->linha = VarAtual->defvar;
    FuncAtual->este = FuncAtual[-1].este;
    FuncAtual->expr = VarAtual->defvar + Instr::endNome + 2;
    FuncAtual->inivar = VarAtual + 1;
    FuncAtual->fimvar = VarAtual + 1;
    FuncAtual->numarg = 0;
    FuncAtual->tipo = 0;
    return true;
}
