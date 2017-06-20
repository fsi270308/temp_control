/*
 * LCD恒温器
 * http://www.syyyd.com/forum.php?mod=viewthread&tid=2551
 * 旋转编码器、lcd显示、温度数据记录
*/

# include <LiquidCrystal.h>

# define beta 4090
# define resistance 33

//LiquidCrystal display with:
//rs->12  rw->11  enable->10  d4-7 -> 5-2

LiquidCrystal lcd(12,11,10,5,4,3,2);

int ledPin = 15;
int relayPin = 16;
int aPin = 8;
int bPin = 7;
int buttonPin = 6;
int analogPin = 0;

float setTemp = 20.0;
float measuredTemp;
char mode = 'C';
boolean override = false;
float hysteresis = 0.25;

void setup()
{
  lcd.begin(2, 20);
  pinMode(ledPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(aPin, INPUT);
  pinMode(bPin, INPUT);
  pinMode(buttonPin, INPUT);
  lcd.clear();
}

void loop()
{
  static int count = 0;
  measuredTemp = readTemp();
  if(digitalRead(buttonPin))
  {
    override = !override;
    updateDisplay();
    delay(500);  //debounce去抖
  }
  int change = getEncoderTurn();
  setTemp =setTemp + change * 0.1;
  if(count == 1000)
  {
    updateDisplay();
    updateOutputs();
    count = 0;
  }
  count ++;
}

int getEncoderTurn()
{
  static int oldA = LOW;
  static int oldB = LOW;
  int result = 0;
  int newA = digitalRead(aPin);
  int newB = digitalRead(bPin);
  if(newA != oldA || newB != oldB)
  {
    //something has changed
    if(oldA == LOW && newA == HIGH)
    {
      result = -(oldB * 2 - 1);
    }
  }
  oldA = newA;
  oldB = newB;
  return result;
}

float readTemp()
{
  long a = analogRead(analogPin);
  float temp = beta / (log(((1025.0 * resistance / a) - 33.0) / 33.0) + (beta / 298.0)) - 273.0;
  return temp;
}

void updateOutputs()
{
  if(override || measuredTemp < setTemp - hysteresis)  //hysteresis为摆动值 防止继电器频繁跳动
  {
    digitalWrite(ledPin, HIGH);
    digitalWrite(relayPin, HIGH);
  }
  else if(!override && measuredTemp > setTemp + hysteresis)  //hysteresis为摆动值 防止继电器频繁跳动
  {
    digitalWrite(ledPin, LOW);
    digitalWrite(relayPin, LOW);
  }
}

void updateDisplay()
{
  lcd.setCursor(0, 0);
  lcd.print("Actual:");
  lcd.print(adjustUnits(measuredTemp));
  lcd.print("℃");
  lcd.print(mode);
  lcd.print(" ");

  lcd.setCursor(0, 1);
  if(override)
  {
    lcd.print("OVERRIDE ON");
  }
  else
  {
    lcd.print("Set:");
    lcd.print(adjustUnits(setTemp));
    lcd.print("℃");
    lcd.print(mode);
    lcd.print(" ");
  }
}

float adjustUnits(float temp)
{
  if(mode == 'C')
  {
    return temp;
  }
  else
  {
    return(temp * 9) / 5 + 32;
  }
}

