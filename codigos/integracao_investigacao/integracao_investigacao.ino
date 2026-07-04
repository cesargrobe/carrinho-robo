#include <Servo.h>

const byte PIN_GARRA    = 4;
const byte PIN_COTOVELO = 5;
const byte PIN_OMBRO    = 6;
const byte PIN_BASE     = 7;

const byte PIN_COR_OUT = A0;
const byte PIN_COR_S0  = A1;
const byte PIN_COR_S1  = A2;
const byte PIN_COR_S2  = A3;
const byte PIN_COR_S3  = A4;

const byte GARRA_FECHADA = 5;
const byte GARRA_ABERTA  = 37;

const byte BASE_DIREITA  = 60;
const byte BASE_CENTRO   = 90;
const byte BASE_ESQUERDA = 120;

const byte OMBRO_MIN = 40;
const byte OMBRO_MAX = 165;
const byte COTOVELO_MIN = 40;
const byte COTOVELO_MAX = 140;

const byte TEMPO_NORMAL_MS = 30;
const byte TEMPO_CURVA_MS = 25;

const byte AMOSTRAS_COR = 7;
const byte LEITURAS_CONFIRMACAO = 3;
const byte SUSPEITAS_NECESSARIAS = 2;
const unsigned long TIMEOUT_COR_US = 30000UL;

// Diferenca maxima entre as proporcoes RGB da amostra e da referencia.
// Comece com 0.18 e ajuste observando os valores mostrados no Serial.
const float LIMIAR_DISCREPANCIA = 0.18;

const byte BASE_VARREDURA_MIN = 70;
const byte BASE_VARREDURA_MAX = 110;
const byte PASSO_BASE = 5;
const byte TOTAL_COLUNAS =
    ((BASE_VARREDURA_MAX - BASE_VARREDURA_MIN) / PASSO_BASE) + 1;
const float RAIO_SENSOR_CM = 16.0;
const float LARGURA_PASSO_CM = 1.40;

struct Pose {
  float altura;
  int base;
  int ombro;
  int cotovelo;
  int garra;
};

struct LeituraCor {
  float vermelho;
  float verde;
  float azul;
  float total;
  bool valida;
};

struct RegiaoSuspeita {
  bool encontrada;
  byte quantidade;
  byte linhaMin;
  byte linhaMax;
  byte colunaMin;
  byte colunaMax;
  unsigned int somaLinhas;
  unsigned int somaColunas;
};

Servo garra;
Servo cotovelo;
Servo ombro;
Servo base;

const Pose POSE_SEGURA = {0, BASE_CENTRO, 40, 130, GARRA_ABERTA};

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

const byte TOTAL_PONTOS = sizeof(curva) / sizeof(curva[0]);

Pose atual = POSE_SEGURA;
bool cicloEmExecucao = false;
LeituraCor corSaudavel = {0, 0, 0, 0, false};
bool mapaSuspeito[TOTAL_PONTOS][TOTAL_COLUNAS];
bool mapaVisitado[TOTAL_PONTOS][TOTAL_COLUNAS];

void setup() {
  Serial.begin(9600);

  pinMode(PIN_COR_S0, OUTPUT);
  pinMode(PIN_COR_S1, OUTPUT);
  pinMode(PIN_COR_S2, OUTPUT);
  pinMode(PIN_COR_S3, OUTPUT);
  pinMode(PIN_COR_OUT, INPUT);

  // Escala de frequencia em 20% no TCS230/TCS3200.
  digitalWrite(PIN_COR_S0, HIGH);
  digitalWrite(PIN_COR_S1, LOW);

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

  imprimirAjuda();
  mostrarPose();
}

void loop() {
  if (!Serial.available() || cicloEmExecucao) return;

  char comando = Serial.read();

  switch (comando) {
    case 'v':
      executarVarredura();
      break;

    case 'i':
      executarInvestigacaoAutomatica();
      break;

    case 't':
      executarTesteCompleto();
      break;

    case 'r':
      executarCorteManual();
      break;

    case 'c':
      calibrarCorSaudavel();
      break;

    case 'l':
      mostrarLeituraCor(lerCor());
      break;

    case 's':
      retornarSeguro();
      break;

    case 'p':
      mostrarPose();
      break;

    case 'h':
      imprimirAjuda();
      break;

    case '1': case '2': case '3':
    case '4': case '5': case '6':
    case '7': case '8': case '9':
      moverParaPontoCurva(comando - '1');
      break;
  }
}

void executarTesteCompleto() {
  Serial.println(F("========================================"));
  Serial.println(F(" TESTE COMPLETO: VARREDURA, LEITURA E CORTE"));
  Serial.println(F("========================================"));

  if (!corSaudavel.valida) {
    Serial.println(F("Teste bloqueado: calibre uma folha saudavel com 'c'."));
    return;
  }

  Serial.println(F("O corte ocorrera somente se 2 de 3 leituras forem suspeitas."));
  executarInvestigacaoAutomatica();

  Serial.println(F("========================================"));
  Serial.println(F(" TESTE COMPLETO FINALIZADO"));
  Serial.println(F("========================================"));
}

void executarInvestigacaoAutomatica() {
  if (!corSaudavel.valida) {
    Serial.println(F("Investigacao bloqueada: calibre primeiro com o comando 'c'."));
    return;
  }

  cicloEmExecucao = true;
  limparMapas();

  Serial.println(F("=== VARREDURA 2D DA AREA ==="));
  retornarSeguro();

  byte colunaAtual = TOTAL_COLUNAS - 1;
  Pose destino = curva[0];
  destino.base = anguloDaColuna(colunaAtual);
  irParaPoseSuave(destino, TEMPO_NORMAL_MS);

  for (byte linha = 0; linha < TOTAL_PONTOS; linha++) {
    destino = curva[linha];
    destino.base = anguloDaColuna(colunaAtual);
    irParaPoseSuave(destino, TEMPO_CURVA_MS);

    bool esquerdaParaDireita = (linha % 2) == 1;
    int inicio = esquerdaParaDireita ? 0 : TOTAL_COLUNAS - 1;
    int fim = esquerdaParaDireita ? TOTAL_COLUNAS : -1;
    int passo = esquerdaParaDireita ? 1 : -1;

    for (int coluna = inicio; coluna != fim; coluna += passo) {
      destino = curva[linha];
      destino.base = anguloDaColuna(coluna);
      irParaPoseSuave(destino, TEMPO_CURVA_MS);
      colunaAtual = coluna;
      delay(250);

      Serial.print(F("Celula -> altura: "));
      Serial.print(curva[linha].altura);
      Serial.print(F(" cm | base: "));
      Serial.println(destino.base);

      mapaSuspeito[linha][coluna] = confirmarDiscrepancia();
    }
  }

  RegiaoSuspeita regiao = localizarMaiorRegiao();

  if (regiao.encontrada) {
    byte linhaCentro = (regiao.somaLinhas + regiao.quantidade / 2) /
                       regiao.quantidade;
    byte colunaCentro = (regiao.somaColunas + regiao.quantidade / 2) /
                        regiao.quantidade;

    mostrarRegiao(regiao, linhaCentro, colunaCentro);
    moverDaUltimaLinhaPara(linhaCentro, colunaCentro);

    Serial.println(F("Centro da regiao alcancado: efetuando o corte."));
    moverGarra(GARRA_FECHADA);
    delay(1000);
    moverGarra(GARRA_ABERTA);
    delay(500);

    recolherDaLinha(linhaCentro);
  } else {
    Serial.println(F("Nenhuma area suspeita conectada foi encontrada."));
    recolherDaLinha(TOTAL_PONTOS - 1);
  }

  Serial.println(F("=== FIM DA INVESTIGACAO ==="));

  cicloEmExecucao = false;
}

byte anguloDaColuna(byte coluna) {
  return BASE_VARREDURA_MIN + coluna * PASSO_BASE;
}

void limparMapas() {
  for (byte linha = 0; linha < TOTAL_PONTOS; linha++) {
    for (byte coluna = 0; coluna < TOTAL_COLUNAS; coluna++) {
      mapaSuspeito[linha][coluna] = false;
      mapaVisitado[linha][coluna] = false;
    }
  }
}

RegiaoSuspeita localizarMaiorRegiao() {
  RegiaoSuspeita maior = {false, 0, 0, 0, 0, 0, 0, 0};
  byte filaLinha[TOTAL_PONTOS * TOTAL_COLUNAS];
  byte filaColuna[TOTAL_PONTOS * TOTAL_COLUNAS];

  for (byte linha = 0; linha < TOTAL_PONTOS; linha++) {
    for (byte coluna = 0; coluna < TOTAL_COLUNAS; coluna++) {
      if (!mapaSuspeito[linha][coluna] || mapaVisitado[linha][coluna]) continue;

      RegiaoSuspeita atualRegiao = {true, 0, linha, linha, coluna, coluna, 0, 0};
      byte inicioFila = 0;
      byte fimFila = 0;

      filaLinha[fimFila] = linha;
      filaColuna[fimFila] = coluna;
      fimFila++;
      mapaVisitado[linha][coluna] = true;

      while (inicioFila < fimFila) {
        byte linhaAtual = filaLinha[inicioFila];
        byte colunaAtual = filaColuna[inicioFila];
        inicioFila++;

        atualRegiao.quantidade++;
        atualRegiao.somaLinhas += linhaAtual;
        atualRegiao.somaColunas += colunaAtual;
        atualRegiao.linhaMin = min(atualRegiao.linhaMin, linhaAtual);
        atualRegiao.linhaMax = max(atualRegiao.linhaMax, linhaAtual);
        atualRegiao.colunaMin = min(atualRegiao.colunaMin, colunaAtual);
        atualRegiao.colunaMax = max(atualRegiao.colunaMax, colunaAtual);

        for (int deslocamentoLinha = -1; deslocamentoLinha <= 1; deslocamentoLinha++) {
          for (int deslocamentoColuna = -1; deslocamentoColuna <= 1; deslocamentoColuna++) {
            if (deslocamentoLinha == 0 && deslocamentoColuna == 0) continue;

            int vizinhaLinha = linhaAtual + deslocamentoLinha;
            int vizinhaColuna = colunaAtual + deslocamentoColuna;

            if (vizinhaLinha < 0 || vizinhaLinha >= TOTAL_PONTOS ||
                vizinhaColuna < 0 || vizinhaColuna >= TOTAL_COLUNAS) continue;

            if (mapaSuspeito[vizinhaLinha][vizinhaColuna] &&
                !mapaVisitado[vizinhaLinha][vizinhaColuna]) {
              mapaVisitado[vizinhaLinha][vizinhaColuna] = true;
              filaLinha[fimFila] = vizinhaLinha;
              filaColuna[fimFila] = vizinhaColuna;
              fimFila++;
            }
          }
        }
      }

      if (atualRegiao.quantidade > maior.quantidade) maior = atualRegiao;
    }
  }

  return maior;
}

void mostrarRegiao(RegiaoSuspeita regiao, byte linhaCentro, byte colunaCentro) {
  float larguraCm = (regiao.colunaMax - regiao.colunaMin + 1) * LARGURA_PASSO_CM;
  float alturaCm = curva[regiao.linhaMin].altura - curva[regiao.linhaMax].altura;

  Serial.println(F("=== MAIOR AREA SUSPEITA ==="));
  Serial.print(F("Celulas: ")); Serial.println(regiao.quantidade);
  Serial.print(F("Largura aproximada: ")); Serial.print(larguraCm, 1);
  Serial.println(F(" cm"));
  Serial.print(F("Altura aproximada: ")); Serial.print(alturaCm, 1);
  Serial.println(F(" cm"));
  Serial.print(F("Centro -> altura: ")); Serial.print(curva[linhaCentro].altura);
  Serial.print(F(" cm | base: ")); Serial.println(anguloDaColuna(colunaCentro));
}

void moverDaUltimaLinhaPara(byte linhaDestino, byte colunaDestino) {
  for (int linha = TOTAL_PONTOS - 2; linha >= linhaDestino; linha--) {
    Pose destino = curva[linha];
    destino.base = atual.base;
    irParaPoseSuave(destino, TEMPO_CURVA_MS);
    delay(300);
  }

  Pose centro = curva[linhaDestino];
  centro.base = anguloDaColuna(colunaDestino);
  irParaPoseSuave(centro, TEMPO_CURVA_MS);
}

void recolherDaLinha(byte linhaAtual) {
  for (int linha = linhaAtual - 1; linha >= 0; linha--) {
    Pose destino = curva[linha];
    destino.base = atual.base;
    irParaPoseSuave(destino, TEMPO_CURVA_MS);
    delay(300);
  }

  Pose topoCentral = curva[0];
  topoCentral.base = BASE_CENTRO;
  irParaPoseSuave(topoCentral, TEMPO_NORMAL_MS);
  irParaPoseSuave(POSE_SEGURA, TEMPO_NORMAL_MS);
}

void executarCorteManual() {
  Serial.println(F("Corte manual na pose atual."));
  moverGarra(GARRA_FECHADA);
  delay(1000);
  moverGarra(GARRA_ABERTA);
}

void calibrarCorSaudavel() {
  Serial.println(F("Calibrando com a folha saudavel..."));
  Serial.println(F("Mantenha a folha e a iluminacao paradas."));
  delay(1000);

  LeituraCor acumulada = {0, 0, 0, 0, false};
  byte validas = 0;

  for (byte i = 0; i < 5; i++) {
    LeituraCor leitura = lerCor();
    if (leitura.valida) {
      acumulada.vermelho += leitura.vermelho;
      acumulada.verde += leitura.verde;
      acumulada.azul += leitura.azul;
      validas++;
    }
    delay(150);
  }

  if (validas < 3) {
    Serial.println(F("Falha na calibracao: sinal insuficiente do sensor."));
    corSaudavel.valida = false;
    return;
  }

  corSaudavel.vermelho = acumulada.vermelho / validas;
  corSaudavel.verde = acumulada.verde / validas;
  corSaudavel.azul = acumulada.azul / validas;
  corSaudavel.total = corSaudavel.vermelho +
                      corSaudavel.verde +
                      corSaudavel.azul;
  corSaudavel.valida = corSaudavel.total > 0;

  Serial.println(F("Referencia saudavel registrada:"));
  mostrarLeituraCor(corSaudavel);
}

bool confirmarDiscrepancia() {
  byte suspeitas = 0;

  for (byte i = 0; i < LEITURAS_CONFIRMACAO; i++) {
    LeituraCor leitura = lerCor();
    mostrarLeituraCor(leitura);

    if (leitura.valida && corDiferenteDaReferencia(leitura)) {
      suspeitas++;
      Serial.println(F("Leitura suspeita."));
    } else {
      Serial.println(F("Leitura compativel ou invalida."));
    }

    delay(180);
  }

  Serial.print(F("Confirmacoes suspeitas: "));
  Serial.print(suspeitas);
  Serial.print(F("/"));
  Serial.println(LEITURAS_CONFIRMACAO);

  return suspeitas >= SUSPEITAS_NECESSARIAS;
}

bool corDiferenteDaReferencia(LeituraCor leitura) {
  if (!leitura.valida || !corSaudavel.valida) return false;

  float vermelhoAtual = leitura.vermelho / leitura.total;
  float verdeAtual = leitura.verde / leitura.total;
  float azulAtual = leitura.azul / leitura.total;

  float vermelhoReferencia = corSaudavel.vermelho / corSaudavel.total;
  float verdeReferencia = corSaudavel.verde / corSaudavel.total;
  float azulReferencia = corSaudavel.azul / corSaudavel.total;

  float diferencaVermelho = abs(vermelhoAtual - vermelhoReferencia);
  float diferencaVerde = abs(verdeAtual - verdeReferencia);
  float diferencaAzul = abs(azulAtual - azulReferencia);

  float maiorDiferenca = max(diferencaVermelho,
                             max(diferencaVerde, diferencaAzul));

  Serial.print(F("Discrepancia: "));
  Serial.println(maiorDiferenca, 3);

  return maiorDiferenca >= LIMIAR_DISCREPANCIA;
}

LeituraCor lerCor() {
  LeituraCor leitura;

  leitura.vermelho = lerCanalCor(LOW, LOW);
  leitura.azul = lerCanalCor(LOW, HIGH);
  leitura.verde = lerCanalCor(HIGH, HIGH);
  leitura.total = leitura.vermelho + leitura.verde + leitura.azul;
  leitura.valida = leitura.vermelho > 0 &&
                   leitura.verde > 0 &&
                   leitura.azul > 0 &&
                   leitura.total > 0;

  return leitura;
}

float lerCanalCor(byte estadoS2, byte estadoS3) {
  digitalWrite(PIN_COR_S2, estadoS2);
  digitalWrite(PIN_COR_S3, estadoS3);
  delay(3);

  float somaFrequencias = 0;
  byte amostrasValidas = 0;

  for (byte i = 0; i < AMOSTRAS_COR; i++) {
    unsigned long pulso = pulseIn(PIN_COR_OUT, LOW, TIMEOUT_COR_US);

    if (pulso > 0) {
      // A frequencia e proporcional a intensidade luminosa do canal.
      somaFrequencias += 500000.0 / pulso;
      amostrasValidas++;
    }
  }

  if (amostrasValidas == 0) return 0;
  return somaFrequencias / amostrasValidas;
}

void mostrarLeituraCor(LeituraCor leitura) {
  if (!leitura.valida) {
    Serial.println(F("RGB: leitura invalida"));
    return;
  }

  Serial.print(F("RGB normalizado -> R: "));
  Serial.print(leitura.vermelho / leitura.total, 3);
  Serial.print(F(" | G: "));
  Serial.print(leitura.verde / leitura.total, 3);
  Serial.print(F(" | B: "));
  Serial.print(leitura.azul / leitura.total, 3);
  Serial.print(F(" | Intensidade: "));
  Serial.println(leitura.total, 1);
}

void moverParaPontoCurva(int destino) {
  if (destino < 0 || destino >= TOTAL_PONTOS) return;

  int origem = localizarPontoCurva();
  if (origem < 0) {
    retornarSeguro();
    irParaPoseSuave(curva[0], TEMPO_NORMAL_MS);
    origem = 0;
  }

  int passo = destino > origem ? 1 : -1;
  while (origem != destino) {
    origem += passo;
    irParaPoseSuave(curva[origem], TEMPO_CURVA_MS);
    delay(350);
  }

  mostrarPose();
  Serial.println(F("Posicione uma folha saudavel diante do sensor e envie 'c'."));
}

void executarVarredura() {
  Serial.println(F("Varredura da base..."));

  // A base somente gira quando o restante do braco esta recolhido.
  retornarSeguro();
  moverBase(BASE_DIREITA);
  delay(500);
  moverBase(BASE_CENTRO);
  delay(500);
  moverBase(BASE_ESQUERDA);
  delay(500);
  moverBase(BASE_CENTRO);
  delay(500);

  Serial.println(F("Varredura concluida."));
}

void retornarSeguro() {
  if (posesIguais(atual, POSE_SEGURA)) return;

  Serial.println(F("Retornando para a posicao segura..."));

  // Se estiver na curva, passa primeiro pelo ponto mais alto.
  int indice = localizarPontoCurva();
  if (indice > 0) {
    for (int i = indice - 1; i >= 0; i--) {
      irParaPoseSuave(curva[i], TEMPO_CURVA_MS);
      delay(300);
    }
  }

  irParaPoseSuave(POSE_SEGURA, TEMPO_NORMAL_MS);
}

void moverBase(int destino) {
  Pose novaPose = atual;
  novaPose.base = constrain(destino, BASE_DIREITA, BASE_ESQUERDA);
  irParaPoseSuave(novaPose, TEMPO_NORMAL_MS);
}

void moverGarra(int destino) {
  Pose novaPose = atual;
  novaPose.garra = constrain(destino, GARRA_FECHADA, GARRA_ABERTA);
  irParaPoseSuave(novaPose, 20);
}

void irParaPoseSuave(Pose destino, int tempo) {
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
  base.write(constrain(pose.base, BASE_DIREITA, BASE_ESQUERDA));
  ombro.write(constrain(pose.ombro, OMBRO_MIN, OMBRO_MAX));
  cotovelo.write(constrain(pose.cotovelo, COTOVELO_MIN, COTOVELO_MAX));
  garra.write(constrain(pose.garra, GARRA_FECHADA, GARRA_ABERTA));
}

int localizarPontoCurva() {
  for (byte i = 0; i < TOTAL_PONTOS; i++) {
    if (posesIguais(atual, curva[i])) return i;
  }

  return -1;
}

bool posesIguais(Pose a, Pose b) {
  return a.base == b.base &&
         a.ombro == b.ombro &&
         a.cotovelo == b.cotovelo &&
         a.garra == b.garra;
}

void mostrarPose() {
  Serial.print(F("Pose -> h: ")); Serial.print(atual.altura);
  Serial.print(F(" | Base: ")); Serial.print(atual.base);
  Serial.print(F(" | Ombro: ")); Serial.print(atual.ombro);
  Serial.print(F(" | Cotovelo: ")); Serial.print(atual.cotovelo);
  Serial.print(F(" | Garra: ")); Serial.println(atual.garra);
}

void imprimirAjuda() {
  Serial.println(F("=== INVESTIGACAO SELETIVA POR COR ==="));
  Serial.println(F("1..9 - posicionar o braco para calibracao"));
  Serial.println(F("c - registrar uma folha saudavel como referencia"));
  Serial.println(F("l - fazer uma leitura RGB sem movimentar o braco"));
  Serial.println(F("i - investigar e cortar se confirmar discrepancia"));
  Serial.println(F("t - teste completo de varredura, leitura e corte"));
  Serial.println(F("r - acionar a garra manualmente na pose atual"));
  Serial.println(F("v - testar somente a varredura lateral da base"));
  Serial.println(F("s - retornar para a posicao segura"));
  Serial.println(F("p - mostrar pose atual"));
  Serial.println(F("h - mostrar esta ajuda"));
}
