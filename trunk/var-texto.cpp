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

//#define DEBUG_TXT // Texto guardado em TTextoTxt
//#define DEBUG_MSG // Alocação de memória

//----------------------------------------------------------------------------
TTextoGrupo * TTextoGrupo::Disp = 0;
TTextoGrupo * TTextoGrupo::Usado = 0;
unsigned long TTextoGrupo::Tempo = 0;
const char TextoItem1[] = { 7, 0, Instr::cTextoPos, 0, 0, 0, '+', 0 };

//----------------------------------------------------------------------------
#ifdef DEBUG_TXT
static void DebugTextoTxt(TTextoTxt * txt)
{
    if (txt==0)
        return;
// Checa lista ligada de TTextoPos
    for (TTextoPos * p = txt->Posic; p; p=p->Depois)
    {
        assert(p->Antes ? p->Antes->Depois==p : txt->Posic==p);
        assert(p->Depois==0 || p->Depois->Antes==p);
        assert(p->TextoTxt==txt);
        assert(p->PosicTxt <= txt->Bytes);
        assert(p->LinhaTxt <= txt->Linhas);
    }
// Checa lista ligada de TTextoBloco
    unsigned int lin_atual = 0;
    unsigned int byte_atual = 0;
    assert((txt->Inicio==0) == (txt->Fim==0));
    for (TTextoBloco * obj = txt->Inicio; obj; obj=obj->Depois)
    {
        assert(obj->Antes ? obj->Antes->Depois==obj : txt->Inicio==obj);
        assert(obj->Depois ? obj->Depois->Antes==obj : txt->Fim==obj);
        assert(obj->TextoTxt==txt);
        assert(obj->Bytes != 0);
        lin_atual += obj->Linhas;
        byte_atual += obj->Bytes;
    }
    assert(lin_atual == txt->Linhas);
    assert(byte_atual == txt->Bytes);
// Checa posição de TTextoPos
    for (TTextoPos * p = txt->Posic; p; p=p->Depois)
    {
    // Nenhum texto
        if (txt->Inicio==0)
        {
            assert(p->Bloco==0);
            continue;
        }
    // Obtém dados do bloco
        unsigned int lin_atual = 0;
        unsigned int byte_atual = 0;
        TTextoBloco * obj = txt->Inicio;
        while (obj != p->Bloco)
        {
            lin_atual += obj->Linhas;
            byte_atual += obj->Bytes;
            obj=obj->Depois;
        }
    // Checa bloco
        assert(p->PosicBloco <= obj->Bytes);
        byte_atual += p->PosicBloco;
        for (unsigned int x=0; x < p->PosicBloco; x++)
            if (obj->Texto[x]==Instr::ex_barra_n)
                lin_atual++;
        assert(byte_atual == p->PosicTxt);
        assert(lin_atual == p->LinhaTxt);
    // Checa se é início de linha
        if (p->LinhaTxt==0) // Início do texto
            assert(p->PosicBloco==0 && obj==txt->Inicio);
        else if (p->PosicBloco!=0) // Não está no início do bloco
            assert(obj->Texto[p->PosicBloco-1]==Instr::ex_barra_n);
        else // No início do bloco, checa último byte do bloco anterior
            assert(obj->Antes->Texto[obj->Antes->Bytes-1]==Instr::ex_barra_n);
    }
}
#else
#define DebugTextoTxt(x)
#endif

//----------------------------------------------------------------------------
/// Obtém o número de linhas correspondente a um texto
static int NumLinhas(const char * texto, int total)
{
    int linhas=0;
    for (; total>0; total--, texto++)
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
// Nota: mais eficiente que: while (Posic) Posic->MudarTxt(0);
    for (TTextoPos * obj = Posic; obj; obj=obj->Depois)
        obj->TextoTxt = 0, obj->Bloco = 0;
    Posic = 0;
// Apaga blocos
    while (Inicio)
        Inicio->Apagar();
}

//----------------------------------------------------------------------------
void TTextoTxt::Limpar()
{
// Otimização, no caso de precisar mover algum bloco que será apagado
    for (TTextoBloco * obj = Inicio; obj; obj=obj->Depois)
        obj->Bytes = 0;
// Acerta objetos TTextoPos
    for (TTextoPos * obj = Posic; obj; obj=obj->Depois)
    {
        obj->Bloco = 0;
        obj->PosicBloco = 0;
        obj->PosicTxt = 0;
        obj->LinhaTxt = 0;
    }
    Linhas = 0;
    Bytes = 0;
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
void TTextoTxt::AddTexto(const char * texto, unsigned int tamtexto)
{
    const unsigned int bloco_ini = Inicio->Texto - (char*)Inicio;
    const unsigned int bloco_tam = TOTAL_TXTOBJ - bloco_ini;
    if (Inicio==0)
        IniBloco();
    Bytes += tamtexto;
    while (true)
    {
        unsigned int copiar = bloco_tam - Fim->Bytes;
        if (copiar > tamtexto)
            copiar = tamtexto, tamtexto = 0; // copiar todo o texto
        else
            tamtexto -= copiar; // copiar parte do texto
        int lin = AnotaLinhas(Fim->Texto + Fim->Bytes, texto, copiar);
        Fim->Bytes += copiar;
        Fim->Linhas += lin;
        Linhas += lin;
        texto += copiar;
        if (tamtexto==0)
            break;
        if (Fim->Depois)
            Fim = Fim->Depois;
        else
            Fim = Fim->CriarDepois();
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
// Verifica se o endereço vai mudar
    if (obj == TextoTxt)
        return;
// Retira da lista
    if (TextoTxt)
    {
        (Antes ? Antes->Depois : TextoTxt->Posic) = Depois;
        if (Depois)
            Depois->Antes = Antes;
    }
// Coloca na lista
    if (obj)
    {
        Antes = 0;
        Depois = obj->Posic;
        if (Depois)
            Depois->Antes = this;
        obj->Posic = this;
        //Bloco = TextoTxt->Inicio;
        //PosicBloco = 0;
        //PosicTxt = 0;
        //LinhaTxt = 0;
    }
// Atualiza ponteiro
    TextoTxt = obj;
    Bloco = 0;
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
            return total;
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
void TBlocoPos::MoverPos(int numlinhas)
{
    if (Bloco==0)
        return;
// Avançar
    if (numlinhas>0)
    {
    // Avança até o início de algum bloco
        if (PosicBloco>0)
        {
            if (PosicBloco<Bloco->Bytes)
            {
                int x = PosicBloco;
                int lin = numlinhas;
                while (x<Bloco->Bytes)
                    if (Bloco->Texto[x++] == Instr::ex_barra_n)
                        if (--numlinhas==0)
                            break;
                LinhaTxt += lin - numlinhas;
                PosicTxt += x - PosicBloco;
                PosicBloco = x;
            }
            if (Bloco->Depois==0 || numlinhas==0)
                return;
            Bloco = Bloco->Depois;
        }
    // Avança blocos inteiros
        while (numlinhas > Bloco->Linhas)
        {
            numlinhas -= Bloco->Linhas;
            LinhaTxt += Bloco->Linhas;
            PosicTxt += Bloco->Bytes;
            if (Bloco->Depois)
                Bloco = Bloco->Depois;
            else
            {
                PosicBloco = Bloco->Bytes;
                return;
            }
        }
    // Avança em um bloco
        int x = 0;
        int lin = numlinhas;
        while (x<Bloco->Bytes)
            if (Bloco->Texto[x++] == Instr::ex_barra_n)
                if (--numlinhas==0)
                    break;
        LinhaTxt += lin - numlinhas;
        PosicTxt += x;
        PosicBloco = x;
        return;
    }
// Recuar
    else
    {
        numlinhas = 1-numlinhas;
    // Recua até o fim de algum bloco
        if (PosicBloco<Bloco->Bytes)
        {
            if (PosicBloco>0)
            {
                int x = PosicBloco;
                int lin = numlinhas;
                while (x)
                    if (Bloco->Texto[--x] == Instr::ex_barra_n)
                        if (--numlinhas==0)
                        {
                            x++, numlinhas++;
                            LinhaTxt -= lin - numlinhas;
                            PosicTxt -= PosicBloco - x;
                            PosicBloco = x;
                            return;
                        }
                LinhaTxt -= lin - numlinhas;
                PosicTxt -= PosicBloco;
                PosicBloco = 0;
            }
            if (Bloco->Antes==0)
                return;
            Bloco = Bloco->Antes;
        }
    // Recua blocos inteiros
        while (numlinhas > Bloco->Linhas)
        {
            numlinhas -= Bloco->Linhas;
            LinhaTxt -= Bloco->Linhas;
            PosicTxt -= Bloco->Bytes;
            if (Bloco->Antes)
                Bloco = Bloco->Antes;
            else
            {
                PosicBloco = 0;
                return;
            }
        }
    // Recua em um bloco
        int x = Bloco->Bytes;
        int lin = numlinhas;
        while (x)
            if (Bloco->Texto[--x] == Instr::ex_barra_n)
                if (--numlinhas==0)
                {
                    x++, numlinhas++;
                    LinhaTxt -= lin - numlinhas;
                    PosicTxt -= Bloco->Bytes - x;
                    PosicBloco = x;
                    return;
                }
        LinhaTxt -= lin - numlinhas;
        PosicTxt -= Bloco->Bytes;
        PosicBloco = 0;
        return;
    }
}

//----------------------------------------------------------------------------
void TBlocoPos::Mudar(const char * texto, unsigned int tamtexto,
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
                                // = quantos bytes cabem no bloco
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
                copiar = tamtexto, tamtexto = 0; // copiar todo o texto
            else
                tamtexto -= copiar; // copiar parte do texto
            int lin = AnotaLinhas(obj->Texto + obj->Bytes, texto, copiar);
            obj->Bytes += copiar;
            obj->Linhas += lin;
            dif_linhas += lin;
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
// Caso contrário, objeto Bloco não foi apagado porque não estava vazio
    if (PosicBloco==0)
        Bloco=TextoTxt->Inicio, PosicTxt=0, LinhaTxt=0;

// Obtém bloco/posição após o texto adicionado
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
    // Nota: para pos->PosicTxt == PosicTxt+add_bytes, mais=0
        unsigned int mais = pos->PosicTxt - PosicTxt - add_bytes;
        if (mais > bloco_tam)
            continue;

    // Acerta bloco e posição no bloco
        pos->Bloco = obj;
        mais += posic;
        while (mais > pos->Bloco->Bytes)
            mais -= pos->Bloco->Bytes, pos->Bloco = pos->Bloco->Depois;
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
        obj=obj->Depois, sobrando += bloco_tam - obj->Bytes;
        if (obj->Depois)
            obj=obj->Depois, sobrando += bloco_tam - obj->Bytes;
    }

// Verifica se consegue apagar algum objeto
    if (sobrando < (int)bloco_tam)
        return;

// Compacta o texto, apagando algum objeto
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
    // Passa para o próximo bloco
        ini = proximo;
    }
}

//----------------------------------------------------------------------------
TTextoBloco * TTextoGrupo::Criar()
{
#ifdef DEBUG_MSG
    printf("TTextoBloco::Criar\n"); fflush(stdout);
#endif
// Se tem objeto TListaX disponível...
    if (Usado && Usado->Total < TOTAL_TXTGR)
        return (TTextoBloco*)Usado->Lista[Usado->Total++];
// Se não tem objeto disponível...
    TTextoGrupo * obj;
    if (Disp==0)    // Não tem objeto TTextoGrupo disponível
    {
        obj=new TTextoGrupo;
#ifdef DEBUG_MSG
        printf("  TTextoGrupo::Criar(%p)\n", obj); fflush(stdout);
#endif
    }
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
#ifdef DEBUG_MSG
    printf("TTextoBloco::Apagar\n"); fflush(stdout);
#endif
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
#ifdef DEBUG_MSG
        printf("  TTextoGrupo::Apagar(%p)\n", obj); fflush(stdout);
#endif
        delete obj;
        Tempo = TempoIni;
    }
}

//----------------------------------------------------------------------------
bool TTextoTxt::Func(TVariavel * v, const char * nome)
{
// Primeira linha do texto
    if (comparaZ(nome, "ini")==0)
    {
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(TextoItem1))
            return false;
        TTextoPos * obj = Instr::VarAtual->end_textopos;
        obj->MudarTxt(this);
        obj->Bloco = Inicio;
        obj->PosicBloco = 0;
        obj->PosicTxt = 0;
        obj->LinhaTxt = 0;
        return true;
    }
// Última linha do texto
    if (comparaZ(nome, "fim")==0)
    {
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(TextoItem1))
            return false;
        TTextoPos * obj = Instr::VarAtual->end_textopos;
        obj->MudarTxt(this);
        obj->Bloco = Fim;
        obj->PosicBloco = (Fim ? Fim->Bytes : 0);
        obj->PosicTxt = Bytes;
        obj->LinhaTxt = Linhas;
        return true;
    }
// Adiciona texto no início
    if (comparaZ(nome, "addini")==0)
    {
        for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
        {
            const char * texto = v1->getTxt();
            IniBloco();
            TBlocoPos pos;
            pos.Bloco = Inicio;
            pos.PosicBloco = 0;
            pos.PosicTxt = 0;
            pos.LinhaTxt = 0;
            pos.Mudar(texto, strlen(texto)+1, 0);
        }
        DebugTextoTxt(this);
        return false;
    }
// Adiciona texto no fim
    if (comparaZ(nome, "addfim")==0)
    {
        for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
        {
            const char * texto = v1->getTxt();
            AddTexto(texto, strlen(texto)+1);
        }
        DebugTextoTxt(this);
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
            return Instr::CriarVarTexto("");
    // Obtém o número de bytes
        int total = Inicio->LinhasBytes(0, linhas);
    // Cria variável e aloca memória para o texto
        if (!Instr::CriarVarTexto(0, total-1))
            return Instr::CriarVarTexto("");
    // Obtém tamanho da memória alocada
        int copiar = Instr::VarAtual->tamanho;
    // Anota o texto
        Inicio->CopiarTxt(0, Instr::VarAtual->end_char, copiar);
    // Apaga texto de ListaTxt
        TBlocoPos pos;
        pos.Bloco = Inicio;
        pos.PosicBloco = 0;
        pos.PosicTxt = 0;
        pos.LinhaTxt = 0;
        pos.Mudar(0, 0, total);
        DebugTextoTxt(this);
        return true;
    }
// Remove todos os textos
    if (comparaZ(nome, "limpar")==0)
    {
        Limpar();
        DebugTextoTxt(this);
        return false;
    }
// Quantidade de linhas
    if (comparaZ(nome, "linhas")==0)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(Linhas);
    }
// Quantidade de bytes
    if (comparaZ(nome, "bytes")==0)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(Bytes);
    }
    return false;
}

//----------------------------------------------------------------------------
int TTextoTxt::getValor()
{
    return Inicio!=0;
}

//----------------------------------------------------------------------------
int TTextoPos::Compara(TTextoPos * v)
{
    if (TextoTxt != v->TextoTxt)
        return (TextoTxt < v->TextoTxt ? -1 : 1);
    if (TextoTxt==0 || PosicTxt == v->PosicTxt)
        return 0;
    return (PosicTxt < v->PosicTxt ? -1 : 1);
}

//----------------------------------------------------------------------------
void TTextoPos::Igual(TTextoPos * v)
{
    MudarTxt(v->TextoTxt);
    Bloco = v->Bloco;
    PosicBloco = v->PosicBloco;
    PosicTxt = v->PosicTxt;
    LinhaTxt = v->LinhaTxt;
}

//----------------------------------------------------------------------------
bool TTextoPos::Func(TVariavel * v, const char * nome)
{
// Avança linhas
    if (comparaZ(nome, "depois")==0)
    {
        int total = 1;
        if (Instr::VarAtual >= v+1)
            total = v[1].getInt();
        if (total>0)
            MoverPos(total);
        DebugTextoTxt(TextoTxt);
        return false;
    }
// Recua linhas
    if (comparaZ(nome, "antes")==0)
    {
        int total = 1;
        if (Instr::VarAtual >= v+1)
            total = v[1].getInt();
        if (total>0)
            MoverPos(-total);
        DebugTextoTxt(TextoTxt);
        return false;
    }
// Informa se linha é válida
    if (comparaZ(nome, "lin")==0)
    {
        Instr::ApagarVar(v);
        if (!Instr::CriarVarInt(0))
            return false;
        if (TextoTxt && PosicTxt < TextoTxt->Bytes)
            Instr::VarAtual->setInt(1);
        return true;
    }
// Número da linha
    if (comparaZ(nome, "linha")==0)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(LinhaTxt);
    }
// Quantidade de bytes
    if (comparaZ(nome, "byte")==0)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(PosicTxt);
    }
// Texto da linha atual
    if (comparaZ(nome, "texto")==0)
    {
        int linhas = 1;
        if (Instr::VarAtual >= v+1)
            linhas = v[1].getInt();
        Instr::ApagarVar(v); // Nota: se apagar o TextoTxt, Inicio será 0
        if (linhas<=0 || Bloco==0)
            return Instr::CriarVarTexto("");
    // Obtém o número de bytes
        int total = Bloco->LinhasBytes(PosicBloco, linhas);
    // Cria variável e aloca memória para o texto
        if (!Instr::CriarVarTexto(0, total-1))
            return Instr::CriarVarTexto("");
    // Obtém tamanho da memória alocada
        int copiar = Instr::VarAtual->tamanho;
    // Anota o texto
        Bloco->CopiarTxt(PosicBloco, Instr::VarAtual->end_char, copiar);
        return true;
    }
// Muda o texto da linha atual
    if (comparaZ(nome, "mudar")==0)
    {
        if (TextoTxt==0 || Instr::VarAtual < v+1)
            return false;
        TextoTxt->IniBloco();
    // Obtém variáveis
        const char * txt = v[1].getTxt();
        int tamtxt = strlen(txt) + 1;
        int apagar = Bloco->LinhasBytes(PosicBloco, 1);
        if (apagar>0)
            apagar--, tamtxt--;
    // Altera o texto
        Mudar(txt, tamtxt, apagar);
        DebugTextoTxt(TextoTxt);
        return false;
    }
// Adiciona texto
    if (comparaZ(nome, "add")==0)
    {
        if (TextoTxt==0 || Instr::VarAtual < v+1)
            return false;
    // Se só um argumento: adiciona texto puro
        if (Instr::VarAtual == v+1)
        {
            const char * txt = v[1].getTxt();
            TextoTxt->IniBloco();
            Mudar(txt, strlen(txt)+1, 0);
            DebugTextoTxt(TextoTxt);
            return false;
        }
    // Obtém número de linhas
        int linhas = v[2].getInt();
        if (linhas<0)
            return false;
    // Obtém variável textopos
        if (v[1].defvar[2] != Instr::cTextoPos)
            return false;
        TTextoPos * pos = v[1].end_textopos + v[1].indice;
        if (pos->TextoTxt==0 || pos->Bloco==0)
            return false;
    // Adiciona o texto
    // Evita alocação temporária de memória com "new" se o texto for pequeno
        int total = pos->Bloco->LinhasBytes(pos->PosicBloco, linhas);
        if (total<=0)
            return false;
        TextoTxt->IniBloco();
        if (total <= 8192)
        {
            char txt[8192];
            pos->Bloco->CopiarTxt(pos->PosicBloco, txt, total);
            Mudar(txt, total, 0);
        }
        else
        {
            char * txt = new char[total];
            pos->Bloco->CopiarTxt(pos->PosicBloco, txt, total);
            Mudar(txt, total, 0);
            delete[] txt;
        }
        DebugTextoTxt(TextoTxt);
        return false;
    }
// Remove a linha atual
    if (comparaZ(nome, "remove")==0)
    {
        int linhas = 1;
        if (Instr::VarAtual >= v+1)
            linhas = v[1].getInt();
        if (linhas<=0 || Bloco==0)
            return false;
        int apagar = Bloco->LinhasBytes(PosicBloco, linhas);
        Mudar(0, 0, apagar);
        DebugTextoTxt(TextoTxt);
        return false;
    }
    return false;
}

//----------------------------------------------------------------------------
int TTextoPos::getValor()
{
    return (TextoTxt && PosicTxt < TextoTxt->Bytes);
}
