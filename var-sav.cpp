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
#include "variavel.h"
#include "classe.h"
#include "objeto.h"
#include "arqler.h"
#include "instr.h"
#include "misc.h"
#include "random.h"
#include "shs.h"

//#define DEBUG // Mostrar diretórios e arquivos acessados pela função LIMPAR

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
    SHS_INFO shsInfo;
    shsInit(&shsInfo);
    shsUpdate(&shsInfo, mens, tam);
    shsFinal(&shsInfo);
// Anota na string
    for (int a=0; a<5; a++)
    {
        unsigned long result = shsInfo.digest[a];
        for (int b=0; b<5; result/=90,b++)
            *senhacodif++ = (char)(result%90+33);
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
    while (arqler.Linha(mens, sizeof(mens))>0)
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
        char mens[300];
        copiastr(mens, v[1].getTxt(), sizeof(mens));
        if (*mens==0)
            strcpy(mens, ".");
        Instr::ApagarVar(v);
    // Se inválido: retorna 0
        if (memcmp(mens,"..",3)==0 || !arqvalido(mens))
            return Instr::CriarVar(Instr::InstrVarInt);
    // Válido: coloca na lista de pendentes e retorna 1
        TVarSavDir::NovoDir(mens);
        if (!Instr::CriarVar(Instr::InstrVarInt))
            return false;
        Instr::VarAtual->setInt(1);
        return true;
    }
// Obtém o nome do arquivo
    char arqnome[300]; // Nome do arquivo; nulo se não for válido
    *arqnome=0;
    if (Instr::VarAtual >= v+1)
    {
        char * p =  copiastr(arqnome, v[1].getTxt(), sizeof(arqnome)-4);
    // Verifica se nome permitido
        if (!arqvalido(arqnome))
            *arqnome=0;
        else
        {
    // Acrescenta ".sav" se necessário
            if (p <= arqnome+4)
                strcpy(p, ".sav");
            else if (comparaZ(p-4, ".sav")!=0)
                strcpy(p, ".sav");
            else
                strcpy(p-3, "sav");
        }
    }
// Checa se nome de arquivo é válido
    if (comparaZ(nome, "valido")==0)
    {
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrVarInt))
            return false;
        if (*arqnome)
            Instr::VarAtual->setInt(1);
        return true;
    }
// Checa se arquivo existe
    if (comparaZ(nome, "existe")==0)
    {
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrVarInt))
            return false;
        if (*arqnome)
        {
            struct stat buf;
            if (stat(arqnome, &buf)>=0)
                Instr::VarAtual->setInt(1);
        }
        return true;
    }
// Checar senha
    if (comparaZ(nome, "senha")==0)
    {
        char mens[300];
        TArqLer arqler;
        if (*arqnome==0 || Instr::VarAtual < v+2 || !arqler.Abrir(arqnome))
        {
            Instr::ApagarVar(v);
            return Instr::CriarVar(Instr::InstrVarInt);
        }
        while (arqler.Linha(mens, sizeof(mens))>0)
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
            return Instr::CriarVar(Instr::InstrVarInt);
        }
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrVarInt))
            return false;
        Instr::VarAtual->setInt(1);
        return true;
    }
// Checar a quantidade de dias para expirar
    if (comparaZ(nome, "dias")==0)
    {
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrVarInt))
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
        {
            Instr::ApagarVar(v);
            return Instr::CriarVar(Instr::InstrVarInt);
        }
    // Prepara variáveis
        TListaObj * listaobj = v[2].end_listaobj + v[2].indice;
        TListaX * listaitem = listaobj->Inicio;
        TObjeto * bufobj[1024];
        int numobj=0;
        if (quantidade > 1024)
            quantidade = 1024;
    // Avança até a lista dos tipos de objetos
        while (arqler.Linha(mens, sizeof(mens))>0)
            if (strcmp(mens, "+++")==0)
                break;
    // Cria objetos na listaobj se necessário
        while (arqler.Linha(mens, sizeof(mens))>0)
        {
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
        quantidade = numobj;
        numobj = 0;
        while (arqler.Linha(mens, sizeof(mens))>0)
        {
        // Próximo objeto
            if (strcmp(mens, "+++")==0)
            {
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
            unsigned int indvar = 0;
            while (*p && *p!='=' && *p!='.')
                p++;
            if (*p=='.')
                for (*p++=0; *p>='0' && *p<='9'; p++)
                    indvar = indvar * 10 + *p - '0';
            if (*p!='=')
                continue;
            *p++ = 0;
        // Procura a variável na classe
            TClasse * c = bufobj[numobj]->Classe;
            int indice = c->IndiceNome(mens);
            if (indice<0)
                continue;
        // Prepara objeto TVariavel
            TVariavel var;
            var.defvar = c->InstrVar[indice];
            indice = c->IndiceVar[indice];
            var.bit = indice >> 24;
            var.indice = indvar;
            if (TVariavel::Tamanho(var.defvar)==0 ||
                    indice & 0x400000) // Variável da classe
                continue;
            // Variável do objeto
            var.endvar = bufobj[numobj]->Vars + (indice & 0x3FFFFF);
        // Verifica se variável está marcada como "sav"
            if ((var.defvar[Instr::endProp] & 2) == 0)
                continue;
        // Verifica o índice da variável
            if (indvar && indvar >=
                    (unsigned char)var.defvar[Instr::endVetor])
                continue;
            //printf(" [%s] [%d]\n", mens, indvar); fflush(stdout);
        // Anota valor na variável
            switch (var.Tipo())
            {
            case varInt:
                var.setInt(atoi(p));
                break;
            case varDouble:
                var.setDouble(atof(p));
                break;
            case varTxt:
                {
                    char * d = mens;
                    for (; *p && d<mens+sizeof(mens)-1; p++)
                    {
                        if (*p != '\\')
                            *d++ = *p;
                        else switch (*++p)
                        {
                        case 'n':
                        case 'N':
                            *d++ = Instr::ex_barra_n;
                            break;
                        case 'b':
                        case 'B':
                            *d++ = Instr::ex_barra_b;
                            break;
                        case 'c':
                        case 'C':
                            *d++ = Instr::ex_barra_c;
                            break;
                        case 'd':
                        case 'D':
                            *d++ = Instr::ex_barra_d;
                            break;
                        case '\\':
                            *d++ = '\\';
                            break;
                        }
                    }
                    *d=0;
                    var.setTxt(mens);
                }
                break;
            case varObj:
                indice = (p[0]-0x21) * 0x40 + (p[1]-0x21);
                if (p[0]<0x21 || p[0]>=0x61 || p[1]<0x21 || p[1]>=0x61 ||
                        indice >= quantidade)
                    var.setObj(0);
                else
                    var.setObj(bufobj[indice]);
                break;
            default:
                if (var.defvar[2] == Instr::cListaObj)
                {
                    TListaObj * lobj = var.end_listaobj + var.indice;
                // Limpa a lista
                    while (lobj->Inicio)
                        lobj->Inicio->Apagar();
                // Enquanto houver objetos
                    while (p[0]>=0x21 && p[0]<0x61 && p[1]>=0x21 && p[1]<0x61)
                    {
                // Obtém o índice do objeto
                        int valor = (p[0]-0x21) * 0x40 + (p[1]-0x21);
                        p += 2;
                // Anota na lista
                        if (valor<quantidade && bufobj[valor])
                            lobj->AddFim(bufobj[valor]);
                    }
                }
            } // switch
        } // while
    // Retorna o número de objetos lidos
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrVarInt))
            return false;
        Instr::VarAtual->setInt(quantidade);
        return true;
    }
// Salvar arquivo
    if (comparaZ(nome, "salvar")==0)
    {
        if (*arqnome==0 ||  // Arquivo inválido
                Instr::VarAtual < v+4 || // Menos de 4 argumentos
                v[2].defvar[2] != Instr::cListaObj) // Não é listaobj
        {
            Instr::ApagarVar(v);
            return Instr::CriarVar(Instr::InstrVarInt);
        }
    // Cria arquivo
        FILE * arq = fopen(arqnome, "w");
        if (arq==0)
        {
            Instr::ApagarVar(v);
            return Instr::CriarVar(Instr::InstrVarInt);
        }
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
        TObjeto * bufobj[1024];
        unsigned int numobj=0;
        fprintf(arq, "+++\n");
        for (TListaX * litem = listaobj->Inicio; litem && numobj<1024;)
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
                case varTxt:
                    do {
                        char mens[4096];
                        const char * o = var.getTxt();
                        char * d = mens;
                        for (; *o && d<mens+sizeof(mens)-2; o++)
                            switch (*o)
                            {
                            case Instr::ex_barra_n:
                                *d++ = '\\';
                                *d++ = 'n';
                                break;
                            case Instr::ex_barra_b:
                                *d++ = '\\';
                                *d++ = 'b';
                                break;
                            case Instr::ex_barra_c:
                                *d++ = '\\';
                                *d++ = 'c';
                                break;
                            case Instr::ex_barra_d:
                                *d++ = '\\';
                                *d++ = 'd';
                                break;
                            case '\\':
                                *d++ = '\\';
                            default:
                                *d++ = *o;
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
                default:
                    if (var.defvar[2] != Instr::cListaObj)
                        break;
                    do {
                        char mens[4096];
                        char * d = mens;
                        TListaObj * lobj = var.end_listaobj + var.indice;
                        TListaX * litem = lobj->Inicio;
                        while (litem && d<mens+sizeof(mens)-3)
                        {
                            unsigned int num = litem->Objeto->NumeroSav;
                            if (num<quantidade && bufobj[num]==litem->Objeto)
                            {
                                *d++ = (num/0x40)+0x21;
                                *d++ = (num%0x40)+0x21;
                            }
                            litem = litem->ListaDepois;
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
                } // switch
            } // for ... variáveis
        } // for ... objetos
    // Fecha arquivo
        fclose(arq);
    // Retorna 1
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrVarInt))
            return false;
        Instr::VarAtual->setInt(1);
        return true;
    }
// Apagar arquivo
    if (comparaZ(nome, "apagar")==0)
    {
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrVarInt))
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
