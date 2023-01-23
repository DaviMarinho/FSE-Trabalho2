#include <softPwm.h>
#include <wiringPi.h>

#define RESIS_PINO 4
#define VENT_PINO 5


void ligaResistencia(int valorResistor) {
    pinMode(RESIS_PINO, OUTPUT);
    softPwmCreate(RESIS_PINO, 0, 100);
    softPwmWrite(RESIS_PINO, valorResistor);
}


void ligaVentoinha(int valorVentoinha) {
    pinMode(VENT_PINO, OUTPUT);
    softPwmCreate(VENT_PINO, 0, 100);
    softPwmWrite(VENT_PINO, valorVentoinha);
}

void desligaResistencia() {
    pinMode(RESIS_PINO, OUTPUT);
    softPwmCreate(RESIS_PINO, 0, 100);
    softPwmWrite(RESIS_PINO, 0);
}

void desligaVentoinha() {
    pinMode(VENT_PINO, OUTPUT);
    softPwmCreate(VENT_PINO, 0, 100);
    softPwmWrite(VENT_PINO, 0);
}

void controlePWM(int sinalControle) {
    if (sinalControle > 0) {
        ligaResistencia(sinalControle);
        desligaVentoinha();
    } else {
        if (sinalControle < -40) {
            ligaVentoinha(sinalControle * -1);
        }
        else {
            ligaVentoinha(40);
        }
        desligaResistencia();
    }
}

