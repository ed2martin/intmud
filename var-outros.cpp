/* Este programa � software livre; voc� pode redistribuir e/ou
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
// Verifica se o endere�o vai mudar
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
    // Avan�a TempoMenos
    // Move objetos de VetMais para VetMenos se necess�rio
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
                // Cria argumento: �ndice
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
// Menos que zero: n�o est� em nenhuma lista
    if (valor<=0)
        return;
// M�ximo 0xFFFF
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

//------------------------------------------------------------------------------
bool FuncVetorTxt(TVariavel * v, const char * nome)
{
// Obt�m n�mero de vari�veis e tamanho de uma vari�vel
    int numvar = (unsigned char)v->defvar[Instr::endVetor];
    int tamvar = (unsigned char)v->defvar[Instr::endIndice] + 2;
    if (v->defvar[2] == Instr::cTxt2)
        tamvar += 256;
// Texto do vetor
    if (comparaZ(nome, "texto")==0)
    {
    // Obt�m �ndice inicial e a quantidade de vari�veis
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
    // Cria vari�vel
        const char * origem = v->end_char + ini * tamvar;
        Instr::ApagarVar(v);  // N�o tem import�ncia que v seja apagado aqui
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
    // Acerta a vari�vel
        Instr::VarAtual->endvar = Instr::DadosTopo;
        Instr::VarAtual->tamanho = destino - Instr::DadosTopo;
        Instr::DadosTopo = destino;
        return true;
    }
// Divide em palavras
    if (comparaZ(nome, "palavras")==0)
    {
    // Obt�m n�mero de palavras (par�metro de entrada)
        int total = numvar;
        if (Instr::VarAtual < v+1)
            return false;
        if (Instr::VarAtual >= v+2)
        {
            total = v[2].getInt();
            if (total>numvar)
                total=numvar;
        }
    // Copia o texto obtendo o n�mero de palavras
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
    // Copia o �ltimo texto
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
    // Limpa pr�ximas vari�veis do vetor
        for (; palavras < numvar; destino += tamvar, numvar--)
            *destino=0;
    // Retorna o n�mero de palavras
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(palavras);
    }
// Divide em linhas
    if (comparaZ(nome, "linhas")==0)
    {
    // Obt�m n�mero m�nimo de colunas (par�metro de entrada)
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
        int linha = 0;  // N�mero de linhas copiadas
        colunas -= tamvar-1; // Quantas colunas recuar do final do texto
        while (linha<numvar)
        {
            char * d = destino;
            linha++, destino+=tamvar;
        // Copia linha
            while (*origem && *origem!=Instr::ex_barra_n && d<destino)
                *d++ = *origem++;
        // Linha cabe na vari�vel
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
        // Obt�m aonde deve dividir a linha
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
    // Limpa pr�ximas vari�veis do vetor
        for (; linha < numvar; destino += tamvar, numvar--)
            *destino=0;
    // Retorna o n�mero de palavras
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(linha);
    }
// Limpa as vari�veis do vetor
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
    // Obt�m o texto delimitador
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
    // Cria vari�vel
        const char * origem = v->end_char;
        Instr::ApagarVar(v);  // N�o tem import�ncia que v seja apagado aqui
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
    // Acerta a vari�vel
        Instr::VarAtual->endvar = Instr::DadosTopo;
        Instr::VarAtual->tamanho = destino - Instr::DadosTopo;
        Instr::DadosTopo = destino;
        return true;
    }
// Separa texto
    int valor = 2;
    if (comparaZ(nome, "separar")==0)
        valor=0;
    else if (comparaZ(nome, "separardif")==0)
        valor=1;
    if (valor!=2)
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
        // Obt�m delimitador
            TProcurar proc;
            const char * origem = v[2].getTxt();
            int tampadrao = strlen(origem);
            if (!proc.Padrao(origem, valor))
                break;
        // Obt�m texto
            origem = v[1].getTxt();
            int tam = strlen(origem);
        // Anota texto
            while (true)
            {
                indice++;
                int posic = (indice>=indmax ? -1 : proc.Proc(origem, tam));
                if (posic<0) // N�o encontrou
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
        // Limpa pr�ximas vari�veis do vetor
        for (; indice < numvar; destino += tamvar, numvar--)
            *destino=0;
        // Retorna o n�mero de vari�veis
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(indice);
    }
    return false;
}