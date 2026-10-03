#ifndef MOTORES_H
#define MOTORES_H

#include <Arduino.h>

class Motores{
    public:

    static constexpr uint8_t IZQUIERDO = 0;
    static constexpr uint8_t DERECHO = 1;
    static constexpr uint8_t AVANZAR = 0;
    static constexpr uint8_t RETROCEDER = 1;
    static constexpr uint8_t PARAR = 2;
    static constexpr uint8_t GIRAR_IZQUIERDA = 3;
    static constexpr uint8_t GIRAR_DERECHA = 4;
    static constexpr uint8_t ATACAR =5;

    static constexpr uint8_t VELOCIDAD_BAJA = 0;
    static constexpr uint8_t VELOCIDAD_MEDIA = 1;
    static constexpr uint8_t VELOCIDAD_ALTA = 2;

    static constexpr uint8_t PWM_BAJO = 100;
    static constexpr uint8_t PWM_MEDIO = 150;
    static constexpr uint8_t PWM_ALTO = 255;

    // Datos fisicos del robot: motores de 750 rpm, rueda de ~3 cm y ~8 cm entre ruedas.
    // FACTOR_CARGA estima la velocidad real frente a la teorica (friccion, peso, bateria)
    // y TIEMPO_RESPUESTA_MS cubre la aceleracion/inversion del motor. Calibrar en pista.
    static constexpr float RPM_MOTOR = 750.0f;
    static constexpr float DIAMETRO_RUEDA_CM = 3.0f;
    static constexpr float SEPARACION_RUEDAS_CM = 8.0f;
    static constexpr float FACTOR_CARGA = 0.6f;
    static constexpr unsigned long TIEMPO_RESPUESTA_MS = 40;

    // Dohyo circular de 70 cm de diametro con borde blanco de 1 cm.
    static constexpr float DIAMETRO_DOHYO_CM = 70.0f;
    static constexpr float RADIO_DOHYO_CM = DIAMETRO_DOHYO_CM / 2.0f;
    static constexpr float ANCHO_BORDE_CM = 1.0f;
    static constexpr float VELOCIDAD_MAX_CM_S = RPM_MOTOR / 60.0f * PI * DIAMETRO_RUEDA_CM * FACTOR_CARGA;

    // Tiempo para recorrer una distancia en linea recta al PWM indicado
    // (se asume velocidad proporcional al PWM).
    static constexpr unsigned long msParaDistancia(float cm, uint8_t pwm = PWM_ALTO) {
        return TIEMPO_RESPUESTA_MS + static_cast<unsigned long>(cm * PWM_ALTO / pwm / VELOCIDAD_MAX_CM_S * 1000.0f);
    }

    // Tiempo a PWM_ALTO para girar sobre su eje (ruedas en sentido opuesto).
    static constexpr unsigned long msParaGiro(float grados) {
        return msParaDistancia(grados / 360.0f * PI * SEPARACION_RUEDAS_CM);
    }

    static constexpr uint8_t IN1 = 0;
    static constexpr uint8_t IN2 = 1;
    static constexpr uint8_t PWM = 2;

    private:

    struct PinesMotor {
        uint8_t in1;
        uint8_t in2;
        uint8_t pwm;
    };
    const PinesMotor motorIzquierdo = {5, 6, 13};
    const PinesMotor motorDerecho = {9, 10, 11};


    public:

    void iniciar() const {
        pinMode(motorIzquierdo.in1, OUTPUT);
        pinMode(motorIzquierdo.in2, OUTPUT);
        pinMode(motorIzquierdo.pwm, OUTPUT);
        pinMode(motorDerecho.in1, OUTPUT);
        pinMode(motorDerecho.in2, OUTPUT);
        pinMode(motorDerecho.pwm, OUTPUT);
    }

    void setDireccion(uint8_t motor, uint8_t movimiento) const {
        const PinesMotor& pines = (motor == IZQUIERDO) ? motorIzquierdo : motorDerecho;
        switch(movimiento){
            case AVANZAR:
                digitalWrite(pines.in1, HIGH);
                digitalWrite(pines.in2, LOW);
                break;
            case RETROCEDER:
                digitalWrite(pines.in1, LOW);
                digitalWrite(pines.in2, HIGH);
                break;
            case ATACAR:
                digitalWrite(pines.in1, HIGH);
                digitalWrite(pines.in2, HIGH);
                break;
            case GIRAR_DERECHA:
                if (motor == IZQUIERDO) {
                    digitalWrite(pines.in1, HIGH);
                    digitalWrite(pines.in2, LOW);
                } else {
                    digitalWrite(pines.in1, LOW);
                    digitalWrite(pines.in2, HIGH);
                }
                break;
            case GIRAR_IZQUIERDA:
                if (motor == IZQUIERDO) {
                    digitalWrite(pines.in1, LOW);
                    digitalWrite(pines.in2, HIGH);
                } else {
                    digitalWrite(pines.in1, HIGH);
                    digitalWrite(pines.in2, LOW);
                }
                break;
            default:
                digitalWrite(pines.in1, LOW);
                digitalWrite(pines.in2, LOW);
                break;
        }
    }

    uint8_t getPinMotorIzquierdo() const {
        return motorIzquierdo.pwm;
    }
    uint8_t getPinMotorDerecho() const {
        return motorDerecho.pwm;
    }
    int getPWM(int velocidad) const {
        switch(velocidad){
            case VELOCIDAD_BAJA:
                return PWM_BAJO;
            case VELOCIDAD_MEDIA:
                return PWM_MEDIO;
            case VELOCIDAD_ALTA:
                return PWM_ALTO;
            default:
                return 0;
        }
    }

    int ControlPID(int error, int velocidad) const {
        int pwm = getPWM(velocidad);
        int control = error * 2;
        int pwmIzquierdo = constrain(pwm + control, 0, 255);
        int pwmDerecho = constrain(pwm - control, 0, 255);
        analogWrite(getPinMotorIzquierdo(), pwmIzquierdo);
        analogWrite(getPinMotorDerecho(), pwmDerecho);
        return (pwmIzquierdo + pwmDerecho) / 2;
    }


};

#endif
