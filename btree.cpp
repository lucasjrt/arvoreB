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
        paginaAtual = *lePagina(cabecalhoArvore.paginaRaiz);
        int paginasPai[cabecalhoArvore.alturaArvore];
        // atribui a página que a chave vai ser inserida para paginaAtual
        while(alt < cabecalhoArvore.alturaArvore - 1) {
            paginasPai[alt] = paginaAtual.numeroPagina;
            for (i = 0; i < paginaAtual.numeroElementos; i++)
                if (chave < paginaAtual.chaves[i])
                    break;
            if (i > 0) 
                paginaAtual = *lePagina(paginaAtual.ponteiros[i - 1]);
            else if (i == paginaAtual.numeroElementos)
                paginaAtual = *lePagina(i);
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

void btree::removeChave(int chave) {

    // se remover, atualizar cabecalho
    if (true) {
        cabecalhoArvore.numeroElementos--;
        salvaCabecalho();
    }
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
                if (i > 0)
                    paginaAtual = *lePagina(paginaAtual.ponteiros[i - 1]);
                else
                    return -1;
            }
        }
        alt++;
    }
    return -1;
}

void btree::printTree() {
    printf("=====================================================\n");
    printCabecalho();
    pagina *pg = lePagina(cabecalhoArvore.paginaRaiz);
    printFilhos(pg, 0);
    printf("=====================================================\n");
}

