%% ----------------------------------------------------------------------------------------------------
%% ******* Matlab script for prototyping real-time visualization application for BPM-80 ************ %%
%% ----------------------------------------------------------------------------------------------------

%% Delete COM port object

if exist('s', 'var')
delete(s);
end 


%% Create COM port object to communicate with SAM4E Xplained Pro
s = serialport('COM5', 115200);     % assigns object s to serial port

global beam_location;             % global parameter, since it is also used in the plotting routines
global beam_intensity;
global beam_fwhm;
global beam_skewness;
global circular_index;

beam_location= NaN(24,2);      % Preallocate parameter array
beam_intensity = NaN(24,1);
beam_fwhm = NaN(24,2);
beam_skewness = NaN(24,2);

circular_index = 0;


sgtitle({' ','BPM-80 SIGNAL ANALYSIS SYSTEM', ' '}  , 'FontSize', 18);      % Title on application dashboard


    
%% Create and lay out (sub)plots and operator UI on the dashboard for peak visualization and interaction
% User control panel

refresh_button = uicontrol('String', 'Parameter update', 'Position',[720 350 100 50]);
refresh_button.Callback = {@request_params, s};   % Pass pointer to callback routine, and reference to serial port object
    

% Visualization subplots

subplot('Position',[0.05,0.50,0.4,0.35]);
title('Beam position');

subplot('Position',[0.55,0.50,0.4,0.35]);
title('Beam intensity');

subplot('Position',[0.05,0.05,0.4,0.35]);
title('Beam FWHM');

subplot('Position',[0.55,0.05,0.4,0.35]);
title('Beam skewness');
  
    
%% Callback function definitions


% Request the beam parameter info from the microcontroller

function request_params(~, ~, serialport)
   
    global beam_location;
    global beam_intensity;
    global beam_fwhm;
    global beam_skewness;
    
    global circular_index;
    
    peak_locations = zeros(6,1);
    
    disp('requested parameters');
    
    % Send the command to the microcontroller for getting plot data
    
    write(serialport, 255, 'uint8')
    write(serialport, 2, 'uint8');
    write(serialport, 255, 'uint8');
    
    % Read and display the echo message from microcontroller to see if command is
    % correctly received
    
    echo = zeros(3,1);
    for i = 1:3
        echo(i) = read(serialport, 1, 'uint8');
    end
    
    
    disp(['SAM4E echoed the command for requesting parameters :']);
    disp(echo);
    
    
    
    % Read the beam parameter plot data
   
    if circular_index < 24
    circular_index = circular_index+1;
    end 
    
    
    z = read(serialport,1,'uint8')
    peak_locations(1:6) = read(serialport,6,'uint16')
    beam_intensity(circular_index) = read(serialport, 1, 'uint32')
    beam_fwhm(circular_index,:) = read(serialport, 2, 'uint16')
    beam_skewness(circular_index,:) = read(serialport, 2, 'single')
    z = read(serialport,1,'uint8')
    beam_location(circular_index,1) = peak_locations(1);
    beam_location(circular_index,2) = peak_locations(4);
    
   
  
    
    
    % Create beam parameter window
    
    subplot('Position',[0.05,0.50,0.4,0.35]);
    plot(beam_location(:,1), '-ok', 'Color', 'blue');
    title('Beam position');
    axis([0 25 0 8334]);
    hold on;
    plot(beam_location(:,2), '-ok', 'Color', 'red');
    legend('X peak', 'Y peak');
    hold off;
        
    subplot('Position',[0.55,0.50,0.4,0.35]);
    plot(beam_intensity, '-ok');
    title('Beam intensity');
    axis([0 25 0 100000]);
    
    subplot('Position',[0.05,0.05,0.4,0.35]);
    plot(beam_fwhm(:,1), '-ok', 'Color', 'blue');
    title('Beam FWHM');
    axis([0 25 0 500]);
    hold on;
    plot(beam_fwhm(:,2), '-ok', 'Color', 'red');
    legend('X peak', 'Y peak');
    hold off;
    
    subplot('Position',[0.55,0.05,0.4,0.35]);
    plot(beam_skewness(:,1), '-ok', 'Color', 'blue');
    title('Beam skewness');
    axis([0 25 -3 3]);
    hold on;
    plot(beam_skewness(:,2), '-ok', 'Color', 'red');
    legend('X peak', 'Y peak');
    hold off;
    
    if circular_index == 24
        beam_location = circshift(beam_location,-1,1);
        beam_intensity =circshift(beam_intensity,-1,1);
        beam_fwhm = circshift(beam_fwhm,-1,1);
        beam_skewness = circshift(beam_skewness,-1,1);
    end
    
end
