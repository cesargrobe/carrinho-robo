#include <Servo.h>

// ===================== PINOS =====================
const int PIN_GARRA    = 4;
const int PIN_COTOVELO = 5;
const int PIN_OMBRO    = 6;
const int PIN_BASE     = 7;

// ===================== SERVOS =====================
Servo garra;
Servo cotovelo;
Servo ombro;
Servo base;

// ===================== CALIBRAÇÕES =====================
const int GARRA_ABERTA = 37;

const int OMBRO_SEGURO = 40;
const int COTOVELO_SEGURO = 130;

const int BASE_DIREITA  = 60;
const int BASE_CENTRO   = 90;
const int BASE_ESQUERDA = 120;

int posBase = BASE_CENTRO;

void setup() {
  Serial.begin(9600);

  // Define os destinos antes de energizar cada servo e conecta-os em etapas.
  garra.write(GARRA_ABERTA);
  garra.attach(PIN_GARRA);
  delay(300);
  cotovelo.write(COTOVELO_SEGURO);
  cotovelo.attach(PIN_COTOVELO);
  delay(300);
  ombro.write(OMBRO_SEGURO);
  ombro.attach(PIN_OMBRO);
  delay(300);
  base.write(BASE_CENTRO);
  base.attach(PIN_BASE);
  delay(500);

  posBase = BASE_CENTRO;

  Serial.println("=== TESTE DE MOVIMENTACAO DA BASE ===");
  Serial.println("Comandos:");
  Serial.println("1 - Base para direita  (60 graus)");
  Serial.println("2 - Base para centro   (90 graus)");
  Serial.println("3 - Base para esquerda (120 graus)");
  Serial.println("4 - Varredura direita-centro-esquerda-centro");
  Serial.println("+ - Aumenta 1 grau");
  Serial.println("- - Diminui 1 grau");
  Serial.println("p - Mostrar posicao atual");
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();

    switch (c) {

      case '1':
        Serial.println("Base indo para DIREITA");
        moverBaseSuave(BASE_DIREITA, 25);
        break;

      case '2':
        Serial.println("Base indo para CENTRO");
        moverBaseSuave(BASE_CENTRO, 25);
        break;

      case '3':
        Serial.println("Base indo para ESQUERDA");
        moverBaseSuave(BASE_ESQUERDA, 25);
        break;

      case '4':
        varreduraBase();
        break;

      case '+':
        moverBaseSuave(posBase + 1, 25);
        break;

      case '-':
        moverBaseSuave(posBase - 1, 25);
        break;

      case 'p':
        mostrarBase();
        break;
    }
  }
}

void posicaoSegura() {
  garra.write(GARRA_ABERTA);
  ombro.write(OMBRO_SEGURO);
  cotovelo.write(COTOVELO_SEGURO);
  base.write(BASE_CENTRO);

  posBase = BASE_CENTRO;

  Serial.println("Braco em posicao segura.");
}

void moverBaseSuave(int destino, int tempo) {
  destino = constrain(destino, BASE_DIREITA, BASE_ESQUERDA);

  if (destino > posBase) {
    for (int i = posBase; i <= destino; i++) {
      base.write(i);
      posBase = i;
      delay(tempo);
    }
  } else {
    for (int i = posBase; i >= destino; i--) {
      base.write(i);
      posBase = i;
      delay(tempo);
    }
  }

  mostrarBase();
}

void varreduraBase() {
  Serial.println("Iniciando varredura da base...");

  moverBaseSuave(BASE_DIREITA, 15);
  delay(700);

  moverBaseSuave(BASE_CENTRO, 15);
  delay(700);

  moverBaseSuave(BASE_ESQUERDA, 15);
  delay(700);

  moverBaseSuave(BASE_CENTRO, 15);
  delay(700);

  Serial.println("Varredura finalizada.");
}

void mostrarBase() {
  Serial.print("Base atual: ");
  Serial.print(posBase);
  Serial.println(" graus");
}
