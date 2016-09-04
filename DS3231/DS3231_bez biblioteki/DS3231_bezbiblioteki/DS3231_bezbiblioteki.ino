/*******************************************************************************************************
void ds3231_alarm_irq () - Alarm Interrupt from DS3231 - SQW output 
void ds3231_irq_arm (boolean ds3231_irq) - All interrupt enable INTCN = 1, disable INTCN = 0
void ds3231_oscillator_on () - Enable Oscillator EOSC = 0
void ds3231_clear_alarm_1_irq () - Clear Alarm 1 Flag A1F = 0
void ds3231_alarm_1_arm (boolean alarm_arm) - 
void ds3231_alarm_1_set (byte minutes) - 
void ds3231_disable_bbsqw () - 
void ds3231_disable_en32khz_output () - 
void ds3231_clear_running_flag () - 
boolean ds3231_check_running_flag () - 
void ds3231_get_time () - 
void ds3231_send_time (byte years, byte months, byte date, byte hours, byte minutes, byte seconds) - 
*******************************************************************************************************/
#include <Wire.h>
 
#define DS3231_IRQ_PIN        11 
#define DS3231_I2C_ADDRESS    0x68

//I2C Slave Address  
#define DS3231_ADDRESS 0x68  

//DS3231 Register Addresses
#define DS3231_REG_TIMEDATE   0x00
#define DS3231_REG_ALARM1     0x07
#define DS3231_REG_ALARM2     0x0B

#define DS3231_REG_CONTROL    0x0E
#define DS3231_REG_STATUS     0x0F
#define DS3231_REG_AGING      0x10

// DS3231 Control Register Bits
#define DS3231_A1IE       0
#define DS3231_A2IE       1
#define DS3231_INTCN      2
#define DS3231_RS1        3
#define DS3231_RS2        4
#define DS3231_CONV       5
#define DS3231_BBSQW      6
#define DS3231_EOSC       7

// DS3231 Status Register Bits
#define DS3231_A1F        0
#define DS3231_A2F        1
#define DS3231_BSY        2
#define DS3231_EN32KHZ    3
#define DS3231_OSF        7    // Oscillator Stop Flag 


byte seconds, minutes, hours, days, date, months, years = 0; 
byte new_seconds, new_minutes, new_hours, new_days, new_date, new_months, new_years = 0;
boolean ds3231_running_flag = true;
boolean alarm_arm = false;
boolean ds3231_irq = false;


/******************************************************************************************************/
void setup()
{
  Wire.begin();
  Serial.begin(9600);
  
  // Sometimes necessary to ensure that the clock keeps running on just battery power.
  // Once set, it shouldn't need to be reset but it's a good idea to make sure.
  // Clear EOSC bit
  // ds3231_oscillator_on ();  
  
  /* Setup the IRQ pin for SQW DS3231 Alarm*/
  pinMode(DS3231_IRQ_PIN, INPUT_PULLUP);
  /*Activate the interrupt*/
  attachInterrupt(DS3231_IRQ_PIN, ds3231_alarm_irq, FALLING);
  ds3231_send_time(16, 12, 31, 4,0,0);
  ds3231_oscillator_on ();
  ds3231_disable_en32khz_output ();
  ds3231_alarm_1_set(1);
  ds3231_alarm_1_arm (true);
  ds3231_irq_arm (true);
}
/******************************************************************************************************/

/***********************************************************************************************************************************************************/
/***********************************************************************************************************************************************************/
/***********************************************************************************************************************************************************/
void loop()
{  
    ds3231_get_time (); // Read Time from DS3231
    
    Serial.print(hours); Serial.print(":"); Serial.print(minutes); Serial.print(":"); Serial.println(seconds);  
    Serial.print(years); Serial.print("/"); Serial.print(months); Serial.print("/"); Serial.println(date);
    Serial.println();
 
    //ds3231_oscillator_on();
    if(!(ds3231_check_running_flag ()))
    {
      Serial.println("Clear Running Flag"); 
      ds3231_clear_running_flag ();  
    }
    
    delay(10000);
}
/***********************************************************************************************************************************************************/
/***********************************************************************************************************************************************************/
/***********************************************************************************************************************************************************/

/******************************************************************************************************/
// Send Time to DS3231  
void ds3231_send_time (byte years, byte months, byte date, byte hours, byte minutes, byte seconds)
{
  // send request to receive data starting at register 0
  Wire.beginTransmission(DS3231_I2C_ADDRESS); // 0x68 is DS3231 device address
  Wire.write((byte)0); // start at register 0
       
  seconds = (((seconds / 10) * 16) + (seconds % 10)); // convert decimal to BCD
  minutes = (((minutes / 10) * 16) + (minutes % 10)); // convert decimal to BCD
  hours = (((hours / 10) * 16) + (hours % 10)); // convert decimal to BCD
  date = (((date / 10) * 16) + (date % 10)); // convert decimal to BCD
  months = (((months / 10) * 16) + (months % 10)); // convert decimal to BCD
  years = (((years / 10) * 16) + (years % 10)); // convert decimal to BCD
          
  Wire.write((byte)seconds);  // send seconds
  Wire.write((byte)minutes);  // send minutes
  Wire.write((byte)hours);    // send hours
  Wire.write((byte)0);        // send days
  Wire.write((byte)date);     // send date   
  Wire.write((byte)months);   // send months
  Wire.write((byte)years);    // send years
      
  Wire.endTransmission();
}
/******************************************************************************************************/

/******************************************************************************************************/
// Read Time from DS3231  
void ds3231_get_time ()
{
  // send request to receive data starting at register 0
  Wire.beginTransmission(DS3231_I2C_ADDRESS); // 0x68 is DS3231 device address
  Wire.write((byte)0); // start at register 0
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7); // request seven bytes (seconds, minutes, hours, days,date, months, years)
 
  seconds = Wire.read();  // get seconds
  minutes = Wire.read();  // get minutes
  hours = Wire.read();    // get hours
  days = Wire.read();     // get days
  date = Wire.read();     // get date   
  months = Wire.read();   // get months
  years = Wire.read();    // get years

  seconds = (((seconds & 0b11110000)>>4)*10 + (seconds & 0b00001111)); // convert BCD to decimal
  minutes = (((minutes & 0b11110000)>>4)*10 + (minutes & 0b00001111)); // convert BCD to decimal
  hours = (((hours & 0b00100000)>>5)*20 + ((hours & 0b00010000)>>4)*10 + (hours & 0b00001111)); // convert BCD to decimal (assume 24 hour mode)
  date = (((date & 0b11110000)>>4)*10 + (date & 0b00001111)); // convert BCD to decimal
  months = (((months & 0b00010000)>>4)*10 + (months & 0b00001111)); // convert BCD to decimal
  years = (((years & 0b11110000)>>4)*10 + (years & 0b00001111)); // convert BCD to decimal    
}
/******************************************************************************************************/

/******************************************************************************************************/
/* Oscillator Stop Flag (OSF). A logic 1 in this bit indicates that the oscillator either is stopped 
or was stopped for some period and may be used to judge the validity of the timekeeping data. 
This bit is set to logic 1 any time that the oscillator stops. The following are examples of conditions 
that can cause the OSF bit to be set:
  1) The first time power is applied.
  2) The voltages present on both VCC and VBAT are insufficient to support oscillation.
  3) The EOSC bit is turned off in battery-backed mode.
  4) External influences on the crystal (i.e., noise, leakage, etc.).*/
  
boolean ds3231_check_running_flag ()
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS); // address DS3231
  Wire.write(DS3231_REG_STATUS); // select control register
  Wire.endTransmission();
  
  Wire.requestFrom(DS3231_I2C_ADDRESS, 1); 
  byte status_register = Wire.read(); 
  
    if((status_register) & (0b10000000)) // If OSF = 1 there was an error 
    {
      return 0; // Oscillator stops, error
    }
    else
    {
      return 1; // Everythig is ok, oscillator running
    }
}
/******************************************************************************************************/

/******************************************************************************************************/
void ds3231_clear_running_flag ()
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS); // address DS3231
  Wire.write(DS3231_REG_STATUS); // select status register
  Wire.endTransmission();
  
  Wire.requestFrom(DS3231_I2C_ADDRESS, 1); 
  
  byte status_register = (Wire.read())&(0b01111111); // Read status register and clear OSF bit, error oscillator flag 
  
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(DS3231_REG_STATUS); // Send new status register with OSF = 0, clear error oscillator flag
  Wire.write(status_register);
  Wire.endTransmission(); 
}
/******************************************************************************************************/

/******************************************************************************************************/
/* Enable 32kHz Output (EN32kHz). This bit controls the status of the 32kHz pin. When set to logic 1,
the 32kHz pin is enabled and outputs a 32.768kHz square-wave signal. When set to logic 0, 
the 32kHz pin goes to a high-impedance state. The initial power-up state of this bit is logic 1, 
and a 32.768kHz square-wave signal appears at the 32kHz pin after a power source is applied to 
the DS3231 (if the oscillator is running).*/

void ds3231_disable_en32khz_output ()
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS); // address DS3231
  Wire.write(DS3231_REG_STATUS); // select status register
  Wire.endTransmission();
  
  Wire.requestFrom(DS3231_I2C_ADDRESS, 1); 
  
  byte status_register = (Wire.read())&(0b11110111); // Read status register and clear EN32kHz bit, disable output EN32kHz 
  
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(DS3231_REG_STATUS); // Send new status register with EN32kHz = 0, disable output EN32kHz
  Wire.write(status_register);
  Wire.endTransmission();  
}
/******************************************************************************************************/

/******************************************************************************************************/
/* Battery-Backed Square-Wave Enable(BBSQW). When set to logic 1 and the DS3231 is being
powered by the VBAT pin, this bit enables the squarewave or interrupt output when VCC is absent. 
When BBSQW is logic 0, the INT/SQW pin goes high impedance when VCC falls below the power-fail 
trip point. This bit is disabled (logic 0) when power is first applied.*/

void ds3231_disable_bbsqw ()
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS); // address DS3231
  Wire.write(DS3231_REG_STATUS); // select status register
  Wire.endTransmission();
  
  Wire.requestFrom(DS3231_I2C_ADDRESS, 1); 
  
  byte status_register = (Wire.read())&(0b10111111); // Read status register and clear BBSQW bit, disable Battery-Backed Square-Wave 
  
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(DS3231_REG_STATUS); // Send new status register with BBSQW = 0, disable Battery-Backed Square-Wave
  Wire.write(status_register);
  Wire.endTransmission(); 
}
/******************************************************************************************************/

/******************************************************************************************************/
void ds3231_alarm_1_set (byte minutes)
{
  // send request to receive data starting at register 0
  Wire.beginTransmission(DS3231_I2C_ADDRESS); // 0x68 is DS3231 device address
  Wire.write(DS3231_REG_ALARM1); // start at register 0
  
  byte seconds = random(0, 59);     
  seconds = (((seconds / 10) * 16) + (seconds % 10)); // convert decimal to BCD
  seconds &= 0b01111111; 
  minutes = (((minutes / 10) * 16) + (minutes % 10)); // convert decimal to BCD
  minutes &= 0b01111111;
          
  Wire.write((byte)0);  // send seconds
  Wire.write((byte)minutes);  // send minutes
  Wire.write((byte)(0b10000000));  // hours
  Wire.write((byte)(0b10000000));  // days      
  Wire.endTransmission();  
  
}
/******************************************************************************************************/

/******************************************************************************************************/
void ds3231_alarm_1_arm (boolean alarm_arm)
{    
  Wire.beginTransmission(DS3231_I2C_ADDRESS); // address DS3231
  Wire.write(DS3231_REG_CONTROL); // select control register
  Wire.endTransmission();
  
  Wire.requestFrom(DS3231_I2C_ADDRESS, 1); //1 byte request
  
  byte status_register = Wire.read(); // Read status register and    
  
    if (alarm_arm)
    {
        status_register |= 0b00000001; // A1IE = 1, Alarm 1 interrupt on
    } 
    else
    {
        status_register &= 0b11111110; // A1IE = 0, Alarm 1 interrupt off
    }

  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(DS3231_REG_CONTROL); // Send new status register with 
  Wire.write(status_register);
  Wire.endTransmission();  
 
  //this part cleared A1F interrupt flag in status register 
  ds3231_clear_alarm_1_irq (); 
}
/******************************************************************************************************/

/******************************************************************************************************/
/* Alarm 1 Flag (A1F). A logic 1 in the alarm 1 flag bit indicates that the time matched the 
alarm 1 registers. If the A1IE bit is logic 1 and the INTCN bit is set to logic 1, the INT/SQW pin 
is also asserted. A1F is cleared when written to logic 0. This bit can only be written to logic 0. 
Attempting to write to logic 1 leaves the value unchanged.*/

void ds3231_clear_alarm_1_irq ()
{
  //this part cleared A1F interrupt flag in status register 
  Wire.beginTransmission(DS3231_I2C_ADDRESS); // address DS3231
  Wire.write(DS3231_REG_STATUS); // select status register
  Wire.endTransmission();
  
  Wire.requestFrom(DS3231_I2C_ADDRESS, 1); 
  
  byte status_register = (Wire.read())&(0b11111110); // A1F = 0, Clear Alarm 1 interrupt flag 
  
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(DS3231_REG_STATUS); // Send new status register with A1F = 0
  Wire.write(status_register);
  Serial.println(status_register);
  Wire.endTransmission();     
}

/******************************************************************************************************/
/* Enable Oscillator (EOSC). When set to logic 0, the oscillator is started. When set to logic 1, 
the oscillator is stopped when the DS3231 switches to VBAT. This bit is clear (logic 0) 
when power is first applied. When the DS3231 is powered by VCC, the oscillator is always on
regardless of the status of the EOSC bit. When EOSC is disabled, all register data is static.

Sometimes necessary to ensure that the clock keeps running on just battery power.
Once set, it shouldn't need to be reset but it's a good idea to make sure and clear EOSC bit. */

void ds3231_oscillator_on ()
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS); // address DS3231
  Wire.write(DS3231_REG_CONTROL); // select control register
  Wire.endTransmission();
  
  Wire.requestFrom(DS3231_I2C_ADDRESS, 1); 
  
  byte status_register = (Wire.read())&(0b01111111); // Read status register and clear EOSC bit, enable oscillator 
  
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(DS3231_REG_CONTROL); // Send new status register with EOSC = 0, enable oscillator
  Wire.write(status_register);
  Wire.endTransmission(); 
}
/******************************************************************************************************/

/******************************************************************************************************/
/*Interrupt Control (INTCN). This bit controls the INT/SQW signal. When the INTCN bit is set to logic 0,
a square wave is output on the INT/SQW pin. When the INTCN bit is set to logic 1, then a match between 
the timekeeping registers and either of the alarm registers activates the INT/SQW output (if the alarm 
is also enabled). The corresponding alarm flag is always set regardless of the state of the INTCN bit. 
The INTCN bit is set to logic 1 when power is first applied.*/

void ds3231_irq_arm (boolean ds3231_irq)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS); // address DS3231
  Wire.write(DS3231_REG_CONTROL); // select control register
  Wire.endTransmission();
  
  Wire.requestFrom(DS3231_I2C_ADDRESS, 1); 
  
  byte status_register = Wire.read(); // Read status register 
    
    if (ds3231_irq)
    {
        status_register |= 0b00000100; // Interrupt enable INTCN = 1
    } 
    else
    {
        status_register &= 0b11111011; // Interrupt disable INTCN = 1 
    }
    
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(DS3231_REG_CONTROL); // Send new status register with INTCN
  Wire.write(status_register);
  Wire.endTransmission(); 
}
/******************************************************************************************************/

/******************************************************************************************************/
//Alarm Interrupt from DS3231 SQW  
void ds3231_alarm_irq ()
{
  Serial.println("Alarm IRQ");
  ds3231_clear_alarm_1_irq ();
}
/******************************************************************************************************/

