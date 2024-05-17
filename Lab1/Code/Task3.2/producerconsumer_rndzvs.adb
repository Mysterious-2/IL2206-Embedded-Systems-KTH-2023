with Ada.Text_IO;
use Ada.Text_IO;

with Ada.Real_Time;
use Ada.Real_Time;

with Ada.Numerics.Discrete_Random;

procedure ProducerConsumer_Rndzvs is
	
   N : constant Integer := 10; -- Number of produced and consumed tokens per task
	X : constant Integer := 3; -- Number of producers and consumers	
	
   -- Random Delays
   subtype Delay_Interval is Integer range 50..250;   
   package Random_Delay is new Ada.Numerics.Discrete_Random (Delay_Interval);
   use Random_Delay;
   G : Generator;
   R : Delay_Interval;
   
   task type Buffer is
      entry Append(I : in Integer);
      entry Take(I : out Integer);
   end Buffer;

   task type Producer;

   task type Consumer;
   
   task body Buffer is
         Size: constant Integer := 4;
         type Index is mod Size;
         type Item_Array is array(Index) of Integer;
         B : Item_Array;
         In_Ptr, Out_Ptr : Index := 0;
         Count : Integer range 0..Size := 0;
   begin
      loop
         select
				-- => Complete Code: Service Append
				when Count < Size =>
				   accept Append(I : Integer) do
				      B(In_Ptr) := I;
                  In_Ptr := In_Ptr + 1;
                  Count := Count + 1;
               end Append;
         or
				-- => Complete Code: Service Take
				when Count > 0 =>
				   accept Take(I : out Integer) do
	               I := B(Out_Ptr);
                  Out_Ptr := Out_Ptr + 1;
                  Count := Count - 1;
               end Take;
         or
				-- => Termination
				delay 5.0;
				Put_Line("No request received for 5 seconds. Terminating...");
				exit;
         end select;
      end loop;
   end Buffer;
   
   B : Buffer; 
   
   task body Producer is
      Next : Time;
   begin
      Next := Clock;
      for I in 1..N loop
			R := Random(G);
         -- => Complete code: Write to B (Buffer)
         Put_Line("Putting item " & Integer'Image(R) &
                  " into buffer...");
         B.Append(R);
         -- Next 'Release' in 50..250ms
         Next := Next + Milliseconds(R);
         delay until Next;
      end loop;
   end;

   task body Consumer is
      Next : Time;
      X : Integer;
   begin
      Next := Clock;
      for I in 1..N loop
         -- Complete Code: Read from X
         Put_Line("Trying to get item from the buffer");
         B.Take(X);
         Put_Line("Successfully got item " & Integer'Image(X)
			         & " from the buffer");
         Put_Line(Integer'Image(X));
         Next := Next + Milliseconds(Random(G));
         delay until Next;
      end loop;
   end;
	
	P: array (Integer range 1..X) of Producer;
	C: array (Integer range 1..X) of Consumer;
	
begin -- main task
   null;
end ProducerConsumer_Rndzvs;


