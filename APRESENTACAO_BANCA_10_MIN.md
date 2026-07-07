# Apresentação à banca — Agrinho Robótica 2026

## Projeto

**Carrinho Robô de Capina Mecânica Seletiva**

Tema do regulamento: **“Agro forte, futuro sustentável: equilíbrio entre produção e meio ambiente”.**

Tempo máximo: **10 minutos**  
Equipe: **4 estudantes**  
Meta de ensaio: terminar entre **9min20s e 9min40s**, deixando margem para imprevistos.

---

## Análise segundo a rubrica

### Diagnóstico atual

| Critério | Máximo | Estimativa atual | Evidências | Ponto de atenção |
|---|---:|---:|---|---|
| Apresentação | 15 | 12 | Objetivos, componentes, etapas, mapa de pinos e calibrações documentados | Ensaiar para não ultrapassar 10 minutos e distribuir a fala entre os quatro estudantes |
| Organização | 15 | 13 | Desenvolvimento modular, testes individuais, mapa de pinos, fontes separadas e teste integrado | Atualizar o status final e reunir fotos/vídeos cronológicos |
| Criatividade / inovação | 20 | 17 | Une inspeção por cor, mapeamento bidimensional e corte mecânico seletivo em protótipo educacional | Explicar claramente o que foi criado pela equipe, evitando apresentar como produto agrícola pronto |
| Relevância e impacto | 20 | 18 | Relaciona produção agrícola, redução do uso indiscriminado de herbicidas e intervenção localizada | Apresentar impacto como potencial e não como resultado agronômico já comprovado |
| Complexidade / dificuldade | 30 | 23 | Quatro servos, dois motores, três sensores/módulos, PWM, filtragem, calibração RGB, curva empírica, regiões conectadas e movimentos suaves | A navegação autônoma completa ainda precisa ser unificada ao código de investigação |
| **Total estimado** | **100** | **83** | Projeto tecnicamente competitivo | A integração operacional completa pode elevar principalmente complexidade e organização |

### O que pode elevar a nota

1. Demonstrar uma sequência automática única: deslocar, detectar, parar, investigar, decidir, cortar e retomar.
2. Exibir registros do processo: erros, travamentos por alimentação, conflito de PWM e soluções adotadas.
3. Mostrar no vídeo ou ao vivo uma folha saudável e outra amostra discrepante, com a decisão impressa no Monitor Serial.
4. Explicar a autoria dos estudantes: quem montou, mediu, programou, testou e documentou cada etapa.
5. Levar um esquema elétrico simples e legível, além do protótipo.

### Risco de desclassificação que precisa ser resolvido

O item 1.7 do regulamento desclassifica trabalhos que gerem risco à integridade física dos estudantes. Como a garra utiliza lâminas:

- usar proteção física nas partes cortantes durante transporte e apresentação;
- manter o protótipo em área demarcada e fora do alcance das mãos;
- demonstrar o corte apenas com material vegetal macio, preso em suporte;
- ter chave de desligamento acessível e um responsável pronto para cortar a energia;
- nunca segurar o caule com a mão durante o golpe;
- se não houver proteção mecânica adequada, substituir as lâminas afiadas por elementos sem corte na apresentação.

---

# Roteiro de 10 minutos em quatro partes

## Parte 1 — Problema, ideia e tema

**Estudante 1 — 0:00 a 2:15**

### Fala sugerida

“Bom dia. Nosso projeto é o Carrinho Robô de Capina Mecânica Seletiva. Ele nasceu de uma pergunta: como usar a robótica para atuar somente onde existe um possível problema na plantação, evitando uma intervenção em toda a área?

Nossa proposta é um protótipo educacional de agricultura de precisão. O carrinho se desloca, detecta um ponto de interesse, interrompe o movimento e usa um braço robótico com sensor de cor para investigar a área. Quando encontra uma diferença em relação à folha saudável usada como referência, executa uma remoção mecânica localizada.

O projeto se conecta ao tema ‘Agro forte, futuro sustentável’ porque busca conciliar produtividade e cuidado ambiental. Em vez de simular a aplicação generalizada de produto químico, investigamos uma intervenção pontual e mecânica.

Não apresentamos o protótipo como uma máquina agrícola pronta. Ele é uma prova de conceito em escala reduzida, criada para estudar automação, sensores, programação, sustentabilidade e tomada de decisão.”

### Mostrar

- protótipo completo;
- uma imagem ou slide com o problema e a solução;
- fluxo resumido: **deslocar → detectar → investigar → decidir → cortar**.

### Frase de passagem

“Para transformar essa ideia em um protótipo funcional, organizamos o desenvolvimento em módulos, como será explicado agora.”

---

## Parte 2 — Construção, componentes e método de desenvolvimento

**Estudante 2 — 2:15 a 4:30**

### Fala sugerida

“O protótipo utiliza um Arduino Uno, dois motores DC com ponte H L298N, sensor ultrassônico HC-SR04, sensor de cor TCS3200 e um braço com quatro servomotores: base, ombro, cotovelo e garra.

Nós não ligamos tudo de uma vez. Primeiro testamos cada componente separadamente. Depois calibramos os limites seguros de cada servo e medimos nove posições reais do braço, entre 18,5 e 2,5 centímetros. Essas medições formaram uma curva empírica, usada para evitar movimentos bruscos ou trajetórias que forçassem a estrutura.

Durante os testes, os servos travavam quando recebiam energia pelo Arduino. Identificamos queda de tensão e separamos a alimentação: motores, servos e lógica possuem fontes apropriadas, mas todos compartilham o mesmo GND.

Também encontramos um conflito técnico: a biblioteca Servo usa o Timer1 do Arduino e interfere no PWM dos pinos 9 e 10. Por isso transferimos os controles de velocidade dos motores para os pinos 3 e 11, ligados ao Timer2, e mudamos o disparo do ultrassônico para o pino 13.

Essa sequência de testar, medir, corrigir e integrar tornou a montagem mais estável. O teste conjunto foi executado sem travamentos.”

### Mostrar

- mapa de pinos;
- tabela da curva empírica;
- diagrama das três alimentações com GND comum;
- uma foto de uma etapa de teste.

### Frase de passagem

“Com a parte elétrica estabilizada e os movimentos calibrados, desenvolvemos a lógica de investigação e decisão.”

---

## Parte 3 — Programação, decisão e complexidade técnica

**Estudante 3 — 4:30 a 7:05**

### Fala sugerida

“Na programação, o braço não salta diretamente entre posições. Usamos interpolação suave, com aceleração e desaceleração, para reduzir impactos mecânicos.

O sensor de cor mede vermelho, verde e azul. Como a intensidade da iluminação pode variar, o código compara as proporções normalizadas de cada canal, e não apenas valores brutos. Primeiro registramos uma folha saudável como referência. Depois, cada ponto é lido três vezes e só é considerado suspeito quando pelo menos duas leituras confirmam a diferença. Isso reduz decisões causadas por ruído.

O braço realiza uma varredura em linhas alternadas, formando uma matriz de pontos. O programa marca as células suspeitas e procura grupos conectados, inclusive pelas diagonais. Em seguida identifica a maior região, calcula seu centro e movimenta o braço até esse ponto. Portanto, a decisão não depende de uma única leitura isolada.

O mecanismo de corte também foi testado separadamente. A garra fecha, o braço recolhe, eleva e só depois abre, criando um golpe mecânico mais efetivo.

Além do código principal, mantivemos programas de teste para motores, ultrassônico, servos, integração completa e golpe de corte. Essa modularização ajudou a localizar falhas sem colocar todo o protótipo em risco.”

### Mostrar

- pequena matriz desenhada com células suspeitas e o centro calculado;
- no máximo 8 a 12 linhas de código, destacando normalização, confirmação 2 de 3 e movimento suave;
- Monitor Serial mostrando uma leitura RGB normalizada.

### Frase de passagem

“Agora mostraremos o resultado obtido, os cuidados de segurança e o impacto que enxergamos para essa ideia.”

---

## Parte 4 — Demonstração, resultados, impacto e encerramento

**Estudante 4 — 7:05 a 9:40**

### Demonstração sugerida — até 1min20s

Escolher previamente uma demonstração curta e confiável. Não executar a varredura completa de todas as células, pois ela pode ultrapassar o tempo.

Sequência recomendada:

1. mostrar leitura de referência saudável já calibrada;
2. aproximar uma amostra de cor diferente e mostrar a discrepância no Monitor Serial;
3. executar o golpe de corte em material preso a um suporte;
4. manter rodas suspensas ou motores desligados durante a demonstração do braço.

### Fala sugerida

“Nos testes, validamos separadamente os motores, o sensor ultrassônico, o sensor de cor e os quatro servos. Também executamos um teste conjunto sem travamentos. A demonstração apresentada é curta para respeitar o limite da banca, mas conserva as etapas centrais de percepção, decisão e atuação.

Nosso resultado principal não é apenas o movimento do carrinho. É a construção de um sistema que mede, compara, decide e atua de maneira localizada.

Reconhecemos os limites do protótipo. A classificação atual é baseada em diferença de cor e ainda não identifica espécies de plantas. A navegação autônoma completa é a próxima integração do projeto. Como melhorias futuras, propomos visão computacional, proteção mecânica permanente, testes em diferentes iluminações e validação em ambiente agrícola controlado.

O potencial de impacto está em demonstrar que pequenas máquinas podem apoiar intervenções seletivas, reduzindo desperdícios e estimulando soluções sustentáveis. Para nossa equipe, o projeto também transformou problemas reais — energia, temporizadores, ruído e mecânica — em aprendizagem científica.

Assim, nosso carrinho representa o tema do Agrinho ao buscar uma produção mais inteligente, com tecnologia aplicada ao cuidado com o meio ambiente. Obrigado.”

---

## Controle de tempo

| Momento | Tempo acumulado |
|---|---:|
| Parte 1 concluída | 2:15 |
| Parte 2 concluída | 4:30 |
| Parte 3 concluída | 7:05 |
| Demonstração concluída | 8:25 |
| Encerramento concluído | 9:40 |
| Margem disponível | 0:20 |

---

## Slides sugeridos

Usar **seis slides**, com pouco texto:

1. **Título e problema:** capina seletiva e sustentabilidade.
2. **Como funciona:** fluxo em cinco etapas.
3. **Construção:** componentes, fontes e mapa de pinos.
4. **Engenharia:** curva empírica, conflito de PWM e testes modulares.
5. **Inteligência do sistema:** RGB normalizado, confirmação 2/3 e mapa de regiões.
6. **Resultados, limitações e impacto:** vídeo curto ou demonstração, próximos passos e frase final.

---

## Perguntas prováveis da banca

### “Como o robô sabe que é uma erva daninha?”

Resposta: “Nesta versão ele não reconhece espécies. Ele detecta discrepâncias de cor em relação a uma referência saudável. Chamamos isso de triagem por cor e reconhecemos que identificação botânica exigiria câmera, modelo de visão ou outros sensores.”

### “O robô já é totalmente autônomo?”

Resposta: “Os módulos e o teste conjunto estão validados, e a investigação por cor funciona automaticamente. A unificação da navegação com toda a sequência operacional ainda é a próxima etapa. Preferimos mostrar com precisão o que já foi validado.”

### “Por que usar três fontes?”

Resposta: “Motores e servos produzem picos de corrente que causavam queda de tensão e travamentos. A separação aumenta a estabilidade; os GNDs permanecem comuns para que todos os sinais tenham a mesma referência.”

### “Qual é a inovação?”

Resposta: “A inovação está na combinação, em escala educacional, de mobilidade, varredura bidimensional, comparação relativa de cor, confirmação contra ruído, localização da maior região suspeita e atuação mecânica seletiva.”

### “Como garantem a segurança das lâminas?”

Resposta: “As lâminas ficam protegidas no transporte, a demonstração ocorre em área demarcada, o material é preso em suporte e há desligamento imediato da alimentação. Ninguém segura o caule ou aproxima as mãos durante o movimento.”

### “Por que não usar apenas valores RGB fixos?”

Resposta: “A calibração com uma referência local e a normalização das proporções RGB reduzem a influência da intensidade luminosa. Ainda assim, mantemos a iluminação constante durante calibração e leitura.”

### “Que evidência vocês têm de funcionamento?”

Resposta: “Temos códigos e registros de testes individuais, tabela de calibração do braço, leituras do Monitor Serial e teste integrado sem travamentos. Na apresentação mostramos uma sequência curta e repetível.”

---

## Checklist antes da banca

- [ ] Exatamente quatro estudantes inscritos e presentes.
- [ ] Cada estudante domina sua fala e também entende o projeto completo.
- [ ] Apresentação ensaiada três vezes, sempre abaixo de 10 minutos.
- [ ] Fotos e vídeos do desenvolvimento organizados cronologicamente.
- [ ] Autoria e função de cada estudante registradas.
- [ ] Protótipo carregado e testado no mesmo dia.
- [ ] Amostra saudável e amostra discrepante preparadas.
- [ ] Plano B em vídeo, caso a demonstração ao vivo falhe.
- [ ] Monitor Serial configurado em 9600 baud.
- [ ] Rodas suspensas durante testes de bancada.
- [ ] Lâminas protegidas e material de corte preso em suporte.
- [ ] Chave ou conector de desligamento imediatamente acessível.
- [ ] Nenhuma afirmação de eficácia agronômica sem evidência.
- [ ] Limitações e próximos passos explicados com segurança.

