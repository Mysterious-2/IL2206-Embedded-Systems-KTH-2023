
  pragma Priority_Specific_Dispatching(
      FIFO_Within_Priorities, 2, 30
   );
   pragma Priority_Specific_Dispatching(
      Round_Robin_Within_Priorities, 1, 1
   );
with Ada.Text_IO; use Ada.Text_IO;
with Ada.Float_Text_IO;

with Ada.Real_Time; use Ada.Real_Time;

procedure MixedScheduling is

   package Duration_IO is new Ada.Text_IO.Fixed_IO(Duration);
   package Int_IO is new Ada.Text_IO.Integer_IO(Integer);

   Start : Time; -- Start Time of the System
   Calibrator: constant Integer := 2780; -- Calibration for correct timing
                                         -- ==> Change parameter for your architecture!
   Warm_Up_Time: constant Integer := 100; -- Warmup time in milliseconds

   function To_Float(TS : Time_Span) return Float is
      SC : Seconds_Count;
      Frac : Time_Span;
   begin
      Split(Time_Of(0, TS), SC, Frac);
      return Float(SC) + Time_Unit * Float(Frac/Time_Span_Unit);
   end To_Float;

   function F(N : Integer) return Integer is
      X : Integer := 0;
   begin
      for Index in 1..N loop
         for I in 1..500 loop
            X := X + I;
         end loop;
      end loop;
      return X;
   end F;

   task type T(Id: Integer; Prio: Integer; Phase: Integer; Period : Integer; 
               Computation_Time : Integer; Relative_Deadline: Integer) is
      pragma Priority(Prio);
   end T;

   task body T is
      Next : Time;
      Release: Time;
      Completed : Time;
      Response : Time_Span;
      Average_Response : Float;
      Absolute_Deadline: Time;
      WCRT: Time_Span; -- measured WCRT (Worst Case Response Time)
      Dummy : Integer;
      Iterations : Integer;
   begin
      Release := Clock + Milliseconds(Phase);
      delay until Release;
      Next := Release;
      Iterations := 0;
      Average_Response := 0.0;
      WCRT := Milliseconds(0);
      loop
         Next := Release + Milliseconds(Period);
         Absolute_Deadline := Release + Milliseconds(Relative_Deadline);
         for I in 1..Computation_Time loop
            Dummy := F(Calibrator); 
         end loop;   
         Completed := Clock;
         Response := Completed - Release;
         Average_Response := (Float(Iterations) * Average_Response + To_Float(Response)) / Float(Iterations + 1);
         if Response > WCRT then
            WCRT := Response;
         end if;
         Iterations := Iterations + 1;         
         Put("Task ");
         Int_IO.Put(Id, 1);
         Put("- Release: ");
         Duration_IO.Put(To_Duration(Release - Start), 2, 3);
         Put(", Completion: ");
         Duration_IO.Put(To_Duration(Completed - Start), 2, 3);
         Put(", Response: ");
         Duration_IO.Put(To_Duration(Response), 1, 3);
         Put(", WCRT: ");
         Ada.Float_Text_IO.Put(To_Float(WCRT), fore => 1, aft => 3, exp => 0);   
         Put(", Next Release: ");
         Duration_IO.Put(To_Duration(Next - Start), 2, 3);
         if Completed > Absolute_Deadline then 
            Put(" ==> Task ");
            Int_IO.Put(Id, 1);
            Put(" violates Deadline!");
         end if;
         Put_Line("");
         Release := Next;
         delay until Release;
      end loop;
   end T;

 

   -- tasks scheduled on first-in-first-out basis with respected priorities
   Task_1 : T(1, 30, Warm_Up_Time, 300, 100, 300);
   Task_2 : T(2, 20, Warm_Up_Time, 400, 100, 400);
   Task_3 : T(3, 10, Warm_Up_Time, 600, 100, 600);

  

 
   task type Background_Task(Id: Integer) is
      pragma Priority(1);
   end Background_Task;

   task body Background_Task is
      Dummy : Integer;
   begin
      delay until Clock + Milliseconds(100);
      loop
         for I in 1..100 loop
            Dummy := F(Calibrator);
         end loop;
         Put("Background Task ");
         Int_IO.Put(Id, 1);
         Put_Line(" completed.");
      end loop;
   end Background_Task;

   -- tasks scheduled on round-robin basis with equal priority
   Bg_Task_1 : Background_Task(1);
   Bg_Task_2 : Background_Task(2);
   Bg_Task_3 : Background_Task(3);

begin
   Start := Clock;
   null;
end MixedScheduling;
