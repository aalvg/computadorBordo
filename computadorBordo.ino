
#include <Time.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Adafruit_AHTX0.h>

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

byte a[8] = {B00110, B01001, B00110, B00000, B00000, B00000, B00000, B00000};

Adafruit_AHTX0 aht;

const int buttonPin = 3;
int buttonState = HIGH;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
bool inVoltmeterScreen = false;
int currentScreen = 0; // 0 para a tela do RTC, 1 para a tela do voltímetro

// Definição da função
time_t getTeensy3Time() {
  return rtc.now().unixtime();
}

//...

void setup() {
  Serial.begin(9600); // Inicializa a porta serial
  rtc.begin();
  Wire.begin();
  lcd.backlight(); // Liga o backlight do LCD
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  if (rtc.lostPower()) {
    Serial.println("RTC perdeu energia, ajuste a data e hora!");
    // Aqui você pode definir a data e hora padrão após o RTC perder energia
    rtc.adjust(DateTime(2023, 8, 12, 16, 47, 41));
  }

  lcd.begin(16, 2);

  // Verifica o horário atual para decidir se é "Bom dia", "Boa tarde" ou "Boa noite"
  DateTime now = rtc.now();
  int currentHour = now.hour();
  lcd.setCursor(3, 0);
  if (currentHour >= 0 && currentHour < 12) {
    lcd.print("Bom dia!");
  } else if (currentHour >= 12 && currentHour < 18) {
    lcd.print("Boa tarde!");
  } else {
    lcd.print("Boa noite!");
  }

  lcd.setCursor(0, 3);
  lcd.print("Uno Mille");
  delay(4000);
  lcd.clear();

  // Configuração do botão
  pinMode(buttonPin, INPUT_PULLUP);

  // Inicialização do módulo AHT21
  if (!aht.begin()) {
    Serial.println("Falha ao iniciar o AHT21!");
  }
}

//...


void showVoltmeterScreen() {
  const int pin_div = A0; // Definição do pino analógico para a leitura da tensão
  float R1 = 30000.0; // Valor do resistor R1 para o cálculo da tensão
  float R2 = 10000.0; // Valor do resistor R2 para o cálculo da tensão

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Voltimetro CC");
  
  while (inVoltmeterScreen) {
    float v_lido = analogRead(pin_div);
    float v_medido = (5 * v_lido * (R1 + R2)) / (R2 * 1023);

    lcd.setCursor(0, 1);
    lcd.print("Vcc = ");
    lcd.print(v_medido, 2); // Exibe o valor com 2 casas decimais
    lcd.print(" V");

    delay(100); // Pequeno atraso para evitar atualizações rápidas demais no display

    // Check for button press to exit the Voltmeter screen
    int reading = digitalRead(buttonPin);
    if (reading != lastButtonState) {
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading != buttonState) {
        buttonState = reading;

        if (buttonState == LOW) {
          inVoltmeterScreen = false;
          lcd.clear();
          return; // Return to the main loop when exiting the Voltmeter screen
        }
      }
    }

    lastButtonState = reading;
  }
}

void showTemperatureAndHumidity() {
  lcd.clear();
  lcd.setCursor(0, 0);
  
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  lcd.print(temp.temperature, 2); // Exibe a temperatura com 2 casas decimais
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Umidade: ");
  lcd.print(humidity.relative_humidity, 2); // Exibe a umidade com 2 casas decimais
  lcd.print(" %");
}



//...

void loop() {
  // Leitura do estado do botão com debounce
  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        // Se o botão foi pressionado, alterna entre as telas do RTC e do voltímetro
        currentScreen = (currentScreen + 1) % 2;
        inVoltmeterScreen = currentScreen == 1;

        if (inVoltmeterScreen) {
          lcd.clear();
          lcd.print("Voltimetro CC");
          // Mostrar o restante da tela do voltímetro aqui, se houver.
          showVoltmeterScreen();
        } else {
          lcd.clear();
        }
      }
    }
  }

  lastButtonState = reading;

  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

//...

if (!inVoltmeterScreen) {
  // Tela do RTC
  static time_t tLast;
  time_t t;
  DateTime now = rtc.now();
  int dia = now.day();
  int mes = now.month();
  int ano = now.year();
  int hora = now.hour();
  int minuto = now.minute();
  int segundo = now.second();
  t = now.unixtime(); // Correção aqui para atualizar a variável t com o valor de now.unixtime()
  if (t != tLast) {
    tLast = t;
    lcd.setCursor(0, 0);
    if (dia < 10) lcd.print("0");
    lcd.print(dia);
    lcd.print("/");
    if (mes < 10) lcd.print("0");
    lcd.print(mes);
    lcd.print("/");
    lcd.print(ano);
    lcd.print(" ");
    lcd.print(temp.temperature, 1);// Exibe a temperatura com 1 casa decimal
    lcd.print("c");
    lcd.createChar(1, a);
    lcd.setCursor(15, 0);
    lcd.write(1);
    lcd.setCursor(0, 1);
    if (hora < 10) lcd.print("0");
    lcd.print(hora);
    lcd.print(":");
    if (minuto < 10) lcd.print("0");
    lcd.print(minuto);
    lcd.print(":");
    if (segundo < 10) lcd.print("0");
    lcd.print(segundo);
    lcd.print(" ");
    lcd.print("Ar");
    lcd.print(" ");
    lcd.print(humidity.relative_humidity, 1); // Exibe a umidade com 1 casa decimal
    lcd.print(" %");
  }
  delay(1000); // Atualiza a cada segundo
}
}

//...


// Função para ajustar data e hora via Serial Monitor
void adjustDateTime() {
  String data;
  String hora;
  int dia, mes, ano, hora_h, minuto, segundo;

  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n') break;
    data += c;
  }
  dia = data.substring(0, 2).toInt();
  mes = data.substring(3, 5).toInt();
  ano = data.substring(6).toInt();

  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n') break;
    hora += c;
  }
  hora_h = hora.substring(0, 2).toInt();
  minuto = hora.substring(3, 5).toInt();
  segundo = hora.substring(6).toInt();

  rtc.adjust(DateTime(ano, mes, dia, hora_h, minuto, segundo));

  Serial.println("Data e hora ajustadas com sucesso!");
}
