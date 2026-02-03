// Gerenciamento de erros que derrubam o programa
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <atomic>
#include <string>
#include "config.h"


//---------------------------------------------------------------------------
#if defined(_WIN32)

#include <windows.h>
#include <DbgHelp.h>

static std::string crashArquivo;
static std::atomic<uint64_t> crashDataHora{0};

// a Top-level Exception Handler in process level
static LONG WINAPI crashHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
    char texto[2048];
    uint64_t datahora = crashDataHora.load(std::memory_order_relaxed);
    unsigned year   = (datahora >> 26) & 0xFFF;
    unsigned month  = (datahora >> 22) & 0xF;
    unsigned day    = (datahora >> 17) & 0x1F;
    unsigned hour   = (datahora >> 12) & 0x1F;
    unsigned minute = (datahora >> 6)  & 0x3F;
    //unsigned second = datahora & 0x3F;

    snprintf(texto, sizeof(texto), "%s-%04d-%02d-%02d.log",
             crashArquivo.c_str(), year, month, day);
    HANDLE h = CreateFileA(texto,
                           FILE_APPEND_DATA,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);
    if (h == INVALID_HANDLE_VALUE)
        return EXCEPTION_CONTINUE_SEARCH;
    SetFilePointer(h, 0, NULL, FILE_END); // mover para o fim

    const char *  erro = "Unrecognized Exception";
    switch (pExceptionInfo->ExceptionRecord->ExceptionCode)
    {
        case EXCEPTION_ACCESS_VIOLATION:
            erro = "EXCEPTION_ACCESS_VIOLATION"; break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            erro = "EXCEPTION_ARRAY_BOUNDS_EXCEEDED"; break;
        case EXCEPTION_BREAKPOINT:
            erro = "EXCEPTION_BREAKPOINT"; break;
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            erro = "EXCEPTION_DATATYPE_MISALIGNMENT"; break;
        case EXCEPTION_FLT_DENORMAL_OPERAND:
            erro = "EXCEPTION_FLT_DENORMAL_OPERAND"; break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            erro = "EXCEPTION_FLT_DIVIDE_BY_ZERO"; break;
        case EXCEPTION_FLT_INEXACT_RESULT:
            erro = "EXCEPTION_FLT_INEXACT_RESULT"; break;
        case EXCEPTION_FLT_INVALID_OPERATION:
            erro = "EXCEPTION_FLT_INVALID_OPERATION"; break;
        case EXCEPTION_FLT_OVERFLOW:
            erro = "EXCEPTION_FLT_OVERFLOW"; break;
        case EXCEPTION_FLT_STACK_CHECK:
            erro = "EXCEPTION_FLT_STACK_CHECK"; break;
        case EXCEPTION_FLT_UNDERFLOW:
            erro = "EXCEPTION_FLT_UNDERFLOW"; break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            erro = "EXCEPTION_ILLEGAL_INSTRUCTION"; break;
        case EXCEPTION_IN_PAGE_ERROR:
            erro = "EXCEPTION_IN_PAGE_ERROR"; break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            erro = "EXCEPTION_INT_DIVIDE_BY_ZERO"; break;
        case EXCEPTION_INT_OVERFLOW:
            erro = "EXCEPTION_INT_OVERFLOW"; break;
        case EXCEPTION_INVALID_DISPOSITION:
            erro = "EXCEPTION_INVALID_DISPOSITION"; break;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
            erro = "EXCEPTION_NONCONTINUABLE_EXCEPTION"; break;
        case EXCEPTION_PRIV_INSTRUCTION:
            erro = "EXCEPTION_PRIV_INSTRUCTION"; break;
        case EXCEPTION_SINGLE_STEP:
            erro = "EXCEPTION_SINGLE_STEP"; break;
        case EXCEPTION_STACK_OVERFLOW:
            erro = "EXCEPTION_STACK_OVERFLOW"; break;
    }

    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();
    CONTEXT* ctx = pExceptionInfo->ContextRecord;

    // UNDNAME para nomes demanglados
    // DEFERRED_LOADS para carregar símbolos quando necessários
    SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME);
    SymInitialize(process, NULL, TRUE);

    STACKFRAME64 frame;
    ZeroMemory(&frame, sizeof(STACKFRAME64));

#ifdef _M_IX86
    DWORD machineType = IMAGE_FILE_MACHINE_I386;
    frame.AddrPC.Offset    = ctx->Eip;
    frame.AddrFrame.Offset = ctx->Ebp;
    frame.AddrStack.Offset = ctx->Esp;
#elif _M_X64
    DWORD machineType = IMAGE_FILE_MACHINE_AMD64;
    frame.AddrPC.Offset    = ctx->Rip;
    frame.AddrFrame.Offset = ctx->Rbp;
    frame.AddrStack.Offset = ctx->Rsp;
#elif _M_ARM64
    DWORD machineType = IMAGE_FILE_MACHINE_ARM64;
    frame.AddrPC.Offset    = ctx->Pc;
    frame.AddrFrame.Offset = ctx->Fp;
    frame.AddrStack.Offset = ctx->Sp;
#elif _M_ARM
    DWORD machineType = IMAGE_FILE_MACHINE_ARM;
    frame.AddrPC.Offset    = ctx->Pc;
    frame.AddrFrame.Offset = ctx->R11; // frame pointer
    frame.AddrStack.Offset = ctx->Sp;
#else
#error Unsupported architecture
#endif
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Mode = AddrModeFlat;

    DWORD written = 0;
    snprintf(texto, sizeof(texto),
            "===== %04d-%02d-%02d %02d:%02d "
            PACKAGE " V" VERSION " %s em %p =====\n",
            year, month, day, hour, minute,
            erro, (void*)frame.AddrPC.Offset);
    WriteFile(h, texto, (DWORD)strlen(texto), &written, NULL);

    for (int i = 0; i < 120; i++)
    {
        if (!StackWalk64(machineType, process, thread,
                &frame, ctx, NULL,
                SymFunctionTableAccess64, SymGetModuleBase64, NULL))
            break;

        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        DWORD64 displacement = 0;
        if (SymFromAddr(process, frame.AddrPC.Offset, &displacement, symbol))
            snprintf(texto, sizeof(texto), "%p %s\n",
                    (void*)frame.AddrPC.Offset, symbol->Name);
        else
            snprintf(texto, sizeof(texto), "%p\n", (void*)frame.AddrPC.Offset);
        WriteFile(h, texto, (DWORD)strlen(texto), &written, NULL);
    }

    SymCleanup(process);
    CloseHandle(h);
    return EXCEPTION_CONTINUE_SEARCH;
}

void crashAtualizaHora()
{
    uint64_t v = 0;
    SYSTEMTIME lt = {};
    GetLocalTime(&lt);
    v |= ((uint64_t)lt.wYear   & 0xFFF) << 26;
    v |= ((uint64_t)lt.wMonth  & 0xF)   << 22;
    v |= ((uint64_t)lt.wDay    & 0x1F)  << 17;
    v |= ((uint64_t)lt.wHour   & 0x1F)  << 12;
    v |= ((uint64_t)lt.wMinute & 0x3F)  << 6;
    v |= ((uint64_t)lt.wSecond & 0x3F);
    crashDataHora.store(v, std::memory_order_relaxed);
}

void crashInicializa(const char * arquivo)
{
    crashArquivo = arquivo;
    crashAtualizaHora();
    SetUnhandledExceptionFilter(crashHandler);
}

//---------------------------------------------------------------------------
#elif defined(HAVE_BACKTRACE)

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif
#include <signal.h>
#include <time.h>
#include <unistd.h> // write, close
#include <fcntl.h>  // open, O_WRONLY, O_CREAT, O_APPEND
#include <string.h> // strlen
#include <dlfcn.h>

static std::string crashArquivo;
static std::atomic<uint64_t> crashDataHora{0};

static void crashHandler(int sig)
{
    char texto[2048];
    uint64_t datahora = crashDataHora.load(std::memory_order_relaxed);
    unsigned year   = (datahora >> 26) & 0xFFF;
    unsigned month  = (datahora >> 22) & 0xF;
    unsigned day    = (datahora >> 17) & 0x1F;
    unsigned hour   = (datahora >> 12) & 0x1F;
    unsigned minute = (datahora >> 6)  & 0x3F;
    //unsigned second = datahora & 0x3F;

    snprintf(texto, sizeof(texto), "%s-%04d-%02d-%02d.log",
             crashArquivo.c_str(), year, month, day);
    int fd = open(texto, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd != -1)
    {
        dprintf(fd,
            "===== %04d-%02d-%02d %02d:%02d "
            PACKAGE " V" VERSION " Signal %d %s =====\n",
            year, month, day, hour, minute, sig,
            sig == SIGSEGV ? "SIGSEGV" :
            sig == SIGABRT ? "SIGABRT" :
            sig == SIGFPE ? "SIGFPE" :
            sig == SIGILL ? "SIGILL" : "Unknown");

        void * buffer[50];
        void ** buf2 = buffer;
        int size = backtrace(buffer, 50);
        if (size >= 2)
            buf2 += 2, size -= 2;

        // Nota: backtrace_symbols_fd também gera um bom log, porém, se passar
        // por alguma função exportada para o executável, o offset vai ser
        // em relação à função, Fica mais legível porém ruim para addr2line
        // backtrace_symbols_fd(buf2, size2, fd);

        for (int i = 0; i < size; i++)
        {
            Dl_info info;
            if (dladdr(buf2[i], &info) && info.dli_fname)
            {
                uintptr_t addr = (uintptr_t)buf2[i];
                uintptr_t base = (uintptr_t)info.dli_fbase;
                uintptr_t sym  = (uintptr_t)info.dli_saddr;
                uintptr_t off_bin = addr - base;
                uintptr_t off_sym = info.dli_sname ? addr - sym : 0;

                dprintf(fd, "%s(+0x%lx) [%p]",
                    info.dli_fname, off_bin, buf2[i]);
                if (info.dli_sname)
                    dprintf(fd, " <%s+0x%lx>", info.dli_sname, off_sym);
                dprintf(fd, "\n");
            }
            else
                dprintf(fd, "[%p]\n", buffer[i]);
        }

        close(fd);
    }
    //_exit(1);

    // Retorna o controle ao sistema operacional para encerrar o programa
    // Restaura o manipulador padrão e re-lança o sinal
    // para garantir o encerramento correto.
    signal(sig, SIG_DFL);
    raise(sig);
}

void crashAtualizaHora()
{
    uint64_t v = 0;
    struct tm * tm;
    time_t tempoatual;
    time(&tempoatual);
    tm = localtime(&tempoatual);
    v |= ((uint64_t)(tm->tm_year + 1900) & 0xFFF) << 26;
    v |= ((uint64_t)(tm->tm_mon + 1)     & 0xF)   << 22;
    v |= ((uint64_t)(tm->tm_mday)        & 0x1F)  << 17;
    v |= ((uint64_t)(tm->tm_hour)        & 0x1F)  << 12;
    v |= ((uint64_t)(tm->tm_min)         & 0x3F)  << 6;
    v |= ((uint64_t)(tm->tm_sec)         & 0x3F);
    crashDataHora.store(v, std::memory_order_relaxed);
}

void crashInicializa(const char * arquivo)
{
    crashArquivo = arquivo;
    crashAtualizaHora();

    struct sigaction sa{};
    sa.sa_handler = crashHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;

    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGFPE, &sa, nullptr);
    sigaction(SIGILL, &sa, nullptr);
}

//---------------------------------------------------------------------------
#else

void crashInicializa(const char * arquivo) {}
void crashAtualizaHora() { }

#endif
