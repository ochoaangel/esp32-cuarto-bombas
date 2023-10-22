#include <stdio.h>
#include <stdlib.h>

// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <driver/uart.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <driver/uart.h>

#define UART_NUM_1 1
#define UART_TXD_PIN 18
#define UART_RXD_PIN 19

static const char *TAG = "main";

// Variables de temperatura
float T1 = 25.0;
float T2 = 20.0;
float T3 = 15.0;

// Cola de mensajes
QueueHandle_t queue_mensajes;

// Cola de datos
QueueHandle_t queue_datos;

// Función para actualizar las variables de temperatura
void actualizar_temperaturas(void *arg) {
  while (1) {
    // Actualizar T1
    T1 += 0.1;

    // Actualizar T2
    T2 -= 0.2;

    // Actualizar T3
    T3 *= 0.5;

    // Leer el dato recibido por RS485
    char c = uart_read_byte(UART_NUM_1);

    // Añadir el dato recibido a la cola de mensajes
    xQueueSend(queue_mensajes, &c, portMAX_DELAY);

    // Dormir durante 1 segundo
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// Función para procesar los mensajes de la cola
void procesar_mensajes(void *arg) {
  while (1) {
    // Leer un mensaje de la cola
    char c;
    xQueueReceive(queue_mensajes, &c, portMAX_DELAY);

    // Analizar el mensaje recibido
    if (c == 'T1') {
      // Enviar el valor de T1
      uart_write_bytes(UART_NUM_1, (const char *)&T1, sizeof(T1));
    } else if (c == 'T2') {
      // Enviar el valor de T2
      uart_write_bytes(UART_NUM_1, (const char *)&T2, sizeof(T2));
    } else if (c == 'T3') {
      // Enviar el valor de T3
      uart_write_bytes(UART_NUM_1, (const char *)&T3, sizeof(T3));
    }
  }
}

// Función para enviar los datos de la cola de transmisión
void enviar_datos(void *arg) {
  while (1) {
    // Leer un dato de la cola de transmisión
    float temperatura;
    xQueueReceive(queue_datos, &temperatura, portMAX_DELAY);

    // Enviar el dato recibido
    uart_write_bytes(UART_NUM_1, (const char *)&temperatura, sizeof(temperatura));
  }
}

// Función principal
void app_main(void) {
  // Inicializar FreeRTOS
  xTaskCreate(actualizar_temperaturas, "actualizar_temperaturas", 2048, NULL, 5, NULL);

  // Inicializar la cola de mensajes
  queue_mensajes = xQueueCreate(10, sizeof(char));

  // Inicializar la cola de datos
  queue_datos = xQueueCreate(10, sizeof(float));

  // Iniciar la tarea para procesar los mensajes
  xTaskCreate(procesar_mensajes, "procesar_mensajes", 2048, NULL, 5, NULL);

  // Iniciar la tarea para enviar los datos
  xTaskCreate(enviar_datos, "enviar_datos", 2048, NULL, 5, NULL);

  // Esperar a que finalice la ejecución
  while (1) {
    vTaskDelay(portMAX_DELAY);
  }
}
