#include <stdio.h>
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <string.h>
#include <termios.h>        //Used for UART
#include <stdlib.h>
#include "uart.h"

int escreveUART(int uartFilestream, char *txBuffer, char *pTxBuffer) {
    if (uartFilestream != -1)
    {
        int count = write(uartFilestream, txBuffer, (pTxBuffer - txBuffer));
        if (count < 0)
        {
            printf("UART TX error\n");
            return -1;
        }
        else
        {
            return 0;
        }
    }
    else
        return -1;
}


int leIntUART(int uartFilestream) {
    if (uartFilestream != -1)
    {
        // Read up to 255 characters from the port if they are there
        unsigned char *rx_buffer = malloc(256);
        int rx_length = read(uartFilestream, (void*)rx_buffer, 255); //Filestream, buffer to store in, number of bytes to read (max)
        if (rx_length < 0)
        {
            printf("Erro na leitura.\n"); //An error occured (will occur if there are no bytes)
            return 0;
        }
        else if (rx_length == 0)
        {
            printf("Nenhum dado disponível.\n"); //No data waiting
            return 0;
        }
        else
        {
            int dado = 0;
            memcpy(&dado, &rx_buffer[3], 4);
            return dado;
        }
    }
    else
    {
        printf("Erro - Problema na UART.\n");
        return 0;
    }
}

float leFloatUART(int uartFilestream) {
    if (uartFilestream != -1)
    {
        // Read up to 255 characters from the port if they are there
        unsigned char *rx_buffer = malloc(256);
        int rx_length = read(uartFilestream, (void*)rx_buffer, 255); //Filestream, buffer to store in, number of bytes to read (max)
        if (rx_length < 0)
        {
            printf("Erro na leitura.\n"); //An error occured (will occur if there are no bytes)
            return 0;
        }
        else if (rx_length == 0)
        {
            printf("Nenhum dado disponível.\n"); //No data waiting
            return 0;
        }
        else
        {
            float dado = 0;
            memcpy(&dado, &rx_buffer[3], sizeof(float));
            return dado;
        }
    }
    else
    {
        printf("Erro - Problema na UART.\n");
        return 0;
    }
}

int inicializaUART(char dispositivo[]) {

    int uart0_filestream = -1;

    uart0_filestream = open(dispositivo, O_RDWR | O_NOCTTY | O_NDELAY);      //Open in non blocking read/write mode
    if (uart0_filestream == -1)
    {
        printf("Erro - Não foi possível iniciar a UART.\n");
    }
    else
    {
        printf("UART inicializada!\n");
    }    
    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;     //<Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);

    return uart0_filestream;
}