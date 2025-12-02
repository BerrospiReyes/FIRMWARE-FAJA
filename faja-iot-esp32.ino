#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "NOMBRE_DE_TU_WIFI";
const char* password = "TU_CONTRASEÑA_AQUI";

String serverUrl = "PON AQUI TU URL DE RENDER"; 

// PINES
#define ENA 26
#define IN1 27
#define IN2 14
#define LASER_S1 15
#define LASER_S2 4
#define LASER_S3 2
#define LDR_S1 34
#define LDR_S2 35
#define LDR_S3 32

int umbral = 1500;
bool motorFisicoOn = false; // Estado real del motor

// Variables de lógica de detección
int cPeq = 0, cMed = 0, cGra = 0;
String lastSize = "", lastBits = "", lastTime = "";
bool processing = false;
unsigned long procStart = 0;
bool acc_s1 = false, acc_s2 = false, acc_s3 = false;
unsigned long lastDetectMillis = 0;
unsigned long lastHttpSend = 0;

void setup() {
  Serial.begin(115200);
  
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  ledcSetup(0, 5000, 8); ledcAttachPin(ENA, 0);
  
  pinMode(LASER_S1, OUTPUT); pinMode(LASER_S2, OUTPUT); pinMode(LASER_S3, OUTPUT);
  digitalWrite(LASER_S1, HIGH); digitalWrite(LASER_S2, HIGH); digitalWrite(LASER_S3, HIGH);
  pinMode(LDR_S1, INPUT); pinMode(LDR_S2, INPUT); pinMode(LDR_S3, INPUT);

  WiFi.begin(ssid, pass);
  while(WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("WiFi Conectado!");
}

// Lógica de detección
void logicDetection(bool s1, bool s2, bool s3) {
   if(!processing){
    if((s1 || s2 || s3) && millis() - lastDetectMillis > 200){
      processing = true; procStart = millis();
      acc_s1 = s1; acc_s2 = s2; acc_s3 = s3;
    }
  } else {
    acc_s1 |= s1; acc_s2 |= s2; acc_s3 |= s3;
    if(millis() - procStart >= 300){
      String bits = String(acc_s1)+"|"+String(acc_s2)+"|"+String(acc_s3);
      String tipo = "";
      if(acc_s1 && !acc_s2 && !acc_s3) { tipo = "Pequeña"; cPeq++; }
      else if(acc_s1 && acc_s2 && !acc_s3) { tipo = "Mediana"; cMed++; }
      else if(acc_s1 && acc_s2 && acc_s3) { tipo = "Grande"; cGra++; }
      
      if(tipo != "") {
        lastSize = tipo; lastBits = bits;
        lastDetectMillis = millis();
      }
      processing = false;
    }
  }
}

void loop() {
  // 1. Leer Sensores
  bool s1 = analogRead(LDR_S1) > umbral;
  bool s2 = analogRead(LDR_S2) > umbral;
  bool s3 = analogRead(LDR_S3) > umbral;
  
  logicDetection(s1, s2, s3);

  // 2. Controlar Motor
  if(motorFisicoOn) {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); ledcWrite(0, 255);
  } else {
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); ledcWrite(0, 0);
  }

  // 3. Enviar a Render cada 500ms (aprox)
  if(millis() - lastHttpSend > 500) {
    if(WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverUrl + "/esp/update");
      http.addHeader("Content-Type", "application/json");

      // Crear JSON
      String json;
      StaticJsonDocument<512> doc;
      doc["pequenas"] = cPeq;
      doc["medianas"] = cMed;
      doc["grandes"] = cGra;
      
      JsonObject sens = doc.createNestedObject("sensors");
      sens["s1"] = s1; sens["s2"] = s2; sens["s3"] = s3;

      if(lastSize != "") {
        JsonObject det = doc.createNestedObject("lastDetection");
        det["size"] = lastSize; det["bits"] = lastBits;
        // Limpiamos para no enviar lo mismo siempre
        lastSize = ""; 
      }

      serializeJson(doc, json);
      
      // Enviar POST y recibir respuesta
      int httpResponseCode = http.POST(json);
      
      if(httpResponseCode > 0) {
        String response = http.getString();
        // Parsear respuesta para ver si prendemos motor
        StaticJsonDocument<200> respDoc;
        deserializeJson(respDoc, response);
        motorFisicoOn = respDoc["motorCommand"];
      }
      
      http.end();
    }
    lastHttpSend = millis();
  }
}