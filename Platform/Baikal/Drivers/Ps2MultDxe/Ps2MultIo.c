/**
 * PS2MULT protocol implementation based on linux ps2mult driver
**/

#include <Library/BaseMemoryLib.h>
#include <Library/DwUartLib.h>

#include "Ps2Mult.h"

#define PS2MULT_KB_SELECTOR             0xA0
#define PS2MULT_MS_SELECTOR             0xA1
#define PS2MULT_ESCAPE                  0x7D
#define PS2MULT_BSYNC                   0x7E
#define PS2MULT_SESSION_START           0x55
#define PS2MULT_SESSION_END             0x56

#define PS2MULT_NUM_PORTS       2
#define PS2MULT_KBD_PORT        0
#define PS2MULT_MOUSE_PORT      1

STATIC CONST UINT8 Ps2MultControls[] = {
  PS2MULT_KB_SELECTOR,
  PS2MULT_MS_SELECTOR,
  PS2MULT_ESCAPE,
  PS2MULT_BSYNC,
  PS2MULT_SESSION_START,
  PS2MULT_SESSION_END,
};

/**
  Return current value of system performance counter.

  @return          timestamp value.
**/
STATIC
UINTN
GetTimeStamp (
  VOID
  )
{
  UINT64                    Result;

  Result = GetTimeInNanoSecond (GetPerformanceCounter ());
  Result /= 1000;

  return Result;
}

/**
  Return difference between Start and End timestamp values
  (in microseconds).

  @param Start      First timestamp value
  @param End        Second timestamp value

  @return           difference of timestamp values.
**/
STATIC
UINTN
TimeStampDifference (
  IN  UINTN                Start,
  IN  UINTN                End
  )
{
  UINTN                    Result;
  UINTN                    MaxTimeStamp;
  UINT64                   StartTicks;
  UINT64                   EndTicks;

  GetPerformanceCounterProperties (&StartTicks, &EndTicks);
  MaxTimeStamp = GetTimeInNanoSecond ((StartTicks > EndTicks) ?
                                      StartTicks :
                                      EndTicks);
  MaxTimeStamp /= 1000;
  if (Start > End) {
    Result = MaxTimeStamp - End + Start;
  } else {
    Result = End - Start;
  }

  return Result;
}

/**
  Return the count of data bytes in the queue.

  @param Queue     Pointer to instance of INPUT_DATA_QUEUE.

  @return          Count of the data bytes.
**/
UINTN
GetInputDataCount (
  IN INPUT_DATA_QUEUE       *Queue
  )
{
  if (Queue->Head <= Queue->Tail) {
    return Queue->Tail - Queue->Head;
  } else {
    return Queue->Tail + PS2MULT_INPUT_DATA_MAX_COUNT - Queue->Head;
  }
}

/**

  Read & remove several bytes from the input data buffer.

  @param Queue     Pointer to instance of INPUT_DATA_QUEUE.
  @param Count     Number of bytes to be read
  @param Buf       Store the results

  @retval EFI_SUCCESS   success to get the input data
  @retval EFI_NOT_READY queue is empty
**/
EFI_STATUS
PopInputDataHead (
  IN  INPUT_DATA_QUEUE      *Queue,
  IN  UINTN                 Count,
  OUT UINT8                 *Buf OPTIONAL
  )
{
  UINTN                     Index;

  //
  // Check the valid range of parameter 'Count'
  //
  if (GetInputDataCount (Queue) < Count) {
    return EFI_NOT_READY;
  }
  //
  // Retrieve and remove the values
  //
  for (Index = 0; Index < Count; Index++,
       Queue->Head = (Queue->Head + 1) % PS2MULT_INPUT_DATA_MAX_COUNT) {
    if (Buf != NULL) {
      Buf[Index] = Queue->Buffer[Queue->Head];
    }
  }

  return EFI_SUCCESS;
}

/**
  Push one byte to the input data buffer.

  @param Queue     Pointer to instance of INPUT_DATA_QUEUE.
  @param Data      The byte to push.
**/
VOID
PushInputDataTail (
  IN  INPUT_DATA_QUEUE      *Queue,
  IN  UINT8                 Data
  )
{
  if (GetInputDataCount (Queue) == PS2MULT_INPUT_DATA_MAX_COUNT - 1) {
    PopInputDataHead (Queue, 1, NULL);
  }

  Queue->Buffer[Queue->Tail] = Data;
  Queue->Tail = (Queue->Tail + 1) % PS2MULT_INPUT_DATA_MAX_COUNT;
}

/**
  Fill the keyboard and/or mouse input data queue.

  @param Dev Pointer to instance of PS2MULT_DEV

**/
STATIC
VOID
Ps2MultFillDataQueue (
  IN PS2MULT_DEV *Dev
  )
{
  UINT8 Data;

  if (Dev->UartBase) {
    while(DwUartPoll(Dev->UartBase)) {
      DwUartRead(Dev->UartBase, &Data, 1);
      if (Dev->Escape) {
        Dev->Escape = FALSE;
	if (Dev->InPort == PS2MULT_KBD_PORT) {
          PushInputDataTail (&Dev->KeyboardInputData, Data);
          DEBUG ((EFI_D_VERBOSE, "Ps2Mult: keyboard read %02X\r\n", Data));
	} // else discard mouse data
	continue;
      }
      switch(Data) {
      case PS2MULT_ESCAPE:
        Dev->Escape = TRUE;
        break;
      case PS2MULT_BSYNC:
        Dev->InPort = Dev->OutPort;
        break;
      case PS2MULT_KB_SELECTOR:
        Dev->InPort = PS2MULT_KBD_PORT;
        break;
      case PS2MULT_MS_SELECTOR:
        Dev->InPort = PS2MULT_MOUSE_PORT;
        break;
      case PS2MULT_SESSION_START:
      case PS2MULT_SESSION_END:
        break;
      default:
	if (Dev->InPort == PS2MULT_KBD_PORT) {
          PushInputDataTail (&Dev->KeyboardInputData, Data);
          DEBUG ((EFI_D_VERBOSE, "Ps2Mult: keyboard read %02X\r\n", Data));
	} // else discard mouse data
      }
    }
  }
}

/**
  Attempt to fill the keyboard and/or mouse input data queue.

  @param Dev Pointer to instance of PS2MULT_DEV

**/
STATIC
VOID
Ps2MultTryFillDataQueue (
  IN PS2MULT_DEV *Dev
  )
{
  UINTN                     CurrentTime;
  STATIC UINTN              QueryTime = 0;

  CurrentTime = GetTimeStamp ();
  if (TimeStampDifference (QueryTime, CurrentTime) >= PS2MULT_RX_DELAY) {
    QueryTime = CurrentTime;
    Ps2MultFillDataQueue (Dev);
  }
}

/**
  Read data from the keyboard input data queue.

  @param Dev       Pointer to instance of PS2MULT_DEV
  @param Data      Pointer to variable to store value in

  @return true if operation has succeeded, false otherwise

**/
BOOLEAN
KeyReadData (
  IN PS2MULT_DEV             *Dev,
  OUT UINT8                  *Data
  )

{
  if (GetInputDataCount (&Dev->KeyboardInputData) == 0) {
    Ps2MultTryFillDataQueue (Dev);
  }
  return PopInputDataHead (&Dev->KeyboardInputData, 1, Data) == EFI_SUCCESS;
}

/**
  Write data to the keyboard.

  @param Dev       Pointer to instance of KEYBOARD_CONSOLE_IN_DEV
  @param Data      Value to be written

**/
VOID
KeyWriteData (
  IN PS2MULT_DEV             *Dev,
  IN UINT8                   Data
  )
{
  UINT8 SndBuf[4];
  UINTN SndLen = 0;

  DEBUG ((EFI_D_VERBOSE, "Ps2Mult: keyboard write %02X\r\n", Data));
  if (Dev->UartBase) {
    if (Dev->OutPort != PS2MULT_KBD_PORT) {
      SndBuf[0] = PS2MULT_KB_SELECTOR;
      SndLen++;
      Dev->OutPort = PS2MULT_KBD_PORT;
    }
    if (ScanMem8(Ps2MultControls, sizeof(Ps2MultControls), Data) != NULL) {
      SndBuf[SndLen] = PS2MULT_ESCAPE;
      SndLen++;
    }
    SndBuf[SndLen] = Data;
    SndLen++;
    DwUartWrite(Dev->UartBase, SndBuf, SndLen);
  }
}

/**
  Start multiplexer session.

  @param Dev   - the device instance

  @return status of operation

**/
EFI_STATUS
Ps2MultStart (
  IN OUT PS2MULT_DEV *Dev
  )
{
  UINT8                   Data = PS2MULT_SESSION_START;

  Dev->OutPort = PS2MULT_KBD_PORT;
  Dev->InPort = PS2MULT_KBD_PORT;
  if (DwUartWrite(Dev->UartBase, &Data, 1))
    return EFI_SUCCESS;
  return EFI_DEVICE_ERROR;
}

/**
  Enable multiplexer input data polling.

  @param Dev   - the device instance

  @return status of operation

**/
EFI_STATUS
Ps2MultPollStart (
  IN OUT PS2MULT_DEV *Dev
  )
{
  EFI_STATUS              Status;

  Status = gBS->SetTimer (
                  Dev->TimerEvent,
                  TimerPeriodic,
                  PS2MULT_TIMER_INTERVAL
                  );
  if (EFI_ERROR (Status)) {
    Status      = EFI_OUT_OF_RESOURCES;
  }
  return Status;
}

/**
  Disable multiplexer input data polling.

  @param Dev   - the device instance

  @return status of operation

**/
EFI_STATUS
Ps2MultPollStop (
  IN OUT PS2MULT_DEV *Dev
  )
{
  EFI_STATUS              Status;

  Status = gBS->SetTimer (
                  Dev->TimerEvent,
                  TimerCancel,
                  PS2MULT_TIMER_INTERVAL
                  );
  if (EFI_ERROR (Status)) {
    Status      = EFI_OUT_OF_RESOURCES;
  }
  return Status;
}

/**
  Timer event handler: read a series of scancodes from
  input data queue and put them into memory scancode buffer.
  it read as much scancodes to either fill
  the memory buffer or empty the keyboard buffer.
  It is registered as running under TPL_NOTIFY

  @param Event       The timer event
  @param Context     A PS2MULT_DEV pointer

**/
VOID
EFIAPI
Ps2MultTimerHandler (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )

{
  PS2MULT_DEV              *Dev;

  Dev = (PS2MULT_DEV *) Context;

  if (Dev->KeyboardEnabled && !Dev->KeyboardErr) {
    KeyboardGetPacket (Dev);
  }
}
