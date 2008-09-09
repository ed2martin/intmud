#ifndef ARQLER_H
#define ARQLER_H

//----------------------------------------------------------------------------
class TArqLer
{
public:
    TArqLer();
    ~TArqLer();
    bool Abrir(const char * arquivo);
                        // Abre arquivo; retorna false se n�o conseguiu
    void Fechar();      // Fecha arquivo
    int  Linha(char * destino, int tamanho);
                        // L� pr�xima linha do arquivo; pula linhas vazias
                        // Retorna o n�mero da linha lida, ou:
                        // 0 se fim do arquivo, -1 se erro na leitura

private:
    int  arq;
    char buf[2048];     // Bytes lidos do arquivo
    char *pler;         // Poneiro de leitura
    char *ptotal;       // At� aonde pode ler
    int  linhanum;      // N�mero da �ltima linha lida
    unsigned char linhaCRLF; // Para detectar fim da linha (CR e LF)
};

//----------------------------------------------------------------------------

#endif
