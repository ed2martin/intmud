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
#include "var-texto.h"
#include "variavel.h"
#include "instr.h"
#include "misc.h"

//----------------------------------------------------------------------------
TTextoGrupo * TTextoGrupo::Disp = 0;
TTextoGrupo * TTextoGrupo::Usado = 0;
unsigned long TTextoGrupo::Tempo = 0;

//----------------------------------------------------------------------------
void TTextoTxt::Apagar()
{
    // Otimização, no caso de precisar mover algum bloco que será apagado
    for (TTextoBloco * obj = Inicio; obj; obj=obj->Depois)
        obj->Bytes = 0;
    // Apaga blocos
    while (Inicio)
        Inicio->Apagar();
}

//----------------------------------------------------------------------------
void TTextoTxt::Mover(TTextoTxt * destino)
{
    // Acerta TTextoBLoco::TextoTxt em todos os TTextoBLoco do objeto
    for (TTextoBloco * obj = Inicio; obj; obj=obj->Depois)
        obj->TextoTxt = destino;
    move_mem(destino, this, sizeof(TTextoTxt));
}

//----------------------------------------------------------------------------
bool TTextoTxt::Func(TVariavel * v, const char * nome)
{
// Adiciona texto no início
    if (comparaZ(nome, "addini")==0)
    {
        for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
        {
            const char * texto = v1->getTxt();
            if (Inicio==0)
                CriarFim();
            Inicio->AddTxt(0, texto, true);
        }
        Instr::ApagarVar(v);
        return false;
    }
// Adiciona texto no fim
    if (comparaZ(nome, "addfim")==0)
    {
        for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
        {
            const char * texto = v1->getTxt();
            if (Fim==0)
                CriarFim();
            Fim->AddTxt(0x100, texto, true);
        }
        Instr::ApagarVar(v);
        return false;
    }
// Remove todos os textos
    if (comparaZ(nome, "limpar")==0)
    {
        for (TTextoBloco * obj = Inicio; obj; obj=obj->Depois)
            obj->Bytes = 0;
        while (Inicio)
            Inicio->Apagar();
        return false;
    }
// Remove uma quantidade de linhas
    if (comparaZ(nome, "remove")==0)
    {
        if (Instr::VarAtual < v+1)
            return false;
        int linhas = v[1].getInt();
        Instr::ApagarVar(v); // Nota: se apagar o TextoTxt, Inicio será 0
        if (linhas<=0 || Inicio==0)
            return false;
    // Obtém o número de bytes
        int total = Inicio->LinhasBytes(0, linhas);
    // Cria variável e aloca memória para o texto
        if (!Instr::CriarVarTexto(0, total-1))
            return false;
    // Obtém tamanho da memória alocada
        total = Instr::VarAtual->tamanho;
    // Anota o texto
        Inicio->CopiarTxt(0, Instr::VarAtual->end_char, total);
    // Apaga texto de ListaTxt
        Inicio->ApagarTxt(0, total);
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
int TTextoTxt::getValor()
{
    return Inicio!=0;
}

//----------------------------------------------------------------------------
void TTextoPos::Apagar() { }
void TTextoPos::Mover(TTextoPos * destino)
    { move_mem(destino, this, sizeof(TTextoPos)); }
bool TTextoPos::Func(TVariavel * v, const char * nome) { return false; }
int TTextoPos::getValor() { return 0; }


//----------------------------------------------------------------------------
void TTextoTxt::CriarFim()
{
    TTextoBloco * objnovo = TTextoGrupo::Criar();
    objnovo->TextoTxt = this;
    objnovo->Antes = Fim;
    objnovo->Depois = 0;
    objnovo->Linhas = 0;
    objnovo->Bytes = 0;
    (Fim ? Fim->Depois : Inicio) = objnovo;
    Fim = objnovo;
}

//----------------------------------------------------------------------------
TTextoBloco * TTextoBloco::Apagar()
{
    (Antes ? Antes->Depois : TextoTxt->Inicio) = Depois;
    (Depois ? Depois->Antes : TextoTxt->Fim) = Antes;
    return TTextoGrupo::Apagar(this);
}

//----------------------------------------------------------------------------
void TTextoBloco::Mover(TTextoBloco * destino)
{
    (Antes ? Antes->Depois : TextoTxt->Inicio) = destino;
    (Depois ? Depois->Antes : TextoTxt->Fim) = destino;
    int total = Bytes + (Texto - ObjChar);
    assert(total <= TOTAL_TXTOBJ);
    memcpy(destino, this, total);
}

//----------------------------------------------------------------------------
void TTextoBloco::AddTxt(int posic, const char * texto, bool novalinha)
{
    char buffer[0x100];
    int  tambuf=0;
    int  linhabuf=0;

// Verifica se vai acrescentar alguma coisa
    if (*texto==0 && novalinha==false)
        return;

// Move texto a partir de posic para o buffer
    if (posic<=0)
    {
        memcpy(buffer, Texto, Bytes);
        tambuf = Bytes;
        linhabuf = Linhas;
        Bytes=0, Linhas=0;
    }
    else if (posic<Bytes)
    {
        memcpy(buffer, Texto+posic, Bytes-posic);
        tambuf = Bytes-posic;
        Bytes = posic;
        int lin=0;
        for (int p=0; p<tambuf; p++)
            if (buffer[p] == Instr::ex_barra_n)
                lin++;
        linhabuf+=lin, Linhas-=lin;
    }

// Adiciona texto
    TTextoBloco * obj = this;
    int addlinhas=0, addbytes=0;
    while (true)
    {
    // Obtém endereços do próximo byte e final
        char * destino = obj->Texto + obj->Bytes;
        char * destfim = obj->ObjChar + TOTAL_TXTOBJ;
    // Copia texto para o objeto
        int lin = 0;
        while (destino < destfim)
        {
            if (*texto == Instr::ex_barra_n)
                *destino++ = *texto++, lin++;
            else if (*texto)
                *destino++ = *texto++;
            else
            {
                if (novalinha)
                    *destino++ = Instr::ex_barra_n, lin++, novalinha=false;
                break;
            }
        }
    // Acerta número de bytes e linhas acrescentados
        addbytes += destino - obj->Texto;
        addlinhas += lin;
        obj->Bytes = destino - obj->Texto;
        obj->Linhas += lin;
    // Termina se fim do texto
        if (*texto==0 && !novalinha)
            break;
    // Cria novo objeto após obj
        TTextoBloco * objnovo = TTextoGrupo::Criar();
        objnovo->TextoTxt = TextoTxt;
        objnovo->Antes = obj;
        objnovo->Depois = obj->Depois;
        objnovo->Linhas = 0;
        objnovo->Bytes = 0;
        (objnovo->Depois ? objnovo->Depois->Antes : TextoTxt->Fim) = objnovo;
        obj->Depois = objnovo;
        obj = objnovo;
    }

// Acerta dados da TextoTxt
    TextoTxt->Linhas += addlinhas;
    TextoTxt->Bytes += addbytes;

// Adiciona texto do buffer
    while (tambuf)
    {
    // Obtém endereços do próximo byte e final
        char * destino = obj->Texto + obj->Bytes;
        char * destfim = obj->ObjChar + TOTAL_TXTOBJ;
    // Tem espaço: copia no final do objeto
        if (destfim-destino >= tambuf)
        {
            memcpy(destino, buffer, tambuf);
            obj->Bytes += tambuf;
            obj->Linhas += linhabuf;
            break;
        }
    // Não tem espaço: cria novo objeto e copia texto
        TTextoBloco * objnovo = TTextoGrupo::Criar();
        objnovo->TextoTxt = TextoTxt;
        objnovo->Antes = obj;
        objnovo->Depois = obj->Depois;
        objnovo->Linhas = linhabuf;
        objnovo->Bytes = tambuf;
        (objnovo->Depois ? objnovo->Depois->Antes : TextoTxt->Fim) = objnovo;
        obj->Depois = objnovo;
        obj = objnovo;
        memcpy(obj->Texto, buffer, tambuf);
        break;
    }

// Ajusta texto; apaga objetos se for possível
    obj->AjustarTxt();
}

//----------------------------------------------------------------------------
void TTextoBloco::ApagarTxt(int posic, int numbytes)
{
    TTextoBloco * obj = this;
    if (numbytes<=0)
        return;
// Acerta posic no início ou no final do bloco
    if (posic<0)
        posic=0;
    else if (posic>=Bytes)
    {
        obj=Depois, posic=0;
        if (obj==0)
            return;
    }
// Apaga até o fim do bloco
    else if (posic+numbytes >= Bytes)
    {
        int lin=0;
        for (int x=posic; x<Bytes; x++)
            if (obj->Texto[x] == Instr::ex_barra_n)
                lin++;
        numbytes -= Bytes - posic;
        TextoTxt->Bytes -= Bytes - posic;
        TextoTxt->Linhas -= lin;
        Bytes = posic;
        Linhas -= lin;
        if (Depois==0)
            numbytes=0;
        else
            obj=Depois;
        posic=0;
    }
// Limpa blocos inteiros (indica que estão vazios)
    while (numbytes >= obj->Bytes)
    {
        numbytes -= obj->Bytes;
        TextoTxt->Bytes -= obj->Bytes;
        TextoTxt->Linhas -= obj->Linhas;
        obj->Bytes = 0;
        obj->Linhas = 0;
        if (obj->Depois==0)
        {
            numbytes = 0;
            break;
        }
        obj = obj->Depois;
    }
// Apaga uma parte do bloco
    if (numbytes > 0)
    {
        int origem = posic + numbytes;
        int lin=0;
        for (int x=posic; x<origem; x++)
            if (obj->Texto[x] == Instr::ex_barra_n)
                lin++;
        memcpy(obj->Texto + posic, obj->Texto + origem, obj->Bytes - origem);
        TextoTxt->Linhas -= lin;
        TextoTxt->Bytes -= numbytes;
        obj->Linhas -= lin;
        obj->Bytes -= numbytes;
    }
// Apaga blocos vazios
    obj = (Bytes==0 || Depois==0 ? this : Depois);
    while (obj->Bytes==0)
    {
        TTextoBloco * dep = obj->Depois;
        if (dep==0)
            dep = obj->Antes;
        if (dep != obj->Apagar())
            obj = dep;
        if (obj==0)
            return;
    }
// Ajusta texto; apaga objetos se for possível
    obj->AjustarTxt();
}

//----------------------------------------------------------------------------
int TTextoBloco::CopiarTxt(int posic, char * buffer, int tambuf)
{
    const char * bufini = buffer;
    if (tambuf<=1)
    {
        *buffer=0;
        return 1;
    }
    tambuf--;
    TTextoBloco * obj = this;
// Acerta posic no início ou no final do bloco
    if (posic<0)
        posic=0;
    else if (posic>=Bytes)
    {
        obj=Depois, posic=0;
        if (obj==0)
        {
            *buffer=0;
            return 1;
        }
    }
// Copia bytes
    while (tambuf > obj->Bytes - posic)
    {
        int total = obj->Bytes - posic;
        memcpy(buffer, obj->Texto+posic, total);
        buffer += total;
        tambuf -= total;
        obj=Depois, posic=0;
        if (obj==0)
        {
            *buffer=0;
            return buffer-bufini+1;
        }
    }
// Copia bytes do último bloco
    if (tambuf > 0)
    {
        memcpy(buffer, obj->Texto+posic, tambuf);
        buffer += tambuf;
    }
    *buffer=0;
    return buffer-bufini+1;
}

//----------------------------------------------------------------------------
int TTextoBloco::LinhasBytes(int posic, int numlinhas)
{
    TTextoBloco * obj = this;
    int total = 0;
    if (numlinhas<=0)
        return 0;
// Avança até o início de algum bloco
    if (posic>0)
    {
        if (posic<Bytes)
        {
            for (int x=posic; x<obj->Bytes; x++)
                if (obj->Texto[x] == Instr::ex_barra_n)
                    if (--numlinhas==0)
                        return x+1-posic;
            total = obj->Bytes - posic;
        }
        obj=Depois;
        if (obj==0)
            return 0;
    }
// Avança blocos inteiros
    while (numlinhas > obj->Linhas)
    {
        numlinhas -= obj->Linhas;
        total += obj->Bytes;
        obj = obj->Depois;
        if (obj==0)
            return total;
    }
// Avança em um bloco
    for (int x=0; x<obj->Bytes; x++)
        if (obj->Texto[x] == Instr::ex_barra_n)
            if (--numlinhas==0)
                return total+x+1;
    return total + obj->Bytes;
}

//----------------------------------------------------------------------------
void TTextoBloco::AjustarTxt()
{
    const int objbytes = ObjChar + TOTAL_TXTOBJ - Texto;
// Verifica se objeto está vazio
    if (Bytes==0)
    {
        (Antes ? Antes->Depois : TextoTxt->Inicio) = Depois;
        (Depois ? Depois->Antes : TextoTxt->Fim) = Antes;
        TTextoGrupo::Apagar(this);
        return;
    }
// Obtém objetos inicial e final e número de bytes
    TTextoBloco * ini = this;
    TTextoBloco * fim = this;
    int maximo = objbytes;
    int atual = Bytes;
    if (ini->Antes)
    {
        ini=ini->Antes, atual+=ini->Bytes, maximo+=objbytes;
        if (ini->Antes)
            ini=ini->Antes, atual+=ini->Bytes, maximo+=objbytes;
    }
    if (ini->Depois)
    {
        fim=fim->Depois, atual+=fim->Bytes, maximo+=objbytes;
        if (ini->Depois)
            fim=fim->Depois, atual+=fim->Bytes, maximo+=objbytes;
    }
// Verifica se consegue apagar algum objeto
    if (maximo - atual < objbytes)
        return;
// Compacta o texto
    TTextoBloco * obj = ini->Depois;
    while (true)
    {
    // Obtém número de bytes disponíveis em ini
        int total = objbytes - ini->Bytes;
    // Se pode copiar o bloco inteiro...
        if (total >= obj->Bytes)
        {
        // Copia bloco
            total = obj->Bytes;
            memcpy(ini->Texto + ini->Bytes, obj->Texto, total);
            ini->Bytes += total;
            ini->Linhas += obj->Linhas;
        // Apaga objeto
            obj->Apagar();
            return;
        }
    // Obtém número de linhas
        int linhas = 0;
        for (const char * p = obj->Texto; p < obj->Texto + total; p++)
            if (*p == Instr::ex_barra_n)
                linhas++;
    // Copia parte do bloco
        memcpy(ini->Texto + ini->Bytes, obj->Texto, total);
        memcpy(obj->Texto, obj->Texto + total, obj->Bytes - total);
        ini->Bytes += total;
        ini->Linhas += linhas;
        obj->Bytes -= total;
        obj->Linhas -= linhas;
    }
}

//----------------------------------------------------------------------------
TTextoBloco * TTextoGrupo::Criar()
{
// Se tem objeto TListaX disponível...
    if (Usado && Usado->Total < TOTAL_TXTGR)
        return (TTextoBloco*)Usado->Lista[Usado->Total++];
// Se não tem objeto disponível...
    TTextoGrupo * obj;
    if (Disp==0)    // Não tem objeto TTextoGrupo disponível
        obj=new TTextoGrupo;
    else            // Tem objeto TTextoGrupo disponível
        obj=Disp, Disp=Disp->Depois; // Retira da lista Disp
    obj->Total = 1;
    obj->Depois = Usado; // Coloca na lista Usado
    Usado = obj;
    return (TTextoBloco*)obj->Lista[0];
}

//----------------------------------------------------------------------------
TTextoBloco * TTextoGrupo::Apagar(TTextoBloco * lista)
{
    TTextoBloco * lfim = (TTextoBloco*)Usado->Lista[Usado->Total - 1];
    if (lista != lfim)
        lfim->Mover(lista);
    Usado->Total--;
    if (Usado->Total==0)
    {
        if (Disp==0)
            Tempo = TempoIni;
        TTextoGrupo * obj = Usado;
        Usado = Usado->Depois; // Retira da lista Usado
        obj->Depois = Disp;    // Coloca na lista Disp
        Disp = obj;
    }
    return lfim;
}

//----------------------------------------------------------------------------
void TTextoGrupo::ProcEventos()
{
    if (Disp && Tempo+10 < TempoIni)
    {
        TTextoGrupo * obj = Disp;
        Disp = Disp->Depois; // Retira da lista Disp
        delete obj;
        Tempo = TempoIni;
    }
}
