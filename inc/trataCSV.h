#ifndef TRATASCV_H_
#define TRATASCV_H_

typedef struct {
    int colTempo[10];
    float colTemperatura[10];
} DadosCurva;

DadosCurva carregaArquivoCSV(DadosCurva linha);
void criaArquivoCSV();
void escreveArquivoCSV(float TE, float TI, float TR, int sinalControle);

#endif /* TRATASCV_H_ */
