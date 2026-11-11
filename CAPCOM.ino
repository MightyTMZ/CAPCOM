const int NUM_PINS = 30;

int pinNumbers[NUM_PINS];
int pinOffset = 22;
int delayTime = 200;
int numberOfPinsToLight = 1;

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

void setup() {
  for (int i = 0; i < NUM_PINS; i++){
    pinNumbers[i] = pinOffset + i;
    pinMode(pinNumbers[i], OUTPUT);
  }
}

void loop() {
  // Initialize random pin states
  int randPositions[numberOfPinsToLight];
  randomDistinct(randPositions, numberOfPinsToLight, 22, 51);

  for (int pn: randPositions) {
    digitalWrite(pn, 1);
  }

  delay(delayTime);

  // Shut off all lights (instant + no delay)
  for (int i = 0; i < NUM_PINS; i++) {
    digitalWrite(pinNumbers[i], 0);
  }

  if (numberOfPinsToLight < NUM_PINS) {
    numberOfPinsToLight += 1;
  }  else {
    numberOfPinsToLight = 1;
  }
}