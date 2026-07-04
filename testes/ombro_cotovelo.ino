
#include <Servo.h>

Servo cotovelo;
Servo ombro;

const int PIN_COTOVELO = 5;
const int PIN_OMBRO    = 6;

int angCotovelo = 130;
int angOmbro    = 40;

void setup() {
  Serial.begin(9600);

  cotovelo.attach(PIN_COTOVELO);
  ombro.attach(PIN_OMBRO);

  cotovelo.write(angCotovelo);
  ombro.write(angOmbro);

  Serial.println("=== CALIBRACAO OMBRO + COTOVELO ===");
  Serial.println("Comandos:");
  Serial.println("1 / 2 -> Ombro + / -");
  Serial.println("3 / 4 -> Cotovelo + / -");
  Serial.println("q / w -> Ombro +5 / -5");
  Serial.println("e / r -> Cotovelo +5 / -5");
  Serial.println("m -> voltar para 90/90");
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
      angOmbro = 90;
      angCotovelo = 90;
    }

    angOmbro = constrain(angOmbro, 0, 180);
    angCotovelo = constrain(angCotovelo, 0, 180);

    ombro.write(angOmbro);
    cotovelo.write(angCotovelo);

    Serial.print("Ombro: ");
    Serial.print(angOmbro);
    Serial.print(" | Cotovelo: ");
    Serial.println(angCotovelo);
  }
}