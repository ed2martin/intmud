#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "var-arqprog.h"
#include "variavel.h"
#include "arqmapa.h"
#include "instr.h"
#include "misc.h"

char * TArqIncluir::arqnome = 0;
TArqIncluir * TArqIncluir::Inicio = 0;
TArqIncluir * TArqIncluir::Fim = 0;

//----------------------------------------------------------------------------
TArqIncluir::TArqIncluir(const char * nome)
{
    copiastr(Padrao, nome, sizeof(Padrao));
    Anterior=Fim, Proximo=0;
    (Fim ? Fim->Proximo : Inicio)=this;
    Fim=this;
}

//----------------------------------------------------------------------------
TArqIncluir::~TArqIncluir()
{
    (Anterior ? Anterior->Proximo : Inicio) = Proximo;
    (Proximo ? Proximo->Anterior : Fim) = Anterior;
}

//----------------------------------------------------------------------------
bool TArqIncluir::ProcPadrao(const char * nome)
{
    if (*nome==0)
        return true;
    for (TArqIncluir * obj = Inicio; obj; obj=obj->Proximo)
        if (strcmp(nome, obj->Padrao)==0)
            return true;
    return false;
}

//----------------------------------------------------------------------------
bool TArqIncluir::ProcArq(const char * nome)
{
    for (TArqIncluir * obj = Inicio; obj; obj=obj->Proximo)
    {
        int tam = strlen(obj->Padrao);
        if (memcmp(nome, obj->Padrao, tam) != 0)
            continue;
        const char * p = nome + tam;
        while (*p && *p!='\\' && *p!='/')
            p++;
        if (*p == 0)
            return true;
    }
    return false;
}

//----------------------------------------------------------------------------
void TArqIncluir::ArqNome(const char * nome)
{
    if (arqnome)
    {
        delete[] arqnome;
        arqnome = 0;
    }
    if (nome == 0)
        return;
    int tam = strlen(nome) + 1;
    arqnome = new char[tam];
    memcpy(arqnome, nome, tam);
}

//----------------------------------------------------------------------------
const char * TArqIncluir::ArqNome()
{
    return arqnome;
}

//----------------------------------------------------------------------------
void TVarArqProg::Criar()
{
#ifdef __WIN32__
    wdir = INVALID_HANDLE_VALUE;
#else
    dir = 0;
#endif
}

//----------------------------------------------------------------------------
void TVarArqProg::Apagar()
{
    Fechar();
}

//----------------------------------------------------------------------------
void TVarArqProg::Abrir()
{
    Fechar();
    copiastr(Arquivo, TArqIncluir::ArqNome(), sizeof(Arquivo));
    Incluir = TArqIncluir::Inicio;
}

//----------------------------------------------------------------------------
void TVarArqProg::Fechar()
{
    *Arquivo = 0;
#ifdef __WIN32__
    if (wdir != INVALID_HANDLE_VALUE)
        FindClose(wdir);
    wdir = INVALID_HANDLE_VALUE;
#else
    if (dir)
        closedir(dir);
    dir = 0;
#endif
}

//----------------------------------------------------------------------------
void TVarArqProg::Proximo()
{
    char mens[ARQINCLUIR_TAM]; // Para procurar arquivos em um diretório
    const char * nomearq = 0; // Nome do arquivo encontrado

    while (true)
    {

// Se diretório fechado: começa a busca e obtém a primeira entrada
#ifdef __WIN32__
        if (wdir == INVALID_HANDLE_VALUE)
#else
        if (dir == 0)
#endif
        {
        // Se não há próximo diretório: fim da busca
            if (Incluir == 0)
            {
                *Arquivo = 0;
                return;
            }
        // Acerta variáveis Arquivo, ArqBarra e ArqPadrao
            copiastr(Arquivo, Incluir->Padrao, sizeof(Arquivo));
            ArqBarra = 0;
            const char * p;
            for (p = Arquivo; *p; p++)
                if (*p == '/' || *p == '\\')
                    ArqBarra = p+1-Arquivo;
            ArqPadrao = p - Arquivo;
#ifdef __WIN32__
        // Obtém o nome a procurar
            copiastr(mens, Incluir->Padrao, sizeof(mens)-1);
            strcat(mens, "*");
            for (char * p = mens; *p; p++)
                if (*p == '/')
                    *p = '\\';
        // Passa para o próximo Incluir
            Incluir = Incluir->Proximo;
        // Abre o diretório e obtém a primeira entrada
            WIN32_FIND_DATA ffd;
            wdir = FindFirstFile(mens, &ffd);
            if (wdir == INVALID_HANDLE_VALUE)
                continue;
            if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;
            nomearq = ffd.cFileName;
#else
        // Obtém o nome a procurar
            copiastr(mens, Incluir->Padrao, sizeof(mens)-1);
            strcat(mens, "*");
            char * p2 = mens;
            for (char * p = mens; *p; p++)
                switch (*p)
                {
                case '\\': *p = '/';
                case '/':  p2 = p;
                }
            *p2 = 0;
            if (p2 == mens)
                strcpy(mens, ".");
        // Passa para o próximo Incluir
            Incluir = Incluir->Proximo;
        // Abre o diretório e obtém a primeira entrada
            dir = opendir(mens);
            if (dir==0)
                continue;
            struct dirent * sdir = readdir(dir);
            if (sdir==0)
            {
                closedir(dir);
                dir = 0;
                continue;
            }
            if (sdir->d_type != DT_REG)
                continue;
            nomearq = sdir->d_name;
#endif
        }
        else

// Diretório está aberto: obtém a próxima entrada
        {
#ifdef __WIN32__
            WIN32_FIND_DATA ffd;
            if (FindNextFile(wdir, &ffd) == 0)
            {
                FindClose(wdir);
                wdir = INVALID_HANDLE_VALUE;
                continue;
            }
            if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;
            nomearq = ffd.cFileName;
#else
            struct dirent * sdir = readdir(dir);
            if (sdir==0)
            {
                closedir(dir);
                dir = 0;
                continue;
            }
            if (sdir->d_type != DT_REG)
                continue;
            nomearq = sdir->d_name;
#endif
        }

// Ignora entradas "." e ".."
        //if (strcmp(nomearq, ".")==0 || strcmp(nomearq, "..")==0)
        //    continue;

        if (*nomearq == '.') // Ignora arquivos cujo nome começa com um ponto
            continue;

// Checa início do nome do arquivo
        if (memcmp(nomearq, Arquivo+ArqBarra, ArqPadrao-ArqBarra) != 0)
            continue;

// Checa fim do nome do arquivo
        int tam = strlen(nomearq) - 4;
        if (tam < ArqPadrao - ArqBarra)
            continue;
        if (strcmp(nomearq + tam, ".int") != 0)
            continue;

// Checa se nome válido para arquivo ".int"
        if (!TArqMapa::NomeValido(nomearq))
            continue;

// Anota nome do arquivo
        if (tam > static_cast<int>(sizeof(Arquivo)) - ArqBarra + 1)
            continue;
        memcpy(Arquivo + ArqBarra, nomearq, tam);
        Arquivo[ArqBarra + tam] = 0;

// Checa arquivo inicial
        if (strcmp(Arquivo + ArqBarra, TArqIncluir::ArqNome()) == 0)
            continue;
        return;
    }
}

//------------------------------------------------------------------------------
bool TVarArqProg::Func(TVariavel * v, const char * nome)
{
// Pesquisar entrada atual no diretório
    if (comparaZ(nome, "lin")==0)
    {
        bool b = (*Arquivo != 0);
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(b);
    }
    if (comparaZ(nome, "texto")==0)
    {
        char arq[ARQINCLUIR_TAM];
        copiastr(arq, Arquivo, sizeof(arq));
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(arq);
    }
    if (comparaZ(nome, "depois")==0)
    {
        Proximo();
        return false;
    }

// Iniciar a busca
    if (comparaZ(nome, "abrir")==0)
    {
        Fechar();
        Abrir();
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto("");
    }
    if (comparaZ(nome, "fechar")==0)
    {
        Fechar();
        return false;
    }
    return false;
}
