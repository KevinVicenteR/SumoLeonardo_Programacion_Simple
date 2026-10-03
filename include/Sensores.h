#ifndef SENSORES_H
#define SENSORES_H

#include <Arduino.h>

constexpr uint8_t IZQUIERDO = 0;
constexpr uint8_t DERECHO = 1;
constexpr uint8_t FRONTAL_IZQ = 0;
constexpr uint8_t FRONTAL_CENTRO = 1;
constexpr uint8_t FRONTAL_DER = 2;
constexpr uint8_t PISO_IZQ = 0;
constexpr uint8_t PISO_DER = 1;


class Sensores{
    private:
    
    static constexpr uint8_t FRONTAL_IZQUIERDO_PIN = A0;
    static constexpr uint8_t FRONTAL_CENTRO_PIN = A1;
    static constexpr uint8_t FRONTAL_DERECHO_PIN = A2;
    static constexpr uint8_t LATERAL_IZQUIERDO_PIN = A3;
    static constexpr uint8_t LATERAL_DERECHO_PIN = A4;
    static constexpr uint8_t PISO_IZQUIERDO_PIN = A5;
    static constexpr uint8_t PISO_DERECHO_PIN = A6;

    static const int UMBRAL_BORDE = 120;
    static const int HISTERESIS = 15;
    bool estadoBordeIzquierdo = false;
    bool estadoBordeDerecho = false;

    public:

    void iniciar() const {
        pinMode(FRONTAL_IZQUIERDO_PIN, INPUT);
        pinMode(FRONTAL_CENTRO_PIN, INPUT);
        pinMode(FRONTAL_DERECHO_PIN, INPUT);
        pinMode(LATERAL_IZQUIERDO_PIN, INPUT);
        pinMode(LATERAL_DERECHO_PIN, INPUT);
        pinMode(PISO_IZQUIERDO_PIN, INPUT);
        pinMode(PISO_DERECHO_PIN, INPUT);
    }

    int getSensorPiso(uint8_t sensor) const {
        return analogRead(sensor == PISO_IZQ ? PISO_IZQUIERDO_PIN : PISO_DERECHO_PIN);
    }
    int getSensorFrontal(uint8_t sensor) const {
        switch (sensor) {
            case FRONTAL_IZQ: return digitalRead(FRONTAL_IZQUIERDO_PIN);
            case FRONTAL_CENTRO: return digitalRead(FRONTAL_CENTRO_PIN);
            default: return digitalRead(FRONTAL_DERECHO_PIN);
        }
    }
    int getSensorLateral(uint8_t sensor) const {
        return digitalRead(sensor == IZQUIERDO ? LATERAL_IZQUIERDO_PIN : LATERAL_DERECHO_PIN);
    }

    // El borde blanco mide solo 1 cm: se activa apenas cruza el umbral (sin restar
    // histeresis) para no perder la linea a alta velocidad; la histeresis solo
    // aplica al soltar el estado.
    bool esBorde(uint8_t sensor){
        bool& estado = sensor == PISO_IZQ ? estadoBordeIzquierdo : estadoBordeDerecho;
        int valor = getSensorPiso(sensor);
        if (!estado && valor < UMBRAL_BORDE) {
            estado = true;
        } else if (estado && valor > (UMBRAL_BORDE + HISTERESIS)) {
            estado = false;
        }
        return estado;
    }

    bool esEnemigoFRONTAL(uint8_t sensor) const {
        return getSensorFrontal(sensor) == HIGH;
    }
    
    bool esEnemigoLATERAL(uint8_t sensor) const {
        return getSensorLateral(sensor) == HIGH;
    }

};

#endif
