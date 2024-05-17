with Ada.Text_IO;
use Ada.Text_IO;

with Ada.Real_Time;
use Ada.Real_Time;

with Ada.Numerics.Discrete_Random;

with Semaphores;
use Semaphores;

procedure ProducerConsumer_Sem is

	N : constant Integer := 10; -- Number of produced and consumed tokens per task
	X : constant Integer := 3; -- Number of producers and consumer

	-- Buffer Definition
	Size: constant Integer := 4;
	type Index is mod Size;
	type Item_Array is array(Index) of Integer;
	B : Item_Array;
	In_Ptr, Out_Ptr : Index := 0;
   Count : Integer range 0..Size;

   -- Random Delays
   subtype Delay_Interval is Integer range 50..250;
   package Random_Delay is new Ada.Numerics.Discrete_Random (Delay_Interval);
   use Random_Delay;
   G : Generator;
   R : Delay_Interval;

   -- => Complete code: Declation of Semaphores
	--    1. Semaphore 'NotFull' to indicate that buffer is not full
    NotFull : CountingSemaphore(Size, Size); -- currently not full
	--    2. Semaphore 'NotEmpty' to indicate that buffer is not empty
    NotEmpty : CountingSemaphore(Size, 0); -- currently empty
	--    3. Semaphore 'AtomicAccess' to ensure an atomic access to the buffer
    AtomicAccess : CountingSemaphore(1, 1); -- 1 access allowed, 0 access not allowed

   task type Producer;

   task type Consumer;

   task body Producer is
      Next : Time;
   begin
      Next := Clock;
      for I in 1..N loop
         R := Random(G);
         -- => Complete Code: Write to Buffer
         NotFull.Wait;
         AtomicAccess.Wait;
         Put_Line("AtomicAccess is busy, producer is working");
         B(In_Ptr) := R;
         Put_Line("Put item " & Integer'Image(R) & " into buffer");
         In_Ptr := In_Ptr + 1;
         Count := Count + 1;
         Put_Line("AtomicAccess is releasing, producer is done");
         AtomicAccess.Signal;
         NotEmpty.Signal;
         -- Next 'Release' in 50..250ms
         Next := Next + Milliseconds(Random(G));
         --delay until Next;
      end loop;
   end;

   task body Consumer is
      Next : Time;
      X : Integer;
   begin
      Next := Clock;
      for I in 1..N loop
         -- => Complete Code: Read from Buffer
         NotEmpty.Wait;
         AtomicAccess.Wait;
         Put_Line("AtomicAccess is busy, consumer is working");
         X := B(Out_Ptr);
         Put_Line("Got item " & Integer'Image(X) & " from buffer");
         Out_Ptr := Out_Ptr + 1;
         Count := Count - 1;
         Put_Line("AtomicAccess is releasing, consumer is done");
         AtomicAccess.Signal;
         NotFull.Signal;
			-- Next 'Release' in 50..250ms
         Next := Next + Milliseconds(Random(G));
         delay until Next;
      end loop;
   end;

	P: array (Integer range 1..X) of Producer;
	C: array (Integer range 1..X) of Consumer;

begin -- main task
   null;
end ProducerConsumer_Sem;
