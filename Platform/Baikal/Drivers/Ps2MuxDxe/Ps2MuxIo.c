#include "Ps2Mux.h"
#include "Ps2Mouse.h"

typedef struct {
  UINTN                           OperationCount;
  EFI_I2C_OPERATION               Request;
} PS2MUX_I2C_REQUEST;

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
    return Queue->Tail + PS2MUX_INPUT_DATA_MAX_COUNT - Queue->Head;
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
       Queue->Head = (Queue->Head + 1) % PS2MUX_INPUT_DATA_MAX_COUNT) {
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
  if (GetInputDataCount (Queue) == PS2MUX_INPUT_DATA_MAX_COUNT - 1) {
    PopInputDataHead (Queue, 1, NULL);
  }

  Queue->Buffer[Queue->Tail] = Data;
  Queue->Tail = (Queue->Tail + 1) % PS2MUX_INPUT_DATA_MAX_COUNT;
}

/**
  Fill the keyboard and/or mouse input data queue.

  @param Dev Pointer to instance of PS2MUX_DEV

**/
STATIC
VOID
Ps2MuxFillDataQueue (
  IN PS2MUX_DEV *Dev
  )
{
  UINT8 Index;
  UINT8 Count;
  UINT8 Address;
  PS2MUX_I2C_REQUEST Request;
  UINT8 Message[PS2MUX_DATA_MESSAGE_SIZE];
  EFI_STATUS Status;

  if (Dev->I2cDevice != NULL) {
    Request.OperationCount = 1;
    Request.Request.Flags = I2C_FLAG_READ;
    Request.Request.LengthInBytes = sizeof(Message);
    Request.Request.Buffer = Message;
    Status = Dev->I2cDevice->QueueRequest (Dev->I2cDevice, 0,
                                           NULL,
                                           (EFI_I2C_REQUEST_PACKET *)&Request,
                                           NULL);
    if (!EFI_ERROR (Status)) {
      Count = Message[0] & 0x0F;
      Address = (Message[0] >> 4) & 0x0F;
      if (Count > (PS2MUX_DATA_MESSAGE_SIZE - 1)) {
        Count = PS2MUX_DATA_MESSAGE_SIZE - 1;
      }
      if (Address == KEYBOARD_DATA_ADDRESS) {
        for (Index = 0; Index < Count; Index++) {
          PushInputDataTail (&Dev->KeyboardInputData, Message[Index + 1]);
          DEBUG ((EFI_D_INFO, "Ps2Mux: keyboard read %02X\r\n", Message[Index + 1]));
        }
      } else if (Address == MOUSE_DATA_ADDRESS) {
        for (Index = 0; Index < Count; Index++) {
          PushInputDataTail (&Dev->MouseInputData, Message[Index + 1]);
          DEBUG ((EFI_D_INFO, "Ps2Mux: mouse read %02X\r\n", Message[Index + 1]));
        }
      }
    } else {
      if (Status != EFI_TIMEOUT) {
        DEBUG ((EFI_D_ERROR, "Ps2Mux: read failed: %r\r\n", Status));
      }
    }
  }
}

/**
  Attempt to fill the keyboard and/or mouse input data queue.

  @param Dev Pointer to instance of PS2MUX_DEV

**/
STATIC
VOID
Ps2MuxTryFillDataQueue (
  IN PS2MUX_DEV *Dev
  )
{
  UINTN                     CurrentTime;
  STATIC UINTN              QueryTime = 0;

  CurrentTime = GetTimeStamp ();
  if (TimeStampDifference (QueryTime, CurrentTime) >= PS2MUX_RX_DELAY) {
    QueryTime = CurrentTime;
    Ps2MuxFillDataQueue (Dev);
  }
}

/**
  Read data from the keyboard input data queue.

  @param Dev       Pointer to instance of PS2MUX_DEV
  @param Data      Pointer to variable to store value in

  @return true if operation has succeeded, false otherwise

**/
BOOLEAN
KeyReadData (
  IN PS2MUX_DEV              *Dev,
  OUT UINT8                  *Data
  )

{
  if (GetInputDataCount (&Dev->KeyboardInputData) == 0) {
    Ps2MuxTryFillDataQueue (Dev);
  }
  return PopInputDataHead (&Dev->KeyboardInputData, 1, Data) == EFI_SUCCESS;
}

/**
  Read data from the mouse input data queue.

  @param Dev       Pointer to instance of PS2MUX_DEV
  @param Data      Pointer to variable to store value in

  @return true if operation has succeeded, false otherwise

**/
BOOLEAN
MouseReadData (
  IN PS2MUX_DEV              *Dev,
  OUT UINT8                  *Data
  )

{
  if (GetInputDataCount (&Dev->MouseInputData) == 0) {
    Ps2MuxTryFillDataQueue (Dev);
  }
  return PopInputDataHead (&Dev->MouseInputData, 1, Data) == EFI_SUCCESS;
}

/**
  Write data to the keyboard.

  @param Dev       Pointer to instance of KEYBOARD_CONSOLE_IN_DEV
  @param Data      Value to be written

**/
VOID
KeyWriteData (
  IN PS2MUX_DEV              *Dev,
  IN UINT8                   Data
  )
{
  PS2MUX_I2C_REQUEST Request;
  EFI_STATUS Status;
  UINT8 Message[2];

  DEBUG ((EFI_D_INFO, "Ps2Mux: keyboard write %02X\r\n", Data));
  if (Dev->I2cDevice != NULL) {
    Request.OperationCount = 1;
    Request.Request.Flags = 0;
    Request.Request.LengthInBytes = sizeof(Message);
    Request.Request.Buffer = Message;
    Message[0] = ((KEYBOARD_DATA_ADDRESS & 0x0F) << 4) | 1;
    Message[1] = Data;
    Status = Dev->I2cDevice->QueueRequest (Dev->I2cDevice, 0,
                                           NULL,
                                           (EFI_I2C_REQUEST_PACKET *)&Request,
                                           NULL);
    if (EFI_ERROR (Status) && (Status != EFI_TIMEOUT)) {
      DEBUG ((EFI_D_ERROR, "Ps2Mux: keyboard write failed: %r\r\n", Status));
    }
  }
}

/**
  Write data to the mouse.

  @param Dev       Pointer to instance of KEYBOARD_CONSOLE_IN_DEV
  @param Data      Value to be written

**/
VOID
MouseWriteData (
  IN PS2MUX_DEV              *Dev,
  IN UINT8                   Data
  )
{
  PS2MUX_I2C_REQUEST Request;
  EFI_STATUS Status;
  UINT8 Message[2];

  DEBUG ((EFI_D_INFO, "Ps2Mux: mouse write %02X\r\n", Data));
  if (Dev->I2cDevice != NULL) {
    Request.OperationCount = 1;
    Request.Request.Flags = 0;
    Request.Request.LengthInBytes = sizeof(Message);
    Request.Request.Buffer = Message;
    Message[0] = ((MOUSE_DATA_ADDRESS & 0x0F) << 4) | 1;
    Message[1] = Data;
    Status = Dev->I2cDevice->QueueRequest (Dev->I2cDevice, 0,
                                           NULL,
                                           (EFI_I2C_REQUEST_PACKET *)&Request,
                                           NULL);
    if (EFI_ERROR (Status) && (Status != EFI_TIMEOUT)) {
      DEBUG ((EFI_D_ERROR, "Ps2Mux: mouse write failed: %r\r\n", Status));
    }
  }
}

/**
  Enable multiplexer input data polling.

  @param Dev   - the device instance

  @return status of operation

**/
EFI_STATUS
Ps2MuxPollStart (
  IN OUT PS2MUX_DEV *Dev
  )
{
  EFI_STATUS              Status;

  Status = gBS->SetTimer (
                  Dev->TimerEvent,
                  TimerPeriodic,
                  PS2MUX_TIMER_INTERVAL
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
Ps2MuxPollStop (
  IN OUT PS2MUX_DEV *Dev
  )
{
  EFI_STATUS              Status;

  Status = gBS->SetTimer (
                  Dev->TimerEvent,
                  TimerCancel,
                  PS2MUX_TIMER_INTERVAL
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
  @param Context     A PS2MUX_DEV pointer

**/
VOID
EFIAPI
Ps2MuxTimerHandler (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )

{
  PS2MUX_DEV              *Dev;

  Dev = (PS2MUX_DEV *) Context;

  if (Dev->MouseEnabled) {
    Ps2MouseGetPacket (Dev);
  }

  if (Dev->KeyboardEnabled && !Dev->KeyboardErr) {
    KeyboardGetPacket (Dev);
  }
}
