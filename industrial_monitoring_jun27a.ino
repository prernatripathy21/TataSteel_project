#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_MPU6050 mpu;

const int buzzerPin = 1;
const int ledPin = 2;

// HC-SR04 sensor pins
const int trigPin = 5;
const int echoPin = 6;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000);  // Pause for 1 second to allow serial monitor to start

  // Initialize I2C communication
  Wire.begin();

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(1000);  // Pause for 1 second
  display.clearDisplay();

  // Configure the MPU6050 sensor
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Initialize buzzer pin
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  // Initialize LED pin
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Initialize ultrasonic sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

long readUltrasonicDistance() {
  long duration, distance;
  
  // Clear the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Set the trigPin on HIGH state for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance
  distance = duration * 0.034 / 2;

  return distance;
}

void loop() {
  // Get new sensor events with the readings
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Read ultrasonic sensor and get the distance
  long distance = readUltrasonicDistance();
  Serial.print("distance=");
  Serial.println(distance);

  // Clear the buffer
  display.clearDisplay();

  // Display acceleration values
  display.setTextSize(0.5);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Accel X: "); display.println(a.acceleration.x);
  display.print("Accel Y: "); display.println(a.acceleration.y);
  display.print("Accel Z: "); display.println(a.acceleration.z);

  // Display gyroscope values
  display.setCursor(0, 25);
  display.print("Gyro X: "); display.println(g.gyro.x);
  display.print("Gyro Y: "); display.println(g.gyro.y);
  display.print("Gyro Z: "); display.println(g.gyro.z);

  // Display temperature value
  display.setCursor(0, 50);
  display.print("Temp: "); display.print(temp.temperature); display.println(" *C");

  // Display ultrasonic sensor value
  display.setCursor(0, 60);
  display.print("Distance: "); display.print(distance); display.println(" cm");

  // Check if the temperature exceeds certail threshold
  if (temp.temperature > 50.0) {
    tone(buzzerPin, 1000);  // Turn on the buzzer 
    digitalWrite(ledPin, HIGH);  // Turn on the LED
    Serial.println("Temperature exceeded limit!");
  } else {
    noTone(buzzerPin);   // Turn off the buzzer
    digitalWrite(ledPin, LOW);   // Turn off the LED
  }

  // Check if the tilt exceeds safe threshold
  if (abs(a.acceleration.x) > 10.0 || abs(a.acceleration.y) > 10.0 || abs(a.acceleration.z) > 10.0) {
    tone(buzzerPin, 1000);  // Turn on the buzzer 
    digitalWrite(ledPin, HIGH);  // Turn on the LED
    Serial.println("Tilt exceeded safe threshold!");
  } else if (temp.temperature <= 50.0) {
    noTone(buzzerPin);   // Turn off the buzzer 
    digitalWrite(ledPin, LOW);   // Turn off the LED 
  }

  // Check if the distance is less than a certain threshold
  if (distance < 2) {
    tone(buzzerPin, 1000);  // Turn on the buzzer
    digitalWrite(ledPin, HIGH);  // Turn on the LED
    Serial.println("Object detected within 2 cm!");
  } else if (temp.temperature <= 50.0 && abs(a.acceleration.x) <= 10.0 && abs(a.acceleration.y) <= 10.0 && abs(a.acceleration.z) <= 10.0) {
    noTone(buzzerPin);   // Turn off the buzzer 
    digitalWrite(ledPin, LOW);   // Turn off the LED 
  }

  display.display();

  // Delay to make it readable
  delay(500);
}
