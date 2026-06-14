#include <Arduino.h>
#include <ArduinoBLE.h>
#include <pre-demo_project_emg_sensors_inferencing.h>

static const int EMG_PIN_1 = A0;
static const int EMG_PIN_2 = A1;
static const int BUTTON_PIN = 2;

static const int SAMPLE_RATE_HZ = 499;
static const unsigned long SAMPLE_INTERVAL_US = 1000000UL / SAMPLE_RATE_HZ;

static const int ADC_BITS = 10;

static const unsigned long WARMUP_MS = 5000;

static const float CONFIDENCE_THRESHOLD = 0.60f;

static const char *PICO_SERVICE_UUID = "c3feed70-b50c-400a-836c-c8981beb0b1c";
static const char *PICO_CMD_CHAR_UUID = "c3feed71-b50c-400a-836c-c8981beb0b1c";

static BLEDevice picoDevice;
static BLECharacteristic cmdCharacteristic;

static float ei_buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

struct GestureResult
{
    const char *label;
    float confidence;
    bool valid;
    ei_impulse_result_t raw;
};

GestureResult classifyGesture()
{
    unsigned long next_us = micros();

    for (int i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;)
    {
        while ((long)(micros() - next_us) < 0)
        {
        }
        next_us += SAMPLE_INTERVAL_US / 2;
        ei_buffer[i++] = (float)analogRead(EMG_PIN_1);

        while ((long)(micros() - next_us) < 0)
        {
        }
        next_us += SAMPLE_INTERVAL_US / 2;
        ei_buffer[i++] = (float)analogRead(EMG_PIN_2);
    }

    signal_t signal;
    numpy::signal_from_buffer(ei_buffer,
                              EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE,
                              &signal);

    ei_impulse_result_t result = {};
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);

    if (err != EI_IMPULSE_OK)
    {
        return {"ERROR", 0.0f, false, result};
    }

    int top_idx = 0;
    float top_score = 0.0f;
    for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++)
    {
        if (result.classification[i].value > top_score)
        {
            top_score = result.classification[i].value;
            top_idx = i;
        }
    }

    return {
        top_score >= CONFIDENCE_THRESHOLD
            ? result.classification[top_idx].label
            : "LOW_CONF",
        top_score,
        true,
        result};
}

void printResult(const GestureResult &g)
{
    if (!g.valid)
    {
        Serial.println(">> ERROR: classifier failed");
        return;
    }

    Serial.print(">> ");
    Serial.print(g.label);
    Serial.print("  (");
    Serial.print(g.confidence * 100.0f, 1);
    Serial.print("%)");
    if (g.confidence < CONFIDENCE_THRESHOLD)
        Serial.print("  [LOW CONF]");
    Serial.println();

    for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++)
    {
        Serial.print("   ");
        Serial.print(g.raw.classification[i].label);
        Serial.print(": ");
        Serial.print(g.raw.classification[i].value * 100.0f, 1);
        Serial.println("%");
    }

    Serial.print("   [DSP ");
    Serial.print(g.raw.timing.dsp);
    Serial.print(" ms | Inf ");
    Serial.print(g.raw.timing.classification);
    Serial.println(" ms]");
}

bool buttonPressed()
{
    static bool last_state = HIGH;
    static unsigned long last_ms = 0;

    bool state = digitalRead(BUTTON_PIN);
    if (state == LOW && last_state == HIGH && (millis() - last_ms) > 50)
    {
        last_ms = millis();
        last_state = state;
        return true;
    }
    last_state = state;
    return false;
}

void sendGestureCommand(const char *label)
{
    if (!picoDevice || !picoDevice.connected())
    {
        Serial.println("BLE: not connected, skipping send");
        return;
    }
    if (!cmdCharacteristic.canWrite())
    {
        Serial.println("BLE: characteristic not writable");
        return;
    }
    if (cmdCharacteristic.writeValue(label))
    {
        Serial.print("BLE: sent → ");
        Serial.println(label);
    }
    else
    {
        Serial.print("BLE: write failed for → ");
        Serial.println(label);
    }
}

void connectToPico()
{
    Serial.println("BLE: scanning for Pico W...");

    if (!BLE.scanForUuid(PICO_SERVICE_UUID))
    {
        Serial.println("BLE: scan failed");
        return;
    }

    unsigned long start = millis();
    BLEDevice peripheral;
    while (millis() - start < 5000)
    {
        peripheral = BLE.available();
        if (peripheral)
            break;
        delay(10);
    }
    BLE.stopScan();

    if (!peripheral)
    {
        Serial.println("BLE: no device found");
        return;
    }

    Serial.print("BLE: found ");
    Serial.println(peripheral.address());

    if (!peripheral.connect())
    {
        Serial.println("BLE: connection failed");
        return;
    }
    if (!peripheral.discoverAttributes())
    {
        Serial.println("BLE: attribute discovery failed");
        peripheral.disconnect();
        return;
    }

    cmdCharacteristic = peripheral.characteristic(PICO_CMD_CHAR_UUID);
    if (!cmdCharacteristic)
    {
        Serial.println("BLE: command characteristic not found");
        peripheral.disconnect();
        return;
    }

    picoDevice = peripheral;
    Serial.println("BLE: connected to Pico W");
}

void setup()
{
    Serial.begin(115200);
    unsigned long t0 = millis();
    while (!Serial && (millis() - t0 < 3000))
    {
    }

    analogReadResolution(ADC_BITS);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    Serial.println("# -- EI EMG Classifier ------------------");
    Serial.print("# Frequency : ");
    Serial.print(EI_CLASSIFIER_FREQUENCY);
    Serial.println(" Hz");
    Serial.print("# Labels    : ");
    for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++)
    {
        Serial.print(ei_classifier_inferencing_categories[i]);
        if (i < EI_CLASSIFIER_LABEL_COUNT - 1)
            Serial.print(", ");
    }
    Serial.println();

    if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 2)
    {
        Serial.println("FATAL: model expects != 2 axes.");
        while (true)
        {
        }
    }

    if (!BLE.begin())
    {
        Serial.println("FATAL: BLE init failed");
        while (true)
        {
        }
    }
    BLE.setLocalName("Nano");
    Serial.println("# BLE initialised");

    Serial.println("# Warming up — keep muscles RELAXED for 5 s...");
    delay(WARMUP_MS);
    Serial.println("# Ready. Press button to classify a gesture.\n");
}

void loop()
{
    if (!picoDevice || !picoDevice.connected())
    {
        static unsigned long last_attempt = 0;
        if (millis() - last_attempt >= 3000)
        {
            connectToPico();
            last_attempt = millis();
        }
    }

    if (!buttonPressed())
        return;

    Serial.println("# Sampling...");

    GestureResult gesture = classifyGesture();
    printResult(gesture);

    if (gesture.valid && strcmp(gesture.label, "LOW_CONF") != 0 && strcmp(gesture.label, "ERROR") != 0)
    {
        sendGestureCommand(gesture.label);
    }

    Serial.println("\n# Done. Press button for next classification.");
    Serial.println("# __________________________________________________\n");
}
