#include <Servo.h>

Servo garra;

const int PIN_GARRA = 4;

int angulo = 45;

void setup() {
  Serial.begin(9600);

  garra.attach(PIN_GARRA);
  garra.write(angulo);

  Serial.println("=== CALIBRACAO DA GARRA ===");
  Serial.println("Posicao inicial: 45 graus");
  Serial.println();
  Serial.println("Comandos:");
  Serial.println("+  aumenta 1 grau");
  Serial.println("-  diminui 1 grau");
  Serial.println("a  aumenta 5 graus");
  Serial.println("d  diminui 5 graus");
  Serial.println("m  volta para 45 graus");
  Serial.println("p  mostra angulo atual");
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();

    if (c == '+') {
      angulo++;
    }

    if (c == '-') {
      angulo--;
    }

    if (c == 'a') {
      angulo += 5;
    }

    if (c == 'd') {
      angulo -= 5;
    }

    if (c == 'm') {
      angulo = 45;
    }

    if (c == 'p') {
      Serial.print("Angulo atual: ");
      Serial.println(angulo);
      return;
    }

    angulo = constrain(angulo, 0, 180);

    garra.write(angulo);

    Serial.print("Garra em: ");
    Serial.print(angulo);
    Serial.println(" graus");
  }
}