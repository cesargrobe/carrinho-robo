// Teste isolado do botao de giro.
// Ligacao: botao entre D9 e GND. Nao precisa resistor externo.

const byte PIN_BOTAO = 9;
const unsigned long TEMPO_DEBOUNCE_MS = 40;

bool estadoEstavel = HIGH;
bool ultimaLeitura = HIGH;
unsigned long instanteMudancaMs = 0;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_BOTAO, INPUT_PULLUP);

  Serial.println(F("=== TESTE DO BOTAO D9 ==="));
  Serial.println(F("Pressione e solte o botao ligado entre D9 e GND."));
  Serial.println(F("Aguardando..."));
}

void loop() {
  bool leitura = digitalRead(PIN_BOTAO);

  if (leitura != ultimaLeitura) {
    instanteMudancaMs = millis();
    ultimaLeitura = leitura;
  }

  if (millis() - instanteMudancaMs >= TEMPO_DEBOUNCE_MS &&
      leitura != estadoEstavel) {
    estadoEstavel = leitura;

    if (estadoEstavel == LOW) {
      Serial.println(F("Botao PRESSIONADO"));
    } else {
      Serial.println(F("Botao SOLTO"));
    }
  }
}
