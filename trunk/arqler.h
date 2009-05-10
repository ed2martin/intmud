#ifndef ARQLER_H
#define ARQLER_H

//----------------------------------------------------------------------------
/** Ler arquivos de texto, linha por linha */
class TArqLer /// L� arquivos de texto
{
public:
    TArqLer();  ///< Construtor
    ~TArqLer(); ///< Destrutor
    bool Abrir(const char * arquivo);
            /**< Abre arquivo
                 @param arquivo do arquivo
                 @return true se conseguiu abrir */
    void Fechar();
            /**< Fecha arquivo */
    int  Linha(char * destino, int tamanho);
            /**< L� pr�xima linha do arquivo; pula linhas vazias
                 @param  destino Endere�o destino
                 @param  tamanho Tamanho do buffer em destino
                 @return N�mero da linha lida,
                        ou 0 se fim do arquivo,
                        ou -1 se erro na leitura */

private:
    int  arq;           ///< Para acessar o arquivo
    char buf[2048];     ///< Bytes lidos do arquivo
    char *pler;         ///< Ponteiro de leitura
    char *ptotal;       ///< At� aonde pode ler
    int  linhanum;      ///< N�mero da �ltima linha lida
    unsigned char linhaCRLF; ///< Para detectar fim da linha (CR e LF)
};

//----------------------------------------------------------------------------

#endif
