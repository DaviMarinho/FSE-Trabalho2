#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART
#include <string.h>

#include "modbus.h"
#include "crc16.h"
#include "uart.h"

// Retorna o resultado da escrita na UART
int enviaDado(int uart0_filestream, int comando, int dadoInt, float dadoFloat, int8_t dadoByte) {

    unsigned char tx_buffer[20];
    unsigned char *p_tx_buffer;
    p_tx_buffer = &tx_buffer[0];

    // 1 int / 2 float / 3 byte
    int tipoDado;

    //    Comandos
    *p_tx_buffer++ = 0x01;
    *p_tx_buffer++ = 0x16;

    switch(comando) {
        case 1:
            *p_tx_buffer++ = 0xD1;
            tipoDado = 1;
            break;
        case 2:
            *p_tx_buffer++ = 0xD2;
            tipoDado = 2;
            break;
        case 3:
            *p_tx_buffer++ = 0xD3;
            tipoDado = 3;
            break;
        case 4:
            *p_tx_buffer++ = 0xD4;
            tipoDado = 3;
            break;
        case 5:
            *p_tx_buffer++ = 0xD5;
            tipoDado = 3;
            break;
        case 6:
            *p_tx_buffer++ = 0xD6;
            tipoDado = 3;
            break;
    }

    //    Matricula
    *p_tx_buffer++ = 6;
    *p_tx_buffer++ = 6;
    *p_tx_buffer++ = 0;
    *p_tx_buffer++ = 0;


    //    Dado
    if (tipoDado == 1) {
        char dadoIntBytes[sizeof(int)];
        memcpy(dadoIntBytes, &dadoInt, sizeof(int));
        *p_tx_buffer++ = dadoIntBytes[0];
        *p_tx_buffer++ = dadoIntBytes[1];
        *p_tx_buffer++ = dadoIntBytes[2];
        *p_tx_buffer++ = dadoIntBytes[3];
    } else if (tipoDado == 2) {    
        char dadoFloatBytes[sizeof(float)];
        memcpy(dadoFloatBytes, &dadoFloat, sizeof(float));
        *p_tx_buffer++ = dadoFloatBytes[0];
        *p_tx_buffer++ = dadoFloatBytes[1];
        *p_tx_buffer++ = dadoFloatBytes[2];
        *p_tx_buffer++ = dadoFloatBytes[3];
    } else if (tipoDado == 3)
        *p_tx_buffer++ = dadoByte;

    //    CRC
    int crc = calculaCRC(tx_buffer, (p_tx_buffer - &tx_buffer[0])); 
    char crcBytes[sizeof(short)];
    memcpy(crcBytes, &crc, sizeof(short));
    *p_tx_buffer++ = crcBytes[0];
    *p_tx_buffer++ = crcBytes[1];

    int resultado = escreveUART(uart0_filestream, &tx_buffer[0], p_tx_buffer);
    if (resultado == -1) {
        printf("Erro - Problema na UART.\n");
        return -1;   
    }
    return 0;
}

// solicitaDado ()


short calculaCRC(unsigned char *commands, int size) {
    return calcula_CRC(commands, size);
}