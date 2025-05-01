#include <WiFi.h>
#include <esp_now.h>
#include <TFT_eSPI.h>  // Убедись, что TFT_eSPI настроен для T-Display S3

#define BUTTON_PIN 0     // Встроенная кнопка
#define SCREEN_LED_PIN  38 // Подсветка экрана — светодиод, который будет гореть всегда
#define BLINK_LED_PIN    39 // Светодиод для мигания при получении данных
#define WIFI_CHANNEL 1

TFT_eSPI tft = TFT_eSPI();  // Объект дисплея

uint8_t masterMac[] = {0x40, 0x91, 0x51, 0x9F, 0x17, 0xEC};  // MAC-адрес мастера
esp_now_peer_info_t master;

void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  if (len < 2) return;

  float temperature = data[0];
  float humidity = data[1];

  // Очищаем экран перед выводом новых данных
  tft.fillScreen(TFT_TRANSPARENT); // Зеленый фон для экрана

  // Перезаписываем значения температуры и влажности
  tft.setTextSize(2);
  
  // Цвет для температуры
  tft.setCursor(10, 10);
  tft.setTextColor(temperature > 25.0 ? TFT_RED : TFT_BLUE); // Красный для высоких температур, синий для низких
  tft.print("Temp: ");
  tft.print(temperature);
  tft.println(" C");

  // Цвет для влажности
  tft.setCursor(10, 40);
  tft.setTextColor(humidity > 60.0 ? TFT_CYAN : TFT_WHITE); // Бирюзовый для высокой влажности, белый для низкой
  tft.print("Humidity: ");
  tft.print(humidity);
  tft.println(" %");

  // Мигаем светодиодом для уведомления о получении данных
  digitalWrite(BLINK_LED_PIN, HIGH);   // включить светодиод
  delay(100);                         // задержка
  digitalWrite(BLINK_LED_PIN, LOW);    // выключить светодиод
}

void setup() {
  Serial.begin(115200);

  // Дисплей
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_TRANSPARENT);  // Зеленый фон при запуске
  tft.setTextColor(TFT_WHITE); // Белый текст по умолчанию
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Waiting for data...");

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SCREEN_LED_PIN, OUTPUT);  // Подсветка экрана
  pinMode(BLINK_LED_PIN, OUTPUT);   // Светодиод для мигания

  // Включаем подсветку экрана
  digitalWrite(SCREEN_LED_PIN, HIGH);

  // WiFi и ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();  // на всякий случай

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  memcpy(master.peer_addr, masterMac, 6);
  master.channel = WIFI_CHANNEL;
  master.encrypt = false;

  if (esp_now_add_peer(&master) != ESP_OK) {
    Serial.println("Failed to add peer");
  }
}

void loop() {
  static uint8_t lastState = HIGH;
  uint8_t currentState = digitalRead(BUTTON_PIN);

  if (currentState != lastState) {
    lastState = currentState;

    uint8_t msg = (currentState == LOW) ? 1 : 0;  // LOW — кнопка нажата, HIGH — отпущена
    esp_now_send(master.peer_addr, &msg, sizeof(msg));
  }

  lastState = currentState;
  delay(50);
}
