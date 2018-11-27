/*Sistema de detección de golpes para control de acceso a puertas.
  Autor: Javier Vargas. El Hormiguero.
  Basado en proyecto de Grathio https://www.instructables.com/id/Secret-Knock-Detecting-Door-Lock/
  https://creativecommons.org/licenses/by/4.0/
*/

//PINES
#define PinPiezo A0
#define PinLedVerde 11
#define PinLedRojo 10
#define PinSolenoide 9
#define PinBoton 12

//CONFIGURACION
#define UmbralPiezo 70 //Umbral de deteccion del piezoeléctrico
#define TiempoPulsoMin 100 //(ms) Tiempo entre pulsos consecutivos
#define ErrorMax 200 //Error máximo entre pulsos
#define TimeOut 1500 //(ms) Tiempo máximo de espera para grabar/detectar pulsos. 2550 max
#define PulsosMax 30
#define TiempoInicioGrabacion 10000 //Tiempo de espera maximo para empezar a grabar tras entrar en el modo grabacion
#define TiempoParpadeoLed 50 //(ms) Tiempo de parpadeo del led
#define TiempoPuerta 5000 //(ms) Tiempo que el solenoide esta activado para abrir la puerta
#define CodigoMemoria 120 //Codigo guardado en la primera posicion de la EEPROM para comprobar que hay datos guardados

#include <EEPROM.h>

byte codigo[PulsosMax];

void setup() {
  
  //PinMode
  pinMode(PinLedVerde, OUTPUT);
  pinMode(PinLedRojo, OUTPUT);
  pinMode(PinSolenoide, OUTPUT);
  pinMode(PinBoton, INPUT_PULLUP);

  //Cargar EEPROM
  LedON(PinLedVerde);
  LedON(PinLedRojo);
  CargarMemoria();
  LedOFF(PinLedVerde);
  LedOFF(PinLedRojo);
}

/////////////////////////////////////////
///////////////////LOOP//////////////////
/////////////////////////////////////////

void loop() {

  //MODO GRABACIÓN DEL CODIGO
  if (LecturaBoton()) {
    //Modo grabacion
    LedON(PinLedVerde);
    unsigned long tmax = millis() + TiempoInicioGrabacion;
    //Espera a golpear para iniciar a grabar
    while (Golpe() == 0) {
      //Si el tiempo es mayor al de espera de inicio, salimos del modo grabacion y borramos la memoria
      if (millis() > tmax) {
        LedON(PinLedRojo);
        BorrarMemoria();
        BorrarCodigo();
        LedOFF(PinLedVerde);
        LedOFF(PinLedRojo);
        return;
      }
    }
    //Codigo guardado correctamente
    if (GuardarCodigo() == 1) {
      GuardarMemoria();
      Parpadeo(PinLedVerde, 5);
    }
    //Error al guardar el codigo
    else {
      Parpadeo(PinLedRojo, 5);
    }
    LedOFF(PinLedVerde);
  }

  //MODO LECTURA DE GOLPES
  if (Golpe()) {
    //Codigo correcto
    if (LeerCodigo() == 1) {
      LedON(PinLedVerde);
      AbrirPuerta();
      LedOFF(PinLedVerde);
    }
    //Codigo incorrecto
    else {
      Parpadeo(PinLedRojo, 10);
    }
  }

}

/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////

boolean Golpe() {
  static unsigned long tmin = 0;
  static boolean estado = 0;
  //Tiempo antirrebotes superado
  if (millis() > tmin) {
    //Lectura analogica por encima del umbral
    if (analogRead(PinPiezo) > UmbralPiezo) {
      tmin = millis() + TiempoPulsoMin; //Tiempo antirrebotes
      LedON(PinLedRojo);
      estado = 1;
      return 1;
    }
    //Umbral no superado
    else {
      if (estado) {
        estado = 0;
        LedOFF(PinLedRojo);
      }
      return 0;
    }
  }
  else return 0;
}

boolean GuardarCodigo() {
  unsigned long t = millis();
  int addr = 0;
  //Ponemos el código a 0
  BorrarCodigo();
  //Mientras sigamos dando golpes
  while (millis() < t + TimeOut) {
    //Golpe detectado
    if (Golpe()) {
      //Guardamos el tiempo entre el pulso anterior y el actual
      codigo[addr] = (millis() - t) / 10;
      t = millis();
      addr++;
    }
  }
  //No se a guardado ningun golpe
  if (addr == 0) return 0;
  //Se ha guardado almenos un golpe
  else return 1;
}

boolean LeerCodigo() {
  unsigned long t = millis();
  int addr = 0;
  boolean ok = 1;
  //Mientras sigamos dando golpes
  while (millis() < t + TimeOut) {
    //Golpe detectado
    if (Golpe()) {
      unsigned long tiempo = millis() - t; //Tiempo entre el pulso anterior y el actual
      t = millis();
      int Tmin = tiempo - ErrorMax; //Margen de tiempo inferior
      int Tmax = tiempo + ErrorMax; //Margen de tiempo superior
      Tmin = constrain(Tmin, 0, TimeOut);
      //Tiempo de pulso correcto dentro de los margenes
      if (codigo[addr] * 10 > Tmin && codigo[addr] * 10 < Tmax) addr++;
      //Error
      else ok = 0;
    }
  }
  //El golpe ha sido el ultimo almacenado
  if (codigo[addr] == 0 && addr > 0) return ok;
  //Hay mas golpes almacenados
  else return 0;
}

boolean LecturaBoton() {
  return !digitalRead(PinBoton);
}

void Parpadeo(int led, int n) {
  for (int i = 0; i < n; i++) {
    digitalWrite(led, HIGH);
    delay(TiempoParpadeoLed);
    digitalWrite(led, LOW);
    delay(TiempoParpadeoLed);
  }
}

void LedON(int led) {
  digitalWrite(led, HIGH);
}

void LedOFF(int led) {
  digitalWrite(led, LOW);
}

void AbrirPuerta() {
  digitalWrite(PinSolenoide, HIGH);
  delay(TiempoPuerta);
  digitalWrite(PinSolenoide, LOW);
}

void GuardarMemoria() {
  //Borrado de los datos anteriores
  BorrarMemoria();
  //Codigo de sincronizacion
  EEPROM.update(0, CodigoMemoria);
  int addr = 0;
  while (codigo[addr] != 0) {
    //Guardado del codigo en la memoria eeprom
    EEPROM.write(addr + 1, codigo[addr]);
    addr++;
  }
}

void CargarMemoria() {
  //Hay datos guardados?
  if (EEPROM.read(0) == CodigoMemoria) {
    int addr = 0;
    while (1) {
      //Cargamos el valor de la eeprom
      codigo[addr] = EEPROM.read(addr + 1);
      //Codigo cargado completamente
      if (codigo[addr] == 0) break;
      addr++;
    }
  }
  else BorrarMemoria();
  delay(1000);
}

void BorrarMemoria() {
  for (int i = 0; i < 1023; i++) EEPROM.update(i, 0);
}

void BorrarCodigo() {
  for (int i = 0; i < PulsosMax; i++) codigo[i] = 0;
}



