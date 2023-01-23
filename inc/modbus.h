#ifndef MODBUS_H_
#define MODBUS_H_

int enviaDado(int uart0_filestream, int comando, int dadoInt, float dadoFloat, int8_t dadoByte);
short calculaCRC(unsigned char *commands, int size);

#endif /* MODBUS_H_ */