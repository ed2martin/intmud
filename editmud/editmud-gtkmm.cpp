/*
Para compilar:
g++ -Wall -o editmud-gtkmm editmud-gtkmm.cpp `pkg-config --cflags --libs gtkmm-2.4`
strip --strip-all editmud-gtkmm

Para compilar no MinGW:
g++ -Wall -o editmud-gtkmm editmud-gtkmm.cpp -mwindows -lws2_32 -lwsock32 \
  `pkg-config --cflags --libs gtkmm-2.4`
strip --strip-all editmud-gtkmm.exe
*/

//---------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#ifdef __WIN32__
  #include <windows.h>
  #include <winsock.h>
  #include <sys\stat.h>
  #include <io.h>
#else
  #include <sys/types.h>
  #include <sys/socket.h> // Comunicação
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <netinet/in.h> // Contém a estrutura sockaddr_in
  #include <sys/stat.h>
  #include <signal.h>
  #include <errno.h>
#endif
#include <gtkmm.h>

//---------------------------------------------------------------------------
class TFrmConfig : public Gtk::Window
{
public:
    TFrmConfig();
    virtual ~TFrmConfig();
    Glib::ustring serv,porta,senha;
protected:
    void on_show1();
    void on_ok();
    void on_cancelar();
    Gtk::Table  main_tbl;
    Gtk::Label  m_lbl_serv, m_lbl_porta, m_lbl_senha;
    Gtk::Entry  m_txt_serv, m_txt_porta, m_txt_senha;
    Gtk::HButtonBox m_botoes;
    Gtk::Button m_but_ok, m_but_cancelar;
};

//---------------------------------------------------------------------------
class TFrmNomeClasse : public Gtk::Dialog
{
public:
    TFrmNomeClasse();
    Glib::ustring nomearq;
    void Exec(bool valor);
    bool ChkLer();
protected:
    Gtk::Table  main_tbl;
    Gtk::Label  m_lblnome;
    Gtk::Entry  m_txtnome;
    Gtk::CheckButton m_ler;
    Gtk::HButtonBox m_botoes;
};

//---------------------------------------------------------------------------
class TFrmMain : public Gtk::Window
{
public:
    TFrmMain();
    virtual ~TFrmMain();
    void StatusBar(Glib::ustring str);
    void on_ler();
    void on_enviar();
    void on_apagar();
    void on_salvar();
    void on_arqnovo();
    void on_arqapagar();
    void on_config();
    void on_selection_changed();
    void on_text_changed();
    bool on_timer();

protected:
    Gtk::VBox m_vbox1;
      Gtk::MenuBar m_menubar;
        Gtk::MenuItem m_ler;
        Gtk::MenuItem m_enviar;
        Gtk::MenuItem m_apagar;
        Gtk::MenuItem m_salvar;
        Gtk::MenuItem m_arqnovo;
        Gtk::MenuItem m_arqapagar;
        Gtk::MenuItem m_config;
      Gtk::Toolbar m_barra;
        Gtk::ToolButton m_tb_ler;
        Gtk::ToolButton m_tb_enviar;
        Gtk::ToolButton m_tb_apagar;
        Gtk::ToolButton m_tb_salvar;
        Gtk::ToolButton m_tb_arqnovo;
        Gtk::ToolButton m_tb_arqapagar;
        Gtk::ToolButton m_tb_config;
      Gtk::VBox m_hbox1;
        Gtk::HPaned m_painel;
          Gtk::ScrolledWindow m_scroll_lista;
            //TreeView
          Gtk::VBox m_box_texto;
            Gtk::Button m_buttontext;
            Gtk::ScrolledWindow m_scroll_texto;
              Gtk::TextView m_textview;
              Glib::RefPtr<Gtk::TextBuffer> m_textbuffer;
      Gtk::Statusbar m_status;

    //Tree model columns:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ModelColumns() { add(m_col_nome); }
        Gtk::TreeModelColumn<Glib::ustring> m_col_nome;
    };
    ModelColumns m_Columns;
    Gtk::TreeView m_TreeView;
    Glib::RefPtr<Gtk::ListStore> m_refTreeModel;

    int  LinhaAtual; // Número da linha atual
    char ListaNome[100]; // Arquivo sendo editado
    int  ListaMudou;  // Se o arquivo foi alterado; tempo para salvar o arquivo

    void Habilitar(bool enable); // Habilita / desabilita controles
    void TextoLerSalvar(bool salvar); // Lê/salva texto correspondente à lista

    bool Conectar(int destino); // Conecta ao servidor
                                // destino: 0=jogar no log, 1=ler
    void Desconectar(); // Desconecta
    void ProcConec(); // Processa comunicação com o servidor
    char sock_modo; // 0=não está processando, 1=jogar no log, 2=ler
#ifdef __WIN32__
    SOCKET sock_manip; // Socket, para comunicação
#else
    int sock_manip; // Socket, para comunicação
#endif
    char * sock_env; // Texto para enviar
    char * sock_rec; // Texto para receber
    char * sock_penv; // Ponteiro para envio
    char * sock_prec; // Ponteiro para receber
};

//---------------------------------------------------------------------------
TFrmConfig * FrmConfig = 0;
TFrmNomeClasse * FrmNomeClasse = 0;

//---------------------------------------------------------------------------
// Converte para UTF-8
// Necessário porque o GTK trabalha com UTF-8,
// mas o código fonte está em ISO8859-1
Glib::ustring ParaUTF8(Glib::ustring str)
{
    return Glib::convert(str, "utf-8", "iso8859-1");
}

//---------------------------------------------------------------------------
// Semelhante a strcpy(), mas retorna endereço do byte =0 em destino
// tamanho = número de caracteres que pode escrever em destino
char * copiastr(char * destino, const char * origem, int tamanho)
{
    if (*origem==0 || tamanho<=1)
    {
        if (tamanho>=1)
            *destino=0;
        return destino;
    }
    *(destino++) = *(origem++);
    tamanho-=2;  // -2 por causa do zero no final
    for (; tamanho && *origem; tamanho--)
        *(destino++) = *(origem++);
    *destino=0;
    return destino;
}

//---------------------------------------------------------------------------
TFrmConfig::TFrmConfig() :
        main_tbl(4,2,false),
        m_lbl_serv("Servidor"),
        m_lbl_porta("Porta"),
        m_lbl_senha("Senha"),
        m_but_ok(Gtk::Stock::OK),
        m_but_cancelar(Gtk::Stock::CANCEL)
{
    set_title(ParaUTF8("Configuração"));
    set_border_width(10);
    set_modal(true);
    set_position(Gtk::WIN_POS_CENTER);
    add(main_tbl);
    main_tbl.set_border_width(5);
    main_tbl.set_col_spacings(5);
    main_tbl.set_row_spacings(5);
    main_tbl.attach(m_lbl_serv,  0,1,0,1, Gtk::EXPAND, Gtk::EXPAND, 0, 0);
    main_tbl.attach(m_lbl_porta, 0,1,1,2, Gtk::EXPAND, Gtk::EXPAND, 0, 0);
    main_tbl.attach(m_lbl_senha, 0,1,2,3, Gtk::EXPAND, Gtk::EXPAND, 0, 0);
    main_tbl.attach(m_txt_serv,  1,2,0,1, Gtk::EXPAND, Gtk::FILL,   0, 0);
    main_tbl.attach(m_txt_porta, 1,2,1,2, Gtk::EXPAND, Gtk::FILL,   0, 0);
    main_tbl.attach(m_txt_senha, 1,2,2,3, Gtk::EXPAND, Gtk::FILL,   0, 0);
    main_tbl.attach(m_botoes,    0,2,3,4, Gtk::EXPAND, Gtk::EXPAND, 0, 0);
    m_botoes.pack_start(m_but_ok);
    m_botoes.pack_start(m_but_cancelar);
    m_botoes.set_border_width(5);
    m_botoes.set_spacing(5);
    m_botoes.set_layout(Gtk::BUTTONBOX_END);
    m_txt_senha.set_visibility(false);
    show_all_children(true);
    signal_show().connect(sigc::mem_fun(*this, &TFrmConfig::on_show1));
    m_but_ok.signal_clicked().connect(
                sigc::mem_fun(*this, &TFrmConfig::on_ok) );
    m_but_cancelar.signal_clicked().connect(
                sigc::mem_fun(*this, &TFrmConfig::on_cancelar) );
// Lê arquivo de configuração
    serv = "localhost";
    porta = "2000";
    senha = "modo";
    FILE * arq = fopen("editmud.cfg", "r");
    if (arq)
    {
        char * p1, * p2;
        char mens[2048];
        int lido = fread(mens, 1, sizeof(mens)-1, arq);
        fclose(arq);
        mens[lido]=0;
    // Servidor
        for (p1=mens; *p1 && *p1!='\n'; p1++);
        if (*p1) *p1++=0;
        serv=mens;
    // Porta
        p2=p1;
        for (; *p1 && *p1!='\n'; p1++);
        if (*p1) *p1++=0;
        porta=p2;
    // Senha
        p2=p1;
        for (; *p1 && *p1!='\n'; p1++);
        if (*p1) *p1++=0;
        senha=p2;
    }
}

//---------------------------------------------------------------------------
TFrmConfig::~TFrmConfig()
{
}

//---------------------------------------------------------------------------
void TFrmConfig::on_show1()
{
    m_txt_serv.set_text(serv);
    m_txt_porta.set_text(porta);
    m_txt_senha.set_text(senha);
}

//---------------------------------------------------------------------------
void TFrmConfig::on_ok()
{
    serv  = m_txt_serv.get_text();
    porta = m_txt_porta.get_text();
    senha = m_txt_senha.get_text();
    FILE * arq = fopen("editmud.cfg", "w");
    if (arq==0)
    {
        Gtk::MessageDialog dialog1(*this,
            ParaUTF8("Não consegui salvar arquivo de configuração"));
        dialog1.run();
        return;
    }
    std::string x = serv + std::string("\n") +
                    porta + std::string("\n") +
                    senha + std::string("\n");
    fwrite(x.c_str(), 1, x.size(), arq);
    fclose(arq);
    hide();
}

//---------------------------------------------------------------------------
void TFrmConfig::on_cancelar()
{
    hide();
}

//---------------------------------------------------------------------------
TFrmNomeClasse::TFrmNomeClasse() :
        main_tbl(2,2,false),
        m_lblnome("Nome da classe"),
        m_ler(ParaUTF8("Ler classe após criar arquivo"))
{
    set_border_width(10);
    set_modal(true);
    set_position(Gtk::WIN_POS_CENTER);
    get_vbox()->add(main_tbl);
    main_tbl.set_border_width(5);
    main_tbl.set_col_spacings(5);
    main_tbl.set_row_spacings(5);
    main_tbl.attach(m_lblnome, 0,1,0,1, Gtk::EXPAND, Gtk::EXPAND, 0, 0);
    main_tbl.attach(m_txtnome, 1,2,0,1, Gtk::EXPAND, Gtk::FILL,   0, 0);
    main_tbl.attach(m_ler,     0,2,1,2, Gtk::EXPAND, Gtk::FILL,   0, 0);
    show_all_children(true);
    add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
}

//---------------------------------------------------------------------------
void TFrmNomeClasse::Exec(bool valor)
{
    if (valor)
        m_ler.show();
    else
        m_ler.hide();
    m_ler.set_active(true);
    m_txtnome.set_text(nomearq);
    m_txtnome.grab_focus();
    if (run() == Gtk::RESPONSE_OK)
        nomearq = m_txtnome.get_text();
    else
        nomearq = "";
    hide();
}

//---------------------------------------------------------------------------
bool TFrmNomeClasse::ChkLer()
{
    return m_ler.get_active();
}

//---------------------------------------------------------------------------
TFrmMain::TFrmMain() :
        m_ler("_Ler", true),
        m_enviar("_Enviar", true),
        m_apagar("_Apagar", true),
        m_salvar("_Salvar prog", true),
        m_arqnovo("_Novo arquivo", true),
        m_arqapagar("A_pagar arquivo", true),
        m_config(ParaUTF8("_Configuração"), true),
        m_tb_ler(Gtk::Stock::GO_DOWN),
        m_tb_enviar(Gtk::Stock::GO_UP),
        m_tb_apagar(Gtk::Stock::DELETE), //CANCEL),
        m_tb_salvar(Gtk::Stock::SAVE),
        m_tb_arqnovo(Gtk::Stock::ADD), //NEW),
        m_tb_arqapagar(Gtk::Stock::REMOVE), //DELETE),
        m_tb_config(Gtk::Stock::PREFERENCES),
        m_buttontext("Linha 0")
{
    set_title("EditMUD");
    set_border_width(0);
    resize(640,480);
    LinhaAtual = 0;
    *ListaNome = 0;
    ListaMudou = 0;

    m_refTreeModel = Gtk::ListStore::create(m_Columns);
    m_TreeView.set_model(m_refTreeModel);
    m_TreeView.append_column("Arquivo", m_Columns.m_col_nome);

    m_textbuffer = Gtk::TextBuffer::create();
    m_textview.set_buffer(m_textbuffer);

    sock_modo = 0;
    sock_env = new char[65100]; // Texto para enviar
    sock_rec = new char[65100]; // Texto para receber
    sock_penv = 0;
    sock_prec = 0;

    add(m_vbox1);
    m_vbox1.pack_start(m_menubar, Gtk::PACK_SHRINK);
      m_menubar.append(m_ler);
      m_menubar.append(m_enviar);
      m_menubar.append(m_apagar);
      m_menubar.append(m_salvar);
      m_menubar.append(m_arqnovo);
      m_menubar.append(m_arqapagar);
      m_menubar.append(m_config);
    m_vbox1.pack_start(m_barra, Gtk::PACK_SHRINK);
      m_tb_ler.set_label("Ler");
      m_tb_enviar.set_label("Enviar");
      m_tb_apagar.set_label("Apagar");
      m_tb_salvar.set_label("Salvar prog");
      m_tb_arqnovo.set_label("Novo arquivo");
      m_tb_arqapagar.set_label("Apagar arquivo");
      m_tb_config.set_label(ParaUTF8("Configuração"));
      m_barra.append(m_tb_ler, sigc::mem_fun(*this, &TFrmMain::on_ler));
      m_barra.append(m_tb_enviar, sigc::mem_fun(*this, &TFrmMain::on_enviar));
      m_barra.append(m_tb_apagar, sigc::mem_fun(*this, &TFrmMain::on_apagar));
      m_barra.append(m_tb_salvar, sigc::mem_fun(*this, &TFrmMain::on_salvar));
      m_barra.append(m_tb_arqnovo, sigc::mem_fun(*this, &TFrmMain::on_arqnovo));
      m_barra.append(m_tb_arqapagar, sigc::mem_fun(*this, &TFrmMain::on_arqapagar));
      m_barra.append(m_tb_config, sigc::mem_fun(*this, &TFrmMain::on_config));
    m_vbox1.pack_start(m_hbox1);
      m_hbox1.pack_start(m_painel);
      m_painel.set_position(150);
      m_painel.add1(m_scroll_lista);
        m_scroll_lista.add(m_TreeView);
        m_scroll_lista.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
      m_painel.add2(m_box_texto);
        m_box_texto.pack_start(m_buttontext, Gtk::PACK_SHRINK);
        m_box_texto.pack_start(m_scroll_texto);
          m_scroll_texto.add(m_textview);
          m_scroll_texto.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    m_vbox1.pack_start(m_status, Gtk::PACK_SHRINK);

    Gtk::TreeModel::Row row = *(m_refTreeModel->append());
    row[m_Columns.m_col_nome] = "*Lista de classes";
    row = *(m_refTreeModel->append());
    row[m_Columns.m_col_nome] = "*LOG";
    m_status.push("");

// Sinais
    m_ler.signal_activate().connect(
            sigc::mem_fun(*this, &TFrmMain::on_ler));
    m_enviar.signal_activate().connect(
            sigc::mem_fun(*this, &TFrmMain::on_enviar));
    m_apagar.signal_activate().connect(
            sigc::mem_fun(*this, &TFrmMain::on_apagar));
    m_salvar.signal_activate().connect(
            sigc::mem_fun(*this, &TFrmMain::on_salvar));
    m_arqnovo.signal_activate().connect(
            sigc::mem_fun(*this, &TFrmMain::on_arqnovo));
    m_arqapagar.signal_activate().connect(
            sigc::mem_fun(*this, &TFrmMain::on_arqapagar));
    m_config.signal_activate().connect(
            sigc::mem_fun(*this, &TFrmMain::on_config));
    m_textbuffer->signal_changed().connect(
                    sigc::mem_fun(*this, &TFrmMain::on_text_changed) );
    sigc::connection conn = Glib::signal_timeout().connect(
                    sigc::mem_fun(*this, &TFrmMain::on_timer), 200);
    Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
            m_TreeView.get_selection();
    refTreeSelection->signal_changed().connect(
            sigc::mem_fun(*this, &TFrmMain::on_selection_changed));

// Lê diretório atual
    DIR * dir = opendir(".");
    if (dir)
    {
        while (1)
        {
            char mens[100];
            int tam;
            struct dirent * sdir = readdir(dir);
            if (sdir==0)
                break;
            if (memcmp(sdir->d_name, "mud_", 4)!=0)
                continue;
            tam = strlen(sdir->d_name+4);
            if (tam <= 4 || tam > (int)sizeof(mens))
                continue;
            if (memcmp(sdir->d_name+tam, ".txt", 4)!=0)
                continue;
            memcpy(mens, sdir->d_name+4, tam-4);
            mens[tam-4]=0;
            Gtk::TreeModel::Row row = *(m_refTreeModel->append());
            row[m_Columns.m_col_nome] = Glib::filename_to_utf8(mens);
        }
        closedir(dir);
    }

// Acertos finais
    Habilitar(true);
    refTreeSelection = m_TreeView.get_selection();
    row = m_refTreeModel->children()[0];
    if (row)
        refTreeSelection->select(row);
    m_textview.grab_focus();
    show_all_children(true);
}

//---------------------------------------------------------------------------
TFrmMain::~TFrmMain()
{
    if (ListaMudou)
        TextoLerSalvar(true); // Salva texto
    delete[] sock_env;
    delete[] sock_rec;
}

//---------------------------------------------------------------------------
void TFrmMain::StatusBar(Glib::ustring str)
{
    m_status.pop();
    m_status.push(ParaUTF8(str));
}

//---------------------------------------------------------------------------
void TFrmMain::Habilitar(bool enable)
{
    m_ler.set_sensitive(enable && memcmp(ListaNome, "*LO", 3)!=0);
    m_enviar.set_sensitive(enable && ListaNome[0]!='*');
    m_apagar.set_sensitive(enable);
    m_salvar.set_sensitive(enable);
    m_arqnovo.set_sensitive(enable);
    m_arqapagar.set_sensitive(enable && ListaNome[0]!='*');
    m_config.set_sensitive(enable);

    m_tb_ler.set_sensitive(enable && memcmp(ListaNome, "*LO", 3)!=0);
    m_tb_enviar.set_sensitive(enable && ListaNome[0]!='*');
    m_tb_apagar.set_sensitive(enable);
    m_tb_salvar.set_sensitive(enable);
    m_tb_arqnovo.set_sensitive(enable);
    m_tb_arqapagar.set_sensitive(enable && ListaNome[0]!='*');
    m_tb_config.set_sensitive(enable);

    m_textview.set_sensitive(enable);
    m_TreeView.set_sensitive(enable);
}

//---------------------------------------------------------------------------
void TFrmMain::TextoLerSalvar(bool salvar)
{
    char nomearq[65100];
    if (ListaNome[0] == 0)
    {
        if (!salvar)
            m_textbuffer->set_text("");
        return;
    }
    if (memcmp(ListaNome, "*Li", 3)==0)
        strcpy(nomearq, "mudclasse.txt");
    else if (memcmp(ListaNome, "*LO", 3)==0)
        strcpy(nomearq, "mudlog.txt");
    else
    {
        strcpy(nomearq, "mud_");
        copiastr(nomearq+4, ListaNome, 100);
        strcat(nomearq, ".txt");
    }
    if (salvar)
    {
        std::string str = Glib::filename_from_utf8(nomearq);
        FILE * arq = fopen(str.c_str(), "w");
        if (arq)
        {
            std::string mens = m_textbuffer->get_text();
            fwrite(mens.c_str(), 1, mens.size(), arq);
            fclose(arq);
        }
        //putchar('.'); fflush(stdout);
    }
    else
    {
        char mens[0xF000];
        std::string str = Glib::filename_from_utf8(nomearq);
        FILE * arq = fopen(str.c_str(), "r");
        *mens = 0;
        if (arq)
        {
            int lido = fread(mens, 1, sizeof(mens)-1, arq);
            mens[lido]=0;
            fclose(arq);
        }
        m_textbuffer->set_text(mens);
    }
}

//---------------------------------------------------------------------------
void TFrmMain::on_ler()
{
    if (*ListaNome != '*')
        sprintf(sock_env, "%s ler %s\n",
                FrmConfig->senha.c_str(), ListaNome);
    else
        sprintf(sock_env, "%s lista\n",
                FrmConfig->senha.c_str());
    Conectar(1);
}

//---------------------------------------------------------------------------
void TFrmMain::on_enviar()
{
    char * p = sock_env;
    sprintf(sock_env, "%s mudar %s\n",
            FrmConfig->senha.c_str(), ListaNome);
    while (*p)
        p++;
    copiastr(p, m_textbuffer->get_text().c_str(), 65000);
    strcat(p, "\n---\n");
    Conectar(0);
}

//---------------------------------------------------------------------------
void TFrmMain::on_apagar()
{
// Pergunta o nome da classe
    if (*ListaNome!='*')
        FrmNomeClasse->nomearq = ListaNome;
    else
    {
        Glib::RefPtr<Gtk::TextBuffer> buffer = m_textview.get_buffer();
        Gtk::TextIter ini = buffer->get_insert()->get_iter();
        Gtk::TextIter fim = buffer->get_insert()->get_iter();
        ini.set_line_offset(0);
        while (!fim.ends_line()) fim++;
        FrmNomeClasse->nomearq = ini.get_slice(fim);
    }
    FrmNomeClasse->set_title("Apagar classe");
    FrmNomeClasse->set_transient_for(*this);
    FrmNomeClasse->Exec(false);
    if (FrmNomeClasse->nomearq == "")
        return;

// Apaga a classe
    sprintf(sock_env, "%s apagar %s\n",
            FrmConfig->senha.c_str(),
            FrmNomeClasse->nomearq.c_str());
    Conectar(0);
}

//---------------------------------------------------------------------------
void TFrmMain::on_salvar()
{
    sprintf(sock_env, "%s salvar\n", FrmConfig->senha.c_str());
    Conectar(0);
}

//---------------------------------------------------------------------------
void TFrmMain::on_arqnovo()
{
// Pergunta o nome do arquivo
    Glib::RefPtr<Gtk::TextBuffer> buffer = m_textview.get_buffer();
    Gtk::TextIter ini = buffer->get_insert()->get_iter();
    Gtk::TextIter fim = buffer->get_insert()->get_iter();
    ini.set_line_offset(0);
    while (!fim.ends_line()) fim++;
    FrmNomeClasse->nomearq = ini.get_slice(fim);
    FrmNomeClasse->set_title("Novo arquivo");
    FrmNomeClasse->set_transient_for(*this);
    FrmNomeClasse->Exec(true);
    if (FrmNomeClasse->nomearq == "")
        return;

// Obtém o nome do arquivo
    char nomearq[128];
    strcpy(nomearq, "mud_");
    copiastr(nomearq+4, FrmNomeClasse->nomearq.c_str(), 100);
    strcat(nomearq, ".txt");
    std::string str = Glib::filename_from_utf8(nomearq);

// Checa se arquivo já existe
    struct stat buf;
    if (stat(str.c_str(), &buf)>=0)
    {
        StatusBar("Arquivo já existe");
        return;
    }

// Cria arquivo
    FILE * arq = fopen(str.c_str(), "w");
    if (arq==0)
    {
        StatusBar("Não consegui criar arquivo");
        return;
    }
    fclose(arq);

// Adiciona na lista e seleciona arquivo
    copiastr(nomearq, FrmNomeClasse->nomearq.c_str(), 100);
    Gtk::TreeModel::Row row = *(m_refTreeModel->append());
    row[m_Columns.m_col_nome] = nomearq;
    Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_TreeView.get_selection();
    refTreeSelection->select(row);
    m_TreeView.scroll_to_row(Gtk::TreePath(row), 0.0);
    StatusBar("Arquivo criado");

// Lê do servidor
    if (FrmNomeClasse->ChkLer())
        on_ler();
}

//---------------------------------------------------------------------------
void TFrmMain::on_arqapagar()
{
// Pede confirmação do usuário
    Gtk::MessageDialog dialog(*this,
            "Apagar arquivo?",
            false, // use_markup
            Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
    dialog.set_transient_for(*this);
    if (dialog.run() != Gtk::RESPONSE_YES)
        return;

// Apaga arquivo
    char nomearq[128];
    strcpy(nomearq, "mud_");
    copiastr(nomearq+4, ListaNome, 100);
    strcat(nomearq, ".txt");
    std::string str = Glib::filename_from_utf8(nomearq);
    if ( ::remove(str.c_str()) < 0 )
    {
        StatusBar("Não consegui apagar arquivo");
        return;
    }

// Tira da lista
    Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_TreeView.get_selection();
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    ListaMudou=0;

    Gtk::TreeModel::iterator iter2 = iter;
    iter2++;
    if (iter2 == 0)
        iter2 = iter, iter2--;
    //iter2 = m_refTreeModel->children().begin();
    if (iter2)
        refTreeSelection->select(iter2);
    m_refTreeModel->erase(iter);

// Avisa que apagou
    StatusBar("Arquivo apagado");
}

//---------------------------------------------------------------------------
void TFrmMain::on_config()
{
    FrmConfig->set_transient_for(*this);
    FrmConfig->show();
}

//---------------------------------------------------------------------------
void TFrmMain::on_selection_changed()
{
    if (ListaMudou)
        TextoLerSalvar(true); // Salva texto

    Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_TreeView.get_selection();
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    Glib::ustring str;
    if (iter)
        str = (*iter)[m_Columns.m_col_nome];
    copiastr(ListaNome, str.c_str(), sizeof(ListaNome));

    Habilitar(1);
    TextoLerSalvar(false); // Lê texto
    ListaMudou = 0;
}

//---------------------------------------------------------------------------
void TFrmMain::on_text_changed()
{
    if (ListaMudou==0)
        ListaMudou = 15;
}

//---------------------------------------------------------------------------
bool TFrmMain::on_timer()
{
// Atualiza linha e coluna na tela
    Glib::RefPtr<Gtk::TextBuffer> buffer = m_textview.get_buffer();
    Gtk::TextIter inter = buffer->get_insert()->get_iter();
    int lin = inter.get_line();
    //int col = inter.get_visible_line_offset();
    if (LinhaAtual != lin)
    {
        LinhaAtual = lin;
        char texto[100];
        sprintf(texto, "Linha %d", lin);
        m_buttontext.set_label(texto);
    }

// Salva texto depois de um tempo
    if (ListaMudou > 0)
        if (--ListaMudou==0)
            TextoLerSalvar(true);

// Comunicação com o servidor
    ProcConec();

    return true;
}

//---------------------------------------------------------------------------
bool TFrmMain::Conectar(int destino)
{
    const char * erro = "";
    std::string ender = FrmConfig->serv;
    int porta = atoi(FrmConfig->porta.c_str());
    StatusBar("Conectando...");
    Habilitar(false);
    if (sock_modo)
        Desconectar();
    while (1)
    {
        struct sockaddr_in conSock;
        struct hostent *hnome;
#ifdef __WIN32__
        // Inicializa WinSock
        WSADATA info;
        if (WSAStartup(MAKEWORD(1, 1), &info) == SOCKET_ERROR)
        {
            erro = "Não consegui acessar interface SOCKET";
            break;
        }
        memset(&conSock,0,sizeof(conSock));
        conSock.sin_addr.s_addr=inet_addr(ender.c_str());
        if ( conSock.sin_addr.s_addr == INADDR_NONE )
        {
            if ( (hnome=gethostbyname(ender.c_str())) == NULL )
            {
                erro = "Endereço do servidor inválido";
                WSACleanup();
                break;
            }
            conSock.sin_addr = (*(struct in_addr *)hnome->h_addr);
        }
        conSock.sin_family=AF_INET;
        conSock.sin_port=htons( (u_short)porta );
        if ( (sock_manip = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
        {
            erro = "Erro em socket()";
            WSACleanup();
            break;
        }
        if ( connect(sock_manip, (struct sockaddr *)&conSock,
                                          sizeof(struct sockaddr)) == SOCKET_ERROR)
        {
            erro = "Não consegui conectar";
            closesocket(sock_manip);
            WSACleanup();
            break;
        }
        unsigned long argp=1; // 0=bloquear  1=nao bloquear
        if ( ioctlsocket(sock_manip, FIONBIO, &argp) !=0 )
        {
            erro = "Problema em ioctlsocket()";
            closesocket(sock_manip);
            WSACleanup();
            break;
        }
#else
        memset(&conSock.sin_zero, 0, 8);
        conSock.sin_family=AF_INET;
        conSock.sin_port=htons(porta);
        conSock.sin_addr.s_addr=inet_addr(ender.c_str());
        if ( (conSock.sin_addr.s_addr) == (unsigned long)-1 )
        {
            if ( (hnome=gethostbyname(ender.c_str())) == NULL )
            {
                erro = "Endereço do servidor inválido";
                break;
            }
            conSock.sin_addr = (*(struct in_addr *)hnome->h_addr);
        }
        if ( (sock_manip=socket(PF_INET,SOCK_STREAM,0)) ==-1)
        {
            erro = "Erro em socket()";
            break;
        }
        //printf("Conectando em %s porta %d\n",
        //            inet_ntoa(conSock.sin_addr), porta);
        if ( connect(sock_manip, (struct sockaddr *)&conSock,
                                          sizeof(struct sockaddr))==-1)
        {
            erro = "Não consegui conectar";
            close(sock_manip);
            break;
        }
        fcntl(sock_manip, F_SETFL, O_NONBLOCK);
#endif
        StatusBar("Comunicando...");
        sock_modo = (destino ? 2 : 1);
        sock_penv = sock_env;
        sock_prec = sock_rec;
        // Passa para ISO8859-1
        Glib::ustring str = Glib::convert(sock_env, "iso8859-1", "utf-8");
        copiastr(sock_env, str.c_str(), 65000);
        ProcConec();
        return true;
    }

    StatusBar(Glib::ustring("Erro: ") + erro);
    Habilitar(true);
    return false;
}

//---------------------------------------------------------------------------
void TFrmMain::Desconectar()
{
    if (sock_modo==0)
        return;
#ifdef __WIN32__
    closesocket(sock_manip);
    WSACleanup();
#else
    close(sock_manip);
#endif
    Habilitar(true);
    sock_modo = 0;
    sock_penv = 0;
    sock_prec = 0;
}

//---------------------------------------------------------------------------
// Processa comunicação com o servidor
void TFrmMain::ProcConec()
{
    if (sock_modo==0)
        return;
// Envia mensagem
    while (*sock_penv)
    {
        int resposta;
        for (resposta=0; resposta<1024; resposta++)
            if (sock_penv[resposta]==0)
                break;
        resposta = send(sock_manip, sock_penv, resposta, 0);
#ifdef __WIN32__
        if (resposta==0)
            resposta=-1;
        else if (resposta==SOCKET_ERROR)
        {
            resposta=WSAGetLastError();
            resposta = (resposta==WSAEWOULDBLOCK ? 0 : -1);
        }
#else
        if (resposta<=0)
        {
            if (resposta<0 && (errno==EINTR || errno==EWOULDBLOCK || errno==ENOBUFS))
                resposta=0;
            else
                resposta=-1;
        }
#endif
        if (resposta > 0)
            sock_penv += resposta;
        if (resposta < 0)
        {
            StatusBar("Erro: Servidor terminou a conexão");
            Desconectar();
            return;
        }
        if (resposta != 1024)
            break;
    }
// Recebendo mensagem
    while (1)
    {
        char buf[4096];
        int resposta;
        resposta=recv(sock_manip, buf, sizeof(buf)-1, 0);
#ifdef __WIN32__
        if (resposta==0)
            resposta=-1;
        else if (resposta==SOCKET_ERROR)
        {
            resposta=WSAGetLastError();
            resposta = (resposta==WSAEWOULDBLOCK ? 0 : -1);
        }
#else
        if (resposta<=0)
        {
            if (resposta<0 && (errno==EINTR || errno==EWOULDBLOCK || errno==ENOBUFS))
                resposta=0;
            else
                resposta=-1;
        }
#endif
        if (resposta >= 0)
        {
            char *o=buf, *d=sock_prec;
            buf[resposta]=0;
            for (; *o && d<sock_rec+65090; o++)
                if (*(unsigned char*)o >= ' ' || *o==10)
                    *d++=*o;
            sock_prec = d;
            if (resposta == sizeof(buf)-1)
                continue;
            break;
        }
    // Verifica se mensagem foi enviada e recebida
        while (sock_prec > sock_rec && *(unsigned char*)(sock_prec-1)<' ')
            sock_prec--;
        if (*sock_penv)
        {
            StatusBar("Erro: Servidor terminou a conexão");
            Desconectar();
            break;
        }
        if (sock_prec == sock_rec)
        {
            StatusBar("Erro: Nenhuma resposta; senha incorreta?");
            Desconectar();
            break;
        }
    // sock_prec aponta para última linha recebida
        *sock_prec=0;
        while (sock_prec > sock_rec && *(unsigned char*)(sock_prec-1)>=' ')
            sock_prec--;
    // Se mensagem recebida começa com "-" ou sock_modo!=2...
    //      Uma linha:
    //          Status bar
    //          Fim
    //      Caso contrário:
    //          Escolhe janela de log
        const char * status = (*sock_rec=='-' ? "Erro" : "Feito");
        if (*sock_rec=='-' || sock_modo!=2)
        {
            if (sock_prec == sock_rec)
            {
                char * p = copiastr(buf, status, 100);
                *p++ = ':';
                *p++ = ' ';
                copiastr(p, sock_rec, 1024);
                StatusBar(buf);
                Desconectar();
                break;
            }
            Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
                    m_TreeView.get_selection();
            Gtk::TreeModel::iterator iter =
                    m_refTreeModel->children().begin();
            iter++;
            refTreeSelection->select(iter);
        }
    // Se mensagem recebida não começa com "-": deve terminar com "---"
        if (*sock_rec!='-')
        {
            if (strcmp(sock_prec, "---")!=0)
            {
                StatusBar("Erro: Mensagem recebida incompleta");
                Desconectar();
                break;
            }
            while (sock_prec > sock_rec && (unsigned char)sock_prec[-1]<' ')
                sock_prec--;
            *sock_prec = 0;
        }
        StatusBar(status);
    // Texto na caixa de texto
        m_textbuffer->set_text(ParaUTF8(sock_rec));
    // Indica texto alterado
        if (ListaMudou == 0)
            ListaMudou = 15;
        Desconectar();
        break;
    }
}

//---------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    Gtk::Main kit(argc, argv);
    TFrmConfig FrmConfig1;
    TFrmNomeClasse FrmNomeClasse1;
    TFrmMain FrmMain1;
    FrmConfig = &FrmConfig1;
    FrmNomeClasse = &FrmNomeClasse1;
    Gtk::Main::run(FrmMain1);
    return 0;
}
