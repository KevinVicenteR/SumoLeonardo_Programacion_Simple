#ifndef MOTORES_H
#define MOTORES_H

#include <Arduino.h>

class Motores{
    public:

    enum { IZQUIERDO = 0, DERECHO = 1 };
    enum { AVANZAR = 0, RETROCEDER = 1, PARAR = 2, GIRAR_IZQUIERDA = 3, GIRAR_DERECHA = 4 ,ATACAR =5 };

    enum { VELOCIDAD_BAJA = 0, VELOCIDAD_MEDIA = 1, VELOCIDAD_ALTA = 2 };

    enum { PWM_BAJO = 100, PWM_MEDIO = 150, PWM_ALTO = 255 };

    enum { IN1 = 0, IN2 = 1, PWM = 2 }; 
    
    private:
    
    int Motor_Izquierdo_Con_PWM[3] = {5, 6, 13};
    int Motor_Derecho_Con_PWM[3] = {9, 10, 11};
    

    public:

    void iniciar(){
        for (int indice = 0; indice < 3; indice++) {
            pinMode(Motor_Izquierdo_Con_PWM[indice], OUTPUT);
            pinMode(Motor_Derecho_Con_PWM[indice], OUTPUT);
        }
    }

    void setDireccion(int motor, int movimiento){
        int *pines = (motor == IZQUIERDO) ? Motor_Izquierdo_Con_PWM : Motor_Derecho_Con_PWM;
        switch(movimiento){
            case AVANZAR:
                digitalWrite(pines[IN1], HIGH);
                digitalWrite(pines[IN2], LOW);
                break;
            case RETROCEDER:
                digitalWrite(pines[IN1], LOW);
                digitalWrite(pines[IN2], HIGH);
                break;
            case ATACAR:
                digitalWrite(pines[IN1], HIGH);
                digitalWrite(pines[IN2], HIGH);
                break;
            case GIRAR_DERECHA:
                if (motor == IZQUIERDO) {
                    digitalWrite(pines[IN1], HIGH);
                    digitalWrite(pines[IN2], LOW);
                } else {
                    digitalWrite(pines[IN1], LOW);
                    digitalWrite(pines[IN2], HIGH);
                }
                break;
            case GIRAR_IZQUIERDA:
                if (motor == IZQUIERDO) {
                    digitalWrite(pines[IN1], LOW);
                    digitalWrite(pines[IN2], HIGH);
                } else {
                    digitalWrite(pines[IN1], HIGH);
                    digitalWrite(pines[IN2], LOW);
                }
                break;
            default: 
                digitalWrite(pines[IN1], LOW);
                digitalWrite(pines[IN2], LOW);
                break;
        }
    }

    int getPinMotorIzquierdo(){
        return Motor_Izquierdo_Con_PWM[PWM];
    }
    int getPinMotorDerecho(){
        return Motor_Derecho_Con_PWM[PWM];
    }
    int getPWM(int velocidad){
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

    int ControlPID(int error, int velocidad){
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