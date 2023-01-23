#include <string.h>
#include <stdio.h>
#include <time.h>
#include "trataCSV.h"

DadosCurva carregaArquivoCSV(DadosCurva linha) {
    FILE * arq_curva;

    arq_curva = fopen("src/curva_reflow.csv", "r");
    // if(arq_curva != NULL){
    //     // printf("Arquivo aberto com sucesso!\n");
    // } else {
    //     // printf("Erro ao abrir o arquivo!\n");
    // }

    int contador = 0;
    while(fscanf(arq_curva, "%d,%f", &linha.colTempo[contador], &linha.colTemperatura[contador]) == 2){
        contador++;
    }

    fclose(arq_curva);
    // if(fclose(arq_curva) == 0)
    //     // printf("Arquivo fechado com sucesso\n");
    // else
    //     // printf("Erro ao fechar arquivo\n");

    // printf("Arquivo CSV carregado com sucesso!");

    return linha;
}


void criaArquivoCSV() {
    FILE * arq_medicoes;

    arq_medicoes = fopen("resultado.csv", "w");
    // if(arq_medicoes != NULL){
    //     // printf("Arquivo criado com sucesso!\n");
    // } else {
    //     // printf("Erro ao criar o arquivo!\n");
    // }

    fprintf(arq_medicoes, "Data, Temp Externa, Temp Interna, Temp Referencia, Ventoinha, Resistor\n");

    fclose(arq_medicoes);
    // if(fclose(arq_medicoes) == 0)
    //     // printf("Arquivo fechado com sucesso\n");
    // else
    //     // printf("Erro ao fechar arquivo\n");

    // printf("Arquivo CSV criado com sucesso!");
}


void escreveArquivoCSV(float TE, float TI, float TR, int sinalControle) {
    FILE * arq_medicoes;

    arq_medicoes = fopen("resultado.csv", "a");
    // if(arq_medicoes != NULL){
    //     // printf("Arquivo aberto com sucesso!\n");
    // } else {
    //     // printf("Erro ao abrir o arquivo!\n");
    // }

    int resistor = 0;
    int ventoinha = 0;

    if (sinalControle > 0) {
        resistor = sinalControle;
    } else if (sinalControle < 0) {
        if (sinalControle > -40)
            ventoinha = 40;
        ventoinha = sinalControle * -1;
    }

    struct tm *dataHora;
    time_t segundos;
    time(&segundos); 
    dataHora = localtime(&segundos); 

    fprintf(arq_medicoes, "%d-%d-%d %d:%d:%d, %.2f, %.2f, %.2f, %.2d, %.2d\n", dataHora->tm_mday, dataHora->tm_mon+1, dataHora->tm_year+1900, dataHora->tm_hour, dataHora->tm_min, dataHora->tm_sec, TE, TI, TR, ventoinha, resistor);

    
    fclose(arq_medicoes);
    // if(fclose(arq_medicoes) == 0)
    //     // printf("Arquivo fechado com sucesso\n");
    // else
    //     // printf("Erro ao fechar arquivo\n");

    // printf("Arquivo CSV escrito com sucesso!");
}
