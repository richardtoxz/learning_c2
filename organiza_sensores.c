#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    long timestamp;
    char sensor_id[32];
    char valor_str[32];
} leitura_t;

typedef struct
{
    char tipo[16];
    leitura_t *leituras;
    int count;
    int capacidade;
} sensor_tipo_t;

int comparar_timestamp(const void *a, const void *b)
{
    const leitura_t *leitura_a = (const leitura_t *)a;
    const leitura_t *leitura_b = (const leitura_t *)b;

    if (leitura_a->timestamp < leitura_b->timestamp)
        return -1;
    if (leitura_a->timestamp > leitura_b->timestamp)
        return 1;
    return 0;
}

void extrair_tipo_sensor(const char *sensor_id, char *tipo)
{
    if (!sensor_id || !tipo)
        return;
    const char *underscore = strchr(sensor_id, '_');
    if (underscore != NULL)
    {
        size_t len = underscore - sensor_id;
        strncpy(tipo, sensor_id, len);
        tipo[len] = '\0';
    }
    else
    {
        strcpy(tipo, sensor_id);
    }
}

int adicionar_leitura(sensor_tipo_t *sensor, const leitura_t *leitura)
{
    if (sensor == NULL || leitura == NULL)
    {
        return -1;
    }

    if (sensor->count >= sensor->capacidade)
    {
        int nova_capacidade = sensor->capacidade == 0 ? 10 : sensor->capacidade * 2;
        leitura_t *novo_array = realloc(sensor->leituras, nova_capacidade * sizeof(leitura_t));

        if (novo_array == NULL)
        {
            fprintf(stderr, "Erro: Falha ao realocar memoria para sensor %s\n", sensor->tipo);
            return -1;
        }

        sensor->leituras = novo_array;
        sensor->capacidade = nova_capacidade;
    }

    sensor->leituras[sensor->count] = *leitura;
    sensor->count++;

    return 0;
}

sensor_tipo_t *encontrar_ou_criar_sensor(sensor_tipo_t **sensores, int *num_sensores,
                                         int *capacidade_sensores, const char *tipo)
{
    for (int i = 0; i < *num_sensores; i++)
    {
        if (strcmp((*sensores)[i].tipo, tipo) == 0)
        {
            return &(*sensores)[i];
        }
    }

    if (*num_sensores >= *capacidade_sensores)
    {
        int nova_capacidade = *capacidade_sensores == 0 ? 5 : (*capacidade_sensores) * 2;
        sensor_tipo_t *novo_array = realloc(*sensores, nova_capacidade * sizeof(sensor_tipo_t));

        if (novo_array == NULL)
        {
            fprintf(stderr, "Erro: Falha ao realocar memoria para array de sensores\n");
            return NULL;
        }

        *sensores = novo_array;
        *capacidade_sensores = nova_capacidade;
    }

    sensor_tipo_t *novo_sensor = &(*sensores)[*num_sensores];
    strcpy(novo_sensor->tipo, tipo);
    novo_sensor->leituras = NULL;
    novo_sensor->count = 0;
    novo_sensor->capacidade = 0;

    (*num_sensores)++;
    return novo_sensor;
}

int processar_linha(const char *linha,
                    sensor_tipo_t **sensores,
                    int *num_sensores,
                    int *capacidade_sensores)
{
    if (!linha || !sensores || !num_sensores || !capacidade_sensores)
        return -1;

    leitura_t leitura;
    char linha_copia[256];

    strncpy(linha_copia, linha, sizeof(linha_copia) - 1);
    linha_copia[sizeof(linha_copia) - 1] = '\0';
    char *token = strtok(linha_copia, ",");
    if (token == NULL)
    {
        return 0;
    }

    leitura.timestamp = strtol(token, NULL, 10);
    if (leitura.timestamp == 0)
    {
        return 0;
    }

    token = strtok(NULL, ",");
    if (token == NULL)
    {
        return 0;
    }
    strncpy(leitura.sensor_id, token, sizeof(leitura.sensor_id) - 1);
    leitura.sensor_id[sizeof(leitura.sensor_id) - 1] = '\0';
    token = strtok(NULL, ",\n\r");
    if (token == NULL)
    {
        return 0;
    }
    strncpy(leitura.valor_str, token, sizeof(leitura.valor_str) - 1);
    leitura.valor_str[sizeof(leitura.valor_str) - 1] = '\0';

    char tipo[16];
    extrair_tipo_sensor(leitura.sensor_id, tipo);

    sensor_tipo_t *sensor = encontrar_ou_criar_sensor(sensores, num_sensores,
                                                      capacidade_sensores, tipo);
    if (sensor == NULL)
    {
        return -1;
    }

    return adicionar_leitura(sensor, &leitura);
}

int salvar_sensor(const sensor_tipo_t *sensor)
{
    if (!sensor)
    {
        fprintf(stderr, "Erro interno: ponteiro sensor nulo\n");
        return -1;
    }

    char nome_arquivo[64];
    snprintf(nome_arquivo, sizeof(nome_arquivo), "%s_ordenado.txt", sensor->tipo);
    FILE *arquivo = fopen(nome_arquivo, "w");
    if (arquivo == NULL)
    {
        fprintf(stderr, "Erro ao criar arquivo de saida\n");
        return -1;
    }

    fprintf(arquivo, "# Dados do sensor tipo: %s\n", sensor->tipo);
    fprintf(arquivo, "# Formato: timestamp,sensor_id,valor\n");
    fprintf(arquivo, "# Total de leituras: %d\n", sensor->count);

    for (int i = 0; i < sensor->count; i++)
    {
        fprintf(arquivo, "%ld,%s,%s\n",
                sensor->leituras[i].timestamp,
                sensor->leituras[i].sensor_id,
                sensor->leituras[i].valor_str);
    }

    fclose(arquivo);
    printf("Arquivo '%s' criado com %d leituras\n", nome_arquivo, sensor->count);
    return 0;
}

int processar_arquivo(const char *nome_arquivo)
{
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL)
    {
        fprintf(stderr, "Erro: Nao foi possivel abrir o arquivo %s\n", nome_arquivo);
        return -1;
    }

    sensor_tipo_t *sensores = NULL;
    int num_sensores = 0;
    int capacidade_sensores = 0;
    char linha[256];
    int linha_num = 0;
    int linhas_processadas = 0;

    printf("Processando arquivo: %s\n", nome_arquivo);

    while (fgets(linha, sizeof(linha), arquivo) != NULL)
    {
        linha_num++;

        if (linha[0] == '\n' || linha[0] == '#' || linha[0] == '\0')
        {
            continue;
        }

        if (processar_linha(linha, &sensores, &num_sensores, &capacidade_sensores) == 0)
        {
            linhas_processadas++;
        }
        else
        {
            fprintf(stderr, "Erro ao processar linha %d\n", linha_num);
        }
    }
    fclose(arquivo);

    printf("Tipos de sensores encontrados: %d\n", num_sensores);

    for (int i = 0; i < num_sensores; i++)
    {
        qsort(sensores[i].leituras, sensores[i].count,
              sizeof(leitura_t), comparar_timestamp);

        if (salvar_sensor(&sensores[i]) != 0)
        {
            fprintf(stderr, "Erro ao salvar dados do sensor %s\n", sensores[i].tipo);
        }
    }

    for (int i = 0; i < num_sensores; i++)
    {
        free(sensores[i].leituras);
        sensores[i].leituras = NULL;
    }
    free(sensores);
    sensores = NULL;

    return 0;
}

int main(int argc, char *argv[])
{
    printf("=== Sistema de Monitoramento de Sensores Industriais ===\n");
    printf("Programa 1: Organizador de Dados\n\n");

    if (argc != 2)
    {
        printf("Uso: %s <arquivo_entrada>\n", argv[0]);
        printf("Exemplo: %s dados_sensores.txt\n", argv[0]);
        return -1;
    }
    FILE *test_file = fopen(argv[1], "r");
    if (test_file == NULL)
    {
        fprintf(stderr, "Erro: Arquivo '%s' nao existe ou nao pode ser acessado\n", argv[1]);
        return -1;
    }
    fclose(test_file);

    int resultado = processar_arquivo(argv[1]);

    if (resultado == 0)
    {
        printf("\nProcessamento concluido\n");
        printf("Arquivos de saida criados: *_ordenado.txt\n");
    }
    else
    {
        printf("\nErro durante o processamento.\n");
    }

    return resultado;
}
