#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum {
    TIPO_INT,
    TIPO_FLOAT, 
    TIPO_BOOL,
    TIPO_STRING
} tipo_sensor_t;

typedef struct {
    char nome[32];
    tipo_sensor_t tipo;
} sensor_config_t;

tipo_sensor_t string_para_tipo(const char* tipo_str) {
    if (strcmp(tipo_str, "int") == 0) return TIPO_INT;
    if (strcmp(tipo_str, "float") == 0) return TIPO_FLOAT;
    if (strcmp(tipo_str, "bool") == 0) return TIPO_BOOL;
    if (strcmp(tipo_str, "string") == 0) return TIPO_STRING;
    
    return TIPO_INT;
}

const char* tipo_para_string(tipo_sensor_t tipo) {
    switch(tipo) {
        case TIPO_INT: return "int";
        case TIPO_FLOAT: return "float";
        case TIPO_BOOL: return "bool";
        case TIPO_STRING: return "string";
        default: return "unknown";
    }
}

char* gerar_valor_aleatorio(tipo_sensor_t tipo, char* buffer, size_t tamanho_buffer) {
    if (buffer == NULL || tamanho_buffer == 0) {
        return NULL;
    }
    
    switch(tipo) {
        case TIPO_INT:
            snprintf(buffer, tamanho_buffer, "%d", (rand() % 1100) - 100);
            break;
            
        case TIPO_FLOAT:
            snprintf(buffer, tamanho_buffer, "%.2f", (float)(rand() % 10000) / 100.0f);
            break;
            
        case TIPO_BOOL:
            strcpy(buffer, (rand() % 2) ? "true" : "false");
            break;
            
        case TIPO_STRING:
            {
                int len = 6 + (rand() % 5);
                if (len >= (int)tamanho_buffer) {
                    len = tamanho_buffer - 1;
                }
                
                for(int i = 0; i < len; i++) {
                    int tipo_char = rand() % 3;
                    if (tipo_char == 0) {
                        buffer[i] = 'A' + (rand() % 26);
                    } else if (tipo_char == 1) {
                        buffer[i] = 'a' + (rand() % 26);
                    } else {
                        buffer[i] = '0' + (rand() % 10);
                    }
                }
                buffer[len] = '\0';
            }
            break;
            
        default:
            strcpy(buffer, "ERROR");
            break;
    }
    
    return buffer;
}

int parsear_sensor_config(const char* config_str, sensor_config_t* config) {
    if (config_str == NULL || config == NULL) {
        return -1;
    }
    
    char config_copia[64];
    strncpy(config_copia, config_str, sizeof(config_copia) - 1);
    config_copia[sizeof(config_copia) - 1] = '\0';
    
    char* nome = strtok(config_copia, ":");
    char* tipo_str = strtok(NULL, ":");
    
    if (nome == NULL || tipo_str == NULL) {
        fprintf(stderr, "Erro: Formato invalido '%s'. Use NOME:tipo\n", config_str);
        return -1;
    }
    
    if (strlen(nome) >= sizeof(config->nome)) {
        fprintf(stderr, "Erro: Nome do sensor muito longo '%s'\n", nome);
        return -1;
    }
    
    strcpy(config->nome, nome);
    config->tipo = string_para_tipo(tipo_str);
    
    return 0;
}

long gerar_timestamp_aleatorio(long inicio, long fim) {
    if (fim <= inicio) {
        return inicio;
    }
    
    long intervalo = fim - inicio;
    return inicio + (rand() % intervalo);
}

int comparar_timestamps(const void* a, const void* b) {
    long ts_a = *(const long*)a;
    long ts_b = *(const long*)b;
    
    if (ts_a < ts_b) return -1;
    if (ts_a > ts_b) return 1;
    return 0;
}

int gerar_dados(long timestamp_inicio, long timestamp_fim, 
                sensor_config_t* sensores, int num_sensores, 
                int leituras_por_sensor, const char* arquivo_saida) {
    if (!arquivo_saida) {
        fprintf(stderr, "Erro: nome de arquivo de saída inválido\n");
        return -1;
    }
    FILE* arquivo = fopen(arquivo_saida, "w");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro ao criar arquivo de saida\n");
        return -1;
    }
    
    fprintf(arquivo, "# Dados de sensores gerados automaticamente\n");
    fprintf(arquivo, "# Formato: timestamp,sensor_id,valor\n");
    fprintf(arquivo, "# Periodo: %ld a %ld\n", timestamp_inicio, timestamp_fim);
    fprintf(arquivo, "# Sensores configurados:\n");
    
    for (int i = 0; i < num_sensores; i++) {
        fprintf(arquivo, "# - %s (%s)\n", sensores[i].nome, tipo_para_string(sensores[i].tipo));
    }
    
    fprintf(arquivo, "\n");
    
    int total_leituras = num_sensores * leituras_por_sensor;
    
    typedef struct {
        long timestamp;
        char sensor_id[32];
        char valor[32];
    } leitura_temp_t;
    
    leitura_temp_t* todas_leituras = malloc(total_leituras * sizeof(leitura_temp_t));
    if (todas_leituras == NULL) {
        fprintf(stderr, "Erro: Memoria insuficiente para %d leituras\n", total_leituras);
        fclose(arquivo);
        return -1;
    }      int indice_leitura = 0;
    
    for (int s = 0; s < num_sensores; s++) {
        for (int l = 0; l < leituras_por_sensor; l++) {
            leitura_temp_t* leitura = &todas_leituras[indice_leitura];
            
            leitura->timestamp = gerar_timestamp_aleatorio(timestamp_inicio, timestamp_fim);
            
            strcpy(leitura->sensor_id, sensores[s].nome);
            
            gerar_valor_aleatorio(sensores[s].tipo, leitura->valor, sizeof(leitura->valor));
            
            indice_leitura++;
        }    }
    
    for (int i = total_leituras - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        
        leitura_temp_t temp = todas_leituras[i];
        todas_leituras[i] = todas_leituras[j];
        todas_leituras[j] = temp;    }
    
    for (int i = 0; i < total_leituras; i++) {
        fprintf(arquivo, "%ld,%s,%s\n", 
                todas_leituras[i].timestamp,
                todas_leituras[i].sensor_id,
                todas_leituras[i].valor);
    }
      free(todas_leituras);
    todas_leituras = NULL;
    fclose(arquivo);
    
    printf("Arquivo '%s' criado com sucesso!\n", arquivo_saida);
    
    return 0;
}

int main(int argc, char* argv[]) {
    printf("=== Sistema de Monitoramento de Sensores Industriais ===\n");
    printf("Programa 3: Gerador de Dados\n\n");
    
    if (argc < 5) {
        printf("Uso: %s <timestamp_inicio> <timestamp_fim> <sensor1:tipo> [sensor2:tipo] ...\n", argv[0]);
        printf("\nExemplos:\n");
        printf("  %s 1640995200 1640999000 TEMP_01:float PRESS_02:int FLOW_03:bool\n", argv[0]);
        printf("  %s 1640995200 1641000000 TEMP_01:float TEMP_02:float HUMID_01:int STATUS_01:string\n", argv[0]);
        printf("\nTipos suportados: int, float, bool, string\n");
        return -1;
    }
    
    char* endptr;
    long timestamp_inicio = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || timestamp_inicio <= 0) {
        fprintf(stderr, "Erro: Timestamp de inicio invalido '%s'\n", argv[1]);
        return -1;
    }
    
    long timestamp_fim = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0' || timestamp_fim <= timestamp_inicio) {
        fprintf(stderr, "Erro: Timestamp de fim invalido '%s'\n", argv[2]);
        return -1;
    }
    
    int num_sensores = argc - 3;
    if (num_sensores > 50) {
        fprintf(stderr, "Erro: Muitos sensores (%d). Maximo suportado: 50\n", num_sensores);
        return -1;
    }
    
    sensor_config_t* sensores = malloc(num_sensores * sizeof(sensor_config_t));
    if (sensores == NULL) {
        fprintf(stderr, "Erro: Memoria insuficiente para %d sensores\n", num_sensores);
        return -1;
    }
      for (int i = 0; i < num_sensores; i++) {
        if (parsear_sensor_config(argv[i + 3], &sensores[i]) != 0) {
            free(sensores);
            return -1;
        }    }
      srand((unsigned int)time(NULL));
    
    const int leituras_por_sensor = 2000;

    int resultado = gerar_dados(
        timestamp_inicio,
        timestamp_fim,
        sensores,
        num_sensores,
        leituras_por_sensor,
        "dados_gerados.txt"
    );

    free(sensores);
    sensores = NULL;
    
    if (resultado != 0) {
        printf("Erro durante a geracao de dados.\n");
    }
    
    return resultado;
}
