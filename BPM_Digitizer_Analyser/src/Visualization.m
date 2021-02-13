%% ----------------------------------------------------------------------------------------------------
%% ******* Matlab script for prototyping real-time vizualization application for BPM-80 ************ %%
%% ----------------------------------------------------------------------------------------------------

%% Create COM port object to communicate with SAM4E Xplained Pro

a = zeros(8334,1);
s = serialport('COM4', 115200); %assigns the object s to serial port


%% Read the data samples of an individual BPM-80 cycle (for testing purposes)

while read(s,1,'char') == 46
end 
for i = 1:8334
      a(i) = read(s,1,'uint16');
end

subplot(2,2,1:2);
p = plot(a);
title('Oscilloscope display of collector signal');
axis([0 17000 0 2000])


%% Read BPM-80 beam parameter values

message = "";
for i = 1:28
    message = strcat(message, readline(s));
end

display(message)

%% Delete COM port object

delete(s);