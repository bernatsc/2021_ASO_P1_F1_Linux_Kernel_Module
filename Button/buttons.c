//Libraries
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kmod.h>

//Module description
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bernat Segura");
MODULE_DESCRIPTION("KLM to manage 4 buttons to open and close 2 leds and execute a script for each button");
MODULE_VERSION("1.0");

//Defines
#define DEBOUNCE_TIME 200 

//Leds
static unsigned int gpioLED1 = 20;
static unsigned int gpioLED2 = 16;

static bool led1On = false;
static bool led2On = false;

//Buttons
static unsigned int gpioButton1 = 21;
static unsigned int gpioButton2 = 13;
static unsigned int gpioButton3 = 19;
static unsigned int gpioButton4 = 26;

//Counter pressed Buttons
static unsigned int cntButton1 = 0;
static unsigned int cntButton2 = 0;
static unsigned int cntButton3 = 0;
static unsigned int cntButton4 = 0;

//Interrupts
static unsigned int irqNumber1;
static unsigned int irqNumber2;
static unsigned int irqNumber3;
static unsigned int irqNumber4;

//Interruption handler
static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

//LKM init function
static int __init gpio_init(void) {

   int result1;
   int result2;
   int result3;
   int result4;

   //Initial message
   printk(KERN_INFO "ASO: __init: Initializing the ASO P1F1 LKM\n");

   //Check if LED1 and LED2 pins are unsued
   if (!gpio_is_valid(gpioLED1)) {
      printk(KERN_INFO "ASO: __init: leds: invalid LED 1 GPIO: %d\n", gpioLED1);
      return -ENODEV;
   }
   if (!gpio_is_valid(gpioLED2)) {
      printk(KERN_INFO "ASO: __init: invalid LED 2 GPIO: %d\n", gpioLED1);
      return -ENODEV;
   }

   //Set leds inicial state
   led1On = true;
   led2On = true;

   //Request the already checked pins for leds
   gpio_request(gpioLED1, "sysfs");
   gpio_request(gpioLED2, "sysfs");
   printk(KERN_INFO "ASO: __init: leds: gpio request: Done");

   //Set pin direction (output) with initial value
   gpio_direction_output(gpioLED1, led1On);
   gpio_direction_output(gpioLED2, led2On);
   printk(KERN_INFO "ASO: __init: leds: gpio direction output: Done");

   //Make led pins available via sysfs
   gpio_export(gpioLED1, false);
   gpio_export(gpioLED2, false);
   printk(KERN_INFO "ASO: __init: leds: gpio export: Done");

   //Request pins for buttons
   gpio_request(gpioButton1, "sysfs");
   gpio_request(gpioButton2, "sysfs");
   gpio_request(gpioButton3, "sysfs");
   gpio_request(gpioButton4, "sysfs");
   printk(KERN_INFO "ASO: __init: buttons: gpio request: Done");

   //Set buttons direction (input)
   gpio_direction_input(gpioButton1);
   gpio_direction_input(gpioButton2);
   gpio_direction_input(gpioButton3);
   gpio_direction_input(gpioButton4);
   printk(KERN_INFO "ASO: __init: buttons: gpio direction input: Done");

   //Set bounce time for buttons
   gpio_set_debounce(gpioButton1, DEBOUNCE_TIME);
   gpio_set_debounce(gpioButton2, DEBOUNCE_TIME);
   gpio_set_debounce(gpioButton3, DEBOUNCE_TIME);
   gpio_set_debounce(gpioButton4, DEBOUNCE_TIME);
   printk(KERN_INFO "ASO: __init: buttons: gpio set debounce [%d]: Done", DEBOUNCE_TIME);

   //Make button pins available via sysfs
   gpio_export(gpioButton1, false);
   gpio_export(gpioButton2, false);
   gpio_export(gpioButton3, false);
   gpio_export(gpioButton4, false);
   printk(KERN_INFO "ASO: __init: buttons: gpio export: Done");

   //Associate button with an irq
   irqNumber1 = gpio_to_irq(gpioButton1);
   irqNumber2 = gpio_to_irq(gpioButton2);
   irqNumber3 = gpio_to_irq(gpioButton3);
   irqNumber4 = gpio_to_irq(gpioButton4);

   //Logs gpio to irq
   printk(KERN_INFO "ASO: __init: GPIO_BUTTON_1: The button is mapped to IRQ: %d\n", irqNumber1);
   printk(KERN_INFO "ASO: __init: GPIO_BUTTON_2: The button is mapped to IRQ: %d\n", irqNumber2);
   printk(KERN_INFO "ASO: __init: GPIO_BUTTON_3: The button is mapped to IRQ: %d\n", irqNumber3);
   printk(KERN_INFO "ASO: __init: GPIO_BUTTON_4: The button is mapped to IRQ: %d\n", irqNumber4);
   
   //Register an interrupt request
   //OPT: IRQF_TRIGGER_FALLING or IRQF_TRIGGER_RISING
   result1 = request_irq(irqNumber1, (irq_handler_t) gpio_irq_handler, IRQF_TRIGGER_FALLING, "gpio_handler_button_1", NULL);
   result2 = request_irq(irqNumber2, (irq_handler_t) gpio_irq_handler, IRQF_TRIGGER_FALLING, "gpio_handler_button_2", NULL);
   result3 = request_irq(irqNumber3, (irq_handler_t) gpio_irq_handler, IRQF_TRIGGER_FALLING, "gpio_handler_button_3", NULL);
   result4 = request_irq(irqNumber4, (irq_handler_t) gpio_irq_handler, IRQF_TRIGGER_FALLING, "gpio_handler_button_4", NULL);

   //Logs request irq
   printk(KERN_INFO "ASO: __init: GPIO_BUTTON_1: The interrupt request result is: %d\n", result1);
   printk(KERN_INFO "ASO: __init: GPIO_BUTTON_2: The interrupt request result is: %d\n", result2);
   printk(KERN_INFO "ASO: __init: GPIO_BUTTON_3: The interrupt request result is: %d\n", result3);
   printk(KERN_INFO "ASO: __init: GPIO_BUTTON_4: The interrupt request result is: %d\n", result4);

   //Open message
   printk(KERN_INFO "ASO: __init: LKM ready\n");

   return (result1 + result2 + result3 + result4) ? 1 : 0;
}

static void __exit gpio_exit(void) {
   //Exit message 
   printk(KERN_INFO "ASO: __exit: Exiting the ASO P1F1 LKM\n");
   
   //Turn of the leds
   led1On = false;
   led2On = false;
   gpio_set_value(gpioLED1, led1On);
   gpio_set_value(gpioLED2, led2On);
   printk(KERN_INFO "ASO: __exit: leds: gpio set value: turn off leds: Done");

   //Remove leds from sysfs
   gpio_unexport(gpioLED1);
   gpio_unexport(gpioLED2);
   printk(KERN_INFO "ASO: __exit: leds: gpio unexport: Done");

   //Free the interruption 
   free_irq(irqNumber1, NULL);
   free_irq(irqNumber2, NULL);
   free_irq(irqNumber3, NULL);
   free_irq(irqNumber4, NULL);
   printk(KERN_INFO "ASO: __exit: buttons: free irq: Done");

   //Remove buttons from sysfs
   gpio_unexport(gpioButton1);
   gpio_unexport(gpioButton2);
   gpio_unexport(gpioButton3);
   gpio_unexport(gpioButton4);
   printk(KERN_INFO "ASO: __exit: buttons: gpio unexport: Done");

   //Deallocate the GPIO for leds
   gpio_free(gpioLED1);
   gpio_free(gpioLED2);
   printk(KERN_INFO "ASO: __exit: leds: gpio free: Done");

   //Deallocate the GPIO for buttons
   gpio_free(gpioButton1);
   gpio_free(gpioButton2);
   gpio_free(gpioButton3);
   gpio_free(gpioButton4);
   printk(KERN_INFO "ASO: __exit: buttons: gpio free: Done");

   //Closing message
   printk(KERN_INFO "ASO: __exit: LKM closed\n");
}

static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs) {
   //Init usermodehelper vars
   static char *envp[] = {"HOME=/", "TERM=linux", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };
   char *argv1[] = {"/home/pi/F1/Scripts/button1.sh", NULL};
   char *argv2[] = {"/home/pi/F1/Scripts/button2.sh", NULL};
   char *argv3[] = {"/home/pi/F1/Scripts/button3.sh", NULL};
   char *argv4[] = {"/home/pi/F1/Scripts/button4.sh", NULL};
   int err = 0;

   printk(KERN_INFO "ASO: interrupt handler\n");

   if (irq == irqNumber1) {
      //Set state variable 
      led1On = true;
      //Set state to led
      gpio_set_value(gpioLED1, led1On);
      //Increment button counter
      cntButton1++;
      //Execute script
      err = call_usermodehelper(argv1[0], argv1, envp, UMH_NO_WAIT);
      
      //Print logs
      printk(KERN_INFO "ASO: Button_1 pressed: led 1 - [%d]", led1On);   
      printk(KERN_INFO "ASO: Button_1 has been pressed [%d] times", cntButton1);   
      printk(KERN_INFO "ASO: Button_1 script has been executed");   
  
   }else if (irq == irqNumber2) {
      //Set state variable 
      led1On = false;
      //Set state to led
      gpio_set_value(gpioLED1, led1On);
      //Increment button counter
      cntButton2++;
      //Execute script
      err = call_usermodehelper(argv2[0], argv2, envp, UMH_NO_WAIT);
      
      //Print logs
      printk(KERN_INFO "ASO: Button_2 pressed: led 1 - [%d]", led1On);   
      printk(KERN_INFO "ASO: Button_2 has been pressed [%d] times", cntButton2);   
      printk(KERN_INFO "ASO: Button_2 script has been executed");  

   }else if (irq == irqNumber3) {
      //Set state variable 
      led2On = true;
      //Set state to led
      gpio_set_value(gpioLED2, led2On);
      //Increment button counter
      cntButton3++;
      //Execute script
      err = call_usermodehelper(argv3[0], argv3, envp, UMH_NO_WAIT);
      
      //Print logs
      printk(KERN_INFO "ASO: Button_3 pressed: led 2 - [%d]", led2On);   
      printk(KERN_INFO "ASO: Button_3 has been pressed [%d] times", cntButton3);   
      printk(KERN_INFO "ASO: Button_3 script has been executed"); 

   }else if (irq == irqNumber4) {

      //Set state variable 
      led2On = false;
      //Set state to led
      gpio_set_value(gpioLED2, led2On);
      //Increment button counter
      cntButton4++;
      //Execute script
      err = call_usermodehelper(argv4[0], argv4, envp, UMH_NO_WAIT);
      
      //Print logs
      printk(KERN_INFO "ASO: Button_3 pressed: led 2 - [%d]", led2On);   
      printk(KERN_INFO "ASO: Button_3 has been pressed [%d] times", cntButton4);   
      printk(KERN_INFO "ASO: Button_3 script has been executed"); 

   }else {
      printk(KERN_INFO "ASO: ERROR: found unespecified irq number.");
   }

   //Print log if usermodehelper fails
   if (err != 0) {
         printk(KERN_INFO "ASO: ERROR on exec: %d", err);
   }
   
   return (irq_handler_t)IRQ_HANDLED;
}

module_init(gpio_init);
module_exit(gpio_exit);
