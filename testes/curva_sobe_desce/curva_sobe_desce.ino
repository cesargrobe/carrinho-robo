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
const int GARRA_FECHADA = 6;
const int CICLOS_GARRA_RAPIDA = 3;
const int INTERVALO_GARRA_RAPIDA_MS = 350;

const int BASE_MIN = 60;
const int BASE_MAX = 120;
const int OMBRO_MIN = 40;
const int OMBRO_MAX = 165;
const int COTOVELO_MIN = 40;
const int COTOVELO_MAX = 140;

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

  // Prepara e energiza uma articulacao por vez para reduzir o pico de corrente.
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
  Serial.println("c - teste de corte: fecha, eleva e recolhe");
  Serial.println("a - abrir garra");
  Serial.println("f - fechar garra suavemente");
  Serial.println("F - fechar garra imediatamente");
  Serial.println(F("r - fechar e abrir rapidamente 3 vezes"));
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

      case 'c':
        testeCorteComTracao();
        break;

      case 'a':
        abrirGarra();
        break;

      case 'f':
        fecharGarra();
        break;

      case 'F':
        fecharGarraRapido();
        break;

      case 'r':
        testarGarraRapidaTresVezes();
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

  int indiceAtual = localizarPontoAtual();

  // Se o braco estiver fora da curva, entra sempre pelo ponto mais alto.
  if (indiceAtual == -1) {
    Serial.println("Entrando na curva pelo ponto mais alto...");
    irParaPoseSuave(curva[0], 35);
    delay(500);
    indiceAtual = 0;
  }

  int passo = indice > indiceAtual ? 1 : -1;
  while (indiceAtual != indice) {
    indiceAtual += passo;

    Serial.print("Indo para h = ");
    Serial.print(curva[indiceAtual].altura);
    Serial.println(" cm");

    irParaPoseSuave(curva[indiceAtual], 35);
    delay(500);
  }
}

int localizarPontoAtual() {
  for (int i = 0; i < TOTAL_PONTOS; i++) {
    if (atual.base == curva[i].base &&
        atual.ombro == curva[i].ombro &&
        atual.cotovelo == curva[i].cotovelo &&
        atual.garra == curva[i].garra) {
      return i;
    }
  }

  return -1;
}

// =====================================================
// DESCER E SUBIR PELA CURVA VALIDADA
// =====================================================

void descerCurva() {
  Serial.println("Descendo pela curva empirica...");

  abrirGarra();

  // Sempre entra na curva pelo ponto mais alto. Isso evita cortar caminho
  // desde uma pose baixa ou desconhecida ate um ponto intermediario.
  irParaPontoCurva(0);

  for (int i = 1; i < TOTAL_PONTOS; i++) {
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

  // A subida deve comecar no ponto mais baixo da curva.
  if (atual.ombro != curva[TOTAL_PONTOS - 1].ombro ||
      atual.cotovelo != curva[TOTAL_PONTOS - 1].cotovelo) {
    Serial.println("Movimento recusado: use 'd' antes de subir.");
    return;
  }

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
// CORTE COM PEQUENA ELEVACAO E RECOLHIMENTO
// =====================================================

void testeCorteComTracao() {
  Serial.println("Teste de corte iniciado.");
  Serial.println("Mantenha as maos afastadas das laminas.");

  // Aproxima sempre pela curva empirica validada e termina em 2,5 cm.
  irParaPoseSuave(POSE_INICIAL, 25);
  delay(700);
  descerCurva();
  delay(500);

  // Primeiro prende/corta o caule com as laminas.
  fecharGarra();
  delay(800);

  // Mantem a garra fechada e retorna apenas um passo da curva:
  // de 2,5 para 5 cm. Esse passo eleva e recolhe ligeiramente o braco.
  Pose tracao = curva[TOTAL_PONTOS - 2];
  tracao.garra = GARRA_FECHADA;

  Serial.println("Elevando e recolhendo com as laminas fechadas...");
  irParaPoseSuave(tracao, 35);
  delay(800);

  // Solta somente depois de concluir o pequeno movimento de tracao.
  abrirGarra();
  delay(500);

  // Recolhe pelos demais pontos validados, sem cortar caminho.
  for (int i = TOTAL_PONTOS - 3; i >= 0; i--) {
    irParaPoseSuave(curva[i], 25);
    delay(350);
  }

  irParaPoseSuave(POSE_INICIAL, 25);
  Serial.println("Teste de corte finalizado.");
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
  base.write(constrain(p.base, BASE_MIN, BASE_MAX));
  ombro.write(constrain(p.ombro, OMBRO_MIN, OMBRO_MAX));
  cotovelo.write(constrain(p.cotovelo, COTOVELO_MIN, COTOVELO_MAX));
  garra.write(constrain(p.garra, GARRA_FECHADA, GARRA_ABERTA));
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

void fecharGarraRapido() {
  Serial.println("Fechando garra imediatamente...");

  // Envia o destino final em um unico comando, sem interpolacao.
  // Base, ombro e cotovelo permanecem exatamente na pose atual.
  garra.write(GARRA_FECHADA);
  atual.garra = GARRA_FECHADA;
  mostrarPose();
}

void testarGarraRapidaTresVezes() {
  Serial.println(F("Teste rapido da garra: 3 ciclos."));
  Serial.println(F("Mantenha as maos afastadas das laminas."));

  for (int ciclo = 1; ciclo <= CICLOS_GARRA_RAPIDA; ciclo++) {
    Serial.print(F("Ciclo "));
    Serial.print(ciclo);
    Serial.println(F(" - fechando"));
    garra.write(GARRA_FECHADA);
    atual.garra = GARRA_FECHADA;
    delay(INTERVALO_GARRA_RAPIDA_MS);

    Serial.print(F("Ciclo "));
    Serial.print(ciclo);
    Serial.println(F(" - abrindo"));
    garra.write(GARRA_ABERTA);
    atual.garra = GARRA_ABERTA;
    delay(INTERVALO_GARRA_RAPIDA_MS);
  }

  Serial.println(F("Teste rapido finalizado com a garra aberta."));
  mostrarPose();
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
