#define NODO // nodo version uses different pins and LEDs are in a different order
// #define PICO // Old board define PICO, new board don't define it to select proper I/O pins
#ifdef NODO
#ifdef PICO // ESP32-PICO
#define SDA 22
#define SCL 21
#else // ESP32-C3
#define SDA 6
#define SCL 7
#define LED 8
#define LDR_PIN 1
#define NEOPIXEL_PIN 0
#endif
#endif
