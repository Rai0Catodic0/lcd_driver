#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>

#define LCD_ADDR 0x3f
#define LCD_CLEAR 0x01
#define LCD_HOME 0x02
#define LCD_ENTRY_MODE 0x04
#define LCD_DISPLAY_CTRL 0x08
#define LCD_FUNCTION_SET 0x20

void lcd_send_cmd(const struct device *i2c_dev, uint8_t cmd)
{
    uint8_t data[4] = {
        (cmd & 0xF0) | 0x04, // High nibble + EN=1, RS=0
        (cmd & 0xF0) | 0x00, // High nibble + EN=0, RS=0
        ((cmd << 4) & 0xF0) | 0x04, // Low nibble + EN=1, RS=0
        ((cmd << 4) & 0xF0) | 0x00  // Low nibble + EN=0, RS=0
    };
    i2c_write(i2c_dev, data, sizeof(data), LCD_ADDR);
}

void lcd_send_data(const struct device *i2c_dev, uint8_t data)
{
    uint8_t buf[4] = {
        (data & 0xF0) | 0x05, // High nibble + EN=1, RS=1
        (data & 0xF0) | 0x01, // High nibble + EN=0, RS=1
        ((data << 4) & 0xF0) | 0x05, // Low nibble + EN=1, RS=1
        ((data << 4) & 0xF0) | 0x01  // Low nibble + EN=0, RS=1
    };
    i2c_write(i2c_dev, buf, sizeof(buf), LCD_ADDR);
}

void lcd_init(const struct device *i2c_dev)
{
    k_msleep(50);
    
    // Initialization sequence
    lcd_send_cmd(i2c_dev, 0x33);
    lcd_send_cmd(i2c_dev, 0x32);
    lcd_send_cmd(i2c_dev, LCD_FUNCTION_SET | 0x08); // 2 lines, 5x8 dots
    lcd_send_cmd(i2c_dev, LCD_DISPLAY_CTRL | 0x04); // Display on
    lcd_send_cmd(i2c_dev, LCD_CLEAR);
    lcd_send_cmd(i2c_dev, LCD_ENTRY_MODE | 0x02); // Increment cursor
    k_msleep(2);
}

int main(void)
{
    const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));
    
    if (!device_is_ready(i2c_dev)) {
        printk("I2C device not ready\n");
        return -1;
    }

    lcd_init(i2c_dev);
    
    while (1) {
        printk(" send message hello zephyr");
        lcd_send_cmd(i2c_dev, 0x80); // Move to first line
        const char *line1 = "Hello Zephyr!";
        for (int i = 0; i < strlen(line1); i++) {
            lcd_send_data(i2c_dev, line1[i]);
        }
        printk("send message i2c lcd test");
        lcd_send_cmd(i2c_dev, 0xC0); // Move to second line
        const char *line2 = "I2C LCD Test";
        for (int i = 0; i < strlen(line2); i++) {
            lcd_send_data(i2c_dev, line2[i]);
        }
        
        k_msleep(1000);
    }
    return 0;
}
