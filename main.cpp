/*
 * ALUNO: Lucas Justino Resende Teixeira
 * File:   main.cpp
 * Created on October 24, 2019
 */

#include <stdlib.h>
#include <time.h>

#include "btree.cpp"

int main(int argc, char** argv) {

    // iniciar a semente aleatorio
    srand (time(NULL));

    // criar arvore b
    btree *arvore = new btree();

    printf("Use essas chaves como exemplos de consultas que devem ser encontradas: ");

    // inserir numeros aleatorios na arvore
    for (int i = 0; i < 10000; i++) {
        int valor = rand() % 1000000 + 1;
        arvore->insereChave(valor,valor+1);
        if (i % 1000 == 0)
            printf("%d, ",valor);
    }

    printf("\n\nEstatisticas:\n");
    printf("Numero de elementos: %d\n", arvore->getNumeroElementos());
    printf("Altura da arvore: %d\n", arvore->getAlturaArvore());
    printf("Taxa de ocupacao: %d%%\n", arvore->computarTaxaOcupacao());

    int opcao = 0;
    while (opcao != 5) {
        printf("\n\nMenu:\n1-inserir\n2-remover\n3-consultar\n4-imprimir arvore\n5-sair\nOpcao: ");
        scanf("%d",&opcao);
        switch(opcao) {
            int valor, offset;
            case 1:
                printf("\nInsere chave: ");
                scanf("%d",&valor);
                arvore->insereChave(valor,valor);
                break;
            case 2:
                printf("\nRemove chave: ");
                scanf("%d",&valor);
                arvore->removeChave(valor);
                break;
            case 3:
                printf("\nConsulta chave: ");
                scanf("%d",&valor);
                offset = arvore->buscaChave(valor);
                if (offset == -1)
                    printf("\nChave %d nao encontrada.\n",valor);
                else if (offset == valor)
                    printf("\nChave %d encontrada e offset=%d.\n",valor,offset);
                else
                    printf("\nErro: chave %d encontrada e offset=%d.\n",valor,offset);
                break;
            case 4:
                arvore->printTree();
                break;
            case 5:
                // exit :-)
                break;
            default:
                opcao = 0;
                break;
        }
    }

    delete arvore;

    return (EXIT_SUCCESS);
}
