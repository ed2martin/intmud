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
        return;
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
        return tempo;
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
        // Separa o nome da vari�vel do valor
            char * p = mens;
            while (*p && *p!='=') p++;
            if (*p=='=') *p++ = 0;
            if (*p==0)
                break;
        // Procura a vari�vel na classe
            TClasse * c = bufobj[numobj]->Classe;
            int indice = c->IndiceNome(mens);
            if (indice<0)
                continue;
        // Verifica se vari�vel est� marcada como "sav"
            if ((c->InstrVar[indice][Instr::endProp] & 2) == 0)
                continue;

//aaaaaaaa
//aaaaaaaaaa


        }
    // Retorna o n�mero de objetos lidos
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
