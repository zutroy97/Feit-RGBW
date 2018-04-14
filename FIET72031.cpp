#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include "FIET72031.h"

#ifndef REMOTE_REPEAT_TIME
#define REMOTE_REPEAT_TIME 100000 // 100 millisecond
#endif

volatile RFBuilder_t rfBuilder;
volatile RemoteCommand_t receivedCommand;


void RfLoop()
{
  static union RFPacket workingPacket;
  uint32_t pulseDuration;
  uint32_t now = micros();
  if (rfBuilder.interruptTimeStamp == 0)
  {
    return; // Nothing happened, nothing to do.
  }else{
    pulseDuration = rfBuilder.interruptTimeStamp - rfBuilder.lastChange;
    rfBuilder.lastChange = rfBuilder.interruptTimeStamp;
    rfBuilder.interruptTimeStamp = 0;
    rfBuilder.value = 0;
  }
  //Serial.print("D "); Serial.println(pulseDuration);
  if (pulseDuration >= MIN_HEADER_LENGTH && pulseDuration <= MAX_HEADER_LENGTH)
  {
    //Serial.println(bitPosition);
    //Serial.print("H");
    rfBuilder.isHighBitPosition = false; // Ignore the next (low) pulse
    rfBuilder.isCapturing = true; // Start capturing
    rfBuilder.bitPosition = 0; // Start building the value from 0
    workingPacket.value = 0; // Reset our values
    return;
  }
  if (rfBuilder.isCapturing == false)
  {
    return; // If we aren't capturing, nothing to do
  }
  if (pulseDuration >= MIN_SPACE_LENGTH && pulseDuration <= MAX_SPACE_LENGTH)
  {
    rfBuilder.value = 0; // Found a small pulse (high or low, doesn't matter)
    //Serial.print("0");
  }else if (pulseDuration >= MIN_MARK_LENGTH && pulseDuration <= MAX_MARK_LENGTH){
    rfBuilder.value = 1; // Found a long pulse (has to be high,
    //Serial.print("1");
  }else{
    rfBuilder.isCapturing = 0; // Our packet encountered interferrence. Abort.
    //Serial.print("X");
    return;
  }  
  if (rfBuilder.isHighBitPosition)
  { // Are we be interested in this pulse?
    rfBuilder.bitPosition++; // Keep track of how many bits we've collected.
    workingPacket.value = (workingPacket.value << 1) | rfBuilder.value; // Shift left and append value to least significant bit

    if (rfBuilder.bitPosition == 24) // Do we have the entire packet?
    {
      //Serial.print("RID: 0x"); Serial.print(workingPacket.id.remote, HEX);
      //Serial.print(" CID: 0x"); Serial.println(workingPacket.id.command, HEX);
      //Serial.print(" Value: "); Serial.println(workingPacket.value);
      rfBuilder.isCapturing = 0; // Don't capture until next header
      if (receivedCommand.isReady == 0)
      {
        if ((receivedCommand.packet.value != workingPacket.value) 
          || ( (now - receivedCommand.receiveTime) > REMOTE_REPEAT_TIME ) )
        { // Different Packet as last time OR timeout occurred
          receivedCommand.packet.value = workingPacket.value;
          receivedCommand.count = 0;
        }else{
          receivedCommand.count++;
        }
        receivedCommand.receiveTime = now;    
        receivedCommand.isReady = 1;
        //Serial.println("R");
      }
    }
  }
  rfBuilder.isHighBitPosition = !rfBuilder.isHighBitPosition;  
}
void handleRfInterrupt()
{
  rfBuilder.interruptTimeStamp = micros();
}