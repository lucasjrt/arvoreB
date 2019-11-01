#ifndef _BTREE_H
#define	_BTREE_H

#include <stdio.h>

/*
 * Definicao da ordem da arvore
 */
#define ORDEM 511
//#define ORDEM 7

/*
 * Definicao da estrutura de dados do cabecalho. Objetivo do cabecalho é apenas guardar qual numero da pagina raiz da arvore
 */
struct cabecalhoB {
    int paginaRaiz; // numero da pagina raiz da arvore
    int alturaArvore; // altura da arvore
    int numeroElementos; // numero de chaves armazenadas na arvore
    int numeroPaginas; // numero de paginas da arvore
};
typedef struct cabecalhoB cabecalho;
/*
 * Definicao da estrutura de dados das paginas da arvore
 */
struct paginaB {
    int numeroElementos; // numero de elementos na pagina
    int numeroPagina; // vamos guardar o numero da pagina dentro da propria pagina
    int chaves[ORDEM];
    int ponteiros[ORDEM];
};
typedef struct paginaB pagina;

class btree {
public:
    /*
     * Construtor. Abre arquivo de indice.
     */
    btree();

    /*
     * Destrutor. Fecha arquivo de indice.
     */
    virtual ~btree();

    /*
     * Insere par chave e offset para o registro (offset do arquivo de registros) na árvore
     * Tarefas:
     * - localizar pagina para inserir registro
     * - inserir ordenado na pagina
     * - atualizar numeroElementos na pagina
     * - atualizar recursivamente as páginas ancestrais
     */
    void insereChave(int chave, int offsetRegistro);

    /*
     * Remove par chave e offset.
     * Tarefas:
     * - localizar pagina para remover registro
     * - inserir ordenado na pagina
     * - atualizar numeroElementos na pagina
     * - atualizar recursivamente as páginas ancestrais
     */
    void removeChave(int chave);

    /*
     * Busca chave e retorna o offset. Retorna -1 caso nao encontre a chave
     * Tarefas:
     * - localizar pagina
     * - retorna offset
     */
    int buscaChave(int chave);

    /*
     * Retorna numero de elementos armazenado no cabecalho da arvore
     */
    int getNumeroElementos() { return cabecalhoArvore.numeroElementos; }

    /*
     * Retorna altura da arvore armazenada no cabecalho da arvore
     */
    int getAlturaArvore() { return cabecalhoArvore.alturaArvore; }

    /*
     * Retorna o numero medio de elementos por pagina da arvore. Considerar apenas paginas folha
     */
    int computarTaxaOcupacao();

    void printTree();

private:
    /*
     * Cabecalho da arvore
     */
    cabecalho cabecalhoArvore;

    /*
     * Instancia para ler uma pagina
     */
    pagina paginaAtual;

    /*
     * Manipulador do arquivo de dados
     */
    FILE *arquivo;

    /*
     * Criaçao uma nova pagina. Parametro com numero da pagina deve ser passado por referencia (exemplo: int idpagina;
     * btree->novaPagina(&idpagina);) pois no retorno da funçao o idpagina tera o numero da nova pagina.
     */
    pagina *novaPagina(int *idPagina) {
        leCabecalho();
        cabecalhoArvore.numeroPaginas++;
        salvaCabecalho();
        pagina *pg = new pagina;
        pg->numeroElementos = 0;
        pg->numeroPagina = cabecalhoArvore.numeroPaginas;
        idPagina[0] = cabecalhoArvore.numeroPaginas;
        fseek(arquivo, 0, SEEK_END);
        fwrite(pg,sizeof(*pg),1,arquivo);
        return pg;
    }

    /*
     * Leitura de uma pagina existente.
     */
    pagina *lePagina(int idPagina) {
        pagina *pg = new pagina;
        fseek(arquivo, (1+idPagina)*sizeof(*pg), SEEK_SET);
        fread(pg,sizeof(*pg),1,arquivo);
        return pg;
    }

    /*
     * Persistencia de uma pagina.
     */
    void salvaPagina(pagina *pg) {
        int idPagina = pg->numeroPagina;
        fseek(arquivo, (1+idPagina)*sizeof(*pg), SEEK_SET);
        fwrite(pg,sizeof(*pg),1,arquivo);
    }

    /*
     * Salva o cabecalho
     */
    void salvaCabecalho() {
        fseek(arquivo,0,SEEK_SET);
        fwrite(&cabecalhoArvore,sizeof(cabecalhoArvore),1,arquivo);
    }

    /*
     * Le o cabecalho
     */
    void leCabecalho() {
        fseek(arquivo,0,SEEK_SET);
        fread(&cabecalhoArvore,sizeof(cabecalhoArvore),1,arquivo);
    }

    // void reorganizaPagina(pagina *pg, int paginasPai, int altura) {
    //     int i;
    //     if (altura > 1) {
    //         pagina *pai = lePagina(paginasPai[altura - 1]);
    //         pagina *irmaoEsquerda, *irmaoDireita;
    //         for (i = 0; i < pai->numeroElementos; i++) {
    //             if (pai->ponteiros[i] == pg->numeroPagina) {
                    
    //             }
    //         }
    //     }
    // }

    /*
     * Trunca a pagina pg no meio (baseado no tamanho de ORDEM), copia a
     * outra metade para nova pagina, reajusta os indices dos nos acima,
     * e retorna a nova pagina
     */
    pagina *dividePagina(pagina *pg) {
        int idRetorno, metade = 1 + (ORDEM >> 1);
        pagina *irmao = novaPagina(&idRetorno);
        for (int i = 0; i < ORDEM >> 1; i++) {
            irmao->chaves[i] = pg->chaves[i + metade];
            irmao->ponteiros[i] = pg->ponteiros[i + metade];
        }
        pg->numeroElementos = metade;
        irmao->numeroElementos = metade;
        if (ORDEM % 2 == 1)
            irmao->numeroElementos--;
        return irmao;
    }

    /*
     * altura: altura da pagina pg
    */
    void insereNaPagina(int chave, int ponteiro, pagina *pg, int *paginasPai, int altura) {
        if (pg->numeroElementos >= ORDEM) {
            pagina *irmao = dividePagina(pg);
            // atualiza pagina pai
            // se a raiz estiver cheia
            if (!altura) {
                int idRetorno;
                pagina *pai = novaPagina(&idRetorno);
                leCabecalho();
                cabecalhoArvore.paginaRaiz = pai->numeroPagina;
                cabecalhoArvore.alturaArvore++;
                salvaCabecalho();
                pai->chaves[0] = pg->chaves[0];
                pai->chaves[1] = irmao->chaves[0];
                pai->ponteiros[0] = pg->numeroPagina;
                pai->ponteiros[1] = irmao->numeroPagina;
                pai->numeroElementos = 2;
                salvaPagina(pai);
            } else {
                if (paginasPai == NULL)
                    printf("Warning: paginasPai is NULL.");
                insereNaPagina(irmao->chaves[0], irmao->numeroPagina, lePagina(paginasPai[altura - 1]), paginasPai, altura - 1);
            }
            // fim atualiza pagina pai
            if (chave < irmao->chaves[0]) {
                insereNaPagina(chave, ponteiro, pg, paginasPai, altura);
            }
            else 
                insereNaPagina(chave, ponteiro, irmao, paginasPai, altura);
            salvaPagina(irmao);
            salvaPagina(pg);
        } else {
            pagina *p = lePagina(paginasPai[altura - 1]);
            if (altura && chave < pg->chaves[0]){
                int i;
                for (i = 0; i < p->numeroElementos; i++)
                    if (p->ponteiros[i] == pg->numeroPagina)
                        break;
                if (i == p->numeroElementos) {
                    printf("Ta errado o codigo na linha %d em btree.h\n", __LINE__);
                    printf("i: %d\n", i);
                    scanf("%d", &i);
                }
                p->chaves[i] = chave;
                for (int i = pg->numeroElementos - 1; i >= 0; i--) {
                    pg->chaves[i + 1] = pg->chaves[i];
                    pg->ponteiros[i + 1] = pg->ponteiros[i];
                }
                pg->chaves[0] = chave;
                pg->ponteiros[0] = ponteiro;
                salvaPagina(p);
            }
            else {
                for (int i = pg->numeroElementos - 1; i >= 0; i--) {
                    if (chave > pg->chaves[i]) {
                        pg->chaves[i + 1] = chave;
                        pg->ponteiros[i + 1] = ponteiro;
                        break;
                    }
                    pg->chaves[i + 1] = pg->chaves[i];
                    pg->ponteiros[i + 1] = pg->ponteiros[i];
                    if (i == 0) {
                        pg->chaves[0] = chave;
                        pg->ponteiros[0] = ponteiro;
                    }
                }
            }
            pg->numeroElementos++;
            leCabecalho();
            cabecalhoArvore.numeroElementos++;
            salvaCabecalho();
            salvaPagina(pg);
        }
    }

    void printCabecalho() {
        printf("\nCabecalho:\n");
        printf("raiz: %d\n", cabecalhoArvore.paginaRaiz);
        printf("altura: %d\n", cabecalhoArvore.alturaArvore);
        printf("nro elems: %d\n", cabecalhoArvore.numeroElementos);
        printf("nro pags: %d\n\n", cabecalhoArvore.numeroPaginas);
    }

    void printPagina(pagina *pg) {
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

    void printFilhos(pagina *pg, int altura) {
        printPagina(pg);
        if(altura < cabecalhoArvore.alturaArvore - 1)
            for (int i = 0; i < pg->numeroElementos; i++)
                printFilhos(lePagina(pg->ponteiros[i]), altura + 1);
    }
};

#endif	/* _BTREE_H */

