const int EMG_PIN_1 = A0;
const int EMG_PIN_2 = A1;

const int   SAMPLE_RATE_HZ  = 500;
const unsigned long SAMPLE_INTERVAL_US = 1000000UL / SAMPLE_RATE_HZ;

const unsigned long WARMUP_MS = 5000;

const int ADC_BITS = 10;

void setup() {
  Serial.begin(115200);

  unsigned long t0 = millis();
  while (!Serial && (millis() - t0 < 3000)) {}

  analogReadResolution(ADC_BITS);

  Serial.println("# Grove EMG sensor warmup — keep muscles RELAXED for 5 s...");
  delay(WARMUP_MS);
  Serial.println("# Ready! Streaming emg1,emg2 at 500 Hz.");
  Serial.println("# Format: <emg1>,<emg2>  (one line per sample)");
}

void loop() {
  static unsigned long next_sample_us = micros();

  if ((long)(micros() - next_sample_us) >= 0) {
    next_sample_us += SAMPLE_INTERVAL_US;

    int raw1 = analogRead(EMG_PIN_1);
    int raw2 = analogRead(EMG_PIN_2);

    Serial.print(raw1);
    Serial.print(',');
    Serial.println(raw2);
  }
}
