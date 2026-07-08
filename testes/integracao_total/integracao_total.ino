#include <Servo.h>

// =====================================================
// MAPA DEFINITIVO DE PINOS
// =====================================================

const byte PIN_ECHO = 2;
const byte PIN_ENA = 3;
const byte PIN_GARRA = 4;
const byte PIN_COTOVELO = 5;
const byte PIN_OMBRO = 6;
const byte PIN_BASE = 7;
const byte PIN_IN1 = 8;
const byte PIN_IN2 = 10;
const byte PIN_ENB = 11;
const byte PIN_IN3 = 12;
const byte PIN_TRIG = 13;
const byte PIN_IN4 = A5;

const byte PIN_COR_OUT = A0;
const byte PIN_COR_S0 = A1;
const byte PIN_COR_S1 = A2;
const byte PIN_COR_S2 = A3;
const byte PIN_COR_S3 = A4;

// =====================================================
// CONFIGURACOES
// =====================================================

const byte GARRA_FECHADA = 6;
const byte GARRA_ABERTA = 37;
const byte PWM_TESTE = 80;
const unsigned long TIMEOUT_ECO_US = 30000UL;
const unsigned long TIMEOUT_COR_US = 30000UL;

struct Pose {
  float altura;
  int base;
  int ombro;
  int cotovelo;
  int garra;
};

Servo garra;
Servo cotovelo;
Servo ombro;
Servo base;

const Pose POSE_SEGURA = {0, 90, 40, 130, GARRA_ABERTA};

const Pose curva[] = {
  {18.5, 90, 105, 140, GARRA_ABERTA},
  {17.5, 90, 107, 127, GARRA_ABERTA},
  {15.0, 90, 110, 100, GARRA_ABERTA},
  {13.0, 90, 120, 80, GARRA_ABERTA},
  {11.0, 90, 130, 75, GARRA_ABERTA},
  {9.0, 90, 140, 65, GARRA_ABERTA},
  {7.0, 90, 150, 65, GARRA_ABERTA},
  {5.0, 90, 155, 65, GARRA_ABERTA},
  {2.5, 90, 165, 40, GARRA_ABERTA}
};

const byte TOTAL_PONTOS = sizeof(curva) / sizeof(curva[0]);
Pose atual = POSE_SEGURA;
bool testeEmExecucao = false;

void setup() {
  Serial.begin(9600);

  configurarMotores();
  configurarUltrassonico();
  configurarSensorCor();
  configurarServos();

  imprimirAjuda();
}

void loop() {
  if (!Serial.available() || testeEmExecucao) return;

  char comando = Serial.read();

  switch (comando) {
    case 'u': testarUltrassonico(); break;
    case 'c': testarSensorCor(); break;
    case 'b': testarBraco(); break;
    case 'g': testarGarra(); break;
    case 'm': testarMotores(); break;
    case 't': testeCompleto(); break;
    case 'p': pararMotores(); break;
    case 'h': imprimirAjuda(); break;
  }
}

void configurarMotores() {
  pinMode(PIN_ENA, OUTPUT);
  pinMode(PIN_ENB, OUTPUT);
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);
  pararMotores();
}

void configurarUltrassonico() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);
}

void configurarSensorCor() {
  pinMode(PIN_COR_OUT, INPUT);
  pinMode(PIN_COR_S0, OUTPUT);
  pinMode(PIN_COR_S1, OUTPUT);
  pinMode(PIN_COR_S2, OUTPUT);
  pinMode(PIN_COR_S3, OUTPUT);
  digitalWrite(PIN_COR_S0, HIGH);
  digitalWrite(PIN_COR_S1, LOW);
}

void configurarServos() {
  garra.write(atual.garra);
  garra.attach(PIN_GARRA);
  delay(300);
  cotovelo.write(atual.cotovelo);
  cotovelo.attach(PIN_COTOVELO);
  delay(300);
  ombro.write(atual.ombro);
  ombro.attach(PIN_OMBRO);
  delay(300);
  base.write(atual.base);
  base.attach(PIN_BASE);
  delay(700);
}

void testeCompleto() {
  testeEmExecucao = true;
  Serial.println(F("=== INICIO DO TESTE COMPLETO ==="));

  pararMotores();
  testarUltrassonico();
  testarSensorCor();
  testarBraco();
  testarGarra();
  testarMotores();

  pararMotores();
  irParaPoseSuave(POSE_SEGURA, 30);
  Serial.println(F("=== TESTE COMPLETO FINALIZADO ==="));
  testeEmExecucao = false;
}

void testarUltrassonico() {
  float distancia = medirDistancia();
  Serial.print(F("HC-SR04: "));
  if (distancia < 0) {
    Serial.println(F("sem eco"));
  } else {
    Serial.print(distancia, 1);
    Serial.println(F(" cm"));
  }
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

void testarSensorCor() {
  float vermelho = lerCanalCor(LOW, LOW);
  float azul = lerCanalCor(LOW, HIGH);
  float verde = lerCanalCor(HIGH, HIGH);
  float total = vermelho + verde + azul;

  if (total <= 0) {
    Serial.println(F("TCS3200: leitura invalida"));
    return;
  }

  Serial.print(F("TCS3200 -> R: "));
  Serial.print(vermelho / total, 3);
  Serial.print(F(" | G: "));
  Serial.print(verde / total, 3);
  Serial.print(F(" | B: "));
  Serial.println(azul / total, 3);
}

float lerCanalCor(byte estadoS2, byte estadoS3) {
  digitalWrite(PIN_COR_S2, estadoS2);
  digitalWrite(PIN_COR_S3, estadoS3);
  delay(3);

  float soma = 0;
  byte validas = 0;

  for (byte i = 0; i < 5; i++) {
    unsigned long pulso = pulseIn(PIN_COR_OUT, LOW, TIMEOUT_COR_US);
    if (pulso > 0) {
      soma += 500000.0 / pulso;
      validas++;
    }
  }

  return validas > 0 ? soma / validas : 0;
}

void testarBraco() {
  Serial.println(F("Braco: descendo pela curva"));
  irParaPoseSuave(POSE_SEGURA, 30);

  for (byte i = 0; i < TOTAL_PONTOS; i++) {
    irParaPoseSuave(curva[i], 25);
    delay(300);
  }

  Serial.println(F("Braco: subindo pela curva"));
  for (int i = TOTAL_PONTOS - 2; i >= 0; i--) {
    irParaPoseSuave(curva[i], 25);
    delay(300);
  }

  irParaPoseSuave(POSE_SEGURA, 30);
}

void testarGarra() {
  Serial.println(F("Garra: fechando"));
  Pose destino = atual;
  destino.garra = GARRA_FECHADA;
  irParaPoseSuave(destino, 20);
  delay(700);

  Serial.println(F("Garra: abrindo"));
  destino.garra = GARRA_ABERTA;
  irParaPoseSuave(destino, 20);
}

void testarMotores() {
  Serial.println(F("Motores: rodas devem estar suspensas"));
  pararMotores();

  configurarSentidoFrente();
  analogWrite(PIN_ENA, PWM_TESTE);
  delay(800);
  pararMotores();
  delay(400);

  configurarSentidoFrente();
  analogWrite(PIN_ENB, PWM_TESTE);
  delay(800);
  pararMotores();
  delay(400);

  configurarSentidoFrente();
  analogWrite(PIN_ENA, PWM_TESTE);
  analogWrite(PIN_ENB, PWM_TESTE);
  delay(1000);
  pararMotores();
}

void configurarSentidoFrente() {
  digitalWrite(PIN_IN1, HIGH);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, HIGH);
  digitalWrite(PIN_IN4, LOW);
}

void pararMotores() {
  analogWrite(PIN_ENA, 0);
  analogWrite(PIN_ENB, 0);
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);
}

void irParaPoseSuave(Pose destino, int tempo) {
  Pose inicio = atual;
  int passos = max(max(abs(destino.base - inicio.base),
                       abs(destino.ombro - inicio.ombro)),
                   max(abs(destino.cotovelo - inicio.cotovelo),
                       abs(destino.garra - inicio.garra)));

  if (passos == 0) return;

  for (int i = 1; i <= passos; i++) {
    float progresso = (float)i / passos;
    float suave = progresso * progresso * (3.0 - 2.0 * progresso);

    Pose intermediaria;
    intermediaria.altura = destino.altura;
    intermediaria.base = inicio.base + suave * (destino.base - inicio.base);
    intermediaria.ombro = inicio.ombro + suave * (destino.ombro - inicio.ombro);
    intermediaria.cotovelo = inicio.cotovelo + suave * (destino.cotovelo - inicio.cotovelo);
    intermediaria.garra = inicio.garra + suave * (destino.garra - inicio.garra);

    aplicarPose(intermediaria);
    delay(tempo);
  }

  atual = destino;
  aplicarPose(atual);
}

void aplicarPose(Pose pose) {
  base.write(constrain(pose.base, 60, 120));
  ombro.write(constrain(pose.ombro, 40, 165));
  cotovelo.write(constrain(pose.cotovelo, 40, 140));
  garra.write(constrain(pose.garra, GARRA_FECHADA, GARRA_ABERTA));
}

void imprimirAjuda() {
  Serial.println(F("=== DIAGNOSTICO INTEGRADO ==="));
  Serial.println(F("u - HC-SR04"));
  Serial.println(F("c - sensor de cor"));
  Serial.println(F("b - curva completa do braco"));
  Serial.println(F("g - garra"));
  Serial.println(F("m - motores (rodas suspensas)"));
  Serial.println(F("t - executar todos os testes"));
  Serial.println(F("p - parar motores"));
}
