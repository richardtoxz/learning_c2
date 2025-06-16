#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    long timestamp;
    char sensor_id[32];
    char valor_str[32];
} leitura_t;

long labs_custom(long x) {
    return x < 0 ? -x : x;
}

int contar_linhas_validas(const char* nome_arquivo) {
    FILE* arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        return -1;
    }
    
    char linha[256];
    int count = 0;
    
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        if (linha[0] != '\n' && linha[0] != '#' && linha[0] != '\0') {
            count++;
        }
    }
    
    fclose(arquivo);
    return count;
}

leitura_t* carregar_dados_sensor(const char* nome_arquivo, int* tamanho) {
    *tamanho = 0;
    
    int num_linhas = contar_linhas_validas(nome_arquivo);
    if (num_linhas <= 0) {
        fprintf(stderr, "Erro: Arquivo vazio ou nao contem dados validos\n");
        return NULL;
    }
    
    leitura_t* leituras = malloc(num_linhas * sizeof(leitura_t));
    if (leituras == NULL) {
        fprintf(stderr, "Erro: Memoria insuficiente para %d leituras\n", num_linhas);
        return NULL;
    }
    
    FILE* arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        free(leituras);
        return NULL;
    }
    
    char linha[256];
    int indice = 0;
    
    while (fgets(linha, sizeof(linha), arquivo) != NULL && indice < num_linhas) {
        if (linha[0] == '\n' || linha[0] == '#' || linha[0] == '\0') {
            continue;
        }
        
        char* token = strtok(linha, ",");
        if (token == NULL) continue;
        
        leituras[indice].timestamp = strtol(token, NULL, 10);
        
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        
        strncpy(leituras[indice].sensor_id, token, sizeof(leituras[indice].sensor_id) - 1);
        leituras[indice].sensor_id[sizeof(leituras[indice].sensor_id) - 1] = '\0';
        
        token = strtok(NULL, ",\n\r");
        if (token == NULL) continue;
        
        strncpy(leituras[indice].valor_str, token, sizeof(leituras[indice].valor_str) - 1);
        leituras[indice].valor_str[sizeof(leituras[indice].valor_str) - 1] = '\0';
        
        indice++;
    }
      fclose(arquivo);
    *tamanho = indice;
    
    return leituras;
}

leitura_t* busca_binaria_aproximada(leitura_t* vetor, int tamanho, long timestamp_alvo) {
    if (vetor == NULL || tamanho <= 0) {
        return NULL;
    }      int esquerda = 0, direita = tamanho - 1;
    leitura_t* mais_proximo = NULL;
    long menor_diferenca = 999999999L;
    
    while (esquerda <= direita) {
        int meio = esquerda + (direita - esquerda) / 2;
        long timestamp_atual = vetor[meio].timestamp;
        long diferenca = labs_custom(timestamp_atual - timestamp_alvo);
        
        if (diferenca < menor_diferenca) {
            menor_diferenca = diferenca;
            mais_proximo = &vetor[meio];
        }
        
        if (timestamp_atual == timestamp_alvo) {
            break;
        } else if (timestamp_atual < timestamp_alvo) {
            esquerda = meio + 1;
        } else {
            direita = meio - 1;
        }
    }
    
    if (mais_proximo != NULL) {
        int indice_proximo = mais_proximo - vetor;
        
        if (indice_proximo > 0) {
            long diferenca_anterior = labs_custom(vetor[indice_proximo - 1].timestamp - timestamp_alvo);
            if (diferenca_anterior < menor_diferenca) {
                menor_diferenca = diferenca_anterior;
                mais_proximo = &vetor[indice_proximo - 1];
            }
        }
        
        if (indice_proximo < tamanho - 1) {
            long diferenca_posterior = labs_custom(vetor[indice_proximo + 1].timestamp - timestamp_alvo);
            if (diferenca_posterior < menor_diferenca) {
                menor_diferenca = diferenca_posterior;
                mais_proximo = &vetor[indice_proximo + 1];
            }        }
    }
    
    return mais_proximo;
}

int construir_nome_arquivo(const char* sensor_id, char* nome_arquivo, size_t tamanho) {
    const char* underscore = strchr(sensor_id, '_');
    if (underscore != NULL) {
        size_t len = underscore - sensor_id;
        if (len >= tamanho - 20) {
            return -1;
        }
        
        strncpy(nome_arquivo, sensor_id, len);
        nome_arquivo[len] = '\0';
        strcat(nome_arquivo, "_ordenado.txt");
    } else {
        if (strlen(sensor_id) >= tamanho - 20) {
            return -1;
        }
        strcpy(nome_arquivo, sensor_id);
        strcat(nome_arquivo, "_ordenado.txt");
    }
    
    return 0;
}

int consultar_sensor(const char* sensor_id, long timestamp_alvo) {
    char nome_arquivo[128];
    
    if (construir_nome_arquivo(sensor_id, nome_arquivo, sizeof(nome_arquivo)) != 0) {
        fprintf(stderr, "Erro: Nome do sensor muito longo\n");
        return -1;
    }
      
    FILE* test_file = fopen(nome_arquivo, "r");
    if (test_file == NULL) {        fprintf(stderr, "Erro: Arquivo '%s' nao encontrado\n", nome_arquivo);
        return -1;
    }
    fclose(test_file);
    
    int tamanho;
    leitura_t* leituras = carregar_dados_sensor(nome_arquivo, &tamanho);
      if (leituras == NULL || tamanho == 0) {
        fprintf(stderr, "Erro: Nao foi possivel carregar dados do sensor\n");
        return -1;
    }
    
    leitura_t* resultado = busca_binaria_aproximada(leituras, tamanho, timestamp_alvo);
      if (resultado != NULL) {
        long diferenca = labs_custom(resultado->timestamp - timestamp_alvo);
        
        printf("\n=== RESULTADO DA CONSULTA ===\n");
        printf("Sensor ID: %s\n", resultado->sensor_id);
        printf("Timestamp encontrado: %ld\n", resultado->timestamp);
        printf("Timestamp procurado: %ld\n", timestamp_alvo);
        printf("Diferenca: %ld segundos\n", diferenca);
        printf("Valor: %s\n", resultado->valor_str);
        
        int indice = resultado - leituras;
        printf("\n=== CONTEXTO ===\n");
        
        if (indice > 0) {
            printf("Anterior: %ld, %s, %s\n", 
                   leituras[indice-1].timestamp,
                   leituras[indice-1].sensor_id,
                   leituras[indice-1].valor_str);
        }
        
        printf(">>> ATUAL: %ld, %s, %s <<<\n", 
               resultado->timestamp, resultado->sensor_id, resultado->valor_str);
        
        if (indice < tamanho - 1) {
            printf("Proximo: %ld, %s, %s\n", 
                   leituras[indice+1].timestamp,
                   leituras[indice+1].sensor_id,
                   leituras[indice+1].valor_str);
        }
    } else {
        printf("\nNenhuma leitura encontrada para o sensor %s\n", sensor_id);
    }
    
    free(leituras);
    leituras = NULL;
    
    return resultado != NULL ? 0 : -1;
}

int main(int argc, char* argv[]) {
    printf("=== Sistema de Monitoramento de Sensores Industriais ===\n");
    printf("Programa 2: Consulta com Busca Binaria\n\n");
    
    if (argc != 3) {
        printf("Uso: %s <sensor_id> <timestamp>\n", argv[0]);
        printf("Exemplo: %s TEMP_01 1640995275\n", argv[0]);
        printf("Exemplo: %s PRESS_02 1640995500\n", argv[0]);
        return -1;
    }
    
    if (strlen(argv[1]) == 0 || strlen(argv[1]) >= 32) {
        fprintf(stderr, "Erro: ID do sensor invalido\n");
        return -1;
    }
    
    char* endptr;
    long timestamp_alvo = strtol(argv[2], &endptr, 10);
    
    if (*endptr != '\0' || timestamp_alvo <= 0) {
        fprintf(stderr, "Erro: Timestamp invalido '%s'\n", argv[2]);
        fprintf(stderr, "Use um numero inteiro positivo (Unix timestamp)\n");
        return -1;
    }
    
    printf("Procurando leitura mais proxima do timestamp %ld para sensor %s\n\n", 
           timestamp_alvo, argv[1]);
    
    int resultado = consultar_sensor(argv[1], timestamp_alvo);
    
    if (resultado == 0) {
        printf("\nConsulta realizada com sucesso!\n");
    } else {
        printf("\nErro durante a consulta.\n");
    }
    
    return resultado;
}
