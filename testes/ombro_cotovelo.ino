
#include <Servo.h>

Servo cotovelo;
Servo ombro;

const int PIN_COTOVELO = 5;
const int PIN_OMBRO    = 6;

int angCotovelo = 130;
int angOmbro    = 40;

const int OMBRO_MIN = 40;
const int OMBRO_MAX = 165;
const int COTOVELO_MIN = 40;
const int COTOVELO_MAX = 140;

void setup() {
  Serial.begin(9600);

  cotovelo.write(angCotovelo);
  cotovelo.attach(PIN_COTOVELO);
  delay(400);

  ombro.write(angOmbro);
  ombro.attach(PIN_OMBRO);
  delay(500);

  Serial.println("=== CALIBRACAO OMBRO + COTOVELO ===");
  Serial.println("Comandos:");
  Serial.println("1 / 2 -> Ombro + / -");
  Serial.println("3 / 4 -> Cotovelo + / -");
  Serial.println("q / w -> Ombro +5 / -5");
  Serial.println("e / r -> Cotovelo +5 / -5");
  Serial.println("m -> voltar para posicao segura 40/130");
  Serial.println("p -> mostrar posicoes");
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();

    if (c == '1') angOmbro++;
    if (c == '2') angOmbro--;

    if (c == '3') angCotovelo++;
    if (c == '4') angCotovelo--;

    if (c == 'q') angOmbro += 5;
    if (c == 'w') angOmbro -= 5;

    if (c == 'e') angCotovelo += 5;
    if (c == 'r') angCotovelo -= 5;

    if (c == 'm') {
      voltarParaPosicaoSegura();
      return;
    }

    angOmbro = constrain(angOmbro, OMBRO_MIN, OMBRO_MAX);
    angCotovelo = constrain(angCotovelo, COTOVELO_MIN, COTOVELO_MAX);

    ombro.write(angOmbro);
    cotovelo.write(angCotovelo);

    Serial.print("Ombro: ");
    Serial.print(angOmbro);
    Serial.print(" | Cotovelo: ");
    Serial.println(angCotovelo);
  }
}

void voltarParaPosicaoSegura() {
  // Recolhe primeiro o cotovelo e depois o ombro, sem saltos bruscos.
  while (angCotovelo != 130) {
    angCotovelo += angCotovelo < 130 ? 1 : -1;
    cotovelo.write(angCotovelo);
    delay(35);
  }

  while (angOmbro != 40) {
    angOmbro += angOmbro < 40 ? 1 : -1;
    ombro.write(angOmbro);
    delay(35);
  }

  Serial.println("Posicao segura: Ombro 40 | Cotovelo 130");
}
