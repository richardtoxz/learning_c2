# Sistema de Monitoramento de Sensores Industriais

**Disciplina:** Estruturas de Dados  
**Curso:** Análise e Desenvolvimento de Sistemas  
**Linguagem:** C
**Professor:** Zoe Roberto

## Visão Geral

Este projeto reúne três pequenas ferramentas que simulam um fluxo completo de manipulação de dados de sensores:

1. **gera_dados** – Gera um arquivo de leituras aleatórias para múltiplos sensores.  
2. **organiza_sensores** – Separa e ordena essas leituras por tipo de sensor.  
3. **consulta_sensor** – Busca a leitura mais próxima de um determinado instante usando busca binária.

O objetivo é exercitar alocação dinâmica, ordenação, busca e tratamento de arquivos em C.

---

## Funcionalidades Principais

- Geração de 2.000 amostras por sensor dentro de um intervalo de tempo definido  
- Suporte a quatro tipos de dados: inteiro, ponto flutuante, booleano e string  
- Separação e ordenação de leituras em arquivos distintos por tipo de sensor  
- Busca binária aproximada (O(log n)) para consultas rápidas  
- Tratamento robusto de erros (validação de argumentos, alocação de memória e I/O)

---

## Pré-requisitos

- GCC
- Ambiente Windows ou Unix com terminal 

---

## Como Compilar

No diretório do projeto, basta:

```bash
gcc organiza_sensores.c -o organiza_sensores
gcc consulta_sensor.c -o consulta_sensor
gcc gera_dados.c -o gera_dados
```

---

## Exemplos de Uso

1. **Gerar dados de teste**  
   ```bash
   ./gera_dados 1640995200 1640999000 TEMP_01:float PRESS_02:int FLOW_03:bool
   ```
   Isso produz `dados_gerados.txt` com leituras embaralhadas.

2. **Organizar e ordenar**  
   ```bash
   ./organiza_sensores dados_gerados.txt
   ```
   Gera arquivos como `TEMP_ordenado.txt`, `PRESS_ordenado.txt`, etc., cada um em ordem crescente de timestamp.

3. **Consultar um sensor**  
   ```bash
   ./consulta_sensor TEMP_01 1640997000
   ```
   Exibe a leitura mais próxima do instante fornecido, junto com o contexto (vizinhas).

---

## Formato dos Arquivos

### Entrada (`dados_gerados.txt` ou CSV de exemplo)
Cada linha:  
```
<timestamp>,<sensor_id>,<valor>
```
Exemplo:
```
1640995200,TEMP_01,23.5
1640995300,PRESS_02,1013
1640995350,FLOW_03,true
```

### Saída (`<TIPO>_ordenado.txt`)
- Primeiras linhas: comentários com total de leituras e formato  
- Seguimento: mesmas linhas acima, mas ordenadas por timestamp

---

## Por Dentro do Código

- **leitura_t**: armazena `timestamp`, `sensor_id` e `valor` como string.  
- **sensor_tipo_t**: agrupa leituras de um mesmo tipo, ajustando a capacidade com `realloc`.  
- **qsort()**: ordena as leituras em O(n log n).  
- **busca_binaria_aproximada()**: encontra o índice de leitura mais próximo de um timestamp em O(log n).  

Cada programa lida com validações de parâmetro, checagens de I/O e liberações de memória para garantir robustez.