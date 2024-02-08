#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "math.h"

// Defining the frame start, end, and esc bytes
#define START 0xAA
#define END 0xBB
#define ESCAPE 0xDD

typedef struct dataPacket {
  // 2 bytes for ID (11 bits; Don't have to worry about padding bytes with 5 extra bits) + 8 bytes
  // of data + 8 bytes for worst case scenario where all bytes are special characters requiring a
  // preceding escape character = 18 bytes total
  /*
  Not sure if I can for this program but may make the dataFrame an unsized array that can fit
  any size bytes of data based on how many special characters it contains.
  Or alternatively I can make this data array bigger than worst case scenario (like 20 or 24 bytes)
  and fill it with all the data and necessary esc bytes and checksum and include the frame end byte
  at the end of the same array
  */
  uint8_t data[34];
} dataPacket;

// Function to create the spaces in the data array for the Hamming bits and resized to a 16 byte
// array for a 128 bit Hamming code
uint8_t *array_with_parity_bits_create(uint8_t *inArr) {
  uint8_t outArray[16] = { 0 };

  /*
  Since there will be 8 parity bits (7 parities for the split message bits and also 1 more parity
  for the overall message parity) These bits are arranged such that first few parity bits in
  outArray are PPPD PDDD PDDD DXXX For first byte, first bit is reserved for message, next two are
  parity bits at 2^0, 2^1
  */

  // Need to set 4th bit to first bit of message, then last 3 bits after 4th bit, form is
  // PPPD PDDD PDDD DDDD...
  outArray[0] |= (inArr[0] & 0x80) >> 3;  // 1000 0000
  outArray[0] |= (inArr[0] & 0x70) >> 4;  // 0111 0000
  outArray[1] |= (inArr[0] & 0x0F) << 3;  // 0000 1111
  outArray[1] |= (inArr[1] & 0xE0) >> 5;  // 1110 0000
  outArray[2] |= (inArr[1] & 0x1F) << 2;  // 0001 1111
  outArray[2] |= (inArr[2] & 0xC0) >> 6;  // 1100 0000
  outArray[3] |= (inArr[2] & 0x3F) << 2;  // 0011 1111
  outArray[3] |= (inArr[3] & 0xC0) >> 6;  // 1100 0000
  outArray[4] |= (inArr[3] & 0x3F) << 1;  // 0011 1111
  outArray[4] |= (inArr[4] & 0x80) >> 7;  // 1000 0000
  outArray[5] |= (inArr[4] & 0x7F) << 1;  // 0111 1111
  outArray[5] |= (inArr[5] & 0x80) >> 7;  // 1000 0000
  outArray[6] |= (inArr[5] & 0x7F) << 1;  // 0111 1111
  outArray[6] |= (inArr[6] & 0x80) >> 7;  // 1000 0000
  outArray[7] |= (inArr[6] & 0x7F) << 1;  // 0111 1111
  outArray[7] |= (inArr[7] & 0x80) >> 7;  // 1000 0000
  outArray[8] |= (inArr[7] & 0x7F);       // 0111 1111
  outArray[9] |= inArr[8];
  outArray[10] |= inArr[9];  // Since inArr consists of 8 bytes of CAN data and 2 bytes for the ID;
                             // this array is only of size 10 bytes until index [9]
  // Thus will leave the rest of the bits in outArray as is due to those all being 0 valued padding
  // bits for the purpose of the size of the array for Hamming code
}

// This function should calculate and place the correct parity value at the redundant Hamming bit
// locations and output the final Hamming encoded array
/* maybe I'll add another parameter for the function which keeps track of which parity bit we're
setting the value for currently; would mean that the function will have to be called for each parity
bit to be set individually; Likely will use switch statements or something similar*/
uint8_t *parity_value_set(
    uint8_t *defaultParityValueArr,
    uint8_t parityBit) {  // parityBit value is the parity bit that you want to set the value for;
                          // will be evaluated using switch statements
  uint8_t encodeArray[16] = defaultParityValueArr;
  uint8_t parityValue = 0;  // Variable to store parity bit value until end of loop
  switch (parityBit) {
    case 0:
      // Code for P0 parity bit that holds parity of all bits in message (at index 0 in array); Bit
      // mask: 1000 0000
      for (uint8_t j = 0; j < 16; j++) {
        for (uint8_t i = 0; i < 8; i++) {
          parityValue ^= (encodeArray[j] << i) & 0x80;
        }
      }
      encodeArray[0] |= parityValue;
      break;

    case 1:
      // Following is code for setting the first parity bit value P1 (at index 0 in array); Bit
      // mask: 0100 0000 (take 1 bit skip 1 bit...)
      for (uint8_t j = 0; j < 16; j++) {
        for (uint8_t i = 0; i < 8; i += 2) {
          parityValue ^= (encodeArray[j] << i) & 0x40;
        }
      }
      encodeArray[0] |= parityValue;
      break;

    case 2:
      // For P2 (index 0); Bit mask: 0010 0000 (take 2 bits skip 2 bits...)
      for (uint8_t j = 0; j < 16; j++) {
        for (uint8_t i = 0; i < 6; i++) {
          if (i == 0 | i == 1 | i == 4 | i == 5) parityValue ^= (encodeArray[j] << i) & 0x20;
        }
      }
      encodeArray[0] |= parityValue;
      break;

    case 3:
      // For P3 (index 0); Bit mask: 0000 1000 (take 4 skip 4...)
      for (uint8_t j = 0; j < 16; j++) {
        for (uint8_t i = 0; i < 4; i++) {
          parityValue ^= (encodeArray[j] << i) & 0x08;
        }
      }
      encodeArray[0] |= parityValue;
      break;

    case 4:
      // For P4 (index 1); Bit mask: 1000 0000 (take 8 skip 8...)
      for (uint8_t j = 1; j < 16; j += 2) {
        for (uint8_t i = 0; i < 8; i++) {
          parityValue ^= (encodeArray[j] << i) & 0x80;
        }
      }
      encodeArray[1] |= parityValue;
      break;

    case 5:
      // For P5 (index 2); Bit mask: 1000 0000 (take 16 skip 16...)
      for (uint8_t j = 2; j < 16; j += 4) {
        for (uint8_t i = 0; i < 8; i++) {
          parityValue ^= ((encodeArray[j] ^ encodeArray[j + 1]) << i) & 0x80;
        }
      }
      encodeArray[2] |= parityValue;
      break;

    case 6:
      // For P6 (index 4); Bit mask: 1000 0000 (take 32 skip 32...)
      for (uint8_t j = 4; j < 16; j += 8) {
        for (uint8_t i = 0; i < 8; i++) {
          parityValue ^=
              ((encodeArray[j] ^ encodeArray[j + 1] ^ encodeArray[j + 2] ^ encodeArray[j + 3])
               << i) &
              0x80;
        }
      }
      encodeArray[4] |= parityValue;
      break;

    case 7:
      // For P7 (index 8); Bit mask: 1000 0000 (take 64 skip 64...)
      for (uint8_t j = 8; j < 16; j++) {
        for (uint8_t i = 0; i < 8; i++) {
          parityValue ^= (encodeArray[j] << i) & 0x80;
        }
      }
      encodeArray[8] |= parityValue;
      break;

    default:
      // Do nothing
  }
  return encodeArray;
}

/*
the function below is to convert the bytes in the data received from the CAN bus
into escape character separated bytes in case they contain the special characters themselves.
Function then returns array of bytes to be stored in dataPack. Not entirely sure how data will be
structured when it is received from the CAN bus so may have to change function based on the actual
structure but for now the function takes a char array of data as expected input of the data.
*/

// I will not be passing the raw data into this function but instead the data with the hamming code
// implemented on it
uint8_t *dataFrameSet(uint8_t *hammingEncodedArr) {
  uint8_t framePackagedData[34] = {
    0
  };  // twice + 2 the size of the hamming coded array (16 bytes) for worst case scenario overhead
      // in case all msg bytes need escape bytes and then 2 more for
  uint8_t i, j = 0;  // counters for arrays above; i for framePackagedData, j for hammingEncodedArr
  framePackagedData[i] = START;
  // Checking if hammingEncodedArr has special characters for any of its bytes then preceding those
  // bytes with an escape byte
  do {
    if (j <= 15) {  // Check if the variable is a bigger size than hammingEncodedArr array so don't
                    // try to access a non-existent value in array
      if (hammingEncodedArr[j] == START || hammingEncodedArr[j] == END ||
          hammingEncodedArr[j] == ESCAPE) {
        i++;
        framePackagedData[i] = ESCAPE;  // Making byte preceding the data byte equal to escape to
                                        // let program know it is a data byte
        i++;                            // Iterating to next byte to store actual data byte
        framePackagedData[i] = hammingEncodedArr[j];
        j++;
      } else {
        i++;
        framePackagedData[i] = hammingEncodedArr[j];
        j++;
      }
    } else {
      i++;
      framePackagedData[i] = END;
      j++;
    }
  } while (j < 16);  // Could run till 10 I guess since rest of the array are 0 bytes anyways
  return framePackagedData;
}

int main(void) {
  dataPacket *dataCAN = malloc(sizeOf(dataPacket));
  dataCAN->data = { 0 };
  uint8_t rawData[10] = { 0 };  // make equal to direct CAN data packet, not sure which function to
                                // use to get but will be 2 bytes CAN ID and 8 bytes CAN data
  uint8_t dataWithParityBits[16] = array_with_parity_bits_create(rawData);

  for (uint8_t i = 0; i < 8; i++) {
    dataWithParityBits = parity_value_set(dataWithParityBits, i);
  }  // Loop runs through all parity bits and sets them each individually one at a time

  dataCAN->data =
      dataFrameSet(dataWithParityBits);  // Resetting struct value to the final framed array
  return 0;
}
