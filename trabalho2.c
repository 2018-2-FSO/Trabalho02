#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//1111 1111 0000 0000
#define mascara_num_pag 65280
//0000 0000 1111 1111
#define mascara_deslocamento 255
#define quantidade_paginas 256
#define tamanho_pagina 256
#define quantidade_tlb 16

void transforma_endereco(int endereco_logico);
uint8_t verifica_tlb(uint8_t num_pagina2);
void inicializa_tlb();
uint8_t verifica_tabela(uint8_t num_pagina3);
void atualiza_memoria_fisica(int num_pagina4, int quadros_usados);
void atualiza_tlb(int num_pagina, int numero_quadro);
int traduz_endereco(int posicao_memoria);
int tlb_vazia(void);

uint8_t tabela_pagina[quantidade_paginas];
uint8_t memoria_fisica[quantidade_paginas*tamanho_pagina];
int quant_acesso_memoria = 0;
int  erro_pagina =0;
double porc_erro_pagina =0, porc_suscesso_tlb=0;
uint8_t ultimo_tlb = 0;
int tlb_usada = 0;
uint8_t quadros_usados = 0;
int num_pagina, deslocamento, numero_quadro, posicao_memoria, converte;

typedef struct tlb {
  uint8_t pagina;
  uint8_t quadro;
} Tlb;

Tlb tlb[quantidade_tlb];

FILE *arq_enderecos_logicos, *hd;


int main(int argc, char *argv[]) {
    int endereco_logico;
    inicializa_tlb();

    if (argc != 2) {
        fprintf(stderr,"Para Usar: ./trabalho2 addresses.txt\n");
        return -1;
    }

    hd = fopen("BACKING_STORE.bin", "rb");
    arq_enderecos_logicos= fopen(argv[1], "r");

    if(arq_enderecos_logicos == NULL){
		printf("Erro ao abrir o arquivo!\n");
		return -1;
	}

    if(hd == NULL){
		printf("Erro ao abrir o arquivo!\n");
		return -1;
	}

    do{
        while(fscanf(arq_enderecos_logicos, "%d", &endereco_logico )!= EOF){
            quant_acesso_memoria ++;
            printf("\n");
            printf("lendo o valor : %d\n", endereco_logico);
            transforma_endereco(endereco_logico);
    
        }

        porc_erro_pagina = erro_pagina/(quant_acesso_memoria*1.0);
	    porc_suscesso_tlb = tlb_usada/(quant_acesso_memoria*1.0);

		printf("Porcentagem de erro de pagina: %.2lf\n", porc_erro_pagina*100);
		printf("Porcentagem de sucesso da tlb: %.2lf\n", porc_suscesso_tlb*100);

        fclose(hd);
        fclose(arq_enderecos_logicos);
        return 0;
    }while(arq_enderecos_logicos != NULL && hd != NULL);
    

}

void transforma_endereco(int endereco_logico){
    //fazer a operaçao and para zerar ultimos 8 bits e deslocar o numero para a direita 8 vezes
    num_pagina = (endereco_logico & mascara_num_pag) >> 8;
    //fazer a operação and para zerar os 8 primeiros numeros
    deslocamento = (endereco_logico & mascara_deslocamento);//offset
    printf("Numero da pagina: %d, numero de deslocamento: %d\n", num_pagina, deslocamento);
    numero_quadro = verifica_tlb (num_pagina);
    posicao_memoria = numero_quadro*256 + deslocamento;
    converte = traduz_endereco(posicao_memoria);
    printf("Numero de quadro encontrado: %d, Posiçao na memoria encontrada: %d\n", numero_quadro, posicao_memoria);
    printf("Endereço na memoria fisica: %d\n",converte);
}

void inicializa_tlb(){
    for(int i = 0; i < quantidade_tlb; i++){
        tlb[i].pagina = 0;
        tlb[i].quadro = 0;
    }
}

uint8_t verifica_tlb(uint8_t num_pagina2){
    printf("Verificando se a pagina esta na tlb\n");
    if (tlb_vazia() == 1 ){
        for(uint8_t i = 0; i < quantidade_tlb; i++){

			if(tlb[i].pagina == num_pagina2){
				tlb_usada++;
				printf("Pagina %d encontrada na na TLB com numero de quadro %d.\n", tlb[i].pagina, tlb[i].quadro);
				return tlb[i].pagina;
			}
		}
    }
    printf("Pagina nao encontrada na tlb\n");
    return verifica_tabela(num_pagina2);
}

uint8_t verifica_tabela(uint8_t num_pagina3){
    for (uint8_t i = 0; i < quantidade_paginas; i++){
        //nao ha nada na tabela
        if((i == num_pagina3) && (quadros_usados == 0)){
            printf("Pagina nao inicializada\n");
            printf("Erro de página \n");
            atualiza_memoria_fisica(num_pagina3, quadros_usados);
            tabela_pagina[i] = quadros_usados;
            quadros_usados ++;
			
			atualiza_tlb(num_pagina3,tabela_pagina[i]);
			
			return tabela_pagina[i];	
        }
        /*There's no frame for the page we're searching now*/
		else if(i == num_pagina3 && (tabela_pagina[i] == 0)){
			erro_pagina++;
			printf("Pagina nao inicializada\n");
            printf("Erro de página \n");
			printf("Atualizando memoria fisica \n");
			atualiza_memoria_fisica(num_pagina3, quadros_usados);
			tabela_pagina[i] = quadros_usados;
			quadros_usados++;
			
			atualiza_tlb(num_pagina3,tabela_pagina[i]);
			return tabela_pagina[i];
		}
		/*There's already a frame for the page we're searching*/
		else if(i == num_pagina3){
			printf("Atualizando tlb\n");
			atualiza_tlb(num_pagina3,tabela_pagina[i]);
			return tabela_pagina[i];
		}
    }

}

void atualiza_memoria_fisica(int num_pagina4, int quadros_usados){

    fseek(hd, num_pagina4*tamanho_pagina, SEEK_SET);// cabeça

    for(int i = 0; i < 256; i++){
        fread(&memoria_fisica[quadros_usados*256 + i], 1, 1, hd);
    }
}

void atualiza_tlb(int num_pagina5, int numero_quadro){
    if((numero_quadro == 0) && (ultimo_tlb == 0)){
		tlb[0].pagina = num_pagina5;
        tlb[0].quadro = numero_quadro;
        printf("Atualizada a TLB: Posiçao %d, numero da pagina: %d, numero do quadro %d\n", ultimo_tlb, tlb[0].pagina, tlb[0].quadro);
        ultimo_tlb ++;
    }
    else{
        tlb[ultimo_tlb].pagina = num_pagina5;
        tlb[ultimo_tlb].quadro = numero_quadro;
        printf("Atualizada a TLB: Posiçao %d, numero da pagina: %d, numero do quadro %d\n", ultimo_tlb, tlb[ultimo_tlb].pagina, tlb[ultimo_tlb].quadro);
        ultimo_tlb = (ultimo_tlb + 1) % 16;
    }
}

int traduz_endereco(int posicao_memoria){

	int data_read;
	data_read = memoria_fisica[posicao_memoria];
	return data_read;
}

int tlb_vazia(void){
	
	int true1 = (tlb[0].pagina == 0) && (tlb[0].quadro == 0);
	int true2 = (tlb[1].pagina == 0) && (tlb[1].quadro == 0);

	if(true1 == 1 && true2 == 1){
		return 0;
	}
	else{
		return 1;
	}
}