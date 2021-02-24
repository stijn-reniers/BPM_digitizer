s = serialport('COM4', 115200); %assigns the object s to serial port

%%
while 1
   disp(read(s,1,'uint16')); 
    
end

