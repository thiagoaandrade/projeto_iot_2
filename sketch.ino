#include <Keypad.h>
#include <ESP32Servo.h>
#include <LiquidCrystal.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>  // Cliente seguro para HTTPS
#include <HTTPClient.h>

// Configuração do display LCD
LiquidCrystal lcd(12, 14, 27, 26, 25, 33);  // RS, E, D4, D5, D6, D7

// Configuração do teclado numérico
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {16, 17, 18, 19};
byte colPins[COLS] = {25, 26, 27, 14};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// API para onde enviar os dados
const char* apiEndpoint = "http://adfe-45-189-166-207.ngrok-free.app/api/login";  // HTTP em vez de HTTPS

// Servo motor
Servo myServo;

String senhaInserida = "";

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);  
  lcd.print("Conectando WiFi...");

  // Conectando à rede WiFi
  WiFi.begin("Wokwi-GUEST", "", 6);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando...");
    lcd.setCursor(0, 1);
    lcd.print(".");
  }

  Serial.println("WiFi conectado!");
  lcd.clear();
  lcd.print("WiFi conectado");
  delay(2000);
  lcd.clear();
  lcd.print("Insira a senha:");

  // Verificando conectividade com a internet
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi Conectado, verificando conectividade...");
    WiFiClient client;
    if (!client.connect("www.google.com", 80)) {
      Serial.println("Falha ao conectar ao Google. Verifique sua conexão!");
    } else {
      Serial.println("Conectividade OK");
    }
  } else {
    Serial.println("Não foi possível conectar ao WiFi");
  }

  // Servo motor
  myServo.attach(13);  // Conecta o servo ao GPIO 13
  myServo.write(0);    // Inicializa com a porta fechada
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    if (key == '#') {
      enviarSenhaAPI(senhaInserida);
      senhaInserida = "";  
      lcd.clear();
      lcd.print("Insira a senha:");
    } else if (key == '*') {
      senhaInserida = "";
      lcd.clear();
      lcd.print("Senha apagada.");
      delay(2000);
      lcd.clear();
      lcd.print("Insira a senha:");
    } else {
      senhaInserida += key;
      lcd.clear();
      lcd.print("Senha: ");
      lcd.print(senhaInserida);  
      Serial.print("Senha inserida: ");
      Serial.println(senhaInserida);
    }
  }
}

void enviarSenhaAPI(String senha) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClient client;

    http.begin(client, apiEndpoint);  // HTTP ao invés de HTTPS

    http.addHeader("Content-Type", "application/json");

    String postData = "{\"password\":\"" + senha + "\"}";
    Serial.println("Enviando senha: " + postData);

    http.setTimeout(15000);  // Aumenta o tempo de espera para 15 segundos

    int httpResponseCode = http.POST(postData);

    Serial.println("Código de resposta HTTP: " + String(httpResponseCode));

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Resposta da API:");
      Serial.println(response);
      lcd.clear();
      lcd.print("Senha enviada");
    } else {
      Serial.println("Erro ao enviar: " + String(http.errorToString(httpResponseCode)));
      lcd.clear();
      lcd.print("Erro no envio");
    }

    http.end();
  } else {
    Serial.println("Erro na conexão WiFi");
    lcd.clear();
    lcd.print("Erro WiFi");
  }
}
