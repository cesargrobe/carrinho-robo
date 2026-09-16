#include <Servo.h>

// Teste isolado do golpe de corte.
// Sequencia: fechar -> recolher -> elevar -> abrir.

const byte PIN_GARRA    = 4;
const byte PIN_COTOVELO = 5;
const byte PIN_OMBRO    = 6;
const byte PIN_BASE     = 7;

const byte GARRA_FECHADA = 6;
const byte GARRA_ABERTA  = 37;

const byte BASE_MIN = 60;
const byte BASE_MAX = 120;
const byte OMBRO_MIN = 40;
const byte OMBRO_MAX = 165;
const byte COTOVELO_MIN = 40;
const byte COTOVELO_MAX = 140;

const byte TEMPO_GARRA_MS = 1;
const byte TEMPO_BRACO_MS = 40;

struct Pose {
  int base;
  int ombro;
  int cotovelo;
  int garra;
};

Servo garra;
Servo cotovelo;
Servo ombro;
Servo base;

// Pose mais baixa ja validada na curva empirica.
const Pose POSE_CORTE = {90, 165, 40, GARRA_ABERTA};

// Primeiro o cotovelo recolhe o braco, mantendo o ombro parado.
const Pose POSE_RECUADA = {90, 165, 65, GARRA_FECHADA};

// Depois o ombro eleva o conjunto. Esta pose corresponde ao ponto
// validado de 5 cm da curva empirica.
const Pose POSE_ELEVADA = {90, 155, 65, GARRA_FECHADA};

Pose atual = POSE_CORTE;
bool golpeEmExecucao = false;

void setup() {
  Serial.begin(9600);

  // Energiza uma articulacao por vez para reduzir o pico de corrente.
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

  Serial.println(F("=== TESTE ISOLADO DO GOLPE DE CORTE ==="));
  Serial.println(F("c - executar golpe"));
  Serial.println(F("i - retornar a pose inicial de corte"));
  Serial.println(F("p - mostrar pose"));
  Serial.println(F("Mantenha as maos afastadas das laminas."));
  mostrarPose();
}

void loop() {
  if (!Serial.available() || golpeEmExecucao) return;

  char comando = Serial.read();

  switch (comando) {
    case 'c':
      executarGolpe();
      break;

    case 'i':
      retornarParaCorte();
      break;

    case 'p':
      mostrarPose();
      break;
  }
}

void executarGolpe() {
  golpeEmExecucao = true;

  // Garante que todo golpe comece na mesma geometria.
  retornarParaCorte();

  Serial.println(F("1/4 Fechando as laminas..."));
  Pose destino = atual;
  destino.garra = GARRA_FECHADA;
  irParaPoseSuave(destino, TEMPO_GARRA_MS);
  delay(700);

  Serial.println(F("2/4 Recolhendo o braco..."));
  irParaPoseSuave(POSE_RECUADA, TEMPO_BRACO_MS);
  delay(500);

  Serial.println(F("3/4 Elevando um pouco..."));
  irParaPoseSuave(POSE_ELEVADA, TEMPO_BRACO_MS);
  delay(700);

  Serial.println(F("4/4 Abrindo as laminas..."));
  destino = atual;
  destino.garra = GARRA_ABERTA;
  irParaPoseSuave(destino, TEMPO_GARRA_MS);

  Serial.println(F("Golpe finalizado."));
  mostrarPose();
  golpeEmExecucao = false;
}

void retornarParaCorte() {
  if (posesIguais(atual, POSE_CORTE)) return;

  Serial.println(F("Retornando a pose inicial com a garra aberta..."));

  // Abre antes de avancar novamente para a posicao de corte.
  Pose destino = atual;
  destino.garra = GARRA_ABERTA;
  irParaPoseSuave(destino, TEMPO_GARRA_MS);
  irParaPoseSuave(POSE_CORTE, TEMPO_BRACO_MS);
}

void irParaPoseSuave(Pose destino, int tempoMs) {
  Pose inicio = atual;

  int diferencaBase = abs(destino.base - inicio.base);
  int diferencaOmbro = abs(destino.ombro - inicio.ombro);
  int diferencaCotovelo = abs(destino.cotovelo - inicio.cotovelo);
  int diferencaGarra = abs(destino.garra - inicio.garra);

  int passos = max(max(diferencaBase, diferencaOmbro),
                   max(diferencaCotovelo, diferencaGarra));

  if (passos == 0) return;

  for (int i = 1; i <= passos; i++) {
    float progresso = (float)i / passos;
    float suave = progresso * progresso * (3.0 - 2.0 * progresso);

    Pose intermediaria;
    intermediaria.base = inicio.base + suave * (destino.base - inicio.base);
    intermediaria.ombro = inicio.ombro + suave * (destino.ombro - inicio.ombro);
    intermediaria.cotovelo = inicio.cotovelo + suave * (destino.cotovelo - inicio.cotovelo);
    intermediaria.garra = inicio.garra + suave * (destino.garra - inicio.garra);

    aplicarPose(intermediaria);
    delay(tempoMs);
  }

  atual = destino;
  aplicarPose(atual);
}

void aplicarPose(Pose pose) {
  base.write(constrain(pose.base, BASE_MIN, BASE_MAX));
  ombro.write(constrain(pose.ombro, OMBRO_MIN, OMBRO_MAX));
  cotovelo.write(constrain(pose.cotovelo, COTOVELO_MIN, COTOVELO_MAX));
  garra.write(constrain(pose.garra, GARRA_FECHADA, GARRA_ABERTA));
}

bool posesIguais(Pose a, Pose b) {
  return a.base == b.base &&
         a.ombro == b.ombro &&
         a.cotovelo == b.cotovelo &&
         a.garra == b.garra;
}

void mostrarPose() {
  Serial.print(F("Pose -> Base: "));
  Serial.print(atual.base);
  Serial.print(F(" | Ombro: "));
  Serial.print(atual.ombro);
  Serial.print(F(" | Cotovelo: "));
  Serial.print(atual.cotovelo);
  Serial.print(F(" | Garra: "));
  Serial.println(atual.garra);
}
