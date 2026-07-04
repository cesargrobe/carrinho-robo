#include <Servo.h>

Servo garra;

const int PIN_GARRA = 4;

const int GARRA_FECHADA = 5;
const int GARRA_ABERTA  = 37;

int angulo = GARRA_ABERTA;

void setup() {
  Serial.begin(9600);

  garra.write(angulo);
  garra.attach(PIN_GARRA);
  delay(500);

  Serial.println("=== CALIBRACAO DA GARRA ===");
  Serial.println("Posicao inicial: 37 graus (aberta)");
  Serial.println();
  Serial.println("Comandos:");
  Serial.println("+  aumenta 1 grau");
  Serial.println("-  diminui 1 grau");
  Serial.println("a  aumenta 5 graus");
  Serial.println("d  diminui 5 graus");
  Serial.println("m  volta para 37 graus (aberta)");
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
      angulo = GARRA_ABERTA;
    }

    if (c == 'p') {
      Serial.print("Angulo atual: ");
      Serial.println(angulo);
      return;
    }

    angulo = constrain(angulo, GARRA_FECHADA, GARRA_ABERTA);

    garra.write(angulo);

    Serial.print("Garra em: ");
    Serial.print(angulo);
    Serial.println(" graus");
  }
}
