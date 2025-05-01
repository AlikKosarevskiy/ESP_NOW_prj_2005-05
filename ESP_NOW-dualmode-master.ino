#include <esp_now.h>
#include <WiFi.h>
#include <DHT.h>

#define DHTPIN 4        // Пин подключения DHT22
#define DHTTYPE DHT22   // Тип датчика DHT22
DHT dht(DHTPIN, DHTTYPE);

#define LED_PIN 2       // Пин для светодиода
#define WIFI_CHANNEL 1  // Канал WiFi для ESP-NOW

esp_now_peer_info_t slave;
//uint8_t remoteMac[] = {0x10, 0x52, 0x1C, 0x66, 0xD5, 0xE4}; // MAC адрес слейва
uint8_t remoteMac[] = {0xCC, 0xBA, 0x97, 0x25, 0xF5, 0x80};    //cc:ba:97:25:f5:80
const uint8_t maxDataFrameSize = 250;
uint8_t dataToSend[maxDataFrameSize];

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Data Sent Successfully");
  } else {
    Serial.println("Data Send Failed");
  }
}
//void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
//  if (len == 1) {  
  //  digitalWrite(LED_PIN, data[0] ? HIGH : LOW);  // Включаем или выключаем светодиод
 
  if (len > 0) {
    uint8_t signal = data[0];
    digitalWrite(LED_PIN, signal ? HIGH : LOW);
  
  }
}
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());
  WiFi.disconnect();

  if (esp_now_init() == ESP_OK) {
    Serial.println("ESP NOW INIT!");
  } else {
    Serial.println("ESP NOW INIT FAILED....");
  }

  memcpy(&slave.peer_addr, &remoteMac, 6);
  slave.channel = WIFI_CHANNEL;
  slave.encrypt = 0;

  if (esp_now_add_peer(&slave) == ESP_OK) {
    Serial.println("Added Peer!");
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  // Инициализация датчика DHT
  dht.begin();
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Считывание данных с датчика DHT22
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Отправка данных (температура и влажность) на слейв
  dataToSend[0] = (uint8_t)temperature;
  dataToSend[1] = (uint8_t)humidity;

  esp_now_send(slave.peer_addr, dataToSend, 2);  // Отправляем 2 байта данных

  delay(2000);  // Задержка между отправками данных
}
