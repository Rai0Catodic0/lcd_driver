#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

static const struct device *bus = DEVICE_DT_GET(DT_NODELABEL(i2c0));

// LCD Commands
#define LCD_CLEAR_DISPLAY 0x01
#define LCD_RETURN_HOME 0x02
#define LCD_ENTRY_MODE_SET 0x04
#define LCD_DISPLAY_CONTROL 0x08
#define LCD_FUNCTION_SET 0x20
#define LCD_SET_DDRAM_ADDR 0x80

// Delays
#define LCD_POWER_ON_DELAY_MS 50
#define LCD_INIT_DELAY_US 100
#define LCD_CMD_DELAY_US 37
void debug_bin(uint8_t number) {
    char number_str[9];  // 8 bits + null terminator
    printk("The original number is %d\n", number);
    
    for (int i = 0; i < 8; i++) {
        uint8_t bit = (number >> (7 - i)) & 0b00000001;  // Start with MSB
        number_str[i] = bit ? '1' : '0';
    }
    number_str[8] = '\0';  // Null-terminate the string
    
    printk("The binary representation is: %s\n", number_str);
}
void send_data_half(const struct device *dev, uint8_t data, uint8_t addr,
                    uint8_t rs) {
  // uint8_t buffer;
  printk("sending command:\n");
  // High nibble + RS + Backlight ON (0x08)
  // buffer = (data & 0xF0) | (rs ? 0x01 : 0x00) | 0x08;
  uint8_t buffer[4] = {(data & 0xF0) | (rs ? 0x0d : 0x0c),
                       (data & 0xF0) | (rs ? 0x09 : 0x08),
                       ((data << 4) & 0xF0) | (rs ? 0x0d : 0x0c),
                       ((data << 4) & 0xF0) | (rs ? 0x09 : 0x08)};
  // Pulse EN (set high then low)
  for(int i = 0; i < 4; i++){
        debug_bin(buffer[i]);
    }
  i2c_write(dev, &buffer, sizeof(buffer), addr); // EN=1
  k_usleep(LCD_CMD_DELAY_US);
}

void init_my_lcd(const struct device *i2c_dev, uint8_t addr) {
  k_msleep(LCD_POWER_ON_DELAY_MS);

  // Initialization sequence
  send_data_half(i2c_dev, 0x33, addr, 0);
  k_msleep(50);
  send_data_half(i2c_dev, 0x32, addr, 0);
   k_msleep(50);
  send_data_half(i2c_dev, LCD_FUNCTION_SET | 0x08, addr, 0);
  k_msleep(50);
  send_data_half(i2c_dev, LCD_DISPLAY_CONTROL | 0x04, addr, 0);
  k_msleep(50);
  send_data_half(i2c_dev, LCD_CLEAR_DISPLAY, addr, 0);
  k_msleep(50);
  send_data_half(i2c_dev, LCD_ENTRY_MODE_SET | 0x02, addr, 0);
  k_msleep(2);
}

void send_string(const struct device *dev,uint8_t addr, char *string, int str_length){
  for(int i = 0; i < str_length; i++)
  {
    send_data_half(dev, string[i], addr,1);
    k_msleep(2);
  }
}
int main(void) {
  const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));
  uint8_t lcd_addr = 0x3F; // Try 0x27 if this doesn't work

  if (!device_is_ready(i2c_dev)) {
    printk("I2C device not ready\n");
    return -1;
  }
  init_my_lcd(i2c_dev, lcd_addr);
  // Set cursor to first line beginning
  send_data_half(i2c_dev, 0x80, lcd_addr, 0);
  k_msleep(5);
  char msg[12] = "LKCasa teste";
  send_string(i2c_dev, lcd_addr, msg, 12);
  
  // Set cursor to second line beginning
   send_data_half(i2c_dev, 0x0f, lcd_addr,0);
  return 0;
}
