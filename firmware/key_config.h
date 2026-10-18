#ifndef _KEY_CONFIG_H
#define _KEY_CONFIG_H

#include "ch32fun.h"
#include <string.h>

#define CONFIG_FLASH_ADDR   ((uint32_t)0x0000F000)
#define CONFIG_MAGIC        0x4B437201
#define CONFIG_VERSION      0x0001
#define CONFIG_KEYS         16

typedef struct {
    uint8_t mod;
    uint8_t key;
} Action;

typedef struct {
    uint32_t magic;
    uint32_t version;
    Action map[CONFIG_KEYS];
} KeyConfig;

static inline void flash_unlock(void)
{
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
}

static inline void flash_lock(void)
{
    FLASH->CTLR |= FLASH_CTLR_LOCK;
}

static inline void flash_wait_busy(void)
{
    while (FLASH->STATR & FLASH_STATR_BSY);
}

static int flash_page_erase_1k(uint32_t addr)
{
    flash_wait_busy();
    flash_unlock();
    FLASH->CTLR |= FLASH_CTLR_PER;
    FLASH->ADDR = addr;
    FLASH->CTLR |= FLASH_CTLR_STRT;
    flash_wait_busy();
    FLASH->CTLR &= ~FLASH_CTLR_PER;
    flash_lock();
    return 0;
}

static int flash_program_word(uint32_t addr, uint32_t data)
{
    flash_wait_busy();
    flash_unlock();
    FLASH->CTLR |= FLASH_CTLR_PG;
    *(volatile uint32_t *)addr = data;
    flash_wait_busy();
    FLASH->CTLR &= ~FLASH_CTLR_PG;
    flash_lock();
    return 0;
}

static void config_set_defaults(KeyConfig *cfg, const Action *default_map)
{
    cfg->magic = CONFIG_MAGIC;
    cfg->version = CONFIG_VERSION;
    memcpy(cfg->map, default_map, sizeof(cfg->map));
}

static int config_save(const KeyConfig *cfg)
{
    flash_page_erase_1k(CONFIG_FLASH_ADDR);

    uint32_t *src = (uint32_t *)cfg;
    uint32_t words = sizeof(KeyConfig) / sizeof(uint32_t);
    for (uint32_t i = 0; i < words; i++) {
        if (flash_program_word(CONFIG_FLASH_ADDR + i * 4, src[i]) != 0)
            return 1;
    }
    return 0;
}

static int config_load(KeyConfig *cfg, const Action *default_map)
{
    const uint32_t *src = (const uint32_t *)CONFIG_FLASH_ADDR;

    if (src[0] == CONFIG_MAGIC && src[1] == CONFIG_VERSION) {
        memcpy(cfg, (const void *)CONFIG_FLASH_ADDR, sizeof(KeyConfig));
        return 0;
    }

    config_set_defaults(cfg, default_map);
    return 1;
}

#endif
