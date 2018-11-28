#include <stdio.h>
#include <stdlib.h>

//1111 1111 0000 0000
#define mascara_num_pag 65280
//0000 0000 1111 1111
#define mascara_deslocamento 255
#define quantidade_paginas 256
#define tamanho_pagina 256
#define quantidade_tlb 16

void transforma_endereco(int endereco_logico);
int verifica_tlb(int num_pagina, int deslocamento);
void inicializa_tlb();
int verifica_tabela(int num_pagina, int deslocamento);
void atualiza_memoria_fisica(int num_pagina, int quadros_usados);
void atualiza_tlb(int num_pagina, int numero_quadro);
int traduz_endereco(int posicao_memoria);

int tabela_pagina[quantidade_paginas];
int memoria_fisica[quantidade_paginas*tamanho_pagina];
int quant_acesso_memoria = 0;
int tlb_vazia = 0, erro_pagina =0;
int ultimo_tlb = 0;
int tlb_usada = 0, quadros_usados = 0;

typedef struct tlb {
  int pagina;
  int quadro;
} Tlb;

Tlb tlb[quantidade_tlb];

FILE *arq_enderecos_logicos, *hd;


int main(int argc, char *argv[]) {
    int endereco_logico;
    inicializa_tlb();

    if (argc != 2) {
        fprintf(stderr,"Usar: ./proj02.out addresses.txt\n");
        return -1;
    }

    hd = fopen("BACKING_STORE.bin", "rb");
    arq_enderecos_logicos= fopen(argv[1], "r");

    if(arq_enderecos_logicos == NULL){
		printf("Error opening file\n");
		return -1;
	}

    if(hd == NULL){
		printf("Error opening file\n");
		return -1;
	}

    do{
        while(fscanf(arq_enderecos_logicos, "%d", &endereco_logico )!= EOF){
            quant_acesso_memoria ++;
            transforma_endereco(endereco_logico);
            
        }
        fclose(hd);
        fclose(arq_enderecos_logicos);
    }while(arq_enderecos_logicos != NULL && hd != NULL);
    

}

void transforma_endereco(int endereco_logico){
    int num_pagina, deslocamento, numero_quadro, posicao_memoria, converte;
    //fazer a operaçao and para zerar ultimos 8 bits e deslocar o numero para a direita 8 vezes
    num_pagina = (endereco_logico & mascara_num_pag) >> 8;
    //fazer a operação and para zerar os 8 primeiros numeros
    deslocamento = (endereco_logico & mascara_deslocamento);//offset
    printf( "o endereco_logico %d tem numero de pagina: %d e deslocamento de: %d\n",endereco_logico, num_pagina, deslocamento);
    numero_quadro = verifica_tlb (num_pagina, deslocamento);
    printf("Frame number found: %u\n", numero_quadro);
    posicao_memoria = numero_quadro*256 + deslocamento;
    printf("Memory position found: %u\n", posicao_memoria);
    converte = traduz_endereco(posicao_memoria);
    printf("Data read: %u\n", converte);
    printf("\n");
}

void inicializa_tlb(){
    for(int i = 0; i < quantidade_tlb; i++){
        tlb[i].pagina = 0;
        tlb[i].quadro = 0;
        tlb_vazia =0;
    }
}

int verifica_tlb(int num_pagina, int deslocamento){
    printf("Verificando se a pagina esta na tlb\n");
    if (tlb_vazia == 1 ){
        for(int i = 0; i < quantidade_tlb; i++){

			if(tlb[i].pagina == num_pagina){
				tlb_usada++;
				printf("Pagina %d encontrada na na TLB com numero de quadro %d.\n", tlb[i].pagina, tlb[i].quadro);
				return tlb[i].quadro;
			}
		}
    }
    else {
        printf("Pagina nao encontrada na tlb\n");
        return verifica_tabela(num_pagina, deslocamento);
    }
}

int verifica_tabela(int num_pagina, int deslocamento){
    for (int i = 0; i < quantidade_paginas; i++){
        //nao ha nada na tabela
        if((i == num_pagina) && (quadros_usados == 0)){
            printf("Pagina nao inicializada\n");
            printf("Erro de página \n");
            atualiza_memoria_fisica(num_pagina, quadros_usados);
            tabela_pagina[i] = quadros_usados;
            quadros_usados ++;
			
			atualiza_tlb(num_pagina,tabela_pagina[i]);
			
			return tabela_pagina[i];	
        }
        /*There's no frame for the page we're searching now*/
		else if(i == num_pagina && (tabela_pagina[i] == 0)){
			erro_pagina++;
			printf("Pagina nao inicializada\n");
            printf("Erro de página \n");
			printf("Updating physical memory with the new frame...\n");
			atualiza_memoria_fisica(num_pagina, quadros_usados);
			tabela_pagina[i] = quadros_usados;
			quadros_usados++;
			
			atualiza_tlb(num_pagina,tabela_pagina[i]);
			return tabela_pagina[i];
		}
		/*There's already a frame for the page we're searching*/
		else if(i == num_pagina){
			printf("Page found in memory.\n");
			printf("Updating the tlb with the new page number...\n");
			atualiza_tlb(num_pagina,tabela_pagina[i]);
			return tabela_pagina[i];
		}
    }

}

void atualiza_memoria_fisica(int num_pagina, int quadros_usados){
    fseek(hd, num_pagina*tamanho_pagina, SEEK_SET);// cabeça

    for(int i = 0; i < 256; i++){
        fread(&memoria_fisica[quadros_usados*256 + i], 1, 1, hd);
    }
}

void atualiza_tlb(int num_pagina, int numero_quadro){
    if((numero_quadro == 0) && (ultimo_tlb == 0)){
		tlb[0].pagina = num_pagina;
        tlb[0].quadro = numero_quadro;
        printf("Atualizada a TLB: Posiçao %d, numero da pagina: %d, numero do quadro %d\n", atualiza_tlb, tlb[0].pagina, tlb[0].quadro);
    
    }
    else{
        tlb[ultimo_tlb].pagina = num_pagina;
        tlb[ultimo_tlb].quadro = numero_quadro;
        printf("Atualizada a TLB: Posiçao %d, numero da pagina: %d, numero do quadro %d\n", atualiza_tlb, tlb[ultimo_tlb].pagina, tlb[ultimo_tlb].quadro);
        ultimo_tlb = (ultimo_tlb + 1) % 16;
    }
}

int traduz_endereco(int posicao_memoria){

	int data_read;
	data_read = memoria_fisica[posicao_memoria];
	return data_read;
}
