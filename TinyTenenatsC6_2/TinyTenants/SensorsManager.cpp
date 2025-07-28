#include "DebugManager.h"
#include "SensorsManager.h"

#define MOTION_SESNOR_IO_PIN 18
#define MOTION_SESNOR_RX 20  // RX pin on ESP32
#define MOTION_SESNOR_TX 19  // TX pin on ESP32

#define AGS10_I2C_ADDRESS 0x1A  // Replace with the AGS10 I2C address
#define SDA_PIN 7        // I2C SDA pin for ESP32
#define SCL_PIN 6        // I2C SCL pin for ESP32

#define AC_SWITCH1_PIN 4
#define AC_SWITCH2_PIN 5

#define LDR_PIN 3

#define I2S_SCK 21
#define I2S_WS 22
#define I2S_SD 23
#define I2S_PORT           I2S_NUM_0
#define SAMPLE_RATE        16000
#define I2S_SAMPLE_BITS    I2S_BITS_PER_SAMPLE_32BIT
#define I2S_READ_LEN       1024
#define SAMPLES            (I2S_READ_LEN / sizeof(int32_t))
#define REFERENCE_RMS      9500.0f  // Tune this to calibrate silence = ~40 dB

SensorsManager::SensorsManager(): motionSensor(2){}

void SensorsManager::initSensors(){
	uart1.begin(115200, SERIAL_8N1, 17, 16);
  DEBUG_PRINTLN("Initializing Sensors....");
	Wire.begin(SDA_PIN, SCL_PIN);
	
	initAGS10();
  initAHT20();
  initMotionSensor();
  initACSwitchReading();
  pinMode(LDR_PIN, INPUT);
  initMicrophone();
}

/***************************AHT20*******************************/
void SensorsManager::initAHT20() {
    if (aht.begin()) {
        DEBUG_PRINTLN("Found AHT20");
    } else {
        DEBUG_PRINTLN("Didn't find AHT20");
		    uart1.println("{\"AHT20\": \"Failed\"}");
    } 
}
void SensorsManager::getAHT20Data() {
    aht.getEvent(&humidity, &temp);
	DEBUG_PRINTLN("{\"temperature\": " + String(temp.temperature) + ", \"humidity\": " + String(humidity.relative_humidity) + "}");
    uart1.println("{\"temperature\": " + String(temp.temperature) + ", \"humidity\": " + String(humidity.relative_humidity) + "}");
}
/***************************AHT20*******************************/

/***************************Motion Sensor*******************************/
void SensorsManager::initMotionSensor() {
    pinMode(MOTION_SESNOR_IO_PIN, INPUT);
    motionSensor.begin(256000, SERIAL_8N1, MOTION_SESNOR_RX, MOTION_SESNOR_TX); // LD2420 uses 256000 baud rate
}
void SensorsManager::getMotionSensorData() {
    // int motion = digitalRead(MOTION_SESNOR_IO_PIN);
    if (motionSensor.available()) {
        DEBUG_PRINTLN("Data available on UART2: " + String(motionSensor.available()) + " bytes");
        while (motionSensor.available()) {
            char incomingByte = motionSensor.read();  // Read a byte from UART2
            if (incomingByte == '\n') {              // If newline character is detected
                incomingBuffer[bufferIndex] = '\0';
                DEBUG_PRINTLN("Received: " + String(incomingBuffer));
                bufferIndex = 0;  // Reset buffer index
            } else {  // Add the byte to the buffer if there's space
                if (bufferIndex < BUFFER_SIZE - 1) {
                    incomingBuffer[bufferIndex++] = incomingByte;
                } else {
                    DEBUG_PRINTLN("Buffer overflow detected");
                    bufferIndex = 0;  // Reset on overflow to prevent issues
                }
            }
        }
    } else {
        DEBUG_PRINTLN("No data available on UART2");
    }

    // Fallback motion value (if needed) - replace with actual logic
    int motion = digitalRead(MOTION_SESNOR_IO_PIN);
    DEBUG_PRINTLN("{\"motion\": " + String(motion) + "}");
    uart1.println("{\"motion\": " + String(motion) + "}");
}
/***************************Motion Sensor*******************************/

/***************************AGS10*******************************/

void SensorsManager::initAGS10() {
  Wire.beginTransmission(AGS10_I2C_ADDRESS);
  if (Wire.endTransmission() == 0) {
    DEBUG_PRINTLN("Found AGS10");
	  uart1.println("{\"AGS10\": \"success\"}");
  } else {
    DEBUG_PRINTLN("Didn't find AGS10");
	  uart1.println("{\"AGS10\": \"Failed\"}");
  }
}
void SensorsManager::getAGS10Data() {
  Wire.beginTransmission(AGS10_I2C_ADDRESS);
  Wire.write(0x00);
	if (Wire.endTransmission() != 0) {
    DEBUG_PRINTLN("AGS10: I2C transmission failed");
    return;
  }
  delay(50); // Add delay for sensor response
  Wire.requestFrom(AGS10_I2C_ADDRESS, 5); // Request 5 bytes

  int availableBytes = Wire.available();
  DEBUG_PRINTLN("AGS10: Available bytes = " + String(availableBytes));

  if (availableBytes > 0) {
    byte statusByte = Wire.read();
    byte tvocByteA = Wire.read();
    byte tvocByteB = Wire.read();
    byte tvocByteC = Wire.read();
    byte crcByte = Wire.read();

    int tvoc = (tvocByteA << 16) | (tvocByteB << 8) | tvocByteC;
    DEBUG_PRINTLN("AGS10 Raw Data - Status: 0x" + String(statusByte, HEX) + ", TVOC: " + String(tvoc) + ", CRC: 0x" + String(crcByte, HEX));
    DEBUG_PRINTLN("{\"TVOC\": " + String(tvoc) + " ppb}");
    uart1.println("{\"TVOC\": " + String(tvoc) + " ppb}");
  } else {
    DEBUG_PRINTLN("AGS10: No data available");
    uart1.println("{\"TVOC\": \"NoData\", \"TVOC_available\": " + String(availableBytes) + "}");
  }
}
/***************************AGS10*******************************/

/***************************AC Switch Reading*******************************/
void SensorsManager::initACSwitchReading() {
  pinMode(AC_SWITCH1_PIN, INPUT);
  pinMode(AC_SWITCH2_PIN, INPUT);
}
void SensorsManager::getACSwitchReading() {
  int acSwitch1 = digitalRead(AC_SWITCH1_PIN);
  int acSwitch2 = digitalRead(AC_SWITCH2_PIN);
  DEBUG_PRINTLN("{\"acSwitch1\": " + String(acSwitch1) + ", \"acSwitch2\": " + String(acSwitch2) + "}");
  uart1.println("{\"acSwitch1\": " + String(acSwitch1) + ", \"acSwitch2\": " + String(acSwitch2) + "}");
}
/***************************AC Switch Reading*******************************/

/***************************LDR*******************************/
void SensorsManager::getLDRData() {
  int ldr = digitalRead(LDR_PIN);
  DEBUG_PRINTLN("{\"LDR\": " + String(ldr) + "}");
  uart1.println("{\"LDR\": " + String(ldr) + "}");
}
/***************************LDR*******************************/

/***************************Microphone*******************************/
void SensorsManager::initMicrophone() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_SAMPLE_BITS,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 256,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD
    };

    // Install and configure I2S driver
    esp_err_t i2s_install_result = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (i2s_install_result != ESP_OK) {
        DEBUG_PRINTLN("I2S driver installation failed with error: " + String(i2s_install_result));
        return;
    }
    esp_err_t i2s_set_pin_result = i2s_set_pin(I2S_PORT, &pin_config);
    if (i2s_set_pin_result != ESP_OK) {
        DEBUG_PRINTLN("I2S pin configuration failed with error: " + String(i2s_set_pin_result));
        return;
    }
    i2s_zero_dma_buffer(I2S_PORT);

    DEBUG_PRINTLN("I2S initialized for SPH0645 with sample rate: " + String(SAMPLE_RATE) + 
                    ", bits per sample: " + String(I2S_SAMPLE_BITS) + 
                    ", pins - BCK: " + String(I2S_SCK) + ", WS: " + String(I2S_WS) + ", SD: " + String(I2S_SD));
}

void SensorsManager::getMicrophoneData(){
  int32_t i2s_samples[SAMPLES];
  size_t bytes_read = 0;

  esp_err_t i2s_read_result = i2s_read(I2S_PORT, (void *)i2s_samples, I2S_READ_LEN, &bytes_read, portMAX_DELAY);
  if (i2s_read_result != ESP_OK) {
    DEBUG_PRINTLN("I2S read failed with error: " + String(i2s_read_result));
    return;
  }
  DEBUG_PRINTLN("I2S read completed, bytes read: " + String(bytes_read));

  int sample_count = bytes_read / sizeof(int32_t);
  if (sample_count == 0) {
    DEBUG_PRINTLN("No samples read from I2S");
    uart1.println("{\"soundLevel\": \"NoData\"}");
    return;
  }
  DEBUG_PRINTLN("Sample count: " + String(sample_count));

  float sum = 0.0;

  for (int i = 0; i < sample_count; ++i) {
    // Extract valid 24-bit audio from 32-bit frame
    int32_t sample = (i2s_samples[i] >> 8);  // Keep the MSB 24 bits
    sum += (float)sample * (float)sample;
    if (i < 5) { // Print first few samples for debugging
      DEBUG_PRINTLN("Sample " + String(i) + ": " + String(sample));
    }
  }

  float rms = sqrt(sum / sample_count);
  DEBUG_PRINTLN("RMS value: " + String(rms));
  float dB = 20.0 * log10(rms / REFERENCE_RMS);
  DEBUG_PRINTLN("Calculated dB: " + String(dB));

  DEBUG_PRINTF("Sound Level: %.2f dB\n", dB);
  uart1.println("{\"soundLevel\": " + String(dB) + "}");
}
/***************************Microphone*******************************/

void SensorsManager::sendAllSensorsData(){
  getAHT20Data();
  getMotionSensorData();
  getAGS10Data();
  getACSwitchReading();
  getLDRData();
  getMicrophoneData();
}

String SensorsManager::getSensorData() {
  int temperature = random(20, 30);
  int humidity = random(40, 60);
  return "{\"temperature\": " + String(temperature) + ", \"humidity\": " + String(humidity) + "}";
}
