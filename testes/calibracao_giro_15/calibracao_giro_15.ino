// Calibracao isolada dos passos finos de 15 graus do radar.
// Botao entre D9 e GND:
// primeiro toque = passo à esquerda; segundo toque = passo à direita.

const byte PIN_IN1 = 8;
const byte PIN_ENA = 3;
const byte PIN_IN2 = 10;
const byte PIN_ENB = 11;
const byte PIN_IN3 = 12;
const byte PIN_IN4 = A5;
const byte PIN_BOTAO = 9;

const byte PWM_MOTOR_A = 70;
const byte PWM_MOTOR_B = 80;
const unsigned int TEMPO_ESQUERDA_MS = 197;
const unsigned int TEMPO_DIREITA_MS = 180;
const unsigned int ATRASO_INICIO_MS = 1000;
const unsigned int TEMPO_ESTABILIZACAO_MS = 700;

bool proximoPassoEsquerda = true;
bool estadoAnteriorBotao = HIGH;
unsigned long ultimaAcaoMs = 0;

void setup() {
  Serial.begin(9600);

  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);
  pinMode(PIN_ENA, OUTPUT);
  pinMode(PIN_ENB, OUTPUT);
  pinMode(PIN_BOTAO, INPUT_PULLUP);

  pararMotores();
  Serial.println(F("=== CALIBRACAO DOS PASSOS DE 15 GRAUS ==="));
  Serial.println(F("1o toque: esquerda | 2o toque: direita"));
}

void loop() {
  bool estado = digitalRead(PIN_BOTAO);

  if (estadoAnteriorBotao == HIGH && estado == LOW &&
      millis() - ultimaAcaoMs >= 250) {
    ultimaAcaoMs = millis();
    delay(ATRASO_INICIO_MS);
    executarPasso(proximoPassoEsquerda);
    proximoPassoEsquerda = !proximoPassoEsquerda;
  }

  estadoAnteriorBotao = estado;
}

void executarPasso(bool esquerda) {
  Serial.println(esquerda ? F("Passo de 15 graus à ESQUERDA")
                           : F("Passo de 15 graus à DIREITA"));

  digitalWrite(PIN_IN1, esquerda ? LOW : HIGH);
  digitalWrite(PIN_IN2, esquerda ? HIGH : LOW);
  digitalWrite(PIN_IN3, esquerda ? LOW : HIGH);
  digitalWrite(PIN_IN4, esquerda ? HIGH : LOW);

  analogWrite(PIN_ENA, PWM_MOTOR_A);
  analogWrite(PIN_ENB, PWM_MOTOR_B);
  delay(esquerda ? TEMPO_ESQUERDA_MS : TEMPO_DIREITA_MS);
  pararMotores();
  delay(TEMPO_ESTABILIZACAO_MS);
}

void pararMotores() {
  analogWrite(PIN_ENA, 0);
  analogWrite(PIN_ENB, 0);
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);
}
