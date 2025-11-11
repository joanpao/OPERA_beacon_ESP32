// Programa original de Radio Club 'Los Charranes' - AERIS AMICI - (Amigos en las Ondas)
// para decodificar el protocolo OPERA desarrollado por Jose Alberto Nieto Ros, EA5HVK
// Modificado por EA5JTT 20251108:
// - Adaptado a la placa de desarrollo ESP32:  Lilygo T3_V1.6.1 ESP32  (Compilar con TTGO LoRa32-OLED en Arduino IDE)
// - Se incluir la salida por OLED a la SERIE
// - Se suprimir la parte de beacon CW
// - Se crera un carrusel de bandas cada una con su modo OPERA recomendado, 1' de espera entre unos y otros
/* Modos recomendados para cada bannda
Modos Tiempos F.Portadora (RX USB= Fp - 1,5 kHz)
05    0,128s  144,181; 432,0915 y 1296,6015  MHz
1     0,256s  21.075,5; 24.926,5; 28.071,5; 50.701,5 y 70.094,5 kHz
2     0,512s  3.548,5; 5.290,5; 7.039,5; 10.136,5 y 14.063,5 kHz
4     1,024s  1.837,5 kHz
8     2,048s  477 kHz
32    8,192s  136 kHz
65   16,384s
2H   32,768s
*/
/* Tabla maestra de frecuencias y modos
Campo	  Ejemplo	              Qué significa
freq	  183700000ULL	        Frecuencia de transmisión (en centesimas de Hz)
label	  "160m"	              Texto descriptivo de la banda (para mostrar en OLED)
mode	  "OPERA32"	            Modo OPERA recomendado para esa banda
time    512                   tiempo del pulso en funcion del modo 
txTime	32UL * 60UL * 1000UL	Duración del ciclo de transmisión (en milisegundos)
pause	  2UL * 60UL * 1000UL	  Pausa entre transmisiones (en milisegundos)

Ejemplo de Tabla de bandas configuradas
OperaBand bands[] = {
  {183700000ULL, "160m", "OPERA32", 8192, 32UL * 60UL * 1000UL, 2UL * 60UL * 1000UL},
  {354850000ULL, "80m",  "OPERA16", 4096, 16UL * 60UL * 1000UL, 2UL * 60UL * 1000UL},
  {528960000ULL, "60m",  "OPERA8",  2048, 8UL  * 60UL * 1000UL, 2UL * 60UL * 1000UL},
  {703628700ULL, "40m",  "OPERA4",  1024, 4UL  * 60UL * 1000UL, 1UL * 60UL * 1000UL},
  {1013560000ULL,"30m",  "OPERA2",  512,  2UL  * 60UL * 1000UL, 1UL * 60UL * 1000UL},
  {1406221000ULL,"20m",  "OPERA1",  256,  1UL  * 60UL * 1000UL, 1UL * 60UL * 1000UL}
};
*/

#include <si5351.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"
#include "Fixed8x16.h"

Si5351 si5351;

// PINOUT OLED
#define OLED_RST NOT_A_PIN
#define OLED_SDA 21
#define OLED_SCL 22
Adafruit_SSD1306 display(OLED_RST);

// Estructura para cada banda OPERA
struct OperaBand {
  uint64_t freq;           // Frecuencia (Hz)
  const char* bandName;    // Nombre banda
  const char* mode;        // Modo OPERA (OPERA1, 2, 4, etc.)
  unsigned long bitPeriod; // Duración de bit (ms)
  unsigned long txDuration; // Tiempo total transmisión (ms)
  unsigned long pause;      // Pausa entre ciclos (ms)
};
// Estructura con las variables informadas, la frecuencia se obtien tras la calibracion del SI5351
OperaBand bands[] = {
  // {183450000ULL,  "160m","OPERA4", 1024, 4UL  * 60UL * 1000UL, 1UL * 60UL * 1000UL}, //SI5351 no saca apenas señal 
  // {354530000ULL,  "80m", "OPERA2", 512,  2UL  * 60UL * 1000UL, 1UL * 60UL * 1000UL}, //SI5351 no saca apenas señal 
   {703881900ULL,  "40m", "OPERA2", 512,  2UL  * 60UL * 1000UL, 1UL * 60UL * 1000UL}, 
   {1013558000ULL, "30m", "OPERA2", 512,  2UL  * 60UL * 1000UL, 1UL * 60UL * 1000UL},
   {1406221000ULL, "20m", "OPERA2", 512,  2UL  * 60UL * 1000UL, 1UL * 60UL * 1000UL},
   {2107350000ULL, "15m", "OPERA1", 256,  1UL  * 60UL * 1000UL, 1UL * 60UL * 1000UL},
   {2492400000ULL, "13m", "OPERA1", 256,  1UL  * 60UL * 1000UL, 1UL * 60UL * 1000UL},
   {2806850000ULL, "10m", "OPERA1", 256,  1UL  * 60UL * 1000UL, 1UL * 60UL * 1000UL}
   };

const uint8_t NUM_BANDS = sizeof(bands) / sizeof(bands[0]);

// Estado de transmisión
uint8_t currentBand = 0;
unsigned long bitTimer = 0;
unsigned long stateTimer = 0;
unsigned int bitIndex = 0;

enum State { SEND_BITS, PAUSE };
State state = SEND_BITS;

// Secuencia OPERA (la misma para todos los modos, se obtiene para una estación determinada de la opción PIC del programa OPERA https://rosmodem.wordpress.com/)
const char bitSequence[] =
"10110101010010101011010101001010110011001101001011001100110101001011010100110101010011010011010011010100110010101011001011010100101101010101001010110011010101010011001011010101001011001011010011001010101100110100101011010011001100110101010";
const unsigned int bitLen = sizeof(bitSequence) - 1;

// ------------------------------------------------------------
// Funciones básicas
// ------------------------------------------------------------
// Se debe seleccionar la salida del SI5351 usada: CLK0. CLK1, CLK2
// ON y OFF
void carrierOn() {
  si5351.set_freq(bands[currentBand].freq, SI5351_CLK2);
  si5351.output_enable(SI5351_CLK2, 1);
}
void carrierOff() {
  si5351.output_enable(SI5351_CLK2, 0);
}

void showInfo() {
  // Pantalla
  display.clearDisplay();
  display.setFont(&Fixed8x16);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.print(bands[currentBand].bandName);
  display.print(" ");
  display.println(bands[currentBand].mode);
  display.setCursor(0, 30);
  display.print("Frec: ");
  display.println((double)bands[currentBand].freq / 1e8, 4);
  display.display();
  // Salida serie
  Serial.print("[Banda: ");
  Serial.print(bands[currentBand].bandName);
  Serial.print("]  ");
  Serial.print(bands[currentBand].mode);
  Serial.print("  Frec: ");
  Serial.print((double)bands[currentBand].freq / 1e8, 4);
  Serial.println(" MHz");
}

// Cambia de banda automáticamente
void nextBand() {
  currentBand++;
  if (currentBand >= NUM_BANDS) currentBand = 0;
  showInfo();
}

// ------------------------------------------------------------
// SETUP
// ------------------------------------------------------------
void setup() {
  // inicializa salida serie
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n[OPERA MultiBand Beacon] Inicializando...");
  // Inicializa Si5351
  // comparte PINOUT con OLED 21 SDA, 22 SDL, 3.3V o 5V y GND
  if (!si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0)) {
    Serial.println("[ERROR] No se detectó el Si5351.");
    while (1);
  }
  Serial.println("✅ Si5351 listo.");
  // Incializa OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, true, OLED_SDA, OLED_SCL);
  showInfo();
  carrierOff();
  bitTimer = millis();
}

// ------------------------------------------------------------
// LOOP PRINCIPAL
// ------------------------------------------------------------
void loop() {
  unsigned long now = millis();
  OperaBand &b = bands[currentBand];
  switch (state) {
    case SEND_BITS:
      if (now - bitTimer >= b.bitPeriod) {
        if (bitSequence[bitIndex] == '1') carrierOn();
        else carrierOff();
        bitIndex++;
        bitTimer = now;
        // Fin de secuencia
        if (bitIndex >= bitLen) {
          carrierOff();
          bitIndex = 0;
          state = PAUSE;
          stateTimer = now;
          Serial.println("🕒 Fin TX - Pausa...");
        }
      }
      break;
    case PAUSE:
      if (now - stateTimer >= b.pause) {
        state = SEND_BITS;
        bitTimer = now;
        nextBand();  // cambia automáticamente a la siguiente banda
        Serial.println("🚀 Fin pausa - Reinicio TX ...");
      }
      break;
  }
}
