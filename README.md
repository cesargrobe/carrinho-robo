# Carrinho Robô de Capina Mecânica Seletiva

Projeto de robótica educacional desenvolvido com Arduino, sensores e atuadores para simular um sistema automatizado de identificação e remoção mecânica de plantas invasoras ou folhas com possíveis problemas em uma área de plantio simulada.

## 1. Descrição do Projeto

O **Carrinho Robô de Capina Mecânica Seletiva** consiste em um protótipo robótico móvel, construído em escala reduzida, com o objetivo de demonstrar como a robótica pode ser aplicada à agricultura para automatizar tarefas de inspeção e remoção pontual de plantas invasoras.

O sistema utiliza um carrinho com motores DC controlados por uma ponte H L298N, um sensor ultrassônico HC-SR04 para detecção de obstáculos, um sensor de cor TCS230/TCS3200 para análise visual simplificada e um braço robótico com quatro servomotores para simular a investigação e remoção mecânica da planta ou folha analisada.

A proposta busca explorar conceitos de:

* robótica educacional;
* automação agrícola;
* programação com Arduino;
* sensores e atuadores;
* sustentabilidade;
* agricultura de precisão;
* pensamento computacional.

## 2. Objetivo Geral

Desenvolver um veículo robótico terrestre em escala reduzida, capaz de se deslocar por uma área de plantio simulada, identificar possíveis plantas invasoras ou folhas com problemas e acionar um mecanismo mecânico de remoção controlado por um braço robótico.

## 3. Objetivos Específicos

* Construir um carrinho robótico utilizando Arduino Uno;
* Controlar motores DC por meio da ponte H L298N;
* Utilizar o sensor HC-SR04 para detectar obstáculos;
* Utilizar o sensor de cor TCS230/TCS3200 para leitura de cores;
* Calibrar um braço robótico com quatro servomotores;
* Criar movimentos suaves para investigação de folhas;
* Simular a remoção mecânica de plantas invasoras;
* Reduzir a dependência de herbicidas químicos em uma proposta educacional e sustentável.

## 4. Componentes Utilizados

### Componentes eletrônicos e robótica

* Arduino Uno;
* Sensor Shield V5.0;
* Ponte H L298N;
* Sensor ultrassônico HC-SR04;
* Sensor de cor TCS230/TCS3200;
* Kit chassi 2WD para Arduino;
* 2 motores DC;
* 4 servomotores;
* Jumpers;
* 3 baterias 18650;
* Suporte para baterias.

### Materiais complementares

* Papelão;
* Cola quente;
* Elástico;
* Fita isolante;
* Estrutura mecânica para adaptação da garra/capina.

## 5. Mapa de Pinos

### Sensor Ultrassônico HC-SR04

| Componente | Pino Arduino |
| ---------- | ------------ |
| TRIG       | D13          |
| ECHO       | D2           |

### Braço Robótico

| Servo    | Pino Arduino |
| -------- | ------------ |
| Garra    | D4           |
| Cotovelo | D5           |
| Ombro    | D6           |
| Base     | D7           |

### Ponte H L298N

| L298N | Pino Arduino |
| ----- | ------------ |
| IN1   | D8           |
| ENA   | D3           |
| IN2   | D10          |
| ENB   | D11          |
| IN3   | D12          |
| IN4   | A5           |

### Compatibilidade entre servos e PWM dos motores

O TRIG do HC-SR04 foi transferido de D3 para D13 e o ENA da ponte H foi
transferido de D9 para D3. Essa alteração foi necessária porque, no Arduino
Uno, a biblioteca `Servo` utiliza o Timer1 e interfere no PWM (`analogWrite`)
dos pinos D9 e D10.

Com ENA em D3 e ENB em D11, os dois canais da L298N permanecem em pinos PWM
controlados pelo Timer2, mesmo durante o funcionamento dos quatro servos. O
pino D13 atende normalmente ao pulso digital de disparo do HC-SR04.

### Sensor de Cor TCS230/TCS3200

| Sensor | Pino Arduino |
| ------ | ------------ |
| OUT    | A0           |
| S0     | A1           |
| S1     | A2           |
| S2     | A3           |
| S3     | A4           |

### Iluminação do sensor de cor

O módulo TCS230/TCS3200 utilizado no projeto possui quatro LEDs brancos
alimentados diretamente pelo VCC, sem pino ou jumper para controle de
intensidade. Os sinais S0/S1 podem colocar o circuito sensor em modo de baixo
consumo e o sinal OE pode desativar sua saída, mas essas funções não apagam os
LEDs do módulo.

Por esse motivo, os LEDs permanecerão ligados durante o funcionamento. Essa
iluminação constante também será considerada parte das condições de calibração
e leitura das cores. Não será realizada modificação física no módulo para
controlar os LEDs.

### Mapa saudável e máscara da folha

A investigação de uma folha utiliza uma região predefinida de 6 × 6 células,
com margem ao redor de uma folha de até aproximadamente 5 cm de largura por
8 cm de comprimento. As linhas correspondem às alturas de 15, 13, 11, 9, 7 e
5 cm, enquanto as colunas usam a base entre 80° e 100°, em passos de 4°. Os
pontos superiores da curva
empírica continuam sendo usados somente como trajetória segura de entrada e
não recebem leituras. Antes da
comparação são realizadas duas varreduras, mantendo suporte, distância e
iluminação fixos:

1. o comando `b` registra o fundo sem a folha;
2. o comando `c` registra uma folha saudável posicionada no gabarito.

O mapa do fundo é salvo na EEPROM do Arduino. Assim, ele permanece disponível
depois de desligar a placa ou carregar uma nova versão do programa. Na
inicialização, o código verifica assinatura, versão, dimensões da grade e
checksum antes de aceitar os dados. O comando `B` maiúsculo apaga a referência
persistente. Se a geometria, o suporte ou a iluminação mudarem, o fundo deverá
ser calibrado novamente com `b`.

O programa compara as duas varreduras e cria automaticamente uma máscara. Uma
célula somente pertence à máscara quando sua cor se diferencia do fundo acima
do limiar configurado. Assim, o suporte e as regiões externas à folha não são
classificados como doença. Durante a investigação, cada célula válida é
comparada com a mesma posição do mapa saudável, preservando as variações
naturais de cor existentes ao longo da folha.

O limiar inicial da máscara foi ajustado experimentalmente para `0,015`. Nas
primeiras medições, o fundo apresentou variações de até aproximadamente
`0,008`, enquanto a presença da folha produziu diferenças de até `0,024`. O
valor adotado fica acima do ruído observado, mas deve ser confirmado repetindo
a calibração sem alterar o gabarito ou a iluminação.

Para que a comparação espacial seja válida, as folhas devem ser posicionadas
no mesmo gabarito e com orientação, distância e iluminação constantes.
O pecíolo — pequena haste que liga a lâmina da folha ao caule ou ramo — deve
ficar acima da primeira linha da grade, correspondente a 15 cm. Como essa
região está fora da área lida, o programa calcula o centro das células da
borda superior da folha e extrapola uma pose inicial de 16 cm. Essa pose usa
ângulos interpolados entre os pontos empíricos de 15 e 17,5 cm e deve ser
ajustada fisicamente antes de se tornar a posição definitiva de corte.

Folhas próximas ou sobrepostas não podem ser separadas de forma confiável pelo
TCS230/TCS3200, pois ele é um sensor pontual e não produz imagens. A estratégia
prevista para uma planta com várias folhas é realizar primeiro uma varredura
ampla para localizar regiões e, depois, aplicar a grade fina em cada região
isolada. A calibração espacial atual trabalha com uma folha por vez.

## 6. Calibração do Braço Robótico

### Garra

| Posição | Ângulo |
| ------- | -----: |
| Aberta  |    37° |
| Fechada |     6° |
| Inicial |    37° |

### Teste de fechamento rápido

A calibração mecânica atual utiliza a garra aberta em 37° e fechada em 6°.
No teste rápido, o comando é enviado diretamente ao servo, sem interpolação,
com intervalo de 350 ms entre fechar e abrir. Em uma série preliminar de 10
tentativas, o mecanismo realizou 8 cortes, correspondendo a 80% de sucesso.
Esse resultado registra a configuração atual, mas ainda deve ser repetido com
mais amostras antes de ser tratado como taxa definitiva de desempenho.

### Base

| Posição  | Ângulo |
| -------- | -----: |
| Direita  |    60° |
| Centro   |    90° |
| Esquerda |   120° |

### Posição inicial segura

| Articulação | Ângulo |
| ----------- | -----: |
| Base        |    90° |
| Ombro       |    40° |
| Cotovelo    |   130° |
| Garra       |    37° |

## 7. Curva Empírica do Braço

A movimentação vertical da garra foi calibrada empiricamente, utilizando medições reais de altura e posição dos servomotores do ombro e cotovelo.

| Altura da garra | Ombro | Cotovelo |
| --------------: | ----: | -------: |
|         18,5 cm |  105° |     140° |
|         17,5 cm |  107° |     127° |
|           15 cm |  110° |     100° |
|           13 cm |  120° |      80° |
|           11 cm |  130° |      75° |
|            9 cm |  140° |      65° |
|            7 cm |  150° |      65° |
|            5 cm |  155° |      65° |
|          2,5 cm |  165° |      40° |

Essa curva é utilizada para que o braço realize movimentos suaves e seguros, evitando trajetórias que possam forçar os servomotores ou travar a estrutura mecânica.

## 8. Funcionamento Previsto

O funcionamento geral do sistema será organizado da seguinte forma:

1. O carrinho se desloca pela área de plantio simulada;
2. O sensor ultrassônico identifica obstáculos ou pontos de interesse;
3. O carrinho para em posição adequada;
4. O braço robótico realiza movimentos investigativos;
5. O sensor de cor analisa a folha ou objeto próximo;
6. O sistema decide se a cor analisada representa uma condição suspeita;
7. Se houver suspeita, a garra simula o corte ou remoção mecânica;
8. O braço retorna à posição inicial;
9. O carrinho continua o percurso.

### Limite físico de alcance e aproximação para corte

Os testes finais mostraram que a garra consegue realizar o corte a uma
distância máxima de aproximadamente 11,5 cm, medida pelo sensor ultrassônico,
quando o mecanismo está a cerca de 15,5 cm de altura. O pecíolo identificado
fica fora desse alcance quando o carrinho permanece na posição usada para a
investigação.

Para respeitar os limites mecânicos do ombro e do cotovelo, o código não força
o braço além da curva segura. Antes de posicionar a garra para o corte, o
carrinho executa um único pulso de aproximação com PWM 80 durante 250 ms, para
os motores e aguarda 500 ms para estabilização. O comando `j` testa somente
essa aproximação; o comando `k` executa o pulso uma vez, quando necessário, e
depois posiciona o braço no candidato a pecíolo para ajuste.

O comando `J` maiúsculo executa um único pulso manual de ré. A ré não é chamada
automaticamente por nenhuma rotina e possui constantes próprias de PWM e
duração para calibração independente.

Essa aproximação é uma solução funcional para o protótipo atual. Como os
motores DC não possuem encoders, ela não constitui navegação de precisão. Uma
versão futura deverá utilizar encoders, controle independente das rodas ou uma
estrutura mecânica com maior alcance.

## 9. Organização Sugerida do Repositório

```text
carrinho-robo-capina/
│
├── README.md
│
├── codigos/
│   ├── teste_garra/
│   ├── teste_base/
│   ├── teste_motores_l298n/
│   ├── teste_hcsr04/
│   ├── teste_sensor_cor/
│   ├── teste_curva_braco/
│   └── integracao_investigacao/
│
├── imagens/
│   ├── montagem/
│   ├── circuito/
│   └── testes/
│
├── documentos/
│   ├── relatorio_projeto.pdf
│   └── referencias.md
│
└── esquemas/
    └── mapa_de_pinos.md
```

## 10. Códigos do Projeto

Os códigos serão desenvolvidos por etapas, começando por testes individuais dos componentes e avançando para a integração completa do sistema.

### Etapas de programação

* Teste da garra;
* Calibração da base;
* Calibração conjunta de ombro e cotovelo;
* Teste dos motores com L298N;
* Teste do sensor ultrassônico HC-SR04;
* Teste do sensor de cor TCS230/TCS3200;
* Teste da curva empírica do braço;
* Integração do movimento investigativo;
* Integração final com deslocamento do carrinho.

## 11. Alimentação do Projeto

A alimentação deve ser feita com atenção para evitar reinicializações ou danos aos componentes.

### Recomendações

* Motores DC: alimentados pela ponte H L298N com bateria apropriada;
* Servomotores: fonte externa de 5V com corrente suficiente;
* Arduino: alimentação via USB ou fonte própria;
* Todos os GNDs devem estar interligados.

### Observação importante

Não é recomendado alimentar os quatro servomotores diretamente pelo pino 5V do Arduino, pois isso pode causar queda de tensão, travamento dos servos ou reinicialização da placa.

## 12. Status do Projeto

| Etapa                         | Status             |
| ----------------------------- | ------------------ |
| Teste da garra                | Concluído          |
| Calibração da base            | Concluído          |
| Calibração ombro/cotovelo     | Em ajuste fino     |
| Teste da ponte H L298N        | Concluído          |
| Teste do sensor ultrassônico  | Concluído          |
| Teste do sensor de cor        | Concluído          |
| Movimento investigativo       | Em desenvolvimento |
| Integração final com carrinho | Pendente           |

## 13. Melhorias Futuras

* Ajustar a leitura do sensor de cor com maior precisão;
* Criar regras específicas para diferenciar folha saudável, folha com problema e erva daninha;
* Integrar sensores adicionais;
* Melhorar a mecânica da garra;
* Implementar navegação autônoma;
* Registrar dados de leitura durante os testes;
* Criar interface para monitoramento dos valores dos sensores;
* Adicionar LEDs ou buzzer para indicar o status da operação.

## 14. Referências

ARDUINO. *Arduino Documentation*. Disponível em: https://www.arduino.cc/. Acesso em: 2026.

PARANÁ. Secretaria de Estado da Educação. Diretoria de Tecnologia e Inovação. *Robótica: materiais didáticos de robótica educacional*. Curitiba: SEED-PR.

EMBRAPA. *Agricultura de Precisão e Agricultura Digital*. Brasília: EMBRAPA.

## 15. Autor

Projeto desenvolvido em contexto educacional para aplicação de conceitos de robótica, programação e sustentabilidade na agricultura.

**Professor orientador:** Prof. Gerson
**Área:** Robótica Educacional / Arduino / Automação Agrícola
