// Teste conservador dos dois motores DC com a ponte H L298N.
// Nao usa Servo.h, portanto os PWM D9 e D11 ficam disponiveis.

const byte PIN_IN1 = 8;
const byte PIN_ENA = 3;
const byte PIN_IN2 = 10;
const byte PIN_ENB = 11;
const byte PIN_IN3 = 12;
const byte PIN_IN4 = A5;

const byte PWM_INICIAL = 80;
const byte PWM_MINIMO = 50;
const byte PWM_MAXIMO_TESTE = 130;
const byte PASSO_PWM = 10;

byte velocidade = PWM_INICIAL;

void setup() {
  Serial.begin(9600);

  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);
  pinMode(PIN_ENA, OUTPUT);
  pinMode(PIN_ENB, OUTPUT);

  pararMotores();
  imprimirAjuda();
}

void loop() {
  if (!Serial.available()) return;

  char comando = Serial.read();

  switch (comando) {
    case 'f':
      moverFrente();
      break;

    case 't':
      moverTras();
      break;

    case 'e':
      girarEsquerda();
      break;

    case 'd':
      girarDireita();
      break;

    case '1':
      testarMotorA(true);
      break;

    case '2':
      testarMotorA(false);
      break;

    case '3':
      testarMotorB(true);
      break;

    case '4':
      testarMotorB(false);
      break;

    case '+':
      ajustarVelocidade(PASSO_PWM);
      break;

    case '-':
      ajustarVelocidade(-PASSO_PWM);
      break;

    case 'p':
      pararMotores();
      break;

    case 'v':
      mostrarVelocidade();
      break;

    case 'h':
      imprimirAjuda();
      break;
  }
}

void moverFrente() {
  Serial.println(F("Dois motores para FRENTE"));
  configurarMotorA(true);
  configurarMotorB(true);
  aplicarVelocidade(velocidade, velocidade);
}

void moverTras() {
  Serial.println(F("Dois motores para TRAS"));
  configurarMotorA(false);
  configurarMotorB(false);
  aplicarVelocidade(velocidade, velocidade);
}

void girarEsquerda() {
  Serial.println(F("Girando para ESQUERDA"));
  configurarMotorA(false);
  configurarMotorB(true);
  aplicarVelocidade(velocidade, velocidade);
}

void girarDireita() {
  Serial.println(F("Girando para DIREITA"));
  configurarMotorA(true);
  configurarMotorB(false);
  aplicarVelocidade(velocidade, velocidade);
}

void testarMotorA(bool frente) {
  pararMotores();
  configurarMotorA(frente);
  analogWrite(PIN_ENA, velocidade);
  Serial.println(frente ? F("Motor A: sentido 1") : F("Motor A: sentido 2"));
}

void testarMotorB(bool frente) {
  pararMotores();
  configurarMotorB(frente);
  analogWrite(PIN_ENB, velocidade);
  Serial.println(frente ? F("Motor B: sentido 1") : F("Motor B: sentido 2"));
}

void configurarMotorA(bool frente) {
  digitalWrite(PIN_IN1, frente ? HIGH : LOW);
  digitalWrite(PIN_IN2, frente ? LOW : HIGH);
}

void configurarMotorB(bool frente) {
  digitalWrite(PIN_IN3, frente ? HIGH : LOW);
  digitalWrite(PIN_IN4, frente ? LOW : HIGH);
}

void aplicarVelocidade(byte pwmA, byte pwmB) {
  analogWrite(PIN_ENA, constrain(pwmA, 0, PWM_MAXIMO_TESTE));
  analogWrite(PIN_ENB, constrain(pwmB, 0, PWM_MAXIMO_TESTE));
}

void ajustarVelocidade(int alteracao) {
  velocidade = constrain((int)velocidade + alteracao,
                         PWM_MINIMO, PWM_MAXIMO_TESTE);
  aplicarVelocidade(velocidade, velocidade);
  mostrarVelocidade();
}

void pararMotores() {
  analogWrite(PIN_ENA, 0);
  analogWrite(PIN_ENB, 0);
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);
  Serial.println(F("Motores parados"));
}

void mostrarVelocidade() {
  Serial.print(F("PWM atual: "));
  Serial.print(velocidade);
  Serial.print(F("/255 | limite de teste: "));
  Serial.println(PWM_MAXIMO_TESTE);
}

void imprimirAjuda() {
  Serial.println(F("=== TESTE L298N ==="));
  Serial.println(F("1/2 - motor A nos dois sentidos"));
  Serial.println(F("3/4 - motor B nos dois sentidos"));
  Serial.println(F("f/t - frente/tras"));
  Serial.println(F("e/d - girar esquerda/direita"));
  Serial.println(F("+/- - ajustar PWM"));
  Serial.println(F("p - parada imediata"));
  Serial.println(F("v - mostrar PWM"));
}
