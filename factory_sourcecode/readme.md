- ### Functions Overview

  ### `clearBufferArray()`
  - **Description**: Clears the buffer array by setting all indices to `NULL`.
  - **Parameters**: None
  - **Returns**: None

  ### `my_disp_flush`
  - **Description**: Handles display flushing for LVGL (LittlevGL) graphics library.
  - **Parameters**: 
    - `lv_disp_drv_t *disp_drv`: Pointer to the display driver.
    - `const lv_area_t *area`: Pointer to the area to flush.
    - `lv_color_t *color_p`: Pointer to the color data.
  - **Returns**: None

  ### `my_touchpad_read`
  - **Description**: Reads touchpad input and updates LVGL input device data.
  - **Parameters**: 
    - `lv_indev_drv_t * indev_driver`: Pointer to the input device driver.
    - `lv_indev_data_t * data`: Pointer to the input device data.
  - **Returns**: None

  #### `callback1`
  - **Description**: Callback function for updating UI elements based on certain conditions.
  - **Parameters**: None
  - **Returns**: None

  #### `setup`
  - **Description**: Initializes the system, including serial communication, display, and LVGL.
  - **Parameters**: None
  - **Returns**: None

  #### `loop`
  - **Description**: Main loop of the program handling various tasks and user inputs.
  - **Parameters**: None
  - **Returns**: None

  #### `Ce_shi`
  - **Description**: Clears the screen and handles different test cases based on serial input.
  - **Parameters**: None
  - **Returns**: None

  ### `SD_test`
  - **Description**: Tests and initializes the SD card.
  - **Parameters**: None
  - **Returns**: None

  ### `printDirectory`
  - **Description**: Prints the contents of a directory to the serial output.
  - **Parameters**: 
    - `File dir`: Directory to print.
    - `int numTabs`: Number of tabs for formatting.
  - **Returns**: None

  #### `label_xy`
  - **Description**: Creates and configures touch labels on the screen.
  - **Parameters**: None
  - **Returns**: None

  #### `lv_example_bar`
  - **Description**: Creates and configures a bar widget for the UI.
  - **Parameters**: None
  - **Returns**: None

  #### `Home_Page_Create`
  - **Description**: Sets up the home page with a black background.
  - **Parameters**: None
  - **Returns**: None

  #### `touch_calibrate`
  - **Description**: Calibrates the touch screen and sends calibration data to the serial port.
  - **Parameters**: None
  - **Returns**: None