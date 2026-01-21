package com.example;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.servlet.mvc.method.annotation.SseEmitter;

import java.io.IOException;

@RestController
public class SseController {

  private final SerialService serial;

  public SseController(SerialService serial) {
    this.serial = serial;
  }

  @GetMapping("/api/latest")
  public SensorData latest() {
    return serial.getLatest();
  }

  @GetMapping("/api/stream")
  public SseEmitter stream() {
    SseEmitter emitter = new SseEmitter(0L);

    Thread t = new Thread(() -> {
      try {
        while (true) {
          emitter.send(SseEmitter.event().name("data").data(serial.getLatest()));
          Thread.sleep(250);
        }
      } catch (IOException | InterruptedException e) {
        emitter.complete();
      }
    });

    t.setDaemon(true);
    t.start();
    return emitter;
  }
}
