#include <unity.h>

// Gesture enum
enum Gesture {
  OPEN_PALM,
  FIST,
  POINT,
  LIKE,
  MIDDLE_FINGER
};

// EMG thresholds
const int EMG_THRESHOLD_LOW = 200;
const int EMG_THRESHOLD_HIGH = 600;

// Gesture recognition function (same logic as in main.cpp)
Gesture recognizeGesture(int emg_a3, int emg_a4) {
  bool a3_low = emg_a3 < EMG_THRESHOLD_LOW;
  bool a3_mid = emg_a3 >= EMG_THRESHOLD_LOW && emg_a3 < EMG_THRESHOLD_HIGH;
  bool a3_high = emg_a3 >= EMG_THRESHOLD_HIGH;
  
  bool a4_low = emg_a4 < EMG_THRESHOLD_LOW;
  bool a4_mid = emg_a4 >= EMG_THRESHOLD_LOW && emg_a4 < EMG_THRESHOLD_HIGH;
  bool a4_high = emg_a4 >= EMG_THRESHOLD_HIGH;
  
  if (a3_low && a4_low) {
    return OPEN_PALM;
  } else if (a3_high && a4_high) {
    return FIST;
  } else if (a3_high && a4_low) {
    return POINT;
  } else if (a3_low && a4_high) {
    return LIKE;
  } else if (a3_high && a4_mid) {
    return MIDDLE_FINGER;
  }
  return OPEN_PALM;
}

// Test open palm (both low)
void test_gesture_open_palm(void) {
    Gesture result = recognizeGesture(50, 100);
    TEST_ASSERT_EQUAL_INT(OPEN_PALM, result);
}

// Test fist (both high)
void test_gesture_fist(void) {
    Gesture result = recognizeGesture(700, 800);
    TEST_ASSERT_EQUAL_INT(FIST, result);
}

// Test point (A3 high, A4 low)
void test_gesture_point(void) {
    Gesture result = recognizeGesture(700, 100);
    TEST_ASSERT_EQUAL_INT(POINT, result);
}

// Test like (A3 low, A4 high)
void test_gesture_like(void) {
    Gesture result = recognizeGesture(100, 800);
    TEST_ASSERT_EQUAL_INT(LIKE, result);
}

// Test middle finger (A3 high, A4 mid)
void test_gesture_middle_finger(void) {
    Gesture result = recognizeGesture(700, 400);
    TEST_ASSERT_EQUAL_INT(MIDDLE_FINGER, result);
}

// Test boundary conditions - at low threshold
void test_gesture_boundary_low_threshold(void) {
    Gesture result = recognizeGesture(200, 200);
    // Should be FIST since 200 is not < 200, it's >= 200
    TEST_ASSERT_EQUAL_INT(FIST, result);
}

// Test boundary conditions - just below low threshold
void test_gesture_boundary_below_low_threshold(void) {
    Gesture result = recognizeGesture(199, 199);
    TEST_ASSERT_EQUAL_INT(OPEN_PALM, result);
}

// Test boundary conditions - at high threshold
void test_gesture_boundary_high_threshold(void) {
    Gesture result = recognizeGesture(600, 600);
    TEST_ASSERT_EQUAL_INT(FIST, result);
}

// Test boundary conditions - just below high threshold
void test_gesture_boundary_below_high_threshold(void) {
    Gesture result = recognizeGesture(599, 599);
    TEST_ASSERT_EQUAL_INT(OPEN_PALM, result);
}

// Test zero values (should be open palm)
void test_gesture_zero_values(void) {
    Gesture result = recognizeGesture(0, 0);
    TEST_ASSERT_EQUAL_INT(OPEN_PALM, result);
}

// Test max ADC values (should be fist)
void test_gesture_max_adc_values(void) {
    Gesture result = recognizeGesture(1023, 1023);
    TEST_ASSERT_EQUAL_INT(FIST, result);
}

// Test point with mid-range A4
void test_gesture_point_with_mid_a4(void) {
    Gesture result = recognizeGesture(700, 400);
    TEST_ASSERT_EQUAL_INT(MIDDLE_FINGER, result);
}

// Test point with high A4 (should be fist)
void test_gesture_both_high(void) {
    Gesture result = recognizeGesture(750, 750);
    TEST_ASSERT_EQUAL_INT(FIST, result);
}

// Test asymmetric values A3 mid A4 low (should be open palm)
void test_gesture_a3_mid_a4_low(void) {
    Gesture result = recognizeGesture(400, 100);
    TEST_ASSERT_EQUAL_INT(OPEN_PALM, result);
}

// Test asymmetric values A3 low A4 mid (should be open palm)
void test_gesture_a3_low_a4_mid(void) {
    Gesture result = recognizeGesture(100, 400);
    TEST_ASSERT_EQUAL_INT(OPEN_PALM, result);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_gesture_open_palm);
    RUN_TEST(test_gesture_fist);
    RUN_TEST(test_gesture_point);
    RUN_TEST(test_gesture_like);
    RUN_TEST(test_gesture_middle_finger);
    RUN_TEST(test_gesture_boundary_low_threshold);
    RUN_TEST(test_gesture_boundary_below_low_threshold);
    RUN_TEST(test_gesture_boundary_high_threshold);
    RUN_TEST(test_gesture_boundary_below_high_threshold);
    RUN_TEST(test_gesture_zero_values);
    RUN_TEST(test_gesture_max_adc_values);
    RUN_TEST(test_gesture_point_with_mid_a4);
    RUN_TEST(test_gesture_both_high);
    RUN_TEST(test_gesture_a3_mid_a4_low);
    RUN_TEST(test_gesture_a3_low_a4_mid);
    
    UNITY_END();
}

void loop() {
}
