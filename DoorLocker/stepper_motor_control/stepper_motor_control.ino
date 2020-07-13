#define step_pin 24
#define dir_pin 22
#define en_pin 28
#define ms1_pin 23
#define ms2_pin 25
#define ms3_pin 27

#define motor_steps 200
#define left 0
#define right 1

void setup() {
  // put your setup code here, to run once:
  pinMode(step_pin, OUTPUT);           // set pin to input
  digitalWrite(step_pin, LOW);       // turn on pullup resistors

  pinMode(dir_pin, OUTPUT);           // set pin to input
  digitalWrite(dir_pin, LOW);       // turn on pullup resistors

  pinMode(en_pin, OUTPUT);           // set pin to input
  digitalWrite(en_pin, HIGH);       // turn on pullup resistors

  pinMode(ms1_pin, OUTPUT);           // set pin to input
  digitalWrite(ms1_pin, LOW);       // turn on pullup resistors

  pinMode(ms2_pin, OUTPUT);           // set pin to input
  digitalWrite(ms2_pin, LOW);       // turn on pullup resistors

  pinMode(ms3_pin, OUTPUT);           // set pin to input
  digitalWrite(ms3_pin, LOW);       // turn on pullup resistors
  
}

void loop() {
  // put your main code here, to run repeatedly:
  stepper_move(300, left);

  delay(1000);

  stepper_move(300, right);

  delay(1000);
  //delay(1000);
//  digitalWrite(step_pin, HIGH);
//  delay(200);
//  digitalWrite(step_pin, LOW);
//  delay(200);
}

void stepper_move(int steps, int dir) //dir: left=0, right=1
{
  float full_p = 0.5;
  float half_p = 0.2;
  float quarter_p = 0.15;
  float eighth_p = 0.1;
  float sixteenth_p = 0.05;

  int full_s = full_p * steps;
  int half_s = half_p * steps * 2;
  int quarter_s = quarter_p * steps * 4;
  int eighth_s = eighth_p * steps * 8;
  int sixteenth_s = sixteenth_p * steps * 16;
  
  digitalWrite(en_pin, LOW); 
  digitalWrite(dir_pin, dir);
  delay(10);

  set_sixteenth_step();
  move(sixteenth_s/2);
  set_eighth_step();
  move(eighth_s/2);
  set_quarter_step();
  move(quarter_s/2);
  set_half_step();
  move(half_s/2);
  set_full_step();
  move(full_s);
  set_half_step();
  move(half_s/2);
  set_quarter_step();
  move(quarter_s/2);
  set_eighth_step();
  move(eighth_s/2);
  set_sixteenth_step();
  move(sixteenth_s/2);
  
  digitalWrite(en_pin, HIGH); 
}

void move(int steps)
{
  int delay_ms = 2;
  for(int i = 0; i < steps; i++)
  {
    
    digitalWrite(step_pin, HIGH);
    digitalWrite(step_pin, LOW);
    
    delay(delay_ms);
    
  }
}

void set_full_step()
{
  digitalWrite(ms1_pin, LOW);
  digitalWrite(ms2_pin, LOW);
  digitalWrite(ms3_pin, LOW);
}

void set_half_step()
{
  digitalWrite(ms1_pin, HIGH);
  digitalWrite(ms2_pin, LOW);
  digitalWrite(ms3_pin, LOW);
}

void set_quarter_step()
{
  digitalWrite(ms1_pin, LOW);
  digitalWrite(ms2_pin, HIGH);
  digitalWrite(ms3_pin, LOW);
}

void set_eighth_step()
{
  digitalWrite(ms1_pin, HIGH);
  digitalWrite(ms2_pin, HIGH);
  digitalWrite(ms3_pin, LOW);
}

void set_sixteenth_step()
{
  digitalWrite(ms1_pin, HIGH);
  digitalWrite(ms2_pin, HIGH);
  digitalWrite(ms3_pin, HIGH);
}
