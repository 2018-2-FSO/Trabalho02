#include <stdio.h>
#include <stdlib.h>

//1111 1111 0000 0000
#define mascara_num_pag 65280
//0000 0000 1111 1111
#define mascara_deslocamento 255

FILE *arq_enderecos_logicos;
void mascara_endereco(int endereco_logico);

int main(int argc, char *argv[]) {
    int endereco_logico;

    if (argc != 2) {
        fprintf(stderr,"Usar: ./proj02.out addresses.txt\n");
        return -1;
    }

    arq_enderecos_logicos= fopen(argv[1], "r");

    while(fscanf(arq_enderecos_logicos, "%d", &endereco_logico )!= EOF){
        mascara_endereco(endereco_logico);
    }

}

void mascara_endereco(int endereco_logico){
    int num_pagina, deslocamento;
    //fazer a operaçao and para zerar ultimos 8 bits e deslocar o numero para a direita 8 vezes
    num_pagina = (endereco_logico & mascara_num_pag) >> 8;
    //fazer a operação and para zerar os 8 primeiros numeros
    deslocamento = (endereco_logico & mascara_deslocamento);
    printf( "o endereco_logico %d tem numero de pagina: %d e deslocamento de: %d\n",endereco_logico, num_pagina, deslocamento);
}
