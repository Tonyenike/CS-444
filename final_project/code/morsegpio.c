#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/gpio.h>
 
#include "morsegpio.h"
 
#define FIRST_MINOR 0
#define MINOR_CNT 1
 
static dev_t dev;
static struct cdev c_dev;
static struct class *cl;

unsigned long vin_speed = 100; //100 ms by default for both out and in speed.
unsigned long vout_speed = 100;
unsigned int vin_pin = 23; // These are the pin numbers by default.
unsigned int vout_pin = 24;

int vout_valid = 1;
int vin_valid = 1;

char word_read[64] = "";
char * word_write = NULL;

char ** word_w = &word_write;

char char_to_print;
char null_char = '\0';
int printable = 0;

int lplace = 0;
int mplace = 0;
size_t sssize;

int phase = 0;
int totals = 0;

int GETE = 1;

static struct timer_list my_timer;
static struct timer_list boy_timer;

int stop_talking_idiot = 0;

// This function was found at
// https://www.geeksforgeeks.org/morse-code-implementation/
char *  mEncode(char x) { 
  switch (x) { 
  case 'a': 
  case 'A': 
    return ".-"; 
  case 'b': 
  case 'B': 
    return "-..."; 
  case 'c': 
  case 'C': 
    return "-.-."; 
  case 'd': 
  case 'D': 
    return "-.."; 
  case 'e': 
  case 'E': 
    return "."; 
  case 'f': 
  case 'F': 
    return "..-."; 
  case 'g': 
  case 'G': 
    return "--."; 
  case 'h': 
  case 'H': 
    return "...."; 
  case 'i': 
  case 'I': 
    return ".."; 
  case 'j': 
  case 'J': 
    return ".---"; 
  case 'k': 
  case 'K': 
    return "-.-"; 
  case 'l': 
  case 'L': 
    return ".-.."; 
  case 'm': 
  case 'M': 
    return "--"; 
  case 'n': 
  case 'N': 
    return "-."; 
  case 'o': 
  case 'O': 
    return "---"; 
  case 'p': 
  case 'P': 
    return ".--."; 
  case 'q': 
  case 'Q': 
    return "--.-"; 
  case 'r': 
  case 'R': 
    return ".-."; 
  case 's': 
  case 'S': 
    return "..."; 
  case 't': 
  case 'T': 
    return "-"; 
  case 'u': 
  case 'U': 
    return "..-"; 
  case 'v': 
  case 'V': 
    return "...-"; 
  case 'w': 
  case 'W': 
    return ".--"; 
  case 'x': 
  case 'X': 
    return "-..-"; 
  case 'y': 
  case 'Y': 
    return "-.--"; 
  // for space 
  case 'Z': 
  case 'z': 
    return "--.."; 
  default:
    return "...---...";
  } 
}

char mDecode(char * input){

    if(!strcmp(input, ".-"))
        return 'a';
    else if(!strcmp(input, "-..."))
        return 'b';
    else if(!strcmp(input, "-.-."))
        return 'c';
    else if(!strcmp(input, "-.."))
        return 'd';
    else if(!strcmp(input, "."))
        return 'e';
    else if(!strcmp(input, "..-."))
        return 'f';
    else if(!strcmp(input, "--."))
        return 'g';
    else if(!strcmp(input, "...."))
        return 'h';
    else if(!strcmp(input, ".."))
        return 'i';
    else if(!strcmp(input, ".---"))
        return 'j';
    else if(!strcmp(input, "-.-"))
        return 'k';
    else if(!strcmp(input, ".-.."))
        return 'l';
    else if(!strcmp(input, "--"))
        return 'm';
    else if(!strcmp(input, "-."))
        return 'n';
    else if(!strcmp(input, "---"))
        return 'o';
    else if(!strcmp(input, ".--."))
        return 'p';
    else if(!strcmp(input, "--.-"))
        return 'q';
    else if(!strcmp(input, ".-."))
        return 'r';
    else if(!strcmp(input, "..."))
        return 's';
    else if(!strcmp(input, "-"))
        return 't';
    else if(!strcmp(input, "..-"))
        return 'u';
    else if(!strcmp(input, "...-"))
        return 'v';
    else if(!strcmp(input, ".--"))
        return 'w';
    else if(!strcmp(input, "-..-"))
        return 'x';
    else if(!strcmp(input, "-.--"))
        return 'y';
    else if(!strcmp(input, "--.."))
        return 'z';
    else
        return '?';
} 

static void morse_function_dos(unsigned long data){
    int yeet;

    //printk("I am reading and totals is %d\n", totals);

    if(vin_valid){
        yeet = gpio_get_value(vin_pin); 
        if(yeet){
            if(totals < 0)
                totals = 0;
            totals++;
        }
        else{
            if(totals >= 10){
                strcat(word_read, "-");
                totals = 0;
            }
            else if(totals >= 2){
                strcat(word_read, ".");
                totals = 0;
            }
            else if(totals == -6){
                char_to_print = mDecode(word_read);
                printk("Morse is: %s\n", word_read);
                strcpy(word_read, "");
                printable = 1;
            }
            else if(totals == -26){
                char_to_print = ' ';
                strcpy(word_read, "");
                printable = 1;
            }
            totals--;
       }
    }
    else{
    }

    mod_timer(&boy_timer, jiffies + msecs_to_jiffies(vin_speed / 4));
}

static ssize_t out_speed_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	//dev_get_drvdata(dev);

	return sprintf(buf, "%lu\n", vout_speed);
}


static ssize_t out_speed_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	//struct class *yeet = dev_get_drvdata(dev);
	unsigned long state;
	int ret;

	ret = kstrtoul(buf, 0, &state);
	if (ret)
		return ret;

	printk("Storing %lu\n",  state);

    vout_speed = state;

	return size;
}

static ssize_t out_pin_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	//dev_get_drvdata(dev);

	return sprintf(buf, "%u\n", vout_pin);
}


static ssize_t out_pin_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned state;
	int ret;

	ret = kstrtouint(buf, 0, &state);
	if (ret)
		return ret;

	printk("Storing %u\n",  state);

    gpio_free(vout_pin);

    vout_pin = state;
    
    vout_valid = 1;


    if(gpio_request(vout_pin, "worthless label here\0")){
        printk("Whoopsies! Could not request pin: %u\r\n", vout_pin);
        vout_valid = 0;
        return 1;
    }
    if(gpio_direction_output(vout_pin, 0)){ 
        printk("Whoopsies! Could not set pin: %u\r\n", vout_pin);
        vout_valid = 0;
        return 1;
    }

	return size;
}
static ssize_t in_speed_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	//dev_get_drvdata(dev);

	return sprintf(buf, "%lu\n", vin_speed);
}


static ssize_t in_speed_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long state;
	int ret;

	ret = kstrtoul(buf, 0, &state);
	if (ret)
		return ret;

	printk("Storing %lu\n",  state);

    vin_speed = state;

	return size;
}

static ssize_t in_pin_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	//dev_get_drvdata(dev);

	return sprintf(buf, "%u\n", vin_pin);
}


static ssize_t in_pin_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned state;
	int ret;

	ret = kstrtouint(buf, 0, &state);
	if (ret)
		return ret;

	printk("Storing %u\n",  state);

    gpio_free(vin_pin);
    vin_pin = state;
    
    vin_valid = 1;

    if(gpio_request(vin_pin, "worthless label 2 here\0")){
        printk("Whoopsies! Could not request pin: %u\r\n", vin_pin);
        vin_valid = 0;
        return 1;
    }   
 
    if(gpio_direction_input(vin_pin)){ 
        printk("Whoopsies! Could not set pin: %u\r\n", vin_pin);
        vin_valid = 0;
        return 1;
    }


	return size;
}


static DEVICE_ATTR(out_speed, 0644, out_speed_show, out_speed_store);
static DEVICE_ATTR(in_speed, 0644, in_speed_show, in_speed_store);
static DEVICE_ATTR(out_pin, 0644, out_pin_show, out_pin_store);
static DEVICE_ATTR(in_pin, 0644, in_pin_show, in_pin_store);

static int my_open(struct inode *i, struct file *f)
{
    printk("Morse driver open\r\n");
    return 0;
}

static int my_close(struct inode *i, struct file *f)
{
    printk("Morse driver close\r\n");
    stop_talking_idiot = 0;
    return 0;
}

static ssize_t dummy_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    if(!stop_talking_idiot){
        stop_talking_idiot = 1;
        totals = 0;
        printk("Morse driver read\r\n");
    }
    if(printable){
            printable = 0;
            snprintf(buf, size, "%c", char_to_print);
            return strlen(buf);
        }
    else{
            snprintf(buf, size, "%c", null_char);
            return 1;
    }
}

static ssize_t dummy_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
    char lcl_buf[64];

    memset(lcl_buf, 0, sizeof(lcl_buf));

    if (copy_from_user(lcl_buf, buf, min(size, sizeof(lcl_buf))))
        {
            return -EACCES;
        }

    printk("CS444 Dummy driver write %ld bytes: %s\r\n", size, lcl_buf);

    word_write = kstrndup(buf, size, GFP_KERNEL);
    lplace = 0;
    mplace = 0;

    sssize = size;
    GETE = 1;

    return size;
}
 
static struct file_operations dummy_fops =
{
    .owner = THIS_MODULE,
    .open = my_open,
    .read = dummy_read,
    .write = dummy_write,
    .release = my_close
};


static void morse_function(unsigned long data){
    int i = lplace; //Shorthand for lplace
    int j = mplace; //Shorthand for mplace
    int dot = 0;
    int space = 0;
    int spacebar = 0;
    unsigned long delay = msecs_to_jiffies(1000);
    int pin_state = phase;
    char * code;
    char * word = NULL;

   word = kstrndup(*word_w, sssize, GFP_KERNEL);
   if(vout_valid && word != NULL){
   if((i < strlen(word) - 1 && (word[i + 1] == '\n' || word[i + 1] == '\0' || word[i + 1] == ' ')))
        spacebar = 1;

    code = mEncode(word[i]);    

    if(pin_state)
        pin_state = 0;
    else
        pin_state = 1;


    if(code[j] == '.')
        dot = 1;

    if(j == strlen(code) - 1){
        space = 1;
    }


    if(dot){
    switch (phase) {
    case 0:
        delay = msecs_to_jiffies(vout_speed);
        phase++;
        break;
    default:
        delay = msecs_to_jiffies(vout_speed);
        if(space && !spacebar)
            delay = msecs_to_jiffies(vout_speed * 3);
        else if(space && spacebar)
            delay = msecs_to_jiffies(vout_speed * 7);
        phase = 0;
        j++;
        if(j >= strlen(code)){
            j = 0;
            i++;
        }
        break;
    }}  
    else{
    switch (phase) {
    case 0:
        delay = msecs_to_jiffies(vout_speed * 3);
        phase++;
        break;

    default:
        delay = msecs_to_jiffies(vout_speed);
        if(space && !spacebar)
            delay = msecs_to_jiffies(vout_speed * 3);
        else if(space && spacebar)
            delay = msecs_to_jiffies(vout_speed * 7);
        phase = 0;
        j++;
        if(j >= strlen(code)){
            j = 0;
            i++;
        }
        break;
    }
    }

    if(i >= strlen(word)){
        i = 0;
        j = 0;
        GETE = 0;
    }

    while(word[i] == ' ' || word[i] == '\0' || word[i] == '\n'){
        i++;
        if(i >= strlen(word)){
            i = 0;
            j = 0;
            GETE = 0;
        }
    }

    lplace = i;
    mplace = j;

    if(vout_valid){
        //printk("Setting pin out to state %d\n", pin_state);
        gpio_set_value(vout_pin, pin_state);
    }
    }
    else{
    }
    
    mod_timer(&my_timer, jiffies + delay);
}
 
static int __init dummy_init(void)
{
    int ret;
    int rc;
    struct device *dev_ret;
 
    // Allocate the device
    if ((ret = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, "morse_gpio")) < 0)
    {
        return ret;
    }
 
    cdev_init(&c_dev, &dummy_fops);
 
    if ((ret = cdev_add(&c_dev, dev, MINOR_CNT)) < 0)
    {
        return ret;
    }
     
    // Allocate the /dev device (/dev/morse_gpio)
    if (IS_ERR(cl = class_create(THIS_MODULE, "char")))
    {
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(cl);
    }
    if (IS_ERR(dev_ret = device_create(cl, NULL, dev, NULL, "morse_gpio")))
    {
        class_destroy(cl);
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(dev_ret);
    }


    rc = device_create_file(dev_ret, &dev_attr_in_speed);
    if(rc){
        return rc;
    }
 
    rc = device_create_file(dev_ret, &dev_attr_out_speed);
    if(rc){
        return rc;
    }

    rc = device_create_file(dev_ret, &dev_attr_in_pin);
    if(rc){
        return rc;
    }

    rc = device_create_file(dev_ret, &dev_attr_out_pin);
    if(rc){
        return rc;
    }

    if(gpio_request(vout_pin, "worthless label here\0")){
        printk("Whoopsies! Could not request pin: %u\r\n", vout_pin);
        vout_valid = 0;
        return 1;
    }
    if(gpio_direction_output(vout_pin, 0)){ 
        printk("Whoopsies! Could not set pin: %u\r\n", vout_pin);
        vout_valid = 0;
        return 1;
    }
 
    if(gpio_request(vin_pin, "worthless label 2 here\0")){
        printk("Whoopsies! Could not request pin: %u\r\n", vin_pin);
        vin_valid = 0;
        return 1;
    }
    if(gpio_direction_input(vin_pin)){ 
        printk("Whoopsies! Could not set pin: %u\r\n", vin_pin);
        vin_valid = 0;
        return 1;
    }
 
    printk("Morse Driver has been loaded!\r\n");
    
    setup_timer(&my_timer, morse_function, 0); 
    morse_function(0);
    setup_timer(&boy_timer, morse_function_dos, 0); 
    morse_function_dos(0);
    return 0;
}
 
static void __exit dummy_exit(void)
{
    //device_remove_file(dev, &dev_attr_in_speed);
    //device_remove_file(dev, &dev_attr_out_speed);
    //device_remove_file(dev, &dev_attr_in_pin);
    //device_remove_file(dev, &dev_attr_out_pin);
    device_destroy(cl, dev);
    class_destroy(cl);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, MINOR_CNT);
}
 
module_init(dummy_init);
module_exit(dummy_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Group 7 CS 444");
MODULE_DESCRIPTION("CS444 Morse Driver");
