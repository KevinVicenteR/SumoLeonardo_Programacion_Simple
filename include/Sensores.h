#ifndef SENSORES_H
#define SENSORES_H

#include <Arduino.h>

enum { IZQUIERDO = 0, DERECHO = 1 };
enum { FRONTAL_IZQ = 0, FRONTAL_CENTRO = 1, FRONTAL_DER = 2 };
enum { PISO_IZQ = 0, PISO_DER = 1 };


class Sensores{
    private:
    
    int Sensores_Frontales[3]= {A0, A1, A2}; 
    int Sensores_Laterales[2]= {A3, A4}; 
    int Sensores_De_Piso[2]= {A5, A6}; 
    
    static const int UMBRAL_BORDE = 120;
    static const int HISTERESIS = 15;
    bool estadoBorde[2] = {false, false};

    public:

    void iniciar(){
        for (int i = 0; i < 3; i++) pinMode(Sensores_Frontales[i], INPUT);
        for (int i = 0; i < 2; i++) pinMode(Sensores_Laterales[i], INPUT);
        for (int i = 0; i < 2; i++) pinMode(Sensores_De_Piso[i], INPUT);
    }

    int getSensorPiso(int sensor){
        return analogRead(Sensores_De_Piso[sensor]);
    }
    int getSensorFrontal(int sensor){
        return digitalRead(Sensores_Frontales[sensor]);
    }
    int getSensorLateral(int sensor){
        return digitalRead(Sensores_Laterales[sensor]);
    }


    // El borde blanco mide solo 1 cm: se activa apenas cruza el umbral (sin restar
    // histeresis) para no perder la linea a alta velocidad; la histeresis solo
    // aplica al soltar el estado.
    bool esBorde(int sensor){
        int valor = getSensorPiso(sensor);
        if (!estadoBorde[sensor] && valor < UMBRAL_BORDE) {
            estadoBorde[sensor] = true;
        } else if (estadoBorde[sensor] && valor > (UMBRAL_BORDE + HISTERESIS)) {
            estadoBorde[sensor] = false;
        }
        return estadoBorde[sensor];
    }

    bool esEnemigoFRONTAL(int sensor){
        return getSensorFrontal(sensor) == HIGH;
    }
    
    bool esEnemigoLATERAL(int sensor){
        return getSensorLateral(sensor) == HIGH;
    }

};

#endif