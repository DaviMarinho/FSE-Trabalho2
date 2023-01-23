#include <signal.h>
#include <stdio.h>
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <string.h>
#include <termios.h>        //Used for UART
#include <stdlib.h>

#include "uart.h"
#include "modbus.h"
#include "extTemperature.h"
#include "pid.h"
#include "gpio.h"
#include "trataCSV.h"

int uart0_filestream;
struct bme280_dev conexaoBME;
DadosCurva linha;

void trata_SIGINT(int signum) {
    encerraPrograma();
}

void encerraPrograma() {
    printf("Encerrando o programa...\n");
    desligaResistencia();
    desligaVentoinha();
    close(uart0_filestream);
    exit(0);
}

void inicializaPrograma() {
    wiringPiSetup();
    desligaResistencia();
    desligaVentoinha();
    conexaoBME = connect_bme();
    uart0_filestream = inicializaUART("/dev/serial0");
    linha = carregaArquivoCSV(linha);
}

int rotinaPID(float tempReferencia, float tempInterna) {
    
    int resultadoEscrita;
    short crc;
    double sinalDeControle;

    pid_atualiza_referencia(tempReferencia);
    sinalDeControle = pid_controle(tempInterna);

    int sinalCont = (int) sinalDeControle;
    resultadoEscrita = enviaDado(uart0_filestream, 1, sinalCont, -1, -1);
    if (resultadoEscrita == -1)
        encerraPrograma();

    controlePWM(sinalCont);

    return sinalCont;
}

void mostraLogs(float TE, float TI, float TR, int sinalControle) {

    printf("\nTE: %.2f   TI: %.2f   TR: %.2f   | sinalControle: %d\n", TE, TI, TR, sinalControle);

}

int main(int argc, const char * argv[]) {

    signal(SIGINT, trata_SIGINT);

    inicializaPrograma();

    unsigned char tx_buffer[20];
    unsigned char *p_tx_buffer;
    p_tx_buffer = &tx_buffer[0];

    double Kp, Ki, Kd;
    int escolhaReferencia;

    int8_t estadoSistemaS;
    int estadoSistemaB;

    int8_t modoControleS;
    int modoControleB;
    int resultado;
    short crc;

    printf("------- Informe os valores para as constantes usadas no controle PID -------\n\n");
    printf("Kp: ");
    scanf("%lf", &Kp);
    printf("Ki: ");
    scanf("%lf", &Ki);
    printf("Kd: ");
    scanf("%lf", &Kd);

    pid_configura_constantes(Kp, Ki, Kd);

    printf("\n\n");

    printf("------- Escolha como considerar a Temperatura de Referencia (TR): -------\n\n");
    printf("1 - DASHBOARD: ");
    printf("atraves do dashboard via UART\n\n");
    printf("2 - CURVA REFLOW: ");
    printf("atraves da curva de temperatura pre-definida em arquivo\n\n");
    printf("9 - Sair\n");

    scanf("%d", &escolhaReferencia);

    switch (escolhaReferencia) {
        case 1:
            modoControleS = 0;
            modoControleB = 0;
            break;
        case 2:
            modoControleS = 1;
            modoControleB = 1;
            break;
        case 9:
            close(uart0_filestream);
            encerraPrograma();
            break;
        default:
            printf("Comando inválido\n");
            encerraPrograma();
            return 0;
    }

    // // Envia o estado do sistema e o modo de controle
    estadoSistemaS = 1;
    resultado = enviaDado(uart0_filestream, 3, -1, -1, estadoSistemaS);
    if (resultado == -1)
        encerraPrograma();
    resultado = enviaDado(uart0_filestream, 4, -1, -1, modoControleS);
    if (resultado == -1)
        encerraPrograma();

    int segundosPassados = 0;
    criaArquivoCSV();
    
    while(1) {
        
        // Contagem de segundos para a curva reflow
        segundosPassados += 3;
        usleep(500*1000);

        float tempInterna = 0.0;
        float tempDashboard = 0.0;
        float tempExterna = 0.0;

        float tempReferencia;

        int sinalCont;
        int comandoLido;      

        p_tx_buffer = &tx_buffer[0];
        *p_tx_buffer++ = 0x01;
        *p_tx_buffer++ = 0x23;
        *p_tx_buffer++ = 0xC3;
        *p_tx_buffer++ = 6;
        *p_tx_buffer++ = 6;
        *p_tx_buffer++ = 0;
        *p_tx_buffer++ = 0;

        crc = calculaCRC(tx_buffer, (p_tx_buffer - &tx_buffer[0])); 
        char crcBytes3[sizeof(short)];
        memcpy(crcBytes3, &crc, sizeof(short));
        *p_tx_buffer++ = crcBytes3[0];
        *p_tx_buffer++ = crcBytes3[1];


        resultado = escreveUART(uart0_filestream, &tx_buffer[0], p_tx_buffer);
        if (resultado == 0) {
            usleep(500*1000);
            comandoLido = leIntUART(uart0_filestream);
        } else
            printf("Erro - Problema na UART.\n");

        // Se for comando relacionado ao MODO DE CONTROLE
        // E Se o MODO DE CONTROLE foi alterado
        if ((comandoLido == 3 && modoControleB != 0) || (comandoLido == 4 && modoControleB != 1)) {
            
            printf("\nComando Recebido: %d\n", comandoLido);
            if (estadoSistemaB == 1)
                printf("Estado atual do sistema: LIGADO\n");
            if (estadoSistemaB == 0)
                printf("Estado atual do sistema: DESLIGADO\n");

            if (comandoLido == 3) {
                modoControleS = 0;
                modoControleB = 0;
                printf("Modo de controle alterado para: DASHBOARD\n");
            }                
            if (comandoLido == 4) {
                modoControleS = 1;
                modoControleB = 1;
                segundosPassados = 0;
                printf("Modo de controle alterado para: CURVA REFLOW\n");
            }
            printf("\n");

            resultado = enviaDado(uart0_filestream, 4, -1, -1, modoControleS);
            if (resultado == -1)
                encerraPrograma();
        }
        

        // Se for comando relacionado ao ESTADO DO SISTEMA
        // Se o ESTADO DO SISTEMA foi alterado
        if ((comandoLido == 1 && estadoSistemaB != 1) || (comandoLido == 2 && estadoSistemaB != 0)) {
            printf("\nComando Recebido: %d\n", comandoLido);
            if (comandoLido == 1) {
                estadoSistemaS = 1;
                estadoSistemaB = 1;
                printf("Estado do sistema alterado para: LIGADO\n");
            }                
            if (comandoLido == 2) {
                estadoSistemaS = 0;
                estadoSistemaB = 0;
                printf("Estado do sistema alterado para: DESLIGADO\n");
            }

            if (modoControleB == 1)
                printf("Modo de controle atual do sistema: CURVA REFLOW\n");
            if (modoControleB == 0)
                printf("Modo de controle atual do sistema: DASHBOARD\n");
            printf("\n");

            resultado = enviaDado(uart0_filestream, 3, -1, -1, estadoSistemaS);
            if (resultado == -1)
                encerraPrograma();
        }

        // Se LIGOU o sistema, inicia rotina de controle
        if (estadoSistemaB == 1) {
            // Ler TEMPERATURA INTERNA
            p_tx_buffer = &tx_buffer[0];
            *p_tx_buffer++ = 0x01;
            *p_tx_buffer++ = 0x23;
            *p_tx_buffer++ = 0xC1;
            *p_tx_buffer++ = 6;
            *p_tx_buffer++ = 6;
            *p_tx_buffer++ = 0;
            *p_tx_buffer++ = 0;

            crc = calculaCRC(tx_buffer, (p_tx_buffer - &tx_buffer[0])); 
            char crcBytes[sizeof(short)];
            memcpy(crcBytes, &crc, sizeof(short));
            *p_tx_buffer++ = crcBytes[0];
            *p_tx_buffer++ = crcBytes[1];

            resultado = escreveUART(uart0_filestream, &tx_buffer[0], p_tx_buffer);
            if (resultado == 0) {
                usleep(500*1000);
                tempInterna = leFloatUART(uart0_filestream);
            } else
                printf("Erro - Problema na UART.\n");

            // Ler TEMPERATURA REFERENCIA (TR)
            // Caso Dashboard (TR)
            if (modoControleB == 0) {
                p_tx_buffer = &tx_buffer[0];
                *p_tx_buffer++ = 0x01;
                *p_tx_buffer++ = 0x23;
                *p_tx_buffer++ = 0xC2;
                *p_tx_buffer++ = 6;
                *p_tx_buffer++ = 6;
                *p_tx_buffer++ = 0;
                *p_tx_buffer++ = 0;

                crc = calculaCRC(tx_buffer, (p_tx_buffer - &tx_buffer[0])); 
                char crcBytes[sizeof(short)];
                memcpy(crcBytes, &crc, sizeof(short));
                *p_tx_buffer++ = crcBytes[0];
                *p_tx_buffer++ = crcBytes[1];
                
                resultado = escreveUART(uart0_filestream, &tx_buffer[0], p_tx_buffer);
                if (resultado == 0) {
                    usleep(500*1000);
                    tempDashboard = leFloatUART(uart0_filestream);
                } else
                    printf("Erro - Problema na UART.\n");

                sinalCont = rotinaPID(tempDashboard, tempInterna);
                tempReferencia = tempDashboard;

            // Caso CURVA REFLOW (TR)
            } else if (modoControleB == 1) {
                float tempReferenciaCurva;
            
                if (segundosPassados >= linha.colTempo[9])
                    tempReferenciaCurva = linha.colTemperatura[9];
                else if (segundosPassados >= linha.colTempo[8])
                    tempReferenciaCurva = linha.colTemperatura[8];
                else if (segundosPassados >= linha.colTempo[7])
                    tempReferenciaCurva = linha.colTemperatura[7];
                else if (segundosPassados >= linha.colTempo[6])
                    tempReferenciaCurva = linha.colTemperatura[6];
                else if (segundosPassados >= linha.colTempo[5])
                    tempReferenciaCurva = linha.colTemperatura[5];
                else if (segundosPassados >= linha.colTempo[4])
                    tempReferenciaCurva = linha.colTemperatura[4];
                else if (segundosPassados >= linha.colTempo[3])
                    tempReferenciaCurva = linha.colTemperatura[3];
                else if (segundosPassados >= linha.colTempo[2])
                    tempReferenciaCurva = linha.colTemperatura[2];
                else if (segundosPassados >= linha.colTempo[1])
                    tempReferenciaCurva = linha.colTemperatura[1];
                else if (segundosPassados >= linha.colTempo[0])
                    tempReferenciaCurva = linha.colTemperatura[0];

                sinalCont = rotinaPID(tempReferenciaCurva, tempInterna);
                tempReferencia = tempReferenciaCurva;

                resultado = enviaDado(uart0_filestream, 2, -1, tempReferenciaCurva, -1);
                if (resultado == -1)
                    encerraPrograma();
            }

            // Ler TEMPERATURA EXTERNA (TE)
            tempExterna = get_current_temperature(&conexaoBME);

            escreveArquivoCSV(tempExterna, tempInterna, tempReferencia, sinalCont);
            mostraLogs(tempExterna, tempInterna, tempReferencia, sinalCont);

        } else if (estadoSistemaB == 0) {
            printf("Sistema está desligado.\n");
        }


    }
    
    close(uart0_filestream);
    return 0;
}
