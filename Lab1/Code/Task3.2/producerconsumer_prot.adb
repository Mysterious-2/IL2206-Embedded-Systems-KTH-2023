with Ada.Text_IO;
use Ada.Text_IO;

with Ada.Real_Time;
use Ada.Real_Time;

with Buffer;
use Buffer;

with Ada.Numerics.Discrete_Random;

procedure ProducerConsumer_Prot is

   N : constant Integer := 10; -- Number of produced and consumed tokens per task
	X : constant Integer := 3; -- Number of producers and consumers
	
   -- Random Delays
   subtype Delay_Interval is Integer range 50..250;
   package Random_Delay is new Ada.Numerics.Discrete_Random (Delay_Interval);
   DG : Random_Delay.Generator;

   -- ==> Complete code: Use Buffer
   B : CircularBuffer;
   subtype Item_Interval is Integer range 100..999;
   package Random_Item is new Ada.Numerics.Discrete_Random (Item_Interval);
   IG : Random_Item.Generator;
   It : Item_Interval;

   task type Producer;

   task type Consumer;

   task body Producer is
      Next : Time;
   begin
      Next := Clock;
      for I in 1..N loop
         It := Random_Item.Random(IG);
         -- ==> Complete code: Write to Buffer
         Put_Line("Putting item " & Integer'Image(It)
                  & " into the buffer...");
			B.Put(It);
         -- Next 'Release' in 50..250ms
         Next := Next + Milliseconds(Random_Delay.Random(DG));
         delay until Next;
      end loop;
   end;

   task body Consumer is
      Next : Time;
      X : Integer;
   begin
      Next := Clock;
      for I in 1..N loop
         -- ==> Complete code: Read from Buffer
			Put_Line("Trying to get item from the buffer");
			B.Get(X);
			Put_Line("Successfully got item " & Integer'Image(X)
			         & " from the buffer");
         Put_Line(Integer'Image(X));
         Next := Next + Milliseconds(Random_Delay.Random(DG));
         delay until Next;
      end loop;
   end;
	
	P: array (Integer range 1..X) of Producer;
	C: array (Integer range 1..X) of Consumer;
	
begin -- main task
   null;
end ProducerConsumer_Prot;


