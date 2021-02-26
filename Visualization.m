%% ----------------------------------------------------------------------------------------------------
%% ******* Matlab script for prototyping real-time visualization application for BPM-80 ************ %%
%% ----------------------------------------------------------------------------------------------------


%% Delete COM port object

delete(s);

%% Create COM port object to communicate with SAM4E Xplained Pro

a = zeros(600,1);
s = serialport('COM4', 115200); %assigns the object s to serial port
peak_info = zeros(6,1);
beam_params = zeros(6,1);

%figure('Name','BPM-80 Peak Analyzer');

for i = 1:25
    
    %% Read the data samples of an individual BPM-80 cycle (for testing purposes)
    
    %   Read in information of signal peaks
    
    while read(s,1,'uint16') ~= 6666
        disp(",");
    end
    for i = 1:6
        peak_info(i) = read(s,1,'uint16');
    end
    
    
    %   Display X crossectional peak
    
    peak_width_1 = peak_info(3)-peak_info(2);
    peak1 = zeros(peak_width_1,1);
    
    while read(s,1,'uint16') ~= 9999
        disp(".");
    end
    
    for i = 1:peak_width_1
        peak1(i) = read(s,1,'uint16');
    end
    
    subplot(2,2,1);
    plot(peak1);
    title('2-dimensional profile of X-peak');
    
    txt = {'Beam position: ',['------------------------'] ,['peak left edge   : ' num2str(peak_info(2))], ...
        ['peak centre       : ' num2str(peak_info(1))], ...
        ['peak right edge : ' num2str(peak_info(3))]};
    text(63,800,txt)
    
    %   Display Y crossectional peak
    
    peak_width_2 = peak_info(6)-peak_info(5);
    peak2 = zeros(peak_width_2,1);
    
    while read(s,1,'uint16') ~= 8888
        disp(";");
    end
    
    for i = 1:peak_width_2
        peak2(i) = read(s,1,'uint16');
    end
    
    subplot(2,2,2);
    plot(peak2);
    title('2-dimensional profile of Y-peak');
    
    txt = {'Beam position: ',['------------------------'] ,['peak left edge   : ' num2str(peak_info(5))], ...
        ['peak centre       : ' num2str(peak_info(4))], ...
        ['peak right edge : ' num2str(peak_info(6))]};
    text(63,800,txt)
    
    
    while read(s,1,'uint16') ~= 7777
        disp(";");
    end
    for i = 1:6
        beam_params(i) = read(s,1,'uint16');
    end
    %% Create surface plot
    
    subplot(2,2,3);
    
    grid = zeros(peak_width_1,peak_width_2);
    [X,Y] = meshgrid(1:1:peak_width_2,1:1:peak_width_1);
    for i = 1:peak_width_1
        for j = 1:peak_width_2
            grid(i,j) = peak1(i)*peak2(j);
        end
    end
    surf(X,Y,grid);
    title('3 dimensional beam intensity surface plot');
    
    
    %% Create beam parameter window
    
    delete(h);
    beam_str = {'COMPUTED BEAM PARAMETERS: ',['********************************************************************'] , ...
        [],...
        ['Beam intensity X       :      ' num2str(beam_params(1))], ...
        ['Beam intensity Y       :      ' num2str(beam_params(2))], ...
        [],...
        ['Beam skewness X       :    '  num2str(beam_params(3))], ...
        ['Beam skewness Y       :    '  num2str(beam_params(4))], ...
        [],...
        ['Beam FWHM X       :      '  num2str(beam_params(5))], ...
        ['Beam FWHM Y       :      '  num2str(beam_params(6))]};
    
    
    
    ax = subplot(2,2,4);
    h = text(0,0.6,beam_str);
    set ( ax, 'visible', 'off')
    
    pause(0.001);
end


skew1 = skewness(peak1);
skew2 = skewness(peak2);

%     for i = 1:6
%         peak(i) = read(s,1,'uint16');
%     end
    
%     subplot(2,2,1:2);
%     p = plot(a);
%     title('Oscilloscope display of collector signal');
%     axis([0 8334 0 2000])
%     
%     drawnow;
%end
% 
% for i = 600:1600
%    if a(i) == 2000
%        window = a(i:i+368);
%    end
% end
% 
% subplot(2,2,3);
% plot(window);


%% Create surface plot
% 
% subplot(2,2,4);
% 
% grid = zeros(100,100);
% [X,Y] = meshgrid(1:1:100,1:1:100);
% for i = 1:100
%     for j = 1:100
%         grid(i,j) = window(50+i)*window(230+j);
%     end
% end
% surf(X,Y,grid);
% title('Beam intensity surface plot');


%% Read BPM-80 beam parameter values
% 
% output = "";
% message = readline(s);
% while isa(message, 'string')
%     message = readline(s);
%     output = strcat(output, message);
% end
% 
% display(output)

%% Delete COM port object

delete(s);