#ifndef ARQMAPA_H
#define ARQMAPA_H

#define MAPA_NOME_TAM 32 // Tamanho máximo do prefixo dos nomes dos arquivos

//------------------------------------------------------------------------------
class TClasse;
/** Lista de arquivos que compõem o mapa.
    Se não houver arquivo, significa que o mapa está em um único arquivo */
class TArqMapa /// Arquivos que compôem o programa interpretado
{
public:
    TArqMapa(const char * classe);
    ~TArqMapa();
    static bool NomeValido(const char * nome);
        ///< Verifica se nome é um nome válido para arquivo
        /**<
         *  @param nome Nome a pesquisar
         *  @return true se o nome é valido */
    static TArqMapa * Procura(const char * nome);
        ///< Procura um arquivo a partir do nome
        /**<
         *  @param nome Nome a pesquisar
         *  @return Endereço do objeto, ou 0 se não foi encontrado */
    static void SalvarArq(bool tudo);
            ///< Salva classes em arquivo
            /**< @param tudo true=salvar todas as classes,
                             false=salvar só o que foi modificado */
    static unsigned char ParamIndent; ///< Quantos espaços para indentação
    static unsigned char ParamClasse; ///< Linhas entre classes
    static unsigned char ParamFunc;   ///< Linhas entre funções
    static unsigned char ParamVar;    ///< Linhas entre variáveis
    static bool MapaGrande; ///< Se está trabalhando com vários arquivos

    bool Mudou;             ///< Se o arquivo foi alterado
    bool Existe;            ///< Se o arquivo já existe
    char Arquivo[MAPA_NOME_TAM];   /**< ??? de intmud-???.map;
                             Se for nulo, é o arquivo intmud.map */

// Objetos TClasse
    TClasse * ClInicio;     ///< Primeira classe do arquivo
    TClasse * ClFim;        ///< Última classe do arquivo

// Objetos TArqMapa
    TArqMapa *Anterior;     ///< Lista ligada; objeto anterior
    TArqMapa *Proximo;      ///< Lista ligada; próximo objeto
    static TArqMapa *Inicio;///< Lista ligada; primeiro objeto
    static TArqMapa *Fim;   ///< Lista ligada; último objeto
};

//------------------------------------------------------------------------------

#endif
