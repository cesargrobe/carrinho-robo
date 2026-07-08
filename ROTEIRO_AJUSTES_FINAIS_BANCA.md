# Roteiro de ajustes finais para a banca

## Objetivo

Finalizar o protótipo sem acrescentar novas funções. A partir deste ponto,
alterar somente constantes de calibração e corrigir falhas indispensáveis.

## 1. Preparação elétrica e segurança

- Carregar as três baterias 18650 e as quatro pilhas recarregáveis.
- Conferir polaridade, bornes, GND comum e cabos da L298N.
- Confirmar a saída do LM2596 antes de ligar o Arduino.
- Manter o jumper SEL removido na Sensor Shield.
- Conferir se as lâminas estão firmes e protegidas durante calibrações.
- Deixar o desligamento geral acessível.
- Fazer os primeiros movimentos com o braço recolhido e espaço livre.

Critério de aprovação: nenhuma reinicialização, cabo aquecido, servo tremendo ou
motor funcionando sem comando.

## 2. Preparação da simulação com plantas

- Usar a mesma iluminação, piso e suporte previstos para a apresentação.
- Posicionar uma planta por vez para a primeira calibração.
- Marcar no piso a direção original da trilha.
- Marcar distâncias de 60, 40, 25, 15 e 11,5 cm em relação ao HC-SR04.
- Manter folhas vizinhas afastadas no primeiro teste; adicioná-las depois.

## 3. Validar o radar de 15 graus

Valores atualmente calibrados:

- PWM motor A: 70;
- PWM motor B: 80;
- passo de 15 graus à esquerda: 197 ms;
- passo de 15 graus à direita: 180 ms;
- alcance máximo considerado: 80 cm.

Procedimento:

1. Colocar a planta aproximadamente a -45 graus e executar cinco buscas.
2. Repetir em 0, +45 e +75 graus.
3. Colocar duas plantas e confirmar se escolhe a mais próxima.
4. Anotar acertos, ângulo escolhido e desvio final.

Critério de aprovação: pelo menos quatro acertos em cinco tentativas nas
posições que serão usadas na demonstração.

## 4. Calibrar a aproximação reta

Valores iniciais:

- PWM de deslocamento A: 70;
- PWM de deslocamento B: 80;
- duração do pulso: 150 ms;
- distância conservadora de teste: 25 cm.

Procedimento:

1. Começar com a planta ou objeto a aproximadamente 50 cm.
2. Marcar uma linha reta no piso apontando para o alvo.
3. Executar a aproximação e observar qual roda avança menos.
4. Se desviar para a direita, aumentar em 5 o PWM da roda direita ou diminuir
   em 5 o da roda esquerda.
5. Se desviar para a esquerda, fazer o ajuste oposto.
6. Alterar apenas um valor por vez e repetir três vezes.
7. Manter o pulso em 150 ms enquanto calibra os PWMs.

Constantes no arquivo `integracao_investigacao.ino`:

```cpp
PWM_DESLOCAMENTO_MOTOR_A
PWM_DESLOCAMENTO_MOTOR_B
TEMPO_PULSO_APROXIMACAO_MS
DISTANCIA_PARADA_TESTE_CM
```

Critério de aprovação: alcançar 25 cm sem perder a planta do campo do sensor.

### Sequência atual do botão

1. Primeiro toque: radar e seleção da planta mais próxima.
2. Segundo toque: aproximação controlada até a distância configurada.
3. Terceiro toque: investigação da grade e corte apenas se houver uma região
   suspeita confirmada.
4. Quarto toque: recuo e retorno à direção da trilha.

A terceira fase exige os mapas criados por `b` e `c`. Sem a referência
saudável, a investigação permanece bloqueada e nenhum corte é executado.

## 5. Calibrar a distância real de trabalho

Somente depois da aproximação reta:

1. Trocar o objeto pela planta usada na apresentação.
2. Reduzir gradualmente `DISTANCIA_PARADA_TESTE_CM`: 25, 20, 15 e 12 cm.
3. Nunca passar diretamente de 25 para 11,5 cm.
4. Em cada distância, confirmar se o braço alcança o pecíolo sem colisão.
5. Usar 11,5 cm apenas se a leitura permanecer estável e houver margem para
   parar antes da planta.

Critério de aprovação: o sensor para repetidamente em uma distância na qual a
garra alcança o pecíolo sem tocar a planta durante o posicionamento.

## 6. Calibrar folha, máscara e pecíolo

1. Executar `b` sem folha para registrar o fundo, caso a iluminação tenha sido
   alterada. O fundo fica salvo na EEPROM.
2. Colocar a folha saudável no gabarito e executar `c`.
3. Executar `m` e verificar a máscara 6x6 e o candidato a pecíolo.
4. Repetir com folhas próximas e confirmar que a folha principal continua
   reconhecível.
5. Usar `k` para posicionar no candidato sem cortar.
6. Ajustar a pose com `A/D`, `O/P` e `C/V`.
7. Anotar a pose final antes de alterar qualquer constante do braço.

Critério de aprovação: máscara coerente e candidato ao pecíolo correto em pelo
menos quatro de cinco tentativas.

## 7. Validar o corte

1. Fazer primeiro o fechamento sem caule, mantendo pessoas afastadas.
2. Testar em material de simulação preso a um suporte.
3. Testar na planta somente depois de validar posição e distância.
4. Registrar cortes completos em dez tentativas.

Critério de aprovação recomendado: pelo menos oito cortes em dez, resultado já
atingido anteriormente pelo mecanismo.

## 8. Ensaio do ciclo completo

Executar nesta ordem:

1. carrinho alinhado à trilha e braço recolhido;
2. radar identifica e aponta para a planta;
3. aproximação para na distância calibrada;
4. calibração ou investigação da folha;
5. localização do pecíolo;
6. posicionamento e corte;
7. recolhimento seguro do braço;
8. recuo e retorno à direção da trilha.

Fazer pelo menos três ciclos completos. Filmar o melhor ciclo para documentação.

## 9. Plano da demonstração

Não depender de uma execução longa e totalmente automática na banca.

Demonstração recomendada:

1. radar aponta para a planta mais próxima;
2. aproximação curta até a marca calibrada;
3. mostrar no Serial uma máscara saudável já conhecida;
4. mostrar uma leitura com alteração de cor;
5. executar o posicionamento e o corte em material preso ao suporte.

Se a navegação completa estiver instável, apresentar as etapas separadamente e
explicar que o protótipo validou percepção, decisão e atuação. Não afirmar que
há seguimento autônomo contínuo da trilha: atualmente a busca é iniciada pelo
botão em um ponto de inspeção.

## 10. Congelamento da versão

Quando três ciclos forem satisfatórios:

- anotar todos os valores finais neste arquivo;
- não recalibrar no dia da banca sem necessidade;
- guardar uma cópia do `.ino` validado;
- levar baterias carregadas, cabo USB, notebook e material reserva;
- ensaiar a fala com cronômetro e manter a demonstração curta.
