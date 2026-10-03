
#include <Arduino.h>
#include "Sensores.h"
#include "Motores.h"


class Movimientos{
    private:
    Sensores sensores;
    Motores motores;
    int ultimoLadoEnemigo = -1;
    enum EstadoEscape { SIN_ESCAPE, RETROCEDIENDO, GIRANDO_ESCAPE };

    // Busqueda en dohyo de 70 cm: una vuelta completa por giro y tramos de avance
    // cortos (menos de medio radio) para no llegar al borde en cada barrido.
    static constexpr float AVANCE_BUSQUEDA_CM = 15.0f;
    static constexpr unsigned long GIRO_BUSQUEDA_MS = Motores::msParaGiro(360.0f);
    static constexpr unsigned long AVANCE_BUSQUEDA_MS = Motores::msParaDistancia(AVANCE_BUSQUEDA_CM, Motores::PWM_MEDIO);
    static const unsigned long INTERVALO_ATAQUE_MS = 450;
    static const unsigned long PERSISTENCIA_RASTRO_MS = 600;
    static constexpr float RETROCESO_BORDE_CM = 6.0f;
    static constexpr float GIRO_ESCAPE_GRADOS = 135.0f;
    static constexpr unsigned long RETROCESO_BORDE_MS = Motores::msParaDistancia(RETROCESO_BORDE_CM);
    static constexpr unsigned long GIRO_ESCAPE_MS = Motores::msParaGiro(GIRO_ESCAPE_GRADOS);

    EstadoEscape estadoEscape = SIN_ESCAPE;
    bool giroEscapeDerecha = true;
    byte patronBusqueda = 0;
    byte patronAtaque = 0;
    unsigned long ultimoCambioBusqueda = 0;
    unsigned long ultimoCambioAtaque = 0;
    unsigned long ultimoAvistamiento = 0;
    unsigned long finEscape = 0;

    void avanzar(int velocidad) const {
        motores.setDireccion(Motores::IZQUIERDO, Motores::AVANZAR);
        motores.setDireccion(Motores::DERECHO, Motores::AVANZAR);
        motores.ControlPID(0, velocidad);
    }

    void avanzarCorregido(int velocidad, int error) const {
        motores.setDireccion(Motores::IZQUIERDO, Motores::AVANZAR);
        motores.setDireccion(Motores::DERECHO, Motores::AVANZAR);
        motores.ControlPID(error, velocidad);
    }

    void retroceder(int velocidad) const {
        motores.setDireccion(Motores::IZQUIERDO, Motores::RETROCEDER);
        motores.setDireccion(Motores::DERECHO, Motores::RETROCEDER);
        motores.ControlPID(0, velocidad);
    }

    void girarIzquierda(int velocidad) const {
        motores.setDireccion(Motores::IZQUIERDO, Motores::GIRAR_IZQUIERDA);
        motores.setDireccion(Motores::DERECHO, Motores::GIRAR_IZQUIERDA);
        motores.ControlPID(0, velocidad);
    }

    void girarDerecha(int velocidad) const {
        motores.setDireccion(Motores::IZQUIERDO, Motores::GIRAR_DERECHA);
        motores.setDireccion(Motores::DERECHO, Motores::GIRAR_DERECHA);
        motores.ControlPID(0, velocidad);
    }

    void atacar() const {
        avanzar(Motores::VELOCIDAD_ALTA);
    }

    void detener() const {
        motores.setDireccion(Motores::IZQUIERDO, Motores::PARAR);
        motores.setDireccion(Motores::DERECHO, Motores::PARAR);
    }

    void iniciarEscape(bool bordeIzquierdo, unsigned long ahora){
        estadoEscape = RETROCEDIENDO;
        giroEscapeDerecha = bordeIzquierdo;
        finEscape = ahora + RETROCESO_BORDE_MS;
    }

    bool ejecutarEscape(unsigned long ahora){
        if (estadoEscape == SIN_ESCAPE) return false;

        if (estadoEscape == RETROCEDIENDO && ahora >= finEscape) {
            estadoEscape = GIRANDO_ESCAPE;
            finEscape = ahora + GIRO_ESCAPE_MS;
        } else if (estadoEscape == GIRANDO_ESCAPE && ahora >= finEscape) {
            estadoEscape = SIN_ESCAPE;
            return false;
        }

        if (estadoEscape == RETROCEDIENDO) {
            retroceder(Motores::VELOCIDAD_ALTA);
        } else if (giroEscapeDerecha) {
            girarDerecha(Motores::VELOCIDAD_ALTA);
        } else {
            girarIzquierda(Motores::VELOCIDAD_ALTA);
        }
        return true;
    }

    void buscar(unsigned long ahora){
        unsigned long duracion = patronBusqueda < 2 ? GIRO_BUSQUEDA_MS : AVANCE_BUSQUEDA_MS;
        if (ahora - ultimoCambioBusqueda >= duracion) {
            ultimoCambioBusqueda = ahora;
            patronBusqueda = (patronBusqueda + 1) % 4;
        }

        switch (patronBusqueda) {
            case 0: girarDerecha(Motores::VELOCIDAD_ALTA); break;
            case 1: girarIzquierda(Motores::VELOCIDAD_ALTA); break;
            case 2: avanzarCorregido(Motores::VELOCIDAD_MEDIA, 35); break;
            default: avanzarCorregido(Motores::VELOCIDAD_MEDIA, -35); break;
        }
    }

    void ejecutarAtaque(unsigned long ahora){
        if (ahora - ultimoCambioAtaque >= INTERVALO_ATAQUE_MS) {
            ultimoCambioAtaque = ahora;
            patronAtaque = (patronAtaque + 1) % 3;
        }

        switch (patronAtaque) {
            case 0: atacar(); break;
            case 1: avanzarCorregido(Motores::VELOCIDAD_ALTA, 20); break;
            default: avanzarCorregido(Motores::VELOCIDAD_ALTA, -20); break;
        }
    }

    public:

    void iniciar() const {
        sensores.iniciar();
        motores.iniciar();
    }

    // Prioridad: evitar borde -> atacar enemigo -> buscar
    void ejecutar(){
        unsigned long ahora = millis();
        bool bordeIzquierdo = sensores.esBorde(PISO_IZQ);
        bool bordeDerecho = sensores.esBorde(PISO_DER);

        if (bordeIzquierdo || bordeDerecho) {
            iniciarEscape(bordeIzquierdo, ahora);
            ejecutarEscape(ahora);
            return;
        }

        if (ejecutarEscape(ahora)) {
            return;
        }

        if (sensores.esEnemigoFRONTAL(FRONTAL_CENTRO)) {
            ultimoLadoEnemigo = -1;
            ultimoAvistamiento = ahora;
            ejecutarAtaque(ahora);
            return;
        }
        if (sensores.esEnemigoFRONTAL(FRONTAL_IZQ)) {
            ultimoLadoEnemigo = 0;
            ultimoAvistamiento = ahora;
            girarIzquierda(Motores::VELOCIDAD_MEDIA);
            return;
        }
        if (sensores.esEnemigoFRONTAL(FRONTAL_DER)) {
            ultimoLadoEnemigo = 1;
            ultimoAvistamiento = ahora;
            girarDerecha(Motores::VELOCIDAD_MEDIA);
            return;
        }

        if (sensores.esEnemigoLATERAL(IZQUIERDO)) {
            ultimoLadoEnemigo = 0;
            ultimoAvistamiento = ahora;
            girarIzquierda(Motores::VELOCIDAD_MEDIA);
            return;
        }
        if (sensores.esEnemigoLATERAL(DERECHO)) {
            ultimoLadoEnemigo = 1;
            ultimoAvistamiento = ahora;
            girarDerecha(Motores::VELOCIDAD_MEDIA);
            return;
        }

        if (ahora - ultimoAvistamiento >= PERSISTENCIA_RASTRO_MS) {
            ultimoLadoEnemigo = -1;
        }

        if (ultimoLadoEnemigo == 0) {
            girarIzquierda(Motores::VELOCIDAD_MEDIA);
            return;
        }
        if (ultimoLadoEnemigo == 1) {
            girarDerecha(Motores::VELOCIDAD_MEDIA);
            return;
        }

        buscar(ahora);
    }
};

namespace {
Movimientos& obtenerMovimientos(){
    static Movimientos movimientos;
    return movimientos;
}
}

void iniciarEstrategiaCombate(){
    obtenerMovimientos().iniciar();
}

void ejecutarEstrategiaCombate(){
    obtenerMovimientos().ejecutar();
}
