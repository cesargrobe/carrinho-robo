// Teste do sensor ultrassonico HC-SR04.

const byte PIN_TRIG = 13;
const byte PIN_ECHO = 2;

const byte TOTAL_AMOSTRAS = 5;
const unsigned long TIMEOUT_ECO_US = 30000UL;
const unsigned long INTERVALO_LEITURA_MS = 300;

float distanciaDeteccaoCm = 20.0;
bool modoContinuo = false;
unsigned long ultimaLeituraMs = 0;

void setup() {
  Serial.begin(9600);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  imprimirAjuda();
}

void loop() {
  if (Serial.available()) {
    char comando = Serial.read();

    switch (comando) {
      case 'm':
        medirEMostrar();
        break;

      case 'c':
        modoContinuo = !modoContinuo;
        Serial.println(modoContinuo ? F("Modo continuo ATIVADO")
                                    : F("Modo continuo DESATIVADO"));
        break;

      case '+':
        distanciaDeteccaoCm += 1.0;
        mostrarLimite();
        break;

      case '-':
        distanciaDeteccaoCm = max(2.0, distanciaDeteccaoCm - 1.0);
        mostrarLimite();
        break;

      case 'l':
        mostrarLimite();
        break;

      case 'h':
        imprimirAjuda();
        break;
    }
  }

  if (modoContinuo && millis() - ultimaLeituraMs >= INTERVALO_LEITURA_MS) {
    ultimaLeituraMs = millis();
    medirEMostrar();
  }
}

void medirEMostrar() {
  float distancia = medirDistanciaFiltrada();

  if (distancia < 0) {
    Serial.println(F("Sem eco valido"));
    return;
  }

  Serial.print(F("Distancia: "));
  Serial.print(distancia, 1);
  Serial.print(F(" cm | "));

  if (distancia <= distanciaDeteccaoCm) {
    Serial.println(F("OBJETO DETECTADO"));
  } else {
    Serial.println(F("livre"));
  }
}

float medirDistanciaFiltrada() {
  float amostras[TOTAL_AMOSTRAS];
  byte validas = 0;

  for (byte i = 0; i < TOTAL_AMOSTRAS; i++) {
    float distancia = medirDistancia();

    if (distancia >= 2.0 && distancia <= 400.0) {
      amostras[validas] = distancia;
      validas++;
    }

    delay(20);
  }

  if (validas == 0) return -1;

  ordenarAmostras(amostras, validas);
  return amostras[validas / 2];
}

float medirDistancia() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  unsigned long duracao = pulseIn(PIN_ECHO, HIGH, TIMEOUT_ECO_US);
  if (duracao == 0) return -1;

  return duracao * 0.0343 / 2.0;
}

void ordenarAmostras(float valores[], byte quantidade) {
  for (byte i = 0; i < quantidade; i++) {
    for (byte j = i + 1; j < quantidade; j++) {
      if (valores[j] < valores[i]) {
        float temporario = valores[i];
        valores[i] = valores[j];
        valores[j] = temporario;
      }
    }
  }
}

void mostrarLimite() {
  Serial.print(F("Limite de deteccao: "));
  Serial.print(distanciaDeteccaoCm, 1);
  Serial.println(F(" cm"));
}

void imprimirAjuda() {
  Serial.println(F("=== TESTE HC-SR04 ==="));
  Serial.println(F("m - medir uma vez"));
  Serial.println(F("c - ativar/desativar leitura continua"));
  Serial.println(F("+/- - ajustar limite em 1 cm"));
  Serial.println(F("l - mostrar limite"));
  Serial.println(F("h - mostrar ajuda"));
  mostrarLimite();
}
