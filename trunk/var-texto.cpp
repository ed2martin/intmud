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
/// Obtém o número de linhas correspondente a um texto
static int NumLinhas(const char * texto, int total)
{
    int linhas=0;
    for (; total>0; total--)
        if (*texto==Instr::ex_barra_n)
            linhas++;
    return linhas;
}

//----------------------------------------------------------------------------
/// Copia texto e retorna número de linhas
/** @param destino Endereço destino
 *  @param origem Endereço origem
 *  @param total Número de bytes a copiar
 *  @return Número de caracteres Instr::ex_barra_n copiados
 *  @note O caracter 0 é anotado como Instr::ex_barra_n */
static int AnotaLinhas(char * destino, const char * origem, int total)
{
    int linhas=0;
    for (; total>0; total--, origem++)
        if (*origem==0 || *origem==Instr::ex_barra_n)
            *destino++ = Instr::ex_barra_n, linhas++;
        else
            *destino++ = *origem;
    return linhas;
}

//----------------------------------------------------------------------------
void TTextoTxt::Apagar()
{
// Otimização, no caso de precisar mover algum bloco que será apagado
    for (TTextoBloco * obj = Inicio; obj; obj=obj->Depois)
        obj->Bytes = 0;
// Acerta objetos TTextoPos
    while (Posic)
        Posic->MudarTxt(0);
// Apaga blocos
    while (Inicio)
        Inicio->Apagar();
}

//----------------------------------------------------------------------------
void TTextoTxt::Mover(TTextoTxt * destino)
{
    for (TTextoPos * obj = Posic; obj; obj=obj->Depois)
        obj->TextoTxt = destino;
    for (TTextoBloco * obj = Inicio; obj; obj=obj->Depois)
        obj->TextoTxt = destino;
    move_mem(destino, this, sizeof(TTextoTxt));
}

//----------------------------------------------------------------------------
void TTextoTxt::IniBloco()
{
    if (Inicio)
        return;
    TTextoBloco * bloco = TTextoGrupo::Criar();
    bloco->TextoTxt = this;
    bloco->Antes = 0;
    bloco->Depois = 0;
    bloco->Linhas=0;
    bloco->Bytes=0;
    Inicio = Fim = bloco;
    for (TTextoPos * obj = Posic; obj; obj=obj->Depois)
    {
        obj->Bloco = bloco;
        obj->PosicBloco = 0;
        obj->PosicTxt = 0;
        obj->LinhaTxt = 0;
    }
}

//----------------------------------------------------------------------------
void TTextoPos::Apagar()
{
    if (TextoTxt==0)
        return;
    (Antes ? Antes->Depois : TextoTxt->Posic) = Depois;
    if (Depois)
        Depois->Antes = Antes;
    TextoTxt=0;
}

//----------------------------------------------------------------------------
void TTextoPos::Mover(TTextoPos * destino)
{
    if (TextoTxt)
    {
        (Antes ? Antes->Depois : TextoTxt->Posic) = destino;
        if (Depois)
            Depois->Antes = destino;
    }
    move_mem(destino, this, sizeof(TTextoPos));
}

//----------------------------------------------------------------------------
void TTextoPos::MudarTxt(TTextoTxt * obj)
{
// Se 0, remove da lista
    if (obj==0)
    {
        Apagar();
        return;
    }
// Associa ao objeto TTextoTxt
    if (obj!=TextoTxt)
    {
        Apagar();
        TextoTxt = obj;
        Antes = 0;
        Depois = TextoTxt->Posic;
        if (Depois)
            Depois->Antes = this;
        TextoTxt->Posic = this;
    }
    //Bloco = TextoTxt->Inicio;
    //PosicBloco = 0;
    //PosicTxt = 0;
    //LinhaTxt = 0;
}

//----------------------------------------------------------------------------
TTextoBloco * TTextoBloco::CriarAntes()
{
    TTextoBloco * obj = TTextoGrupo::Criar();
    obj->TextoTxt = TextoTxt;
    obj->Antes = Antes;
    obj->Depois = this;
    obj->Linhas = 0;
    obj->Bytes = 0;
    (Antes ? Antes->Depois : TextoTxt->Inicio) = obj;
    Antes = obj;
    return obj;
}

//----------------------------------------------------------------------------
TTextoBloco * TTextoBloco::CriarDepois()
{
    TTextoBloco * obj = TTextoGrupo::Criar();
    obj->TextoTxt = TextoTxt;
    obj->Antes = this;
    obj->Depois = Depois;
    obj->Linhas = 0;
    obj->Bytes = 0;
    (Depois ? Depois->Antes : TextoTxt->Fim) = obj;
    Depois = obj;
    return obj;
}

//----------------------------------------------------------------------------
TTextoBloco * TTextoBloco::Apagar()
{
// Acerta ponteiros
    (Antes ? Antes->Depois : TextoTxt->Inicio) = Depois;
    (Depois ? Depois->Antes : TextoTxt->Fim) = Antes;
    for (TTextoPos * obj = TextoTxt->Posic; obj; obj=obj->Depois)
        if (obj->Bloco==this)
            obj->Bloco=0;
// Apaga objeto
    return TTextoGrupo::Apagar(this);
}

//----------------------------------------------------------------------------
void TTextoBloco::Mover(TTextoBloco * destino)
{
// Acerta ponteiros
    (Antes ? Antes->Depois : TextoTxt->Inicio) = destino;
    (Depois ? Depois->Antes : TextoTxt->Fim) = destino;
    for (TTextoPos * obj = TextoTxt->Posic; obj; obj=obj->Depois)
        if (obj->Bloco==this)
            obj->Bloco=destino;
// Move na memória
    int total = Texto + Bytes - (char*)this;
    assert(total <= TOTAL_TXTOBJ);
    memcpy(destino, this, total);
}

//----------------------------------------------------------------------------
int TTextoBloco::LinhasBytes(unsigned int posic, int numlinhas)
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
int TTextoBloco::CopiarTxt(unsigned int posic, char * buffer, int tambuf)
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
    if (posic>=Bytes)
    {
        obj=Depois, posic=0;
        if (obj==0)
        {
            *buffer=0;
            return 1;
        }
    }
// Copia bytes
    while (tambuf > (int)obj->Bytes - (int)posic)
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
void TBlocoPos::MudarTxt(const char * texto, unsigned int tamtexto,
        unsigned int tamapagar)
{
    const unsigned int bloco_ini = Bloco->Texto - (char*)Bloco;
    const unsigned int bloco_tam = TOTAL_TXTOBJ - bloco_ini;

    int add_bytes = tamtexto;   // Quantidade de bytes inseridos
    int sub_bytes = 0;          // Quantidade de bytes removidos
    int dif_linhas = 0;         // = linhas inseridas - linhas removidas
    TTextoTxt * TextoTxt = Bloco->TextoTxt;
    TTextoBloco * obj;          // Bloco atual
    int posic;                  // Posição no bloco atual

// Acerta posic no início ou no final do bloco
// Nota: PosicBloco só será 0 se estiver no começo do texto
    if (PosicBloco > Bloco->Bytes)
        PosicBloco = Bloco->Bytes;
    else if (PosicBloco==0 && Bloco->Antes)
        Bloco = Bloco->Antes, PosicBloco = Bloco->Bytes;
    posic = PosicBloco;
    obj = Bloco;

// Limpa blocos; preenche com texto se possível
    while (tamapagar + posic >= obj->Bytes)
    {
    // Apaga bytes do bloco a partir de posic
        tamapagar -= obj->Bytes - posic;
        if (posic==0)
        {
            sub_bytes += obj->Bytes;
            dif_linhas -= obj->Linhas;
            obj->Bytes=0, obj->Linhas=0;
        }
        else
        {
            int lin = NumLinhas(obj->Texto + posic, obj->Bytes - posic);
            sub_bytes += obj->Bytes - posic;
            dif_linhas -= lin;
            obj->Bytes = posic;
            obj->Linhas -= lin;
        }
    // Anota texto no bloco a partir de posic
        if (tamtexto)
        {
            unsigned int copiar = bloco_tam - posic;
                                // = qunatos bytes cabem no bloco
            if (copiar > tamtexto)
                copiar = tamtexto, tamtexto = 0; // copiar todo o texto
            else
                tamtexto -= copiar; // copiar parte do texto
            int lin = AnotaLinhas(obj->Texto + posic, texto, copiar);
            obj->Bytes += copiar;
            obj->Linhas += lin;
            dif_linhas += lin;
            texto += copiar;
        }
    // Passa para o próximo bloco
        if (obj->Depois)
        {
            posic = 0;
            obj = obj->Depois;
            continue;
        }
    // Encontrou o fim do texto (não há próximo bloco)
        tamapagar = 0;
        posic = obj->Bytes;
        break;
    }

// Apaga bytes do bloco
// Nota: tamapagar < obj->Bytes - posic
    if (tamapagar)
    {
        char * p = obj->Texto + posic;
        int lin = NumLinhas(p, tamapagar);
        obj->Bytes -= tamapagar;
        obj->Linhas -= lin;
        sub_bytes += tamapagar;
        dif_linhas -= lin;
        memcpy(p, p+tamapagar, obj->Bytes-posic);
        //tamapagar = 0;
    }

// Inserir bytes
    while (tamtexto)
    {
    // Apenas insere bytes no bloco se couber
        if (tamtexto + obj->Bytes <= bloco_tam)
        {
            char * p = obj->Texto + posic;
            move_mem(p + tamtexto, p, obj->Bytes - posic);
            int lin = AnotaLinhas(p, texto, tamtexto);
            obj->Bytes += tamtexto;
            obj->Linhas += lin;
            dif_linhas += lin;
            // tamtexto = 0;
            break;
        }
    // Separa final do bloco
        char buffer[0x100];
        unsigned int tambuf = obj->Bytes - posic;
        int  linhabuf = AnotaLinhas(buffer, obj->Texto + posic, tambuf);
        obj->Linhas -= linhabuf;
        obj->Bytes = posic;
    // Insere bytes
        while (true)
        {
            unsigned int copiar = bloco_tam - obj->Bytes;
            if (copiar > tamtexto)
                copiar = tamtexto;
            int lin = AnotaLinhas(obj->Texto + obj->Bytes, texto, copiar);
            obj->Bytes += copiar;
            obj->Linhas += lin;
            dif_linhas += copiar;
            tamtexto -= copiar;
            texto += copiar;
            if (tamtexto==0)
                break;
            obj = obj->CriarDepois();
        }
    // Insere final do bloco
        if (tambuf)
        {
            if (obj->Bytes + tambuf > bloco_tam)
                obj = obj->CriarDepois();
            memcpy(obj->Texto + obj->Bytes, buffer, tambuf);
            obj->Bytes += tambuf;
            obj->Linhas += linhabuf;
        }
        break;
    } // while (tamtexto)

// Apaga objetos vazios
    if (obj->Bytes && obj->Antes)
        obj = obj->Antes;
    while (obj->Bytes==0)
    {
        TTextoBloco * outro = obj->Antes;
        if (outro==0)
            outro = obj->Depois;
        if (outro != obj->Apagar())
            obj = outro;
        if (obj)
            continue;
    // Texto ficou vazio
        for (TTextoPos * pos = TextoTxt->Posic; pos; pos=pos->Depois)
        {
            pos->Bloco = 0;
            pos->PosicBloco = 0;
            pos->PosicTxt = 0;
            pos->LinhaTxt = 0;
        }
        TextoTxt->Bytes = 0;
        TextoTxt->Linhas = 0;
        return;
    }

// Acerta tamanho do texto
    TextoTxt->Bytes += add_bytes - sub_bytes;
    TextoTxt->Linhas += dif_linhas;

// Se PosicBloco==0, está no início do texto
    if (PosicBloco==0)
        Bloco=TextoTxt->Inicio, PosicTxt=0, LinhaTxt=0;

// Obtém bloco e posição após o texto adicionado
// Posição no arquivo = PosicTxt + add_bytes
    obj = Bloco;
    posic = PosicBloco + add_bytes;
    while (posic > obj->Bytes)
        posic -= obj->Bytes, obj = obj->Depois;

// Acerta objetos TTextoPos
    for (TTextoPos * pos = TextoTxt->Posic; pos; pos=pos->Depois)
    {
    // Antes do texto modificado: nada faz
        if (pos->PosicTxt < PosicTxt)
            continue;

    // Na região do texto apagado: move para o início da modificação
        if (pos->PosicTxt <= PosicTxt + sub_bytes)
        {
            pos->Bloco = Bloco;
            pos->PosicBloco = PosicBloco;
            pos->PosicTxt = PosicTxt;
            pos->LinhaTxt = LinhaTxt;
            continue;
        }

    // Após a região do texto modificado: acerta posição no texto
        pos->PosicTxt += add_bytes - sub_bytes;
        pos->LinhaTxt += dif_linhas;

    // Verifica se deve acertar bloco e posição no bloco
        unsigned int mais = pos->PosicTxt - PosicTxt - add_bytes;
        if (mais > bloco_tam)
            continue;

    // Acerta bloco e posição no bloco
        TTextoBloco * bl = obj;
        mais += posic;
        while (mais > bl->Bytes)
            mais -= bl->Bytes, bl = bl->Depois;
        pos->Bloco = bl;
        pos->PosicBloco = mais;
    }

// Obtém objeto inicial e número de bytes sobrando
    TTextoBloco * ini = obj;
    int sobrando = bloco_tam - ini->Bytes;
    if (ini->Antes)
    {
        ini=ini->Antes, sobrando += bloco_tam - ini->Bytes;
        if (ini->Antes)
            ini=ini->Antes, sobrando += bloco_tam - ini->Bytes;
    }
    if (obj->Depois)
    {
        sobrando += bloco_tam - obj->Depois->Bytes;
        if (obj->Depois->Depois)
            sobrando += bloco_tam - obj->Depois->Depois->Bytes;
    }

// Verifica se consegue apagar algum objeto
    if (sobrando < (int)bloco_tam)
        return;

// Compacta o texto, apagando bloco se possível
    while (ini->Bytes == bloco_tam)
        ini = ini->Depois;
    while (true)
    {
        TTextoBloco * proximo = ini->Depois;
    // Obtém número de bytes disponíveis em ini
        unsigned int total = bloco_tam - ini->Bytes;
    // Se pode copiar o bloco inteiro...
        if (total >= proximo->Bytes)
        {
        // Acerta objetos TTextoPos
            for (TTextoPos * pos = TextoTxt->Posic; pos; pos=pos->Depois)
                if (pos->Bloco == proximo)
                {
                    pos->Bloco = ini;
                    pos->PosicBloco += ini->Bytes;
                }
        // Copia bloco
            memcpy(ini->Texto + ini->Bytes, proximo->Texto, proximo->Bytes);
            ini->Bytes += proximo->Bytes;
            ini->Linhas += proximo->Linhas;
        // Apaga objeto
            proximo->Apagar();
            return;
        }
    // Acerta objetos TTextoPos
        for (TTextoPos * pos = TextoTxt->Posic; pos; pos=pos->Depois)
            if (pos->Bloco == proximo)
            {
                if (pos->PosicBloco < total)
                    pos->PosicBloco += ini->Bytes, pos->Bloco = ini;
                else
                    pos->PosicBloco -= total;
            }
    // Copia parte do bloco
        int linhas = NumLinhas(proximo->Texto, total);
        memcpy(ini->Texto + ini->Bytes, proximo->Texto, total);
        memcpy(proximo->Texto, proximo->Texto + total, proximo->Bytes - total);
        ini->Bytes += total;
        ini->Linhas += linhas;
        proximo->Bytes -= total;
        proximo->Linhas -= linhas;
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

//----------------------------------------------------------------------------
bool TTextoTxt::Func(TVariavel * v, const char * nome)
{
    /*
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
    */
    return false;
}

//----------------------------------------------------------------------------
int TTextoTxt::getValor()
{
    return Inicio!=0;
}

//----------------------------------------------------------------------------
bool TTextoPos::Func(TVariavel * v, const char * nome)
{
    return false;
}

//----------------------------------------------------------------------------
int TTextoPos::getValor()
{
    return (TextoTxt && PosicTxt < TextoTxt->Bytes);
}
