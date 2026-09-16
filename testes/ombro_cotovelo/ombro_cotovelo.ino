#include <Servo.h>

Servo cotovelo;
Servo ombro;

const int PIN_COTOVELO = 5;
const int PIN_OMBRO    = 6;

int angCotovelo = 130;
int angOmbro    = 40;

// Agora os limites NÃO são const, para permitir alteração pelo Serial
int OMBRO_MIN = 40;
int OMBRO_MAX = 165;

int COTOVELO_MIN = 40;
int COTOVELO_MAX = 140;

// Posição segura
const int OMBRO_SEGURO = 40;
const int COTOVELO_SEGURO = 130;

void setup() {
  Serial.begin(9600);

  cotovelo.attach(PIN_COTOVELO);
  ombro.attach(PIN_OMBRO);

  delay(300);

  cotovelo.write(angCotovelo);
  ombro.write(angOmbro);

  delay(500);

  Serial.println("=== CALIBRACAO OMBRO + COTOVELO ===");
  Serial.println();
  Serial.println("Comandos de movimento:");
  Serial.println("1 / 2 -> Ombro +1 / -1");
  Serial.println("3 / 4 -> Cotovelo +1 / -1");
  Serial.println("q / w -> Ombro +5 / -5");
  Serial.println("e / r -> Cotovelo +5 / -5");
  Serial.println();
  Serial.println("Comandos para alterar limites:");
  Serial.println("a / s -> OMBRO_MIN +1 / -1");
  Serial.println("d / f -> OMBRO_MAX +1 / -1");
  Serial.println("z / x -> COTOVELO_MIN +1 / -1");
  Serial.println("c / v -> COTOVELO_MAX +1 / -1");
  Serial.println();
  Serial.println("Outros comandos:");
  Serial.println("m -> voltar para posicao segura 40/130");
  Serial.println("p -> mostrar posicoes");
  Serial.println("L -> mostrar limites atuais");
  Serial.println();

  mostrarPosicoes();
  mostrarLimites();
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();

    // Ignora quebra de linha do Monitor Serial
    if (c == '\n' || c == '\r') {
      return;
    }

    // ============================
    // MOVIMENTO DO OMBRO
    // ============================

    if (c == '1') angOmbro++;
    if (c == '2') angOmbro--;

    if (c == 'q') angOmbro += 5;
    if (c == 'w') angOmbro -= 5;

    // ============================
    // MOVIMENTO DO COTOVELO
    // ============================

    if (c == '3') angCotovelo++;
    if (c == '4') angCotovelo--;

    if (c == 'e') angCotovelo += 5;
    if (c == 'r') angCotovelo -= 5;

    // ============================
    // ALTERACAO DOS LIMITES DO OMBRO
    // ============================

    if (c == 'a') {
      OMBRO_MIN++;
      ajustarLimites();
      Serial.println("OMBRO_MIN aumentado.");
      mostrarLimites();
      return;
    }

    if (c == 's') {
      OMBRO_MIN--;
      ajustarLimites();
      Serial.println("OMBRO_MIN diminuido.");
      mostrarLimites();
      return;
    }

    if (c == 'd') {
      OMBRO_MAX++;
      ajustarLimites();
      Serial.println("OMBRO_MAX aumentado.");
      mostrarLimites();
      return;
    }

    if (c == 'f') {
      OMBRO_MAX--;
      ajustarLimites();
      Serial.println("OMBRO_MAX diminuido.");
      mostrarLimites();
      return;
    }

    // ============================
    // ALTERACAO DOS LIMITES DO COTOVELO
    // ============================

    if (c == 'z') {
      COTOVELO_MIN++;
      ajustarLimites();
      Serial.println("COTOVELO_MIN aumentado.");
      mostrarLimites();
      return;
    }

    if (c == 'x') {
      COTOVELO_MIN--;
      ajustarLimites();
      Serial.println("COTOVELO_MIN diminuido.");
      mostrarLimites();
      return;
    }

    if (c == 'c') {
      COTOVELO_MAX++;
      ajustarLimites();
      Serial.println("COTOVELO_MAX aumentado.");
      mostrarLimites();
      return;
    }

    if (c == 'v') {
      COTOVELO_MAX--;
      ajustarLimites();
      Serial.println("COTOVELO_MAX diminuido.");
      mostrarLimites();
      return;
    }

    // ============================
    // OUTROS COMANDOS
    // ============================

    if (c == 'm') {
      voltarParaPosicaoSegura();
      return;
    }

    if (c == 'p') {
      mostrarPosicoes();
      return;
    }

    if (c == 'L') {
      mostrarLimites();
      return;
    }

    // ============================
    // APLICA LIMITES E MOVE SERVOS
    // ============================

    angOmbro = constrain(angOmbro, OMBRO_MIN, OMBRO_MAX);
    angCotovelo = constrain(angCotovelo, COTOVELO_MIN, COTOVELO_MAX);

    ombro.write(angOmbro);
    cotovelo.write(angCotovelo);

    mostrarPosicoes();
  }
}

void ajustarLimites() {
  // Impede valores fora da faixa física do servo
  OMBRO_MIN = constrain(OMBRO_MIN, 0, 180);
  OMBRO_MAX = constrain(OMBRO_MAX, 0, 180);

  COTOVELO_MIN = constrain(COTOVELO_MIN, 0, 180);
  COTOVELO_MAX = constrain(COTOVELO_MAX, 0, 180);

  // Garante que o mínimo nunca passe do máximo
  if (OMBRO_MIN > OMBRO_MAX) {
    OMBRO_MIN = OMBRO_MAX;
  }

  if (OMBRO_MAX < OMBRO_MIN) {
    OMBRO_MAX = OMBRO_MIN;
  }

  if (COTOVELO_MIN > COTOVELO_MAX) {
    COTOVELO_MIN = COTOVELO_MAX;
  }

  if (COTOVELO_MAX < COTOVELO_MIN) {
    COTOVELO_MAX = COTOVELO_MIN;
  }

  // Reaplica limites nos ângulos atuais
  angOmbro = constrain(angOmbro, OMBRO_MIN, OMBRO_MAX);
  angCotovelo = constrain(angCotovelo, COTOVELO_MIN, COTOVELO_MAX);

  ombro.write(angOmbro);
  cotovelo.write(angCotovelo);
}

void voltarParaPosicaoSegura() {
  Serial.println("Voltando para posicao segura...");

  // Primeiro recolhe o cotovelo
  while (angCotovelo != COTOVELO_SEGURO) {
    angCotovelo += angCotovelo < COTOVELO_SEGURO ? 1 : -1;

    // Se a posição segura estiver fora dos limites atuais, respeita os limites
    angCotovelo = constrain(angCotovelo, COTOVELO_MIN, COTOVELO_MAX);

    cotovelo.write(angCotovelo);
    delay(35);

    if (angCotovelo == COTOVELO_MIN || angCotovelo == COTOVELO_MAX) {
      break;
    }
  }

  // Depois ajusta o ombro
  while (angOmbro != OMBRO_SEGURO) {
    angOmbro += angOmbro < OMBRO_SEGURO ? 1 : -1;

    // Se a posição segura estiver fora dos limites atuais, respeita os limites
    angOmbro = constrain(angOmbro, OMBRO_MIN, OMBRO_MAX);

    ombro.write(angOmbro);
    delay(35);

    if (angOmbro == OMBRO_MIN || angOmbro == OMBRO_MAX) {
      break;
    }
  }

  Serial.println("Retorno finalizado.");
  mostrarPosicoes();
}

void mostrarPosicoes() {
  Serial.print("Ombro: ");
  Serial.print(angOmbro);

  Serial.print(" | Cotovelo: ");
  Serial.println(angCotovelo);
}

void mostrarLimites() {
  Serial.println("=== LIMITES ATUAIS ===");

  Serial.print("Ombro MIN: ");
  Serial.print(OMBRO_MIN);
  Serial.print(" | Ombro MAX: ");
  Serial.println(OMBRO_MAX);

  Serial.print("Cotovelo MIN: ");
  Serial.print(COTOVELO_MIN);
  Serial.print(" | Cotovelo MAX: ");
  Serial.println(COTOVELO_MAX);

  Serial.println("======================");
}