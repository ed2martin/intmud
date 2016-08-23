/* Este arquivo é software livre; você pode redistribuir e/ou
 * modificar nos termos das licenças GPL ou LGPL. Vide arquivos
 * COPYING e COPYING2.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GPL or the LGP licenses.
 * See files COPYING e COPYING2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "var-texto.h"
#include "variavel.h"
#include "procurar.h"
#include "instr.h"
#include "misc.h"
#include "sha1.h"
#include "md5.h"

//#define DEBUG_TXT // Texto guardado em TTextoTxt

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
static TTextoBloco * ProcBloco = 0; // Bloco, =0 se atingiu o fim do texto
static int ProcPosic = 0;  // Posição em textotxt do byte 0 do bloco
static int ProcLinhas = 0; // Quantidade de linhas, <=0 se não está contando

/// Lê próximo bloco de texto; usado em textotxt.txtproc
/** @param buf Buffer aonde colocar o texto
 *  @param tambuf Tamanho do buffer, deve ser maior que zero
 *  @return Quantidade de bytes preenchidos, ou -1 se atingiu o fim do texto
 */
static int ProcLer(char * buf, int tambuf)
{
// Primeiro byte
    if (ProcPosic<0)
    {
        *buf = Instr::ex_barra_n;
        ProcPosic=0;
        if (ProcLinhas <= 0)
            return 1;
        ProcLinhas--;
        if (ProcLinhas==0)
            ProcBloco=0;
        return 1;
    }
// Fim do texto
    if (ProcBloco==0)
        return -1;
// Buffer menor que bloco: copia parte
    int tam = ProcBloco->Bytes - ProcPosic;
    if (tambuf < tam)
    {
        tam = tambuf;
        memcpy(buf, ProcBloco->Texto+ProcPosic, tam);
        ProcPosic += tam;
    }
// Copia do meio até o fim do bloco
    else if (ProcPosic)
    {
        memcpy(buf, ProcBloco->Texto+ProcPosic, tam);
        ProcPosic = 0;
        ProcBloco = ProcBloco->Depois;
    }
// Copia todo o bloco
    else
    {
        memcpy(buf, ProcBloco->Texto, tam);
        if (ProcLinhas > 0 && ProcLinhas > ProcBloco->Linhas)
        {
            ProcLinhas -= ProcBloco->Linhas;
            ProcBloco = ProcBloco->Depois;
            return tam;
        }
        ProcBloco = ProcBloco->Depois;
    }
// Verifica se está contando linhas
    if (ProcLinhas <= 0)
        return tam;
// Conta linhas
    for (int x=0; x<tam; x++)
        if (buf[x]==Instr::ex_barra_n)
            if (--ProcLinhas==0)
            {
                ProcBloco=0;
                return x+1;
            }
    return tam;
}

//----------------------------------------------------------------------------
bool TTextoTxt::Func(TVariavel * v, const char * nome)
{
// Lista das funções de textotxt
// Deve obrigatoriamente estar em letras minúsculas e ordem alfabética
    static const struct {
        const char * Nome;
        bool (TTextoTxt::*Func)(TVariavel * v); } ExecFunc[] = {
        { "addfim",       &TTextoTxt::FuncAddFim },
        { "addini",       &TTextoTxt::FuncAddIni },
        { "bytes",        &TTextoTxt::FuncBytes },
        { "clipler",      &TTextoTxt::FuncClipLer },
        { "clipsalvar",   &TTextoTxt::FuncClipSalvar },
        { "dividelin",    &TTextoTxt::FuncDivideLin },
        { "dividelincor", &TTextoTxt::FuncDivideLinCor },
        { "fim",          &TTextoTxt::FuncFim },
        { "ini",          &TTextoTxt::FuncIni },
        { "juntalin",     &TTextoTxt::FuncJuntaLin },
        { "juntar",       &TTextoTxt::FuncJuntar },
        { "ler",          &TTextoTxt::FuncLer },
        { "limpar",       &TTextoTxt::FuncLimpar },
        { "linhas",       &TTextoTxt::FuncLinhas },
        { "ordena",       &TTextoTxt::FuncOrdena },
        { "ordenalin",    &TTextoTxt::FuncOrdenaLin },
        { "rand",         &TTextoTxt::FuncRand },
        { "remove",       &TTextoTxt::FuncRemove },
        { "salvar",       &TTextoTxt::FuncSalvar },
        { "txtremove",    &TTextoTxt::FuncTxtRemove }  };
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

//----------------------------------------------------------------------------
// Primeira linha do texto
bool TTextoTxt::FuncIni(TVariavel * v)
{
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarTextoPos))
        return false;
    TTextoPos * obj = Instr::VarAtual->end_textopos;
    obj->MudarTxt(this);
    obj->Bloco = Inicio;
    obj->PosicBloco = 0;
    obj->PosicTxt = 0;
    obj->LinhaTxt = 0;
    return true;
}

//----------------------------------------------------------------------------
// Última linha do texto
bool TTextoTxt::FuncFim(TVariavel * v)
{
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarTextoPos))
        return false;
    TTextoPos * obj = Instr::VarAtual->end_textopos;
    obj->MudarTxt(this);
    obj->Bloco = Fim;
    obj->PosicBloco = (Fim ? Fim->Bytes : 0);
    obj->PosicTxt = Bytes;
    obj->LinhaTxt = Linhas;
    return true;
}

//----------------------------------------------------------------------------
// Adiciona texto no início
bool TTextoTxt::FuncAddIni(TVariavel * v)
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

//----------------------------------------------------------------------------
// Adiciona texto no fim
bool TTextoTxt::FuncAddFim(TVariavel * v)
{
    for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
    {
        const char * texto = v1->getTxt();
        AddTexto(texto, strlen(texto)+1);
    }
    DebugTextoTxt(this);
    return false;
}

//----------------------------------------------------------------------------
// Remove uma quantidade de linhas
bool TTextoTxt::FuncRemove(TVariavel * v)
{
    int linhas = 1;
    if (Instr::VarAtual >= v+1)
        linhas = v[1].getInt();
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

//----------------------------------------------------------------------------
// Remove todos os textos
bool TTextoTxt::FuncLimpar(TVariavel * v)
{
    Limpar();
    DebugTextoTxt(this);
    return false;
}

//----------------------------------------------------------------------------
// Quantidade de linhas
bool TTextoTxt::FuncLinhas(TVariavel * v)
{
    Instr::ApagarVar(v);
    return Instr::CriarVarInt(Linhas);
}

//----------------------------------------------------------------------------
// Quantidade de bytes
bool TTextoTxt::FuncBytes(TVariavel * v)
{
    Instr::ApagarVar(v);
    return Instr::CriarVarInt(Bytes);
}

//----------------------------------------------------------------------------
// Ordenar
bool TTextoTxt::FuncOrdena(TVariavel * v)
{
    Ordena(0, 0, 0);
    DebugTextoTxt(this);
    return false;
}

//----------------------------------------------------------------------------
// Ordenar juntando linhas
bool TTextoTxt::FuncOrdenaLin(TVariavel * v)
{
// Com menos de dois argumentos
    if (Instr::VarAtual < v+2)
    {
        Ordena(1, 0, 0);
        DebugTextoTxt(this);
        return false;
    }
// Com dois ou mais argumentos
    char txt1[256],txt2[256];
    copiastr(txt1, v[1].getTxt(), sizeof(txt1));
    copiastr(txt2, v[2].getTxt(), sizeof(txt2));
    Ordena(2, txt1, txt2);
    DebugTextoTxt(this);
    return false;
}

//----------------------------------------------------------------------------
// Ordenar aleatoriamente
bool TTextoTxt::FuncRand(TVariavel * v)
{
    Rand();
    return false;
}

//----------------------------------------------------------------------------
// Acertar texto
bool TTextoTxt::FuncTxtRemove(TVariavel * v)
{
    if (Instr::VarAtual < v+1)
        return false;
    int remove = txtRemove(v[1].getTxt()); // O que deve remover
    if (remove==0)
        return false;
    TxtRemove(remove);
    DebugTextoTxt(this);
    return false;
}

//----------------------------------------------------------------------------
// Apenas juntar linhas
bool TTextoTxt::FuncJuntaLin(TVariavel * v)
{
// Com menos de dois argumentos
    if (Instr::VarAtual < v+2)
    {
        Ordena(3, 0, 0);
        DebugTextoTxt(this);
        return false;
    }
// Com dois ou mais argumentos
    char txt1[256],txt2[256];
    copiastr(txt1, v[1].getTxt(), sizeof(txt1));
    copiastr(txt2, v[2].getTxt(), sizeof(txt2));
    Ordena(4, txt1, txt2);
    DebugTextoTxt(this);
    return false;
}

//----------------------------------------------------------------------------
// Divide linhas
bool TTextoTxt::FuncDivideLin(TVariavel * v)
{
    if (Instr::VarAtual < v+2)
        return false;
    int min = v[1].getInt();
    int max = v[2].getInt();
    if (min>max) min=max;
    if (min>=2)  DivideLin(min, max, false);
    DebugTextoTxt(this);
    return false;
}

//----------------------------------------------------------------------------
// Divide linhas desconsiderando definição de cores
bool TTextoTxt::FuncDivideLinCor(TVariavel * v)
{
    if (Instr::VarAtual < v+2)
        return false;
    int min = v[1].getInt();
    int max = v[2].getInt();
    if (min>max) min=max;
    if (min>=2)  DivideLin(min, max, true);
    DebugTextoTxt(this);
    return false;
}

//----------------------------------------------------------------------------
// Junta linhas
bool TTextoTxt::FuncJuntar(TVariavel * v)
{
// Junta linhas eliminando "\n"
    for (TTextoBloco * bl = Inicio; bl; bl=bl->Depois)
        if (bl->Linhas)
        {
            bl->Linhas = 0;
            for (int x=0; x<bl->Bytes; x++)
                if (bl->Texto[x] == Instr::ex_barra_n)
                    bl->Texto[x] = ' ';
        }
// Coloca "\n" no fim
    Linhas = 0;
    if (Fim)
    {
        Fim->Texto[Fim->Bytes-1] = Instr::ex_barra_n;
        Fim->Linhas = 1;
        Linhas = 1;
    }
// Acerta TextoPos
    for (TTextoPos * pos = Posic; pos; pos=pos->Depois)
    {
        pos->Bloco = Inicio;
        pos->PosicBloco = 0;
        pos->PosicTxt = 0;
        pos->LinhaTxt = 0;
    }
    DebugTextoTxt(this);
    return false;
}

//----------------------------------------------------------------------------
// Lê arquivo de texto
bool TTextoTxt::FuncLer(TVariavel * v)
{
    if (Instr::VarAtual < v+1)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(0);
    }
    char nome[1024]; // Nome do arquivo / buffer de leitura
    copiastr(nome, v[1].getTxt(), sizeof(nome)-4);
    Instr::ApagarVar(v);
    if (!arqvalido(nome, true))
        return Instr::CriarVarInt(0);
// Abre arquivo
    FILE * descr = fopen(nome, "rb");
    if (descr==0)
        return Instr::CriarVarInt(0);
    TextoIni();
// Lê arquivo
    char chfim = 0;  // Último caracter anotado em textotxt
    char linhaCRLF = 0; // Para detectar fim da linha (CR e LF)
    while (true)
    {
        char buf[8192]; // sizeof(buf) = sizeof(nome)*8
        char * d = buf;
        int lido = fread(nome, 1, sizeof(nome), descr);
        if (lido<=0)
            break;
        for (const char * o=nome; o<nome+lido; o++)
        {
            unsigned char ch = *o;
            //putchar(ch);
            if (ch >= ' ')  // Caracteres visíveis
                *d++=ch, linhaCRLF=0;
            else if (ch==9) // Tabulação
            {
                memset(d, ' ', 8);
                d+=8, linhaCRLF=0;
            }
            else if (ch!=13 && ch!=10) // Se não é nova linha
                linhaCRLF=0;
            else if (linhaCRLF==0 || linhaCRLF==ch) // Nova linha
                linhaCRLF=ch, *d++=Instr::ex_barra_n;
        }
        if (d!=buf)
        {
            TextoAnota(buf, d-buf);
            chfim = d[-1];
        }
        if (lido!=sizeof(nome))
            break;
    }
// Garante Instr::ex_barra_n no fim do texto de textotxt
    if (chfim != Instr::ex_barra_n)
    {
        const char barra_n = Instr::ex_barra_n;
        TextoAnota(&barra_n, 1);
    }
// Fecha arquivo
    TextoFim();
    fclose(descr);
    DebugTextoTxt(this);
    return Instr::CriarVarInt(1);
}

//----------------------------------------------------------------------------
// Salva em arquivo de texto
bool TTextoTxt::FuncSalvar(TVariavel * v)
{
    if (Instr::VarAtual < v+1)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(0);
    }
    char nome[1024]; // Nome do arquivo
    copiastr(nome, v[1].getTxt(), sizeof(nome)-4);
    Instr::ApagarVar(v);
    if (!arqvalido(nome, false))
        return Instr::CriarVarInt(0);
// Abre arquivo
    FILE * descr = fopen(nome, "w");
    if (descr==0)
        return Instr::CriarVarInt(0);
// Salva em arquivo
    char buf[4096];
    char * pbuf = buf;
    int pular = 0; // Para não salvar cores em arquivo
    for (TTextoBloco * bl = Inicio; bl; bl=bl->Depois)
    {
        char * p = bl->Texto;
        for (int x=bl->Bytes; x; x--,p++)
        {
            unsigned char ch = *p;
            switch (ch)
            {
            case Instr::ex_barra_n: pular=0, *pbuf++ = '\n'; break;
            case Instr::ex_barra_c: pular=1; break;
            case Instr::ex_barra_d: pular=2; break;
            default:
                switch (pular)
                {
                case 1:
                    if ((ch<'0' || ch>'9') && (ch<'A' || ch>'J') &&
                            (ch<'a' || ch>'j') && ch>=' ')
                        *pbuf++ = ch;
                    pular=0;
                    break;
                case 2:
                    if ((ch<'0' || ch>'7')  && ch>=' ')
                        *pbuf++ = ch;
                    pular=0;
                    break;
                default:
                    if (ch >= ' ')
                        *pbuf++ = ch;
                }
            }
        }
        if (pbuf < buf + sizeof(buf) - 0x100)
            continue;
        fwrite(buf, 1, pbuf-buf, descr);
        pbuf = buf;
    }
    if (pbuf != buf)
        fwrite(buf, 1, pbuf-buf, descr);
// Fecha arquivo
    fclose(descr);
    DebugTextoTxt(this);
    return Instr::CriarVarInt(1);
}

//----------------------------------------------------------------------------
bool TTextoTxt::FuncClipLer(TVariavel * v)
{
    char * p = ClipboardLer();
    Instr::ApagarVar(v);
    if (p==0)
        return Instr::CriarVarInt(0);
    char * d = p;
    for (char * o = p; *o; o++)
    {
        if (*o == 0x0D)
        {
            *d++ = Instr::ex_barra_n;
            if (o[1]==0x0A)
                o++;
        }
        else if (*o == 0x0A)
        {
            *d++ = Instr::ex_barra_n;
            if (o[1]==0x0D)
                o++;
        }
        else if (*(unsigned char*)o >= 0x20)
            *d++ = *o;
    }
    *d++ = Instr::ex_barra_n;
    TextoIni();
    TextoAnota(p, d-p);
    TextoFim();
    delete[] p;
    return Instr::CriarVarInt(1);
}

//----------------------------------------------------------------------------
bool TTextoTxt::FuncClipSalvar(TVariavel * v)
{
    char * buf = new char[Bytes + Linhas + 2];
    char * fim = buf + Linhas + 1;

// Copia texto para buf
    for (TTextoBloco * bl = Inicio; bl; bl=bl->Depois)
    {
        memcpy(fim, bl->Texto, bl->Bytes);
        fim += bl->Bytes;
    }
    *fim++ = 0;
    assert(fim <= buf + Bytes + Linhas + 2);

// Retira as definições de cores
    char * txt = buf + Linhas + 1;
    char * destino = buf;
    while (*txt)
        switch (*txt)
        {
        case Instr::ex_barra_b:
            txt++;
            break;
        case Instr::ex_barra_c:
            if ((txt[1]>='0' && txt[1]<='9') ||
                    (txt[1]>='A' && txt[1]<='J') ||
                    (txt[1]>='a' && txt[1]<='j'))
                txt += 2;
            else
                txt++;
            break;
        case Instr::ex_barra_d:
            if (txt[1]>='0' && txt[1]<='7')
                txt += 2;
            else
                txt++;
            break;
        case Instr::ex_barra_n:
            txt++;
            *destino++ = 0x0D;
            *destino++ = 0x0A;
            break;
        default:
            *destino++ = *txt++;
        }
    if (destino > buf)
        if (destino[-1] == 0x0A)
            destino -= 2;
    *destino = 0;

// Envia para a área de transferência
    Instr::ApagarVar(v);
    bool b = ClipboardMudar(buf);
    delete[] buf;
    return Instr::CriarVarInt(b);
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
// Lista das funções de textopos
// Deve obrigatoriamente estar em letras minúsculas e ordem alfabética
    static const struct {
        const char * Nome;
        int valor;
        bool (TTextoPos::*Func)(TVariavel * v, int valor); } ExecFunc[] = {
        { "add",          0, &TTextoPos::FuncAdd },
        { "addpos",       1, &TTextoPos::FuncAdd },
        { "antes",        0, &TTextoPos::FuncAntes },
        { "byte",         0, &TTextoPos::FuncByte },
        { "depois",       0, &TTextoPos::FuncDepois },
        { "juntar",       0, &TTextoPos::FuncJuntar },
        { "lin",          0, &TTextoPos::FuncLin },
        { "linha",        0, &TTextoPos::FuncLinha },
        { "mudar",        0, &TTextoPos::FuncMudar },
        { "remove",       0, &TTextoPos::FuncRemove },
        { "texto",        0, &TTextoPos::FuncTexto },
        { "textolin",     0, &TTextoPos::FuncTextoLin },
        { "txtmd5",       0, &TTextoPos::FuncMd5Sha1 },
        { "txtproc",      0, &TTextoPos::FuncTxtProc },
        { "txtprocdif",   2, &TTextoPos::FuncTxtProc },
        { "txtprocmai",   1, &TTextoPos::FuncTxtProc },
        { "txtsha1",      1, &TTextoPos::FuncMd5Sha1 } };
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
            return (this->*ExecFunc[meio].Func)(v, ExecFunc[meio].valor);
        if (resultado<0) fim=meio-1; else ini=meio+1;
    }
    return false;
}

//----------------------------------------------------------------------------
// Recua linhas
bool TTextoPos::FuncAntes(TVariavel * v, int valor)
{
    int total = 1;
    if (Instr::VarAtual >= v+1)
        total = v[1].getInt();
    if (total>0)
        MoverPos(-total);
    DebugTextoTxt(TextoTxt);
    return false;
}

//----------------------------------------------------------------------------
// Avança linhas
bool TTextoPos::FuncDepois(TVariavel * v, int valor)
{
    int total = 1;
    if (Instr::VarAtual >= v+1)
        total = v[1].getInt();
    if (total>0)
        MoverPos(total);
    DebugTextoTxt(TextoTxt);
    return false;
}

//----------------------------------------------------------------------------
// Informa se linha é válida
bool TTextoPos::FuncLin(TVariavel * v, int valor)
{
    bool valido = (TextoTxt && PosicTxt < TextoTxt->Bytes);
    Instr::ApagarVar(v);
    if (!Instr::CriarVarInt(0))
        return false;
    if (valido)
        Instr::VarAtual->setInt(1);
    return true;
}

//----------------------------------------------------------------------------
// Número da linha
bool TTextoPos::FuncLinha(TVariavel * v, int valor)
{
    Instr::ApagarVar(v+1);
    Instr::VarAtual->numfunc = 1;
    return true;
}

//----------------------------------------------------------------------------
// Quantidade de bytes
bool TTextoPos::FuncByte(TVariavel * v, int valor)
{
    valor = PosicTxt;
    Instr::ApagarVar(v);
    return Instr::CriarVarInt(valor);
}

//----------------------------------------------------------------------------
// Texto da linha atual
bool TTextoPos::FuncTexto(TVariavel * v, int valor)
{
// Obtém colunas inicial e final
    int colini = 0;
    int coltam = 0x7FFFFFF;
    if (Instr::VarAtual >= v+1)
    {
        colini = v[1].getInt();
        if (colini < 0)
            colini = 0;
        if (Instr::VarAtual >= v+2)
            coltam = v[2].getInt();
    }
    Instr::ApagarVar(v); // Nota: se apagar o TextoTxt, Bloco será 0
    if (coltam<=0 || Bloco==0)
        return Instr::CriarVarTexto("");
// Obtém o número de bytes da linha
    TTextoBloco * bl = Bloco;
    int total = bl->LinhasBytes(PosicBloco, 1) - 1;
    if (colini >= total)
        return Instr::CriarVarTexto("");
    if (coltam > total || colini + coltam > total)
            // Nota: colini+coltam pode ser <0 se ocorrer overflow
        coltam = total - colini;
// Avança coluna inicial
    colini += PosicBloco;
    while (colini > bl->Bytes)
        colini -= bl->Bytes, bl=bl->Depois;
// Cria variável e aloca memória para o texto
// Nota: a variável Bloco pode ser alterada aqui, porque
//       a nova variável pode ocupar o mesmo lugar de TTextoPos
    if (!Instr::CriarVarTexto(0, coltam))
        return Instr::CriarVarTexto("");
// Obtém tamanho da memória alocada
    int copiar = Instr::VarAtual->tamanho;
// Anota o texto
    bl->CopiarTxt(colini, Instr::VarAtual->end_char, copiar);
    return true;
}

//----------------------------------------------------------------------------
// Texto de uma ou mais linhas
bool TTextoPos::FuncTextoLin(TVariavel * v, int valor)
{
    int linhas = 1;
    if (Instr::VarAtual >= v+1)
        linhas = v[1].getInt();
    Instr::ApagarVar(v); // Nota: se apagar o TextoTxt, Bloco será 0
    if (linhas<=0 || Bloco==0)
        return Instr::CriarVarTexto("");
// Obtém o número de bytes
    int total = Bloco->LinhasBytes(PosicBloco, linhas) - 1;
// Cria variável e aloca memória para o texto
    TTextoBloco * bl = Bloco;
    unsigned int pos = PosicBloco;
    if (!Instr::CriarVarTexto(0, total))
        return Instr::CriarVarTexto("");
// Obtém tamanho da memória alocada
    int copiar = Instr::VarAtual->tamanho;
// Anota o texto
    bl->CopiarTxt(pos, Instr::VarAtual->end_char, copiar);
    return true;
}

//----------------------------------------------------------------------------
// Muda o texto da linha atual
bool TTextoPos::FuncMudar(TVariavel * v, int valor)
{
    if (TextoTxt==0 || Instr::VarAtual < v+1)
        return false;
    TextoTxt->IniBloco();
// Obtém colunas inicial e final
    int colini = 0;
    int coltam = 0x7FFFFFF;
    if (Instr::VarAtual >= v+2)
    {
        colini = v[2].getInt();
        if (colini < 0)
            colini = 0;
        if (Instr::VarAtual >= v+3)
            coltam = v[3].getInt();
        if (coltam < 0)
            coltam = 0;
    }
// Obtém variáveis
    TBlocoPos bl = *this;
    const char * txt = v[1].getTxt();
    int tamtxt = strlen(txt) + 1;
    int apagar = Bloco->LinhasBytes(PosicBloco, 1);
// Se linha não está vazia: obtém posição na linha
    if (apagar>0)
    {
        apagar--, tamtxt--;
    // Acerta colini e apagar
        if (colini >= apagar)
            colini = apagar, apagar = 0;
        else
        {
            apagar -= colini;
            if (apagar > coltam)
                apagar = coltam;
        }
    // Posição no texto
        bl.PosicTxt += colini;
        colini += bl.PosicBloco;
        while (colini > bl.Bloco->Bytes)
            colini -= bl.Bloco->Bytes, bl.Bloco=bl.Bloco->Depois;
        bl.PosicBloco = colini;
    }
// Altera o texto
    bl.Mudar(txt, tamtxt, apagar);
    DebugTextoTxt(TextoTxt);
    return false;
}

//----------------------------------------------------------------------------
// Adiciona texto
bool TTextoPos::FuncAdd(TVariavel * v, int valor)
{
    if (TextoTxt==0 || Instr::VarAtual < v+1)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(0);
    }
    int lin_antes = TextoTxt->Linhas;

    while (true)
    {
    // Se só um argumento: adiciona texto puro
        if (Instr::VarAtual == v+1)
        {
            const char * txt = v[1].getTxt();
            TextoTxt->IniBloco();
            Mudar(txt, strlen(txt)+1, 0);
            DebugTextoTxt(TextoTxt);
            break;
        }
    // Obtém número de linhas
        int linhas = v[2].getInt();
        if (linhas<=0)
            break;
    // Obtém variável textopos
        if (v[1].defvar[2] != Instr::cTextoPos)
            break;
        TTextoPos * pos = v[1].end_textopos + v[1].indice;
        if (pos->TextoTxt==0 || pos->Bloco==0)
            break;
    // Adiciona o texto
    // Evita alocação temporária de memória com "new" se o texto for pequeno
        int total = pos->Bloco->LinhasBytes(pos->PosicBloco, linhas);
        if (total<=0)
            break;
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
        break;
    }

    int lin_add = TextoTxt->Linhas - lin_antes;
    if (valor && lin_add)
        MoverPos(lin_add);
    Instr::ApagarVar(v);
    return Instr::CriarVarInt(lin_add);
}

//----------------------------------------------------------------------------
// Remove a linha atual
bool TTextoPos::FuncRemove(TVariavel * v, int valor)
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

//----------------------------------------------------------------------------
// Junta com a linha anterior
bool TTextoPos::FuncJuntar(TVariavel * v, int valor)
{
    Instr::ApagarVar(v);
    if (TextoTxt==0 || Bloco==0 || PosicTxt==0 ||
            PosicTxt>=TextoTxt->Bytes)
        return Instr::CriarVarInt(0);
// Vai para o \n no final da linha anterior
    TBlocoPos bl = *this;
    bl.PosicTxt--;
    bl.LinhaTxt--;
    if (bl.PosicBloco)
        bl.PosicBloco--;
    else
    {
        bl.Bloco = bl.Bloco->Antes;
        bl.PosicBloco = bl.Bloco->Bytes - 1;
    }
// Move posição para a linha anterior
    MoverPos(-1);
    for (TTextoPos * obj = TextoTxt->Posic; obj; obj=obj->Depois)
        if (obj->LinhaTxt == LinhaTxt + 1)
        {
            obj->Bloco = Bloco;
            obj->PosicBloco = PosicBloco;
            obj->PosicTxt = PosicTxt;
            obj->LinhaTxt = LinhaTxt;
        }
// Junta as duas linha
    bl.Mudar(0, 0, 1);
    DebugTextoTxt(TextoTxt);
    return Instr::CriarVarInt(1);
}

//----------------------------------------------------------------------------
// Procura um texto
bool TTextoPos::FuncTxtProc(TVariavel * v, int valor)
{
    if (TextoTxt==0 || Bloco==0 || Instr::VarAtual < v+1)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(-1);
    }
    TProcurar proc;
    const char * txtproc = v[1].getTxt();
    int inicio = (txtproc[0]==Instr::ex_barra_n &&
                    txtproc[1]); // Se texto começa com '\n'
    if (txtproc[0]==0)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(-1);
    }
// Obtém argumentos
    int chini = 0; // Caracter inicial na linha
    int chtam = -1; // Quantidade de linhas
    if (Instr::VarAtual >= v+2)
    {
        chini = v[2].getInt();
        if (chini < 0)
            chini = 0;
        else if (chini > 0)
        {
            int tamlinha = Bloco->LinhasBytes(PosicBloco, 1);
            if (chini > tamlinha)
                chini = tamlinha;
        }
        if (Instr::VarAtual >= v+3)
        {
            chtam = v[3].getInt();
            if (chtam <= 0)
            {
                Instr::ApagarVar(v);
                return Instr::CriarVarInt(-1);
            }
        }
    }
// Acerta chini se texto começa com "\n"
    if (inicio)
        chini--; // Procurar a partir do caracter anterior
    if (chtam > (int)(TextoTxt->Linhas - LinhaTxt))
        chtam = -1;
// Acerta bloco e posição inicial
    ProcLinhas = (chtam>0 ? chtam+inicio : -1);
    ProcBloco = Bloco;
    ProcPosic = PosicBloco + chini;
    while (ProcBloco && ProcPosic >= (int)ProcBloco->Bytes)
        ProcPosic -= ProcBloco->Bytes, ProcBloco = ProcBloco->Depois;
// Procura
    proc.Padrao(txtproc, valor); // Prepara padrão
    int ind = proc.Proc(&ProcLer); // Procura
// Não encontrou
    if (ind<0)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(-1);
    }
// Encontrou
    ind += chini; // ind = quantos bytes deve avançar
// Avança para nova posição - em blocos
    unsigned int pos = PosicBloco + ind + inicio;
                    // somado com inicio para pular o "\n"
    while (pos >= Bloco->Bytes)
    {
        if (Bloco->Depois==0)
        {
            pos = Bloco->Bytes; // Para avançar até o fim do bloco
            break;
        }
        if (PosicBloco==0)
        {
            LinhaTxt += Bloco->Linhas;
            PosicTxt += Bloco->Bytes;
        }
        else
        {
            for (int x=PosicBloco; x<Bloco->Bytes; x++)
                if (Bloco->Texto[x] == Instr::ex_barra_n)
                    LinhaTxt++;
            PosicTxt += Bloco->Bytes - PosicBloco;
            PosicBloco = 0;
        }
        pos -= Bloco->Bytes;
        Bloco = Bloco->Depois;
    }
// Avança para nova posição - no mesmo bloco
    PosicTxt += pos - PosicBloco;
    for (unsigned int x=PosicBloco; x<pos; x++)
        if (Bloco->Texto[x] == Instr::ex_barra_n)
            LinhaTxt++;
// Recua até começo da linha
    unsigned int txtfim = PosicTxt;
    while (true)
    {
        if (pos > 0) // Se não está no começo do bloco
        {
            if (Bloco->Texto[pos-1] == Instr::ex_barra_n)
                break;
            pos--, PosicTxt--;
            continue;
        }
        if (Bloco->Antes==0)
            break;
        Bloco = Bloco->Antes;
        if (Bloco->Linhas) // Bloco contém algum ex_barra_n
            pos = Bloco->Bytes;
        else        // Bloco não contém ex_barra_n
            PosicTxt -= Bloco->Bytes;
    }
    PosicBloco = pos;
    valor = txtfim - PosicTxt;
    Instr::ApagarVar(v);
    return Instr::CriarVarInt(valor);
}

//----------------------------------------------------------------------------
bool TTextoPos::FuncMd5Sha1(TVariavel * v, int valor)
{
    int linhas = 1;
    if (Instr::VarAtual >= v+1)
        linhas = v[1].getInt();
    Instr::ApagarVar(v); // Nota: se apagar o TextoTxt, Bloco será 0

// Obtém o número de bytes e bloco/posição inicial
    char mens[80];
    unsigned int posic = PosicBloco;
    TTextoBloco * obj = Bloco;
    int tambuf = 0;
    if (linhas > 0 && obj)
        tambuf = obj->LinhasBytes(posic, linhas) - 1;

// MD5
    if (valor == 0)
    {
        unsigned char digest[16];
        cvs_MD5Context md5Info;
        cvs_MD5Init(&md5Info);
        if (obj)
            if (posic>=obj->Bytes)
                obj=obj->Depois, posic=0;
        while (obj)
        {
            if (tambuf <= (int)obj->Bytes - (int)posic)
                break;
            int total = obj->Bytes - posic;
            cvs_MD5Update(&md5Info, (unsigned char *)obj->Texto+posic, total);
            tambuf -= total;
            obj=obj->Depois, posic=0;
        }
        if (tambuf > 0)
            cvs_MD5Update(&md5Info, (unsigned char *)obj->Texto+posic, tambuf);
        cvs_MD5Final(digest, &md5Info);
        for (int x=0; x<16; x++)
        {
            int valor = digest[x] >> 4;
            mens[x*2] = (valor<10 ? valor+'0' : valor+'a'-10);
            valor = digest[x] & 0x0F;
            mens[x*2+1] = (valor<10 ? valor+'0' : valor+'a'-10);
        }
        return Instr::CriarVarTexto(mens, 32);
    }
// SHA1
    else
    {
        unsigned char digest[20];
        SHA_CTX shaInfo;
        SHAInit(&shaInfo);
        if (obj)
            if (posic>=obj->Bytes)
                obj=obj->Depois, posic=0;
        while (obj)
        {
            if (tambuf <= (int)obj->Bytes - (int)posic)
                break;
            int total = obj->Bytes - posic;
            SHAUpdate(&shaInfo, (unsigned char *)obj->Texto+posic, total);
            tambuf -= total;
            obj=obj->Depois, posic=0;
        }
        if (tambuf > 0)
            SHAUpdate(&shaInfo, (unsigned char *)obj->Texto+posic, tambuf);
        SHAFinal(digest, &shaInfo);
        for (int x=0; x<20; x++)
        {
            int valor = digest[x] >> 4;
            mens[x*2] = (valor<10 ? valor+'0' : valor+'a'-10);
            valor = digest[x] & 0x0F;
            mens[x*2+1] = (valor<10 ? valor+'0' : valor+'a'-10);
        }
        return Instr::CriarVarTexto(mens, 40);
    }
}

//----------------------------------------------------------------------------
int TTextoPos::getTipo(int numfunc)
{
    return (numfunc ? varInt : varOutros);
}

//----------------------------------------------------------------------------
int TTextoPos::getValor(int numfunc)
{
    if (TextoTxt==0)
        return 0;
    else if (numfunc==0)
        return (PosicTxt < TextoTxt->Bytes);
    else
        return LinhaTxt;
}

//----------------------------------------------------------------------------
void TTextoPos::setValor(int numfunc, int valor)
{
    if (TextoTxt==0 || numfunc==0 || valor==(int)LinhaTxt)
        return;
    if (valor <= 0)
    {
        Bloco = TextoTxt->Inicio;
        PosicBloco = 0;
        PosicTxt = 0;
        LinhaTxt = 0;
    }
    else if (valor >= (int)TextoTxt->Linhas)
    {
        Bloco = TextoTxt->Fim;
        PosicBloco = (TextoTxt->Fim ? TextoTxt->Fim->Bytes : 0);
        PosicTxt = TextoTxt->Bytes;
        LinhaTxt = TextoTxt->Linhas;
    }
    else
        MoverPos(valor - LinhaTxt);
}

//----------------------------------------------------------------------------
void TTextoPos::setTxt(int numfunc, const char * txt)
{
    setValor(numfunc, TxtToInt(txt));
}
