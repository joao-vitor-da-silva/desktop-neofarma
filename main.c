#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================== DEFINICOES GLOBAIS ================== */
#include <stdint.h>

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

/* Laboratório / Fabricante */
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
    int id_produto;                 /* FK -> Produto.id */
    char codigo_lote[MAX_COD_LOTE]; /* código do lote */
    Data data_fabricacao;
    Data data_validade;
    int quantidade;                 /* quantidade atual no lote */
    double preco_compra;            /* preço por unidade pago na compra */
    double preco_venda;             /* preço por unidade deste lote (pode ser diferente do padrão) */
    int em_promocao;                /* 0 = não, 1 = sim */
    double desconto_percent;        /* 0..100 se em_promocao == 1 */
    int ativo;                      /* 1 = válido, 0 = lote desativado/exaurido/descartado */
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
    int ativo;               /* para marcar anulação/excluído - 1 = válido */
} Venda;

/* Compra (registro simples) */
/* Observação: em compras você geralmente cria lotes; aqui deixamos registro mínimo */
typedef struct {
    int id;
    Data data;
    int itens_count;         /* se quiser guardar itens semelhantes a ItemVenda */
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

/* Produtos & Lotes */
void menuProdutos();
void cadastrarProduto();
void editarProduto();
void removerProduto();
void listarProdutos();
void menuLotes();
void cadastrarLote();
void editarLote();
void listarLotes();
void descartarLote();

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
void cadastrarTipo();
void listarTipos();
void cadastrarLaboratorio();
void listarLaboratorios();
void cadastrarCategoria();
void listarCategorias();

/* Utilitários de I/O do menu */
void limparBuffer();
int lerInt(const char *prompt);
void lerString(const char *prompt, char *out, int maxlen);
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
                /* submenu de compras inclui criação de lotes internamente */
                menuComprasVendas(); // dentro esse submenu existe opção de compra
                break;
            case 5:
                menuComprasVendas(); // mesma entrada inclui vendas também
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
                break;
        }
    }
}

/* ----------------- Sub-menus (estruturas prontas) ----------------- */

/* Clientes */
void menuClientes() {
    int op;
    while (1) {
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
            default: printf("Opcao invalida.\n"); break;
        }
    }
}

/* Produtos */
void menuProdutos() {
    int op;
    while (1) {
        printf("\n--- PRODUTOS ---\n");
        printf("1) Cadastrar produto\n");
        printf("2) Editar produto\n");
        printf("3) Remover produto\n");
        printf("4) Listar produtos\n");
        printf("0) Voltar\n");
        op = lerInt("Opcao: ");
        switch (op) {
            case 1: cadastrarProduto(); break;
            case 2: editarProduto(); break;
            case 3: removerProduto(); break;
            case 4: listarProdutos(); pauseAndContinue(); break;
            case 0: return;
            default: printf("Opcao invalida.\n"); break;
        }
    }
}

/* Lotes */
void menuLotes() {
    int op;
    while (1) {
        printf("\n--- LOTES ---\n");
        printf("1) Cadastrar lote (entrada manual)\n");
        printf("2) Editar lote\n");
        printf("3) Descartar lote (parcial ou total)\n");
        printf("4) Listar lotes\n");
        printf("0) Voltar\n");
        op = lerInt("Opcao: ");
        switch (op) {
            case 1: cadastrarLote(); break;
            case 2: editarLote(); break;
            case 3: descartarLote(); break;
            case 4: listarLotes(); pauseAndContinue(); break;
            case 0: return;
            default: printf("Opcao invalida.\n"); break;
        }
    }
}

/* Compras e Vendas: submenu combinado */
void menuComprasVendas() {
    int op;
    while (1) {
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
            default: printf("Opcao invalida.\n"); break;
        }
    }
}

/* Relatórios */
void menuRelatorios() {
    int op;
    while (1) {
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
            default: printf("Opcao invalida.\n"); break;
        }
    }
}

/* Utils: tipos, laboratorios, categorias */
void menuUtils() {
    int op;
    while (1) {
        printf("\n--- UTILS (TIPOS / LABS / CATEGORIAS) ---\n");
        printf("1) Tipos (cadastrar / listar)\n");
        printf("2) Laboratorios (cadastrar / listar)\n");
        printf("3) Categorias (cadastrar / listar)\n");
        printf("0) Voltar\n");
        op = lerInt("Opcao: ");
        switch (op) {
            case 1:
                printf("\n-- TIPOS --\n1) Cadastrar tipo\n2) Listar tipos\n0) Voltar\n");
                if (lerInt("Opcao: ") == 1) cadastrarTipo(); else listarTipos();
                break;
            case 2:
                printf("\n-- LABS --\n1) Cadastrar laboratorio\n2) Listar laboratorios\n0) Voltar\n");
                if (lerInt("Opcao: ") == 1) cadastrarLaboratorio(); else listarLaboratorios();
                break;
            case 3:
                printf("\n-- CATEGORIAS --\n1) Cadastrar categoria\n2) Listar categorias\n0) Voltar\n");
                if (lerInt("Opcao: ") == 1) cadastrarCategoria(); else listarCategorias();
                break;
            case 0: return;
            default: printf("Opcao invalida.\n"); break;
        }
    }
}

/* ----------------- Exemplo de main que chama menuPrincipal ----------------- */
int main(void) {
    printf("Inicializando sistema...\n");
    menuPrincipal();
    return 0;
}

/* ----------------- Stubs (exemplo mínimo) ----------------- */
/* Substitua/implemente cada stub abaixo com sua lógica de arquivos binários. */

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
    h.count = 0;   // nenhum produto ainda
    h.last_id = 1; // começamos IDs em 1
    fwrite(&h, sizeof(FileHeader), 1, f);
    fclose(f);

    //printf("Arquivo %s criado com header inicial.\n", filename);
    return 0;
}

int read_header(const char *filename, FileHeader *h) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("Erro ao abrir %s para leitura.\n", filename);
        return -1;
    }

    fread(h, sizeof(FileHeader), 1, f);
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

/* Clientes */
void cadastrarCliente() {printf("[STUB] cadastrarCliente()\n"); }
void editarCliente()    { printf("[STUB] editarCliente()\n"); }
void removerCliente()   { printf("[STUB] removerCliente()\n"); }
void listarClientes()   { printf("[STUB] listarClientes()\n"); }

/* Produtos & Lotes */
void cadastrarProduto() { printf("[STUB] cadastrarProduto()\n"); }
void editarProduto()    { printf("[STUB] editarProduto()\n"); }
void removerProduto()   { printf("[STUB] removerProduto()\n"); }
void listarProdutos()   { printf("[STUB] listarProdutos()\n"); }

void cadastrarLote()    { printf("[STUB] cadastrarLote()\n"); }
void editarLote()       { printf("[STUB] editarLote()\n"); }
void listarLotes()      { printf("[STUB] listarLotes()\n"); }
void descartarLote()    { printf("[STUB] descartarLote()\n"); }

/* Compras & Vendas */
void efetuarCompra()    { printf("[STUB] efetuarCompra()\n"); }
void efetuarVenda()     { printf("[STUB] efetuarVenda()\n"); }
void listarVendas()     { printf("[STUB] listarVendas()\n"); }
void listarCompras()    { printf("[STUB] listarCompras()\n"); }

/* Relatórios */
void relatorioProdutosProximosVencimento() { printf("[STUB] relatorioProdutosProximosVencimento()\n"); }
void relatorioVendasPorPeriodo()            { printf("[STUB] relatorioVendasPorPeriodo()\n"); }
void relatorioEstoqueBaixo()                { printf("[STUB] relatorioEstoqueBaixo()\n"); }

/* Utils */
void cadastrarTipo()         { printf("[STUB] cadastrarTipo()\n"); }
void listarTipos()           { printf("[STUB] listarTipos()\n"); }
void cadastrarLaboratorio()  { printf("[STUB] cadastrarLaboratorio()\n"); }
void listarLaboratorios()    { printf("[STUB] listarLaboratorios()\n"); }
void cadastrarCategoria()    { printf("[STUB] cadastrarCategoria()\n"); }
void listarCategorias()      { printf("[STUB] listarCategorias()\n"); }
