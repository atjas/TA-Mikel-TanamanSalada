#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Sukidz";          // Nama jaringan Wi-Fi Anda
const char* password = "Sorehari22";  // Kata sandi Wi-Fi Anda
#define mqttserver "broker.hivemq.com"
#define mqttport 1883
const int soilMoisturePin = 4;  // Pin analog untuk sensor kelembaban tanah
const int relayPin = 25;        // Pin untuk mengontrol relay (pompa air)
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 60000;  // Interval pengiriman data dalam milidetik (misalnya, setiap 60 detik)

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting To WiFi");
  }

  Serial.println("Connected");
  client.setServer(mqttserver, mqttport);
  Serial.println(WiFi.localIP());

  pinMode(relayPin, OUTPUT);    // Set pin relay sebagai output
  digitalWrite(relayPin, LOW);  // Inisialisasi relay dalam keadaan mati
  //pinMode(soilMoisturePin, INPUT);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Coba Connect ke MQTT.... ");
    if (client.connect("client_sister")) {
      Serial.print("Connected");
    } else {
      Serial.println("Trying again");
      delay(5000);
    }
  }
}



void loop() {
  char hasil[4];
  int soilMoisture = analogRead(soilMoisturePin);

  


  // Konversi nilai kelembaban ke dalam rentang 0-100%
  int moisturePercentage = map(soilMoisture, 0, 4095, 0, 100);

  Serial.println("Nilai Sensor : " + String(soilMoisture) + "  - ");

  // Print nilai kelembaban ke Serial Monitor
  // Serial.print("Kelembaban Tanah: ");
  // Serial.print(moisturePercentage);
  // Serial.println("%");

  // Kontrol relay berdasarkan nilai kelembaban
  if (moisturePercentage < 60) {
    digitalWrite(relayPin, HIGH);  // Aktifkan relay (nyalakan pompa air)
  } else if (moisturePercentage > 75) {
    digitalWrite(relayPin, LOW);  // Matikan relay (matikan pompa air)
  }

  // Konversi nilai kelembaban menjadi string
  dtostrf(moisturePercentage, 1, 2, hasil);

  // Kirim data kelembaban ke broker MQTT
  client.publish("michaelesp", hasil);

  // Hitung dan tampilkan presentase waktu pengiriman data
  unsigned long currentTime = millis();
  if (currentTime - lastSendTime >= sendInterval) {
    float percentage = ((float)(currentTime - lastSendTime) / (float)sendInterval) * 100;
    Serial.print("Waktu Pengiriman Data: ");
    Serial.print(percentage);
    Serial.println("%");
    lastSendTime = currentTime;
  }

  // Reconnect ke broker MQTT jika tidak terhubung
  if (!client.connected()) {
    reconnect();
  }

  delay(500);
}
