//----------------------------------------------------------------------------
// SSL

// Algumas fontes consultadas:
// http://en.wikipedia.org/wiki/Dynamic_loading
// http://www.koders.com/delphi/fid94FAA2D1B645A46B090CE734B902058EEDDADDD5.aspx
// https://thunked.org/programming/openssl-tutorial-server-t12.html
// Código fonte do programa Cartavox (Projeto Dosvox)

// http://www.sslshopper.com/article-how-to-create-and-install-an-apache-self-signed-certificate.html
// O comando para gerar certificado:
// openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout mysitename.key -out mysitename.crt

// https://thunked.org/programming/openssl-tutorial-server-t12.html
// Os comandos para gerar certificado:
// openssl genrsa -out server.key 4096
// openssl req -new -key server.key -out server.csr
// openssl x509 -req -days 365 -in server.csr -signkey server.key -out server.crt

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ssl.h"

//------------------------------------------------------------------------------
#ifdef __WIN32__
#include <windows.h>
#define HANDLE_DLL HMODULE
#define GETPROC GetProcAddress
#else
#include <dlfcn.h>
#define HANDLE_DLL void*
#define GETPROC dlsym
#endif

//----------------------------------------------------------------------------
static HANDLE_DLL ssl_handle1 = nullptr; // Aponta para DLL aberta - SSL
static HANDLE_DLL ssl_handle2 = nullptr; // Aponta para DLL aberta - util
SSL_CTX * ssl_ctx_cliente = nullptr;
SSL_CTX * ssl_ctx_servidor = nullptr;
SSL_METHOD * ssl_metodo = nullptr;

//----------------------------------------------------------------------------
TSslGetError         SslGetError = nullptr;
TSslLibraryInit      SslLibraryInit = nullptr;
TSslLoadErrorStrings SslLoadErrorStrings = nullptr;
TSslCtxNew           SslCtxNew = nullptr;
TSslCtxFree          SslCtxFree = nullptr;
TSslSetFd            SslSetFd = nullptr;
TTLSMethod           TLSMethod = nullptr;
TSslMethodV2         SslMethodV2 = nullptr;
TSslMethodV3         SslMethodV3 = nullptr;
TSslMethodTLSV1      SslMethodTLSV1 = nullptr;
TSslMethodTLSV12     SslMethodTLSV12 = nullptr;
TSslMethodV23        SslMethodV23 = nullptr;
TSslNew              SslNew = nullptr;
TSslFree             SslFree = nullptr;
TSslAccept           SslAccept = nullptr;
TSslConnect          SslConnect = nullptr;
TSslShutdown         SslShutdown = nullptr;
TSslRead             SslRead = nullptr;
TSslPeek             SslPeek = nullptr;
TSslWrite            SslWrite = nullptr;
TSslPending          SslPending = nullptr;
TSslPrivateKeyFile   SslPrivateKeyFile = nullptr;
TSslCertificateFile  SslCertificateFile = nullptr;
TOPENSSLaddallalgorithms OPENSSLaddallalgorithms = nullptr;
TSslGetPeerCertificate SslGetPeerCertificate = nullptr;
TSslX509free         SslX509free = nullptr;
TSslX509d2i          SslX509d2i = nullptr;
TSslX509i2d          SslX509i2d = nullptr;

//----------------------------------------------------------------------------
void FechaSSL()
{
    if (ssl_handle1 == nullptr)
        return;
    if (ssl_ctx_cliente)
        SslCtxFree(ssl_ctx_cliente);
    ssl_ctx_cliente = nullptr;
    if (ssl_ctx_servidor)
        SslCtxFree(ssl_ctx_servidor);
    ssl_ctx_servidor = nullptr;
#ifdef __WIN32__
    FreeLibrary(ssl_handle1);
    FreeLibrary(ssl_handle2);
#else
    dlclose(ssl_handle1);
    dlclose(ssl_handle2);
#endif
    ssl_handle1 = ssl_handle2 = nullptr;
}

//----------------------------------------------------------------------------
const char * AbreSSL()
{
    if (ssl_handle1)
        return nullptr;
#ifdef __WIN32__
    ssl_handle1 = LoadLibrary("ssleay32.dll");
    if (ssl_handle1 == nullptr)
        ssl_handle1 = LoadLibrary("libssl32.dll");
    if (ssl_handle1 == nullptr)
        return "Erro ao carregar ssleay32.dll ou libssl32.dll";
    ssl_handle2 = LoadLibrary("libeay32.dll");
    if (ssl_handle2 == nullptr)
    {
        FreeLibrary(ssl_handle1);
        ssl_handle1 = nullptr;
        return "Erro ao carregar libeay32.dll";
    }
#else
#ifdef __APPLE__
    ssl_handle1 = dlopen("libssl.dylib", RTLD_LAZY);
#else
    ssl_handle1 = dlopen("libssl.so", RTLD_LAZY);
#endif
    if (ssl_handle1 == nullptr)
        return "Erro ao carregar libssl.so";
#ifdef __APPLE__
    ssl_handle2 = dlopen("libcrypto.dylib", RTLD_LAZY);
#else
    ssl_handle2 = dlopen("libcrypto.so", RTLD_LAZY);
#endif
    if (ssl_handle2 == nullptr)
    {
        dlclose(ssl_handle1);
        ssl_handle1 = nullptr;
        return "Erro ao carregar libcrypto.so";
    }
#endif

    SslGetError         = (TSslGetError)   GETPROC(ssl_handle1, "SSL_get_error");
    SslLibraryInit      = (TSslLibraryInit)GETPROC(ssl_handle1, "SSL_library_init");
    SslLoadErrorStrings = (TSslLoadErrorStrings)GETPROC(ssl_handle1, "SSL_load_error_strings");
    SslCtxNew           = (TSslCtxNew)     GETPROC(ssl_handle1, "SSL_CTX_new");
    SslCtxFree          = (TSslCtxFree)    GETPROC(ssl_handle1, "SSL_CTX_free");
    SslSetFd            = (TSslSetFd)      GETPROC(ssl_handle1, "SSL_set_fd");
    TLSMethod           = (TTLSMethod)     GETPROC(ssl_handle1, "TLS_method");
    SslMethodV2         = (TSslMethodV2)   GETPROC(ssl_handle1, "SSLv2_method");
    SslMethodV3         = (TSslMethodV3)   GETPROC(ssl_handle1, "SSLv3_method");
    SslMethodTLSV1      = (TSslMethodTLSV1)GETPROC(ssl_handle1, "TLSv1_method");
    SslMethodTLSV12     = (TSslMethodTLSV12)GETPROC(ssl_handle1, "TLSv1_2_method");
    SslMethodV23        = (TSslMethodV23)  GETPROC(ssl_handle1, "SSLv23_method");
    SslNew              = (TSslNew)        GETPROC(ssl_handle1, "SSL_new");
    SslFree             = (TSslFree)       GETPROC(ssl_handle1, "SSL_free");
    SslAccept           = (TSslAccept)     GETPROC(ssl_handle1, "SSL_accept");
    SslConnect          = (TSslConnect)    GETPROC(ssl_handle1, "SSL_connect");
    SslShutdown         = (TSslShutdown)   GETPROC(ssl_handle1, "SSL_shutdown");
    SslRead             = (TSslRead)       GETPROC(ssl_handle1, "SSL_read");
    SslPeek             = (TSslPeek)       GETPROC(ssl_handle1, "SSL_peek");
    SslWrite            = (TSslWrite)      GETPROC(ssl_handle1, "SSL_write");
    SslPending          = (TSslPending)    GETPROC(ssl_handle1, "SSL_pending");
    SslPrivateKeyFile   = (TSslPrivateKeyFile)GETPROC(ssl_handle1, "SSL_CTX_use_PrivateKey_file");
    SslCertificateFile  = (TSslCertificateFile)GETPROC(ssl_handle1, "SSL_CTX_use_certificate_file");
    OPENSSLaddallalgorithms = (TOPENSSLaddallalgorithms)GETPROC(ssl_handle2, "OPENSSL_add_all_algorithms_noconf");
    SslGetPeerCertificate = (TSslGetPeerCertificate)GETPROC(ssl_handle1,
            "SSL_get_peer_certificate");
    if (!SslGetPeerCertificate)
        SslGetPeerCertificate = (TSslGetPeerCertificate)GETPROC(ssl_handle1,
                "SSL_get1_peer_certificate");
    SslX509free         = (TSslX509free)   GETPROC(ssl_handle2, "X509_free");
    SslX509d2i          = (TSslX509d2i)    GETPROC(ssl_handle2, "d2i_X509");
    SslX509i2d          = (TSslX509i2d)    GETPROC(ssl_handle2, "i2d_X509");

    const char * erro = nullptr;
    if (!SslGetError)         erro = "SSL_get_error não foi encontrado";
    //if (!SslLibraryInit)      erro = "SSL_library_init não foi encontrado";
    //if (!SslLoadErrorStrings) erro = "SSL_load_error_strings não foi encontrado";
    if (!SslCtxNew)           erro = "SSL_CTX_new não foi encontrado";
    if (!SslCtxFree)          erro = "SSL_CTX_free não foi encontrado";
    if (!SslSetFd)            erro = "SSL_set_fd não foi encontrado";
    if (!TLSMethod && !SslMethodV2 && !SslMethodV23 && !SslMethodV3 && !SslMethodTLSV12)
        erro = "TLS_method/SSLv2_method/SSLv3_method/TLSv1_2_method não foi encontrado";
    if (!SslMethodTLSV1)      erro = "TLSv1_method não foi encontrado";
    if (!SslNew)              erro = "SSL_new não foi encontrado";
    if (!SslFree)             erro = "SSL_free não foi encontrado";
    if (!SslAccept)           erro = "SSL_accept não foi encontrado";
    if (!SslConnect)          erro = "SSL_connect não foi encontrado";
    if (!SslShutdown)         erro = "SSL_shutdown não foi encontrado";
    if (!SslRead)             erro = "SSL_read não foi encontrado";
    if (!SslPeek)             erro = "SSL_peek não foi encontrado";
    if (!SslWrite)            erro = "SSL_write não foi encontrado";
    if (!SslPending)          erro = "SSL_pending não foi encontrado";
    if (!SslPrivateKeyFile)   erro = "SSL_CTX_use_PrivateKey_file não foi encontrado";
    if (!SslCertificateFile)  erro = "SSL_CTX_use_certificate_file não foi encontrado";
    //if (!OPENSSLaddallalgorithms) erro = "OPENSSL_add_all_algorithms_noconf não foi encontrado";
    if (!SslGetPeerCertificate) erro = "SSL_get_peer_certificate não foi encontrado";
    if (!SslX509free)         erro = "X509_free não foi encontrado";
    if (!SslX509d2i)          erro = "d2i_X509 não foi encontrado";
    if (!SslX509i2d)          erro = "i2d_X509 não foi encontrado";
    if (erro)
    {
        FechaSSL();
        return erro;
    }

    //if (SslLibraryInit())
    //{
    //    FechaSSL();
    //    return "Erro ao inicializar OpenSSL (SSL_library_init)";
    //}
    if (SslLibraryInit)
        SslLibraryInit();
    if (SslLoadErrorStrings)
        SslLoadErrorStrings();
    if (OPENSSLaddallalgorithms)
        OPENSSLaddallalgorithms();
    //RAND_screen();

    if (ssl_metodo == nullptr && TLSMethod)
        ssl_metodo = TLSMethod();
    if (ssl_metodo == nullptr && SslMethodV23)
        ssl_metodo = SslMethodV23();
    if (ssl_metodo == nullptr && SslMethodV3)
        ssl_metodo = SslMethodV3();
    if (ssl_metodo == nullptr && SslMethodTLSV12)
        ssl_metodo = SslMethodTLSV12();
    if (ssl_metodo == nullptr && SslMethodV2)
        ssl_metodo = SslMethodV2();
    return nullptr;
}

//----------------------------------------------------------------------------
const char * AbreClienteSSL()
{
    const char * err = AbreSSL();
    if (err)
        return err;
    if (ssl_ctx_cliente == nullptr)
        ssl_ctx_cliente = SslCtxNew(ssl_metodo);
    return nullptr;
}

//----------------------------------------------------------------------------
const char * AbreServidorSSL(const char * arq_crt, const char * arq_key)
{
    const char * err = AbreSSL();
    if (err)
        return err;
    if (ssl_ctx_servidor == nullptr)
    {
        ssl_ctx_servidor = SslCtxNew(ssl_metodo);
        //SSL_CTX_set_options(tlsctx, SSL_OP_SINGLE_DH_USE);
        int valor;
        valor = SslCertificateFile(ssl_ctx_servidor, arq_crt, SSL_FILETYPE_PEM);
        if (valor <= 0)
        {
            SslCtxFree(ssl_ctx_servidor);
            ssl_ctx_servidor = nullptr;
            return "Não foi possível carregar certificado (arquivo CRT)";
        }
        valor = SslPrivateKeyFile(ssl_ctx_servidor, arq_key, SSL_FILETYPE_PEM);
        if (valor <= 0)
        {
            SslCtxFree(ssl_ctx_servidor);
            ssl_ctx_servidor = nullptr;
            return "Não foi possível carregar chave privativa (arquivo KEY)";
        }
    }
    return nullptr;
}
