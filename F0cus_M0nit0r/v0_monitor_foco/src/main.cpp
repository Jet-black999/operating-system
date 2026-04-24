#include <Arduino.h>

// =========================
// Configuração de hardware
// =========================
constexpr int PIN_BTN_START_STOP = 4;   // pushbutton para iniciar/encerrar sessão
constexpr int PIN_SEAT_SWITCH    = 19;  // slide switch simulando sensor de presença/ocupação
constexpr int PIN_TRIG           = 5;   // HC-SR04 trig
constexpr int PIN_ECHO           = 18;  // HC-SR04 echo
constexpr int PIN_TEMP_ADC       = 34;  // potenciômetro simulando temperatura
constexpr int PIN_NOISE_ADC      = 35;  // potenciômetro simulando ruído
constexpr int PIN_LED_GREEN      = 25;
constexpr int PIN_LED_YELLOW     = 26;
constexpr int PIN_LED_RED        = 27;
constexpr int PIN_BUZZER         = 23;

// =========================
// Parâmetros do sistema
// =========================
constexpr float DIST_MIN_CM = 20.0f;
constexpr float DIST_MAX_CM = 70.0f;
constexpr uint16_t NOISE_WARN_THRESHOLD = 2500;
constexpr float TEMP_MIN_OK = 20.0f;
constexpr float TEMP_MAX_OK = 30.0f;
constexpr uint32_t SENSOR_PERIOD_MS = 500;
constexpr uint32_t UI_PERIOD_MS = 250;
constexpr uint32_t MIN_INTERRUPT_MS = 5000;
constexpr uint32_t LONG_ABSENCE_MS = 30000;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 250;

// =========================
// Tipos de dados
// =========================
enum class SystemState : uint8_t {
  BOOT,
  IDLE,
  SESSION_ACTIVE,
  SESSION_ENDED,
  ERROR
};

enum class AttentionState : uint8_t {
  UNKNOWN,
  FOCUSED,
  UNSTABLE,
  TEMPORARY_AWAY,
  ABSENT,
  ENV_WARNING
};

enum class ButtonEventType : uint8_t {
  TOGGLE_SESSION
};

struct ButtonEvent {
  ButtonEventType type;
  uint32_t timestampMs;
};

struct SensorSample {
  uint32_t timestampMs;
  bool seatPresent;
  float distanceCm;
  float temperatureC;
  uint16_t noiseRaw;
};

struct UiStatus {
  SystemState systemState;
  AttentionState attentionState;
  bool noisy;
  bool tempWarning;
  bool sessionActive;
};

struct SessionData {
  bool active;
  uint32_t startTimeMs;
  uint32_t endTimeMs;
  uint32_t totalPresentMs;
  uint32_t totalAbsentMs;
  uint32_t currentContinuousBlockMs;
  uint32_t longestContinuousBlockMs;
  uint32_t currentAbsenceMs;
  uint16_t interruptions;
  uint8_t focusScore;
  float tempMin;
  float tempMax;
  float tempSum;
  uint32_t tempSamples;
  uint32_t noisyWindows;
  uint32_t totalWindows;
  uint16_t noisePeak;
  AttentionState lastAttentionState;
};

// =========================
// Recursos RTOS
// =========================
QueueHandle_t qButtonEvents = nullptr;
QueueHandle_t qSensorSamples = nullptr; // fila de comprimento 1 para manter a amostra mais recente
QueueHandle_t qUiStatus = nullptr;
SemaphoreHandle_t mtxSession = nullptr;

volatile uint32_t gLastButtonInterruptMs = 0;
SystemState gSystemState = SystemState::BOOT;
SessionData gSessionData{};

// =========================
// Utilidades
// =========================
const char* toString(SystemState state) {
  switch (state) {
    case SystemState::BOOT: return "BOOT";
    case SystemState::IDLE: return "IDLE";
    case SystemState::SESSION_ACTIVE: return "SESSION_ACTIVE";
    case SystemState::SESSION_ENDED: return "SESSION_ENDED";
    case SystemState::ERROR: return "ERROR";
    default: return "UNKNOWN";
  }
}

const char* toString(AttentionState state) {
  switch (state) {
    case AttentionState::UNKNOWN: return "UNKNOWN";
    case AttentionState::FOCUSED: return "FOCUSED";
    case AttentionState::UNSTABLE: return "UNSTABLE";
    case AttentionState::TEMPORARY_AWAY: return "TEMPORARY_AWAY";
    case AttentionState::ABSENT: return "ABSENT";
    case AttentionState::ENV_WARNING: return "ENV_WARNING";
    default: return "UNKNOWN";
  }
}

float mapFloat(float x, float inMin, float inMax, float outMin, float outMax) {
  if (x <= inMin) return outMin;
  if (x >= inMax) return outMax;
  return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

float readDistanceCm() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  unsigned long duration = pulseIn(PIN_ECHO, HIGH, 30000UL);
  if (duration == 0) {
    return 999.0f; // timeout -> sem leitura válida
  }
  return (duration * 0.0343f) / 2.0f;
}

float readTemperatureC() {
  int raw = analogRead(PIN_TEMP_ADC);
  return mapFloat(static_cast<float>(raw), 0.0f, 4095.0f, 18.0f, 34.0f);
}

uint16_t readNoiseRaw() {
  return static_cast<uint16_t>(analogRead(PIN_NOISE_ADC));
}

void setLeds(bool green, bool yellow, bool red) {
  digitalWrite(PIN_LED_GREEN, green ? HIGH : LOW);
  digitalWrite(PIN_LED_YELLOW, yellow ? HIGH : LOW);
  digitalWrite(PIN_LED_RED, red ? HIGH : LOW);
}

void beep(uint16_t frequency = 2200, uint16_t durationMs = 80) {
  tone(PIN_BUZZER, frequency, durationMs);
}

void resetSessionData() {
  gSessionData = {};
  gSessionData.active = true;
  gSessionData.startTimeMs = millis();
  gSessionData.focusScore = 100;
  gSessionData.tempMin = 9999.0f;
  gSessionData.tempMax = -9999.0f;
  gSessionData.lastAttentionState = AttentionState::UNKNOWN;
}

void printSessionReport() {
  if (xSemaphoreTake(mtxSession, pdMS_TO_TICKS(100)) != pdTRUE) {
    Serial.println("[REPORT] Falha ao obter mutex da sessão.");
    return;
  }

  const uint32_t durationMs = gSessionData.endTimeMs - gSessionData.startTimeMs;
  const float durationMin = durationMs / 60000.0f;
  const float tempAvg = (gSessionData.tempSamples > 0)
                          ? (gSessionData.tempSum / static_cast<float>(gSessionData.tempSamples))
                          : 0.0f;
  const float noisyPercent = (gSessionData.totalWindows > 0)
                               ? (100.0f * static_cast<float>(gSessionData.noisyWindows) /
                                  static_cast<float>(gSessionData.totalWindows))
                               : 0.0f;

  Serial.println();
  Serial.println("================ RELATORIO DA SESSAO ================");
  Serial.printf("Duracao total: %.2f min\n", durationMin);
  Serial.printf("Tempo presente: %.2f min\n", gSessionData.totalPresentMs / 60000.0f);
  Serial.printf("Tempo ausente: %.2f min\n", gSessionData.totalAbsentMs / 60000.0f);
  Serial.printf("Interrupcoes: %u\n", gSessionData.interruptions);
  Serial.printf("Maior bloco continuo: %.2f min\n", gSessionData.longestContinuousBlockMs / 60000.0f);
  Serial.printf("Indice de foco: %u/100\n", gSessionData.focusScore);
  Serial.printf("Temperatura media: %.2f C\n", tempAvg);
  Serial.printf("Temperatura minima: %.2f C\n", gSessionData.tempMin);
  Serial.printf("Temperatura maxima: %.2f C\n", gSessionData.tempMax);
  Serial.printf("Ruido pico (raw): %u\n", gSessionData.noisePeak);
  Serial.printf("Tempo em ambiente ruidoso: %.2f %%\n", noisyPercent);
  Serial.println("====================================================");
  Serial.println();

  xSemaphoreGive(mtxSession);
}

// =========================
// ISR do botão
// =========================
void IRAM_ATTR onStartStopButton() {
  const uint32_t nowMs = millis();
  if ((nowMs - gLastButtonInterruptMs) < BUTTON_DEBOUNCE_MS) {
    return;
  }
  gLastButtonInterruptMs = nowMs;

  ButtonEvent evt{ButtonEventType::TOGGLE_SESSION, nowMs};
  BaseType_t higherPriorityWoken = pdFALSE;
  xQueueSendFromISR(qButtonEvents, &evt, &higherPriorityWoken);
  if (higherPriorityWoken == pdTRUE) {
    portYIELD_FROM_ISR();
  }
}

// =========================
// Tasks
// =========================
void taskSessionManager(void* parameter) {
  ButtonEvent evt{};
  while (true) {
    if (xQueueReceive(qButtonEvents, &evt, pdMS_TO_TICKS(100)) == pdTRUE) {
      if (evt.type == ButtonEventType::TOGGLE_SESSION) {
        if (xSemaphoreTake(mtxSession, pdMS_TO_TICKS(100)) == pdTRUE) {
          if (!gSessionData.active) {
            resetSessionData();
            gSystemState = SystemState::SESSION_ACTIVE;
            Serial.printf("[SESSION] Sessao iniciada em %lu ms\n", gSessionData.startTimeMs);
            beep(2500, 100);
          } else {
            gSessionData.active = false;
            gSessionData.endTimeMs = millis();
            gSystemState = SystemState::SESSION_ENDED;
            Serial.printf("[SESSION] Sessao encerrada em %lu ms\n", gSessionData.endTimeMs);
            beep(1600, 200);
          }
          xSemaphoreGive(mtxSession);
        }

        if (gSystemState == SystemState::SESSION_ENDED) {
          printSessionReport();
          gSystemState = SystemState::IDLE;
        }
      }
    }
  }
}

void taskSensorService(void* parameter) {
  SensorSample sample{};

  while (true) {
    sample.timestampMs = millis();
    sample.seatPresent = (digitalRead(PIN_SEAT_SWITCH) == HIGH);
    sample.distanceCm = readDistanceCm();
    sample.temperatureC = readTemperatureC();
    sample.noiseRaw = readNoiseRaw();

    xQueueOverwrite(qSensorSamples, &sample);

    vTaskDelay(pdMS_TO_TICKS(SENSOR_PERIOD_MS));
  }
}

void taskFocusEvaluator(void* parameter) {
  SensorSample sample{};
  UiStatus ui{};

  while (true) {
    if (xQueuePeek(qSensorSamples, &sample, pdMS_TO_TICKS(100)) == pdTRUE) {
      if (xSemaphoreTake(mtxSession, pdMS_TO_TICKS(100)) == pdTRUE) {
        ui.systemState = gSystemState;
        ui.sessionActive = gSessionData.active;

        if (!gSessionData.active) {
          ui.attentionState = AttentionState::UNKNOWN;
          ui.noisy = false;
          ui.tempWarning = false;
          xSemaphoreGive(mtxSession);
          xQueueOverwrite(qUiStatus, &ui);
          vTaskDelay(pdMS_TO_TICKS(SENSOR_PERIOD_MS));
          continue;
        }

        const bool distanceOk = sample.distanceCm >= DIST_MIN_CM && sample.distanceCm <= DIST_MAX_CM;
        const bool presentNow = sample.seatPresent && distanceOk;
        const bool noisy = sample.noiseRaw >= NOISE_WARN_THRESHOLD;
        const bool tempWarning = (sample.temperatureC < TEMP_MIN_OK || sample.temperatureC > TEMP_MAX_OK);

        gSessionData.totalWindows++;
        if (noisy) {
          gSessionData.noisyWindows++;
        }
        if (sample.noiseRaw > gSessionData.noisePeak) {
          gSessionData.noisePeak = sample.noiseRaw;
        }

        gSessionData.tempSum += sample.temperatureC;
        gSessionData.tempSamples++;
        if (sample.temperatureC < gSessionData.tempMin) {
          gSessionData.tempMin = sample.temperatureC;
        }
        if (sample.temperatureC > gSessionData.tempMax) {
          gSessionData.tempMax = sample.temperatureC;
        }

        AttentionState newAttentionState = AttentionState::UNKNOWN;

        if (presentNow) {
          gSessionData.totalPresentMs += SENSOR_PERIOD_MS;
          gSessionData.currentContinuousBlockMs += SENSOR_PERIOD_MS;

          if (gSessionData.currentContinuousBlockMs > gSessionData.longestContinuousBlockMs) {
            gSessionData.longestContinuousBlockMs = gSessionData.currentContinuousBlockMs;
          }

          if (gSessionData.currentAbsenceMs >= MIN_INTERRUPT_MS) {
            gSessionData.interruptions++;
            if (gSessionData.focusScore >= 8) {
              gSessionData.focusScore -= 8;
            } else {
              gSessionData.focusScore = 0;
            }
            Serial.printf("[FOCUS] Interrupcao registrada. Total=%u\n", gSessionData.interruptions);
            beep(1900, 120);
          }
          gSessionData.currentAbsenceMs = 0;

          if (tempWarning || noisy) {
            newAttentionState = AttentionState::ENV_WARNING;
          } else if (sample.distanceCm < DIST_MIN_CM + 4.0f || sample.distanceCm > DIST_MAX_CM - 4.0f) {
            newAttentionState = AttentionState::UNSTABLE;
          } else {
            newAttentionState = AttentionState::FOCUSED;
          }
        } else {
          gSessionData.totalAbsentMs += SENSOR_PERIOD_MS;
          gSessionData.currentAbsenceMs += SENSOR_PERIOD_MS;
          gSessionData.currentContinuousBlockMs = 0;

          if (gSessionData.focusScore > 0) {
            gSessionData.focusScore = max(0, static_cast<int>(gSessionData.focusScore) - 1);
          }

          if (gSessionData.currentAbsenceMs >= LONG_ABSENCE_MS) {
            newAttentionState = AttentionState::ABSENT;
          } else {
            newAttentionState = AttentionState::TEMPORARY_AWAY;
          }
        }

        if (newAttentionState != gSessionData.lastAttentionState) {
          Serial.printf("[STATE] %s -> %s | seat=%d dist=%.1f temp=%.1f noise=%u\n",
                        toString(gSessionData.lastAttentionState),
                        toString(newAttentionState),
                        sample.seatPresent,
                        sample.distanceCm,
                        sample.temperatureC,
                        sample.noiseRaw);
          gSessionData.lastAttentionState = newAttentionState;
        }

        ui.attentionState = newAttentionState;
        ui.noisy = noisy;
        ui.tempWarning = tempWarning;
        xSemaphoreGive(mtxSession);
        xQueueOverwrite(qUiStatus, &ui);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(SENSOR_PERIOD_MS));
  }
}

void taskUiAndLogger(void* parameter) {
  UiStatus ui{};
  bool lastNoisy = false;
  bool lastTempWarning = false;

  while (true) {
    if (xQueuePeek(qUiStatus, &ui, pdMS_TO_TICKS(UI_PERIOD_MS)) == pdTRUE) {
      if (!ui.sessionActive) {
        setLeds(false, false, false);
      } else {
        switch (ui.attentionState) {
          case AttentionState::FOCUSED:
            setLeds(true, false, false);
            break;
          case AttentionState::UNSTABLE:
          case AttentionState::ENV_WARNING:
            setLeds(false, true, false);
            break;
          case AttentionState::TEMPORARY_AWAY:
          case AttentionState::ABSENT:
            setLeds(false, false, true);
            break;
          default:
            setLeds(false, false, false);
            break;
        }
      }

      if (ui.noisy && !lastNoisy) {
        Serial.println("[ENV] Ambiente ruidoso detectado.");
      }
      if (ui.tempWarning && !lastTempWarning) {
        Serial.println("[ENV] Temperatura fora da faixa desejada.");
      }

      lastNoisy = ui.noisy;
      lastTempWarning = ui.tempWarning;
    }

    vTaskDelay(pdMS_TO_TICKS(UI_PERIOD_MS));
  }
}

// =========================
// Setup / loop
// =========================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n[V0] Monitor de Sessao de Estudo - Inicializando...");

  pinMode(PIN_BTN_START_STOP, INPUT_PULLUP);
  pinMode(PIN_SEAT_SWITCH, INPUT_PULLUP);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  analogReadResolution(12);
  setLeds(false, false, false);

  qButtonEvents = xQueueCreate(8, sizeof(ButtonEvent));
  qSensorSamples = xQueueCreate(1, sizeof(SensorSample));
  qUiStatus = xQueueCreate(1, sizeof(UiStatus));
  mtxSession = xSemaphoreCreateMutex();

  if (!qButtonEvents || !qSensorSamples || !qUiStatus || !mtxSession) {
    gSystemState = SystemState::ERROR;
    Serial.println("[BOOT] Falha ao criar recursos RTOS.");
    setLeds(false, false, true);
    while (true) {
      delay(1000);
    }
  }

  gSystemState = SystemState::IDLE;
  gSessionData.active = false;

  attachInterrupt(digitalPinToInterrupt(PIN_BTN_START_STOP), onStartStopButton, FALLING);

  xTaskCreatePinnedToCore(taskSessionManager, "SessionManager", 4096, nullptr, 3, nullptr, 1);
  xTaskCreatePinnedToCore(taskSensorService, "SensorService", 4096, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(taskFocusEvaluator, "FocusEvaluator", 4096, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(taskUiAndLogger, "UiLogger", 3072, nullptr, 1, nullptr, 0);

  Serial.println("[BOOT] Sistema em IDLE. Pressione o botao para iniciar a sessao.");
}

void loop() {
  // Em projetos com FreeRTOS na ESP32, o trabalho principal fica nas tasks.
  vTaskDelay(pdMS_TO_TICKS(1000));
}
