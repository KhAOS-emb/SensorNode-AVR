# SensorNode-AVR

[![SensorNode-AVR CI](https://github.com/KhAOS-emb/SensorNode-AVR/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/KhAOS-emb/SensorNode-AVR/actions/workflows/ci.yml)
[![SensorNode-AVR CI](https://github.com/KhAOS-emb/SensorNode-AVR/actions/workflows/ci.yml/badge.svg?branch=develop)](https://github.com/KhAOS-emb/SensorNode-AVR/actions/workflows/ci.yml)
[![Docs](https://img.shields.io/badge/docs-github%20pages-blue)](https://KhAOS-emb.github.io/SensorNode-AVR/)

![](https://img.shields.io/github/issues/KhAOS-emb/SensorNode-AVR)
![](https://img.shields.io/github/last-commit/KhAOS-emb/SensorNode-AVR)
![](https://img.shields.io/github/repo-size/KhAOS-emb/SensorNode-AVR)

# SensorNode-AVR

ATmega328P-basierter Sensorknoten in C++ mit klarer Hardware-Abstraktion, modularen Treibern, Unit Tests und CI/CD.

## Überblick

Dieses Projekt zeigt einen professionell aufgebauten Embedded-Sensorknoten für den ATmega328P.  
Die Architektur trennt Hardware-Zugriff, Treiber und Anwendungslogik sauber voneinander.  
So bleibt der Code testbar, portierbar und leicht erweiterbar.

## Ziele des Projekts

- Saubere Trennung von HAL, Treibern und Applikationslogik
- Zugriff auf GPIO, ADC, I²C, UART und PWM über eigene Abstraktionsschichten
- Sensoren und Aktoren über Interfaces ankoppeln
- Logik auf dem PC mit Unit Tests prüfen
- Automatische Builds und Prüfungen über GitHub Actions

## Enthaltene Bausteine

- **ATmega328P / Arduino Uno**
- **C++17**
- **GPIO HAL**
- **ADC HAL**
- **I²C HAL**
- **UART HAL mit Interrupt-Empfang**
- **PWM HAL**
- **DHT11-Temperatur- und Feuchtigkeitssensor**
- **SSD1306 OLED-Display**
- **Relay-Steuerung**
- **Buzzer-Steuerung**
- **AlarmManager als Zustandsautomat**
- **GoogleTest für Unit Tests**
- **GitHub Actions für CI/CD**

## Architektur

Die Projektstruktur folgt einer klaren Schichtung:

- **HAL**  
  Kapselt die direkten Registerzugriffe auf den ATmega328P.

- **Drivers**  
  Enthält gerätespezifische Klassen wie DHT11, OLED, Relay und Buzzer.

- **App**  
  Enthält die eigentliche Logik, zum Beispiel den AlarmManager.

- **Tests**  
  Prüft Logik ohne echte Hardware mithilfe von Mocks.

## Funktionsprinzip

Der Sensorknoten liest Sensordaten ein, verarbeitet sie und steuert daraufhin Ausgänge wie Relay oder Buzzer.  
Der AlarmManager arbeitet mit einem Zustandsautomaten:

- **NORMAL**
- **ALARM_PENDING**
- **ALARM_ACTIVE**

Kurzzeitige Ausreißer werden dadurch abgefangen.  
Eine Bedingung muss eine gewisse Zeit anliegen, bevor ein Alarm wirklich aktiv wird.

## Beispielhafte Hardware-Funktionen

- **DHT11** liefert Temperatur und Luftfeuchtigkeit
- **SSD1306 OLED** zeigt Werte und Status an
- **Relay** schaltet externe Lasten
- **Buzzer** gibt akustische Signale aus
- **UART** dient für Debug-Ausgaben und serielle Kommunikation

## Projektstruktur

```text
SensorNode-AVR/
├── src/
│   ├── hal/
│   ├── drivers/
│   ├── app/
│   └── main.cpp
├── tests/
├── include/
├── lib/
├── platformio.ini
├── CMakeLists.txt
└── .github/workflows/ci.yml
```

## Build und Flash

Das Projekt ist für eine typische AVR-Entwicklungsumgebung vorgesehen.  
Je nach lokaler Einrichtung kannst du es über PlatformIO oder über die in der Anleitung beschriebene Toolchain bauen.

Typischer Ablauf:

```bash
git clone https://github.com/KhAOS-emb/SensorNode-AVR.git
cd SensorNode-AVR
```

Danach:

- Projekt in VS Code öffnen
- Build starten
- Firmware flashen
- Seriellen Monitor öffnen

## Tests

Die Logik ist so aufgebaut, dass Teile davon ohne Hardware testbar sind.  
Dafür werden Mocks eingesetzt, zum Beispiel für Sensoren, Relay und Buzzer.

Beispielhafte Testziele:

- Mittelwertbildung
- Zustandswechsel im AlarmManager
- Fehlerbehandlung bei Sensordaten
- Reaktion auf Grenzwertüberschreitungen

## CI/CD

Die GitHub-Actions-Pipeline kann unter anderem folgende Aufgaben übernehmen:

- Unit Tests ausführen
- AVR-Code kompilieren
- Statische Analyse durchführen
- Dokumentation erzeugen

## Lernziele des Projekts

Dieses Projekt dient nicht nur als Firmware, sondern auch als Lernpfad für:

- Embedded C++
- Registernahe Hardware-Programmierung
- Zustandsautomaten
- Testbare Architektur
- Professionelle Projektstruktur
- Build- und Release-Prozesse

## Status

Das Projekt ist als strukturierter Lern- und Praxisaufbau angelegt.  
Jeder Meilenstein baut auf dem vorherigen auf.

## Lizenz

Noch nicht festgelegt.
