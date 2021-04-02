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

global h;                           % global parameter to store beam parameter update text, deleted at every update

beam_location= zeros(6,1);      % Preallocate parameter array
beam_intensity = 0;
beam_fwhm = zeros(2,1);
beam_skewness = zeros(2,1);


sgtitle({' ','BPM-80 SIGNAL ANALYSIS SYSTEM', ' '}  , 'FontSize', 18);      % Title on application dashboard


    
%% Create and lay out (sub)plots and operator UI on the dashboard for peak visualization and interaction
    
subplot('Position',[0.05,0.5,0.35,0.35]);
title('Oscilloscope display of signal sweep');
      
% User control panel

panel = uipanel('Title','Operator control panel','FontSize',12,'Position',[0.5 0.6 0.4 0.25]);
subpanel1 = uipanel('Parent',panel,'Title','Request plot snapshot','FontSize',10,...
              'Position',[.05 .1 .3 .8]);
subpanel2 = uipanel('Parent',panel,'Title','Trigger characteristics','FontSize',10,...
              'Position',[.4 .1 .5 .8]);     
         
refresh_button = uicontrol(subpanel1, 'String', 'Snapshot update', 'Position',[50 50 100 20]);
refresh_button.Callback = {@request_plot, s};   % Pass pointer to callback routine, and reference to serial port object
    
refresh_button = uicontrol(subpanel1, 'String', 'Parameter update', 'Position',[50 20 100 20]);
refresh_button.Callback = {@request_params, s}; % Pass pointer to callback routine, and reference to serial port object
    
signal_choice  = uicontrol(subpanel1, 'Style', 'popupmenu', 'String', {'Collector signal', 'Fiducial signal'}, 'Position',[50 85 100 20]);
signal_choice.Callback = {@select_signal, s};
    
trigger_level = uicontrol(subpanel2, 'Style', 'edit', 'Position',[80 60 40 20]);
TL_button = uicontrol(subpanel2, 'String', 'Adjust trigger level', 'Position',[50 20 100 20]);
TL_button.Callback = {@adjust_trigger_level,trigger_level, s};
    
trigger_phase_delay = uicontrol(subpanel2, 'Style', 'edit', 'Position',[220 60 40 20]);
TP_button = uicontrol(subpanel2, 'String', 'Adjust trigger delay', 'Position',[190 20 100 20]);
TP_button.Callback = {@adjust_trigger_delay,trigger_phase_delay, s};
   
% Visualization subplots

subplot('Position',[0.5,0.35,0.15,0.2]);
title('Profile of X-peak');
subplot('Position',[0.75,0.35,0.15,0.2]);
title('Profile of Y-peak');
subplot('Position',[0.5,0.05,0.15,0.2]);
title('Beam intensity surface plot');
subplot('Position',[0.75,0.05,0.15,0.2]);
title('Beam intensity contour plot');
  
    
%% Callback function definitions

% Request entire plot data of a single BPM-80 cycle collector/fiducial sweep

function request_plot(~, ~, serialport)
    
    global beam_location;                         % Refer to the global parameter vector
    
    plot_data = zeros(8334,1);                      % Preallocate the plot_data vector containing entire plot cycle
    
    disp('requested plot');
    
    % Send the command to the microcontroller for getting plot data
    
    write(serialport, 255, 'uint8')
    write(serialport, 3, 'uint8');
    write(serialport, 255, 'uint8');
    
    % Read and display the echo message from microcontroller to see if command is
    % correctly received
    
    echo = zeros(3,1);
    for i = 1:3
        echo(i) = read(serialport, 1, 'uint8');
    end
    
    disp(['SAM4E echoed the command for requesting plot :']);
    disp(echo);
    
    % Read the plot data from microcontroller
    
    tic;
            plot_data = read(serialport,8334,'uint16');
    toc; 
    
    % Plot the data as a single sweep
    
    
    
    subplot('Position',[0.05,0.5,0.35,0.35]);
    plot(plot_data);
    title('Oscilloscope display of signal sweep');
    
    
    
    
    % Compute and plot peak1 (X-crossection)
    
    peak_width_1 =  beam_location(3)- beam_location(2);
    peak1 = plot_data(beam_location(2): beam_location(3));
    subplot('Position',[0.5,0.35,0.15,0.2]);
    bar(peak1);
    title('Profile of X-peak');
    
    % Compute and plot peak2 (Y-crossection)
    
    peak_width_2 = beam_location(6)- beam_location(5);
    peak2 = plot_data(beam_location(5): beam_location(6));
    subplot('Position',[0.75,0.35,0.15,0.2]);
    bar(peak2);
    title('Profile of Y-peak');
    
    % Create surface plot 
    
    grid = zeros(peak_width_1,peak_width_2);
    [X,Y] = meshgrid(1:1:peak_width_2,1:1:peak_width_1);
    for i = 1:peak_width_1
        for j = 1:peak_width_2
            grid(i,j) = (peak1(i)/sum(peak1))*(peak2(j)/sum(peak2));
        end
    end
    subplot('Position',[0.5,0.05,0.15,0.2]);
    surf(X,Y,grid, 'edgecolor', 'none');
    title('Beam intensity surface plot');

    % Create contour plot 
    
    grid = zeros(peak_width_1,peak_width_2);
    [X,Y] = meshgrid(1:1:peak_width_2,1:1:peak_width_1);
    for i = 1:peak_width_1
        for j = 1:peak_width_2
            grid(i,j) = (peak1(i)/sum(peak1))*(peak2(j)/sum(peak2));
        end
    end
    
    subplot('Position',[0.75,0.05,0.15,0.2]);
    contour(X,Y,grid);
    title('Beam intensity contour plot');

end

% Request the beam parameter info from the microcontroller

function request_params(~, ~, serialport)
   
    global beam_location;
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
   
    z = read(serialport,1,'uint8')
    beam_location(1:6) = read(serialport,6,'uint16')
    beam_intensity = read(serialport, 1, 'uint32')
    beam_fwhm = read(serialport, 2, 'uint16')
    beam_skewness = read(serialport, 2, 'single')
    z = read(serialport,1,'uint8')
   

    % Create beam parameter window
    
    % Delete the text from previous update
 
    global h;
    if (exist('h', 'var'))
        delete(h);
    end
    
    % Create the new text for this update
    
    beam_str = {'COMPUTED BEAM PARAMETERS ',['********************************************************************'] , ...
        [],...
        ['                                                  X crossection             Y crossection'],...
        [],...
        ['Peak left edge   :                             ' num2str(beam_location(2)) '                           ' num2str(beam_location(5))], ...
        [],...
        ['Peak centre      :                              ' num2str(beam_location(1)) '                           ' num2str(beam_location(4))], ...
        [],...
        ['Peak right edge      :                        ' num2str(beam_location(3)) '                           ' num2str(beam_location(6))], ...
        [],...
        ['Beam intensity        :                       ' num2str(beam_intensity) '                         ' num2str(beam_intensity)], ...
        [],...
        ['Beam FWHM        :                           '  num2str(beam_fwhm(1)) '                                ' num2str(beam_fwhm(2))], ...
        [],...
        ['Beam skewness        :                  '  num2str(beam_skewness(1)) '                      ' num2str(beam_skewness(2))]};
    
    
    % Position and display the updated text
    
    ax = subplot('Position',[0.05,0.1,0.35,0.35]);
    %ax = uipanel('Title','Beam parameters','FontSize',12,'Position',[0.05,0.1,0.35,0.35]);
    %h = uicontrol('Style','text','String',beam_str,'Parent',ax, 'Position',[50 50 100 500]);
    h = text(0,0.4,beam_str, 'FontSize', 10);
    set ( ax, 'visible', 'off')
   
    
end

%  Possibility to select either collector or fiducial signal for display

function select_signal(src, ~, serialport)
    val = src.Value;
    str = src.String;
    disp(['chosen signal : ' str{val}]);
    
%     write(serialport, 255, 'uint8')
%     write(serialport, 4, 'uint8');
%     write(serialport, uint8(val), 'uint8');
    
end

% Adjust the threshold voltage level of the fiducial signal for triggering

function adjust_trigger_level(~,~, extra, serialport)
   
    val = extra.String;
    disp(['Changed trigger level to : ' val]);
    newval = uint8(str2double(val));
    
    write(serialport, 255, 'uint8')
    write(serialport, 1, 'uint8');
    write(serialport, newval, 'uint8');
    
    echo = zeros(3,1);
    for i = 1:3
        echo(i) = read(serialport, 1, 'uint8');
    end
    
    
    disp(['SAM4E echoed the command for adjusting trigger level :']);
    disp(echo);
end

% Adjust the time delay between fiducial peak and actual trigger

function adjust_trigger_delay(~,~, extra, serialport)
    val = extra.String;
    disp(['Changed trigger phase offset to : ' val]);
    newval = uint8(str2double(val));
    
    write(serialport, 255, 'uint8')
    write(serialport, 0, 'uint8');
    write(serialport, newval, 'uint8');
    
    echo = zeros(3,1);
    for i = 1:3
        echo(i) = read(serialport, 1, 'uint8');
    end
    
    disp(['SAM4E echoed the command for adjusting trigger phase offset : ']);
    disp(echo);
end