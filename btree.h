#ifndef _BTREE_H
#define	_BTREE_H

#include <stdio.h>

/*
 * Definicao da ordem da arvore
 */
// #define ORDEM 511
#define ORDEM 7

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

    void printPagina(int idPagina);

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

    /*
     * Casos possiveis:
     * 1. Juntar a pagina sem precisar juntar o pai
     * 2. Juntar a pagina e precisar juntar o pai
     */
    void juntaPaginas(pagina *pg1, pagina *pg2, int paginasPai[], int altura) {
        // Caso 1.
        leCabecalho();
        for (int i = 0; i < pg2->numeroElementos; i++) {
            pg1->chaves[pg1->numeroElementos] = pg2->chaves[i];
            pg1->ponteiros[pg1->numeroElementos] = pg2->ponteiros[i];
            pg1->numeroElementos++;
        }
        pagina *pai = lePagina(paginasPai[altura - 1]);
        printf("id pai: %d\n", pai->numeroPagina);
        for (int i = 0; i < pai->numeroElementos; i++) {
            if (pai->ponteiros[i] == pg2->numeroPagina) {
                for(int j = i; j < pai->numeroElementos - 1; j++) {
                    pai->chaves[j] = pai->chaves[j+1];
                    pai->ponteiros[j] = pai->ponteiros[j+1];
                }
                break;
            }
        }
        pai->numeroElementos--;
        cabecalhoArvore.numeroPaginas--;
        salvaPagina(pg1);
        // Caso 2.
        /* junta pai com 2 casos:
        * 2.1. Pai e' raiz
        * 2.1.1. Promove o filho para raiz e reduz a altura da arvore
        * 2.2. Pai nao e' raiz
        * 2.2.1. Tio da esquerda tem chave para fornecer
        * 2.2.2. Tio da direita tem chave para fornecer
        * 2.2.3. Nenhum tio tem chave pra oferecer
        */
        if (pai->numeroElementos < ORDEM >> 1) {
            // 2.1.
            if (pai->numeroPagina == cabecalhoArvore.paginaRaiz && pai->numeroElementos == 1) {
                cabecalhoArvore.paginaRaiz = pai->ponteiros[0];
                cabecalhoArvore.numeroPaginas--;
                cabecalhoArvore.alturaArvore--;
            } 
            // 2.2.
            else {
                pagina *avo = lePagina(paginasPai[altura - 2]);
                pagina *tioE, *tioD;
                for (int i = 0; i < avo->numeroElementos; i++) {
                    if (avo->ponteiros[i] == pai->numeroPagina) {
                        if (i > 0)
                            tioE = lePagina(avo->ponteiros[i - 1]);
                        if (i < avo->numeroElementos - 1)
                            tioD = lePagina(avo->ponteiros[i + 1]);
                        break;
                    }
                }
                // 2.2.1.
                if (tioE != NULL && tioE->numeroElementos > ORDEM >> 1) {
                    tioE->numeroElementos--;
                    for (int i = pai->numeroElementos - 1; i >= 0; i--) {
                        pai->chaves[i + 1] = pai->chaves[i];
                        pai->ponteiros[i + 1] = pai->ponteiros[i];
                    }
                    pai->chaves[0] = tioE->chaves[tioE->numeroElementos];
                    pai->ponteiros[0] = tioE->ponteiros[tioE->numeroElementos];
                    pai->numeroElementos++;
                    salvaPagina(tioE);
                } 
                // 2.2.2.
                else if (tioD != NULL && tioD->numeroElementos > ORDEM >> 1) {
                    pai->chaves[pai->numeroElementos] = tioD->chaves[0];
                    pai->ponteiros[pai->numeroElementos] = tioD->ponteiros[0];
                    tioD->numeroElementos--;
                    for (int i = 0; i < tioD->numeroElementos; i++) {
                        tioD->chaves[i] = tioD->chaves[i + 1];
                        tioD->ponteiros[i] = tioD->ponteiros[i + 1];
                    }
                    pai->numeroElementos++;
                    salvaPagina(tioD);
                }
                // 2.2.3.
                else {
                    if (tioE != NULL)
                        juntaPaginas(tioE, pai, paginasPai, altura - 1);
                    else
                        juntaPaginas(pai, tioD, paginasPai, altura - 1);
                }
            }
        }
        printf("Pai antes do retorno:");
        printPagina(pai);
        salvaPagina(pai);
        pai = lePagina(pai->numeroPagina);
        printf("Pagina escrita em disco:");
        printPagina(pai);
        salvaCabecalho();
    }

    /*
     * Trunca a pagina pg no meio (baseado no tamanho de ORDEM), copia a
     * outra metade para nova pagina, reajusta os indices dos nos acima,
     * e retorna a nova pagina
     */
    pagina *dividePagina(pagina *pg) {
        int idRetorno, metade = 1 + (ORDEM >> 1), metade2;
        if (ORDEM % 2)
            metade2 = ORDEM >> 1;
        else
            metade2 = (ORDEM >> 1) + 1;
        pagina *irmao = novaPagina(&idRetorno);
        for (int i = 0; i < metade2; i++) {
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

    // retorna 1 se sucesso, 0 caso contrario
    int removeDaPagina(pagina *pg, int chave, int paginasPai[], int altura) {
        printf("Removendo a chave %d da pagina %d\n", chave, pg->numeroPagina);
        int i;
        char removido = 0;
        for (i = 0; i < pg->numeroElementos; i++) {
            if (chave == pg->chaves[i]) {
                printf("chave encontrada\n");
                for (int j = i; j < pg->numeroElementos - 1; j++) {
                    pg->chaves[j] = pg->chaves[j+1];
                    pg->ponteiros[j] = pg->ponteiros[j+1];
                }
                // se necessario atualizar o ponteiro do pai
                if (altura && i == 0) {
                    printf("Primeiro elemento\n");
                    pagina *pai = lePagina(paginasPai[altura - 1]);
                    for (int j = 0; j < pai->numeroElementos; j++) {
                        if (pai->ponteiros[j] == pg->numeroPagina) {
                            printf("atualizando indice do pai\n");
                            printf("alterando de %d para %d\n", pai->chaves[j], pg->chaves[0]);
                            pai->chaves[j] = pg->chaves[0];
                            break;
                        }
                    }
                    salvaPagina(pai);
                }
                pg->numeroElementos--;
                removido = 1;
                printf("Removido\n");
                break;
            }
        }
        salvaPagina(&paginaAtual);
        printf("i: %d, numero elementos: %d\n", i, pg->numeroElementos);
        return (int) removido;
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

