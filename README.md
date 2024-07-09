This readme provides an overview of the functionalities and requirements for controlling a helicopter using the TIVA board and
Orbit OLED display.

1. Quadrature Signal Processing
      * Helicopter yaw information is obtained from a 2-channel quadrature signal using interrupt-driven decoding.
      * Interrupt-driven decoding implementation.
      * The signed degrees of helicopter yaw is displayed on the Orbit OLED display.
        
2. Altitude Sensor and Display
      * The analogue signal is sampled and converted from the altitude sensor using TIVA ADC (module 0, channel 9).
      * Helicopter altitude is displayed as a percentage on the Orbit OLED board.
      * Maximum altitude is set as 100% and minimum altitude is set as 0% (when landed).
      * Filter implemented for reliable altitude estimation.

4. PWM Signal Generation
      * Two PWM signals generated for main rotor motor (module 0, PWM 7) and tail rotor motor (module 1, PWM 5).
      * PWM signal has a set frequency between 150 Hz to 300 Hz.
      * PWM duty cycles for each motor are displayed as percentages on the Orbit OLED display.

5. Helicopter Mode Control
      * Use SW1 slider switch on the Orbit board to control helicopter mode.
      * Switch down position indicates landed or landing state; switching from down to up initiates take-off.

7. User-Interactive Controls
      * TIVA-Orbit board buttons are used to control altitude and yaw of the helicopter interactively.
      * Each operation of the UP button on the Orbit board should cause the helicopter to increase its altitude by 10% of its
        total vertical range. If the helicopter is already within 10% of its maximum altitude, pushing the UP button should
        take the helicopter to its maximum altitude. When the helicopter is at its maximum altitude any operation of the UP
        button should be ignored.
      * Each operation of the DOWN button on the Orbit board should cause the helicopter to decrease its altitude by 10% of
        its total vertical range. If the helicopter is already within 10% of its minimum altitude, pushing the DOWN button
        should take the helicopter to its minimum altitude. When the helicopter is at its minimum altitude any operation of
        the DOWN button should be ignored.
      * Neither the UP nor the DOWN button should affect the yaw of the helicopter.
      * Each operation of the CCW (left) button on the TIVA board should cause the helicopter to rotate 15◦ counterclockwise
        (viewed from above).
      * Each operation of the CW (right) button on the TIVA board should cause the helicopter to rotate 15◦ clockwise (viewed
        from above).
      * Neither the CCW nor the CW button should affect the height of the helicopter.

9. Emulation and Remote Control
      * Respond to pushbutton signals asserted for 200ms; one button operation at a time.
      * Utilize virtual signals for remote operation
      * Implement virtual RESET button functionality to call SysCtlReset() API function for reset.
        
11. Real-Time Kernel and Robust Behavior
      * Interrupt-based scheduler implemented for foreground/background tasks.
      * Tested for robust behavior and responsiveness at all times.
        
12. Controller Implementation
      * Implemented a PI controller with a constant offset for controlled flight.

14. Serial Communication
      * Helicopter status information is transmitted via UART0 at 9600 baud with specified settings.
      * Information includes desired and actual yaw, altitude, PWM duty cycles, and operating mode in a concise and readable
        format for remote monitoring.
