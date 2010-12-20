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
#include "procurar.h"
#include "misc.h"

//#define DEBUG  // Mostrar e testar vetores de TVarIntTempo

// Valor máximo de IntTempo é INTTEMPO_MAX*INTTEMPO_MAX-1
#define INTTEMPO_MAX 0x400

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
    return (v >= INTTEMPO_MAX*INTTEMPO_MAX ? INTTEMPO_MAX*INTTEMPO_MAX-1 : v);
}

//------------------------------------------------------------------------------
int TVarIncDec::getDec()
{
    unsigned int v = valor - TempoIni;
    return (v >= INTTEMPO_MAX*INTTEMPO_MAX ? 0 : v);
}

//------------------------------------------------------------------------------
void TVarIncDec::setInc(int v)
{
    if (v < 0)  v=0;
    if (v >= INTTEMPO_MAX*INTTEMPO_MAX)  v = INTTEMPO_MAX*INTTEMPO_MAX-1;
    valor = TempoIni - v;
}

//------------------------------------------------------------------------------
void TVarIncDec::setDec(int v)
{
    if (v < 0)  v=0;
    if (v >= INTTEMPO_MAX*INTTEMPO_MAX)  v = INTTEMPO_MAX*INTTEMPO_MAX-1;
    valor = TempoIni + v;
}

//------------------------------------------------------------------------------
bool TVarIncDec::FuncVetorInc(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar")==0)
        for (unsigned int x=0; x<(unsigned char)v->defvar[Instr::endVetor]; x++)
            this[x].valor = TempoIni;
    return false;
}

//------------------------------------------------------------------------------
bool TVarIncDec::FuncVetorDec(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar")==0)
        for (unsigned int x=0; x<(unsigned char)v->defvar[Instr::endVetor]; x++)
            this[x].valor = TempoIni;
    return false;
}

//------------------------------------------------------------------------------
void TVarIntTempo::PreparaIni()
{
    if (TVarIntTempo::VetMenos)
        return;
    VetMenos = new TVarIntTempo*[INTTEMPO_MAX];
    VetMais = new TVarIntTempo*[INTTEMPO_MAX];
    memset(VetMenos, 0, sizeof(TVarIntTempo*)*INTTEMPO_MAX);
    memset(VetMais, 0, sizeof(TVarIntTempo*)*INTTEMPO_MAX);
}

//------------------------------------------------------------------------------
int TVarIntTempo::TempoEspera()
{
#ifdef DEBUG
    DebugVet(true);
#endif
    int menos = TempoMenos;
    int total = 0;
    for (; menos<INTTEMPO_MAX && total<10; menos++,total++)
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
        if (TempoMenos < INTTEMPO_MAX-1)
            TempoMenos++;
        else
        {
            TempoMenos = 0;
            TempoMais = (TempoMais + 1) % INTTEMPO_MAX;
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
    int valor = ((IndiceMais - TempoMais) * INTTEMPO_MAX +
            IndiceMenos - TempoMenos);
    if (valor<0)
        valor += INTTEMPO_MAX * INTTEMPO_MAX;
    return valor;
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
    for (int x=0; x<INTTEMPO_MAX; x++)
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
    for (int x=0; x<INTTEMPO_MAX; x++)
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

//------------------------------------------------------------------------------
bool FuncVetorTxt(TVariavel * v, const char * nome)
{
// Obtém número de variáveis e tamanho de uma variável
    int numvar = (unsigned char)v->defvar[Instr::endVetor];
    int tamvar = (unsigned char)v->defvar[Instr::endIndice] + 2;
    if (v->defvar[2] == Instr::cTxt2)
        tamvar += 256;
// Texto do vetor
    if (comparaZ(nome, "texto")==0)
    {
    // Obtém índice inicial e a quantidade de variáveis
        int ini = 0;
        int total = numvar;
        if (Instr::VarAtual >= v+1)
        {
            ini = v[1].getInt();
            if (ini<0)
                ini=0;
            if (Instr::VarAtual >= v+2)
            {
                total = v[2].getInt() + 1;
                if (total>numvar)
                    total=numvar;
            }
        }
        total -= ini;
    // Cria variável
        const char * origem = v->end_char + ini * tamvar;
        Instr::ApagarVar(v);  // Não tem importância que v seja apagado aqui
        if (!Instr::CriarVar(Instr::InstrTxtFixo))
            return false;
        if (Instr::DadosTopo >= Instr::DadosFim)
            return false;
    // Anota o texto
        char * destino = Instr::DadosTopo;
        for (; total>0; total--,origem+=tamvar)
        {
            //printf("texto = ["); fflush(stdout);
            //for (const char * x=origem; *x; x++)
            //    if (*(signed char*)x >= ' ')
            //        putchar(*x);
            //    else
            //        printf("(%02X)", *(unsigned char*)x);
            //printf("]\n"); fflush(stdout);
            for (const char * o = origem; *o && destino < Instr::DadosFim-1; )
                *destino++ = *o++;
        }
        *destino++ = 0;
    // Acerta a variável
        Instr::VarAtual->endvar = Instr::DadosTopo;
        Instr::VarAtual->tamanho = destino - Instr::DadosTopo;
        Instr::DadosTopo = destino;
        return true;
    }
// Divide em palavras
    if (comparaZ(nome, "palavras")==0)
    {
    // Obtém número de palavras (parâmetro de entrada)
        int total = numvar;
        if (Instr::VarAtual < v+1)
            return false;
        if (Instr::VarAtual >= v+2)
        {
            total = v[2].getInt();
            if (total>numvar)
                total=numvar;
        }
    // Copia o texto obtendo o número de palavras
        int palavras = 0;
        const char * origem = v[1].getTxt();
        char * destino = v->end_char;
        for (; total>1; total--)
        {
            while (*origem==' ' || *origem==Instr::ex_barra_n)
                origem++;
            if (*origem==0)
                break;
            char * d = destino;
            destino += tamvar, palavras++;
            for (; *origem && *origem!=' ' &&
                    *origem!=Instr::ex_barra_n; origem++)
                if (d<destino-1)
                    *d++ = *origem;
            *d = 0;
        }
    // Copia o último texto
        while (*origem==' ' || *origem==Instr::ex_barra_n)
            origem++;
        if (*origem)
        {
            char * d = destino;
            destino += tamvar, palavras++;
            while (*origem && d<destino-1)
                *d++ = *origem++;
            while (*origem==' ' || *origem==Instr::ex_barra_n)
                origem++;
            if (*origem==0)
            {
                for (d--; d>=destino-tamvar; d--)
                    if (*d!=' ' && *d!=Instr::ex_barra_n)
                        break;
                d++;
            }
            *d = 0;
        }
    // Limpa próximas variáveis do vetor
        for (; palavras < numvar; destino += tamvar, numvar--)
            *destino=0;
    // Retorna o número de palavras
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(palavras);
    }
// Divide em linhas
    if (comparaZ(nome, "linhas")==0)
    {
    // Obtém número mínimo de colunas (parâmetro de entrada)
        int colunas = tamvar - 1;
        if (Instr::VarAtual < v+1)
            return false;
        if (Instr::VarAtual >= v+2)
        {
            colunas = v[2].getInt();
            if (colunas>tamvar-1)
                colunas=tamvar-1;
            if (colunas<0)
                colunas=0;
        }
    // Copia o texto
        const char * origem = v[1].getTxt();
        char * destino = v->end_char;
        int linha = 0;  // Número de linhas copiadas
        colunas -= tamvar-1; // Quantas colunas recuar do final do texto
        while (linha<numvar)
        {
            char * d = destino;
            linha++, destino+=tamvar;
        // Copia linha
            while (*origem && *origem!=Instr::ex_barra_n && d<destino)
                *d++ = *origem++;
        // Linha cabe na variável
            if (d<destino)
            {
                *d=0;
            // Fim da linha
                if (*origem==Instr::ex_barra_n)
                {
                    origem++;
                    continue;
                }
            // Fim do texto
                break;
            }
        // Obtém aonde deve dividir a linha
            d--,origem--; // Recua um caracter
            int x;
            for (x=-1; x>=colunas; x--)
                if (origem[x]==' ')
                    break;
            if (x<colunas)
                for (x=-1; x>=colunas; x--)
                    if (origem[x]=='.')
                        break;
        // Divide a linha
            if (x>=colunas)
            {
                x++;
                origem += x;
                d += x;
            }
            *d=0;
        }
    // Limpa próximas variáveis do vetor
        for (; linha < numvar; destino += tamvar, numvar--)
            *destino=0;
    // Retorna o número de palavras
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(linha);
    }
// Limpa as variáveis do vetor
    if (comparaZ(nome, "limpar")==0)
    {
        char * destino = v->end_char;
        for (; numvar>0; numvar--, destino+=tamvar)
            *destino=0;
        return false;
    }
// Junta texto
    if (comparaZ(nome, "juntar")==0)
    {
    // Obtém o texto delimitador
        int total = numvar;
        char mens[4096];
        *mens=0;
        if (Instr::VarAtual >= v+1)
        {
            copiastr(mens, v[1].getTxt(), sizeof(mens));
            if (Instr::VarAtual >= v+2)
            {
                total = v[2].getInt();
                if (total>numvar)
                    total=numvar;
            }
        }
    // Cria variável
        const char * origem = v->end_char;
        Instr::ApagarVar(v);  // Não tem importância que v seja apagado aqui
        if (!Instr::CriarVar(Instr::InstrTxtFixo))
            return false;
        if (Instr::DadosTopo >= Instr::DadosFim)
            return false;
    // Anota o texto
        char * destino = Instr::DadosTopo;
        for (; total>0; total--,origem+=tamvar)
        {
            for (const char * o = origem; *o && destino < Instr::DadosFim-1; )
                *destino++ = *o++;
            if (total<=1)
                break;
            for (const char * o = mens; *o && destino < Instr::DadosFim-1; )
                *destino++ = *o++;
        }
        *destino++ = 0;
    // Acerta a variável
        Instr::VarAtual->endvar = Instr::DadosTopo;
        Instr::VarAtual->tamanho = destino - Instr::DadosTopo;
        Instr::DadosTopo = destino;
        return true;
    }
// Separa texto
    int valor = 10;
    if (comparaZ(nome, "separar")==0)
        valor=0;
    else if (comparaZ(nome, "separarmai")==0)
        valor=1;
    else if (comparaZ(nome, "separardif")==0)
        valor=2;
    if (valor!=10)
    {
        int indice = 0;
        int indmax = numvar;
        char * destino = v->end_char;
        while (true)
        {
            if (Instr::VarAtual < v+2)
                break;
            if (Instr::VarAtual >= v+3)
            {
                indmax = v[3].getInt();
                if (indmax<1)
                    break;
                if (indmax>numvar)
                    indmax=numvar;
            }
        // Obtém delimitador
            TProcurar proc;
            const char * origem = v[2].getTxt();
            int tampadrao = strlen(origem);
            if (!proc.Padrao(origem, valor))
                break;
        // Obtém texto
            origem = v[1].getTxt();
            int tam = strlen(origem);
        // Anota texto
            while (true)
            {
                indice++;
                int posic = (indice>=indmax ? -1 : proc.Proc(origem, tam));
                if (posic<0) // Não encontrou
                {
                    copiastr(destino, origem, tamvar);
                    destino += tamvar;
                    break;
                }
                int total = (posic < tamvar-1 ? posic : tamvar-1);
                memcpy(destino, origem, total);
                destino[total] = 0;
                origem += posic + tampadrao;
                tam -= posic + tampadrao;
                destino += tamvar;
            }
            break;
        }
        // Limpa próximas variáveis do vetor
        for (; indice < numvar; destino += tamvar, numvar--)
            *destino=0;
        // Retorna o número de variáveis
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(indice);
    }
    return false;
}

//const char Int1_Valor[] = { 8, 0, Instr::cInt1, 0, 0, 1, '=', '0', 0 };

//------------------------------------------------------------------------------
bool FuncVetorInt1(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar")==0)
    {
        unsigned int total = (unsigned char)v->defvar[Instr::endVetor];
        unsigned char * p = (unsigned char*)v->end_char;
        unsigned char bitmask = v->bit;
        if (bitmask > 1)
        {
            for (; total>0 && bitmask; total--,bitmask<<=1)
                *p &= ~bitmask;
            p++;
        }
        while (total>=8)
            *p++=0, total-=8;
        for (bitmask=1; total>0; total--,bitmask<<=1)
            *p &= ~bitmask;
        return false;
    }
    if (comparaZ(nome, "bits")==0)
    {
        Instr::ApagarVar(v+1);
        v->indice = 0;
        v->numfunc = 1;
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
int GetVetorInt1(TVariavel * v)
{
    unsigned int bitmask = v->bit;
    unsigned int bitnum = 0;
    int total = (unsigned char)v->defvar[Instr::endVetor];
    unsigned char * p = (unsigned char *)v->end_char;
    int valor = 0;
// Obtém o número do bit inicial, de 0 a 7
    if (bitmask & 0xF0)
        bitnum+=4, bitmask>>=4;
    if (bitmask & 0x0C)
        bitnum+=2, bitmask>>=2;
    if (bitmask & 2)
        bitnum++;
// Primeiro byte
    if (total>0 && bitnum)
    {
        valor = *p++ >> bitnum;
        bitnum = 8-bitnum;
        switch (total)
        {
        case 1: valor &= 1; break;
        case 2: valor &= 3; break;
        case 3: valor &= 7; break;
        case 4: valor &= 15; break;
        case 5: valor &= 31; break;
        case 6: valor &= 63; break;
        case 7: valor &= 127; break;
        }
        total -= bitnum;
    }
// Demais bytes
    for (; total>0 && bitnum<32; total-=8,bitnum+=8)
        switch (total)
        {
        case 1: valor |= (*p & 1) << bitnum; break;
        case 2: valor |= (*p & 3) << bitnum; break;
        case 3: valor |= (*p & 7) << bitnum; break;
        case 4: valor |= (*p & 15) << bitnum; break;
        case 5: valor |= (*p & 31) << bitnum; break;
        case 6: valor |= (*p & 63) << bitnum; break;
        case 7: valor |= (*p & 127) << bitnum; break;
        default: valor |= *p++ << bitnum;
        }
    return valor;
}

//------------------------------------------------------------------------------
void SetVetorInt1(TVariavel * v, int valor)
{
    unsigned int bitmask = v->bit;
    int bitnum = 0;
    int total = (unsigned char)v->defvar[Instr::endVetor];
    unsigned char * p = (unsigned char *)v->end_char;
// Obtém o número do bit inicial, de 0 a 7
    if (bitmask & 0xF0)
        bitnum+=4, bitmask>>=4;
    if (bitmask & 0x0C)
        bitnum+=2, bitmask>>=2;
    if (bitmask & 2)
        bitnum++;
// Altera as variáveis
    while (total > 0)
    {
        switch (total)
        {
        case 1: bitmask = 1; break;
        case 2: bitmask = 3; break;
        case 3: bitmask = 7; break;
        case 4: bitmask = 15; break;
        case 5: bitmask = 31; break;
        case 6: bitmask = 63; break;
        case 7: bitmask = 127; break;
        default: bitmask = 255; break;
        }
    // Bits já estão alinhados
        if (bitnum == 0)
        {
            *p &= ~bitmask;
            *p |= (valor & bitmask);
            p++, total-=8, valor>>=8;
            continue;
        }
    // Bits não estão alinhados
        *p &= ~(bitmask << bitnum);
        *p |= (valor & bitmask) << bitnum;
        p++, total-=8-bitnum, valor>>=8-bitnum;
        bitnum = 0;
    }
}
