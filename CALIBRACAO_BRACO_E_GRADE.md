# Ficha de recalibração do braço e da grade

Data: ____/____/________  
Responsável pelo teste: ______________________________  
Tensão das pilhas dos servos: ______ V  
Saída do LM2596: ______ V  
Distância desejada entre sensor e folha: ______ cm

## Condições do ensaio

- Manter a base em 90° durante as medições verticais.
- Fixar a folha ou uma placa de referência sempre no mesmo plano.
- Medir a distância perpendicular entre a face do TCS3200 e esse plano.
- Usar sempre o mesmo ponto do sensor como referência da régua.
- Fazer três medições de distância em cada posição.
- Anotar ruído, esforço, tremor, colisão ou aproximação excessiva.
- Os pontos 3 a 8 formam atualmente a grade vertical de investigação.

## Tabela principal

| Ponto | Uso | Altura nominal | Altura medida | Distância 1 | Distância 2 | Distância 3 | Ombro nominal | Ombro ajustado | Cotovelo nominal | Cotovelo ajustado | Observações |
|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| 1 | Trajetória | 18,5 cm | ____ | ____ | ____ | ____ | 105° | ____ | 140° | ____ | __________________ |
| 2 | Trajetória | 17,5 cm | ____ | ____ | ____ | ____ | 107° | ____ | 127° | ____ | __________________ |
| 3 | Grade — linha 0 | 15,0 cm | ____ | ____ | ____ | ____ | 110° | ____ | 100° | ____ | __________________ |
| 4 | Grade — linha 1 | 13,0 cm | ____ | ____ | ____ | ____ | 120° | ____ | 80° | ____ | __________________ |
| 5 | Grade — linha 2 | 11,0 cm | ____ | ____ | ____ | ____ | 130° | ____ | 75° | ____ | __________________ |
| 6 | Grade — linha 3 | 9,0 cm | ____ | ____ | ____ | ____ | 140° | ____ | 65° | ____ | __________________ |
| 7 | Grade — linha 4 | 7,0 cm | ____ | ____ | ____ | ____ | 150° | ____ | 65° | ____ | __________________ |
| 8 | Grade — linha 5 | 5,0 cm | ____ | ____ | ____ | ____ | 155° | ____ | 65° | ____ | __________________ |
| 9 | Limite inferior | 2,5 cm | ____ | ____ | ____ | ____ | 165° | ____ | 40° | ____ | __________________ |

## Médias calculadas

Depois das medições, calcular:

```text
distância média = (distância 1 + distância 2 + distância 3) / 3
erro de altura  = altura medida - altura nominal
```

| Ponto | Distância média | Erro de altura | Distância adequada? | Movimento seguro? |
|---:|---:|---:|---|---|
| 1 | ____ cm | ____ cm | Sim / Não | Sim / Não |
| 2 | ____ cm | ____ cm | Sim / Não | Sim / Não |
| 3 | ____ cm | ____ cm | Sim / Não | Sim / Não |
| 4 | ____ cm | ____ cm | Sim / Não | Sim / Não |
| 5 | ____ cm | ____ cm | Sim / Não | Sim / Não |
| 6 | ____ cm | ____ cm | Sim / Não | Sim / Não |
| 7 | ____ cm | ____ cm | Sim / Não | Sim / Não |
| 8 | ____ cm | ____ cm | Sim / Não | Sim / Não |
| 9 | ____ cm | ____ cm | Sim / Não | Sim / Não |

## Calibração horizontal da grade

| Coluna | Base nominal | Base ajustada | Posição horizontal medida | Observações |
|---:|---:|---:|---:|---|
| 0 | 80° | ____ | ____ cm | __________________ |
| 1 | 84° | ____ | ____ cm | __________________ |
| 2 | 88° | ____ | ____ cm | __________________ |
| 3 | 92° | ____ | ____ cm | __________________ |
| 4 | 96° | ____ | ____ cm | __________________ |
| 5 | 100° | ____ | ____ cm | __________________ |

## Pose virtual do pecíolo

Estimativa atual antes do ajuste manual:

| Altura | Base | Ombro | Cotovelo | Garra |
|---:|---:|---:|---:|---:|
| 16,0 cm | calculada pela máscara | 109° | 111° | 37° |

Pose correta obtida com `k` e os comandos de ajuste:

```text
Altura aproximada: ______ cm
Base: ______°
Ombro: ______°
Cotovelo: ______°
Distância até o pecíolo: ______ cm
Resultado do fechamento: __________________________________________
```

## Sequência recomendada

1. Testar os pontos 1 a 9 sem folha e sem corte.
2. Preencher altura, distância e ângulos ajustados.
3. Repetir cada distância três vezes.
4. Enviar a tabela preenchida antes de alterar a curva principal.
5. Atualizar os ângulos no código.
6. Repetir a trajetória completa para verificar suavidade e segurança.
7. Somente depois recalibrar `b`, `c`, a máscara e a pose do pecíolo.

## Calibração dos motores

| Motor | PWM calibrado |
|---|---:|
| A | 70 |
| B | 80 |

O motor A apresentou maior velocidade ou força e, por isso, recebeu PWM
menor para equilibrar o movimento com o motor B.

### Calibração inicial do giro

| Movimento | Medição com 300 ms | Tempo estimado para 90 graus |
|---|---:|---:|
| Sentido 1 — esquerda | 25 graus | 1080 ms |
| Sentido 2 — direita | 20 graus | 1350 ms |

Os tempos de 90 graus ainda devem ser confirmados no piso, pois a partida,
o atrito e a derrapagem podem impedir uma proporção perfeitamente linear.

Segundo ensaio: 1080 ms produziram cerca de 180 graus à esquerda e 1350 ms
produziram cerca de 150 graus à direita. Considerando os dois ensaios e o
tempo necessário para vencer a inércia, os novos valores de teste são:

- esquerda: 625 ms;
- direita: 865 ms.

Terceiro ensaio: a esquerda atingiu 90 graus e a direita atingiu 86 graus.
O tempo final de teste da direita foi corrigido de 865 ms para 905 ms.

### Teste da varredura radar

Com o carrinho inicialmente alinhado à trilha e o braço recolhido:

1. O primeiro toque no botão aguarda 1 segundo.
2. O carrinho vai a -90 graus em três movimentos separados de 30 graus.
3. Mede de -90 a +90 graus em intervalos de 15 graus (13 setores).
4. Fica apontado para a menor distância válida, limitada a 80 cm.
5. O toque seguinte retorna o carrinho à direção original da trilha.

Tempos iniciais estimados para cada passo de 30 graus:

- esquerda: 325 ms;
- direita: 385 ms.

Depois do primeiro teste, verificar se os seis passos de 30 graus realmente
levam o carrinho de -90 a +90 graus. Ajustar os tempos se houver erro acumulado.

O salto direto inicial de 90 graus foi removido após o primeiro ensaio do
radar, que acertou 3 de 5 buscas. Todos os deslocamentos do radar passaram a
usar passos iguais de 30 graus para reduzir diferenças de inércia e derrapagem.

Após corrigir um cabo dos motores, os passos de 325 ms à esquerda e 385 ms à
direita passaram a produzir aproximadamente 40 graus. Os tempos foram
reduzidos proporcionalmente para 244 ms à esquerda e 289 ms à direita.

No ajuste fino, o passo à esquerda ficou fraco. Seu tempo foi aumentado para
270 ms; o passo à direita permaneceu em 289 ms.

### Calibração dos passos de 15 graus

Antes de ampliar o radar para 13 setores, foi criado um teste isolado com os
seguintes tempos iniciais:

- esquerda: 190 ms;
- direita: 145 ms.

Esses valores precisam ser medidos no piso. Eles não correspondem simplesmente
à metade do passo de 30 graus porque existe um tempo inicial para vencer a
inércia dos motores e das rodas.

No primeiro ensaio, 190 ms produziram 15 graus à esquerda e 145 ms produziram
10 graus à direita. Combinando esse resultado com o passo de 30 graus, o tempo
da direita foi ajustado por interpolação para 181 ms.

Calibração confirmada com os valores ajustados manualmente. O radar principal
passou a usar 197 ms à esquerda e 180 ms à direita, com setores em -90, -75,
-60, -45, -30, -15, 0, +15, +30,
+45, +60, +75 e +90 graus.

### Aproximação de teste com objetos

Após o radar apontar para o objeto mais próximo:

1. O segundo toque no botão avança em pulsos de 150 ms.
2. Entre os pulsos, o HC-SR04 mede novamente a distância.
3. O carrinho para inicialmente a 25 cm do objeto.
4. Há um limite de 25 pulsos para evitar avanço indefinido.
5. O terceiro toque recua a mesma quantidade de pulsos e retorna à trilha.

O alvo de 25 cm é conservador para objetos. A distância de corte de 11,5 cm
somente será aplicada depois da calibração real com plantas.

Os pulsos iniciais de 80 ms faziam o carrinho desviar para a direita porque
uma roda começava a mover antes da outra vencer a inércia. O pulso foi
aumentado para 150 ms antes de qualquer nova alteração dos PWMs.

Os comandos `g` e `G` permanecem disponíveis no Monitor Serial para testar
isoladamente os giros à esquerda e à direita.
