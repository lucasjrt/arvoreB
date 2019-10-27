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
        printf("inserindo a raiz\n");
        int idRetorno;
        pagina *raiz = novaPagina(&idRetorno);
        raiz->numeroElementos++;
        raiz->chaves[0] = chave;
        raiz->ponteiros[0] = offsetRegistro;
        cabecalhoArvore.numeroElementos++;
        cabecalhoArvore.paginaRaiz = raiz->numeroPagina;
        cabecalhoArvore.alturaArvore = 1;
        printf("id raiz: %d\n", cabecalhoArvore.paginaRaiz);
        salvaCabecalho();
        return;
    }
    if (cabecalhoArvore.paginaRaiz == 1) {
        printf("inserindo na raiz\n");
        pagina paginaAtual = *lePagina(1);
        if (paginaAtual.numeroElementos >= ORDEM) {
            // divide a pagina
            int idRetorno, metade = 1 + (ORDEM >> 1);
            pagina *irmao = novaPagina(&idRetorno);
            pagina *pai = novaPagina(&idRetorno);
            for (int i = 0; i < ORDEM >> 1; i++) {
                irmao->chaves[i] = paginaAtual.chaves[i + metade];
                irmao->ponteiros[i] = paginaAtual.ponteiros[i + metade];
            }
            cabecalhoArvore.paginaRaiz = idRetorno;
            cabecalhoArvore.alturaArvore++;
            pai->chaves[0] = paginaAtual.chaves[0];
            pai->chaves[1] = irmao->chaves[0];
            pai->ponteiros[0] = paginaAtual.numeroPagina;
            pai->ponteiros[1] = irmao->numeroPagina;
            paginaAtual.numeroElementos = metade;
            irmao->numeroElementos = metade;
            pai->numeroElementos = 2;
            salvaPagina(pai);
            salvaPagina(irmao);
            salvaPagina(&paginaAtual);
            if (chave >= irmao->chaves[0])
                paginaAtual = *irmao;
        }

        int pos = 0;
        for (int i = 0; i < paginaAtual.numeroElementos; i++) {
            if (paginaAtual.chaves[i] >= chave) 
                break;
            pos++;
        }
        for (int i = paginaAtual.numeroElementos; i < pos; i--) {
            paginaAtual.chaves[i+1] = paginaAtual.chaves[i];
            paginaAtual.ponteiros[i+1] = paginaAtual.ponteiros[i];
        }
        paginaAtual.chaves[pos] = chave;
        paginaAtual.ponteiros[pos] = offsetRegistro;
        paginaAtual.numeroElementos++;
    }
    // senao...
    else {
        
    }

    // ler pagina raiz: pagina *pg = lePagina(cabecalhoArvore.paginaRaiz);

    // se inserir, atualizar cabecalho
    cabecalhoArvore.numeroElementos++;
    salvaCabecalho();
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
            if (paginaAtual.chaves[i] == chave)
                return paginaAtual.numeroPagina * ORDEM + i * sizeof(paginaAtual.chaves[i]);
            else if (paginaAtual.chaves[i] > chave)
                paginaAtual = *lePagina(paginaAtual.ponteiros[i - 1]);
        }
        alt += 1;
    }
    return -1;
}

