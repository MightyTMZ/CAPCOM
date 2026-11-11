const int NUM_PINS = 30;

int pinNumbers[NUM_PINS];
int pinOffset = 22;
int delayTime = 100;
int numberOfPinsToLight = 1;

bool keepAllLightsOnAfter = true;

// Generates k distinct random integers between minVal and maxVal (inclusive)
void randomDistinct(int result[], int k, int minVal, int maxVal) {
  int n = maxVal - minVal + 1;

  if (k < 0 || k > n || n <= 0) return;

  int numbers[n];

  for (int i = 0; i < n; i++) {
    numbers[i] = minVal + i;
  }

  // Fisher-Yates shuffle (partial)
  for (int i = 0; i < k; i++) {
    int j = random(i, n);

    int temp = numbers[i];
    numbers[i] = numbers[j];
    numbers[j] = temp;

    result[i] = numbers[i];
  }
}

void turnOffAllLights(){
  for (int i = 0; i < NUM_PINS; i++){
    digitalWrite(pinNumbers[i], 0);
  }
}

void turnOnAllLights(){
  for (int i = 0; i < NUM_PINS; i++){
    digitalWrite(pinNumbers[i], 1);
  }
}

void setup() {
  for (int i = 0; i < NUM_PINS; i++){
    pinNumbers[i] = pinOffset + i;
    pinMode(pinNumbers[i], OUTPUT);
  }
}

void loop() {
  while (delayTime > 20) {
    for (int i = 0; i < NUM_PINS; i++) {
      digitalWrite(pinNumbers[i], 1);
      delay(delayTime);
      turnOffAllLights();
    }
    for (int i = NUM_PINS - 1; i > 0; i--) {
      digitalWrite(pinNumbers[i], 1);
      delay(delayTime);
      turnOffAllLights();
    }
    delayTime -= 20;
  }

  while(keepAllLightsOnAfter) {turnOnAllLights();}
}