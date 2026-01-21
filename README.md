## Air Monitor – Embedded + Web Project

This project integrates an Arduino-based embedded system with a Java Spring Boot
web application for real-time environmental monitoring.

### Embedded (Arduino)
- Language: C/C++ (Arduino)
- Sensors: DHT22 (temperature/humidity), MQ-135 (air quality), OKY3106 (light)
- Display: OLED SSD1306 (SPI)
- Features: local display, LED status, JSON serial output

### Web Application
- Backend: Java 21, Spring Boot
- Real-time streaming: Server-Sent Events (SSE)
- Frontend: HTML, CSS, Vanilla JavaScript
- Data source: Serial communication over USB
