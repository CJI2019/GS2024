my_id = nil;
my_name = nil;
my_xPos = nil;
my_yPos = nil;
my_type = nil;
my_level = nil;
--배치 좌표, 이름, TYPE, Level

function Set_info(id)
   my_id = id;
   my_name = "obj_"..id;
   return my_name;
end
