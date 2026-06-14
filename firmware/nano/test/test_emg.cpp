#include <unity.h>
#include <Arduino.h>

// Simulated EMG encoding function (same logic as in main.cpp)
void encodeEMG(int emg_a3, int emg_a4, unsigned char* data) {
    data[0] = (emg_a3 >> 8) & 0xFF;
    data[1] = emg_a3 & 0xFF;
    data[2] = (emg_a4 >> 8) & 0xFF;
    data[3] = emg_a4 & 0xFF;
}

// Test EMG encoding with zero values
void test_emg_encoding_zero(void) {
    unsigned char data[4];
    encodeEMG(0, 0, data);
    
    TEST_ASSERT_EQUAL_UINT8(0x00, data[0]);
    TEST_ASSERT_EQUAL_UINT8(0x00, data[1]);
    TEST_ASSERT_EQUAL_UINT8(0x00, data[2]);
    TEST_ASSERT_EQUAL_UINT8(0x00, data[3]);
}

// Test EMG encoding with max 10-bit ADC value (1023)
void test_emg_encoding_max_adc(void) {
    unsigned char data[4];
    encodeEMG(1023, 1023, data);
    
    TEST_ASSERT_EQUAL_UINT8(0x03, data[0]);
    TEST_ASSERT_EQUAL_UINT8(0xFF, data[1]);
    TEST_ASSERT_EQUAL_UINT8(0x03, data[2]);
    TEST_ASSERT_EQUAL_UINT8(0xFF, data[3]);
}

// Test EMG encoding with mixed values
void test_emg_encoding_mixed_values(void) {
    unsigned char data[4];
    encodeEMG(256, 512, data);
    
    TEST_ASSERT_EQUAL_UINT8(0x01, data[0]);  // 256 >> 8
    TEST_ASSERT_EQUAL_UINT8(0x00, data[1]);  // 256 & 0xFF
    TEST_ASSERT_EQUAL_UINT8(0x02, data[2]);  // 512 >> 8
    TEST_ASSERT_EQUAL_UINT8(0x00, data[3]);  // 512 & 0xFF
}

// Test EMG encoding with threshold values
void test_emg_encoding_threshold_low(void) {
    unsigned char data[4];
    encodeEMG(200, 200, data);
    
    TEST_ASSERT_EQUAL_UINT8(0x00, data[0]);
    TEST_ASSERT_EQUAL_UINT8(200, data[1]);
    TEST_ASSERT_EQUAL_UINT8(0x00, data[2]);
    TEST_ASSERT_EQUAL_UINT8(200, data[3]);
}

// Test EMG encoding with threshold values (high)
void test_emg_encoding_threshold_high(void) {
    unsigned char data[4];
    encodeEMG(600, 600, data);
    
    TEST_ASSERT_EQUAL_UINT8(0x02, data[0]);
    TEST_ASSERT_EQUAL_UINT8(88, data[1]);   // 600 & 0xFF = 88
    TEST_ASSERT_EQUAL_UINT8(0x02, data[2]);
    TEST_ASSERT_EQUAL_UINT8(88, data[3]);
}

// Test EMG decoding (verifying round-trip)
void test_emg_encode_decode_roundtrip(void) {
    unsigned char data[4];
    int original_a3 = 345;
    int original_a4 = 678;
    
    encodeEMG(original_a3, original_a4, data);
    
    // Decode back
    int decoded_a3 = ((unsigned char)data[0] << 8) | (unsigned char)data[1];
    int decoded_a4 = ((unsigned char)data[2] << 8) | (unsigned char)data[3];
    
    TEST_ASSERT_EQUAL_INT(original_a3, decoded_a3);
    TEST_ASSERT_EQUAL_INT(original_a4, decoded_a4);
}

// Test EMG encoding asymmetric values
void test_emg_encoding_asymmetric(void) {
    unsigned char data[4];
    encodeEMG(100, 900, data);
    
    int decoded_a3 = ((unsigned char)data[0] << 8) | (unsigned char)data[1];
    int decoded_a4 = ((unsigned char)data[2] << 8) | (unsigned char)data[3];
    
    TEST_ASSERT_EQUAL_INT(100, decoded_a3);
    TEST_ASSERT_EQUAL_INT(900, decoded_a4);
}

void setup() {
    delay(2000); // service delay
    UNITY_BEGIN();
    
    RUN_TEST(test_emg_encoding_zero);
    RUN_TEST(test_emg_encoding_max_adc);
    RUN_TEST(test_emg_encoding_mixed_values);
    RUN_TEST(test_emg_encoding_threshold_low);
    RUN_TEST(test_emg_encoding_threshold_high);
    RUN_TEST(test_emg_encode_decode_roundtrip);
    RUN_TEST(test_emg_encoding_asymmetric);
    
    UNITY_END();
}

void loop() {
}
