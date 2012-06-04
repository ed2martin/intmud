#ifndef SSL_H
#define SSL_H

//----------------------------------------------------------------------------
// Constantes
#define SSL_ERROR_NONE          0
#define SSL_ERROR_SSL           1
#define SSL_ERROR_WANT_READ     2
#define SSL_ERROR_WANT_WRITE    3
#define SSL_ERROR_WANT_X509_LOOKUP 4
#define SSL_ERROR_SYSCALL       5
#define SSL_ERROR_ZERO_RETURN   6
#define SSL_ERROR_WANT_CONNECT  7
#define SSL_ERROR_WANT_ACCEPT   8

#define X509_FILETYPE_PEM       1
#define SSL_FILETYPE_PEM        X509_FILETYPE_PEM

//----------------------------------------------------------------------------
// Vari�veis
struct SSL;
struct SSL_CTX;
struct SSL_METHOD;
struct X509;
struct X509_STORE_CTX;

//----------------------------------------------------------------------------
// Tipos
typedef int (*TSslGetError)(const SSL *s,int ret_code);
typedef int (*TSslLibraryInit)(void );
typedef void (*TSslLoadErrorStrings)(void);
typedef SSL_CTX * (*TSslCtxNew)(SSL_METHOD *meth);
typedef void     (*TSslCtxFree)(SSL_CTX *);
typedef int      (*TSslSetFd)(SSL *s, int fd);
typedef SSL_METHOD *(*TSslMethodV2)(void);
typedef SSL_METHOD *(*TSslMethodV3)(void);
typedef SSL_METHOD *(*TSslMethodTLSV1)(void);
typedef SSL_METHOD *(*TSslMethodV23)(void);
typedef SSL *   (*TSslNew)(SSL_CTX *ctx);
typedef void    (*TSslFree)(SSL *ssl);
typedef int     (*TSslAccept)(SSL *ssl);
typedef int     (*TSslConnect)(SSL *ssl);
typedef int     (*TSslShutdown)(SSL *s);
typedef int     (*TSslRead)(SSL *ssl,void *buf,int num);
typedef int     (*TSslPeek)(SSL *ssl,void *buf,int num);
typedef int     (*TSslWrite)(SSL *ssl,const void *buf,int num);
typedef int     (*TSslPending)(const SSL *s);
typedef int     (*TSslPrivateKeyFile)(SSL_CTX *ctx, const char *file, int type);
typedef int     (*TSslCertificateFile)(SSL_CTX *ctx, const char *file, int type);
typedef void    (*TOPENSSLaddallalgorithms)(void);
typedef X509 *  (*TSslGetPeerCertificate)(const SSL *ssl);
typedef void    (*TSslX509free)(X509 *x);
typedef X509 *  (*TSslX509d2i)(X509 **px, const unsigned char **in, int len);
typedef int     (*TSslX509i2d)(X509 *x, unsigned char **out);

//----------------------------------------------------------------------------
// Ponteiros de fun��es
extern TSslGetError         SslGetError;
extern TSslLibraryInit      SslLibraryInit;
extern TSslLoadErrorStrings SslLoadErrorStrings;
extern TSslCtxNew           SslCtxNew;
extern TSslCtxFree          SslCtxFree;
extern TSslSetFd            SslSetFd;
extern TSslMethodV2         SslMethodV2;
extern TSslMethodV3         SslMethodV3;
extern TSslMethodTLSV1      SslMethodTLSV1;
extern TSslMethodV23        SslMethodV23;
extern TSslNew              SslNew;
extern TSslFree             SslFree;
extern TSslAccept           SslAccept;
extern TSslConnect          SslConnect;
extern TSslShutdown         SslShutdown;
extern TSslRead             SslRead;
extern TSslPeek             SslPeek;
extern TSslWrite            SslWrite;
extern TSslPending          SslPending;
extern TSslPrivateKeyFile   SslPrivateKeyFile;
extern TSslCertificateFile  SslCertificateFile;
extern TOPENSSLaddallalgorithms OPENSSLaddallalgorithms;
extern TSslGetPeerCertificate SslGetPeerCertificate;
extern TSslX509free         SslX509free;
extern TSslX509d2i          SslX509d2i;
extern TSslX509i2d          SslX509i2d;

//----------------------------------------------------------------------------
// Outros
extern SSL_CTX * ssl_ctx_cliente;
extern SSL_CTX * ssl_ctx_servidor;
extern SSL_METHOD * ssl_metodo;

/// Fecha biblioteca SSL
void FechaSSL();

/// Abre biblioteca SSL
/** @return 0 se sucesso ou a mensagem de erro */
const char * AbreSSL();

/// Abre bibliteca SSL e contexto de cliente
/** @return 0 se sucesso ou a mensagem de erro */
const char * AbreClienteSSL();

/// Abre bibliteca SSL e contexto de servidor
/** @return 0 se sucesso ou a mensagem de erro */
const char * AbreServidorSSL(const char * arq_crt, const char * arq_key);

//----------------------------------------------------------------------------

#endif
