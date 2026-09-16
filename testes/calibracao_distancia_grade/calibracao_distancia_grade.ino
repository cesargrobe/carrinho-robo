#include <Servo.h>

// Calibracao empirica da distancia entre o sensor e o plano da folha.
// Selecione uma linha da grade e ajuste ombro/cotovelo em passos de 1 grau.

const byte PIN_GARRA    = 4;
const byte PIN_COTOVELO = 5;
const byte PIN_OMBRO    = 6;
const byte PIN_BASE     = 7;

const byte GARRA_ABERTA = 37;
const byte BASE_CENTRO = 90;
const byte OMBRO_MIN = 40;
const byte OMBRO_MAX = 165;
const byte COTOVELO_MIN = 40;
const byte COTOVELO_MAX = 140;

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

const Pose POSE_SEGURA = {0, BASE_CENTRO, 40, 130, GARRA_ABERTA};

// Curva nominal validada. As linhas usadas pela grade sao os indices 2 a 7.
const Pose curva[] = {
  {18.5, BASE_CENTRO, 105, 140, GARRA_ABERTA},
  {17.5, BASE_CENTRO, 107, 127, GARRA_ABERTA},
  {15.0, BASE_CENTRO, 110, 100, GARRA_ABERTA},
  {13.0, BASE_CENTRO, 120, 80,  GARRA_ABERTA},
  {11.0, BASE_CENTRO, 130, 75,  GARRA_ABERTA},
  {9.0,  BASE_CENTRO, 140, 65,  GARRA_ABERTA},
  {7.0,  BASE_CENTRO, 150, 65,  GARRA_ABERTA},
  {5.0,  BASE_CENTRO, 155, 65,  GARRA_ABERTA},
  {2.5,  BASE_CENTRO, 165, 40,  GARRA_ABERTA}
};

const byte PRIMEIRO_PONTO_GRADE = 2;
const byte TOTAL_LINHAS_GRADE = 6;

Pose atual = POSE_SEGURA;
int linhaAtual = -1;

void setup() {
  Serial.begin(9600);

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

  imprimirAjuda();
  mostrarPose();
}

void loop() {
  if (!Serial.available()) return;

  char comando = Serial.read();

  if (comando >= '1' && comando <= '6') {
    moverParaLinha(comando - '1');
    return;
  }

  switch (comando) {
    case 'q':
      ajustarOmbro(1);
      break;

    case 'a':
      ajustarOmbro(-1);
      break;

    case 'w':
      ajustarCotovelo(1);
      break;

    case 's':
      ajustarCotovelo(-1);
      break;

    case 'r':
      restaurarLinhaNominal();
      break;

    case 'i':
      retornarSeguro();
      break;

    case 'p':
      mostrarPose();
      break;

    case 'h':
      imprimirAjuda();
      break;
  }
}

void moverParaLinha(byte novaLinha) {
  if (novaLinha >= TOTAL_LINHAS_GRADE) return;

  // Volta primeiro a uma pose nominal conhecida antes de trocar de linha.
  if (linhaAtual >= 0) {
    irParaPoseSuave(curva[indiceCurvaDaLinha(linhaAtual)], 30);
  } else {
    irParaPoseSuave(curva[0], 30);
  }

  int indiceAtual = linhaAtual >= 0
                        ? indiceCurvaDaLinha(linhaAtual)
                        : 0;
  int indiceDestino = indiceCurvaDaLinha(novaLinha);
  int passo = indiceDestino > indiceAtual ? 1 : -1;

  while (indiceAtual != indiceDestino) {
    indiceAtual += passo;
    irParaPoseSuave(curva[indiceAtual], 30);
    delay(250);
  }

  linhaAtual = novaLinha;
  Serial.print(F("Linha selecionada: "));
  Serial.println(linhaAtual + 1);
  mostrarPose();
}

void ajustarOmbro(int variacao) {
  if (!linhaSelecionada()) return;

  atual.ombro = constrain(atual.ombro + variacao, OMBRO_MIN, OMBRO_MAX);
  aplicarPose(atual);
  mostrarPose();
}

void ajustarCotovelo(int variacao) {
  if (!linhaSelecionada()) return;

  atual.cotovelo = constrain(atual.cotovelo + variacao,
                             COTOVELO_MIN, COTOVELO_MAX);
  aplicarPose(atual);
  mostrarPose();
}

bool linhaSelecionada() {
  if (linhaAtual >= 0) return true;

  Serial.println(F("Selecione primeiro uma linha de 1 a 6."));
  return false;
}

void restaurarLinhaNominal() {
  if (!linhaSelecionada()) return;

  Serial.println(F("Restaurando os angulos nominais desta linha..."));
  irParaPoseSuave(curva[indiceCurvaDaLinha(linhaAtual)], 30);
  mostrarPose();
}

void retornarSeguro() {
  if (linhaAtual >= 0) {
    int indiceAtual = indiceCurvaDaLinha(linhaAtual);
    irParaPoseSuave(curva[indiceAtual], 30);

    for (int indice = indiceAtual - 1; indice >= 0; indice--) {
      irParaPoseSuave(curva[indice], 30);
      delay(250);
    }
  }

  irParaPoseSuave(POSE_SEGURA, 30);
  linhaAtual = -1;
  mostrarPose();
}

byte indiceCurvaDaLinha(byte linha) {
  return PRIMEIRO_PONTO_GRADE + linha;
}

void irParaPoseSuave(Pose destino, int tempoMs) {
  Pose inicio = atual;

  int diferencaBase = abs(destino.base - inicio.base);
  int diferencaOmbro = abs(destino.ombro - inicio.ombro);
  int diferencaCotovelo = abs(destino.cotovelo - inicio.cotovelo);
  int diferencaGarra = abs(destino.garra - inicio.garra);
  int passos = max(max(diferencaBase, diferencaOmbro),
                   max(diferencaCotovelo, diferencaGarra));

  if (passos == 0) {
    atual = destino;
    return;
  }

  for (int passo = 1; passo <= passos; passo++) {
    float progresso = (float)passo / passos;
    float suave = progresso * progresso * (3.0 - 2.0 * progresso);

    Pose intermediaria;
    intermediaria.altura = destino.altura;
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
  base.write(constrain(pose.base, 60, 120));
  ombro.write(constrain(pose.ombro, OMBRO_MIN, OMBRO_MAX));
  cotovelo.write(constrain(pose.cotovelo, COTOVELO_MIN, COTOVELO_MAX));
  garra.write(constrain(pose.garra, 6, GARRA_ABERTA));
}

void mostrarPose() {
  Serial.print(F("Linha: "));
  if (linhaAtual < 0) {
    Serial.print(F("segura"));
  } else {
    Serial.print(linhaAtual + 1);
  }
  Serial.print(F(" | altura nominal: "));
  Serial.print(atual.altura, 1);
  Serial.print(F(" cm | ombro: "));
  Serial.print(atual.ombro);
  Serial.print(F(" | cotovelo: "));
  Serial.println(atual.cotovelo);
}

void imprimirAjuda() {
  Serial.println(F("=== CALIBRACAO DA DISTANCIA DA GRADE ==="));
  Serial.println(F("1..6 - selecionar linha de 15 a 5 cm"));
  Serial.println(F("q/a - aumentar/diminuir ombro em 1 grau"));
  Serial.println(F("w/s - aumentar/diminuir cotovelo em 1 grau"));
  Serial.println(F("r - restaurar angulos nominais da linha"));
  Serial.println(F("i - retornar com seguranca"));
  Serial.println(F("p - mostrar angulos atuais"));
  Serial.println(F("h - mostrar ajuda"));
}
