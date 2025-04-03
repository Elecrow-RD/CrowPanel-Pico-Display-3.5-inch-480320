#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include "ui.h"
#include <Wire.h>
#include "DHT20.h"

int led;
//tft背光引脚
#define  LCD_BL 18


static const uint16_t screenWidth  = 480;
static const uint16_t screenHeight = 320;
 
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * screenHeight / 10 ];

TFT_eSPI tft = TFT_eSPI(); /* TFT instance screenWidth, screenHeight */

DHT20 DHT(&Wire);

#define I2C0_SDA 20
#define I2C0_SCL 21


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
    
  }
}



uint16_t calData[5] = { 192, 3590, 335, 3362, 1  };


void setup()
{
 
  Serial.begin( 115200 );
  
  
  
  
  //IO Port Pins
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW);

  Wire.setSDA(I2C0_SDA);
  Wire.setSCL(I2C0_SCL);
  Wire.begin();
  DHT.begin(); 

  

  /*Set the touchscreen calibration data,
    the actual data for your display can be acquired using
    the Generic -> Touch_calibrate example from the TFT_eSPI library*/
  tft.setTouch(calData);

  lv_init();

  tft.begin();          /* TFT init */
  tft.setRotation( 1); /* Landscape orientation, flipped */
  tft.invertDisplay(false);/* This line of code turns off the invert function of the display. Normally, the invert function inverts the colors on the screen, i.e. black to white and white to black. In this case, it is set to false, which means that no inversion is performed. */
  tft.fillScreen(TFT_BLACK);
  
// #if LV_USE_LOG != 0
//   lv_log_register_print_cb( my_print ); /* register print function for debugging */
// #endif

  lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * screenHeight / 10 );

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
      
  tft.fillScreen(TFT_BLACK);
  delay(200);

  pinMode(LCD_BL,OUTPUT);
//  analogWrite(LCD_BL,220);
  digitalWrite(LCD_BL, HIGH);
  delay(100);
 
   ui_init();//开机UI界面
}

void loop()
{

  char DHT_buffer[6];

  int status = DHT.read();
  
  int a = (int)DHT.getTemperature();
  int b = (int)DHT.getHumidity();
  snprintf(DHT_buffer, sizeof(DHT_buffer), "%d", a);
  lv_label_set_text(ui_Label1, DHT_buffer);
  snprintf(DHT_buffer, sizeof(DHT_buffer), "%d", b);
  lv_label_set_text(ui_Label2, DHT_buffer);

  Serial.print(led);
  if(led == 1)
  
  digitalWrite(5, HIGH);
  if(led == 0)
  digitalWrite(5, LOW);
  lv_timer_handler(); /* let the GUI do its work */
  delay( 10 );

}

