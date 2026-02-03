/* Este arquivo é software livre; você pode redistribuir e/ou
 * modificar nos termos da licença LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
 #include <windows.h>
 //#include <stdafx.h>
#else
 #include <fcntl.h>
 #include <unistd.h>
 #include <signal.h>
 #include <errno.h>
 #include <sys/wait.h>
#endif
#include "exec.h"
#include "misc.h"

TExec * TExec::Inicio = nullptr;

//------------------------------------------------------------------------------
#ifndef _WIN32
void TExec::proc_sigchld_handler(int signum)
{
    int pid, status, serrno;
    serrno = errno;
    while (true)
    {
        pid = waitpid(-1, &status, WNOHANG);
        if (pid < 0 && errno!=ECHILD) // -1 significa que ocorreu algum erro
        {
            //perror("waitpid");
            break;
        }
        if (pid <= 0)
            break;
        for (TExec * obj = Inicio; obj; obj=obj->Depois)
        {
            if (!obj->Rodando || obj->pid != pid)
                continue;
            obj->pid = -1;
            if (WIFEXITED(status))
                obj->retorno = WEXITSTATUS(status);
            else
                obj->retorno = -1;
            break;
        }
    }
    errno = serrno;
    signal(SIGCHLD, proc_sigchld_handler); // Processar sinal SIGCHLD
}
#endif

//------------------------------------------------------------------------------
void TExec::Inicializa()
{
#ifndef _WIN32
    signal(SIGCHLD, proc_sigchld_handler); // Processar sinal SIGCHLD
#endif
}

//------------------------------------------------------------------------------
TExec::TExec()
{
    Rodando = false;
    Antes = nullptr;
    Depois = Inicio;
    if (Depois)
        Depois->Antes = this;
    Inicio = this;
    CodRetorno = 0;
#ifdef _WIN32
    pipe_in = pipe_out = INVALID_HANDLE_VALUE;
#else
    pipe_in = pipe_out = -1;
#endif
}

//------------------------------------------------------------------------------
TExec::~TExec()
{
    Fechar();
    (Antes ? Antes->Depois : Inicio) = Depois;
    if (Depois)
        Depois->Antes = Antes;
}

//------------------------------------------------------------------------------
const char * TExec::Abrir(const char * nomeprog, bool visivel)
{
    if (Rodando)
        return "Já está rodando um programa";
    FecharPipe();

    while (*nomeprog == ' ')
        nomeprog++;
    if (*nomeprog == 0)
        return "Arquivo não existe";

#ifdef _WIN32
    BOOL Success;
    HANDLE descrpipe[4];

// Prepara estrutura SECURITY_ATTRIBUTES
    memset(&SecurityAttributes, 0, sizeof(SecurityAttributes));
    SecurityAttributes.nLength              = sizeof(SECURITY_ATTRIBUTES);
    SecurityAttributes.bInheritHandle       = TRUE;
    SecurityAttributes.lpSecurityDescriptor = NULL;

// Cria pipe de entrada (STDOUT do programa)
    Success = CreatePipe(descrpipe, // Handle de leitura
                         descrpipe + 1, // Handle de escrita
                         &SecurityAttributes, // Atributos de segurança
                         0); // Número de bytes reservado para o pipe, 0=default
    if (!Success)
        return "Erro ao criar PIPE";

// Cria pipe de saída (STDIN do programa)
    Success = CreatePipe(descrpipe + 2, // Handle de leitura
                         descrpipe + 3, // Handle de escrita
                         &SecurityAttributes, // Atributos de segurança
                         0); // Número de bytes reservado para o pipe, 0=default
    if (!Success)
    {
        CloseHandle(descrpipe[0]);
        CloseHandle(descrpipe[1]);
        return "Erro ao criar PIPE";
    }

    DWORD dwMode = PIPE_READMODE_MESSAGE | PIPE_NOWAIT;
    Success = SetNamedPipeHandleState(
        descrpipe[3],   // Handle de leitura
        &dwMode,        // Modo do pipe
        NULL,           // don't set maximum bytes
        NULL);          // don't set maximum time

// Prepara estrutura STARTUPINFO
    memset(&StartupInfo, 0, sizeof(StartupInfo));
    StartupInfo.cb           = sizeof(STARTUPINFO);
    StartupInfo.dwFlags      = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    //StartupInfo.dwFlags      = STARTF_USESTDHANDLES;
    StartupInfo.wShowWindow  = visivel ? SW_SHOW : SW_HIDE;
    StartupInfo.hStdInput    = descrpipe[2];
    StartupInfo.hStdOutput   = descrpipe[1];
    StartupInfo.hStdError    = descrpipe[1];

// Cria processo filho
    memset(&ProcessInfo, 0, sizeof(ProcessInfo));
    Success = CreateProcess(
            NULL,             // Nome do módulo executável, pode ser NULL
            LPTSTR(nomeprog), // Linha de comando (nota: na versão unicode não deve ser const)
            NULL,             // Atributos de segurança
            NULL,             // Atributos de segurança de thread
            TRUE,             // Herda handles
            DETACHED_PROCESS, // Flags de criação e prioridade do processo
            NULL,             // Variáveis de ambiente
            NULL,             // Diretório do programa
            &StartupInfo,     // Estrutura STARTUPINFO
            &ProcessInfo);    // Estrutura PROCESS_INFORMATION
    if ( !Success )
    {
        DWORD dw = GetLastError();
        CloseHandle(descrpipe[0]);
        CloseHandle(descrpipe[1]);
        CloseHandle(descrpipe[2]);
        CloseHandle(descrpipe[3]);
        switch (dw)
        {
        case ERROR_FILE_NOT_FOUND: return "Arquivo não encontrado";
        case ERROR_PATH_NOT_FOUND: return "Caminho não encontrado";
        case ERROR_ACCESS_DENIED:
            return "Acesso negado";
        case ERROR_INVALID_EXE_SIGNATURE:
        case ERROR_EXE_MARKED_INVALID:
            return "Não é possível rodar o programa";
        case ERROR_BAD_EXE_FORMAT:
            return "Arquivo não é um aplicativo Win32 válido";
        case ERROR_EXE_MACHINE_TYPE_MISMATCH:
            return "Programa não é compatível com a sua versão do Windows";
        }
        return "Erro ao executar programa";
    }

// Acerta variáveis
    pipe_in = descrpipe[0];// Entrada
    CloseHandle(descrpipe[1]);
    CloseHandle(descrpipe[2]);    // Saída
    pipe_out = descrpipe[3];

    Rodando = true;
    return nullptr;

#else

// Obtém os argumentos
    int argc = 0;
    char * argv[20];
    char argtxt[1024];

    char * d = argtxt;
    char * dmax = argtxt + sizeof(argtxt);
    argv[argc++] = d;

    for (; d < dmax; nomeprog++)
    {
        // Encontrou barra invertida
        //     Copia o próximo caracter
        if (*nomeprog == '\\')
        {
            if (nomeprog[1] == 0)
                continue;
            nomeprog += 1;
            *d++ = *nomeprog;
            continue;
        }
        // Encontrou aspas
        //     Copia texto até encontrar aspas novamente
        //     Exceto se encontrar \"  aí considera aspas
        if (*nomeprog == '\"')
        {
            nomeprog++;
            while (*nomeprog && *nomeprog!='\"' && d<dmax)
            {
                if (nomeprog[0] == '\\')
                    if (nomeprog[1] == '\"')
                        nomeprog++;
                *d++=nomeprog[0], nomeprog++;
            }
            continue;
        }
        // Encontrou espaço ou fim do texto
        //     Passa para o próximo argumento
        //     Avança enquanto houver espaço
        if (*nomeprog == ' ' || *nomeprog == 0)
        {
            *d++ = 0;
            while (*nomeprog == ' ')
                nomeprog++;
            if (argc >= 19 || *nomeprog == 0)
                break;
            argv[argc++] = d;
            nomeprog--;
            continue;
        }

        *d++ = *nomeprog;
    }
    if (d >= dmax)
        return "Comando muito grande";
    argv[argc] = 0;

// Obtém o nome do executável
    char nomebuf[256];
    const char * nomearq = argv[0];

    if (*nomearq == 0)
        return "Arquivo não existe";
    if (strchr(nomearq, '/'))
    {
        if (access(nomearq, X_OK) < 0)
            nomearq = nullptr;
    }
    else
    {
        int nometam = strlen(nomearq);
        const char * path = getenv("PATH");
        if (path == nullptr)
            path = "/bin:/usr/bin";
        while (true)
        {
            if (*path == 0)
                return "Arquivo não existe";
            int pathtam = 0;
            const char * p = path;
            while (*path && *path != ':')
                path++;
            if (p == path)
                pathtam = 1, p = ".";
            else
                pathtam = path - p;
            if (*path == ':')
                path++;
            if (pathtam + nometam + 2 > (int)sizeof(nomebuf))
                continue;
            memcpy(nomebuf, p, pathtam);
            nomebuf[pathtam] = '/';
            memcpy(nomebuf + pathtam + 1, nomearq, nometam);
            nomebuf[pathtam + nometam + 1] = 0;
            if (access(nomebuf, X_OK) < 0)
                continue;
            nomearq = nomebuf;
            break;
        }
    }

#if 0
    // Mostra o nome do programa e os argumentos
    printf(">%s\n", nomearq);
    for (int x=0; argv[x]; x++)
        printf(">%s\n", argv[x]);
    fflush(stdout);
#endif

// Cria pipe
    int descrpipe[4];
    if (pipe(descrpipe) < 0)    // Cria "pipe" de entrada
        return "Erro ao criar PIPE";
    if (pipe(descrpipe + 2) < 0)// Cria "pipe" de saída
    {
        close(descrpipe[0]);
        close(descrpipe[1]);
        return "Erro ao criar PIPE";
    }

// Cria processo e executa programa
    char ch;
    pid = fork();               // Cria novo processo
    if (pid < 0)                // Se <0: ocorreu erro
    {
        close(descrpipe[0]);    // Entrada
        close(descrpipe[1]);
        close(descrpipe[2]);    // Saída
        close(descrpipe[3]);
        return "Erro ao criar processo";
    }
    if (pid > 0)                // Se >0: processo "pai"
    {
        pipe_in = descrpipe[0];// Entrada
        close(descrpipe[1]);
        close(descrpipe[2]);    // Saída
        pipe_out = descrpipe[3];
        safe_read(pipe_in, &ch, 1); // Sincroniza com processo filho
        fcntl(pipe_in, F_SETFL, O_NONBLOCK);
        fcntl(pipe_out, F_SETFL, O_NONBLOCK);
        safe_write(pipe_out, "a", 1);
        Rodando = true;
        return nullptr;
    }
    dup2(descrpipe[1],STDOUT_FILENO); // Entrada deste programa: stdout do outro
    dup2(descrpipe[1],STDERR_FILENO);
    dup2(descrpipe[2],STDIN_FILENO);  // Saída deste programa: stdin do outro
    close(descrpipe[0]);
    close(descrpipe[1]);
    close(descrpipe[2]);
    close(descrpipe[3]);
    signal(SIGPIPE,SIG_DFL);    // Acerta sinais
    signal(SIGCHLD,SIG_DFL);
    safe_write(STDOUT_FILENO, "a", 1); // Sincroniza com processo pai
    safe_read(STDIN_FILENO, &ch, 1);   // Lê um dado

// Executa programa
    execv(nomearq, argv);
    _exit(EXIT_FAILURE);
#endif
}

//------------------------------------------------------------------------------
void TExec::Fechar()
{
    if (Rodando)
    {
#ifdef _WIN32
        if (TerminateProcess(ProcessInfo.hProcess, 0))
            Rodando = false;
#else
        if (pid > 0)
            kill(pid, SIGTERM);
        Rodando = false;
#endif
    }
    FecharPipe();
}

//------------------------------------------------------------------------------
void TExec::FecharPipe()
{
#ifdef _WIN32
    if (pipe_in != INVALID_HANDLE_VALUE)  CloseHandle(pipe_in);
    if (pipe_out != INVALID_HANDLE_VALUE) CloseHandle(pipe_out);
    pipe_in = pipe_out = INVALID_HANDLE_VALUE;
#else
    if (pipe_in >= 0)  close(pipe_in);
    if (pipe_out >= 0) close(pipe_out);
    pipe_in = pipe_out = -1;
#endif
}

//------------------------------------------------------------------------------
int TExec::InfoProg()
{
#ifdef _WIN32
    if (!Rodando)
        return 0;
    if ( WaitForSingleObject(ProcessInfo.hProcess, 0) != WAIT_OBJECT_0 )  //lint !e1924 (warning about C-style cast)
        return 1;
    DWORD cod = 0;
    if (GetExitCodeProcess(ProcessInfo.hProcess, &cod) < 0)
        CodRetorno = -1;
    else
        CodRetorno = cod;
    // Notas:
    // Se GetExitCodeProcess() retornar FALSE, não foi possível obter o código de retorno
    // Se cod==STILL_ACTIVE, o processo ainda está rodando
#else
    if (!Rodando)
        return 0;
    if (pid >= 0)
        return 1;
    CodRetorno = retorno;
#endif
    Rodando = false;
    return 2;
}

//------------------------------------------------------------------------------
int TExec::Ler(char * destino, int total)
{
#ifdef _WIN32
    if (total <= 0 || pipe_in == INVALID_HANDLE_VALUE)
        return 0;
    DWORD   NumBytesRead = 0;
    DWORD   TotalBytesAvailable = 0;
    DWORD   BytesLeftThisMessage = 0;

    BOOL Success = PeekNamedPipe(
            pipe_in,            // Handle para o PIPE que será lido
            destino,            // Buffer para onde ler
            1,                  // Tamanho do buffer
            &NumBytesRead,      // Quantos bytes foram lidos
            &TotalBytesAvailable, // Quantos bytes disponíveis para leitura
            &BytesLeftThisMessage); // Bytes não lidos nessa mensagem
    if (!Success)
    {
        DWORD dw = GetLastError();
        if (dw != ERROR_BROKEN_PIPE)
            return 0;
        CloseHandle(pipe_in);
        pipe_in = INVALID_HANDLE_VALUE;
        return 0;
    }
    if (TotalBytesAvailable == 0)
        return 0;

    if (total > TotalBytesAvailable)
        total = TotalBytesAvailable;

    Success = ReadFile(
            pipe_in,            // Handle para o PIPE que será lido
            destino,            // Buffer para onde ler
            total,              // Quantos bytes ler
            &NumBytesRead,      // Quantos bytes foram lidos
            NULL);              // Estrutura para overlapped I/O
    return (Success ? NumBytesRead : 0);
#else
    if (total <= 0 || pipe_in < 0)
        return 0;
    int resposta = read(pipe_in, destino, total);
    if (resposta <= 0)
    {
        if (resposta < 0 && (errno == EINTR || errno == EWOULDBLOCK ||
                errno == ENOBUFS))
            resposta = 0;
        else
            resposta = -1;
    }
    if (resposta >= 0)
        return resposta;
    close(pipe_in);
    pipe_in = -1;
    return 0;
#endif
}

//------------------------------------------------------------------------------
int TExec::Escrever(const char * destino, int total)
{
#ifdef _WIN32
    if (total <= 0 || pipe_out == INVALID_HANDLE_VALUE)
        return 0;
    unsigned long bytesescritos = 0;
    if (WriteFile(pipe_out, destino, total, &bytesescritos, NULL))
        return bytesescritos;
    DWORD dw = GetLastError();
    if (dw != ERROR_BROKEN_PIPE)
        return 0;
    CloseHandle(pipe_out);
    pipe_out = INVALID_HANDLE_VALUE;
    return 0;
#else
    if (total <= 0 || pipe_out < 0)
        return 0;
    int resposta = write(pipe_out, destino, total);
    if (resposta > 0)
        return resposta;
    if (resposta < 0 && (errno == EINTR || errno == EWOULDBLOCK ||
            errno == ENOBUFS))
        return 0;
    close(pipe_out);
    pipe_out = -1;
    return 0;
#endif
}

//------------------------------------------------------------------------------
// Teste do exec
/*
#ifndef _WIN32
void Sleep(int tempo)
{
    struct timeval tselect;
    tselect.tv_sec = tempo / 1000;
    tselect.tv_usec = tempo % 1000 * 1000;
    select(FD_SETSIZE, 0, 0, 0, &tselect);
}
#endif

int main()
{
    TExec prg;
    TExec::Inicializa();

    //const char * err = prg.Abrir("ping intervox.nce.ufrj.br");
    //const char * err = prg.Abrir("ping localhost");
    const char * err = prg.Abrir("./a");
    if (err)
    {
        printf("ERRO: %s\n", err);
        return 1;
    }

    char cont='\n';
    while (true)
    {
        char mens[1000];
        bool rodando = (prg.InfoProg() == 1);
        int retorno = prg.CodRetorno;
        int lido = prg.Ler(mens, sizeof(mens) - 1);
        if (lido > 0)
        {
            mens[lido] = 0;
            printf("LIDO: %s\n", mens); fflush(stdout);
        }

        if (cont == '\n')
            cont = 'a';
        else if (cont == 'j')
            cont = '\n';
        else
            cont++;
        prg.Escrever(&cont, 1);
        putchar('.'); fflush(stdout);

        if (!rodando)
        {
            printf("Retornou %d\n", retorno);
            break;
        }
        Sleep(100);
    }

    prg.Fechar();
    return 0;
}
*/
