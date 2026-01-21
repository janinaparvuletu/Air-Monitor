package com.example;

import com.fazecast.jSerialComm.SerialPort;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.springframework.stereotype.Service;

import jakarta.annotation.PostConstruct;
import jakarta.annotation.PreDestroy;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.atomic.AtomicReference;

@Service
public class SerialService {

	//ține cel mai recent SensorData.
  private final AtomicReference<SensorData> latest =
      new AtomicReference<>(new SensorData(0,0,0,0,1));

  //obiect de tip objectMapper care face transferul intre json si Recordul SensorData
  private final ObjectMapper mapper = new ObjectMapper();
  private SerialPort port;//portul din care preia datele
  private Thread worker; //thread ul care tine legatura continua cu portul

  //face legatura cu portul
  private final String portName = "COM5"; // PORT CORECT
  private final int baud = 115200;//viteza seriala

  //
  public SensorData getLatest() {
    return latest.get();//preia in siguranta ultimele date din arduino
  }

  @PostConstruct
  public void start() {
    port = SerialPort.getCommPort(portName);//ia un obiect serial port pt com5
    port.setComPortParameters(baud, 8,SerialPort.ONE_STOP_BIT, SerialPort.NO_PARITY);//pune setarile pe obiect 
    						//legatura, nr biti. un stop bit si nu are paritate	
    // 🔴 FARA TIMEOUT – citire continua
    port.setComPortTimeouts(SerialPort.TIMEOUT_READ_BLOCKING, 0, 0);
   //le am pus active doar ca uneori arduino are nevoie de ele continuu deschise ca sa poata citi date 					
    port.setDTR();
    port.setRTS();

    if (!port.openPort()) {
      System.out.println("NU pot deschide portul " + portName);
      return;
    }

    System.out.println("Serial stabil pe " + portName);

    worker = new Thread(() -> {
      StringBuilder buffer = new StringBuilder();

      try (InputStream in = port.getInputStream()) {//incearca sa ia streamul de citire din portq
        while (!Thread.currentThread().isInterrupted()) {
          int b = in.read();
          if (b == -1) continue;

          char c = (char) b;//transforma byte ul in caraacter
          if (c == '\n') {
            String line = buffer.toString().trim();
            buffer.setLength(0);

            try {
              SensorData data = mapper.readValue(line, SensorData.class);
              latest.set(data);
            } catch (Exception ignore) {
              // ignoram linii incomplete
            }
          } else {
            buffer.append(c);
          }
        }
      } catch (Exception e) {
        System.out.println("Serial error: " + e.getMessage());
      }
    });

    worker.setDaemon(true);
    worker.start();
  }

  @PreDestroy
  public void stop() {
    if (worker != null) worker.interrupt();
    if (port != null && port.isOpen()) port.closePort();
  }
}
