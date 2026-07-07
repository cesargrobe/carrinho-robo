#include <Servo.h>

// Teste isolado da garra em alta velocidade.
// Nenhum servo do braco e movimentado por este programa.

const byte PIN_GARRA = 4;

const byte GARRA_FECHADA = 6;
const byte GARRA_ABERTA  = 30;

Servo garra;

void setup() {
  Serial.begin(9600);

  // Prepara a posicao antes de conectar o sinal ao servo.
  garra.write(GARRA_ABERTA);
  garra.attach(PIN_GARRA);
  delay(700);

  Serial.println(F("=== FECHAMENTO RAPIDO DA GARRA ==="));
  Serial.println(F("f - fechar imediatamente"));
  Serial.println(F("a - abrir imediatamente"));
  Serial.println(F("Mantenha as maos afastadas das laminas."));
}

void loop() {
  if (!Serial.available()) return;

  char comando = Serial.read();

  switch (comando) {
    case 'f':
      Serial.println(F("Fechando garra..."));

      // Um unico comando, sem passos intermediarios ou delays.
      // A velocidade efetiva fica limitada pelo proprio servomotor.
      garra.write(GARRA_FECHADA);
      break;

    case 'a':
      Serial.println(F("Abrindo garra..."));
      garra.write(GARRA_ABERTA);
      break;
  }
}
