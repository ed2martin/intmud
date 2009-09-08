#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
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

bool TVarSav::InicVar = false;
int TVarSav::HoraReg = 0;
int TVarSav::Dia = 0;
int TVarSav::Hora = 0;
int TVarSav::Min = 0;

//----------------------------------------------------------------------------
// Acerta vari�veis
void TVarSav::ProcEventos()
{
// Obt�m a hora
    time_t tempoatual=0;
    struct tm * tempolocal;
    time(&tempoatual);
    // localtime() Converte para representa��ao local de tempo
    tempolocal = localtime(&tempoatual);
    Dia = tempolocal->tm_yday+1;
    Hora = tempolocal->tm_hour;
    Min = tempolocal->tm_min;
// Data e hora para o registro
    HoraReg = tempolocal->tm_year + 300; // Anos, desde 1900
                            // Somando 300 para ser m�ltiplo de 400
    HoraReg--;
    HoraReg = HoraReg*365 + HoraReg/4 - HoraReg/100 + HoraReg/400;
    HoraReg += tempolocal->tm_yday; // Dias completos desde o in�cio do ano
    HoraReg = HoraReg*1440 + Hora*60 + Min;
    //printf("%d\n", (int)HoraReg); fflush(stdout);
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
// Codifica
    SHS_INFO shsInfo;
    shsInit(&shsInfo);
    shsUpdate(&shsInfo, (unsigned char *)&fator, 1);
    if (*senha)
        shsUpdate(&shsInfo, (unsigned char *)senha, strlen(senha));
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
        if (compara(mens, "data")!=0)
            continue;
        char * p = mens;
        while (*p && *p!='=') p++;
        if (*p=='=') p++;
        if (*p==0)
            break;
        int tempo = HoraReg - atoi(p);
        if (tempo<0)
            tempo=0;
        return (tempo+1439) / 1440;
    }
    return -1;
}

//----------------------------------------------------------------------------
bool TVarSav::Func(TVariavel * v, const char * nome)
{
// Inicializa vari�veis se necess�rio
    if (!InicVar)
    {
        ProcEventos();
        InicVar=true;
    }
// Obt�m o nome do arquivo
    char arqnome[300]; // Nome do arquivo; nulo se n�o for v�lido
    *arqnome=0;
    if (Instr::VarAtual >= v+1)
    {
        char * p =  copiastr(arqnome, v[1].getTxt(), sizeof(arqnome)-4);
    // Verifica se nome permitido
        if (!arqvalido(arqnome))
            *arqnome=0;
        else
        {
    // Acrescenta ".sav" se necess�rio
            if (p <= arqnome+4)
                strcpy(p, ".log");
            else if (comparaZ(p-4, ".log")!=0)
                strcpy(p, ".log");
            else
                strcpy(p-3, "log");
        }
    }
// Checa se nome de arquivo � v�lido
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
            if (compara(mens, "senha")!=0)
                continue;
            char * p = mens;
            while (*p && *p!='=') p++;
            if (*p=='=') p++;
            if (*p==0)
                break;
            char mens2[100];
            Senha(mens2, v[2].getTxt(), *p);
            if (strcmp(p, mens2)==0)
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
        if (*arqnome)
            Instr::VarAtual->setInt((Tempo(arqnome) - HoraReg)/1440);
        return true;
    }
// Ler arquivo
    if (comparaZ(nome, "ler")==0)
    {
        int quantidade = 0xFFFF; // N�mero m�ximo de objetos
        char mens[8192];
        TArqLer arqler;
        if (Instr::VarAtual >= v+3)
            quantidade = v[2].getInt();
        if (*arqnome==0 ||  // Arquivo inv�lido
                Instr::VarAtual < v+2 || // Menos de dois argumentos
                quantidade <= 0 ||  // N�mero de objetos <=0
                v[2].defvar[2] != Instr::cListaObj || // N�o � listaobj
                !arqler.Abrir(arqnome))  // N�o conseguiu abrir arquivo
        {
            Instr::ApagarVar(v);
            return Instr::CriarVar(Instr::InstrVarInt);
        }
    // Prepara vari�veis
        TListaObj * listaobj = v[2].end_listaobj + v[2].indice;
        TListaX * listaitem = listaobj->Inicio;
        TObjeto * bufobj[1024];
        int numobj=0;
        if (quantidade > 1024)
            quantidade = 1024;
    // Avan�a at� a lista dos tipos de objetos
        while (arqler.Linha(mens, sizeof(mens))>0)
            if (strcmp(mens, "+++")==0)
                break;
    // Cria objetos na listaobj se necess�rio
        while (arqler.Linha(mens, sizeof(mens))>0)
        {
            if (strcmp(mens, "+++")==0)
                break;
            if (numobj >= quantidade)
                continue;
        // Objeto est� na lista
            if (listaitem)
            {
                bufobj[numobj++] = listaitem->Objeto;
                listaitem = listaitem->ListaDepois;
                continue;
            }
        // Obt�m a classe
            TClasse * c = TClasse::Procura(mens);
        // Classe inv�lida: n�o cria objeto
            if (c==0)
            {
                bufobj[numobj++] = 0;
                continue;
            }
        // Classe v�lida: cria objeto e adiciona na lista
            bufobj[numobj] = TObjeto::Criar(c);
            listaobj->AddFim(bufobj[numobj]);
            numobj++;
        }
    // L� objetos
        quantidade = numobj;
        numobj = 0;
        while (arqler.Linha(mens, sizeof(mens))>0)
        {
        // Pr�ximo objeto
            if (strcmp(mens, "+++")==0)
            {
                numobj++;
                if (numobj >= quantidade)
                    break;
                continue;
            }
        // Verifica se objeto v�lido
            if (bufobj[numobj]==0)
                continue;
        // Separa nome da vari�vel, �ndice e valor
            char * p = mens;
            unsigned int indvar = 0;
            while (*p && *p!='=' && *p!='.')
                p++;
            if (*p=='.')
                for (*p++=0; *p && *p!='='; p++)
                    if (*p>='0' && *p<='9')
                        indvar = indvar * 10 + *p - '0';
            if (*p!='=')
                continue;
            *p++ = 0;
        // Procura a vari�vel na classe
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
                    indice & 0x400000) // Vari�vel da classe
                continue;
            // Vari�vel do objeto
            var.endvar = bufobj[numobj]->Vars + (indice & 0x3FFFFF);
        // Verifica se vari�vel est� marcada como "sav"
            if ((var.defvar[Instr::endProp] & 2) == 0)
                continue;
        // Verifica o �ndice da vari�vel
            if (indvar && indvar >=
                    (unsigned char)var.defvar[Instr::endIndice])
                continue;
        // Anota valor na vari�vel
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
                    for (; *p; p++)
                    {
                        if (*p != '\\')
                            *d++ = *p;
                        else switch (++*p)
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
                // Obt�m o �ndice do objeto
                        int valor = (p[0]-0x21) * 0x40 + (p[1]-0x21);
                        p += 2;
                // Anota na lista
                        if (valor<quantidade && bufobj[valor])
                            lobj->AddFim(bufobj[valor]);
                    }
                }
            } // switch
        } // while
    // Retorna o n�mero de objetos lidos
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrVarInt))
            return false;
        Instr::VarAtual->setInt(quantidade);
        return true;
    }
// Salvar arquivo
    if (comparaZ(nome, "salvar")==0)
    {
        if (*arqnome==0 ||  // Arquivo inv�lido
                Instr::VarAtual < v+4 || // Menos de 4 argumentos
                v[2].defvar[2] != Instr::cListaObj) // N�o � listaobj
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
        // Checa se objeto j� est� em bufobj
            unsigned int num = obj->NumeroSav;
            if (num<numobj && bufobj[num]==obj)
            {
            // Acerta vari�veis
                obj->NumeroSav = numobj;
                bufobj[numobj++] = obj;
            // Anota no arquivo
                fprintf(arq, "%s\n", obj->Classe->Nome);
            }
        // Passa para pr�ximo objeto
            litem = litem->ListaDepois;
        }
    // Anota as vari�veis dos objetos
        unsigned int quantidade = numobj;
        for (numobj=0; numobj<quantidade; numobj++)
        {
            fprintf(arq, "+++\n");
            TClasse * c = bufobj[numobj]->Classe;
                // Procura todas as vari�veis
            for (unsigned int x=0; x<c->NumVar; x++)
            {
            // Verifica se � "sav" e n�o � "comum"
                if ((c->InstrVar[x][Instr::endProp] & 3) != 2)
                    continue;
            // Prepara objeto TVariavel
                TVariavel var;
                var.defvar = c->InstrVar[x];
                int indice = c->IndiceVar[x];
                var.bit = indice >> 24;
                var.indice = 0;
                if (TVariavel::Tamanho(var.defvar)==0 ||
                        indice & 0x400000) // Vari�vel da classe
                    continue;
                // Vari�vel do objeto
                var.endvar = bufobj[numobj]->Vars + (indice & 0x3FFFFF);
            // Anota valor na vari�vel
                indice = (unsigned char)var.defvar[Instr::endIndice];
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
                    } while (++var.indice < indice);
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
                    } while (++var.indice < indice);
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
                    } while (++var.indice < indice);
                    break;
                case varObj:
                    do {
                        char mens[3] = "";
                        TObjeto * obj = var.getObj();
                        if (obj)
                        {
                            unsigned int num = obj->NumeroSav;
                            if (num<numobj && bufobj[num]==obj)
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
                    } while (++var.indice < indice);
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
                            if (num<numobj && bufobj[num]==litem->Objeto)
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
                    } while (++var.indice < indice);
                    break;
                } // switch
                break;
            } // for ... vari�veis
        } // for ... objetos
    // Fecha arquivo
        fclose(arq);
    // Retorna 1
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrVarInt))
            return false;
        Instr::VarAtual->setInt(quantidade);
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
            if (stat(arqnome, &buf)<0) // se arquivo n�o existe: sucesso
                Instr::VarAtual->setInt(1);
        }
        return true;
    }
    return false;
}
