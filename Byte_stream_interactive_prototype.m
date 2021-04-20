%% ----------------------------------------------------------------------------------------------------
%% ******* Matlab script for prototyping real-time visualization application for BPM-80 ************ %%
%% ----------------------------------------------------------------------------------------------------

%% Delete COM port object

if exist('s', 'var')
delete(s);
end 

if exist('save_data', 'var')
delete(s);
end 


%% Create COM port object to communicate with SAM4E Xplained Pro
s = serialport('COM5', 115200);     % assigns object s to serial port

global beam_location;             % global parameter, since it is also used in the plotting routines

global beam_location_temp;
global beam_intensity_temp;
global beam_fwhm_temp;
global beam_skewness_temp;
global fwhm_data_2;

global h;                           % global parameter to store beam parameter update text, deleted at every update

beam_location= zeros(6,1);      % Preallocate parameter array
beam_intensity = 0;
beam_fwhm = zeros(2,1);
beam_skewness = zeros(2,1);

beam_location_temp= NaN(24,2);      % Preallocate parameter array
beam_intensity_temp = NaN(24,1);
beam_fwhm_temp = NaN(24,2);
beam_skewness_temp = NaN(24,2);

global circular_index;
circular_index = 0;

global instant_vis;
instant_vis = figure('Name', 'Instantaneous visualization');
sgtitle({' ','BPM-80 SIGNAL ANALYSIS SYSTEM', ' '}  , 'FontSize', 18);      % Title on application dashboard

global temporal_vis;
temporal_vis = figure('Name', 'Temporal evolution visualization');
sgtitle({' ','BPM-80 SIGNAL ANALYSIS SYSTEM', ' '}  , 'FontSize', 18);      % Title on application dashboard

    
%% Create and lay out (sub)plots and operator UI on the dashboard for peak visualization and interaction
set(0,'CurrentFigure',instant_vis)

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
    
param_refresh_button = uicontrol(subpanel1, 'String', 'Parameter update', 'Position',[50 20 100 20]);
param_refresh_button.Callback = {@request_params, s}; % Pass pointer to callback routine, and reference to serial port object
    
signal_choice  = uicontrol(subpanel1, 'Style', 'popupmenu', 'String', {'Collector signal', 'Fiducial signal'}, 'Position',[50 85 100 20]);
signal_choice.Callback = {@select_signal, s};
    
trigger_level = uicontrol(subpanel2, 'Style', 'edit', 'Position',[80 60 40 20]);
TL_button = uicontrol(subpanel2, 'String', 'Adjust DC-offset', 'Position',[50 20 100 20]);
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
  

%% Create and lay out (sub)plots and operator UI on the dashboard for temporal evolution plots

set(0,'CurrentFigure',temporal_vis)

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

% Request entire plot data of a single BPM-80 cycle collector/fiducial sweep

function request_plot(~, ~, serialport)

    global fwhm_data_2;
    global beam_location;                         % Refer to the global parameter vector
    global instant_vis;
    set(0,'CurrentFigure',instant_vis);
    
    plot_data = zeros(8334,1);                      % Preallocate the plot_data vector containing entire plot cycle
    
    disp('requested plot');
    
    % Send the command to the microcontroller for getting plot data
    
    write(serialport, 255, 'uint8')
    write(serialport, 3, 'uint8');
    write(serialport, 255, 'uint8');
    
    % Read and display the echo message from microcontroller to see if command is
    % correctly received
    
%     echo = zeros(3,1);
%     for i = 1:3
%         echo(i) = read(serialport, 1, 'uint8');
%     end
    
%     disp(['SAM4E echoed the command for requesting plot :']);
%     disp(echo);
%     
    % Read the plot data from microcontroller
    
    tic;
            plot_data = read(serialport,8334,'uint16');
    toc; 
    
    fwhm_data_2 = plot_data;
   
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
    global circular_index;
    global temporal_vis;
    global instant_vis;
    
    global beam_location_temp;
    global beam_intensity_temp;
    global beam_fwhm_temp;
    global beam_skewness_temp;
    
    disp('requested parameters');
    
    % Send the command to the microcontroller for getting parameter data
    
    write(serialport, 255, 'uint8')
    write(serialport, 2, 'uint8');
    write(serialport, 255, 'uint8');
    
    % Read and display the echo message from microcontroller to see if command is
    % correctly received
    
%     echo = zeros(3,1);
%     for i = 1:3
%         echo(i) = read(serialport, 1, 'uint8');
%     end
%     
%     
%     disp(['SAM4E echoed the command for requesting parameters :']);
%     disp(echo);
%     
        
    % Read the beam parameter plot data
   
    z = read(serialport,1,'uint8')
    beam_location(1:6) = read(serialport,6,'uint16')
    beam_intensity = read(serialport, 1, 'uint32')
    beam_fwhm = read(serialport, 2, 'uint16')
    beam_skewness = read(serialport, 2, 'int32')/10000
    beam_location_variance = read(serialport, 2, 'int32')/10000
    z = read(serialport,1,'uint8')
   

    % Create beam parameter window
    
    % Delete the text from previous update
 
    global h;
    if (exist('h', 'var'))
        delete(h);
    end
    
    % Create the new text for this update
    set(0,'CurrentFigure',instant_vis)
    
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
        ['Peak position variance      :            ' num2str(beam_location_variance(1)) '                        ' num2str(beam_location_variance(2))], ...
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
    
    
    % Temporal visualization
    set(0,'CurrentFigure',temporal_vis)
    
    if circular_index < 24
    circular_index = circular_index+1;
    end 
    
       
    beam_intensity_temp(circular_index) = beam_intensity;
    beam_fwhm_temp(circular_index,:) = beam_fwhm;
    beam_skewness_temp(circular_index,:) = beam_skewness;
    beam_location_temp(circular_index,1) = beam_location(1);
    beam_location_temp(circular_index,2) = beam_location(4);
    
         
    
    % Create beam parameter window
    
    subplot('Position',[0.05,0.50,0.4,0.35]);
    plot(beam_location_temp(:,1), '-ok', 'Color', 'blue');
    title('Beam position');
    axis([0 25 0 8334]);
    hold on;
    plot(beam_location_temp(:,2), '-ok', 'Color', 'red');
    legend('X peak', 'Y peak');
    hold off;
        
    subplot('Position',[0.55,0.50,0.4,0.35]);
    plot(beam_intensity_temp(:,1), '-ok', 'Color', 'black');
    title('Beam intensity');
    axis([0 25 0 100000]);
    
    subplot('Position',[0.05,0.05,0.4,0.35]);
    plot(beam_fwhm_temp(:,1), '-ok', 'Color', 'blue');
    title('Beam FWHM');
    axis([0 25 0 500]);
    hold on;
    plot(beam_fwhm_temp(:,2), '-ok', 'Color', 'red');
    legend('X peak', 'Y peak');
    hold off;
    
    subplot('Position',[0.55,0.05,0.4,0.35]);
    plot(beam_skewness_temp(:,1), '-ok', 'Color', 'blue');
    title('Beam skewness');
    axis([0 25 -3 3]);
    hold on;
    plot(beam_skewness_temp(:,2), '-ok', 'Color', 'red');
    legend('X peak', 'Y peak');
    hold off;
    
    if circular_index == 24
        beam_location_temp = circshift(beam_location_temp,-1,1);
        beam_intensity_temp =circshift(beam_intensity_temp,-1,1);
        beam_fwhm_temp = circshift(beam_fwhm_temp,-1,1);
        beam_skewness_temp = circshift(beam_skewness_temp,-1,1);
    end
   
    
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