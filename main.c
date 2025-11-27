// main.c - Sistema de Gestao (versao com stubs organizados, pronta pra compilar)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <ctype.h>
#include <conio.h>
#include <stdint.h>
#include <time.h>
/* ================== DEFINICOES GLOBAIS ================== */
#define MAX_NOME       100
#define MAX_CPF        20
#define MAX_EMAIL      80
#define MAX_DESC       200
#define MAX_COD_LOTE   32
#define MAX_TELEFONE   20
#define MAX_ENDERECO   150
#define MAX_ITENS_VENDA 20

/* ------------------ Estrutura de data simples ------------------ */
typedef struct {
    int dia;
    int mes;
    int ano;
} Data;

/* ------------------ Cabeçalho do arquivo (um por arquivo) ------------------ */
typedef struct {
    long count;   /* número total de registros (pode incluir inativos) */
    int last_id;  /* próximo id a usar (ou último usado + 1) */
} FileHeader;

/* ------------------ Entidades básicas ------------------ */

/* Cliente */
typedef struct {
    int id;                           /* id único */
    char nome[MAX_NOME];
    char cpf[MAX_CPF];                /* cpf ou cnpj, tamanho fixo */
    char telefone[MAX_TELEFONE];
    char email[MAX_EMAIL];
    char endereco[MAX_ENDERECO];
    int ativo;                        /* 1 = ativo, 0 = inativo/excluído */
} Cliente;

/* Tipo de produto (genérico, manipulados, etc.) */
typedef struct {
    int id;
    char nome[MAX_NOME];
    char descricao[MAX_DESC];
    int ativo;
} TipoProduto;



typedef struct {
    int id;
    char nome[MAX_NOME];
    char cnpj[MAX_CPF];
    char descricao[MAX_DESC];
    int ativo;
} Laboratorio;

/* Categoria (ex: Analgésicos, Vitaminas) */
typedef struct {
    int id;
    char nome[MAX_NOME];
    char descricao[MAX_DESC];
    int ativo;
} Categoria;

/* Produto (informações estáticas; estoque por lotes) */
typedef struct {
    int id;
    char nome[MAX_NOME];
    int id_tipo;            /* FK -> TipoProduto.id */
    int id_laboratorio;     /* FK -> Laboratorio.id */
    int id_categoria;       /* FK -> Categoria.id */
    char descricao[MAX_DESC];
    double preco_venda_padrao;
    int ativo;
} Produto;

/* Lote: controla validade, quantidade, preço por lote */
typedef struct {
    int id;
    int id_produto;                  // FK → Produto.id
    char codigo_lote[MAX_COD_LOTE]; // Código do lote (ex: "ABC1234")
    Data data_fabricacao;
    Data data_validade;
    int quantidade;                 // Quantidade atual no lote
    double preco_compra;            // Preço pago por unidade no lote
    double preco_venda;             // Preço de venda por unidade do lote
    int em_promocao;                // 0 = não, 1 = sim
    double desconto_percent;        // % de desconto (se em_promocao = 1)
    int ativo;                      // 1 = ativo, 0 = removido/inativo
} Lote;


/* Item usado nas estruturas de Venda/Compra */
typedef struct {
    int id_lote;    /* referência ao lote usado (venda) */
    int qtd;        /* quantidade vendida/comprada */
    double preco_unit; /* preco unitário aplicado */
} ItemVenda;

/* Venda (registro) */
typedef struct {
    int id;
    Data data;
    int id_cliente;          /* 0 para venda avulsa/cliente não cadastrado */
    int itens_count;         /* número de itens válidos no array abaixo */
    ItemVenda itens[MAX_ITENS_VENDA];
    double total;            /* soma já calculada para facilitar relatórios */
    int ativo;
} Venda;

/* Compra (registro simples) */
typedef struct {
    int id;
    Data data;
    int itens_count;
    ItemVenda itens[MAX_ITENS_VENDA];
    double total;
    int ativo;
} Compra;

/* Descarte / Movimentação (opcional para auditoria) */
typedef struct {
    int id;
    Data data;
    int id_lote;
    int qtd_descartada;
    char motivo[MAX_DESC];
} Descarte;

/* ----------------- Constantes de arquivo ----------------- */
#define ARQ_PRODUTOS "produtos.bin"
#define ARQ_LOTES    "lotes.bin"
#define ARQ_CLIENTES "clientes.bin"
#define ARQ_TIPOS    "tipos.bin"
#define ARQ_LABS     "laboratorios.bin"
#define ARQ_CATS     "categorias.bin"
#define ARQ_VENDAS   "vendas.bin"
#define ARQ_COMPRAS  "compras.bin"

/* ----------------- Prototipos (implemente em seguida) ----------------- */
/* Persistência / header */
int ensure_file_with_header(const char *filename);
int read_header(const char *filename, FileHeader *h);
int write_header(const char *filename, FileHeader *h);

/* Clientes */
void menuClientes();
void cadastrarCliente();
void editarCliente();
void removerCliente();
void listarClientes();
int buscarClientePorId(int id, Cliente *out, long *index);

/* Produtos & Lotes */
void menuProdutos();
int buscarProdutoPorId(int id, Produto *out, int *index);
void cadastrarProduto();
void editarProduto();
void removerProduto();
void listarProdutos();
void listarAtivarProdutosInativos();


void menuLotes();
void cadastrarLote();
void editarLote();
void listarLotes();
void descartarLote();
int buscarLotePorId(int id, Lote *out, int *index);
void removerLote();
void listarAtivarLotesInativos();
int validarDatasLote(Data fabricacao, Data validade);

/* Compras & Vendas */
void menuComprasVendas();
void efetuarCompra();
void efetuarVenda();
void listarVendas();
void listarCompras();

/* Relatórios */
void menuRelatorios();
void relatorioProdutosProximosVencimento();
void relatorioVendasPorPeriodo();
void relatorioEstoqueBaixo();

/* Utils (tipos, laboratorios, categorias) */
void menuUtils();
void menuTipos();
void menuLaboratorios();
void menuCategorias();


/* Tipos CRUD */
void cadastrarTipo();
void editarTipo();
void removerTipo();
void listarTipos();
int buscarTipoPorId(int id, TipoProduto *out, int *index);
int tipoJaExiste(const char *nome);
void limparTiposInativos();
void listarAtivarTiposInativos();

/* Laboratorios CRUD */
void cadastrarLaboratorio();
void editarLaboratorio();
void removerLaboratorio();
void listarLaboratorios();
int buscarLaboratorioPorId(int id, Laboratorio *out, int *index);
void listarAtivarLaboratoriosInativos();
int laboratorioJaExiste(const char *nome);
int laboratorioTemProdutosVinculados(int id_lab);

/* Categorias CRUD */
void cadastrarCategoria();
void editarCategoria();
void removerCategoria();
void listarCategorias();
int buscarCategoriaPorId(int id, Categoria *out, int *index);
void listarAtivarCategoriasInativas();
int categoriaExistePorNome(const char *nome);


/* Utilitários de I/O do menu */
void limparBuffer();
int lerInt(const char *prompt);
void lerString(const char *prompt, char *out, int maxlen);
int validarDataStruct(Data d);
void lerData(const char *mensagem, Data *saida);
void pauseAndContinue();

/* ----------------- Implementação das utilitárias do menu ----------------- */
void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

int lerInt(const char *prompt) {
    int x;
    char line[128];
    while (1) {
        printf("%s", prompt);
        if (!fgets(line, sizeof(line), stdin)) return 0;
        if (sscanf(line, "%d", &x) == 1) return x;
        printf("Entrada invalida. Tente novamente.\n");
    }
}

void lerString(const char *prompt, char *out, int maxlen) {
    printf("%s", prompt);
    if (fgets(out, maxlen, stdin) == NULL) {
        out[0] = '\0';
        return;
    }
    size_t ln = strlen(out);
    if (ln > 0 && out[ln-1] == '\n') out[ln-1] = '\0';
}

/* Ano bissexto */
int anoBissexto(int ano) {
    return ( (ano % 4 == 0 && ano % 100 != 0) || (ano % 400 == 0) );
}

/* Quantidade de dias no mês */
int diasNoMes(int mes, int ano) {
    if (mes < 1 || mes > 12) return 0;

    int dias[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (mes == 2 && anoBissexto(ano)) return 29;

    return dias[mes - 1];
}

/* Verifica se uma Data existe */
int dataValida(Data d) {
    if (d.ano < 1900 || d.ano > 2100) return 0;
    if (d.mes < 1 || d.mes > 12) return 0;
    if (d.dia < 1 || d.dia > diasNoMes(d.mes, d.ano)) return 0;
    return 1;
}

/* Comparar datas  
   - retorna -1 se a < b  
   - retorna  0 se igual  
   - retorna  1 se a > b  
*/
int compararDatas(Data a, Data b) {
    if (a.ano < b.ano) return -1;
    if (a.ano > b.ano) return 1;

    if (a.mes < b.mes) return -1;
    if (a.mes > b.mes) return 1;

    if (a.dia < b.dia) return -1;
    if (a.dia > b.dia) return 1;

    return 0;
}

/* Pega a data de hoje */
Data dataDeHoje() {
    time_t agora = time(NULL);
    struct tm *p = localtime(&agora);

    Data d;
    d.dia = p->tm_mday;
    d.mes = p->tm_mon + 1;
    d.ano = p->tm_year + 1900;
    return d;
}

void lerData(const char *mensagem, Data *saida) {
    char entrada[32];
    int d, m, a;

    while (1) {

        printf("%s (DD/MM/AAAA): ", mensagem);
        fgets(entrada, sizeof(entrada), stdin);

        /* remover \n */
        entrada[strcspn(entrada, "\n")] = '\0';

        /* checar formato via sscanf */
        if (sscanf(entrada, "%d/%d/%d", &d, &m, &a) != 3) {
            printf("Formato invalido! Use DD/MM/AAAA.\n");
            continue;
        }

        /* preencher struct temporariamente */
        Data temp;
        temp.dia = d;
        temp.mes = m;
        temp.ano = a;

        /* validar data real */
        if (!dataValida(temp)) {
            printf("Data inexistente! Verifique o dia/mes/ano.\n");
            continue;
        }

        /* se chegou aqui, está tudo certo */
        *saida = temp;
        return;
    }
}

int lerDataOpcional(const char *prompt, Data *out) {
    char linha[50];
    printf("%s", prompt);

    fgets(linha, sizeof(linha), stdin);

    /* ENTER → mantém valor */
    if (linha[0] == '\n')
        return 0;

    int d, m, a;
    if (sscanf(linha, "%d/%d/%d", &d, &m, &a) == 3) {
        out->dia = d; out->mes = m; out->ano = a;
        return 1; /* nova data informada */
    }

    printf("Formato invalido. Ignorando e mantendo valor atual.\n");
    return 0;
}



void pauseAndContinue() {
    printf("\nPressione ENTER para continuar...");
    limparBuffer();
}

/* ----------------- Menu principal ----------------- */
void menuPrincipal() {
    int opc;

    /* Garante que os arquivos binários existam com header inicial */
    ensure_file_with_header(ARQ_PRODUTOS);
    ensure_file_with_header(ARQ_LOTES);
    ensure_file_with_header(ARQ_CLIENTES);
    ensure_file_with_header(ARQ_TIPOS);
    ensure_file_with_header(ARQ_LABS);
    ensure_file_with_header(ARQ_CATS);
    ensure_file_with_header(ARQ_VENDAS);
    ensure_file_with_header(ARQ_COMPRAS);

    while (1) {
        system("cls");
        printf("\n=============================================\n");
        printf("      SISTEMA DE GESTAO - FARMACIA (PI)\n");
        printf("=============================================\n");
        printf("1) Clientes\n");
        printf("2) Produtos\n");
        printf("3) Lotes\n");
        printf("4) Compras (Entrada de Estoque)\n");
        printf("5) Vendas\n");
        printf("6) Relatorios\n");
        printf("7) Utils (Tipos / Laboratorios / Categorias)\n");
        printf("0) Sair\n");
        printf("---------------------------------------------\n");

        opc = lerInt("Escolha uma opcao: ");

        switch (opc) {
            case 1:
                menuClientes();
                break;
            case 2:
                menuProdutos();
                break;
            case 3:
                menuLotes();
                break;
            case 4:
                menuComprasVendas();
                break;
            case 5:
                menuComprasVendas();
                break;
            case 6:
                menuRelatorios();
                break;
            case 7:
                menuUtils();
                break;
            case 0:
                printf("Saindo... Obrigado.\n");
                return;
            default:
                printf("Opcao invalida. Tente novamente.\n");
                pauseAndContinue();
                break;
        }
    }
}

/* ----------------- Sub-menus (estruturas prontas) ----------------- */

/* Clientes */
void menuClientes() {
    int op;
    while (1) {
        system("cls");
        printf("\n=============================================\n");
        printf("      SISTEMA DE GESTAO - FARMACIA (PI)\n");
        printf("=============================================\n");
        printf("\n--- CLIENTES ---\n");
        printf("1) Cadastrar cliente\n");
        printf("2) Editar cliente\n");
        printf("3) Remover (marcar inativo)\n");
        printf("4) Listar clientes\n");
        printf("0) Voltar\n");
        op = lerInt("Opcao: ");
        switch (op) {
            case 1: cadastrarCliente(); break;
            case 2: editarCliente(); break;
            case 3: removerCliente(); break;
            case 4: listarClientes(); pauseAndContinue(); break;
            case 0: return;
            default: printf("Opcao invalida.\n"); pauseAndContinue(); break;
        }
    }
}

/* Produtos */
void menuProdutos() {
    int op;
    while (1) {
        system("cls");
        printf("\n=============================================\n");
        printf("      SISTEMA DE GESTAO - FARMACIA (PI)\n");
        printf("=============================================\n");
        printf("\n--- PRODUTOS ---\n");
        printf("1) Cadastrar produto\n");
        printf("2) Editar produto\n");
        printf("3) Remover produto\n");
        printf("4) Listar produtos\n");
        printf("5) Listar e restaurar [Registros Inativos]\n");
        printf("0) Voltar\n");
        op = lerInt("Opcao: ");
        switch (op) {
            case 1: cadastrarProduto(); break;
            case 2: editarProduto(); break;
            case 3: removerProduto(); break;
            case 4: listarProdutos(); pauseAndContinue(); break;
            case 5: listarAtivarProdutosInativos(); break;
            case 0: return;
            default: printf("Opcao invalida.\n"); pauseAndContinue(); break;
        }
    }
}

/* Lotes */
void menuLotes() {
    int op;
    while (1) {
        system("cls");
        printf("\n=============================================\n");
        printf("      SISTEMA DE GESTAO - FARMACIA (PI)\n");
        printf("=============================================\n");
        printf("\n--- LOTES ---\n");
        printf("1) Cadastrar lote (entrada manual)\n");
        printf("2) Editar lote\n");
        printf("3) Descartar lote (parcial ou total)\n");
        printf("4) Listar lotes\n");
        printf("5) Remover lote\n");
        printf("6) Listar e restaurar [Registros Inativos]\n");
        printf("0) Voltar\n");
        op = lerInt("Opcao: ");
        switch (op) {
            case 1: cadastrarLote(); break;
            case 2: editarLote(); break;
            case 3: descartarLote(); break;
            case 4: listarLotes(); break;
            case 5: removerLote(); break;
            case 6: listarAtivarLotesInativos();break;
            default: printf("Opcao invalida.\n"); pauseAndContinue(); break;
        }
    }
}

/* Compras e Vendas: submenu combinado */
void menuComprasVendas() {
    int op;
    while (1) {
        system("cls");
        printf("\n=============================================\n");
        printf("      SISTEMA DE GESTAO - FARMACIA (PI)\n");
        printf("=============================================\n");
        printf("\n--- COMPRAS & VENDAS ---\n");
        printf("1) Efetuar compra (gera/atualiza lotes)\n");
        printf("2) Listar compras\n");
        printf("3) Efetuar venda (consome lotes FEFO)\n");
        printf("4) Listar vendas\n");
        printf("0) Voltar\n");
        op = lerInt("Opcao: ");
        switch (op) {
            case 1: efetuarCompra(); break;
            case 2: listarCompras(); pauseAndContinue(); break;
            case 3: efetuarVenda(); break;
            case 4: listarVendas(); pauseAndContinue(); break;
            case 0: return;
            default: printf("Opcao invalida.\n"); pauseAndContinue(); break;
        }
    }
}

/* Relatórios */
void menuRelatorios() {
    int op;
    while (1) {
        system("cls");
        printf("\n=============================================\n");
        printf("      SISTEMA DE GESTAO - FARMACIA (PI)\n");
        printf("=============================================\n");
        printf("\n--- RELATORIOS ---\n");
        printf("1) Produtos proximos do vencimento\n");
        printf("2) Vendas por periodo\n");
        printf("3) Estoque baixo\n");
        printf("0) Voltar\n");
        op = lerInt("Opcao: ");
        switch (op) {
            case 1: relatorioProdutosProximosVencimento(); pauseAndContinue(); break;
            case 2: relatorioVendasPorPeriodo(); pauseAndContinue(); break;
            case 3: relatorioEstoqueBaixo(); pauseAndContinue(); break;
            case 0: return;
            default: printf("Opcao invalida.\n"); pauseAndContinue(); break;
        }
    }
}

/* Utils: tipos, laboratorios, categorias */
void menuUtils() {
    int op;

    while (1) {
        system("cls");
        printf("\n=============================================\n");
        printf("      SISTEMA DE GESTAO - FARMACIA (PI)\n");
        printf("=============================================\n");
        printf("--------------- MENU UTILITARIOS ---------------\n");
        printf("1) Tipos de Produto\n");
        printf("2) Laboratorios\n");
        printf("3) Categorias\n");
        
        printf("0) Voltar ao menu principal\n");
        printf("-----------------------------------------------\n");

        op = lerInt("Opcao: ");

        switch (op) {
            case 1: menuTipos(); break;
            case 2: menuLaboratorios(); break;
            case 3: menuCategorias(); break;
            
            case 0: return;
            default: printf("Opcao invalida!\n"); pauseAndContinue(); break;
        }
    }
}

/* Submenus de utilitarios */
void menuTipos() {
    int op;
    while (1) {
        system("cls");
        printf("\n--------------- CRUD - TIPOS ----------------\n");
        printf("1) Cadastrar Tipo\n");
        printf("2) Editar Tipo\n");
        printf("3) Remover Tipo\n");
        printf("4) Listar Tipos\n");
        //printf("5) Remover fisicamente [Registros Inativos]\n"); Funciona só que quebra todas ligações entre produtos.
        printf("5) Listar e restaurar [Registros Inativos]\n");
        printf("0) Voltar\n");
        op = lerInt("Opcao: ");
        switch (op) {
            case 1: cadastrarTipo(); break;
            case 2: editarTipo(); break;
            case 3: removerTipo(); break;
            case 4: listarTipos(); pauseAndContinue(); break;
            //case 5: limparTiposInativos(); break;
            case 5: listarAtivarTiposInativos(); break;
            case 0: return;
            default: printf("Opcao invalida!\n"); pauseAndContinue(); break;
        }
    }
}

void menuLaboratorios() {
    int op;
    while (1) {
        system("cls");
        printf("\n------------- CRUD - LABORATORIOS ------------\n");
        printf("1) Cadastrar Laboratorio\n");
        printf("2) Editar Laboratorio\n");
        printf("3) Remover Laboratorio\n");
        printf("4) Listar Laboratorios\n");
        printf("5) Listar e restaurar [Registros Inativos]\n");
        printf("0) Voltar\n");
        op = lerInt("Opcao: ");
        switch (op) {
            case 1: cadastrarLaboratorio(); break;
            case 2: editarLaboratorio(); break;
            case 3: removerLaboratorio(); break;
            case 4: listarLaboratorios(); pauseAndContinue(); break;
            case 5: listarAtivarLaboratoriosInativos(); break;
            case 0: return;
            default: printf("Opcao invalida!\n"); pauseAndContinue(); break;
        }
    }
}

void menuCategorias() {
    int op;
    while (1) {
        system("cls");
        printf("\n-------------- CRUD - CATEGORIAS -------------\n");
        printf("1) Cadastrar Categoria\n");
        printf("2) Editar Categoria\n");
        printf("3) Remover Categoria\n");
        printf("4) Listar Categorias\n");
        printf("5) Listar e restaurar [Registros Inativos]\n");
        printf("0) Voltar\n");
        op = lerInt("Opcao: ");
        switch (op) {
            case 1: cadastrarCategoria(); break;
            case 2: editarCategoria(); break;
            case 3: removerCategoria(); break;
            case 4: listarCategorias(); pauseAndContinue(); break;
            case 5: listarAtivarCategoriasInativas(); break;
            case 0: return;
            default: printf("Opcao invalida!\n"); pauseAndContinue(); break;
        }
    }
}

/* ----------------- Main que chama menuPrincipal ----------------- */
int main(void) {
    system("cls");
    printf("\n=============================================\n");
    printf("      SISTEMA DE GESTAO - FARMACIA (PI)\n");
    printf("=============================================\n");
    printf("Inicializando sistema...\n");
    pauseAndContinue();
    menuPrincipal();
    return 0;
}

/* ----------------- Implementação básica de header binário ----------------- */
int ensure_file_with_header(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (f) {
        fclose(f);
        return 0; // Já existe
    }

    // Se não existir, cria com header inicial
    f = fopen(filename, "wb");
    if (!f) {
        printf("Erro ao criar arquivo %s\n", filename);
        return -1;
    }

    FileHeader h;
    h.count = 0;   // nenhum registro ainda
    h.last_id = 1; // começamos IDs em 1
    fwrite(&h, sizeof(FileHeader), 1, f);
    fclose(f);
    return 0;
}

int read_header(const char *filename, FileHeader *h) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        // não imprime mensagem para evitar spam no menu
        return -1;
    }
    if (fread(h, sizeof(FileHeader), 1, f) != 1) {
        fclose(f);
        return -2;
    }
    fclose(f);
    return 0;
}

int write_header(const char *filename, FileHeader *h) {
    FILE *f = fopen(filename, "r+b"); // leitura + escrita binária
    if (!f) {
        printf("Erro ao abrir %s para atualizar header.\n", filename);
        return -1;
    }
    fseek(f, 0, SEEK_SET); // volta ao início do arquivo
    fwrite(h, sizeof(FileHeader), 1, f);
    fclose(f);
    return 0;
}

/* ----------------- Stubs padronizados (VERSAO A) ----------------- */
/* Clientes */
void cadastrarCliente() { printf("[STUB] cadastrarCliente()\n"); }
void editarCliente()    { printf("[STUB] editarCliente()\n"); }
void removerCliente()   { printf("[STUB] removerCliente()\n"); }
void listarClientes()   { printf("[STUB] listarClientes()\n"); }
int buscarClientePorId(int id, Cliente *out, long *index) { (void)id; (void)out; (void)index; return 1; }

/* Produtos */
void cadastrarProduto() {
    Produto p;
    printf("\n=== CADASTRAR PRODUTO ===\n");

    /* ------------------ Validar existência mínima ------------------ */
    FileHeader hTipos, hLabs, hCats;

    read_header(ARQ_TIPOS, &hTipos);
    read_header(ARQ_LABS, &hLabs);
    read_header(ARQ_CATS, &hCats);

    if (hTipos.count == 0) {
        printf("ERRO: Nenhum Tipo cadastrado!\n");
        pauseAndContinue();
        return;
    }
    if (hLabs.count == 0) {
        printf("ERRO: Nenhum Laboratorio cadastrado!\n");
        pauseAndContinue();
        return;
    }
    if (hCats.count == 0) {
        printf("ERRO: Nenhuma Categoria cadastrada!\n");
        pauseAndContinue();
        return;
    }

    /* ------------------ Dados gerais ------------------ */
    lerString("Nome do produto: ", p.nome, MAX_NOME);
    lerString("Descricao: ", p.descricao, MAX_DESC);


    /* ============================================================
       ====================== ESCOLHER TIPO ========================
       ============================================================ */

    int id_tipo = -1;
    TipoProduto t;
    char resp;

    printf("\nDeseja visualizar a lista de Tipos? (S/N): ");
    scanf(" %c", &resp);
    limparBuffer();

    if (resp == 'S' || resp == 's') {
        system("cls");
        listarTipos();
    }

    while (1) {
        id_tipo = lerInt("ID do Tipo (0 = cancelar): ");

        if (id_tipo == 0) {
            printf("Cadastro cancelado.\n");
            return;
        }

        if (buscarTipoPorId(id_tipo, &t, NULL) == 0)
            break;

        printf("Tipo inexistente!\n");

        printf("Deseja visualizar a lista novamente? (S/N): ");
        scanf(" %c", &resp);
        limparBuffer();

        if (resp == 'S' || resp == 's') {
            system("cls");
            listarTipos();
        }
    }

    p.id_tipo = id_tipo;


    /* ============================================================
       ==================== ESCOLHER LABORATORIO ==================
       ============================================================ */

    int id_lab = -1;
    Laboratorio L;

    printf("\nDeseja visualizar a lista de Laboratorios? (S/N): ");
    scanf(" %c", &resp);
    limparBuffer();

    if (resp == 'S' || resp == 's') {
        system("cls");
        listarLaboratorios();
    }

    while (1) {
        id_lab = lerInt("ID do Laboratorio (0 = cancelar): ");

        if (id_lab == 0) {
            printf("Cadastro cancelado.\n");
            return;
        }

        if (buscarLaboratorioPorId(id_lab, &L, NULL) == 0)
            break;

        printf("Laboratorio inexistente!\n");

        printf("Deseja visualizar a lista novamente? (S/N): ");
        scanf(" %c", &resp);
        limparBuffer();

        if (resp == 'S' || resp == 's') {
            system("cls");
            listarLaboratorios();
        }
    }

    p.id_laboratorio = id_lab;


    /* ============================================================
       ===================== ESCOLHER CATEGORIA ====================
       ============================================================ */

    int id_cat = -1;
    Categoria C;

    printf("\nDeseja visualizar a lista de Categorias? (S/N): ");
    scanf(" %c", &resp);
    limparBuffer();

    if (resp == 'S' || resp == 's') {
        system("cls");
        listarCategorias();
    }

    while (1) {
        id_cat = lerInt("ID da Categoria (0 = cancelar): ");

        if (id_cat == 0) {
            printf("Cadastro cancelado.\n");
            return;
        }

        if (buscarCategoriaPorId(id_cat, &C, NULL) == 0)
            break;

        printf("Categoria inexistente!\n");

        printf("Deseja visualizar a lista novamente? (S/N): ");
        scanf(" %c", &resp);
        limparBuffer();

        if (resp == 'S' || resp == 's') {
            system("cls");
            listarCategorias();
        }
    }

    p.id_categoria = id_cat;


    /* ------------------ Preço ------------------ */
    char buff[50];
    lerString("Preco de venda padrao: ", buff, sizeof(buff));
    p.preco_venda_padrao = atof(buff);
    p.ativo = 1;


    /* ------------------ Gravar ------------------ */
    FileHeader h;
    read_header(ARQ_PRODUTOS, &h);

    p.id = h.last_id++;
    h.count++;

    write_header(ARQ_PRODUTOS, &h);

    FILE *f = fopen(ARQ_PRODUTOS, "ab");
    fwrite(&p, sizeof(Produto), 1, f);
    fclose(f);

    printf("Produto cadastrado com ID %d!\n", p.id);
    pauseAndContinue();
}

int buscarProdutoPorId(int id, Produto *out, int *index) {
    FILE *f = fopen(ARQ_PRODUTOS, "rb");
    if (!f) return 1;

    FileHeader h;
    fread(&h, sizeof(FileHeader), 1, f);

    Produto p;
    int idx = 0;

    while (fread(&p, sizeof(Produto), 1, f) == 1) {
        if (p.id == id && p.ativo) {
            if (out) *out = p;
            if (index) *index = idx;
            fclose(f);
            return 0;
        }
        idx++;
    }

    fclose(f);
    return 1;
}

void editarProduto() {
    int id = lerInt("ID do produto que deseja editar: ");

    Produto p;
    int idx;

    if (buscarProdutoPorId(id, &p, &idx)) {
        printf("Produto nao encontrado!\n");
        return;
    }

    printf("\n--- PRODUTO ENCONTRADO ---\n");
    printf("Nome: %s\n", p.nome);
    printf("Descricao: %s\n", p.descricao);
    printf("Preco padrao: %.2f\n", p.preco_venda_padrao);

    printf("\nConfirmar edicao? (S/N): ");
    char r;
    scanf(" %c", &r);
    limparBuffer();
    if (r != 'S' && r != 's') return;

    // -------- Campos opcionais (ENTER mantém) --------
    char buff[MAX_DESC];

    lerString("Novo nome: ", buff, MAX_NOME);
    if (strlen(buff) > 0) strcpy(p.nome, buff);

    lerString("Nova descricao: ", buff, MAX_DESC);
    if (strlen(buff) > 0) strcpy(p.descricao, buff);

    lerString("Novo preco padrao: ", buff, MAX_DESC);
    if (strlen(buff) > 0) p.preco_venda_padrao = atof(buff);

    // -------- Salvar --------
    FILE *f = fopen(ARQ_PRODUTOS, "r+b");

    long offset = sizeof(FileHeader) + idx * sizeof(Produto);
    fseek(f, offset, SEEK_SET);
    fwrite(&p, sizeof(Produto), 1, f);

    fclose(f);

    printf("Produto atualizado!\n");
}

void removerProduto() {
    int id = lerInt("ID do produto que deseja remover: ");

    Produto p;
    int idx;

    if (buscarProdutoPorId(id, &p, &idx)) {
        printf("Produto nao encontrado!\n");
        return;
    }

    printf("Confirma remover (inativar) o produto '%s'? (S/N): ", p.nome);
    char r;
    scanf(" %c", &r);
    limparBuffer();

    if (r != 'S' && r != 's') return;

    p.ativo = 0;

    FILE *f = fopen(ARQ_PRODUTOS, "r+b");
    long offset = sizeof(FileHeader) + idx * sizeof(Produto);
    fseek(f, offset, SEEK_SET);
    fwrite(&p, sizeof(Produto), 1, f);
    fclose(f);

    printf("Produto removido.\n");
}

void listarProdutos() {
    FILE *f = fopen(ARQ_PRODUTOS, "rb");
    if (!f) {
        printf("Erro ao abrir arquivo de produtos.\n");
        pauseAndContinue();
        return;
    }

    FileHeader h;
    if (fread(&h, sizeof(FileHeader), 1, f) != 1) {
        printf("Arquivo de produtos corrompido.\n");
        fclose(f);
        pauseAndContinue();
        return;
    }

    Produto v[h.count];
    int q = 0;

    Produto p;
    while (fread(&p, sizeof(Produto), 1, f) == 1) {
        if (p.ativo)
            v[q++] = p;
    }

    fclose(f);

    /* ---- Bubble Sort por nome ---- */
    for (int i = 0; i < q - 1; i++) {
        for (int j = 0; j < q - 1 - i; j++) {
            if (strcmp(v[j].nome, v[j+1].nome) > 0) {
                Produto tmp = v[j];
                v[j] = v[j+1];
                v[j+1] = tmp;
            }
        }
    }

    /* ---- Cabeçalho ---- */
    printf("\n================================ LISTA DE PRODUTOS ================================\n");
    printf("%-5s | %-25s | %-20s | %-20s | %-20s | %-10s\n",
           "ID", "Produto", "Tipo", "Laboratorio", "Categoria", "Preco");
    printf("----------------------------------------------------------------------------------\n");

    /* ---- Linhas ---- */
    for (int i = 0; i < q; i++) {

        /* Buscar nomes */
        TipoProduto t;  
        Laboratorio L;
        Categoria C;

        char nomeTipo[25] = "N/D";
        char nomeLab[25]  = "N/D";
        char nomeCat[25]  = "N/D";

        if (!buscarTipoPorId(v[i].id_tipo, &t, NULL))
            strncpy(nomeTipo, t.nome, 24);

        if (!buscarLaboratorioPorId(v[i].id_laboratorio, &L, NULL))
            strncpy(nomeLab, L.nome, 24);

        if (!buscarCategoriaPorId(v[i].id_categoria, &C, NULL))
            strncpy(nomeCat, C.nome, 24);

        printf("%-5d | %-25s | %-20s | %-20s | %-20s | %-10.2f\n",
               v[i].id,
               v[i].nome,
               nomeTipo,
               nomeLab,
               nomeCat,
               v[i].preco_venda_padrao
        );
    }

    pauseAndContinue();
}

void listarAtivarProdutosInativos() {
    FILE *f = fopen(ARQ_PRODUTOS, "r+b");
    if (!f) {
        printf("Erro ao abrir arquivo de produtos.\n");
        pauseAndContinue();
        return;
    }

    FileHeader h;
    if (fread(&h, sizeof(FileHeader), 1, f) != 1) {
        printf("Arquivo de produtos corrompido ou vazio.\n");
        fclose(f);
        pauseAndContinue();
        return;
    }

    Produto p;
    int idx = 0;
    int encontrou = 0;

    printf("\n=========== PRODUTOS INATIVOS ===========\n");

    /* LISTAR TODOS OS INATIVOS */
    while (fread(&p, sizeof(Produto), 1, f) == 1) {
        if (p.ativo == 0) {
            encontrou = 1;
            printf("ID: %d | Nome: %s | Preco: %.2f\n",
                   p.id, p.nome, p.preco_venda_padrao);
        }
        idx++;
    }

    if (!encontrou) {
        printf("Nenhum produto desativado encontrado.\n");
        fclose(f);
        pauseAndContinue();
        return;
    }

    printf("\nO que deseja fazer?\n");
    printf("1) Ativar UM produto específico\n");
    printf("2) Ativar TODOS os produtos inativos\n");
    printf("0) Voltar sem ativar nada\n");

    int op = lerInt("Opcao: ");

    /* ----------- VOLTAR ----------- */
    if (op == 0) {
        fclose(f);
        return;
    }

    /* ----------- ATIVAR UM ESPECÍFICO ----------- */
    if (op == 1) {
        int id = lerInt("Informe o ID do produto a ativar: ");

        rewind(f);
        fread(&h, sizeof(FileHeader), 1, f);

        idx = 0;

        while (fread(&p, sizeof(Produto), 1, f) == 1) {

            if (p.id == id && p.ativo == 0) {
                p.ativo = 1;

                int pos = sizeof(FileHeader) + (int)idx * sizeof(Produto);

                fseek(f, pos, SEEK_SET);
                fwrite(&p, sizeof(Produto), 1, f);

                printf("Produto '%s' (ID %d) reativado com sucesso!\n", p.nome, p.id);
                fclose(f);
                pauseAndContinue();
                return;
            }

            idx++;
        }

        printf("ID informado nao corresponde a um produto inativo.\n");
        fclose(f);
        pauseAndContinue();
        return;
    }

    /* ----------- ATIVAR TODOS ----------- */
    if (op == 2) {
        rewind(f);
        fread(&h, sizeof(FileHeader), 1, f);

        idx = 0;
        int ativados = 0;

        while (fread(&p, sizeof(Produto), 1, f) == 1) {

            int posRegistro = sizeof(FileHeader) + (int)idx * sizeof(Produto);

            if (p.ativo == 0) {
                p.ativo = 1;

                fseek(f, posRegistro, SEEK_SET);
                fwrite(&p, sizeof(Produto), 1, f);

                /* Voltar ao fluxo do fread */
                fseek(f, posRegistro + sizeof(Produto), SEEK_SET);

                ativados++;
            }

            idx++;
        }

        printf("%d produto(s) reativado(s) com sucesso!\n", ativados);
        fclose(f);
        pauseAndContinue();
        return;
    }

    printf("Opcao invalida!\n");
    fclose(f);
    pauseAndContinue();
}

/* Lotes */
void cadastrarLote() {
    Lote L;
    printf("\n=== CADASTRAR LOTE ===\n");

    /* ------------------- 1) Verifica se existem produtos ------------------- */
    FileHeader hProd;
    read_header(ARQ_PRODUTOS, &hProd);

    if (hProd.count == 0) {
        printf("ERRO: Nenhum produto cadastrado!\n");
        pauseAndContinue();
        return;
    }

    /* ------------------- 2) Escolher Produto ------------------- */
    printf("Deseja ver a lista de produtos? <S/N>: ");
    char resp;
    scanf(" %c", &resp);
    limparBuffer();
    if (toupper(resp) == 'S')
        listarProdutos();

    Produto P;
    int idProd = -1;

    while (1) {

        idProd = lerInt("ID do produto (0 cancela): ");

        if (idProd == 0) {
            printf("Cadastro cancelado.\n");
            return;
        }

        if (!buscarProdutoPorId(idProd, &P, NULL)) {
            L.id_produto = idProd;
            break;   /* válido, sai do loop */
        }

        printf("Produto invalido. Deseja ver a lista novamente? <S/N>: ");
        char op;
        scanf(" %c", &op);
        limparBuffer();

        if (toupper(op) == 'S')
            listarProdutos();
    }

    /* ------------------- 3) Código do lote ------------------- */
    lerString("Codigo do lote: ", L.codigo_lote, MAX_COD_LOTE);

    /* ------------------- 4) Datas com validação ------------------- */
    int codErro;

    do {
        lerData("Data de fabricacao", &L.data_fabricacao);
        lerData("Data de validade", &L.data_validade);

        codErro = validarDatasLote(L.data_fabricacao, L.data_validade);
        if (codErro != 0) {
            switch (codErro)
            {
            case 1: printf("\nData invalida !\n");break;
            case 2: printf("\nFabricação não pode estar no futuro!\n");break;
            case 3: printf("\nValidade deve ser maior que fabricação!\n");break;
            case 4: printf("\nValidade não pode ser menor que data atual!\n");break;
            default:
                break;
            }
                                
            printf("Digite novamente as datas.\n\n");
        }

    } while (codErro != 0);

    /* ------------------- 5) Quantidade ------------------- */
    L.quantidade = lerInt("Quantidade inicial: ");
    if (L.quantidade < 0) L.quantidade = 0;

    /* ------------------- 6) Preços ------------------- */
    char buf[50];

    lerString("Preco de compra: ", buf, sizeof(buf));
    L.preco_compra = atof(buf);

    lerString("Preco de venda: ", buf, sizeof(buf));
    L.preco_venda = atof(buf);

    /* ------------------- 7) Promoção ------------------- */
    L.em_promocao = lerInt("Esta em promocao? (1=Sim, 0=Nao): ");
    if (L.em_promocao == 1) {
        lerString("Percentual de desconto (%): ", buf, sizeof(buf));
        L.desconto_percent = atof(buf);

        if (L.desconto_percent < 0)   L.desconto_percent = 0;
        if (L.desconto_percent > 100) L.desconto_percent = 100;

    } else {
        L.em_promocao = 0;
        L.desconto_percent = 0;
    }

    L.ativo = 1;

    /* ------------------- 8) Gravação ------------------- */
    FileHeader h;
    read_header(ARQ_LOTES, &h);

    L.id = h.last_id++;
    h.count++;

    write_header(ARQ_LOTES, &h);

    FILE *f = fopen(ARQ_LOTES, "ab");
    fwrite(&L, sizeof(Lote), 1, f);
    fclose(f);

    printf("Lote cadastrado com ID %d!\n", L.id);
    pauseAndContinue();
}

void editarLote() {
    int id = lerInt("ID do lote a editar: ");

    Lote L;
    int idx;

    if (buscarLotePorId(id, &L, &idx)) {
        printf("Lote nao encontrado!\n");
        pauseAndContinue();
        return;
    }

    printf("\n=== EDITANDO LOTE ===\n");
    printf("Produto ID: %d\n", L.id_produto);
    printf("Codigo atual: %s\n", L.codigo_lote);
    printf("Quantidade atual: %d\n", L.quantidade);
    printf("Fabricacao atual: %02d/%02d/%04d\n",
           L.data_fabricacao.dia, L.data_fabricacao.mes, L.data_fabricacao.ano);
    printf("Validade atual:   %02d/%02d/%04d\n",
           L.data_validade.dia, L.data_validade.mes, L.data_validade.ano);
    printf("Preco compra: %.2f\n", L.preco_compra);
    printf("Preco venda : %.2f\n", L.preco_venda);
    printf("Desconto: %.2f%%\n", L.desconto_percent);
    printf("\n(ENTER mantém valor)\n\n");

    char buf[100];

    /* ------------------ Código ------------------ */
    lerString("Novo codigo: ", buf, sizeof(buf));
    if (strlen(buf) > 0)
        strcpy(L.codigo_lote, buf);

    /* ------------------ Quantidade ------------------ */
    lerString("Nova quantidade: ", buf, sizeof(buf));
    if (strlen(buf) > 0) {
        int q = atoi(buf);
        if (q >= 0) L.quantidade = q;
    }

    /* ------------------ Data de fabricação ------------------ */
    Data novaFab;
    int hasFab = lerDataOpcional("Nova data de fabricacao (ENTER = atual): ", &novaFab);

    /* ------------------ Data de validade ------------------ */
    Data novaVal;
    int hasVal = lerDataOpcional("Nova data de validade (ENTER = atual): ", &novaVal);

    /* Se usuário digitou alguma, validar conjunto */
    if (hasFab || hasVal) {

        Data fabFinal = hasFab ? novaFab : L.data_fabricacao;
        Data valFinal = hasVal ? novaVal : L.data_validade;

        char msg[100];
        int codErro;

        codErro = validarDatasLote(L.data_fabricacao, L.data_validade);
        if (codErro != 0) {
            switch (codErro)
            {
            case 1: printf("\nData invalida !\n");break;
            case 2: printf("\nFabricação não pode estar no futuro!\n");break;
            case 3: printf("\nValidade deve ser maior que fabricação!\n");break;
            case 4: printf("\nValidade não pode ser menor que data atual!\n");break;
            default:
                break;
            }
                                
            printf("Digite novamente as datas.\n\n");
        }

        /* Datas válidas → aplicar */
        L.data_fabricacao = fabFinal;
        L.data_validade = valFinal;
    }

    /* ------------------ Preço compra ------------------ */
    lerString("Novo preco de compra: ", buf, sizeof(buf));
    if (strlen(buf) > 0) {
        double x = atof(buf);
        if (x >= 0) L.preco_compra = x;
    }

    /* ------------------ Preço venda ------------------ */
    lerString("Novo preco de venda: ", buf, sizeof(buf));
    if (strlen(buf) > 0) {
        double x = atof(buf);
        if (x >= 0) L.preco_venda = x;
    }

    /* ------------------ Desconto / Promoção ------------------ */
    lerString("Novo desconto (%): ", buf, sizeof(buf));
    if (strlen(buf) > 0) {
        double d = atof(buf);
        if (d < 0) d = 0;
        if (d > 100) d = 100;
        L.desconto_percent = d;
        L.em_promocao = (d > 0);
    }

    /* ------------------ Atualizar no arquivo ------------------ */
    FILE *f = fopen(ARQ_LOTES, "r+b");
    if (!f) {
        printf("ERRO ao abrir arquivo de lotes!\n");
        pauseAndContinue();
        return;
    }

    long pos = sizeof(FileHeader) + (long)idx * sizeof(Lote);
    fseek(f, pos, SEEK_SET);
    fwrite(&L, sizeof(Lote), 1, f);
    fclose(f);

    printf("\nLote atualizado com sucesso!\n");
    pauseAndContinue();
}

void listarLotes() {
    FILE *f = fopen(ARQ_LOTES, "rb");
    if (!f) {
        printf("Nenhum lote cadastrado.\n");
        pauseAndContinue();
        return;
    }

    FileHeader h;
    fread(&h, sizeof(FileHeader), 1, f);

    Lote L;
    Produto P;

    printf("\n=== LISTA DE LOTES ===\n");
    printf("%-5s %-20s %-12s %-12s %-10s %-10s %-5s\n",
           "ID", "Produto", "Fabricacao", "Validade", "Qtd", "P.Venda", "%Desc");

    while (fread(&L, sizeof(Lote), 1, f) == 1) {

        if (!L.ativo) continue;

        if (buscarProdutoPorId(L.id_produto, &P, NULL) != 0)
            continue;

        printf("%-5d %-20s %02d/%02d/%04d %02d/%02d/%04d %-10d %-10.2f %-5.1f\n",
               L.id, P.nome,
               L.data_fabricacao.dia, L.data_fabricacao.mes, L.data_fabricacao.ano,
               L.data_validade.dia, L.data_validade.mes, L.data_validade.ano,
               L.quantidade,
               L.preco_venda,
               L.em_promocao ? L.desconto_percent : 0.0
        );
    }

    fclose(f);
    pauseAndContinue();
}

void descartarLote()    { printf("[STUB] descartarLote()\n"); }

int buscarLotePorId(int id, Lote *out, int *index) {
    FILE *f = fopen(ARQ_LOTES, "rb");
    if (!f) return 1;

    fseek(f, sizeof(FileHeader), SEEK_SET);

    Lote L;
    int idx = 0;

    while (fread(&L, sizeof(Lote), 1, f) == 1) {
        if (L.id == id && L.ativo) {
            if (out) *out = L;
            if (index) *index = idx;
            fclose(f);
            return 0;
        }
        idx++;
    }

    fclose(f);
    return 1;
}

void removerLote() {
    int id = lerInt("ID do lote a remover: ");

    Lote L;
    int idx;

    if (buscarLotePorId(id, &L, &idx)) {
        printf("Lote nao encontrado!\n");
        pauseAndContinue();
        return;
    }

    printf("Deseja remover este lote? (S/N): ");
    char r;
    scanf(" %c", &r);
    limparBuffer();

    if (r != 'S' && r != 's') return;

    L.ativo = 0;

    FILE *f = fopen(ARQ_LOTES, "r+b");
    long pos = sizeof(FileHeader) + idx * sizeof(Lote);
    fseek(f, pos, SEEK_SET);
    fwrite(&L, sizeof(Lote), 1, f);
    fclose(f);

    printf("Lote removido!\n");
    pauseAndContinue();
}

void listarAtivarLotesInativos() {
    FILE *f = fopen(ARQ_LOTES, "r+b");
    if (!f) {
        printf("Arquivo nao encontrado.\n");
        pauseAndContinue();
        return;
    }

    FileHeader h;
    fread(&h, sizeof(FileHeader), 1, f);

    Lote L;
    int idx = 0;
    int encontrou = 0;

    printf("\n=== LOTES INATIVOS ===\n");

    while (fread(&L, sizeof(Lote), 1, f) == 1) {
        if (L.ativo == 0) {
            encontrou = 1;
            printf("ID %d | ProdID: %d | Qtd: %d\n", L.id, L.id_produto, L.quantidade);
        }
        idx++;
    }

    if (!encontrou) {
        printf("Nenhum lote inativo.\n");
        fclose(f);
        pauseAndContinue();
        return;
    }

    printf("\n1) Ativar um\n");
    printf("2) Ativar todos\n");
    printf("0) Voltar\n");

    int op = lerInt("Opcao: ");

    if (op == 0) { fclose(f); return; }

    /* ================= ATIVAR UM ================= */
    if (op == 1) {
        int id = lerInt("ID do lote: ");

        rewind(f);
        fread(&h, sizeof(FileHeader), 1, f);

        idx = 0;

        while (fread(&L, sizeof(Lote), 1, f) == 1) {

            if (L.id == id && L.ativo == 0) {
                L.ativo = 1;

                long pos = sizeof(FileHeader) + idx * sizeof(Lote);
                fseek(f, pos, SEEK_SET);

                fwrite(&L, sizeof(Lote), 1, f);
                fflush(f);

                printf("Lote reativado!\n");
                fclose(f);
                pauseAndContinue();
                return;
            }

            idx++;
        }

        printf("ID nao pertence a lote inativo.\n");
        fclose(f);
        pauseAndContinue();
        return;
    }

    /* ================= ATIVAR TODOS ================= */
    if (op == 2) {

        rewind(f);
        fread(&h, sizeof(FileHeader), 1, f);

        idx = 0;
        int ativados = 0;

        while (fread(&L, sizeof(Lote), 1, f) == 1) {

            if (!L.ativo) {

                L.ativo = 1;

                long pos = sizeof(FileHeader) + idx * sizeof(Lote);
                fseek(f, pos, SEEK_SET);
                fwrite(&L, sizeof(Lote), 1, f);
                fflush(f);

                /* VOLTA PARA POSIÇÃO DE LEITURA CORRETA (!)
                   avança o cursor para depois do lote atual */
                fseek(f, sizeof(FileHeader) + (idx+1) * sizeof(Lote), SEEK_SET);

                ativados++;
            }

            idx++;
        }

        printf("%d lotes reativados!\n", ativados);
        fclose(f);
        pauseAndContinue();
        return;
    }

    printf("Opcao invalida.\n");
    fclose(f);
    pauseAndContinue();
}


int validarDatasLote(Data fabricacao, Data validade) {

    Data hoje = dataDeHoje();

    /* 1) Verificar se datas existem */
    if (!dataValida(fabricacao) || !dataValida(validade))
        return 1;

    /* 2) Fabricação não pode estar no futuro */
    if (compararDatas(fabricacao, hoje) == 1)
        return 2;

    /* 3) Validade deve ser estritamente maior que fabricação */
    if (compararDatas(validade, fabricacao) <= 0)
        return 3;

    /* 4) Validade não pode ser menor que hoje (lote já vencido) */
    if (compararDatas(validade, hoje) == -1)
        return 4;

    return 0; /* tudo OK */
}

/* Mensagem de erro amigável */
void mostrarErroDataLote(int erro) {
    switch (erro) {
        case 1: printf("Data invalida.\n"); break;
        case 2: printf("A data de fabricacao nao pode estar no futuro.\n"); break;
        case 3: printf("A data de validade deve ser maior que a data de fabricacao.\n"); break;
        case 4: printf("A data de validade ja passou.\n"); break;
        default: printf("Erro desconhecido.\n");
    }
}

/* Compras & Vendas */
void efetuarCompra()    { printf("[STUB] efetuarCompra()\n"); }
void efetuarVenda()     { printf("[STUB] efetuarVenda()\n"); }
void listarVendas()     { printf("[STUB] listarVendas()\n"); }
void listarCompras()    { printf("[STUB] listarCompras()\n"); }

/* Relatórios */
void relatorioProdutosProximosVencimento() { printf("[STUB] relatorioProdutosProximosVencimento()\n"); }
void relatorioVendasPorPeriodo()            { printf("[STUB] relatorioVendasPorPeriodo()\n"); }
void relatorioEstoqueBaixo()                { printf("[STUB] relatorioEstoqueBaixo()\n"); }

/* Tipos */
void cadastrarTipo() { 
    TipoProduto t;

    printf("\n--- CADASTRAR TIPO ---\n");
    lerString("Nome: ", t.nome, MAX_NOME);

    if (tipoJaExiste(t.nome)) {
        printf("ERRO: Ja existe um tipo cadastrado com esse nome!\n");
        pauseAndContinue();
        return;
    }

    lerString("Descricao: ", t.descricao, MAX_DESC);

    FileHeader h;
    read_header(ARQ_TIPOS, &h);

    t.id = h.last_id++;
    t.ativo = 1;

    /* 👇 CORREÇÃO IMPORTANTE */
    h.count++;   // <-- INCREMENTA O CONTADOR DE REGISTROS

    // ------------------ SALVAR ------------------
    FILE *f = fopen(ARQ_TIPOS, "r+b");
    fseek(f, 0, SEEK_SET);
    fwrite(&h, sizeof(FileHeader), 1, f);

    fseek(f, 0, SEEK_END);
    fwrite(&t, sizeof(TipoProduto), 1, f);

    fclose(f);

    printf("Tipo cadastrado com ID %d!\n", t.id);
    pauseAndContinue();
}


void editarTipo() {
    int id = lerInt("ID do tipo que deseja editar: ");

    TipoProduto t;
    int idx;

    if (buscarTipoPorId(id, &t, &idx)) {
        printf("Tipo nao encontrado!\n");
        return;
    }

    // ------------------ MOSTRAR REGISTRO ENCONTRADO ------------------
    printf("\n--- REGISTRO ENCONTRADO ---\n");
    printf("ID: %d\n", t.id);
    printf("Nome atual: %s\n", t.nome);
    printf("Descricao atual: %s\n", t.descricao);

    // Confirmação
    printf("\nDeseja realmente editar este tipo? (S/N): ");
    char resp;
    scanf(" %c", &resp);
    limparBuffer(); // limpar ENTER

    if (resp != 'S' && resp != 's') {
        printf("Edicao cancelada.\n");
        return;
    }

    // ------------------ EDIÇÃO COM CAMPOS OPCIONAIS ------------------
    char bufferNome[MAX_NOME];
    char bufferDesc[MAX_DESC];

    printf("\nPressione ENTER para manter o valor atual.\n");

    // Nome
    lerString("Novo nome: ", bufferNome, MAX_NOME);
    if (strlen(bufferNome) > 0) {
        strcpy(t.nome, bufferNome);
    }

    // Descrição
    lerString("Nova descricao: ", bufferDesc, MAX_DESC);
    if (strlen(bufferDesc) > 0) {
        strcpy(t.descricao, bufferDesc);
    }

    // ------------------ SALVAR ------------------
    FILE *f = fopen(ARQ_TIPOS, "r+b");
    if (!f) {
        printf("Erro ao abrir arquivo de tipos!\n");
        pauseAndContinue();
        return;
    }

    long pos = sizeof(FileHeader) + idx * sizeof(TipoProduto);
    fseek(f, pos, SEEK_SET);
    fwrite(&t, sizeof(TipoProduto), 1, f);

    fclose(f);

    printf("\nTipo atualizado com sucesso!\n");
    pauseAndContinue();
}

void removerTipo() {
    int id = lerInt("ID do tipo a remover: ");

    TipoProduto t;
    int idx;

    if (buscarTipoPorId(id, &t, &idx)) {
        printf("Tipo nao encontrado!\n");
        pauseAndContinue();
        return;
    }

    // ------------------ MOSTRAR REGISTRO ENCONTRADO ------------------
    printf("\n--- REGISTRO ENCONTRADO ---\n");
    printf("ID: %d\n", t.id);
    printf("Nome atual: %s\n", t.nome);
    printf("Descricao atual: %s\n", t.descricao);

    // Confirmação
    printf("\nDeseja realmente excluir(logicamente) este tipo? (S/N): ");
    char resp;
    scanf(" %c", &resp);
    limparBuffer(); // limpar ENTER

    if (resp != 'S' && resp != 's') {
        printf("Exclusao cancelada.\n");
        pauseAndContinue();
        return;
    }

    t.ativo = 0;

    // ------------------ SALVAR ------------------
    FILE *f = fopen("tipos.bin", "r+b");

    long offset = sizeof(FileHeader) + idx * sizeof(TipoProduto);
    fseek(f, offset, SEEK_SET);
    fwrite(&t, sizeof(TipoProduto), 1, f);

    fclose(f);

    printf("Tipo removido!\n");
    pauseAndContinue();
}

void listarTipos() {
    FileHeader h;
    if (read_header(ARQ_TIPOS, &h) != 0) {
        printf("Erro ao ler header de tipos.\n");
        return;
    }

    FILE *f = fopen(ARQ_TIPOS, "rb");
    if (!f) {
        printf("Arquivo de tipos nao encontrado.\n");
        return;
    }

    fseek(f, sizeof(FileHeader), SEEK_SET);

    TipoProduto t;

    printf("\n==================== LISTA DE TIPOS ====================\n");
    printf("%-5s | %-25s | %-40s\n", "ID", "Nome", "Descricao");
    printf("--------------------------------------------------------------------------\n");

    while (fread(&t, sizeof(TipoProduto), 1, f) == 1) {
        if (t.ativo) {
            printf("%-5d | %-25s | %-40s\n",
                   t.id,
                   t.nome,
                   t.descricao);
        }
    }

    fclose(f);
}

int buscarTipoPorId(int id, TipoProduto *tipoSaida, int *index) {
    FILE *f = fopen("tipos.bin", "rb");
    if (!f) return 1;

    fseek(f, sizeof(FileHeader), SEEK_SET);

    TipoProduto t;
    int idx = 0;

    while (fread(&t, sizeof(TipoProduto), 1, f) == 1) {
        if (t.id == id && t.ativo) {
            if (tipoSaida) *tipoSaida = t;//Verifica se não é NULL e depois copia o registro.
            if (index) *index = idx;//Verifica se não é NULL e depois copia o index do registro.
            fclose(f);
            return 0;
        }
        idx++;
    }

    fclose(f);
    return 1;
}

int tipoJaExiste(const char *nome) {
    FILE *f = fopen(ARQ_TIPOS, "rb");
    if (!f) return 0;

    // Pula o header
    fseek(f, sizeof(FileHeader), SEEK_SET);

    TipoProduto t;
    while (fread(&t, sizeof(TipoProduto), 1, f) == 1) {
        if (t.ativo && stricmp(t.nome, nome) == 0) {
            fclose(f);
            return 1; // Já existe
        }
    }

    fclose(f);
    return 0; // Não existe
}

void limparTiposInativos() {
    FILE *f = fopen(ARQ_TIPOS, "rb");
    if (!f) {
        printf("Erro ao abrir o arquivo de tipos.\n");
        pauseAndContinue();
        return;
    }

    FileHeader h;
    fread(&h, sizeof(FileHeader), 1, f);

    // Arquivo temporário onde gravaremos apenas registros ativos
    FILE *temp = fopen("tipos_temp.bin", "wb");
    if (!temp) {
        fclose(f);
        printf("Erro ao criar arquivo temporario.\n");
        pauseAndContinue();
        return;
    }

    // Novo header (mesmo last_id, mas count será recalculado)
    FileHeader novo;
    novo.last_id = h.last_id;
    novo.count = 0;  // vamos recontar apenas ativos
    fwrite(&novo, sizeof(FileHeader), 1, temp);

    TipoProduto t;
    long ativos = 0;

    // Copiar apenas os tipos ativos
    while (fread(&t, sizeof(TipoProduto), 1, f) == 1) {
        if (t.ativo == 1) {
            fwrite(&t, sizeof(TipoProduto), 1, temp);
            ativos++;
        }
    }

    // Atualizar header do arquivo temporário
    fseek(temp, 0, SEEK_SET);
    novo.count = ativos;
    fwrite(&novo, sizeof(FileHeader), 1, temp);

    fclose(f);
    fclose(temp);

    // Substituir o arquivo original pelo novo
    remove(ARQ_TIPOS);
    rename("tipos_temp.bin", ARQ_TIPOS);

    printf("Limpeza concluida! %ld registros ativos mantidos.\n", ativos);
    pauseAndContinue();
}

void listarAtivarTiposInativos() {
    FILE *f = fopen(ARQ_TIPOS, "r+b");
    if (!f) {
        printf("Erro ao abrir arquivo de tipos.\n");
        pauseAndContinue();
        return;
    }

    FileHeader h;
    fread(&h, sizeof(FileHeader), 1, f);

    TipoProduto t;
    int idx = 0;
    int encontrou = 0;

    printf("\n--- TIPOS INATIVOS ---\n");

    // Primeiro: LISTAR os inativos
    while (fread(&t, sizeof(TipoProduto), 1, f) == 1) {
        if (t.ativo == 0) {
            encontrou = 1;
            printf("ID: %d | Nome: %s | Descricao: %s\n", t.id, t.nome, t.descricao);
        }
        idx++;
    }

    if (!encontrou) {
        printf("Nenhum tipo inativo encontrado.\n");
        pauseAndContinue();
        fclose(f);
        return;
    }

    printf("\nO que deseja fazer?\n");
    printf("1) Ativar UM tipo específico\n");
    printf("2) Ativar TODOS os inativos\n");
    printf("0) Não ativar nada (voltar)\n");

    int op = lerInt("Opcao: ");

    if (op == 0) {
        fclose(f);
        return;
    }

    // ---------------------------------------
    // ATIVAR UM REGISTRO ESPECÍFICO
    // ---------------------------------------
    if (op == 1) {
        int id = lerInt("Informe o ID do tipo a ativar: ");

        rewind(f);
        fread(&h, sizeof(FileHeader), 1, f);

        idx = 0;
        while (fread(&t, sizeof(TipoProduto), 1, f) == 1) {
            if (t.id == id && t.ativo == 0) {

                t.ativo = 1;

                long offset = sizeof(FileHeader) + idx * sizeof(TipoProduto);
                fseek(f, offset, SEEK_SET);
                fwrite(&t, sizeof(TipoProduto), 1, f);

                printf("Tipo ID %d reativado!\n", id);
                pauseAndContinue();
                fclose(f);
                return;
            }
            idx++;
        }

        printf("ID informado nao corresponde a um tipo inativo.\n");
        pauseAndContinue();
        fclose(f);
        return;
    }

    // ---------------------------------------
    // ATIVAR TODOS
    // ---------------------------------------
    if (op == 2) {
        rewind(f);
        fread(&h, sizeof(FileHeader), 1, f);

        idx = 0;
        int reativados = 0;

        while (fread(&t, sizeof(TipoProduto), 1, f) == 1) {
            if (t.ativo == 0) {
                t.ativo = 1;
                int pos = sizeof(FileHeader) + idx * sizeof(TipoProduto);
                fseek(f, pos, SEEK_SET);
                fwrite(&t, sizeof(TipoProduto), 1, f);
                reativados++;
            }
            idx++;
        }

        printf("%d registros reativados com sucesso!\n", reativados);
        pauseAndContinue();
        fclose(f);
        return;
    }

    // ---------------------------------------
    // QUALQUER OPÇÃO INVALIDA
    // ---------------------------------------
    printf("Opcao invalida.\n");
    fclose(f);
}


/* Laboratorios */
int validarCNPJ(const char *cnpj) {
    int i, j, dig1, dig2, soma, peso;
    char num[15]; // 14 dígitos + '\0'

    // remover tudo que nao for digito
    int k = 0;
    for (i = 0; cnpj[i] != '\0' && k < 14; i++) {
        if (isdigit(cnpj[i])) {
            num[k++] = cnpj[i];
        }
    }
    num[k] = '\0';

    // precisa ter exatamente 14 dígitos
    if (strlen(num) != 14) return 0;

    // rejeitar CNPJs como 00000000000000, 11111111111111 etc.
    for (i = 1; i < 14; i++) {
        if (num[i] != num[0]) break;
    }
    if (i == 14) return 0;

    // -------- Validação dígito 1 --------
    soma = 0;
    peso = 5;

    for (i = 0; i < 12; i++) {
        soma += (num[i] - '0') * peso;
        peso--;
        if (peso < 2) peso = 9;
    }

    dig1 = 11 - (soma % 11);
    if (dig1 > 9) dig1 = 0;

    if (dig1 != (num[12] - '0')) return 0;

    // -------- Validação dígito 2 --------
    soma = 0;
    peso = 6;

    for (i = 0; i < 13; i++) {
        soma += (num[i] - '0') * peso;
        peso--;
        if (peso < 2) peso = 9;
    }

    dig2 = 11 - (soma % 11);
    if (dig2 > 9) dig2 = 0;

    if (dig2 != (num[13] - '0')) return 0;

    return 1; // CNPJ válido
}

int cnpjJaExiste(const char *cnpj) {
    FILE *f = fopen(ARQ_LABS, "rb");
    if (!f) return 0;

    fseek(f, sizeof(FileHeader), SEEK_SET);

    Laboratorio L;
    while (fread(&L, sizeof(Laboratorio), 1, f) == 1) {
        if (L.ativo && strcmp(L.cnpj, cnpj) == 0) {
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}

void cadastrarLaboratorio() {
    Laboratorio L;
    printf("\n--- CADASTRAR LABORATORIO ---\n");

    int nomeValido = 0;
    while (!nomeValido) {
        lerString("Nome: ", L.nome, MAX_NOME);
        if (strlen(L.nome) == 0)
            printf("Nome nao pode ser vazio!\n");
        else if (laboratorioJaExiste(L.nome))
            printf("Erro: ja existe um laboratorio com este nome.\n");
        else
            nomeValido = 1;
    }

    int cnpjValido = 0;
    while (!cnpjValido) {
        lerString("CNPJ: ", L.cnpj, MAX_CPF);

        if (!validarCNPJ(L.cnpj))
            printf("CNPJ INVALIDO!\n");
        else if (cnpjJaExiste(L.cnpj))
            printf("Erro: CNPJ já cadastrado.\n");
        else
            cnpjValido = 1;
    }

    lerString("Descricao: ", L.descricao, MAX_DESC);

    FileHeader h;
    read_header(ARQ_LABS, &h);

    L.id = h.last_id++;
    L.ativo = 1;

    h.count++;

    write_header(ARQ_LABS, &h);

    FILE *f = fopen(ARQ_LABS, "ab");
    fwrite(&L, sizeof(Laboratorio), 1, f);
    fclose(f);

    printf("Laboratorio cadastrado com ID %d!\n", L.id);
    pauseAndContinue();
}


void listarLaboratorios() {
    FILE *f = fopen(ARQ_LABS, "rb");
    if (!f) {
        printf("Arquivo de laboratorios nao encontrado.\n");
        pauseAndContinue();
        return;
    }

    fseek(f, sizeof(FileHeader), SEEK_SET);
    Laboratorio L;

    printf("\n================ LISTA DE LABORATORIOS ================\n");
    printf("%-5s | %-25s | %-18s | %-30s\n", "ID", "Nome", "CNPJ", "Descricao");
    printf("---------------------------------------------------------------"
           "-------------------------------------------\n");

    while (fread(&L, sizeof(Laboratorio), 1, f) == 1) {
        if (L.ativo) {
            printf("%-5d | %-25s | %-18s | %-30s\n",
                   L.id,
                   L.nome,
                   L.cnpj,
                   L.descricao);
        }
    }

    fclose(f);
}

void editarLaboratorio() {
    int id = lerInt("ID do laboratorio que deseja editar: ");

    Laboratorio L;
    int idx;
    if (buscarLaboratorioPorId(id, &L, &idx)) {
        printf("Laboratorio nao encontrado!\n");
        pauseAndContinue();
        return;
    }

    printf("\n--- REGISTRO ENCONTRADO ---\n");
    printf("ID: %d\n", L.id);
    printf("Nome atual: %s\n", L.nome);
    printf("CNPJ atual: %s\n", L.cnpj);
    printf("Descricao atual: %s\n", L.descricao);

    printf("\nDeseja realmente editar este laboratorio? (S/N): ");
    char resp;
    scanf(" %c", &resp);
    limparBuffer();
    if (resp != 'S' && resp != 's') {
        printf("Edicao cancelada.\n");
        pauseAndContinue();
        return;
    }

    char bufferNome[MAX_NOME];
    char bufferCNPJ[MAX_CPF];
    char bufferDesc[MAX_DESC];

    printf("Pressione ENTER para manter o valor atual.\n");

    lerString("Novo nome: ", bufferNome, MAX_NOME);
    /* se o usuário digitou um novo nome, checar duplicação (ignorando o próprio ID) */
    if (strlen(bufferNome) > 0) {
        /* se existe outro laboratorio com esse nome e id diferente, recusa */
        FILE *fchk = fopen(ARQ_LABS, "rb");
        if (fchk) {
            fseek(fchk, sizeof(FileHeader), SEEK_SET);
            Laboratorio tmp;
            int conflito = 0;
            while (fread(&tmp, sizeof(Laboratorio), 1, fchk) == 1) {
                if (tmp.ativo && stricmp(tmp.nome, bufferNome) == 0 && tmp.id != L.id) {
                    conflito = 1;
                    break;
                }
            }
            fclose(fchk);
            if (conflito) {
                printf("Erro: outro laboratorio ativo ja usa esse nome. Edicao abortada.\n");
                pauseAndContinue();
                return;
            }
        }
        strcpy(L.nome, bufferNome);
    }

    lerString("Novo CNPJ: ", bufferCNPJ, MAX_CPF);
    if (strlen(bufferCNPJ) > 0) strcpy(L.cnpj, bufferCNPJ);

    lerString("Nova descricao: ", bufferDesc, MAX_DESC);
    if (strlen(bufferDesc) > 0) strcpy(L.descricao, bufferDesc);

    /* Atualizar registro no arquivo */
    FILE *f = fopen(ARQ_LABS, "r+b");
    if (!f) {
        printf("Erro ao abrir arquivo para atualizar.\n");
        return;
    }

    int pos = sizeof(FileHeader) + (int)idx * sizeof(Laboratorio);
    fseek(f, pos, SEEK_SET);
    fwrite(&L, sizeof(Laboratorio), 1, f);
    fclose(f);

    printf("Laboratorio atualizado com sucesso!\n");
}

void removerLaboratorio() {
    int id = lerInt("ID do laboratorio a remover: ");

    Laboratorio L;
    int idx;

    if (buscarLaboratorioPorId(id, &L, &idx)) {
        printf("Laboratorio nao encontrado!\n");
        pauseAndContinue();
        return;
    }
    /*
     -------- BLOQUEIO: verificar se há produtos usando este laboratório --------
    if (laboratorioTemProdutosVinculados(id)) {
        printf("\nERRO: Este laboratorio NAO pode ser removido.\n");
        printf("Ha produtos vinculados a ele.\n");
        pauseAndContinue();
        return;
    }
    */
    // ---------------- MOSTRAR REGISTRO ENCONTRADO ----------------
    printf("\n--- REGISTRO ENCONTRADO ---\n");
    printf("ID: %d\n", L.id);
    printf("Nome atual: %s\n", L.nome);
    printf("CNPJ: %s\n", L.cnpj);
    printf("Descricao: %s\n", L.descricao);

    // ---------------- CONFIRMAÇÃO ----------------
    printf("\nDeseja realmente excluir (logicamente) este laboratorio? (S/N): ");
    char resp;
    scanf(" %c", &resp);
    limparBuffer();

    if (resp != 'S' && resp != 's') {
        printf("Exclusao cancelada.\n");
        pauseAndContinue();
        return;
    }

    // --------- MARCAR COMO INATIVO ----------
    L.ativo = 0;

    // --------- SALVAR NO ARQUIVO ----------
    FILE *f = fopen(ARQ_LABS, "r+b");
    if (!f) {
        printf("Erro ao abrir arquivo para escrita.\n");
        pauseAndContinue();
        return;
    }

    long offset = sizeof(FileHeader) + (long)idx * sizeof(Laboratorio);
    fseek(f, offset, SEEK_SET);
    fwrite(&L, sizeof(Laboratorio), 1, f);

    fclose(f);

    printf("Laboratorio removido com sucesso!\n");
    pauseAndContinue();
}

void listarAtivarLaboratoriosInativos() {
    FILE *f = fopen(ARQ_LABS, "r+b");
    if (!f) {
        printf("Erro ao abrir arquivo de laboratorios.\n");
        pauseAndContinue();
        return;
    }

    FileHeader h;
    if (fread(&h, sizeof(FileHeader), 1, f) != 1) {
        printf("Arquivo de laboratorios corrompido ou vazio.\n");
        pauseAndContinue();
        fclose(f);
        return;
    }

    Laboratorio L;
    int idx = 0;
    int encontrou = 0;
    printf("\n--- LABORATORIOS INATIVOS ---\n");

    while (fread(&L, sizeof(Laboratorio), 1, f) == 1) {
        if (L.ativo == 0) {
            encontrou = 1;
            printf("ID: %d | Nome: %s | Descricao: %s\n", L.id, L.nome, L.descricao);
        }
        idx++;
    }

    if (!encontrou) {
        printf("Nenhum laboratorio inativo encontrado.\n");
        fclose(f);
        return;
    }

    printf("\nO que deseja fazer?\n");
    printf("1) Ativar UM laboratorio especifico\n");
    printf("2) Ativar TODOS os inativos\n");
    printf("0) Nao ativar nada (voltar)\n");

    int op = lerInt("Opcao: ");
    if (op == 0) { fclose(f); return; }

    /* ativar um específico */
    if (op == 1) {
        int id = lerInt("Informe o ID do laboratorio a ativar: ");
        rewind(f); fread(&h, sizeof(FileHeader), 1, f);
        idx = 0;
        while (fread(&L, sizeof(Laboratorio), 1, f) == 1) {
            if (L.id == id && L.ativo == 0) {
                L.ativo = 1;
                long offset = sizeof(FileHeader) + (long)idx * sizeof(Laboratorio);
                fseek(f, offset, SEEK_SET);
                fwrite(&L, sizeof(Laboratorio), 1, f);
                printf("Laboratorio ID %d reativado!\n", id);
                pauseAndContinue();
                fclose(f);
                return;
            }
            idx++;
        }
        printf("ID informado nao corresponde a um laboratorio inativo.\n");
        pauseAndContinue();
        fclose(f);
        return;
    }

    /* ativar todos */
    if (op == 2) {
        rewind(f); fread(&h, sizeof(FileHeader), 1, f);
        idx = 0;
        int reativados = 0;

        while (fread(&L, sizeof(Laboratorio), 1, f) == 1) {

            long posRegistro = sizeof(FileHeader) + (long)idx * sizeof(Laboratorio);

            if (L.ativo == 0) {
                L.ativo = 1;

                /* Escrever sem quebrar posição do fread */
                fseek(f, posRegistro, SEEK_SET);
                fwrite(&L, sizeof(Laboratorio), 1, f);

                /* Voltar para onde fread deveria continuar */
                fseek(f, posRegistro + sizeof(Laboratorio), SEEK_SET);

                reativados++;
            }

            idx++;
        }

        printf("%d registros reativados com sucesso!\n", reativados);
        pauseAndContinue();
        fclose(f);
        return;
    }


    printf("Opcao invalida.\n");
    pauseAndContinue();
    fclose(f);
}

int buscarLaboratorioPorId(int id, Laboratorio *out, int *index) {
    FILE *f = fopen(ARQ_LABS, "rb");
    if (!f) return 1;

    /* pular header */
    fseek(f, sizeof(FileHeader), SEEK_SET);

    Laboratorio L;
    int idx = 0;
    while (fread(&L, sizeof(Laboratorio), 1, f) == 1) {
        if (L.id == id && L.ativo) {
            if (out) *out = L;
            if (index) *index = idx;
            fclose(f);
            return 0;
        }
        idx++;
    }

    fclose(f);
    return 1;
}

int laboratorioJaExiste(const char *nome) {
    FILE *f = fopen(ARQ_LABS, "rb");
    if (!f) return 0; /* se não existir arquivo, não existe */

    fseek(f, sizeof(FileHeader), SEEK_SET);
    Laboratorio L;
    while (fread(&L, sizeof(Laboratorio), 1, f) == 1) {
        if (L.ativo && stricmp(L.nome, nome) == 0) {
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}

int laboratorioTemProdutosVinculados(int id_lab) {
    FILE *fprod = fopen(ARQ_PRODUTOS, "rb");
    if (!fprod) return 0; /* se arquivo de produtos não existir, não há vínculos */

    /* pular header de produtos */
    fseek(fprod, sizeof(FileHeader), SEEK_SET);

    Produto p;
    while (fread(&p, sizeof(Produto), 1, fprod) == 1) {
        if (p.ativo && p.id_laboratorio == id_lab) {
            fclose(fprod);
            return 1;
        }
    }

    fclose(fprod);
    return 0;
}

/* Categorias */
int categoriaExistePorNome(const char *nome) {
    FILE *f = fopen(ARQ_CATS, "rb");
    if (!f) return 0;

    FileHeader h;
    fread(&h, sizeof(FileHeader), 1, f);

    Categoria c;
    while (fread(&c, sizeof(Categoria), 1, f) == 1) {
        if (c.ativo && stricmp(c.nome, nome) == 0) {
            fclose(f);
            return 1; // Já existe
        }
    }

    fclose(f);
    return 0;
}

void cadastrarCategoria() {
    Categoria c;

    printf("\n--- CADASTRAR CATEGORIA ---\n");

    /* -------------------- Validar Nome -------------------- */
    int nomeValido = 0;
    while (!nomeValido) {
        lerString("Nome: ", c.nome, MAX_NOME);

        if (strlen(c.nome) == 0) {
            printf("Erro: nome nao pode ser vazio!\n");
        }
        else if (categoriaExistePorNome(c.nome)) {
            printf("Erro: ja existe categoria com esse nome!\n");
        }
        else {
            nomeValido = 1;
        }
    }

    /* -------------------- Descricao -------------------- */
    lerString("Descricao: ", c.descricao, MAX_DESC);

    /* -------------------- Gravar -------------------- */
    FileHeader h;
    read_header(ARQ_CATS, &h);

    c.id = h.last_id++;
    c.ativo = 1;

    h.count++;

    FILE *f = fopen(ARQ_CATS, "r+b");
    if (!f) {
        printf("Erro ao abrir arquivo de categorias.\n");
        pauseAndContinue();
        return;
    }

    /* Atualizar header */
    fseek(f, 0, SEEK_SET);
    fwrite(&h, sizeof(FileHeader), 1, f);

    /* Gravar categoria no final */
    fseek(f, 0, SEEK_END);
    fwrite(&c, sizeof(Categoria), 1, f);

    fclose(f);

    printf("Categoria cadastrada com ID %d!\n", c.id);
    pauseAndContinue();
}


int buscarCategoriaPorId(int id, Categoria *out, int *index) {
    FILE *f = fopen(ARQ_CATS, "rb");
    if (!f) return 1;

    FileHeader h;
    fread(&h, sizeof(FileHeader), 1, f);

    Categoria c;
    int idx = 0;

    while (fread(&c, sizeof(Categoria), 1, f) == 1) {
        if (c.id == id && c.ativo) {
            if (out) *out = c;
            if (index) *index = idx;

            fclose(f);
            return 0;
        }
        idx++;
    }

    fclose(f);
    return 1;
}

void listarCategorias() {
    FILE *f = fopen(ARQ_CATS, "rb");
    if (!f) {
        printf("Erro ao abrir arquivo de categorias.\n");
        pauseAndContinue();
        return;
    }

    FileHeader h;
    if (fread(&h, sizeof(FileHeader), 1, f) != 1) {
        printf("Arquivo de categorias corrompido.\n");
        fclose(f);
        pauseAndContinue();
        return;
    }

    Categoria c;
    int encontrou = 0;

    printf("\n==================== LISTA DE CATEGORIAS ====================\n");
    printf("%-5s | %-25s | %-40s\n", "ID", "Nome", "Descricao");
    printf("--------------------------------------------------------------------------\n");

    while (fread(&c, sizeof(Categoria), 1, f) == 1) {
        if (c.ativo) {
            encontrou = 1;
            printf("%-5d | %-25s | %-40s\n",
                   c.id,
                   c.nome,
                   c.descricao);
        }
    }

    if (!encontrou)
        printf("Nenhuma categoria cadastrada.\n");

    fclose(f);
    pauseAndContinue();
}

void editarCategoria() {
    int id = lerInt("ID da categoria que deseja editar: ");

    Categoria c;
    int idx;

    if (buscarCategoriaPorId(id, &c, &idx)) {
        printf("Categoria nao encontrada!\n");
        pauseAndContinue();
        return;
    }

    printf("\n--- REGISTRO ENCONTRADO ---\n");
    printf("ID: %d\nNome atual: %s\nDescricao atual: %s\n",
           c.id, c.nome, c.descricao);
    
    printf("\nDeseja realmente editar essa categoria? (S/N): ");
    char resp;
    scanf(" %c", &resp);
    limparBuffer();
    if (resp != 'S' && resp != 's') {
        printf("Edicao cancelada.\n");
        pauseAndContinue();
        return;
    }
    
    char novoNome[MAX_NOME];
    char novaDesc[MAX_DESC];

    lerString("Novo nome (ENTER para manter): ", novoNome, MAX_NOME);
    lerString("Nova descricao (ENTER para manter): ", novaDesc, MAX_DESC);

    if (strlen(novoNome) > 0) {
        if (!stricmp(novoNome, c.nome) == 0 && categoriaExistePorNome(novoNome)) {
            printf("Erro: Ja existe categoria com esse nome!\n");
            pauseAndContinue();
            return;
        }
        strcpy(c.nome, novoNome);
    }

    if (strlen(novaDesc) > 0)
        strcpy(c.descricao, novaDesc);

    FILE *f = fopen(ARQ_CATS, "r+b");

    int pos = sizeof(FileHeader) + (int)idx * sizeof(Categoria);
    fseek(f, pos, SEEK_SET);
    fwrite(&c, sizeof(Categoria), 1, f);

    fclose(f);

    printf("Categoria atualizada!\n");
    pauseAndContinue();
}

void removerCategoria() {
    int id = lerInt("ID da categoria a remover: ");

    Categoria c;
    int idx;

    if (buscarCategoriaPorId(id, &c, &idx)) {
        printf("Categoria nao encontrada!\n");
        pauseAndContinue();
        return;
    }

    printf("\n--- REGISTRO ENCONTRADO ---\n");
    printf("ID: %d\nNome: %s\nDescricao: %s\n",
           c.id, c.nome, c.descricao);

    printf("\nDeseja realmente excluir (S/N)? ");
    char resp;
    scanf(" %c", &resp);
    limparBuffer();

    if (resp != 'S' && resp != 's') {
        printf("Exclusao cancelada.\n");
        pauseAndContinue();
        return;
    }

    c.ativo = 0;

    FILE *f = fopen(ARQ_CATS, "r+b");

    int pos = sizeof(FileHeader) + (int)idx * sizeof(Categoria);
    fseek(f, pos, SEEK_SET);
    fwrite(&c, sizeof(Categoria), 1, f);

    fclose(f);

    printf("Categoria removida!\n");
    pauseAndContinue();
}

void listarAtivarCategoriasInativas() {
    FILE *f = fopen(ARQ_CATS, "r+b");
    if (!f) {
        printf("Erro ao abrir arquivo de categorias.\n");
        pauseAndContinue();
        return;
    }

    FileHeader h;
    fread(&h, sizeof(FileHeader), 1, f);

    Categoria c;
    int idx = 0, encontrou = 0;

    printf("\n--- CATEGORIAS INATIVAS ---\n");

    while (fread(&c, sizeof(Categoria), 1, f) == 1) {
        if (!c.ativo) {
            encontrou = 1;
            printf("ID: %d | Nome: %s | Descricao: %s\n",
                   c.id, c.nome, c.descricao);
        }
        idx++;
    }

    if (!encontrou) {
        printf("Nenhuma categoria inativa encontrada.\n");
        fclose(f);
        pauseAndContinue();
        return;
    }

    printf("\n1) Ativar um especifico\n2) Ativar todos\n0) Cancelar\n");
    int op = lerInt("Opcao: ");

    if (op == 0) { fclose(f); return; }

    // ---- Ativar um ----
    if (op == 1) {
        int id = lerInt("ID da categoria a ativar: ");

        rewind(f); fread(&h, sizeof(FileHeader), 1, f);
        idx = 0;

        while (fread(&c, sizeof(Categoria), 1, f) == 1) {
            if (c.id == id && !c.ativo) {
                c.ativo = 1;

                long pos = sizeof(FileHeader) + (long)idx * sizeof(Categoria);
                fseek(f, pos, SEEK_SET);
                fwrite(&c, sizeof(Categoria), 1, f);

                printf("Categoria ativada!\n");
                fclose(f);
                pauseAndContinue();
                return;
            }
            idx++;
        }

        printf("ID invalido ou categoria ja ativa.\n");
        fclose(f);
        pauseAndContinue();
        return;
    }

    // ---- Ativar todos ----
    if (op == 2) {
        rewind(f); fread(&h, sizeof(FileHeader), 1, f);
        idx = 0;
        int ativados = 0;

        while (fread(&c, sizeof(Categoria), 1, f) == 1) {
            if (!c.ativo) {
                c.ativo = 1;

                long pos = sizeof(FileHeader) + (long)idx * sizeof(Categoria);
                fseek(f, pos, SEEK_SET);
                fwrite(&c, sizeof(Categoria), 1, f);

                fseek(f, pos + sizeof(Categoria), SEEK_SET);

                ativados++;
            }
            idx++;
        }

        printf("%d categorias reativadas!\n", ativados);
        fclose(f);
        pauseAndContinue();
        return;
    }

    printf("Opcao invalida.\n");
    fclose(f);
    pauseAndContinue();
}

