#ifndef GPIO_H_
#define GPIO_H_

void ligaResistencia(int valorResistor);
void ligaVentoinha(int valorVentoinha);
void desligaResistencia();
void desligaVentoinha();
void controlePWM(int sinalControle);

#endif /* GPIO_H_ */