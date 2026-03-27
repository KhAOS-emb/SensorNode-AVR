#include <Arduino.h>

#include "hal/i2c_hal.h"
#include "hal/gpio_hal.h"
#include "hal/uart_hal.h"

#include "drivers/dht11_driver.h"
#include "drivers/ssd1306_driver.h"

// ─── Objekte ────────────────────────────────────────
DHT11Driver  dht11(&DDRD, &PORTD, &PIND, PD4);
SSD1306Driver oled;

void setup() {
    UART_HAL::init(115200);
    delay(2000);  // Serial Monitor
    delay(2000);  // DHT11 Anlaufzeit ← neu
    UART_HAL::sendLine("=== Setup startet ===");
    
}

void loop() {
    if (dht11.update()) {
        UART_HAL::sendString("Temp: ");
        UART_HAL::sendInt(static_cast<int>(dht11.getTemperature()));
        UART_HAL::sendString("C  Hum: ");
        UART_HAL::sendInt(static_cast<int>(dht11.getHumidity()));
        UART_HAL::sendLine("%");
    } else {
        UART_HAL::sendLine("DHT11: FEHLER");
    }

    delay(2000);
}