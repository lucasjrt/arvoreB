#include <sys/types.h>
#include <sys/stat.h>

#include "btree.h"

bool fileExists(const char *filename) {
    struct stat statBuf; 
    if (stat(filename,&statBuf) < 0) 
        return false;
    return S_ISREG(statBuf.st_mode);
}

btree::btree() {
    char nomearquivo[30] = "arvoreb.dat";

    // se arquivo ja existir, abrir e carregar cabecalho
    if (fileExists(nomearquivo)) {
        // abre arquivo
        arquivo = fopen(nomearquivo,"r+");
        leCabecalho();
    }
    // senao, criar novo arquivo e salva o cabecalho
    else {
        // cria arquivo
        arquivo = fopen(nomearquivo,"w+");

        // atualiza cabecalho
        cabecalhoArvore.numeroElementos = 0;
        cabecalhoArvore.paginaRaiz = -1;
        salvaCabecalho();
    }
}

btree::~btree() {
    // fechar arquivo
    fclose(arquivo);
}

// media de ocupacao de registros nas folhas
int btree::computarTaxaOcupacao() {
    return 0;
}

void btree::insereChave(int chave, int offsetRegistro) {
    // cabecalho contem o numero da pagina raiz
    // se (cabecalhoArvore.paginaRaiz == 1) entao raiz eh a unica pagina. Ler raiz, inserir e salvar. Senao...
            // Exemplo:
            // pagina *pg = lePagina(cabecalhoArvore.paginaRaiz);
            // adicionar <chave,offsetRegistro> na pagina pg
            // salvar pagina: salvaPagina(pg->numeroPagina, pg);
    if (cabecalhoArvore.paginaRaiz == -1) {
        int idRetorno;
        pagina *raiz = novaPagina(&idRetorno);
        raiz->numeroElementos++;
        raiz->chaves[0] = chave;
        raiz->ponteiros[0] = offsetRegistro;
        leCabecalho();
        cabecalhoArvore.numeroElementos++;
        cabecalhoArvore.paginaRaiz = raiz->numeroPagina;
        cabecalhoArvore.alturaArvore = 1;
        salvaPagina(raiz);
        salvaCabecalho();
        return;
    }
    // senao...
    else {
        // encontrar a página que eu vou inserir o nó
        int i, alt = 0;
        int paginasPai[cabecalhoArvore.alturaArvore];
        paginaAtual = *lePagina(cabecalhoArvore.paginaRaiz);
        // atribui a página que a chave vai ser inserida para paginaAtual
        while(alt < cabecalhoArvore.alturaArvore - 1) {
            paginasPai[alt] = paginaAtual.numeroPagina;
            for (i = 0; i < paginaAtual.numeroElementos; i++) {
                if(chave == paginaAtual.chaves[i])
                    return;
                if (chave < paginaAtual.chaves[i])
                    break;
            }
            if (i > 0) 
                paginaAtual = *lePagina(paginaAtual.ponteiros[i - 1]);
            else
                paginaAtual = *lePagina(paginaAtual.ponteiros[0]);
            alt++;
        }
        insereNaPagina(chave, offsetRegistro, &paginaAtual, paginasPai, alt);
        // se não, só insere
    }

    // ler pagina raiz: pagina *pg = lePagina(cabecalhoArvore.paginaRaiz);

    // se inserir, atualizar cabecalho
}
/* Casos de remoção:
 * DONE: 1. Arvore vazia
 * DONE: 2. Arvore com um elemento
 * DONE: 3. Arvore com uma pagina apenas e com mais de 1 elemento
 * DONE: 4. Pagina com mais de 50% de capacidade
 * DONE: 5. Pagina com 50% de capacidade com irmao tendo mais de 50% da capacidade
 * DONE: 6. Pagina e irmao com 50% da capacidade
 */
void btree::removeChave(int chave) {
    leCabecalho();
    printf("Removendo chave %d\n", chave);
    // Caso 1.
    if (!cabecalhoArvore.numeroElementos) {
        printf("Arvore vazia\n");
        return;
    }
    // Caso 2.
    if (cabecalhoArvore.numeroElementos == 1) {
        printf("Esvaziando arvore.\n");
        cabecalhoArvore.paginaRaiz = -1;
        cabecalhoArvore.alturaArvore = 0;
        cabecalhoArvore.numeroElementos = 0;
        cabecalhoArvore.numeroPaginas = 0;
        salvaCabecalho();
        return;
    } 
    // Caso 3.
    else if (cabecalhoArvore.numeroPaginas == 1) {
        printf("Removendo da unica pagina da arvore.\n");
        paginaAtual = *lePagina(cabecalhoArvore.paginaRaiz);
        removeDaPagina(&paginaAtual, chave, NULL, 0);
    }
    // Procura a pagina do elemento
    else {
        printf("Procurando pagina a remover o elemento.\n");
        paginaAtual = *lePagina(cabecalhoArvore.paginaRaiz);
        int i, alt = 0;
        int paginasPai[cabecalhoArvore.numeroElementos];
        while(alt < cabecalhoArvore.alturaArvore - 1) {
            paginasPai[alt++] = paginaAtual.numeroPagina;
            for (i = 0; i < paginaAtual.numeroElementos; i++) {
                if(chave == paginaAtual.chaves[i]) {
                    printf("chave encontrada, i: %d\n", i);
                    paginaAtual = *lePagina(paginaAtual.ponteiros[i]);
                    break;
                }
                if(chave < paginaAtual.chaves[i]) {
                    printf("chave < elemento[%d] (%d < %d)\n", i, chave, paginaAtual.chaves[i]);
                    if (i > 0) 
                        paginaAtual = *lePagina(paginaAtual.ponteiros[i - 1]);
                    else
                        paginaAtual = *lePagina(paginaAtual.ponteiros[0]);
                    break;
                }
            }
        }
        printf("Pagina encontrada: %d\n", paginaAtual.numeroPagina);
        // Caso 4.
        if (!removeDaPagina(&paginaAtual, chave, paginasPai, alt)) {
            printf("Elemento nao encontrado.\n");
            return;
        }
        if (paginaAtual.numeroElementos < ORDEM >> 1) {
            pagina *pai = lePagina(paginasPai[alt - 1]);
            pagina *irmaoE = NULL, *irmaoD = NULL;
            // encontrar irmaos na pagina pai
            for (int i = 0; i < pai->numeroElementos; i++) {
                if (paginaAtual.numeroPagina == pai->ponteiros[i]) {
                    if (i > 0)
                        irmaoE = lePagina(pai->ponteiros[i - 1]);
                    if (i < pai->numeroElementos - 1)
                        irmaoD = lePagina(pai->ponteiros[i + 1]);
                    break;
                }
            }
            //DEBUG 
            if (irmaoE !=  NULL)
                printf("ID irmao esquerdo: %d\n", irmaoE->numeroPagina);
            else
                printf("Irmao esquerdo nao existe\n");
            if (irmaoD != NULL)
                printf("ID irmao direito: %d\n", irmaoD->numeroPagina);
            else
                printf("Irmao direito nao existe\n");
            // Caso 5.
            // 5.1.
            int indicePagina;
            for (int i = 0; i < pai->numeroElementos; i++)
                if (pai->ponteiros[i] == paginaAtual.numeroPagina)
                    indicePagina = i;
            if (irmaoE != NULL && irmaoE->numeroElementos > ORDEM >> 1) {
                printf("Solicitando elemento do irmao esquerdo\n");
                irmaoE->numeroElementos--;
                for (int i = paginaAtual.numeroElementos; i >= 0; i--) {
                    paginaAtual.chaves[i + 1] = paginaAtual.chaves[i];
                    paginaAtual.ponteiros[i + 1] = paginaAtual.ponteiros[i];
                }
                paginaAtual.chaves[0] = irmaoE->chaves[irmaoE->numeroElementos];
                paginaAtual.ponteiros[0] = irmaoE->ponteiros[irmaoE->numeroElementos];
                paginaAtual.numeroElementos++;
                pai->chaves[indicePagina] = paginaAtual.chaves[0];
                salvaPagina(irmaoE);
                salvaPagina(pai);
            }
            // 5.2.
            else if(irmaoD != NULL && irmaoD->numeroElementos > ORDEM >> 1) {
                printf("Solicitando elemento do irmao direito\n");
                paginaAtual.chaves[paginaAtual.numeroElementos] = irmaoD->chaves[0];
                paginaAtual.ponteiros[paginaAtual.numeroElementos] = irmaoD->ponteiros[0];
                paginaAtual.numeroElementos++;
                irmaoD->numeroElementos--;
                for (int i = 0; i < irmaoD->numeroElementos; i++) {
                    irmaoD->chaves[i] = irmaoD->chaves[i + 1];
                    irmaoD->ponteiros[i] = irmaoD->ponteiros[i + 1];
                }
                pai->chaves[indicePagina + 1] = irmaoD->chaves[0];
                salvaPagina(irmaoD);
                salvaPagina(pai);
            }
            // caso 6.
            else {
                if (irmaoE != NULL) {
                    printf("juntando paginas %d e %d\n", paginaAtual.numeroPagina, irmaoE->numeroPagina);
                    juntaPaginas(irmaoE, &paginaAtual, paginasPai, alt);
                }
                else {
                    printf("juntando paginas %d e %d\n", paginaAtual.numeroPagina, irmaoD->numeroPagina);
                    juntaPaginas(&paginaAtual, irmaoD, paginasPai, alt);
                }
            }
            salvaPagina(&paginaAtual);
        } else {
            printf("Pagina nao esta com menos elementos que o normal.\n");
        }
    }
    cabecalhoArvore.numeroElementos--;
    salvaCabecalho();
}

void btree::printPagina(int idPagina) {
    pagina *pg = lePagina(idPagina);
    printf("\nPagina:\n");
    printf("num elems: %d\n", pg->numeroElementos);
    printf("numeroPagina: %d\n", pg->numeroPagina);
    printf("chaves: {");
    for (int i = 0; i < pg->numeroElementos; i++) {
        printf("%d%s", pg->chaves[i], i < pg->numeroElementos - 1 ? ", "  : "}\n");
    }
    if (!pg->numeroElementos)
        printf(" }\n");
    printf("ponteiros: {");
    for (int i = 0; i < pg->numeroElementos; i++) {
        printf("%d%s", pg->ponteiros[i], i < pg->numeroElementos - 1 ? ", " : "}\n");
    }
    if (!pg->numeroElementos)
        printf(" }\n");
    printf("\n");
}

int btree::buscaChave(int chave) {
    // caso não encontrar chave, retornar -1
    int i, alt = 0;
    paginaAtual = *lePagina(cabecalhoArvore.paginaRaiz);
    while (alt < cabecalhoArvore.alturaArvore) {
        for (i = 0; i < paginaAtual.numeroElementos - 1; i++) {
            if (paginaAtual.chaves[i] == chave) {
                // se for folha
                if (alt == cabecalhoArvore.alturaArvore - 1)
                    return paginaAtual.ponteiros[i];
                paginaAtual = *lePagina(paginaAtual.ponteiros[i]);
                alt++;
                while (alt < cabecalhoArvore.alturaArvore) {
                    paginaAtual = *lePagina(paginaAtual.ponteiros[0]);
                    alt++;
                }
                return paginaAtual.ponteiros[0];
            }
            else if (paginaAtual.chaves[i] > chave) {
                if (i > 0) {
                    paginaAtual = *lePagina(paginaAtual.ponteiros[i - 1]);
                    break;
                }
                else
                    return -1;
            }
        }
        alt++;
    }
    return -1;
}

void btree::printTree() {
    leCabecalho();
    printf("=====================================================\n");
    printCabecalho();
    if (cabecalhoArvore.numeroElementos > 0) {
        pagina *pg = lePagina(cabecalhoArvore.paginaRaiz);
        printFilhos(pg, 0);
    }
    printf("=====================================================\n");
}

