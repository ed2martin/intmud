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
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <assert.h>
#include "var-sav.h"
#include "var-listaobj.h"
#include "var-texto.h"
#include "var-textovar.h"
#include "var-datahora.h"
#include "variavel.h"
#include "classe.h"
#include "objeto.h"
#include "arqler.h"
#include "instr.h"
#include "misc.h"
#include "random.h"
#include "sha1.h"

//#define DEBUG // Mostrar diretórios e arquivos acessados pela função LIMPAR

#define MAX_OBJ 1024 // Número de objetos por arquivo

bool TVarSav::InicVar = false;
int TVarSav::HoraReg = 0;
int TVarSav::Dia = 0;
int TVarSav::Hora = 0;
int TVarSav::Min = 0;
int TVarSav::TempoSav=0;

//----------------------------------------------------------------------------
// Acerta variáveis
void TVarSav::ProcEventos(int tempoespera)
{
// Obtém a hora
    time_t tempoatual=0;
    struct tm * tempolocal;
    time(&tempoatual);
    // localtime() Converte para representaçãao local de tempo
    tempolocal = localtime(&tempoatual);
    Dia = tempolocal->tm_yday+1;
    Hora = tempolocal->tm_hour;
    Min = tempolocal->tm_min;
// Data e hora para o registro
    HoraReg = tempolocal->tm_year + 300; // Anos, desde 1900
                            // Somando 300 para ser múltiplo de 400
    HoraReg--;
    HoraReg = HoraReg*365 + HoraReg/4 - HoraReg/100 + HoraReg/400;
    HoraReg += tempolocal->tm_yday; // Dias completos desde o início do ano
    HoraReg = HoraReg*1440 + Hora*60 + Min;
    //printf("%d\n", (int)HoraReg); fflush(stdout);
// Checa arquivos SAV pendentes
    TempoSav += tempoespera;
    if (TempoSav<0)
        TempoSav=0;
    if (TempoSav>=10)
    {
        TempoSav=0;
        TVarSavDir::Checa();
    }
}

//----------------------------------------------------------------------------
void TVarSav::Senha(char * senhacodif, const char * senha, char fator)
{
// Checa se fator nulo
    *senhacodif++ = fator;
    if (fator==0)
    {
        *senhacodif = 0;
        return;
    }
// Prepara a senha
    unsigned char mens[512];
    unsigned int tam = strlen(senha);
    if (tam > sizeof(mens)-1)
        tam = sizeof(mens)-1;
    mens[0] = fator;
    memcpy(mens+1, senha, tam);
// Codifica
    unsigned char digest[20];
    SHA_CTX shaInfo;
    SHAInit(&shaInfo);
    SHAUpdate(&shaInfo, mens, tam);
    SHAFinal(digest, &shaInfo);
// Anota na string
    for (int x=0; x<20; x+=4)
    {
        unsigned int valor = digest[x]   * 0x1000000+
                             digest[x+1] * 0x10000+
                             digest[x+2] * 0x100+
                             digest[x+3];
        for (int b=0; b<5; valor/=90,b++)
            *senhacodif++ = (char)(valor%90+33);
    }
    *senhacodif = 0;
}

//----------------------------------------------------------------------------
int TVarSav::Tempo(const char * arqnome)
{
    char mens[300];
    TArqLer arqler;
    if (!arqler.Abrir(arqnome))
        return -1;
    while (arqler.Linha(mens, sizeof(mens), false)>0)
    {
        if (strcmp(mens, "+++")==0)
            break;
        if (compara(mens, "data=",5)!=0)
            continue;
        int tempo = atoi(mens+5) - HoraReg;
        return (tempo<0 ? 0 : tempo);
    }
    return -1;
}

//----------------------------------------------------------------------------
bool TVarSav::Func(TVariavel * v, const char * nome)
{
// Inicializa variáveis se necessário
    if (!InicVar)
    {
        ProcEventos(0);
        InicVar=true;
    }
// Checar um diretório
    if (comparaZ(nome, "limpar")==0)
    {
        if (Instr::VarAtual < v+1) // Menos de 1 argumento
        {
            TVarSavDir::ChecaTudo();
            return false;
        }
        char mens[512];
        copiastr(mens, v[1].getTxt(), sizeof(mens));
        if (*mens==0)
            strcpy(mens, ".");
        Instr::ApagarVar(v);
    // Se inválido: retorna 0
        if (!arqvalido(mens, ""))
            return Instr::CriarVarInt(0);
    // Válido: coloca na lista de pendentes e retorna 1
        TVarSavDir::NovoDir(mens);
        return Instr::CriarVarInt(1);
    }
// Obtém o nome do arquivo
    char arqnome[512]; // Nome do arquivo; nulo se não for válido
    *arqnome=0;
    if (Instr::VarAtual >= v+1)
    {
        copiastr(arqnome, v[1].getTxt(), sizeof(arqnome)-4);
    // Verifica se nome permitido
        if (!arqvalido(arqnome))
            *arqnome=0;
    }
// Checa se nome de arquivo é válido
    if (comparaZ(nome, "valido")==0)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(*arqnome != 0);
    }
// Checa se arquivo existe
    if (comparaZ(nome, "existe")==0)
    {
        struct stat buf;
        Instr::ApagarVar(v);
        if (*arqnome && stat(arqnome, &buf)<0)
            *arqnome = 0;
        return Instr::CriarVarInt(*arqnome != 0);
    }
// Checar senha
    if (comparaZ(nome, "senha")==0)
    {
        char mens[512];
        TArqLer arqler;
        if (*arqnome==0 || Instr::VarAtual < v+2 || !arqler.Abrir(arqnome))
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarInt(0);
        }
        while (arqler.Linha(mens, sizeof(mens), false)>0)
        {
            if (strcmp(mens, "+++")==0)
                break;
            if (compara(mens, "senha=", 6)!=0)
                continue;
            char mens2[100];
            Senha(mens2, v[2].getTxt(), mens[6]);
            if (strcmp(mens+6, mens2)==0)
                break;
            Instr::ApagarVar(v);
            return Instr::CriarVarInt(0);
        }
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(1);
    }
// Checar a quantidade de dias para expirar
    if (comparaZ(nome, "dias")==0)
    {
        Instr::ApagarVar(v);
        if (!Instr::CriarVarInt(0))
            return false;
        if (*arqnome==0)
            return true;
        int x = Tempo(arqnome);
        if (x > 0)
            x = (x+1439)/1440;
        Instr::VarAtual->setInt(x);
        return true;
    }
// Ler arquivo
    if (comparaZ(nome, "ler")==0)
    {
        int x = Ler(v, arqnome);
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(x);
    }
// Salvar arquivo
    if (comparaZ(nome, "salvar")==0)
    {
        int x = Salvar(v, arqnome);
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(x);
    }
// Apagar arquivo
    if (comparaZ(nome, "apagar")==0)
    {
        Instr::ApagarVar(v);
        if (!Instr::CriarVarInt(0))
            return false;
        if (*arqnome)
        {
            remove(arqnome);
            struct stat buf;
            if (stat(arqnome, &buf)<0) // se arquivo não existe: sucesso
                Instr::VarAtual->setInt(1);
        }
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
bool TVarSav::ObterVar(TVariavel * var, TObjeto * obj, const char * nomevar)
{
    char nome[CLASSE_NOME_TAM+2];
    char * d = nome;
    int indvar = 0;

// Separa o nome da variável
    while (*nomevar && *nomevar!='.')
        if (d < nome + sizeof(nome) - 1)
            *d++ = *nomevar++;
    *d=0;

// Obtém o índice o vetor
    if (*nomevar=='.')
        for (nomevar++; *nomevar>='0' && *nomevar<='9'; nomevar++)
            indvar = indvar * 10 + *nomevar - '0';

// Obtém a posição da variável na classe
    TClasse * c = obj->Classe;
    int indice = c->IndiceNome(nome);
    if (indice<0)
    {
        var->defvar = 0;
        return false;
    }

// Prepara objeto TVariavel
    var->defvar = c->InstrVar[indice];
    indice = c->IndiceVar[indice];
    var->bit = indice >> 24;
    var->indice = indvar;
    if (TVariavel::Tamanho(var->defvar)==0 ||
            indice & 0x400000) // Variável da classe
    {
        var->defvar = 0;
        return false;
    }
    // Variável do objeto
    var->endvar = obj->Vars + (indice & 0x3FFFFF);

// Verifica se variável está marcada como "sav"
// Verifica o índice da variável
    if ((var->defvar[Instr::endProp] & 2) == 0 ||
            (indvar && indvar >=
            (unsigned char)var->defvar[Instr::endVetor]))
    {
        var->defvar = 0;
        return false;
    }
    //printf(" [%s] [%d]\n", var.defvar+Instr::endNome, indvar); fflush(stdout);
    return true;
}

//----------------------------------------------------------------------------
int TVarSav::Ler(TVariavel * v, const char * arqnome)
{
    int quantidade = 0xFFFF; // Número máximo de objetos
    char mens[8192];
    TArqLer arqler;
    if (Instr::VarAtual >= v+3)
        quantidade = v[2].getInt();
    if (*arqnome==0 ||  // Arquivo inválido
            Instr::VarAtual < v+2 || // Menos de dois argumentos
            quantidade <= 0 ||  // Número de objetos <=0
            v[2].defvar[2] != Instr::cListaObj || // Não é listaobj
            !arqler.Abrir(arqnome))  // Não conseguiu abrir arquivo
        return 0;
// Prepara variáveis
    TListaObj * listaobj = v[2].end_listaobj + v[2].indice;
    TListaX * listaitem = listaobj->Inicio;
    TObjeto * bufobj[MAX_OBJ];
    int numobj=0;
    if (quantidade > MAX_OBJ)
        quantidade = MAX_OBJ;
// Avança até a lista dos tipos de objetos
    while (arqler.Linha(mens, sizeof(mens), false)>0)
        if (strcmp(mens, "+++")==0)
            break;
// Cria objetos na listaobj se necessário
    while (arqler.Linha(mens, sizeof(mens), false)>0)
    {
        //printf("1> %s\n", mens); fflush(stdout);
        if (strcmp(mens, "+++")==0)
            break;
        if (numobj >= quantidade)
            continue;
    // Objeto está na lista
        if (listaitem)
        {
            bufobj[numobj++] = listaitem->Objeto;
            listaitem = listaitem->ListaDepois;
            continue;
        }
    // Obtém a classe
        TClasse * c = TClasse::Procura(mens);
    // Classe inválida: não cria objeto
        if (c==0)
        {
            bufobj[numobj++] = 0;
            continue;
        }
    // Classe válida: cria objeto e adiciona na lista
        bufobj[numobj] = TObjeto::Criar(c);
        listaobj->AddFim(bufobj[numobj]);
        numobj++;
    }
// Lê objetos
    TListaX * listaitem_fim = 0;
    TTextoTxt * textotxt_obj = 0; // textotxt que falta anota \n no final
    TTextoVar * textovar_obj = 0; // Para anotar texto em textovar
    char mensvar[65100];       // Texto que será anotado
    char * pmensvar = mensvar; // Aonde adicionar texto
    TVariavel var;
    var.defvar = 0;
    quantidade = numobj;
    numobj = 0;
    while (arqler.Linha(mens, sizeof(mens), false)>0)
    {
    // Próximo objeto
        if (strcmp(mens, "+++")==0)
        {
            var.defvar = 0;
            numobj++;
            if (numobj >= quantidade)
                break;
            continue;
        }
    // Verifica se objeto válido
        if (bufobj[numobj]==0)
            continue;
    // Separa nome da variável, índice e valor
        char * p = mens;
        while (*p && *p!='=')
            p++;
        if (*p!='=')
            continue;
        *p++ = 0;
    // Obtém a variável
        if (mens[0]!='.' || mens[1]!=0)
        {
            ObterVar(&var, bufobj[numobj], mens);
            listaitem_fim = 0;
            if (textotxt_obj)
            {
                textotxt_obj->AddTexto("\n", 1);
                textotxt_obj=0;
            }
        }
        if (var.defvar==0)
            continue;
    // Anota valor na variável
        switch (var.Tipo())
        {
    // Variáveis numéricas
        case varInt:
            var.setInt(atoi(p));
            break;
    // Ponto flutuante
        case varDouble:
            var.setDouble(atof(p));
            break;
    // Texto
        case varTxt:
            {
                char * d = mens;
                for (; *p; p++)
                {
                    if (*p != '\\' || p[1]==0)
                        *d++ = *p;
                    else switch (*++p)
                    {
                    case 'n':
                    case 'N': *d++ = Instr::ex_barra_n; break;
                    case 'b':
                    case 'B': *d++ = Instr::ex_barra_b; break;
                    case 'c':
                    case 'C': *d++ = Instr::ex_barra_c; break;
                    case 'd':
                    case 'D': *d++ = Instr::ex_barra_d; break;
                    case '\\': *d++ = '\\'; break;
                    }
                }
                *d=0;
                var.setTxt(mens);
            }
            break;
    // Referência a objetos
        case varObj:
            {
                int indice = (p[0]-0x21) * 0x40 + (p[1]-0x21);
                if (p[0]<0x21 || p[0]>=0x61 || p[1]<0x21 || p[1]>=0x61 ||
                        indice >= quantidade)
                    var.setObj(0);
                else
                    var.setObj(bufobj[indice]);
                break;
            }
    // Checa outros tipos de variáveis
        default:
    // ListaObj
            if (var.defvar[2]==Instr::cListaObj)
            {
                TListaObj * lobj = var.end_listaobj + var.indice;
            // Limpa a lista
                if (*p=='\'')
                {
                    p++;
                    while (lobj->Inicio)
                        lobj->Inicio->Apagar();
                }
            // Muda a lista
                while (*p)
                {
                // Obtém número do objeto
                    int subobj=0;
                    for (; *p>='0' && *p<='9'; p++)
                        subobj = subobj * 10 + *p - '0';
                // Verifica se adicionar novo objeto
                    if (*p!='.')
                    {
                        if (subobj < quantidade && bufobj[subobj])
                            listaitem_fim = lobj->AddFim(bufobj[subobj]);
                        else
                            listaitem_fim = 0;
                        while (*p && *p!='\'') p++;
                        if (*p) p++;
                        continue;
                    }
                // Adicionar referência a objeto
                    p++;
                    char * nomeobj = p;
                    while (*p && *p!='\'') p++;
                    char ch = *p;
                    *p++ = 0;
                    TVariavel varitem;
                    if (subobj < quantidade && bufobj[subobj])
                      if (ObterVar(&varitem, bufobj[subobj], nomeobj))
                        if (varitem.defvar[2] == Instr::cListaItem)
                        varitem.end_listaitem[varitem.indice].MudarRef(listaitem_fim);
                    if (ch==0)
                        break;
                }
                break;
            }
    // TextoTxt
            if (var.defvar[2]==Instr::cTextoTxt)
            {
                TTextoTxt * txtobj = var.end_textotxt + var.indice;
            // Outro textotxt: anota \n no fim do anterior
                if (textotxt_obj && textotxt_obj != txtobj)
                {
                    textotxt_obj->AddTexto("\n", 1);
                    textotxt_obj=0;
                }
            // Limpar textotxt
                if ((*p|0x20)=='l')
                {
                    p++;
                    txtobj->Limpar();
                    textotxt_obj=0;
                }
            // Anotar textopos
                if ((*p|0x20)=='p')
                {
                // Obtém número do objeto
                    TVariavel varitem;
                    int subobj=0;
                    for (p++; *p>='0' && *p<='9'; p++)
                        subobj = subobj * 10 + *p - '0';
                // Obtém a variável textopos
                    if (*p!='.' ||
                        subobj > quantidade || bufobj[subobj]==0 ||
                        ObterVar(&varitem, bufobj[subobj], p+1)==0 ||
                        varitem.defvar[2] != Instr::cTextoPos)
                        break;
                    TTextoPos * pos = varitem.end_textopos + varitem.indice;
                // Adicina \n no final de textotxt se necessário
                    if (textotxt_obj)
                    {
                        textotxt_obj->AddTexto("\n", 1);
                        textotxt_obj=0;
                    }
                // Aponta textopos para o final de textotxt
                    pos->MudarTxt(txtobj);
                    pos->Bloco = txtobj->Fim;
                    pos->PosicBloco = (pos->Bloco ? pos->Bloco->Bytes : 0);
                    pos->PosicTxt = txtobj->Bytes;
                    pos->LinhaTxt = txtobj->Linhas;
                    break;
                }
            // Anotar texto em textotxt
                if ((*p|0x20)=='t')
                {
                    char * d = mens;
                    for (p++; *p; p++)
                    {
                        if (*p != '\\' || p[1]==0)
                            *d++ = *p;
                        else switch (*++p)
                        {
                        case 'n':
                        case 'N': *d++ = Instr::ex_barra_n; break;
                        case 'b':
                        case 'B': *d++ = Instr::ex_barra_b; break;
                        case 'c':
                        case 'C': *d++ = Instr::ex_barra_c; break;
                        case 'd':
                        case 'D': *d++ = Instr::ex_barra_d; break;
                        case '\\': *d++ = '\\'; break;
                        }
                    }
                    if (d!=mens)
                    {
                        textotxt_obj = (d[-1]==Instr::ex_barra_n ?
                                        0 : txtobj);
                        txtobj->AddTexto(mens, d-mens);
                    }
                    break;
                }
                break;
            }
    // TextoVar
            if (var.defvar[2]==Instr::cTextoVar)
            {
                TTextoVar * txtvar = var.end_textovar + var.indice;
            // Outro textovar: anota texto e inicializa textovar
                if (txtvar != textovar_obj)
                {
                    if (pmensvar != mensvar)
                    {
                        *pmensvar=0;
                        textovar_obj->Mudar(mensvar);
                    }
                    txtvar->Limpar();
                    textovar_obj = txtvar;
                    pmensvar = mensvar;
                }
            // Acrescenta texto
                for (; *p; p++)
                {
                    if (*p != '\\' || p[1]==0)
                        *pmensvar++ = *p;
                    else switch (*++p)
                    {
                    case 'n':
                    case 'N': *pmensvar++ = Instr::ex_barra_n; break;
                    case 'b':
                    case 'B': *pmensvar++ = Instr::ex_barra_b; break;
                    case 'c':
                    case 'C': *pmensvar++ = Instr::ex_barra_c; break;
                    case 'd':
                    case 'D': *pmensvar++ = Instr::ex_barra_d; break;
                    case '\\': *pmensvar++ = '\\'; break;
                    case '0':
                        *pmensvar=0;
                        txtvar->Mudar(mensvar);
                        pmensvar = mensvar;
                    }
                    if (pmensvar >= mensvar + sizeof(mensvar))
                        pmensvar--;
                }
                break;
            }
    // DataHora
            if (var.defvar[2]==Instr::cDataHora)
                var.end_datahora[var.indice].LerSav(p);
        } // switch
    } // while
    if (textotxt_obj)
        textotxt_obj->AddTexto("\n", 1);

    if (pmensvar != mensvar)
    {
        *pmensvar=0;
        textovar_obj->Mudar(mensvar);
    }

    return quantidade;
}

//----------------------------------------------------------------------------
int TVarSav::Salvar(TVariavel * v, const char * arqnome)
{
    if (*arqnome==0 ||  // Arquivo inválido
            Instr::VarAtual < v+4 || // Menos de 4 argumentos
            v[2].defvar[2] != Instr::cListaObj) // Não é listaobj
        return 0;
// Cria arquivo
    FILE * arq = fopen(arqnome, "w");
    if (arq==0)
        return 0;
// Anota a quantidade de dias para expirar
    int valor = v[3].getInt();
    if (valor>=1)
        fprintf(arq, "data=%d\n", HoraReg + valor * 1440);
// Anota a senha
    const char * senha = v[4].getTxt();
    if (*senha)
    {
        char mens[256];
        Senha(mens, senha, circle_random() % 90 + 33);
        fprintf(arq, "senha=%s\n", mens);
    }
// Anota os tipos de objetos
    TListaObj * listaobj = v[2].end_listaobj + v[2].indice;
    TObjeto * bufobj[MAX_OBJ];
    unsigned int numobj=0;
    fprintf(arq, "+++\n");
    for (TListaX * litem = listaobj->Inicio; litem && numobj<MAX_OBJ;)
    {
        TObjeto * obj = litem->Objeto;
    // Checa se objeto já está em bufobj
        unsigned int num = obj->NumeroSav;
        if (num>=numobj || bufobj[num]!=obj)
        {
        // Acerta variáveis
            obj->NumeroSav = numobj;
            bufobj[numobj++] = obj;
        // Anota no arquivo
            fprintf(arq, "%s\n", obj->Classe->Nome);
        }
    // Passa para próximo objeto
        litem = litem->ListaDepois;
    }
// Anota as variáveis dos objetos
    unsigned int quantidade = numobj;
    for (numobj=0; numobj<quantidade; numobj++)
    {
        fprintf(arq, "+++\n");
        TClasse * c = bufobj[numobj]->Classe;
            // Procura todas as variáveis
        for (unsigned int x=0; x<c->NumVar; x++)
        {
        // Verifica se é "sav" e não é "comum"
            if ((c->InstrVar[x][Instr::endProp] & 3) != 2)
                continue;
        // Prepara objeto TVariavel
            TVariavel var;
            var.defvar = c->InstrVar[x];
            int posic = c->IndiceVar[x];
            var.bit = posic >> 24;
            var.indice = 0;
            if (TVariavel::Tamanho(var.defvar)==0 ||
                    (posic & 0x400000)) // Variável da classe
                continue;
            // Variável do objeto
            var.endvar = bufobj[numobj]->Vars + (posic & 0x3FFFFF);
        // Anota valor na variável
            posic = (unsigned char)var.defvar[Instr::endVetor];
            switch (var.Tipo())
            {
        // Variáveis numéricas
            case varInt:
                do {
                    if (var.indice)
                        fprintf(arq, "%s.%d=%d\n",
                                var.defvar+Instr::endNome,
                                var.indice, var.getInt());
                    else
                        fprintf(arq, "%s=%d\n",
                                var.defvar+Instr::endNome,
                                var.getInt());
                } while (++var.indice < posic);
                break;
        // Ponto flutuante
            case varDouble:
                do {
                    if (var.indice)
                        fprintf(arq, "%s.%d=%f\n",
                                var.defvar+Instr::endNome,
                                var.indice, var.getDouble());
                    else
                        fprintf(arq, "%s=%f\n",
                                var.defvar+Instr::endNome,
                                var.getDouble());
                } while (++var.indice < posic);
                break;
        // Texto
            case varTxt:
                do {
                    char mens[4096];
                    const char * o = var.getTxt();
                    char * d = mens;
                    for (; *o && d<mens+sizeof(mens)-2; o++)
                        switch (*o)
                        {
                        case Instr::ex_barra_n: *d++='\\'; *d++='n'; break;
                        case Instr::ex_barra_b: *d++='\\'; *d++='b'; break;
                        case Instr::ex_barra_c: *d++='\\'; *d++='c'; break;
                        case Instr::ex_barra_d: *d++='\\'; *d++='d'; break;
                        case '\\': *d++='\\';
                        default: *d++ = *o;
                        }
                    *d=0;
                    if (var.indice)
                        fprintf(arq, "%s.%d=%s\n",
                                var.defvar+Instr::endNome,
                                var.indice, mens);
                    else
                        fprintf(arq, "%s=%s\n",
                                var.defvar+Instr::endNome,
                                mens);
                } while (++var.indice < posic);
                break;
        // Referência a objetos
            case varObj:
                do {
                    char mens[3] = "";
                    TObjeto * obj = var.getObj();
                    if (obj)
                    {
                        unsigned int num = obj->NumeroSav;
                        if (num<quantidade && bufobj[num]==obj)
                        {
                            mens[0] = (num/0x40)+0x21;
                            mens[1] = (num%0x40)+0x21;
                            mens[2] = 0;
                        }
                    }
                    if (var.indice)
                        fprintf(arq, "%s.%d=%s\n",
                                var.defvar+Instr::endNome,
                                var.indice, mens);
                    else
                        fprintf(arq, "%s=%s\n",
                                var.defvar+Instr::endNome,
                                mens);
                } while (++var.indice < posic);
                break;
        // Checa outros tipos de variáveis
            default:
              switch (var.defvar[2])
              {
        // ListaObj
              case Instr::cListaObj:
                do {
                    char mens[512];
                    char * d = mens;
                    TListaObj * lobj = var.end_listaobj + var.indice;
                    TListaX * listax = lobj->Inicio;
                    if (var.indice)
                        sprintf(d, "%s.%d=",
                                var.defvar+Instr::endNome, var.indice);
                    else
                        sprintf(d, "%s=", var.defvar+Instr::endNome);
                    while (*d)
                        d++;
                    if (listax==0)
                        *d++ = '\'';
                    for (; listax; listax=listax->ListaDepois)
                    {
                    // Obtém o número do objeto
                        unsigned int num = listax->Objeto->NumeroSav;
                        if (num>=quantidade || bufobj[num]!=listax->Objeto)
                            continue;
                    // Verifica se tem espaço em mens
                        if (d >= mens + 150)
                        {
                            *d = 0;
                            fprintf(arq, "%s\n", mens);
                            d=copiastr(mens, ".=");
                        }
                        else
                            *d++ = '\'';
                    // Anota o número do objeto
                        sprintf(d, "%d", num);
                        while (*d) d++;
                    // Anota objetos listaitem
                        TListaItem * listaitem = listax->ListaItem;
                        for (; listaitem; listaitem = listaitem->Depois)
                        {
                        // Verifica se está marcado como sav
                            if ((listaitem->defvar[Instr::endProp] & 2)==0)
                                continue;
                        // Verifica se lista pertence a algum objeto
                            if (listaitem->Objeto==0)
                                continue;
                        // Verifica se objeto da listaitem será salvo
                            unsigned int nitem = listaitem->Objeto->NumeroSav;
                            if (nitem>=quantidade ||
                                    bufobj[nitem]!=listaitem->Objeto)
                                continue;
                        // Verifica se tem espaço em mens
                            if (d >= mens + 150)
                            {
                                *d = 0;
                                fprintf(arq, "%s\n", mens);
                                d=copiastr(mens, ".=");
                            }
                            else
                                *d++ = '\'';
                        // Anota dados da listaitem
                            if (listaitem->indice)
                                sprintf(d, "%d.%s.%d", nitem,
                                        listaitem->defvar+Instr::endNome,
                                        listaitem->indice);
                            else
                                sprintf(d, "%d.%s", nitem,
                                        listaitem->defvar+Instr::endNome);
                            while (*d) d++;
                        }
                    }
                    *d = 0;
                    fprintf(arq, "%s\n", mens);
                } while (++var.indice < posic);
                break;
        // TextoTxt
              case Instr::cTextoTxt:
                do {
                    char mens[512];
                    char * d = mens;
                    TTextoTxt * txtobj = var.end_textotxt + var.indice;
                    if (var.indice)
                        sprintf(d, "%s.%d=lt",
                                var.defvar+Instr::endNome, var.indice);
                    else
                        sprintf(d, "%s=lt", var.defvar+Instr::endNome);
                    while (*d)
                        d++;
                // Anota texto
                    TTextoBloco * bl = txtobj->Inicio;
                    char * ppos = (bl ? bl->Texto : 0);
                    int tampos = (bl ? bl->Bytes : 0);
                    unsigned int pos_atual = 0;
                    unsigned int pos_proc = 0;
                    while (true)
                    {
                    // Anota variáveis textopos
                        if (pos_atual == pos_proc)
                        {
                            bool textovazio = (pos_atual==0);
                            pos_proc = txtobj->Bytes + 1;
                            for (TTextoPos * posobj = txtobj->Posic; posobj;
                                    posobj=posobj->Depois)
                            {
                            // Verifica se está marcado como sav
                                if ((posobj->defvar[Instr::endProp] & 2)==0)
                                    continue;
                            // Verifica se já passou pela posição
                                if (posobj->PosicTxt < pos_atual)
                                    continue;
                            // Verifica se textopos será salvo
                                unsigned int nitem = posobj->Objeto->NumeroSav;
                                if (nitem>=quantidade ||
                                        bufobj[nitem]!=posobj->Objeto)
                                    continue;
                            // Acerta lin_proc
                                if (posobj->PosicTxt != pos_atual)
                                {
                                    if (pos_proc > posobj->PosicTxt)
                                        pos_proc = posobj->PosicTxt;
                                    continue;
                                }
                            // Anota posição
                                if (textovazio)
                                    d--;
                                else
                                    d=copiastr(d, "\n.=");
                                textovazio = true;
                                if (posobj->indice)
                                    sprintf(d, "p%d.%s.%d\n", nitem,
                                            posobj->defvar+Instr::endNome,
                                            posobj->indice);
                                else
                                    sprintf(d, "p%d.%s\n", nitem,
                                            posobj->defvar+Instr::endNome);
                                fprintf(arq, "%s", mens);
                                d = copiastr(mens, ".=t");
                            }
                        }
                        pos_atual++;
                    // Passa para o próximo bloco se necessário
                        if (tampos <= 0)
                        {
                            if (bl==0 || bl->Depois==0)
                                break;
                            bl = bl->Depois;
                            ppos = bl->Texto;
                            tampos = bl->Bytes;
                        }
                    // Anota texto se buffer cheio
                        if (d >= mens + 150)
                        {
                            *d = 0;
                            fprintf(arq, "%s\n", mens);
                            d=copiastr(mens, ".=t");
                        }
                    // Anota um byte
                        switch (*ppos)
                        {
                        case Instr::ex_barra_n: *d++='\\'; *d++='n'; break;
                        case Instr::ex_barra_b: *d++='\\'; *d++='b'; break;
                        case Instr::ex_barra_c: *d++='\\'; *d++='c'; break;
                        case Instr::ex_barra_d: *d++='\\'; *d++='d'; break;
                        case '\\': *d++='\\';
                        default: *d++ = *ppos;
                        }
                        ppos++, tampos--;
                    }
                    *d = 0;
                    if (strcmp(mens, ".=t")!=0)
                        fprintf(arq, "%s\n", mens);
                } while (++var.indice < posic);
                break;
        // TextoVar
              case Instr::cTextoVar:
                do {
                    char mens[512];
                    char * d = mens;
                    TTextoVar * txtvar = var.end_textovar + var.indice;
                    if (var.indice)
                        sprintf(d, "%s.%d=",
                                var.defvar+Instr::endNome, var.indice);
                    else
                        sprintf(d, "%s=", var.defvar+Instr::endNome);
                    while (*d)
                        d++;
                    TBlocoVar * bl = txtvar->RBroot;
                    if (bl)
                        bl = bl->RBfirst();
                    for (; bl; bl=TBlocoVar::RBnext(bl))
                    {
                        int cont=1;
                        const char * p = bl->Texto;
                        while (true)
                        {
                            if (*p==0)
                                if (cont--==0)
                                    break;
                            switch (*p)
                            {
                            case 0: *d++='='; break;
                            case Instr::ex_barra_n: *d++='\\'; *d++='n'; break;
                            case Instr::ex_barra_b: *d++='\\'; *d++='b'; break;
                            case Instr::ex_barra_c: *d++='\\'; *d++='c'; break;
                            case Instr::ex_barra_d: *d++='\\'; *d++='d'; break;
                            case '\\': *d++='\\';
                            default: *d++ = *p;
                            }
                            p++;
                            if (d-mens < 150)
                                continue;
                            *d=0;
                            fprintf(arq, "%s\n", mens);
                            d = copiastr(mens, ".=");
                        }
                        *d++ = '\\';
                        *d++ = '0';
                    }
                    *d=0;
                    if (strcmp(mens, ".=\\0")!=0)
                        fprintf(arq, "%s\n", mens);
                } while (++var.indice < posic);
                break;
        // DataHora
              case Instr::cDataHora:
                do {
                    char mens[100];
                    var.end_datahora[var.indice].SalvarSav(mens);
                    if (var.indice)
                        fprintf(arq, "%s.%d=%s\n",
                                var.defvar+Instr::endNome,
                                var.indice, mens);
                    else
                        fprintf(arq, "%s=%s\n",
                                var.defvar+Instr::endNome, mens);
                } while (++var.indice < posic);
                break;
              }
            } // switch
        } // for ... variáveis
    } // for ... objetos
// Fecha arquivo
    fclose(arq);
    return 1;
}

//----------------------------------------------------------------------------
void TVarSavDir::NovoDir(const char * nomedir)
{
// Procura objeto com o mesmo nome, retorna se encontrou
    TVarSavDir * y = RBroot;
    while (y)
    {
        int i = strcmp(nomedir, y->Nome);
        if (i==0)
            return;
        if (i<0)
            y = y->RBleft;
        else
            y = y->RBright;
    }
// Cria objeto
    new TVarSavDir(nomedir);
}

//----------------------------------------------------------------------------
TVarSavDir::TVarSavDir(const char * nomedir)
{
#ifdef DEBUG
    printf("Novo dir: %s\n", nomedir); fflush(stdout);
#endif
// Aloca memória em Nome
    int tam = strlen(nomedir)+1;
    Nome = new char[tam];
    memcpy(Nome, nomedir, tam);
// Coloca na RBT
    RBinsert();
// Coloca na lista ligada
    if (Inicio==0)
        Inicio=this;
    if (Fim)
        Fim->Proximo=this;
    Fim=this;
    Proximo=0;
}

//----------------------------------------------------------------------------
TVarSavDir::~TVarSavDir()
{
#ifdef DEBUG
    printf("Fim do dir: %s\n", Nome); fflush(stdout);
#endif
// Desaloca memória em Nome
    if (Nome)
        delete[] Nome;
// Tira da RBT
    RBremove();
// Tira da lista ligada
// Via de regra TVarSavDir só apaga o primeiro elemento da lista
    assert(Inicio==this);
    Inicio=Proximo;
    if (Fim==this)
        Fim=0;
}

//----------------------------------------------------------------------------
bool TVarSavDir::Checa()
{
    static DIR * dir = 0;
    for (int x=0; x<10; x++)
    {
    // Abre diretório se não estiver aberto
        if (dir==0)
        {
            if (Inicio==0)
                return false;
            dir = opendir(Inicio->Nome);
            if (dir==0)
                delete Inicio;
            continue;
        }
        while (true)
        {
    // Lê o próximo arquivo
            dirent * sdir = readdir(dir);
            if (sdir==0)
            {
                delete Inicio;
                closedir(dir);
                dir=0;
                break;
            }
    // Checa se é arquivo .sav
            char * pont = sdir->d_name;
            while (*pont)
                pont++;
            pont-=4;
            if (pont <= sdir->d_name)
                continue;
            if ( pont[0]!='.' || (pont[1]|0x20)!='s' ||
                 (pont[2]|0x20)!='a' || (pont[3]|0x20)!='v' )
                continue;
    // Apaga arquivo se expirou
            char arq[2048];
#ifdef __WIN32__
            mprintf(arq, sizeof(arq), "%s\\%s", Inicio->Nome, sdir->d_name);
#else
            mprintf(arq, sizeof(arq), "%s/%s", Inicio->Nome, sdir->d_name);
#endif
#ifdef DEBUG
            printf("%s arq: %s\n", TVarSav::Tempo(arq) == 0 ?
                    "Removendo" : "Checando", arq);
            fflush(stdout);
#endif
            if (TVarSav::Tempo(arq) == 0)
                remove(arq);
            break;
        }
    }
    return true;
}

//----------------------------------------------------------------------------
void TVarSavDir::ChecaTudo()
{
    while (Checa());
}

//----------------------------------------------------------------------------
bool TVarSavDir::ChecaPend()
{
    return (Inicio!=0);
}

//----------------------------------------------------------------------------
int TVarSavDir::RBcomp(TVarSavDir * x, TVarSavDir * y)
{
    return strcmp(x->Nome, y->Nome);
}

//----------------------------------------------------------------------------
TVarSavDir * TVarSavDir::Inicio=0;
TVarSavDir * TVarSavDir::Fim=0;
TVarSavDir * TVarSavDir::RBroot=0;
#define CLASS TVarSavDir    // Nome da classe
#define RBmask 1 // Máscara para bit 0
#include "rbt.cpp.h"
