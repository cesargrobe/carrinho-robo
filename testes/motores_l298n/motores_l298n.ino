// Calibracao independente dos dois motores DC com a ponte H L298N.
// Permite encontrar um PWM diferente para cada motor.

const byte PIN_IN1 = 8;
const byte PIN_ENA = 3;
const byte PIN_IN2 = 10;
const byte PIN_ENB = 11;
const byte PIN_IN3 = 12;
const byte PIN_IN4 = A5;

const byte PWM_INICIAL_A = 70;
const byte PWM_INICIAL_B = 80;
const byte PWM_MINIMO = 50;
const byte PWM_MAXIMO_TESTE = 130;
const byte PASSO_PWM = 5;

byte pwmMotorA = PWM_INICIAL_A;
byte pwmMotorB = PWM_INICIAL_B;

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
  mostrarPWM();
}

void loop() {
  if (!Serial.available()) return;

  char comando = Serial.read();

  switch (comando) {
    case '1': testarMotorA(true);  break;
    case '2': testarMotorA(false); break;
    case '3': testarMotorB(true);  break;
    case '4': testarMotorB(false); break;

    case 'f': moverFrente();       break;
    case 't': moverTras();         break;
    case 'e': girarEsquerda();     break;
    case 'd': girarDireita();      break;

    case 'A': ajustarMotorA(PASSO_PWM);  break;
    case 'a': ajustarMotorA(-PASSO_PWM); break;
    case 'B': ajustarMotorB(PASSO_PWM);  break;
    case 'b': ajustarMotorB(-PASSO_PWM); break;

    case 'p': pararMotores();      break;
    case 'v': mostrarPWM();        break;
    case 'h': imprimirAjuda();     break;
  }
}

void moverFrente() {
  Serial.println(F("Dois motores para FRENTE"));

  // Sentidos validados no codigo principal do projeto.
  configurarMotorA(true);
  configurarMotorB(true);
  aplicarPWM();
}

void moverTras() {
  Serial.println(F("Dois motores para TRAS"));
  configurarMotorA(false);
  configurarMotorB(false);
  aplicarPWM();
}

void girarEsquerda() {
  Serial.println(F("Giro no proprio eixo: sentido 1"));
  configurarMotorA(false);
  configurarMotorB(true);
  aplicarPWM();
}

void girarDireita() {
  Serial.println(F("Giro no proprio eixo: sentido 2"));
  configurarMotorA(true);
  configurarMotorB(false);
  aplicarPWM();
}

void testarMotorA(bool frente) {
  pararMotores();
  configurarMotorA(frente);
  analogWrite(PIN_ENA, pwmMotorA);
  Serial.println(frente ? F("Somente motor A: frente")
                         : F("Somente motor A: re"));
  mostrarPWM();
}

void testarMotorB(bool frente) {
  pararMotores();
  configurarMotorB(frente);
  analogWrite(PIN_ENB, pwmMotorB);
  Serial.println(frente ? F("Somente motor B: frente")
                         : F("Somente motor B: re"));
  mostrarPWM();
}

void configurarMotorA(bool frente) {
  // Motor A esta montado no sentido oposto ao motor B.
  digitalWrite(PIN_IN1, frente ? LOW : HIGH);
  digitalWrite(PIN_IN2, frente ? HIGH : LOW);
}

void configurarMotorB(bool frente) {
  digitalWrite(PIN_IN3, frente ? HIGH : LOW);
  digitalWrite(PIN_IN4, frente ? LOW : HIGH);
}

void aplicarPWM() {
  analogWrite(PIN_ENA, pwmMotorA);
  analogWrite(PIN_ENB, pwmMotorB);
  mostrarPWM();
}

void ajustarMotorA(int alteracao) {
  pwmMotorA = constrain((int)pwmMotorA + alteracao,
                        PWM_MINIMO, PWM_MAXIMO_TESTE);
  Serial.println(F("PWM do motor A ajustado. Repita o comando de movimento."));
  mostrarPWM();
}

void ajustarMotorB(int alteracao) {
  pwmMotorB = constrain((int)pwmMotorB + alteracao,
                        PWM_MINIMO, PWM_MAXIMO_TESTE);
  Serial.println(F("PWM do motor B ajustado. Repita o comando de movimento."));
  mostrarPWM();
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

void mostrarPWM() {
  Serial.print(F("PWM A: "));
  Serial.print(pwmMotorA);
  Serial.print(F(" | PWM B: "));
  Serial.println(pwmMotorB);
}

void imprimirAjuda() {
  Serial.println(F("=== CALIBRACAO L298N ==="));
  Serial.println(F("1/2 - somente motor A, frente/re"));
  Serial.println(F("3/4 - somente motor B, frente/re"));
  Serial.println(F("f/t - dois motores, frente/re"));
  Serial.println(F("e/d - giro nos dois sentidos"));
  Serial.println(F("A/a - aumentar/diminuir PWM do motor A em 5"));
  Serial.println(F("B/b - aumentar/diminuir PWM do motor B em 5"));
  Serial.println(F("p - parada imediata"));
  Serial.println(F("v - mostrar os dois valores PWM"));
  Serial.println(F("h - mostrar ajuda"));
}
