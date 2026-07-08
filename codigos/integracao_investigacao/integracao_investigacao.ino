#include <Servo.h>
#include <EEPROM.h>

const byte PIN_GARRA    = 4;
const byte PIN_COTOVELO = 5;
const byte PIN_OMBRO    = 6;
const byte PIN_BASE     = 7;

const byte PIN_MOTOR_IN1 = 8;
const byte PIN_MOTOR_ENA = 3;
const byte PIN_MOTOR_IN2 = 10;
const byte PIN_MOTOR_ENB = 11;
const byte PIN_MOTOR_IN3 = 12;
const byte PIN_MOTOR_IN4 = A5;

const byte PIN_ULTRASSOM_ECHO = 2;
const byte PIN_ULTRASSOM_TRIG = 13;
const byte PIN_BOTAO_GIRO = 9;

const byte PIN_COR_OUT = A0;
const byte PIN_COR_S0  = A1;
const byte PIN_COR_S1  = A2;
const byte PIN_COR_S2  = A3;
const byte PIN_COR_S3  = A4;

const byte GARRA_FECHADA = 6;
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
const unsigned int TEMPO_CORTE_RAPIDO_MS = 350;
const byte PWM_APROXIMACAO_CORTE = 80;
const unsigned int TEMPO_APROXIMACAO_CORTE_MS = 250;
const byte PWM_RE_TESTE = 80;
const unsigned int TEMPO_RE_TESTE_MS = 250;
const unsigned int TEMPO_ESTABILIZACAO_CORTE_MS = 500;

// Valores iniciais conservadores para calibrar o giro no proprio eixo.
// Ajuste TEMPO_GIRO_TESTE_MS ate o carrinho girar exatamente 90 graus.
const byte PWM_GIRO_MOTOR_A = 70;
const byte PWM_GIRO_MOTOR_B = 80;
// Calibracao inicial obtida no piso:
// 300 ms = aproximadamente 25 graus no sentido 1 (esquerda)
// 300 ms = aproximadamente 20 graus no sentido 2 (direita)
const unsigned int TEMPO_GIRO_ESQUERDA_MS = 625;
const unsigned int TEMPO_GIRO_DIREITA_MS = 905;
const unsigned int TEMPO_PASSO_RADAR_ESQUERDA_MS = 270;
const unsigned int TEMPO_PASSO_RADAR_DIREITA_MS = 289;
const unsigned int TEMPO_PASSO_FINO_ESQUERDA_MS = 197;
const unsigned int TEMPO_PASSO_FINO_DIREITA_MS = 180;
const unsigned int TEMPO_ESTABILIZACAO_GIRO_MS = 700;
const unsigned int ATRASO_BOTAO_GIRO_MS = 1000;
const byte AMOSTRAS_DISTANCIA = 5;
const unsigned long TIMEOUT_ECO_US = 30000UL;
const float DISTANCIA_MAXIMA_BUSCA_CM = 80.0;
const float DISTANCIA_PARADA_TESTE_CM = 25.0;
const byte PWM_DESLOCAMENTO_MOTOR_A = 70;
const byte PWM_DESLOCAMENTO_MOTOR_B = 80;
const unsigned int TEMPO_PULSO_APROXIMACAO_MS = 150;
const unsigned int INTERVALO_PULSO_APROXIMACAO_MS = 350;
const byte MAX_PULSOS_APROXIMACAO = 25;

const byte AMOSTRAS_COR = 7;
const byte LEITURAS_CONFIRMACAO = 3;
const byte SUSPEITAS_NECESSARIAS = 2;
const unsigned long TIMEOUT_COR_US = 30000UL;

// Diferenca maxima entre as proporcoes RGB da amostra e da referencia.
// Comece com 0.18 e ajuste observando os valores mostrados no Serial.
const float LIMIAR_DISCREPANCIA = 0.18;

// Diferenca minima entre fundo e folha saudavel para uma celula fazer
// parte da mascara da folha. O valor usa proporcoes RGB normalizadas.
const float LIMIAR_MASCARA_FOLHA = 0.015;

// Grade com margem ao redor de uma folha de ate 5 x 8 cm.
const byte BASE_VARREDURA_MIN = 80;
const byte BASE_VARREDURA_MAX = 100;
const byte PASSO_BASE = 4;
const byte TOTAL_COLUNAS =
    ((BASE_VARREDURA_MAX - BASE_VARREDURA_MIN) / PASSO_BASE) + 1;
const byte PRIMEIRO_PONTO_GRADE = 2;  // 15 cm na curva empirica.
const byte TOTAL_LINHAS_GRADE = 6;   // 15, 13, 11, 9, 7 e 5 cm.
const float RAIO_SENSOR_CM = 16.0;
const float LARGURA_PASSO_CM = 1.12;

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

struct FundoPersistente {
  unsigned long assinatura;
  byte versao;
  byte linhas;
  byte colunas;
  byte vermelho[TOTAL_LINHAS_GRADE][TOTAL_COLUNAS];
  byte verde[TOTAL_LINHAS_GRADE][TOTAL_COLUNAS];
  byte valida[TOTAL_LINHAS_GRADE][TOTAL_COLUNAS];
  unsigned int checksum;
};

const unsigned long ASSINATURA_FUNDO = 0x464F4C48UL; // "FOLH"
const byte VERSAO_FUNDO = 1;
const int ENDERECO_FUNDO_EEPROM = 0;

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

// Ponto inicial estimado acima da linha 0. Ombro e cotovelo foram
// interpolados entre as poses empiricas de 17,5 cm e 15 cm.
const Pose POSE_PECIOLO_VIRTUAL = {
  16.0, BASE_CENTRO, 109, 111, GARRA_ABERTA
};

const byte TOTAL_PONTOS = sizeof(curva) / sizeof(curva[0]);

Pose atual = POSE_SEGURA;
bool cicloEmExecucao = false;
bool fundoCalibrado = false;
bool mapaSaudavelCalibrado = false;
bool ajusteCorteAtivo = false;
bool carrinhoAproximadoParaCorte = false;
bool pecioloLocalizado = false;
bool botaoGiroAnterior = HIGH;
unsigned long ultimaAcaoBotaoMs = 0;
// Posição escolhida em passos de 15 graus: -6 (esquerda) a +6 (direita).
// Zero também indica que o carrinho está alinhado com a trilha.
int passoPlantaSelecionada = 0;
// 0 = procurar; 1 = apontado; 2 = aproximado; 3 = investigacao concluida.
byte etapaBuscaPlanta = 0;
byte pulsosAproximacaoExecutados = 0;
float colunaPeciolo = 0;
int anguloBasePeciolo = BASE_CENTRO;
byte mapaFundoR[TOTAL_LINHAS_GRADE][TOTAL_COLUNAS];
byte mapaFundoG[TOTAL_LINHAS_GRADE][TOTAL_COLUNAS];
bool mapaFundoValido[TOTAL_LINHAS_GRADE][TOTAL_COLUNAS];
byte mapaSaudavelR[TOTAL_LINHAS_GRADE][TOTAL_COLUNAS];
byte mapaSaudavelG[TOTAL_LINHAS_GRADE][TOTAL_COLUNAS];
bool mascaraFolha[TOTAL_LINHAS_GRADE][TOTAL_COLUNAS];
bool mapaSuspeito[TOTAL_LINHAS_GRADE][TOTAL_COLUNAS];
bool mapaVisitado[TOTAL_LINHAS_GRADE][TOTAL_COLUNAS];

void setup() {
  Serial.begin(9600);

  pinMode(PIN_COR_S0, OUTPUT);
  pinMode(PIN_COR_S1, OUTPUT);
  pinMode(PIN_COR_S2, OUTPUT);
  pinMode(PIN_COR_S3, OUTPUT);
  pinMode(PIN_COR_OUT, INPUT);

  pinMode(PIN_MOTOR_IN1, OUTPUT);
  pinMode(PIN_MOTOR_IN2, OUTPUT);
  pinMode(PIN_MOTOR_IN3, OUTPUT);
  pinMode(PIN_MOTOR_IN4, OUTPUT);
  pinMode(PIN_MOTOR_ENA, OUTPUT);
  pinMode(PIN_MOTOR_ENB, OUTPUT);
  pararMotores();

  pinMode(PIN_ULTRASSOM_TRIG, OUTPUT);
  pinMode(PIN_ULTRASSOM_ECHO, INPUT);
  digitalWrite(PIN_ULTRASSOM_TRIG, LOW);

  // Botao entre D9 e GND; o resistor interno mantem a entrada em HIGH.
  pinMode(PIN_BOTAO_GIRO, INPUT_PULLUP);

  // Escala de frequencia em 20% no TCS230/TCS3200.
  digitalWrite(PIN_COR_S0, HIGH);
  digitalWrite(PIN_COR_S1, LOW);

  carregarFundoEEPROM();

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
  verificarBotaoGiro();

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

    case 'b':
      calibrarFundo();
      break;

    case 'B':
      apagarFundoEEPROM();
      break;

    case 'c':
      calibrarMapaSaudavel();
      break;

    case 'l':
      mostrarLeituraCor(lerCor());
      break;

    case 'm':
      if (mapaSaudavelCalibrado) {
        mostrarMascaraFolha();
        estimarPecioloPelaMascara();
      } else {
        Serial.println(F("Mascara indisponivel: execute 'b' e depois 'c'."));
      }
      break;

    case 'k':
      posicionarNoPecioloParaAjuste();
      break;

    case 'j':
      aproximarCarrinhoParaCorte();
      break;

    case 'J':
      afastarCarrinhoEmReTeste();
      break;

    case 'z':
      pararMotores();
      Serial.println(F("Motores parados."));
      break;

    case 'u':
      medirDistanciaEMostrar();
      break;

    case 'g':
      girarCarrinhoTeste(true);
      break;

    case 'G':
      girarCarrinhoTeste(false);
      break;

    case 'A':
      ajustarPoseCorte(-1, 0, 0);
      break;

    case 'D':
      ajustarPoseCorte(1, 0, 0);
      break;

    case 'O':
      ajustarPoseCorte(0, -1, 0);
      break;

    case 'P':
      ajustarPoseCorte(0, 1, 0);
      break;

    case 'C':
      ajustarPoseCorte(0, 0, -1);
      break;

    case 'V':
      ajustarPoseCorte(0, 0, 1);
      break;

    case 'x':
      testarFechamentoNaPoseAjustada();
      break;

    case 's':
      if (ajusteCorteAtivo) {
        sairDoAjusteCorte();
      } else {
        retornarSeguro();
      }
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

  if (!mapaSaudavelCalibrado) {
    Serial.println(F("Teste bloqueado: calibre o fundo com 'b' e a folha com 'c'."));
    return;
  }

  Serial.println(F("O corte ocorrera somente se 2 de 3 leituras forem suspeitas."));
  executarInvestigacaoAutomatica();

  Serial.println(F("========================================"));
  Serial.println(F(" TESTE COMPLETO FINALIZADO"));
  Serial.println(F("========================================"));
}

void executarInvestigacaoAutomatica() {
  if (!mapaSaudavelCalibrado) {
    Serial.println(F("Investigacao bloqueada: use 'b' sem folha e depois 'c' com folha saudavel."));
    return;
  }

  cicloEmExecucao = true;
  limparMapas();

  Serial.println(F("=== VARREDURA 2D DA AREA ==="));
  retornarSeguro();

  byte colunaAtual = TOTAL_COLUNAS - 1;
  moverParaInicioGrade(colunaAtual);

  for (byte linha = 0; linha < TOTAL_LINHAS_GRADE; linha++) {
    byte indiceCurva = indiceCurvaDaLinha(linha);
    Pose destino = curva[indiceCurva];
    destino.base = anguloDaColuna(colunaAtual);
    irParaPoseSuave(destino, TEMPO_CURVA_MS);

    bool esquerdaParaDireita = (linha % 2) == 1;
    int inicio = esquerdaParaDireita ? 0 : TOTAL_COLUNAS - 1;
    int fim = esquerdaParaDireita ? TOTAL_COLUNAS : -1;
    int passo = esquerdaParaDireita ? 1 : -1;

    for (int coluna = inicio; coluna != fim; coluna += passo) {
      destino = curva[indiceCurva];
      destino.base = anguloDaColuna(coluna);
      irParaPoseSuave(destino, TEMPO_CURVA_MS);
      colunaAtual = coluna;
      delay(250);

      Serial.print(F("Celula -> altura: "));
      Serial.print(curva[indiceCurva].altura);
      Serial.print(F(" cm | base: "));
      Serial.println(destino.base);

      if (mascaraFolha[linha][coluna]) {
        mapaSuspeito[linha][coluna] = confirmarDiscrepancia(linha, coluna);
      } else {
        mapaSuspeito[linha][coluna] = false;
        Serial.println(F("Celula fora da mascara da folha."));
      }
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
    recolherDaLinha(TOTAL_LINHAS_GRADE - 1);
  }

  Serial.println(F("=== FIM DA INVESTIGACAO ==="));

  cicloEmExecucao = false;
}

byte anguloDaColuna(byte coluna) {
  return BASE_VARREDURA_MIN + coluna * PASSO_BASE;
}

byte indiceCurvaDaLinha(byte linha) {
  return PRIMEIRO_PONTO_GRADE + linha;
}

void moverParaInicioGrade(byte coluna) {
  Pose destino = curva[0];
  destino.base = anguloDaColuna(coluna);
  irParaPoseSuave(destino, TEMPO_NORMAL_MS);

  // Entra na grade passando pelos pontos validados, sem cortar caminho.
  for (byte indice = 1; indice <= PRIMEIRO_PONTO_GRADE; indice++) {
    destino = curva[indice];
    destino.base = anguloDaColuna(coluna);
    irParaPoseSuave(destino, TEMPO_CURVA_MS);
    delay(250);
  }
}

void limparMapas() {
  for (byte linha = 0; linha < TOTAL_LINHAS_GRADE; linha++) {
    for (byte coluna = 0; coluna < TOTAL_COLUNAS; coluna++) {
      mapaSuspeito[linha][coluna] = false;
      mapaVisitado[linha][coluna] = false;
    }
  }
}

RegiaoSuspeita localizarMaiorRegiao() {
  RegiaoSuspeita maior = {false, 0, 0, 0, 0, 0, 0, 0};
  byte filaLinha[TOTAL_LINHAS_GRADE * TOTAL_COLUNAS];
  byte filaColuna[TOTAL_LINHAS_GRADE * TOTAL_COLUNAS];

  for (byte linha = 0; linha < TOTAL_LINHAS_GRADE; linha++) {
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

            if (vizinhaLinha < 0 || vizinhaLinha >= TOTAL_LINHAS_GRADE ||
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
  float alturaCm = curva[indiceCurvaDaLinha(regiao.linhaMin)].altura -
                   curva[indiceCurvaDaLinha(regiao.linhaMax)].altura;

  Serial.println(F("=== MAIOR AREA SUSPEITA ==="));
  Serial.print(F("Celulas: ")); Serial.println(regiao.quantidade);
  Serial.print(F("Largura aproximada: ")); Serial.print(larguraCm, 1);
  Serial.println(F(" cm"));
  Serial.print(F("Altura aproximada: ")); Serial.print(alturaCm, 1);
  Serial.println(F(" cm"));
  Serial.print(F("Centro -> altura: "));
  Serial.print(curva[indiceCurvaDaLinha(linhaCentro)].altura);
  Serial.print(F(" cm | base: ")); Serial.println(anguloDaColuna(colunaCentro));
}

void moverDaUltimaLinhaPara(byte linhaDestino, byte colunaDestino) {
  for (int linha = TOTAL_LINHAS_GRADE - 2; linha >= linhaDestino; linha--) {
    Pose destino = curva[indiceCurvaDaLinha(linha)];
    destino.base = atual.base;
    irParaPoseSuave(destino, TEMPO_CURVA_MS);
    delay(300);
  }

  Pose centro = curva[indiceCurvaDaLinha(linhaDestino)];
  centro.base = anguloDaColuna(colunaDestino);
  irParaPoseSuave(centro, TEMPO_CURVA_MS);
}

void recolherDaLinha(byte linhaAtual) {
  int indiceAtual = indiceCurvaDaLinha(linhaAtual);

  for (int indice = indiceAtual - 1; indice >= 0; indice--) {
    Pose destino = curva[indice];
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

void posicionarNoPecioloParaAjuste() {
  if (!mapaSaudavelCalibrado) {
    Serial.println(F("Corte bloqueado: execute 'b' e depois 'c'."));
    return;
  }

  if (!pecioloLocalizado) {
    Serial.println(F("Corte bloqueado: nenhum candidato a peciolo foi localizado."));
    return;
  }

  cicloEmExecucao = true;
  ajusteCorteAtivo = false;

  if (!carrinhoAproximadoParaCorte) {
    aproximarCarrinhoParaCorte();
  }

  Serial.print(F("Indo ao peciolo -> altura: "));
  Serial.print(POSE_PECIOLO_VIRTUAL.altura);
  Serial.print(F(" cm | base: "));
  Serial.println(anguloBasePeciolo);

  retornarSeguro();
  moverParaPecioloVirtual(anguloBasePeciolo);
  cicloEmExecucao = false;
  ajusteCorteAtivo = true;

  Serial.println(F("Posicao candidata alcancada; nenhum corte foi executado."));
  Serial.println(F("A/D base | O/P ombro | C/V cotovelo | x testar fechamento"));
  mostrarPose();
}

void aproximarCarrinhoParaCorte() {
  if (cicloEmExecucao && carrinhoAproximadoParaCorte) return;

  Serial.println(F("Aproximando o carrinho para permitir o alcance do corte..."));

  // Motor A no sentido de avanço.
  digitalWrite(PIN_MOTOR_IN1, LOW);
  digitalWrite(PIN_MOTOR_IN2, HIGH);

  // Motor B invertido porque esta montado espelhado em relacao ao A.
  digitalWrite(PIN_MOTOR_IN3, HIGH);
  digitalWrite(PIN_MOTOR_IN4, LOW);

  analogWrite(PIN_MOTOR_ENA, PWM_APROXIMACAO_CORTE);
  analogWrite(PIN_MOTOR_ENB, PWM_APROXIMACAO_CORTE);
  delay(TEMPO_APROXIMACAO_CORTE_MS);
  pararMotores();
  delay(TEMPO_ESTABILIZACAO_CORTE_MS);

  carrinhoAproximadoParaCorte = true;
  Serial.println(F("Pulso de aproximacao concluido; motores parados."));
}

void afastarCarrinhoEmReTeste() {
  Serial.println(F("Executando pulso manual de re..."));

  // Sentidos exatamente opostos aos valores de avanco validados acima.
  digitalWrite(PIN_MOTOR_IN1, HIGH);
  digitalWrite(PIN_MOTOR_IN2, LOW);
  digitalWrite(PIN_MOTOR_IN3, LOW);
  digitalWrite(PIN_MOTOR_IN4, HIGH);

  analogWrite(PIN_MOTOR_ENA, PWM_RE_TESTE);
  analogWrite(PIN_MOTOR_ENB, PWM_RE_TESTE);
  delay(TEMPO_RE_TESTE_MS);
  pararMotores();
  delay(TEMPO_ESTABILIZACAO_CORTE_MS);

  // A re nunca e chamada automaticamente. Este estado permite que um novo
  // comando 'k' execute novamente a aproximacao, caso seja necessario.
  carrinhoAproximadoParaCorte = false;
  Serial.println(F("Pulso manual de re concluido; motores parados."));
}

void pararMotores() {
  analogWrite(PIN_MOTOR_ENA, 0);
  analogWrite(PIN_MOTOR_ENB, 0);
  digitalWrite(PIN_MOTOR_IN1, LOW);
  digitalWrite(PIN_MOTOR_IN2, LOW);
  digitalWrite(PIN_MOTOR_IN3, LOW);
  digitalWrite(PIN_MOTOR_IN4, LOW);
}

void girarCarrinhoTeste(bool sentido1) {
  if (!posesIguais(atual, POSE_SEGURA)) {
    Serial.println(F("Giro bloqueado: retorne o braco com 's'."));
    return;
  }

  Serial.println(sentido1 ? F("Giro de teste: SENTIDO 1")
                           : F("Giro de teste: SENTIDO 2"));

  girarCarrinhoPorTempo(
      sentido1,
      sentido1 ? TEMPO_GIRO_ESQUERDA_MS : TEMPO_GIRO_DIREITA_MS);

  Serial.println(F("Giro concluido. Use 'u' para medir nessa direcao."));
}

void girarCarrinhoPorTempo(bool esquerda, unsigned int tempoMs) {
  if (!posesIguais(atual, POSE_SEGURA)) return;

  // Um motor avanca e o outro recua para girar no proprio eixo.
  digitalWrite(PIN_MOTOR_IN1, esquerda ? LOW : HIGH);
  digitalWrite(PIN_MOTOR_IN2, esquerda ? HIGH : LOW);
  digitalWrite(PIN_MOTOR_IN3, esquerda ? LOW : HIGH);
  digitalWrite(PIN_MOTOR_IN4, esquerda ? HIGH : LOW);

  analogWrite(PIN_MOTOR_ENA, PWM_GIRO_MOTOR_A);
  analogWrite(PIN_MOTOR_ENB, PWM_GIRO_MOTOR_B);
  delay(tempoMs);
  pararMotores();
  delay(TEMPO_ESTABILIZACAO_GIRO_MS);
}

void verificarBotaoGiro() {
  bool estadoAtual = digitalRead(PIN_BOTAO_GIRO);

  // Detecta somente o instante em que o botao e pressionado.
  if (botaoGiroAnterior == HIGH && estadoAtual == LOW &&
      millis() - ultimaAcaoBotaoMs >= 250) {
    ultimaAcaoBotaoMs = millis();
    Serial.println(F("Botao acionado: operacao inicia em 1 segundo..."));
    delay(ATRASO_BOTAO_GIRO_MS);

    if (etapaBuscaPlanta == 0) {
      buscarPlantaMaisProxima();
    } else if (etapaBuscaPlanta == 1) {
      aproximarDoObjetoSelecionado();
    } else if (etapaBuscaPlanta == 2) {
      investigarECortarObjetoSelecionado();
    } else {
      recuarAproximacaoERetornarTrilha();
    }
  }

  botaoGiroAnterior = estadoAtual;
}

void buscarPlantaMaisProxima() {
  if (!posesIguais(atual, POSE_SEGURA)) {
    Serial.println(F("Busca bloqueada: retorne o braco com 's'."));
    return;
  }

  cicloEmExecucao = true;
  Serial.println(F("=== VARREDURA RADAR DE PLANTA ==="));

  const byte TOTAL_SETORES_RADAR = 13;
  float distancias[TOTAL_SETORES_RADAR];
  int melhorIndice = -1;
  float menorDistancia = DISTANCIA_MAXIMA_BUSCA_CM + 1.0;

  // Sai de 0 grau e chega a -90 usando os mesmos passos finos do radar.
  // Isso reduz a diferença causada por inércia e derrapagem em um giro longo.
  for (byte passo = 0; passo < 6; passo++) {
    girarCarrinhoPorTempo(true, TEMPO_PASSO_FINO_ESQUERDA_MS);
  }

  for (byte indice = 0; indice < TOTAL_SETORES_RADAR; indice++) {
    int angulo = -90 + indice * 15;
    distancias[indice] = medirDistanciaFiltrada();
    mostrarDistanciaAngulo(angulo, distancias[indice]);

    if (distancias[indice] >= 2.0 &&
        distancias[indice] <= DISTANCIA_MAXIMA_BUSCA_CM &&
        distancias[indice] < menorDistancia) {
      menorDistancia = distancias[indice];
      melhorIndice = indice;
    }

    if (indice < TOTAL_SETORES_RADAR - 1) {
      girarCarrinhoPorTempo(false, TEMPO_PASSO_FINO_DIREITA_MS);
    }
  }

  // Ao terminar, o carrinho está em +90 graus (passo +6).
  int passoEscolhido = melhorIndice < 0 ? 0 : melhorIndice - 6;
  int passosParaVoltar = 6 - passoEscolhido;

  for (int passo = 0; passo < passosParaVoltar; passo++) {
    girarCarrinhoPorTempo(true, TEMPO_PASSO_FINO_ESQUERDA_MS);
  }

  if (melhorIndice < 0) {
    Serial.println(F("Nenhuma planta encontrada no alcance configurado."));
    passoPlantaSelecionada = 0;
    etapaBuscaPlanta = 0;
  } else {
    passoPlantaSelecionada = passoEscolhido;
    etapaBuscaPlanta = 1;
    Serial.print(F("Mais proxima -> angulo: "));
    Serial.print(passoEscolhido * 15);
    Serial.print(F(" graus | distancia: "));
    Serial.print(menorDistancia, 1);
    Serial.println(F(" cm"));
  }

  cicloEmExecucao = false;
  Serial.println(F("Busca concluida."));
}

void aproximarDoObjetoSelecionado() {
  if (!posesIguais(atual, POSE_SEGURA)) {
    Serial.println(F("Aproximacao bloqueada: retorne o braco com 's'."));
    return;
  }

  cicloEmExecucao = true;
  pulsosAproximacaoExecutados = 0;
  float distancia = medirDistanciaFiltrada();

  Serial.println(F("=== APROXIMACAO CONTROLADA ==="));

  if (distancia < 2.0 || distancia > DISTANCIA_MAXIMA_BUSCA_CM) {
    Serial.println(F("Aproximacao cancelada: objeto sem leitura valida."));
    cicloEmExecucao = false;
    return;
  }

  while (distancia > DISTANCIA_PARADA_TESTE_CM &&
         pulsosAproximacaoExecutados < MAX_PULSOS_APROXIMACAO) {
    executarPulsoDeslocamento(true, TEMPO_PULSO_APROXIMACAO_MS);
    pulsosAproximacaoExecutados++;
    delay(INTERVALO_PULSO_APROXIMACAO_MS);

    distancia = medirDistanciaFiltrada();
    Serial.print(F("Pulso "));
    Serial.print(pulsosAproximacaoExecutados);
    Serial.print(F(" | distancia: "));

    if (distancia < 0) {
      Serial.println(F("sem eco; aproximacao interrompida"));
      break;
    }

    Serial.print(distancia, 1);
    Serial.println(F(" cm"));
  }

  pararMotores();
  etapaBuscaPlanta = 2;
  cicloEmExecucao = false;

  if (distancia >= 2.0 && distancia <= DISTANCIA_PARADA_TESTE_CM) {
    Serial.println(F("Distancia de teste alcancada."));
  } else {
    Serial.println(F("Aproximacao encerrada pelo limite de seguranca."));
  }

  Serial.println(F("Novo toque: investigar a folha e decidir o corte."));
}

void investigarECortarObjetoSelecionado() {
  if (!mapaSaudavelCalibrado) {
    Serial.println(F("Investigacao bloqueada: calibre com 'b' e 'c'."));
    Serial.println(F("A etapa permanece aguardando a calibracao."));
    return;
  }

  Serial.println(F("=== LEITURA E DECISAO DE COR ==="));
  executarInvestigacaoAutomatica();

  // A investigacao recolhe o braco. Se houver regiao suspeita confirmada,
  // a rotina executa o corte; caso contrario, encerra sem fechar a garra.
  etapaBuscaPlanta = 3;
  Serial.println(F("Novo toque: recuar e retornar a trilha."));
}

void executarPulsoDeslocamento(bool frente, unsigned int tempoMs) {
  digitalWrite(PIN_MOTOR_IN1, frente ? LOW : HIGH);
  digitalWrite(PIN_MOTOR_IN2, frente ? HIGH : LOW);
  digitalWrite(PIN_MOTOR_IN3, frente ? HIGH : LOW);
  digitalWrite(PIN_MOTOR_IN4, frente ? LOW : HIGH);

  analogWrite(PIN_MOTOR_ENA, PWM_DESLOCAMENTO_MOTOR_A);
  analogWrite(PIN_MOTOR_ENB, PWM_DESLOCAMENTO_MOTOR_B);
  delay(tempoMs);
  pararMotores();
}

void recuarAproximacaoERetornarTrilha() {
  if (!posesIguais(atual, POSE_SEGURA)) {
    Serial.println(F("Recuo bloqueado: retorne o braco com 's'."));
    return;
  }

  cicloEmExecucao = true;
  Serial.println(F("Recuando os pulsos usados na aproximacao..."));

  for (byte pulso = 0; pulso < pulsosAproximacaoExecutados; pulso++) {
    executarPulsoDeslocamento(false, TEMPO_PULSO_APROXIMACAO_MS);
    delay(100);
  }

  pulsosAproximacaoExecutados = 0;
  cicloEmExecucao = false;
  retornarDirecaoDaTrilha();
}

void retornarDirecaoDaTrilha() {
  if (!posesIguais(atual, POSE_SEGURA)) {
    Serial.println(F("Retorno bloqueado: retorne o braco com 's'."));
    return;
  }

  cicloEmExecucao = true;
  Serial.println(F("Retornando à direção da trilha..."));

  if (passoPlantaSelecionada < 0) {
    for (int passo = 0; passo < -passoPlantaSelecionada; passo++) {
      girarCarrinhoPorTempo(false, TEMPO_PASSO_FINO_DIREITA_MS);
    }
  } else if (passoPlantaSelecionada > 0) {
    for (int passo = 0; passo < passoPlantaSelecionada; passo++) {
      girarCarrinhoPorTempo(true, TEMPO_PASSO_FINO_ESQUERDA_MS);
    }
  }

  passoPlantaSelecionada = 0;
  etapaBuscaPlanta = 0;
  cicloEmExecucao = false;
  Serial.println(F("Carrinho alinhado novamente com a trilha."));
}

void mostrarDistanciaLado(const __FlashStringHelper *lado, float distancia) {
  Serial.print(lado);
  Serial.print(F(": "));

  if (distancia < 0) {
    Serial.println(F("sem eco valido"));
  } else {
    Serial.print(distancia, 1);
    Serial.println(F(" cm"));
  }
}

void mostrarDistanciaAngulo(int angulo, float distancia) {
  Serial.print(F("Angulo "));
  if (angulo > 0) Serial.print('+');
  Serial.print(angulo);
  Serial.print(F(" graus: "));

  if (distancia < 0) {
    Serial.println(F("sem eco valido"));
  } else {
    Serial.print(distancia, 1);
    Serial.println(F(" cm"));
  }
}

void medirDistanciaEMostrar() {
  float distancia = medirDistanciaFiltrada();

  if (distancia < 0) {
    Serial.println(F("Distancia: sem eco valido."));
    return;
  }

  Serial.print(F("Distancia ultrassonica: "));
  Serial.print(distancia, 1);
  Serial.println(F(" cm"));
}

float medirDistanciaFiltrada() {
  float amostras[AMOSTRAS_DISTANCIA];
  byte validas = 0;

  for (byte i = 0; i < AMOSTRAS_DISTANCIA; i++) {
    digitalWrite(PIN_ULTRASSOM_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_ULTRASSOM_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_ULTRASSOM_TRIG, LOW);

    unsigned long duracao = pulseIn(PIN_ULTRASSOM_ECHO,
                                    HIGH,
                                    TIMEOUT_ECO_US);
    if (duracao > 0) {
      float distancia = duracao * 0.0343 / 2.0;
      if (distancia >= 2.0 && distancia <= 400.0) {
        amostras[validas++] = distancia;
      }
    }
    delay(25);
  }

  if (validas == 0) return -1;

  // Ordenacao simples; a mediana elimina ecos isolados.
  for (byte i = 0; i < validas; i++) {
    for (byte j = i + 1; j < validas; j++) {
      if (amostras[j] < amostras[i]) {
        float temporario = amostras[i];
        amostras[i] = amostras[j];
        amostras[j] = temporario;
      }
    }
  }

  return amostras[validas / 2];
}

void ajustarPoseCorte(int ajusteBase,
                      int ajusteOmbro,
                      int ajusteCotovelo) {
  if (!ajusteCorteAtivo) {
    Serial.println(F("Ajuste bloqueado: use 'k' primeiro."));
    return;
  }

  Pose destino = atual;
  destino.base = constrain(destino.base + ajusteBase,
                           BASE_DIREITA, BASE_ESQUERDA);
  destino.ombro = constrain(destino.ombro + ajusteOmbro,
                            OMBRO_MIN, OMBRO_MAX);
  destino.cotovelo = constrain(destino.cotovelo + ajusteCotovelo,
                               COTOVELO_MIN, COTOVELO_MAX);

  irParaPoseSuave(destino, TEMPO_CURVA_MS);
  mostrarPose();
}

void testarFechamentoNaPoseAjustada() {
  if (!ajusteCorteAtivo) {
    Serial.println(F("Teste bloqueado: use 'k' primeiro."));
    return;
  }

  Serial.println(F("ATENCAO: executando fechamento rapido na pose ajustada."));
  garra.write(GARRA_FECHADA);
  atual.garra = GARRA_FECHADA;
  delay(TEMPO_CORTE_RAPIDO_MS);

  garra.write(GARRA_ABERTA);
  atual.garra = GARRA_ABERTA;
  delay(TEMPO_CORTE_RAPIDO_MS);

  mostrarPose();
}

void sairDoAjusteCorte() {
  Serial.println(F("Saindo do ajuste pela trajetoria validada..."));

  Pose nominal = POSE_PECIOLO_VIRTUAL;
  nominal.base = anguloBasePeciolo;
  nominal.garra = GARRA_ABERTA;
  irParaPoseSuave(nominal, TEMPO_CURVA_MS);

  ajusteCorteAtivo = false;

  Pose destino = curva[1];
  destino.base = nominal.base;
  irParaPoseSuave(destino, TEMPO_CURVA_MS);

  destino = curva[0];
  destino.base = nominal.base;
  irParaPoseSuave(destino, TEMPO_CURVA_MS);

  destino.base = BASE_CENTRO;
  irParaPoseSuave(destino, TEMPO_NORMAL_MS);
  irParaPoseSuave(POSE_SEGURA, TEMPO_NORMAL_MS);
}

void moverParaPecioloVirtual(int anguloBase) {
  Pose destino = curva[0];
  destino.base = BASE_CENTRO;
  irParaPoseSuave(destino, TEMPO_NORMAL_MS);

  destino = curva[1];
  destino.base = BASE_CENTRO;
  irParaPoseSuave(destino, TEMPO_CURVA_MS);
  delay(250);

  destino = POSE_PECIOLO_VIRTUAL;
  destino.base = BASE_CENTRO;
  irParaPoseSuave(destino, TEMPO_CURVA_MS);

  destino.base = anguloBase;
  irParaPoseSuave(destino, TEMPO_CURVA_MS);
}

void calibrarFundo() {
  Serial.println(F("=== CALIBRACAO DO FUNDO ==="));
  Serial.println(F("Retire a folha e mantenha suporte e iluminacao parados."));
  fundoCalibrado = false;
  mapaSaudavelCalibrado = false;
  executarVarreduraCalibracao(true);
}

void calibrarMapaSaudavel() {
  if (!fundoCalibrado) {
    Serial.println(F("Calibracao bloqueada: primeiro retire a folha e envie 'b'."));
    return;
  }

  Serial.println(F("=== CALIBRACAO DO MAPA SAUDAVEL ==="));
  Serial.println(F("Coloque a folha saudavel no gabarito e nao altere a iluminacao."));
  mapaSaudavelCalibrado = false;
  executarVarreduraCalibracao(false);
}

void executarVarreduraCalibracao(bool registrarFundo) {
  cicloEmExecucao = true;
  retornarSeguro();

  byte colunaAtual = TOTAL_COLUNAS - 1;
  byte leiturasValidas = 0;
  byte celulasNaMascara = 0;

  moverParaInicioGrade(colunaAtual);

  for (byte linha = 0; linha < TOTAL_LINHAS_GRADE; linha++) {
    byte indiceCurva = indiceCurvaDaLinha(linha);
    Pose destino = curva[indiceCurva];
    destino.base = anguloDaColuna(colunaAtual);
    irParaPoseSuave(destino, TEMPO_CURVA_MS);

    bool esquerdaParaDireita = (linha % 2) == 1;
    int inicio = esquerdaParaDireita ? 0 : TOTAL_COLUNAS - 1;
    int fim = esquerdaParaDireita ? TOTAL_COLUNAS : -1;
    int passo = esquerdaParaDireita ? 1 : -1;

    for (int coluna = inicio; coluna != fim; coluna += passo) {
      destino = curva[indiceCurva];
      destino.base = anguloDaColuna(coluna);
      irParaPoseSuave(destino, TEMPO_CURVA_MS);
      colunaAtual = coluna;
      delay(250);

      byte vermelho;
      byte verde;
      bool valida = lerCorNormalizadaMedia(vermelho, verde);

      Serial.print(F("Mapa ["));
      Serial.print(linha);
      Serial.print(F("]["));
      Serial.print(coluna);
      Serial.print(F("] -> "));

      if (!valida) {
        Serial.println(F("leitura invalida"));
        if (registrarFundo) {
          mapaFundoValido[linha][coluna] = false;
        } else {
          mascaraFolha[linha][coluna] = false;
        }
        continue;
      }

      leiturasValidas++;
      Serial.print(F("R: "));
      Serial.print(vermelho);
      Serial.print(F(" | G: "));
      Serial.println(verde);

      if (registrarFundo) {
        mapaFundoR[linha][coluna] = vermelho;
        mapaFundoG[linha][coluna] = verde;
        mapaFundoValido[linha][coluna] = true;
      } else {
        mapaSaudavelR[linha][coluna] = vermelho;
        mapaSaudavelG[linha][coluna] = verde;

        float diferencaFundo = diferencaNormalizada(
            vermelho, verde,
            mapaFundoR[linha][coluna], mapaFundoG[linha][coluna]);

        mascaraFolha[linha][coluna] =
            mapaFundoValido[linha][coluna] &&
            diferencaFundo >= LIMIAR_MASCARA_FOLHA;

        Serial.print(F("Diferenca do fundo: "));
        Serial.print(diferencaFundo, 3);
        Serial.print(F(" | Classificacao: "));
        Serial.println(mascaraFolha[linha][coluna]
                           ? F("FOLHA")
                           : F("FUNDO"));

        if (mascaraFolha[linha][coluna]) celulasNaMascara++;
      }
    }
  }

  recolherDaLinha(TOTAL_LINHAS_GRADE - 1);

  byte minimoLeituras = (TOTAL_LINHAS_GRADE * TOTAL_COLUNAS) / 2;
  if (leiturasValidas < minimoLeituras) {
    Serial.println(F("Calibracao rejeitada: poucas leituras validas."));
  } else if (registrarFundo) {
    fundoCalibrado = true;
    salvarFundoEEPROM();
    Serial.println(F("Mapa do fundo registrado."));
  } else if (celulasNaMascara == 0) {
    Serial.println(F("Calibracao rejeitada: nenhuma celula de folha encontrada."));
  } else {
    mapaSaudavelCalibrado = true;
    Serial.print(F("Mapa saudavel registrado. Celulas da folha: "));
    Serial.print(celulasNaMascara);
    Serial.print(F("/"));
    Serial.println(TOTAL_LINHAS_GRADE * TOTAL_COLUNAS);
    mostrarMascaraFolha();
    estimarPecioloPelaMascara();
  }

  cicloEmExecucao = false;
}

unsigned int calcularChecksumFundo(const FundoPersistente &dados) {
  unsigned int checksum = dados.versao + dados.linhas + dados.colunas;

  for (byte linha = 0; linha < TOTAL_LINHAS_GRADE; linha++) {
    for (byte coluna = 0; coluna < TOTAL_COLUNAS; coluna++) {
      checksum = checksum * 31U + dados.vermelho[linha][coluna];
      checksum = checksum * 31U + dados.verde[linha][coluna];
      checksum = checksum * 31U + dados.valida[linha][coluna];
    }
  }

  return checksum;
}

void salvarFundoEEPROM() {
  FundoPersistente dados;
  dados.assinatura = ASSINATURA_FUNDO;
  dados.versao = VERSAO_FUNDO;
  dados.linhas = TOTAL_LINHAS_GRADE;
  dados.colunas = TOTAL_COLUNAS;

  for (byte linha = 0; linha < TOTAL_LINHAS_GRADE; linha++) {
    for (byte coluna = 0; coluna < TOTAL_COLUNAS; coluna++) {
      dados.vermelho[linha][coluna] = mapaFundoR[linha][coluna];
      dados.verde[linha][coluna] = mapaFundoG[linha][coluna];
      dados.valida[linha][coluna] = mapaFundoValido[linha][coluna] ? 1 : 0;
    }
  }

  dados.checksum = calcularChecksumFundo(dados);
  EEPROM.put(ENDERECO_FUNDO_EEPROM, dados);
  Serial.println(F("Fundo salvo na EEPROM e preservado apos recompilacao."));
}

void carregarFundoEEPROM() {
  FundoPersistente dados;
  EEPROM.get(ENDERECO_FUNDO_EEPROM, dados);

  bool estruturaValida =
      dados.assinatura == ASSINATURA_FUNDO &&
      dados.versao == VERSAO_FUNDO &&
      dados.linhas == TOTAL_LINHAS_GRADE &&
      dados.colunas == TOTAL_COLUNAS &&
      dados.checksum == calcularChecksumFundo(dados);

  if (!estruturaValida) {
    fundoCalibrado = false;
    Serial.println(F("Nenhum fundo compativel encontrado na EEPROM."));
    return;
  }

  for (byte linha = 0; linha < TOTAL_LINHAS_GRADE; linha++) {
    for (byte coluna = 0; coluna < TOTAL_COLUNAS; coluna++) {
      mapaFundoR[linha][coluna] = dados.vermelho[linha][coluna];
      mapaFundoG[linha][coluna] = dados.verde[linha][coluna];
      mapaFundoValido[linha][coluna] = dados.valida[linha][coluna] != 0;
    }
  }

  fundoCalibrado = true;
  Serial.println(F("Mapa do fundo carregado automaticamente da EEPROM."));
}

void apagarFundoEEPROM() {
  unsigned long assinaturaVazia = 0;
  EEPROM.put(ENDERECO_FUNDO_EEPROM, assinaturaVazia);
  fundoCalibrado = false;
  mapaSaudavelCalibrado = false;
  Serial.println(F("Fundo persistente apagado. Execute 'b' novamente."));
}

void mostrarMascaraFolha() {
  Serial.print(F("Mascara "));
  Serial.print(TOTAL_LINHAS_GRADE);
  Serial.print(F("x"));
  Serial.print(TOTAL_COLUNAS);
  Serial.println(F(" (# = folha, . = fundo):"));

  for (byte linha = 0; linha < TOTAL_LINHAS_GRADE; linha++) {
    for (byte coluna = 0; coluna < TOTAL_COLUNAS; coluna++) {
      Serial.print(mascaraFolha[linha][coluna] ? '#' : '.');
    }
    Serial.println();
  }

  Serial.print(F("Limiar utilizado: "));
  Serial.println(LIMIAR_MASCARA_FOLHA, 3);
}

void estimarPecioloPelaMascara() {
  pecioloLocalizado = false;

  // O peciolo pode ser fino demais para aparecer na mascara. Localizamos a
  // primeira linha ocupada da lamina, calculamos seu centro horizontal e
  // extrapolamos a posicao para cima da grade.
  int primeiraLinha = -1;
  byte quantidade = 0;
  unsigned int somaColunas = 0;

  for (byte linha = 0; linha < TOTAL_LINHAS_GRADE; linha++) {
    quantidade = 0;
    somaColunas = 0;

    for (byte coluna = 0; coluna < TOTAL_COLUNAS; coluna++) {
      if (mascaraFolha[linha][coluna]) {
        quantidade++;
        somaColunas += coluna;
      }
    }

    if (quantidade > 0) {
      primeiraLinha = linha;
      break;
    }
  }

  if (primeiraLinha < 0) {
    Serial.println(F("Peciolo nao estimado: nenhuma celula de folha foi encontrada."));
    return;
  }

  colunaPeciolo = (float)somaColunas / quantidade;
  anguloBasePeciolo = BASE_VARREDURA_MIN +
                      (int)(colunaPeciolo * PASSO_BASE + 0.5);
  anguloBasePeciolo = constrain(anguloBasePeciolo,
                                BASE_DIREITA, BASE_ESQUERDA);
  pecioloLocalizado = true;

  Serial.print(F("Primeira linha ocupada: "));
  Serial.print(primeiraLinha);
  Serial.print(F(" | borda superior: "));
  Serial.print(quantidade);
  Serial.println(F(" celula(s)."));
  if (quantidade > 2) {
    Serial.println(F("Aviso: borda larga; a coluna do peciolo e uma estimativa."));
  }

  Serial.print(F("Candidato a peciolo -> acima da linha 0 | altura inicial: "));
  Serial.print(POSE_PECIOLO_VIRTUAL.altura);
  Serial.print(F(" cm | coluna media: "));
  Serial.print(colunaPeciolo, 1);
  Serial.print(F(" | base: "));
  Serial.println(anguloBasePeciolo);
}

bool confirmarDiscrepancia(byte linha, byte coluna) {
  byte suspeitas = 0;

  for (byte i = 0; i < LEITURAS_CONFIRMACAO; i++) {
    LeituraCor leitura = lerCor();
    mostrarLeituraCor(leitura);

    if (leitura.valida &&
        corDiferenteDaReferencia(leitura, linha, coluna)) {
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

bool corDiferenteDaReferencia(LeituraCor leitura,
                              byte linha,
                              byte coluna) {
  if (!leitura.valida || !mapaSaudavelCalibrado ||
      !mascaraFolha[linha][coluna]) return false;

  byte vermelhoAtual = normalizarCanal(leitura.vermelho, leitura.total);
  byte verdeAtual = normalizarCanal(leitura.verde, leitura.total);

  float maiorDiferenca = diferencaNormalizada(
      vermelhoAtual, verdeAtual,
      mapaSaudavelR[linha][coluna], mapaSaudavelG[linha][coluna]);

  Serial.print(F("Discrepancia: "));
  Serial.println(maiorDiferenca, 3);

  return maiorDiferenca >= LIMIAR_DISCREPANCIA;
}

bool lerCorNormalizadaMedia(byte &vermelho, byte &verde) {
  unsigned int somaVermelho = 0;
  unsigned int somaVerde = 0;
  byte validas = 0;

  for (byte i = 0; i < LEITURAS_CONFIRMACAO; i++) {
    LeituraCor leitura = lerCor();
    if (leitura.valida) {
      somaVermelho += normalizarCanal(leitura.vermelho, leitura.total);
      somaVerde += normalizarCanal(leitura.verde, leitura.total);
      validas++;
    }
    delay(100);
  }

  if (validas < SUSPEITAS_NECESSARIAS) return false;

  vermelho = somaVermelho / validas;
  verde = somaVerde / validas;
  return true;
}

byte normalizarCanal(float canal, float total) {
  if (total <= 0) return 0;
  return constrain((int)(255.0 * canal / total + 0.5), 0, 255);
}

float diferencaNormalizada(byte vermelhoA, byte verdeA,
                            byte vermelhoB, byte verdeB) {
  int azulA = constrain(255 - vermelhoA - verdeA, 0, 255);
  int azulB = constrain(255 - vermelhoB - verdeB, 0, 255);

  int diferencaVermelho = abs((int)vermelhoA - vermelhoB);
  int diferencaVerde = abs((int)verdeA - verdeB);
  int diferencaAzul = abs(azulA - azulB);
  int maiorDiferenca = max(diferencaVermelho,
                           max(diferencaVerde, diferencaAzul));

  return maiorDiferenca / 255.0;
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
  Serial.println(F("Para calibrar: use 'b' sem folha e depois 'c' com a folha no gabarito."));
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
  ajusteCorteAtivo = false;
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
  Serial.println(F("b - varrer e registrar o fundo sem folha"));
  Serial.println(F("B - apagar o fundo salvo na EEPROM"));
  Serial.println(F("c - varrer a folha saudavel e criar a mascara"));
  Serial.println(F("l - fazer uma leitura RGB sem movimentar o braco"));
  Serial.println(F("m - mostrar a mascara 6x6 e o candidato a peciolo"));
  Serial.println(F("k - posicionar no peciolo para ajuste, sem cortar"));
  Serial.println(F("j - executar somente um pulso de aproximacao do carrinho"));
  Serial.println(F("J - executar somente um pulso manual de re"));
  Serial.println(F("z - garantir parada dos motores"));
  Serial.println(F("u - medir distancia na direcao atual"));
  Serial.println(F("g/G - giro curto nos sentidos 1/2 para calibracao"));
  Serial.println(F("botao D9-GND - radar; aproximar; investigar; retornar"));
  Serial.println(F("A/D - ajustar base em -1/+1 grau"));
  Serial.println(F("O/P - ajustar ombro em -1/+1 grau"));
  Serial.println(F("C/V - ajustar cotovelo em -1/+1 grau"));
  Serial.println(F("x - testar fechamento rapido na pose ajustada"));
  Serial.println(F("i - investigar e cortar se confirmar discrepancia"));
  Serial.println(F("t - teste completo de varredura, leitura e corte"));
  Serial.println(F("r - acionar a garra manualmente na pose atual"));
  Serial.println(F("v - testar somente a varredura lateral da base"));
  Serial.println(F("s - retornar para a posicao segura"));
  Serial.println(F("p - mostrar pose atual"));
  Serial.println(F("h - mostrar esta ajuda"));
}
