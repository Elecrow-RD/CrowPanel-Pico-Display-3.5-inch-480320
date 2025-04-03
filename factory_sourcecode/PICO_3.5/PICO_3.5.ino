/*Using LVGL with Arduino requires some extra steps:
  Be sure to read the docs here: https://docs.lvgl.io/master/get-started/platforms/arduino.html  */
#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <../lvgl/src/demos/lv_demos.h>
#include "ui.h"
#include <Ticker.h> 
#include <hardware/watchdog.h>
static int first_flag = 0;
extern int zero_clean;
extern int goto_widget_flag;
extern int bar_flag;
extern lv_obj_t * ui_MENU;
extern lv_obj_t * ui_TOUCH;
extern lv_obj_t * ui_JIAOZHUN;
extern lv_obj_t * ui_Label2;
static lv_obj_t * ui_Label;//TOUCH界面label
static lv_obj_t * ui_Label3;//TOUCH界面label3
static lv_obj_t * ui_Labe2;//Menu界面进度条label
static lv_obj_t * bar;//Menu界面进度条
static int val = 100;
/*To use the built-in examples and demos of LVGL uncomment the includes below respectively.
  You also need to copy `lvgl/examples` to `lvgl/src/examples`. Similarly for the demos `lvgl/demos` to `lvgl/src/demos`.
  Note that the `lv_examples` library is for LVGL v7 and you shouldn't install it for this version (since LVGL v8)
  as the examples and demos are now part of the main LVGL library. */
const int _MISO = 12;  // AKA SPI RX
const int _MOSI = 11;  // AKA SPI TX
const int _CS = 22;
const int _SCK = 10;

// include the SD library:
#include <SPI.h>
#include <SD.h>

File root;

#include <Wire.h>
#include "DHT20.h"

DHT20 DHT(&Wire);
DHT20 dht(&Wire1);
#define I2C0_SDA 20
#define I2C0_SCL 21
#define I2C1_0_SDA 2
#define I2C1_0_SCL 3
#define I2C1_1_SDA 6
#define I2C1_1_SCL 7
//串口TX，RX
#define  Serial1_RX 1
#define  Serial1_TX 0
#define  Serial2_RX 5
#define  Serial2_TX 4

//tft背光引脚
#define  LCD_BL 18
#define  IO_LED 19
#define  Uart_LED 25

//无源蜂鸣器
#define Buzzwr_Pin 14
//频率
long frequency = 2700;

//ADC
#define ADC0 26
#define ADC1 27
#define ADC2 28
uint8_t Count = 0;
char CloseData;
char CloseData1;
int NO_Test_Flag = 0;
int Test_Flag = 0;
int Close_Flag = 0;

int touch_flag = 0;
unsigned char buffer[256]; // buffer array for data recieve over serial port
int count = 0;   // counter for buffer array
void clearBufferArray()              // function to clear buffer array
{
  for (int i = 0; i < count; i++)
  {
    buffer[i] = NULL; // clear all index of array with command NULL
  }
}
/*Change to your screen resolution*/
static const uint16_t screenWidth  = 480;
static const uint16_t screenHeight = 320;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[ screenWidth * screenHeight / 8 ];
//static lv_color_t buf2[ screenWidth * screenHeight / 8 ];
TFT_eSPI tft = TFT_eSPI(); /* TFT instance screenWidth, screenHeight */

#if LV_USE_LOG != 0
/* Serial1 debugging */
void my_print(const char * buf)
{
  Serial1.printf(buf);
  Serial1.flush();
}
#endif

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p )
{
  uint32_t w = ( area->x2 - area->x1 + 1 );
  uint32_t h = ( area->y2 - area->y1 + 1 );

  tft.startWrite();
  tft.setAddrWindow( area->x1, area->y1, w, h );
  tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
  tft.endWrite();

  lv_disp_flush_ready( disp_drv );
}

uint16_t touchX, touchY;
/*Read Touchpad*/
void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
//  data->state = LV_INDEV_STATE_REL;
  bool touched = tft.getTouch( &touchX, &touchY, 600);
  if ( !touched )
  {
    data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    data->state = LV_INDEV_STATE_PR;

    /*Setting the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;

    Serial1.print( "Data x " );
    Serial1.println( touchX );

    Serial1.print( "Data y " );
    Serial1.println(  touchY);

    
    Serial.print( "Data x " );
    Serial.println( touchX );

    Serial.print( "Data y " );
    Serial.println(  touchY);
  }
}
uint16_t calData[5] =  { 192, 3590, 335, 3362, 1 };
void callback1()  //Callback function
{
  if (bar_flag == 6)
  {
    if (val > 1)
    {
      val--;
      lv_bar_set_value(bar, val, LV_ANIM_OFF);
      lv_label_set_text_fmt(ui_Labe2, "%d %%", val);
      if(val==0)
        val=100;
    }
    else
    {
      lv_obj_clear_flag(ui_touch, LV_OBJ_FLAG_CLICKABLE);
      lv_label_set_text(ui_Labe2, "Loading");
      delay(150);
      val = 100;
      bar_flag = 0; //停止进度条标志
      goto_widget_flag = 1; //进入widget标志

    }
  }
}
Ticker ticker1(callback1, 30, 0, MILLIS);
void setup()
{

  Serial1.setRX(Serial1_RX);
  Serial1.setTX(Serial1_TX);
  Serial1.begin( 115200 ); /* prepare for possible serial debug */
  Serial2.setRX(Serial2_RX);
  Serial2.setTX(Serial2_TX);
  Serial2.begin( 115200 );
  Serial.begin( 115200 );
  String LVGL_Arduino = "Hello Arduino! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

  Serial1.println( LVGL_Arduino );
  Serial1.println( "I am LVGL_Arduino" );
  

  
  tft.begin();          /* TFT init */
  tft.setRotation( 1); /* Landscape orientation, flipped */
  tft.invertDisplay(false);
  /*Set the touchscreen calibration data,
    the actual data for your display can be acquired using
    the Generic -> Touch_calibrate example from the TFT_eSPI library*/
  tft.setTouch(calData);
//  touch_calibrate();
  lv_init();
//  delay(1000);

#if LV_USE_LOG != 0
  lv_log_register_print_cb( my_print ); /* register print function for debugging */
#endif

  lv_disp_draw_buf_init( &draw_buf, buf1, NULL, screenWidth * screenHeight / 8 );

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init( &disp_drv );
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register( &disp_drv );

  /*Initialize the (dummy) input device driver*/
      static lv_indev_drv_t indev_drv;
      lv_indev_drv_init( &indev_drv );
      indev_drv.type = LV_INDEV_TYPE_POINTER;
      indev_drv.read_cb = my_touchpad_read;
      lv_indev_drv_register( &indev_drv );

  /* Create simple label */
//  lv_obj_t *label = lv_label_create( lv_scr_act() );
//  lv_label_set_text( label, "Hello Ardino and LVGL!");
//  lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );

  /* Try an example. See all the examples
     online: https://docs.lvgl.io/master/examples.html
     source codes: https://github.com/lvgl/lvgl/tree/e7f88efa5853128bf871dde335c0ca8da9eb7731/examples */
  //lv_example_btn_1();
  
  tft.fillScreen(TFT_BLACK);
  delay(300);
  pinMode(Uart_LED,OUTPUT);
  digitalWrite(Uart_LED, HIGH);
  pinMode(LCD_BL,OUTPUT);
  digitalWrite(LCD_BL, HIGH);
//  analogWrite(LCD_BL,255);
  

   ui_init();//开机UI界面

   while(1)
  {
        if (goto_widget_flag == 1)//进入widget
    {
      if (ticker1.state() == RUNNING)
      {
        ticker1.stop();
      }
      goto_widget_flag = 0;
      delay(300);
      break;
    }
        if (goto_widget_flag == 3)//进入触摸界面，先把进度条线程关闭
    {
      bar_flag = 0; //停止进度条标志
      if (ticker1.state() == RUNNING)
      {
        ticker1.stop();
      }
      if (first_flag == 0 || first_flag == 1)
      {
        label_xy();
        first_flag = 2;
      }
      if (zero_clean == 1)
      {
        touchX = 0;
        touchY = 0;
        zero_clean = 0;
      }
      lv_label_set_text(ui_Label, "Touch Adjust:");
      lv_label_set_text_fmt(ui_Label3, "%d  %d", touchX, touchY); //显示触摸信息
    }
    if (goto_widget_flag == 4)//触摸界面返回到Menu界面,使进度条加满
    {
      val = 100;
      delay(100);
      ticker1.resume();//每35ms调用callback1
      goto_widget_flag = 0;
    }
    if (goto_widget_flag == 5) //触发校准信号
    {
      lv_scr_load_anim(ui_touch_calibrate, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
      lv_timer_handler();
      lv_timer_handler();
      delay(100);
      touch_calibrate();//触摸校准
      tft.setTouch( calData );
      lv_scr_load_anim(ui_TOUCH, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
      lv_timer_handler();
      delay(100);
      goto_widget_flag = 3; //进入触摸界面标志
      touchX = 0;
      touchY = 0;
    }
    if (bar_flag == 6)//刚开机进入Menu界面时运行进度条一次，之后就不再运行
    {
      if (first_flag == 0)
      {
        lv_example_bar();
        ticker1.start();//每35ms调用callback1
        first_flag = 1;
      }
    }
    ticker1.update();
    lv_timer_handler();
  }   
  tft.fillScreen(TFT_BLACK);
  /*Or try out a demo. Don't forget to enable the demos in lv_conf.h. E.g. LV_USE_DEMOS_WIDGETS*/
  lv_demo_widgets();
  // lv_demo_benchmark();
  // lv_demo_keypad_encoder();
  // lv_demo_music();
  // lv_demo_printer();
  // lv_demo_stress();
  


  Serial.println( "Setup done" );
}

void loop()
{
   //延迟打开背光
//  digitalWrite(18, HIGH);



  
  while (NO_Test_Flag == 0)
  {
    lv_timer_handler();
    delay(10);
    CloseData1 = Serial.read();
    if (CloseData1 == 'b')  //Exit
    {
      NO_Test_Flag = 1;
      Test_Flag = 1;
      tft.fillScreen(TFT_BLACK);

      Serial.println("Access to the test program");
      break;
    }
  }

  //*********************************Test Program***************************************
  //*********************************Test Program***************************************
  //*********************************Test Program***************************************
  while (Test_Flag == 1)
  {
    Ce_shi();
  }
  //强制刷新，避免黑屏
  lv_timer_handler();
  lv_obj_invalidate(lv_scr_act()); 



}

void Ce_shi() {
  char serialData;
  if (Serial.available() > 0) {
      serialData = Serial.read();
  }

  switch (serialData) {
    case 'R':
      Close_Flag = 1;
      //      WiFi.disconnect();//Disconnect
      tft.fillScreen(TFT_BLACK);
      Serial1.println("Red Screen");
      Serial.println("Red Screen");
      while (Close_Flag == 1)
      {
        tft.fillScreen(TFT_RED);
        CloseData = Serial1.read();
        CloseData1 = Serial.read();
        if (CloseData == 'I'||CloseData1 == 'I')  //Exit
        {
          Close_Flag = 0;
          tft.fillScreen(TFT_BLACK);
          Serial1.println("Exit Red Screen");
          Serial.println("Exit Red Screen");
          break;
        }
      }
      break;

    case 'G':
      Close_Flag = 1;
      tft.fillScreen(TFT_BLACK);
      Serial1.println("Green Screen");
      Serial.println("Green Screen");
      while (Close_Flag == 1)
      {
        tft.fillScreen(TFT_GREEN);
        CloseData = Serial1.read();
        CloseData1 = Serial.read();
        if (CloseData == 'I'||CloseData1 == 'I')  //Exit
        {
          Close_Flag = 0;
          tft.fillScreen(TFT_BLACK);
          Serial1.println("Exit Green Screen");
          Serial.println("Exit Green Screen");
          break;
        }
      }
      break;

    case 'B':
      Close_Flag = 1;
      tft.fillScreen(TFT_BLACK);
      Serial1.println("Blue Screen");
      Serial.println("Blue Screen");
      while (Close_Flag == 1)
      {
        tft.fillScreen(TFT_BLUE);
        CloseData = Serial1.read();
        CloseData1 = Serial.read();
        if (CloseData == 'I'||CloseData1 == 'I')  //Exit
        {
          Close_Flag = 0;
          tft.fillScreen(TFT_BLACK);
          Serial1.println("Exit blue Screen");
          Serial.println("Exit blue Screen");
          break;
        }
      }
      break;
    case 'w':
      Close_Flag = 1;
      tft.fillScreen(TFT_BLACK);
      Serial1.println("White Screen");
      Serial.println("White Screen");
      while (Close_Flag == 1)
      {
        tft.fillScreen(TFT_WHITE);
        CloseData = Serial1.read();
        CloseData1 = Serial.read();
        if (CloseData == 'I'||CloseData1 == 'I')  //Exit
        {
          Close_Flag = 0;
          tft.fillScreen(TFT_BLACK);
          Serial1.println("Exit White Screen");
          Serial.println("Exit White Screen");
          break;
        }
      }
      break;

    case 'h':
      Close_Flag = 1;
      tft.fillScreen(TFT_BLACK);
      Serial1.println("Gray Screen");
      Serial.println("Gray Screen");
      while (Close_Flag == 1)
      {
        tft.fillScreen(TFT_LIGHTGREY);
        CloseData = Serial1.read();
        CloseData1 = Serial.read();
        if (CloseData == 'I'||CloseData1 == 'I')  //Exit
        {
          Close_Flag = 0;
          tft.fillScreen(TFT_BLACK);
          Serial1.println("Exit Gray Screen");
          Serial.println("Exit Gray Screen");
          break;
        }
      }
      break;
    case 'S':
      Close_Flag = 1;
      tft.fillScreen(TFT_BLACK);
      Serial.println("TF card init");
      SD_test();
      while (Close_Flag == 1)
      {
        CloseData1 = Serial.read();
        if (CloseData1 == 'I')  //Exit
        {
          Close_Flag = 0;
          tft.fillScreen(TFT_BLACK);
          Serial.println("exit TF card init");
          watchdog_reboot(0, 0, 0);
          break;
        }
      }
      break;

    case 'T':
      Close_Flag = 1;
      tft.fillScreen(TFT_BLACK);
      Serial1.println("Touch Screen");
      Serial.println("Touch Screen");
      delay(100);
      touch_flag = 0;
      tft.setCursor(70, 70, 4);
      tft.printf("Please Touch");
      Serial1.println("TOUCH");
      Serial.println("TOUCH");
      delay(100);
      while (Close_Flag == 1)
      {
        uint16_t x, y;
        if (touch_flag < 3)
        {
          if (tft.getTouch(&x, &y))
          {
            if (x > 0 && x < 60 && y > 0 && y < 60 && touch_flag == 0)
            {
              tft.setCursor(140, 120, 4);
              tft.printf("               ");
              tft.setCursor(140, 140, 4);
              tft.printf("               ");
              tft.setCursor(120, 120, 4);
              tft.printf("x: %i     ", x);
              tft.setCursor(120, 140, 4);
              tft.printf("y: %i    ", y);
              Serial1.print( "Data x " );
              Serial1.println( x );
              Serial1.print( "Data y " );
              Serial1.println( y );
              
              Serial.print( "Data x " );
              Serial.println( x );
              Serial.print( "Data y " );
              Serial.println( y );
              touch_flag = 1;
            }
            if (x > 275 && x < 320 && y > 185 && y < 240 && touch_flag == 1)
            {
              tft.setCursor(140, 120, 4);
              tft.printf("               ");
              tft.setCursor(140, 140, 4);
              tft.printf("               ");
              tft.setCursor(120, 120, 4);
              tft.printf("x: %i     ", x);
              tft.setCursor(120, 140, 4);
              tft.printf("y: %i    ", y);
              Serial1.print( "Data x " );
              Serial1.println( x );
              Serial1.print( "Data y " );
              Serial1.println( y );

              Serial.print( "Data x " );
              Serial.println( x );
              Serial.print( "Data y " );
              Serial.println( y );
              touch_flag = 3;
              Serial1.println("OVER");
              Serial.println("OVER");
              delay(100);
              Serial1.println("OVER");
              Serial.println("OVER");
            }
            if (x > 0 && x < 4000 && y > 0 && y < 4000)
            {
              tft.setCursor(140, 120, 4);
              tft.printf("               ");
              tft.setCursor(140, 140, 4);
              tft.printf("               ");
              tft.setCursor(120, 120, 4);
              tft.printf("x: %i     ", x);
              tft.setCursor(120, 140, 4);
              tft.printf("y: %i    ", y);
              Serial1.print( "Data x " );
              Serial1.println( x );
              Serial1.print( "Data y " );
              Serial1.println( y );

              Serial.print( "Data x " );
              Serial.println( x );
              Serial.print( "Data y " );
              Serial.println( y );
            }
            CloseData = Serial1.read();
            CloseData1 = Serial.read();
            if (CloseData == 'I'||CloseData1 == 'I')  //Exit
            {
              Close_Flag = 0;
              touch_flag = 0;
              tft.fillScreen(TFT_BLACK);
              break;
            }
            delay(80);
          }
        }
        if (touch_flag == 3)
        {
          Serial1.println("OVER");
          Serial.println("OVER");
          if (tft.getTouch(&x, &y))
          {
            tft.fillScreen(TFT_BLACK);
            if (x > 0 && x < 4000 && y > 0 && y < 4000)
            {
              tft.setCursor(140, 120, 4);
              tft.printf("               ");
              tft.setCursor(140, 140, 4);
              tft.printf("               ");
              tft.setCursor(120, 120, 4);
              tft.printf("x: %i     ", x);
              tft.setCursor(120, 140, 4);
              tft.printf("y: %i    ", y);
              Serial1.print( "Data x " );
              Serial1.println( x );
              Serial1.print( "Data y " );
              Serial1.println( y );

              Serial.print( "Data x " );
              Serial.println( x );
              Serial.print( "Data y " );
              Serial.println( y );
            }
          }
          delay(80);
        }
        CloseData = Serial1.read();
        CloseData1 = Serial.read();
        if (CloseData == 'I'||CloseData1 == 'I')  //Exit
        {
          Close_Flag = 0;
          tft.fillScreen(TFT_BLACK);
          Serial1.println("Exit Touch Screen");
          Serial.println("Exit Touch Screen");
          break;
        }

      }
      break;

    case 'L':
      Close_Flag = 1;
      Serial.println("IO Port Test");
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(50, 100, 4);
      tft.printf("IO port output testing");
      pinMode(IO_LED,OUTPUT);
      while (Close_Flag == 1)
      {
        digitalWrite(IO_LED, HIGH);//Pull high level to turn on the light
 
        CloseData1 = Serial.read();
        if (CloseData1 == 'I')  //Exit
        {
          Close_Flag = 0;
          digitalWrite(IO_LED, LOW);
          tft.fillScreen(TFT_BLACK);
 
          Serial.println("Exit IO port test");
          break;
        }
      }
      break;

    case 'i':
      Close_Flag = 1;
      Wire.setSDA(I2C0_SDA);
      Wire.setSCL(I2C0_SCL);
      Wire.begin();
      DHT.begin();    //  ESP32 default pins 21 22
 
      Serial1.println("IIC");
      Serial.println("IIC");
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(100, 100, 4);
      tft.printf("IIC testing");
      while (Close_Flag == 1)
      {
          if(millis() - DHT.lastRead() >= 1000)
          {
            //  READ DATA
            uint32_t start = micros();
            int status = DHT.read();
            uint32_t stop = micros();
        
            if ((Count % 10) == 0)
            {
              Count = 0;
              Serial1.println();
              Serial1.println("Type\tHumidity (%)\tTemp (°C)\tTime (µs)\tStatus");
              Serial.println();
              Serial.println("Type\tHumidity (%)\tTemp (°C)\tTime (µs)\tStatus");
            }
            Count++;
        
            Serial1.print("DHT20 \t");
            //  DISPLAY DATA, sensor has only one decimal.
            Serial1.print(DHT.getHumidity(), 1);
            Serial1.print("\t\t");
            Serial1.print(DHT.getTemperature(), 1);
            Serial1.print("\t\t");
            Serial1.print(stop - start);
            Serial1.print("\t\t");

            Serial.print("DHT20 \t");
            //  DISPLAY DATA, sensor has only one decimal.
            Serial.print(DHT.getHumidity(), 1);
            Serial.print("\t\t");
            Serial.print(DHT.getTemperature(), 1);
            Serial.print("\t\t");
            Serial.print(stop - start);
            Serial.print("\t\t");
            switch (status)
            {
              case DHT20_OK:
                Serial1.print("OK");
                Serial.print("OK");
                break;
              case DHT20_ERROR_CHECKSUM:
                Serial1.print("Checksum error");
                Serial.print("Checksum error");
                break;
              case DHT20_ERROR_CONNECT:
                Serial1.print("Connect error");
                Serial.print("Connect error");
                break;
              case DHT20_MISSING_BYTES:
                Serial1.print("Missing bytes");
                Serial.print("Missing bytes");
                break;
              case DHT20_ERROR_BYTES_ALL_ZERO:
                Serial1.print("All bytes read zero");
                Serial.print("All bytes read zero");
                break;
              case DHT20_ERROR_READ_TIMEOUT:
                Serial1.print("Read time out");
                Serial.print("Read time out");
                break;
              case DHT20_ERROR_LASTREAD:
                Serial1.print("Error read too fast");
                Serial.print("Error read too fast");
                break;
              default:
                Serial1.print("Unknown error");
                Serial.print("Unknown error");
                break;
            }
            Serial1.print("\n");
            Serial.print("\n");
          }
 
        CloseData = Serial1.read();
        CloseData1 = Serial.read();
        if (CloseData == 'I'||CloseData1 == 'I')  //Exit
        {
          Close_Flag = 0;
          tft.fillScreen(TFT_BLACK);
          Serial1.println("Exit IIC");
          Serial.println("Exit IIC");
          break;
        }
      }
      break;
    case 'j':
        Close_Flag = 1;

        Serial.println("IO Port Test");
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(50, 100, 4);
        tft.printf("IO port output testing");
        pinMode(2,OUTPUT);
        pinMode(3,OUTPUT);
        while (Close_Flag == 1)
        {
          digitalWrite(2, HIGH);//Pull high level to turn on the light
          digitalWrite(3, HIGH);//Pull high level to turn on the light
          CloseData1 = Serial.read();
          if (CloseData1 == 'I')  //Exit
          {
            Close_Flag = 0;
            digitalWrite(2, LOW);
            digitalWrite(3, LOW);
            tft.fillScreen(TFT_BLACK);
            Serial.println("Exit IO port test");
            break;
          }
        }
        break;
//      Close_Flag = 1;
//      Wire1.setSDA(I2C1_0_SDA);
//      Wire1.setSCL(I2C1_0_SCL);
//      Wire1.begin();
//      dht.begin();    //  ESP32 default pins 21 22
// 
//      Serial1.println("IIC");
//      Serial.println("IIC");
//      tft.fillScreen(TFT_BLACK);
//      tft.setCursor(100, 100, 4);
//      tft.printf("IIC testing");
//      while (Close_Flag == 1)
//      {
//          if(millis() - dht.lastRead() >= 1000)
//          {
//            //  READ DATA
//            uint32_t start = micros();
//            int status = dht.read();
//            uint32_t stop = micros();
//        
//            if ((Count % 10) == 0)
//            {
//              Count = 0;
//              Serial1.println();
//              Serial1.println("Type\tHumidity (%)\tTemp (°C)\tTime (µs)\tStatus");
//              Serial.println();
//              Serial.println("Type\tHumidity (%)\tTemp (°C)\tTime (µs)\tStatus");
//            }
//            Count++;
//        
//            Serial1.print("DHT20 \t");
//            //  DISPLAY DATA, sensor has only one decimal.
//            Serial1.print(dht.getHumidity(), 1);
//            Serial1.print("\t\t");
//            Serial1.print(dht.getTemperature(), 1);
//            Serial1.print("\t\t");
//            Serial1.print(stop - start);
//            Serial1.print("\t\t");
//
//            Serial.print("DHT20 \t");
//            //  DISPLAY DATA, sensor has only one decimal.
//            Serial.print(DHT.getHumidity(), 1);
//            Serial.print("\t\t");
//            Serial.print(DHT.getTemperature(), 1);
//            Serial.print("\t\t");
//            Serial.print(stop - start);
//            Serial.print("\t\t");
//            switch (status)
//            {
//              case DHT20_OK:
//                Serial1.print("OK");
//                Serial.print("OK");
//                break;
//              case DHT20_ERROR_CHECKSUM:
//                Serial1.print("Checksum error");
//                Serial.print("Checksum error");
//                break;
//              case DHT20_ERROR_CONNECT:
//                Serial1.print("Connect error");
//                Serial.print("Connect error");
//                break;
//              case DHT20_MISSING_BYTES:
//                Serial1.print("Missing bytes");
//                Serial.print("Missing bytes");
//                break;
//              case DHT20_ERROR_BYTES_ALL_ZERO:
//                Serial1.print("All bytes read zero");
//                Serial.print("All bytes read zero");
//                break;
//              case DHT20_ERROR_READ_TIMEOUT:
//                Serial1.print("Read time out");
//                Serial.print("Read time out");
//                break;
//              case DHT20_ERROR_LASTREAD:
//                Serial1.print("Error read too fast");
//                Serial.print("Error read too fast");
//                break;
//              default:
//                Serial1.print("Unknown error");
//                Serial.print("Unknown error");
//                break;
//            }
//            Serial1.print("\n");
//            Serial.print("\n");
//          }
// 
//        CloseData = Serial1.read();
//        CloseData1 = Serial.read();
//        if (CloseData == 'I'||CloseData1 == 'I')  //Exit
//        {
//          Close_Flag = 0;
//          tft.fillScreen(TFT_BLACK);
//          Wire1.end();
//          Serial1.println("Exit IIC");
//          Serial.println("Exit IIC");
//          break;
//        }
//      }
//      break;
    case 'k':
      Close_Flag = 1;
      Wire1.setSDA(I2C1_1_SDA);
      Wire1.setSCL(I2C1_1_SCL);
      Wire1.begin();
      dht.begin();    //  ESP32 default pins 21 22
 
      Serial1.println("IIC");
      Serial.println("IIC");
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(100, 100, 4);
      tft.printf("IIC testing");
      while (Close_Flag == 1)
      {
          if(millis() - dht.lastRead() >= 1000)
          {
            //  READ DATA
            uint32_t start = micros();
            int status = dht.read();
            uint32_t stop = micros();
        
            if ((Count % 10) == 0)
            {
              Count = 0;
              Serial1.println();
              Serial1.println("Type\tHumidity (%)\tTemp (°C)\tTime (µs)\tStatus");
              Serial.println();
              Serial.println("Type\tHumidity (%)\tTemp (°C)\tTime (µs)\tStatus");
            }
            Count++;
        
            Serial1.print("DHT20 \t");
            //  DISPLAY DATA, sensor has only one decimal.
            Serial1.print(dht.getHumidity(), 1);
            Serial1.print("\t\t");
            Serial1.print(dht.getTemperature(), 1);
            Serial1.print("\t\t");
            Serial1.print(stop - start);
            Serial1.print("\t\t");

            Serial.print("DHT20 \t");
            //  DISPLAY DATA, sensor has only one decimal.
            Serial.print(dht.getHumidity(), 1);
            Serial.print("\t\t");
            Serial.print(dht.getTemperature(), 1);
            Serial.print("\t\t");
            Serial.print(stop - start);
            Serial.print("\t\t");
            switch (status)
            {
              case DHT20_OK:
                Serial1.print("OK");
                Serial.print("OK");
                break;
              case DHT20_ERROR_CHECKSUM:
                Serial1.print("Checksum error");
                Serial.print("Checksum error");
                break;
              case DHT20_ERROR_CONNECT:
                Serial1.print("Connect error");
                Serial.print("Connect error");
                break;
              case DHT20_MISSING_BYTES:
                Serial1.print("Missing bytes");
                Serial.print("Missing bytes");
                break;
              case DHT20_ERROR_BYTES_ALL_ZERO:
                Serial1.print("All bytes read zero");
                Serial.print("All bytes read zero");
                break;
              case DHT20_ERROR_READ_TIMEOUT:
                Serial1.print("Read time out");
                Serial.print("Read time out");
                break;
              case DHT20_ERROR_LASTREAD:
                Serial1.print("Error read too fast");
                Serial.print("Error read too fast");
                break;
              default:
                Serial1.print("Unknown error");
                Serial.print("Unknown error");
                break;
            }
            Serial1.print("\n");
            Serial.print("\n");
          }
 
        CloseData = Serial1.read();
        CloseData1 = Serial.read();
        if (CloseData == 'I'||CloseData1 == 'I')  //Exit
        {
          Close_Flag = 0;
          tft.fillScreen(TFT_BLACK);
          Serial1.println("Exit IIC");
          Serial.println("Exit IIC");
          break;
        }
      }
      break;      
    case 'U':
      Close_Flag = 1;
      Serial.println("Open UART1");
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(100, 100, 4);
      CloseData1 ='0';
      tft.printf("UART1 testing");
      while (Close_Flag == 1)
      {
        digitalWrite(Uart_LED, HIGH);
        if (Serial2.available())              // if date is comming from softwareserial port ==> data is comming from SoftSerial shield
        {
          while (Serial2.available())         // reading data into char array
          {
            digitalWrite(Uart_LED, LOW);
            
            buffer[count++] = Serial2.read();   // writing data into array
            if (count == 256)break;
            delay(30);
            digitalWrite(Uart_LED, HIGH);
            delay(30);
//            CloseData1 = Serial.read();
//            if (CloseData == 'I'||CloseData1 == 'I')  //Exit
//            {
//              break;
//            }
          }
         // if no data transmission ends, write buffer to hardware serial port
          Serial.write(buffer, count); 
          clearBufferArray();              // call clearBufferArray function to clear the storaged data from the array
          count = 0;                       // set counter of while loop to zero
          delay(100);
        }
      // write it to the SoftSerial shield
        if (Serial.available())            // if data is available on hardwareserial port ==> data is comming from PC or notebook
         {
          while(Serial.available())
          {
            CloseData1 = Serial.read();
            Serial2.write(CloseData1);       // write it to the SoftSerial shield
          }
          
         }

        if (CloseData1 == 'I')  //Exit
        {
          Close_Flag = 0;
          CloseData1 ='0';
          digitalWrite(Uart_LED, HIGH);
          tft.fillScreen(TFT_BLACK);
          Serial.println("Close UART1");
          break;
        }
      }
      break;
   case 'X':
      Close_Flag = 1;
      Serial.println("Open UART0");
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(100, 100, 4);
      CloseData1 ='0';
      tft.printf("UART0 testing");
      while (Close_Flag == 1)
      {
        digitalWrite(Uart_LED, HIGH);
        if (Serial1.available())              // if date is comming from softwareserial port ==> data is comming from SoftSerial shield
        {
          while (Serial1.available())         // reading data into char array
          {
            digitalWrite(Uart_LED, LOW);
            
            buffer[count++] = Serial1.read();   // writing data into array
            if (count == 256)break;
            delay(30);
            digitalWrite(Uart_LED, HIGH);
            delay(30);
//            CloseData1 = Serial.read();
//            if (CloseData == 'I'||CloseData1 == 'I')  //Exit
//            {
//              break;
//            }
          }
         // if no data transmission ends, write buffer to hardware serial port
          Serial.write(buffer, count); 
          clearBufferArray();              // call clearBufferArray function to clear the storaged data from the array
          count = 0;                       // set counter of while loop to zero
          delay(100);
        }
      // write it to the SoftSerial shield
        if (Serial.available())            // if data is available on hardwareserial port ==> data is comming from PC or notebook
         {
          while(Serial.available())
          {
            CloseData1 = Serial.read();
            Serial1.write(CloseData1);   // write it to the SoftSerial shield
          }
          
         }
         
        if (CloseData1 == 'I')  //Exit
        {
          Close_Flag = 0;
          CloseData1 ='0';
          digitalWrite(Uart_LED, HIGH);
          tft.fillScreen(TFT_BLACK);
          Serial.println("Close UART0");
          break;
        }
      }
      break;
    case 'F':
      Close_Flag = 1;
      Serial.println("Open Buzzer");
      Serial1.println("Open Buzzer");
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(100, 100, 4);
      tft.printf("Buzzer testing");
      tone(Buzzwr_Pin, frequency);    
      while (Close_Flag == 1)
      {
        
        CloseData = Serial1.read();
        CloseData1 = Serial.read();
        if (CloseData == 'I'||CloseData1 == 'I')  //Exit
        {
          tone(Buzzwr_Pin, 0); 
          delay(50);
          Close_Flag = 0;
          tft.fillScreen(TFT_BLACK);
          Serial1.println("Close Buzzer");
          Serial.println("Close Buzzer");
          break;
        }
      }
      break;

    case 'V':
      Close_Flag = 1;
      Serial1.println("ADC TEST");
      Serial.println("ADC TEST");
      tft.fillScreen(TFT_BLACK);
      while (Close_Flag == 1)
      {
        int v = analogRead(ADC0); //从A0口读取电压，模拟端口电压测量范围为0-5V，返回的值为0-1024
        float volt = v * (5.0 / 1024.0); //将返回值换算成电压
        delay(500); //输出后等待1s，降低刷新速度
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(20, 80, 4);
        tft.printf("%.2f V",volt);

        v = analogRead(ADC1);
        volt = v * (5.0 / 1024.0); 
        tft.setCursor(20, 100, 4);
        tft.printf("%.2f V",volt);

        v = analogRead(ADC2);
        volt = v * (5.0 / 1024.0); 
        tft.setCursor(20, 120, 4);
        tft.printf("%.2f V",volt);

        float Temp_degC = analogReadTemp();
        tft.setCursor(20, 0, 4);
        tft.printf(" CPU:");
        tft.setCursor(85, 0, 4);
        tft.printf(" %lf °C",Temp_degC);
        
        CloseData = Serial1.read();
        CloseData1 = Serial.read();
        if (CloseData == 'I'||CloseData1 == 'I')  //Exit
        {
          Close_Flag = 0;
          delay(50);
          tft.fillScreen(TFT_BLACK);
          Serial1.println("EXIT ADC");
          Serial.println("EXIT ADC");
          break;
        }
      }
      break;

    //Exit Software Testing Program
    case 'C':
      NO_Test_Flag = 0;
      Test_Flag = 0;
      tft.fillScreen(TFT_BLACK);

      Serial.println("Exit the software test program");
      delay(500);
      break;
  }
}

void SD_test()
{
  Serial1.println("\nInitializing SD card...");
  Serial.println("\nInitializing SD card...");
  bool sdInitialized = false;
  // Ensure the SPI pinout the SD card is connected to is configured properly
  // Select the correct SPI based on _MISO pin for the RP2040
  if (_MISO == 0 || _MISO == 4 || _MISO == 16) {
    SPI.setRX(_MISO);
    SPI.setTX(_MOSI);
    SPI.setSCK(_SCK);
    sdInitialized = SD.begin(_CS);
  } else if (_MISO == 8 || _MISO == 12) {
    SPI1.setRX(_MISO);
    SPI1.setTX(_MOSI);
    SPI1.setSCK(_SCK);
    sdInitialized = SD.begin(_CS, SPI1);
  } else {
    Serial.println(F("ERROR: Unknown SPI Configuration"));
    Serial1.println(F("ERROR: Unknown SPI Configuration"));
    return;
  }

  if (!sdInitialized) {
    Serial1.println("initialization failed. Things to check:");
    Serial1.println("* is a card inserted?");
    Serial1.println("* is your wiring correct?");
    Serial1.println("* did you change the chipSelect pin to match your shield or module?");
    
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return;
  } else {
    Serial1.println("Wiring is correct and a card is present.");
    Serial.println("Wiring is correct and a card is present.");
  }
  // 0 - SD V1, 1 - SD V2, or 3 - SDHC/SDXC
  // print the type of card
  Serial1.println();
  Serial1.print("Card type:         ");
  Serial.println();
  Serial.print("Card type:         ");
  switch (SD.type()) {
    case 0:
      Serial1.println("SD1");
      Serial.println("SD1");
      break;
    case 1:
      Serial1.println("SD2");
      Serial.println("SD2");
      break;
    case 3:
      Serial1.println("SDHC/SDXC");
      Serial.println("SDHC/SDXC");
      break;
    default:
      Serial1.println("Unknown");
      Serial.println("Unknown");
  }

  Serial1.print("Cluster size:          ");
  Serial1.println(SD.clusterSize());
  Serial1.print("Blocks x Cluster:  ");
  Serial1.println(SD.blocksPerCluster());
  Serial1.print("Blocks size:  ");
  Serial1.println(SD.blockSize());

  Serial.print("Cluster size:          ");
  Serial.println(SD.clusterSize());
  Serial.print("Blocks x Cluster:  ");
  Serial.println(SD.blocksPerCluster());
  Serial.print("Blocks size:  ");
  Serial.println(SD.blockSize());
  
  Serial1.print("Total Blocks:      ");
  Serial1.println(SD.totalBlocks());
  Serial1.println();
    
  Serial.print("Total Blocks:      ");
  Serial.println(SD.totalBlocks());
  Serial.println();

  Serial1.print("Total Cluster:      ");
  Serial1.println(SD.totalClusters());
  Serial1.println();
  
  Serial.print("Total Cluster:      ");
  Serial.println(SD.totalClusters());
  Serial.println();

  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial1.print("Volume type is:    FAT");
  Serial1.println(SD.fatType(), DEC);

  Serial.print("Volume type is:    FAT");
  Serial.println(SD.fatType(), DEC);
  
  volumesize = SD.totalClusters();
  volumesize *= SD.clusterSize();
  volumesize /= 1000;
  Serial1.print("Volume size (Kb):  ");
  Serial1.println(volumesize);
  Serial1.print("Volume size (Mb):  ");

  Serial.print("Volume size (Kb):  ");
  Serial.println(volumesize);
  Serial.print("Volume size (Mb):  ");
  volumesize /= 1024;
  Serial1.println(volumesize);
  Serial1.print("Volume size (Gb):  ");
  Serial1.println((float)volumesize / 1024.0);

  Serial.println(volumesize);
  Serial.print("Volume size (Gb):  ");
  Serial.println((float)volumesize / 1024.0);
  
  Serial1.print("Card size:  ");
  Serial1.println((float)SD.size() / 1000);

  Serial.print("Card size:  ");
  Serial.println((float)SD.size() / 1000);
  FSInfo fs_info;
  SDFS.info(fs_info);

  Serial1.print("Total bytes: ");
  Serial1.println(fs_info.totalBytes);

  Serial1.print("Used bytes: ");
  Serial1.println(fs_info.usedBytes);

  Serial.print("Total bytes: ");
  Serial.println(fs_info.totalBytes);

  Serial.print("Used bytes: ");
  Serial.println(fs_info.usedBytes);
  root = SD.open("/");
  printDirectory(root, 0);
  
}
void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry = dir.openNextFile();
    if (!entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial1.print('\t');
      Serial.print('\t');
    }
    Serial1.print(entry.name());
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial1.println("/");
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial1.print("\t\t");
      Serial1.print(entry.size(), DEC);
      Serial.print("\t\t");
      Serial.print(entry.size(), DEC);
      time_t cr = entry.getCreationTime();
      time_t lw = entry.getLastWrite();
      struct tm* tmstruct = localtime(&cr);
      Serial1.printf("\tCREATION: %d-%02d-%02d %02d:%02d:%02d", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
      Serial.printf("\tCREATION: %d-%02d-%02d %02d:%02d:%02d", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
      tmstruct = localtime(&lw);
      Serial.printf("\tLAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
      Serial1.printf("\tLAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    }
    entry.close();
  }
}
//触摸Label控件
void label_xy()
{
  ui_Label = lv_label_create(ui_TOUCH);
  lv_obj_enable_style_refresh(true);
  lv_obj_set_width(ui_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Label, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_Label, -35);
  lv_obj_set_y(ui_Label, -60);
  lv_obj_set_align(ui_Label, LV_ALIGN_CENTER);
  lv_obj_set_style_text_color(ui_Label, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_Label, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Label3 = lv_label_create(ui_TOUCH);
  lv_obj_enable_style_refresh(true);
  lv_obj_set_width(ui_Label3, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Label3, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_Label3, 65);
  lv_obj_set_y(ui_Label3, -60);
  lv_obj_set_align(ui_Label3, LV_ALIGN_CENTER);
  lv_obj_set_style_text_color(ui_Label3, lv_color_hex(0x00FF00), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_Label3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_Label3, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
}

//进度条控件
void lv_example_bar(void)
{
  //////////////////////////////
  bar = lv_bar_create(ui_MENU);
  lv_bar_set_value(bar, 0, LV_ANIM_OFF);
  lv_obj_set_width(bar, 300);
  lv_obj_set_height(bar, 20);
  lv_obj_set_x(bar, 0);
  lv_obj_set_y(bar, 120);
  lv_obj_set_align(bar, LV_ALIGN_CENTER);
  lv_obj_set_style_bg_img_src(bar, &ui_img_bar_480_01_png, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_set_style_bg_img_src(bar, &ui_img_bar_480_02_png, LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(bar, lv_color_hex(0x2D8812), LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(bar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
  //////////////////////
  ui_Labe2 = lv_label_create(bar);//创建标签
  lv_obj_set_style_text_color(ui_Labe2, lv_color_hex(0x09BEFB), LV_STATE_DEFAULT);
  lv_label_set_text(ui_Labe2, "0%");
  lv_obj_center(ui_Labe2);
}


//设置黑色背景
void Home_Page_Create(void)
{
  static lv_style_t style;                  //创建样式
  lv_style_init(&style);                    //初始化样式

  lv_obj_t* bgk;
  bgk = lv_obj_create(lv_scr_act());//创建对象
  lv_obj_set_style_bg_color(bgk, lv_color_hex(0x000000 ), LV_STATE_DEFAULT); // obj背景色设成黄色

  lv_style_set_border_color(&style, lv_palette_main(LV_PALETTE_NONE)); //设置外框颜色
  lv_obj_add_style(bgk, &style, 0);         //将样式添加到文字对象中
  lv_obj_set_size(bgk, 480, 320);             // 设置到屏幕大小

}
void touch_calibrate()//屏幕校准
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;
  Serial.println("屏幕校准");

  //校准
  //  lcd.fillScreen(TFT_BLACK);
  //  lcd.setCursor(20, 0);
  //  Serial.println("setCursor");
  //  lcd.setTextFont(2);
  //  Serial.println("setTextFont");
  //  lcd.setTextSize(1);
  //  Serial.println("setTextSize");
  //  lcd.setTextColor(TFT_WHITE, TFT_BLACK);

  //  lcd.println("按指示触摸角落");
  Serial.println("按指示触摸角落");
  lv_timer_handler();

  Serial.println("setTextFont(1)");
  tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
  Serial.println("calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15)");
  Serial.println(); Serial.println();
  Serial.println("//在setup()中使用此校准代码:");
  Serial.print("uint16_t calData[5] = ");
  Serial.print("{ ");



  for (uint8_t i = 0; i < 5; i++)
  {
    Serial.print(calData[i]);
    if (i < 4) Serial.print(", ");
  }

  Serial.println(" };");
  Serial.print("  tft.setTouch(calData);");
  Serial.println(); Serial.println();
  tft.fillScreen(TFT_BLACK);
  //
  //  lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  //  lcd.println("XZ OK!");
  //  lcd.println("Calibration code sent to Serial port.");

}
