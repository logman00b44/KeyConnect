#define LCD_RS 0x01
#define LCD_RW 0x02
#define LCD_EN 0x04
#define LCD_BL 0x08

#define LCD_SDA_BIT  (LCD_SDA_PIN - 16u)
#define LCD_SCL_BIT  (LCD_SCL_PIN - 16u)
#define LCD_SDA_SET  (1u << LCD_SDA_BIT)
#define LCD_SCL_SET  (1u << LCD_SCL_BIT)
#define LCD_SDA_CLR  (1u << (LCD_SDA_BIT + 16))
#define LCD_SCL_CLR  (1u << (LCD_SCL_BIT + 16))
#define LCD_SDA_SHIFT (LCD_SDA_BIT * 4u)
#define LCD_SCL_SHIFT (LCD_SCL_BIT * 4u)

#define GPIO_IN 0
#define GPIO_OD_10MHz 5

static void lcd_pin_init(void)
{
    RCC->APB2PCENR |= RCC_APB2Periph_AFIO;
    AFIO->PCFR1 = (AFIO->PCFR1 & ~AFIO_PCFR1_SWJ_CFG) | AFIO_PCFR1_SWJ_CFG_DISABLE;

    uint32_t cfg = GPIOC->CFGXR;
    cfg &= ~((uint32_t)0x0F << LCD_SDA_SHIFT);
    cfg |=  (uint32_t)GPIO_OD_10MHz << LCD_SDA_SHIFT;
    cfg &= ~((uint32_t)0x0F << LCD_SCL_SHIFT);
    cfg |=  (uint32_t)GPIO_OD_10MHz << LCD_SCL_SHIFT;
    GPIOC->CFGXR = cfg;

    GPIOC->BSXR = LCD_SDA_SET;
    GPIOC->BSXR = LCD_SCL_SET;
}

static void i2c_delay(void)
{
    Delay_Us(5);
}

static void sda_high(void) { GPIOC->BSXR = LCD_SDA_SET; }
static void sda_low(void)  { GPIOC->BSXR = LCD_SDA_CLR; }
static void scl_high(void) { GPIOC->BSXR = LCD_SCL_SET; }
static void scl_low(void)  { GPIOC->BSXR = LCD_SCL_CLR; }
static uint8_t sda_read(void) { return (GPIOC->INDR >> LCD_SDA_PIN) & 1; }

static void i2c_start(void)
{
    sda_high(); scl_high(); i2c_delay();
    sda_low();  i2c_delay();
    scl_low();  i2c_delay();
}

static void i2c_stop(void)
{
    sda_low();  i2c_delay();
    scl_high(); i2c_delay();
    sda_high(); i2c_delay();
}

static uint8_t i2c_write_byte(uint8_t data)
{
    for (int i = 0; i < 8; i++) {
        if (data & 0x80) sda_high(); else sda_low();
        i2c_delay();
        scl_high(); i2c_delay();
        scl_low();
        data <<= 1;
    }
    sda_high();
    i2c_delay();
    scl_high(); i2c_delay();
    uint8_t ack = !sda_read();
    scl_low();
    return ack;
}

static uint8_t pcf8574_write(uint8_t addr, uint8_t val)
{
    i2c_start();
    if (!i2c_write_byte(addr << 1)) { i2c_stop(); return 0; }
    i2c_write_byte(val);
    i2c_stop();
    return 1;
}

static uint8_t lcd_addr;

static void lcd_nibble(uint8_t nibble, uint8_t rs)
{
    if (!lcd_addr) return;
    uint8_t val = (nibble & 0xF0) | (rs ? LCD_RS : 0) | LCD_BL;
    pcf8574_write(lcd_addr, val | LCD_EN);
    i2c_delay();
    pcf8574_write(lcd_addr, val & ~LCD_EN);
    i2c_delay();
}

void lcd_cmd(uint8_t cmd)
{
    lcd_nibble(cmd & 0xF0, 0);
    lcd_nibble((cmd << 4) & 0xF0, 0);
    Delay_Ms(2);
}

void lcd_data(uint8_t data)
{
    lcd_nibble(data & 0xF0, 1);
    lcd_nibble((data << 4) & 0xF0, 1);
    Delay_Us(50);
}

void lcd_str(const char *str)
{
    while (*str) lcd_data((uint8_t)*str++);
}

void lcd_set_cursor(uint8_t col, uint8_t row)
{
    static const uint8_t addr[] = { 0x00, 0x40, 0x14, 0x54 };
    lcd_cmd(LCD_SET_DDRAM | (addr[row] + col));
}

void lcd_init(void)
{
    lcd_pin_init();
    Delay_Ms(50);

    lcd_addr = 0;
    static const uint8_t probe[] = { 0x20, 0x27, 0x38, 0x3F };
    for (int i = 0; i < 4; i++) {
        if (pcf8574_write(probe[i], LCD_BL | LCD_EN)) {
            lcd_addr = probe[i];
            break;
        }
    }
    if (!lcd_addr)
        return;

    lcd_nibble(0x30, 0); Delay_Ms(5);
    lcd_nibble(0x30, 0); Delay_Us(150);
    lcd_nibble(0x30, 0); Delay_Us(150);
    lcd_nibble(0x20, 0); Delay_Us(150);
    lcd_cmd(LCD_FUNCTION);
    lcd_cmd(LCD_DISPLAY_ON);
    lcd_cmd(LCD_CLEAR); Delay_Ms(2);
    lcd_cmd(LCD_ENTRY_MODE);
}
