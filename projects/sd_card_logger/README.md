<!--
General guidelines
These are just guidelines, not strict rules - document however seems best.
A README for a firmware-only project (e.g. Babydriver, MPXE, bootloader, CAN explorer) should answer the following questions:
    - What is it?
    - What problem does it solve?
    - How do I use it? (with usage examples / example commands, etc)
    - How does it work? (architectural overview)
A README for a board project (powering a hardware board, e.g. power distribution, centre console, charger, BMS carrier) should answer the following questions:
    - What is the purpose of the board?
    - What are all the things that the firmware needs to do?
    - How does it fit into the overall system?
    - How does it work? (architectural overview, e.g. what each module's purpose is or how data flows through the firmware)
-->
# sd_card_logger

## Description

API for interfacing with SD cards through SPI (Serial Peripheral Interface). Currently allows only basic block read/writes, but other functions should also be implementable quite seamlessly. Meant to be used for solar car telemetry later down the line (saving telemetry data on-vehicle for later review). Note that this library currently does not support maintenance of a filesystem on the SD card; implementation of that should come later, once it's confirmed that this library works as intended.

Most information on how to implement this was sourced from [the SD specification](https://academy.cba.mit.edu/classes/networking_communications/SD/SD.pdf) and [Elm-Chan's notes on the subject](http://elm-chan.org/docs/mmc/mmc_e.html). Some information was also acquired from [Kingston Technology's datasheet for their microSDXC card](https://www.kingston.com/datasheets/SDCIT-specsheet-64gb_jp.pdf), which although very specific does have some useful info on it pertaining to electrical requirements.

## Provided Methods
### `sd_card_init (SpiPort spi)`
Initializes the SD card attached to the given `SpiPort`. You must run this before attempting to read/write blocks from the SD card. Also, be sure to run `spi_init` with the provided settings before running `sd_card_init`:

```c
static SpiSettings spi_settings = {
  .baudrate = 10000000,  // 10 Mhz
  .mode = SPI_MODE_0,
  .mosi = { .port = GPIO_PORT_B, .pin = 15 },
  .miso = { .port = GPIO_PORT_B, .pin = 14 },
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },
  .cs = { .port = GPIO_PORT_B, .pin = 12 },
};
```

**Returns**: `StatusCode`, where
- 0 (`STATUS_CODE_OK`): Successfully initialized
- 5 (`STATUS_CODE_TIMEOUT`): Initialization timed out. This is most likely due to the SD card not responding, either due to a faulty connection or a problem with the SD card itself.
- 10 (`STATUS_CODE_INTERNAL_ERROR`): The SD card was identified but it is deemed to be incompatiable with the board.

### `sd_read_blocks (SpiPort spi, uint8_t* dest, uint32_t readAddr, uint32_t numberOfBlocks)`
Reads `numberOfBlocks` data blocks starting at `readAddr`, each **512 bytes** in size from the SD card attached at `spi` into local memory pointed to by `dest`. Most SD cards function optimally when the `readAddr` is **sector-aligned** to 512 bytes, so make sure that your `readAddr` is a multiple of 512 before attempting a read. Some SD cards might outright reject the operation if this condition isn't satisfied.

**Returns**: `StatusCode`, where
- 0 (`STATUS_CODE_OK`): Operation Success
- 2 (`STATUS_CODE_INVALID_ARGS`): Invalid arguments provided
- 5 (`STATUS_CODE_TIMEOUT`): Operation timed out
- 10 (`STATUS_CODE_INTERNAL_ERROR`): SD card encountered an error while reading. The data at `dest` may have been partially overwritten in the process, so if it's important make sure you have a backup.

### `sd_write_blocks (SpiPort spi, uint8_t* src, uint32_t writeAddr, uint32_t numberOfBlocks)`
Writes `numberOfBlocks * 512` bytes from `src` into the address `writeAddr` on the SD card attached at `spi`.

Again, make sure that your write address is sector-aligned.

**Returns**: `StatusCode`, where
- 0 (`STATUS_CODE_OK`): Successfully initialized
- 2 (`STATUS_CODE_INVALID_ARGS`): Invalid arguments provided
- 5 (`STATUS_CODE_TIMEOUT`): Operation timed out
- 10 (`STATUS_CODE_INTERNAL_ERROR`): SD card encountered an error while writing. Unsure whether this leaves the data on the card in a corrupted state (it probably does?)

### `sd_is_initialized (SpiPort spi)`
**Returns**: `bool` (True if SD card has been initialized on `spi`, False otherwise)

## Architecture

**NOTE**: This library required changes to `libraries/ms-common/src/arm/spi.c` in order to enable block reads/writes, since the chip select (CS) needs to be held down for the entire duration of an SD read/write. 

Internally, most operations on the SD card are performed in SPI mode by sending a series of **commands** to the card. This operation is abstracted in `sd_binary.c` as simply `prv_send_cmd`, which sends a command to the SD card and begins a transaction. For most commands other than read/write, sending the command alone is pretty much the full transaction, so `prv_send_cmd` has to be immediately followed up with `prv_end_transaction`, which pulls CS up to officially end transmissions. For read and write, the CS line has to be held down for the entire duration while blocks are being sent, so `prv_end_transaction` is called only once the command CMD12 (end block transfer) is sent and handled by the SD card.

`prv_send_cmd` itself also relies on a few other abstractions which I'll go over here. 

- `prv_write_read_byte`: Writes a byte to the SD card, then waits for a single byte response and returns if found.
- `prv_read_[byte/multi]`: Reads bytes received from the SD card.
- `prv_wait_sd_response`: Waits for a command response from the SD card, normally sent on the SPI MISO (master-in-slave-out) line, and writes it to a provided memory address.
- `prv_sd_get_next_data_token`: Waits for the data tokens that serve as a header for block read/writes. Usually followed up with code that reads the following block of data.
- `prv_sd_get_next_write_data_response`: Waits for the data response byte that the SD card sends after every sent block during block writes.



