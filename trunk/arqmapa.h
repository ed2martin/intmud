#ifndef ARQMAPA_H
#define ARQMAPA_H

//------------------------------------------------------------------------------
/** Lista de arquivos que compõem o mapa.
    Se não houver arquivo, significa que o mapa está em um único arquivo */
class TArqMapa /// Arquivos que compôem o programa interpretado
{
public:
    TArqMapa(const char * classe);
    ~TArqMapa();
    static void Salvar(const char * classe);
            /**< Indica que deve salvar arquivo
                 @note Deve ser chamado quando uma classe for criada ou apagada */
    bool Mudou;
            /**< Se o arquivo foi alterado */
    char Arquivo[32];   /**< ??? de intmud-???.map;
                             Se for nulo, é o arquivo intmud.map */
    TArqMapa *Anterior;     ///< Lista ligada; objeto anterior
    TArqMapa *Proximo;      ///< Lista ligada; próximo objeto
    static TArqMapa *Inicio;///< Lista ligada; primeiro objeto
    static TArqMapa *Fim;   ///< Lista ligada; último objeto
};

//------------------------------------------------------------------------------

#endif
