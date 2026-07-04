#include <Servo.h>

// =====================================================
// PINOS DEFINITIVOS
// =====================================================

const int PIN_GARRA    = 4;
const int PIN_COTOVELO = 5;
const int PIN_OMBRO    = 6;
const int PIN_BASE     = 7;

// =====================================================
// SERVOS
// =====================================================

Servo garra;
Servo cotovelo;
Servo ombro;
Servo base;

// =====================================================
// CALIBRAÇÃO DA GARRA
// =====================================================

const int GARRA_ABERTA  = 37;
const int GARRA_FECHADA = 5;

// =====================================================
// ESTRUTURA DA POSE
// =====================================================

struct Pose {
  float altura;
  int base;
  int ombro;
  int cotovelo;
  int garra;
};

// =====================================================
// CURVA EMPÍRICA REAL DO BRAÇO
// =====================================================

Pose curva[] = {
  {18.5, 90, 105, 140, GARRA_ABERTA},
  {17.5, 90, 107, 127, GARRA_ABERTA},
  {15.0, 90, 110, 100, GARRA_ABERTA},
  {13.0, 90, 120, 80,  GARRA_ABERTA},
  {11.0, 90, 130, 75,  GARRA_ABERTA},
  {9.0,  90, 140, 65,  GARRA_ABERTA},
  {7.0,  90, 150, 65,  GARRA_ABERTA},
  {5.0,  90, 155, 65,  GARRA_ABERTA},
  {2.5,  90, 165, 40,  GARRA_ABERTA}
};

const int TOTAL_PONTOS = sizeof(curva) / sizeof(curva[0]);

// =====================================================
// POSIÇÃO INICIAL SEGURA
// =====================================================

Pose POSE_INICIAL = {0, 90, 40, 130, GARRA_ABERTA};

Pose atual = {0, 90, 40, 130, GARRA_ABERTA};

// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(9600);

  garra.attach(PIN_GARRA);
  cotovelo.attach(PIN_COTOVELO);
  ombro.attach(PIN_OMBRO);
  base.attach(PIN_BASE);

  aplicarPose(atual);

  Serial.println("===================================");
  Serial.println(" TESTE CURVA EMPIRICA DO BRACO");
  Serial.println("===================================");
  Serial.println("Comandos:");
  Serial.println("i - posicao inicial segura");
  Serial.println("1 - ir para 18.5 cm");
  Serial.println("2 - ir para 17.5 cm");
  Serial.println("3 - ir para 15 cm");
  Serial.println("4 - ir para 13 cm");
  Serial.println("5 - ir para 11 cm");
  Serial.println("6 - ir para 9 cm");
  Serial.println("7 - ir para 7 cm");
  Serial.println("8 - ir para 5 cm");
  Serial.println("9 - ir para 2.5 cm");
  Serial.println("d - descer pela curva completa");
  Serial.println("s - subir pela curva completa");
  Serial.println("t - teste completo: desce e sobe");
  Serial.println("a - abrir garra");
  Serial.println("f - fechar garra");
  Serial.println("p - mostrar pose atual");
  Serial.println("===================================");

  mostrarPose();
}

// =====================================================
// LOOP
// =====================================================

void loop() {
  if (Serial.available()) {
    char c = Serial.read();

    switch (c) {

      case 'i':
        Serial.println("Indo para posicao inicial segura...");
        irParaPoseSuave(POSE_INICIAL, 25);
        break;

      case '1':
        irParaPontoCurva(0);
        break;

      case '2':
        irParaPontoCurva(1);
        break;

      case '3':
        irParaPontoCurva(2);
        break;

      case '4':
        irParaPontoCurva(3);
        break;

      case '5':
        irParaPontoCurva(4);
        break;

      case '6':
        irParaPontoCurva(5);
        break;

      case '7':
        irParaPontoCurva(6);
        break;

      case '8':
        irParaPontoCurva(7);
        break;

      case '9':
        irParaPontoCurva(8);
        break;

      case 'd':
        descerCurva();
        break;

      case 's':
        subirCurva();
        break;

      case 't':
        testeCompleto();
        break;

      case 'a':
        abrirGarra();
        break;

      case 'f':
        fecharGarra();
        break;

      case 'p':
        mostrarPose();
        break;
    }
  }
}

// =====================================================
// MOVIMENTO PARA PONTO ESPECÍFICO
// =====================================================

void irParaPontoCurva(int indice) {
  if (indice < 0 || indice >= TOTAL_PONTOS) return;

  Serial.print("Indo para h = ");
  Serial.print(curva[indice].altura);
  Serial.println(" cm");

  irParaPoseSuave(curva[indice], 25);
}

// =====================================================
// DESCER E SUBIR PELA CURVA VALIDADA
// =====================================================

void descerCurva() {
  Serial.println("Descendo pela curva empirica...");

  abrirGarra();

  for (int i = 0; i < TOTAL_PONTOS; i++) {
    Serial.print("Ponto ");
    Serial.print(i + 1);
    Serial.print(" | h = ");
    Serial.print(curva[i].altura);
    Serial.println(" cm");

    irParaPoseSuave(curva[i], 25);
    delay(500);
  }

  Serial.println("Fim da descida.");
}

void subirCurva() {
  Serial.println("Subindo pela curva empirica...");

  abrirGarra();

  for (int i = TOTAL_PONTOS - 1; i >= 0; i--) {
    Serial.print("Ponto ");
    Serial.print(i + 1);
    Serial.print(" | h = ");
    Serial.print(curva[i].altura);
    Serial.println(" cm");

    irParaPoseSuave(curva[i], 25);
    delay(500);
  }

  Serial.println("Fim da subida.");
}

void testeCompleto() {
  Serial.println("Teste completo iniciado.");

  irParaPoseSuave(POSE_INICIAL, 25);
  delay(700);

  descerCurva();
  delay(1000);

  fecharGarra();
  delay(1000);

  abrirGarra();
  delay(1000);

  subirCurva();
  delay(1000);

  irParaPoseSuave(POSE_INICIAL, 25);

  Serial.println("Teste completo finalizado.");
}

// =====================================================
// MOVIMENTO SUAVE ENTRE POSES
// =====================================================

void irParaPoseSuave(Pose destino, int tempo) {
  Pose inicio = atual;

  int difBase     = abs(destino.base - inicio.base);
  int difOmbro    = abs(destino.ombro - inicio.ombro);
  int difCotovelo = abs(destino.cotovelo - inicio.cotovelo);
  int difGarra    = abs(destino.garra - inicio.garra);

  int passos = max(max(difBase, difOmbro), max(difCotovelo, difGarra));

  if (passos == 0) {
    mostrarPose();
    return;
  }

  for (int i = 0; i <= passos; i++) {
    float t = (float)i / passos;

    // Movimento com aceleração e desaceleração
    float s = suavizar(t);

    Pose intermediaria;

    intermediaria.altura = destino.altura;

    intermediaria.base = inicio.base + s * (destino.base - inicio.base);
    intermediaria.ombro = inicio.ombro + s * (destino.ombro - inicio.ombro);
    intermediaria.cotovelo = inicio.cotovelo + s * (destino.cotovelo - inicio.cotovelo);
    intermediaria.garra = inicio.garra + s * (destino.garra - inicio.garra);

    aplicarPose(intermediaria);
    delay(tempo);
  }

  atual = destino;
  mostrarPose();
}

// Smoothstep: suaviza início e fim do movimento
float suavizar(float t) {
  return t * t * (3.0 - 2.0 * t);
}

// =====================================================
// APLICAR POSE
// =====================================================

void aplicarPose(Pose p) {
  base.write(p.base);
  ombro.write(p.ombro);
  cotovelo.write(p.cotovelo);
  garra.write(p.garra);
}

// =====================================================
// GARRA
// =====================================================

void abrirGarra() {
  Pose destino = atual;
  destino.garra = GARRA_ABERTA;

  Serial.println("Abrindo garra...");
  irParaPoseSuave(destino, 20);
}

void fecharGarra() {
  Pose destino = atual;
  destino.garra = GARRA_FECHADA;

  Serial.println("Fechando garra...");
  irParaPoseSuave(destino, 20);
}

// =====================================================
// MONITORAMENTO
// =====================================================

void mostrarPose() {
  Serial.print("Pose atual -> h: ");
  Serial.print(atual.altura);

  Serial.print(" cm | Base: ");
  Serial.print(atual.base);

  Serial.print(" | Ombro: ");
  Serial.print(atual.ombro);

  Serial.print(" | Cotovelo: ");
  Serial.print(atual.cotovelo);

  Serial.print(" | Garra: ");
  Serial.println(atual.garra);
}
