# KeyConnect R1 Configuration Protocol

## Goal

Allow a host application to read/write keyboard configuration (keymap, layer
assignments, etc.) over standard HID Feature reports — **no custom driver
needed**.

## Report Layout

### Report 1 – Keyboard Input (already implemented)

| Offset | Size | Description          |
|--------|------|----------------------|
| 0      | 1    | Modifier byte        |
| 1      | 1    | Reserved (0)         |
| 2–7    | 6    | Key codes (6KRO)     |

### Report 2 – Feature: Read Keymap Entry (Report ID = 0x01)

Two-step exchange:

**Step 1 – SET_REPORT** (set read cursor):
| Offset | Size | Description                |
|--------|------|----------------------------|
| 0      | 1    | Report ID (0x01)           |
| 1      | 1    | Layer (0–15)               |
| 2      | 1    | Physical key index (0–15)  |
| 3      | 1    | Reserved (0)               |
| 4      | 1    | Reserved (0)               |

**Step 2 – GET_REPORT** (read entry at cursor):
| Offset | Size | Description                |
|--------|------|----------------------------|
| 0      | 1    | Report ID (0x01)           |
| 1      | 1    | Layer (echo)               |
| 2      | 1    | Physical key index (echo)  |
| 3      | 1    | Modifier byte (returned)   |
| 4      | 1    | HID usage code (returned)  |

Send SET_REPORT with layer+index first, then GET_REPORT to read the mapped modifier+usage for that key.

### Report 3 – Feature: Write Keymap Entry (Report ID = 0x02)

| Offset | Size | Description                |
|--------|------|----------------------------|
| 0      | 1    | Report ID (0x02)           |
| 1      | 1    | Layer (0–15)               |
| 2      | 1    | Physical key index (0–15)  |
| 3      | 1    | Modifier byte              |
| 4      | 1    | HID usage code             |

Send → Device stores the mapping in RAM.

### Report 4 – Feature: Save to Flash (Report ID = 0x03)

| Offset | Size | Description      |
|--------|------|------------------|
| 0      | 1    | Report ID (0x03) |
| 1      | 1    | Magic byte (0xA5)|

Send with magic → Device commits current mapping to flash.

### Report 5 – Feature: Read Matrix State (Report ID = 0x04)

**GET_REPORT** only (no parameter needed):

| Offset | Size | Description                          |
|--------|------|--------------------------------------|
| 0      | 1    | Report ID (0x04)                     |
| 1      | 1    | Bits 0–7 of debounced key state      |
| 2      | 1    | Bits 8–15 of debounced key state     |

Bit `n` in the 16-bit mask corresponds to physical key index `n` (A=0 ... P=15).
1 = pressed (pin pulled low). Returns `0xFF` on error.
