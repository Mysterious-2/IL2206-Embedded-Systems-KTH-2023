with Semaphores;
with Ada.Text_IO; use Ada.Text_IO;

procedure Semaphores_Test is

   S : Semaphores.CountingSemaphore(3, 3);

   task type TT is
      entry Start (Id : Integer);
   end TT;

   task body TT is
      Task_Id : Integer;
   begin
      accept Start (Id : Integer) do
         Task_Id := Id;
      end Start;
   Put_Line("Task " & Integer'Image(Task_Id) & " trying to acquire.");
   S.Wait; -- ask a permission from semaphore to run the task
   Put_Line("Task " & Integer'Image(Task_Id) & " acquired semaphore.");
   delay 0.5; -- wait a bit, aka task is running
   S.Signal; -- task is done
   Put_Line("Task " & Integer'Image(Task_Id) & " released semaphore.");
   end TT;

   My_Tasks : array (1 .. 5) of TT;
begin
   Put_Line ("In main");

   for I in My_Tasks'Range loop
      My_Tasks (I).Start (I);
   end loop;
end Semaphores_Test;
