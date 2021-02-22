%% ----------------------------------------------------------------------------------------------------
%% ******* Matlab script for prototyping real-time visualization application for BPM-80 ************ %%
%% ----------------------------------------------------------------------------------------------------


%% Create COM port object to communicate with SAM4E Xplained Pro

a = zeros(600,1);
s = serialport('COM5', 115200); %assigns the object s to serial port
peak_info = zeros(6,1);
beam_params = zeros(6,1);

%fig = figure;
sgtitle({' ','BPM-80 SIGNAL ANALYSIS SYSTEM', ' '}  , 'FontSize', 18);

%for packet = 1:30
    
    %% Read the data samples of an individual BPM-80 cycle (for testing purposes)
    
    subplot('Position',[0.05,0.5,0.35,0.35]);
        
    panel = uipanel('Title','Operator control panel','FontSize',12,'Position',[0.5 0.6 0.4 0.25]);
    
    subpanel1 = uipanel('Parent',panel,'Title','Request plot snapshot','FontSize',10,...
              'Position',[.05 .1 .3 .8]);
    subpanel2 = uipanel('Parent',panel,'Title','Trigger characteristics','FontSize',10,...
              'Position',[.4 .1 .5 .8]);     
         
    refresh_button = uicontrol(subpanel1, 'String', 'Snapshot update', 'Position',[20 20 100 20]);
    refresh_button.Callback = @request_plot;
    
    signal_choice  = uicontrol(subpanel1, 'Style', 'popupmenu', 'String', {'Collector signal', 'Fiducial signal'}, 'Position',[20 55 100 20]);
    signal_choice.Callback = @select_signal;
    
    trigger_level = uicontrol(subpanel2, 'Style', 'edit', 'Position',[80 60 40 20]);
    TL_button = uicontrol(subpanel2, 'String', 'Adjust trigger level', 'Position',[50 20 100 20]);
    TL_button.Callback = {@adjust_trigger_level,trigger_level};
    
    trigger_phase_delay = uicontrol(subpanel2, 'Style', 'edit', 'Position',[220 60 40 20]);
    TP_button = uicontrol(subpanel2, 'String', 'Adjust trigger level', 'Position',[190 20 100 20]);
    TP_button.Callback = {@adjust_trigger_delay,trigger_phase_delay};
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
    
    subplot('Position',[0.5,0.35,0.15,0.2]);
    plot(peak1);
    title('2-dimensional profile of X-peak');
    
    %   Display Y crossectional peak
    
    peak_width_2 = peak_info(6)-peak_info(5);
    peak2 = zeros(peak_width_2,1);
    
    while read(s,1,'uint16') ~= 8888
        disp(";");
    end
    
    for i = 1:peak_width_2
        peak2(i) = read(s,1,'uint16');
    end
    
    subplot('Position',[0.75,0.35,0.15,0.2]);
    plot(peak2);
    title('2-dimensional profile of Y-peak');
    
     
    
    while read(s,1,'uint16') ~= 7777
        disp(";");
    end
    for i = 1:4
        beam_params(i) = read(s,1,'uint16');
    end
    for i = 1:2
        beam_params(4+i) = read(s,1,'double');
    end
    
    %% Create surface and contour plot
    
    subplot('Position',[0.5,0.05,0.15,0.2]);
    
    grid = zeros(peak_width_1,peak_width_2);
    [X,Y] = meshgrid(1:1:peak_width_2,1:1:peak_width_1);
    for i = 1:peak_width_1
        for j = 1:peak_width_2
            grid(i,j) = peak1(i)*peak2(j);
        end
    end
    surface = surf(X,Y,grid);
    
   % direction = [0 0 1];
   % rotate(surface,direction,10*packet)
    
    title('3 dimensional beam intensity surface plot');
    
    subplot('Position',[0.75,0.05,0.15,0.2]);
    
    grid = zeros(peak_width_1,peak_width_2);
    [X,Y] = meshgrid(1:1:peak_width_2,1:1:peak_width_1);
    for i = 1:peak_width_1
        for j = 1:peak_width_2
            grid(i,j) = peak1(i)*peak2(j);
        end
    end
    cont = contour(X,Y,grid);
    
    title('beam intensity contour plot');
    
    
    %% Create beam parameter window
    
    delete(h);
    beam_str = {'COMPUTED BEAM PARAMETERS ',['********************************************************************'] , ...
        [],...
        ['                                                  X crossection             Y crossection'],...
        [],...
        ['Peak left edge   :                             ' num2str(peak_info(2)) '                           ' num2str(peak_info(5))], ...
        [],...
        ['Peak centre      :                              ' num2str(peak_info(1)) '                           ' num2str(peak_info(4))], ...
        [],...
        ['Peak right edge      :                        ' num2str(peak_info(3)) '                           ' num2str(peak_info(6))], ...
        [],...
        ['Beam intensity        :                       ' num2str(beam_params(1)) '                         ' num2str(beam_params(2))], ...
        [],...
        ['Beam FWHM        :                           '  num2str(beam_params(3)) '                                ' num2str(beam_params(4))], ...
        [],...
        ['Beam skewness        :                  '  num2str(beam_params(5)) '                      ' num2str(beam_params(6))]};
    
    
    
    ax = subplot('Position',[0.05,0.1,0.35,0.35]);
    h = text(0,0.4,beam_str, 'FontSize', 10);
    set ( ax, 'visible', 'off')
    
    pause(0.001)
    
      
    
%end

mean1 = 0 
sum = 0;
for i = 1:peak_width_1
    mean1 = mean1 + peak1(i)*(peak_info(2)+i-1);
    sum = sum + peak1(i);
end

mean1 = mean1/sum;

variance = 0;
skewness = 0;

for i = 1:peak_width_1
    variance = variance + (mean1 - (peak_info(2) + i-1))^2*peak1(i);
    skewness = skewness + (((peak_info(2) + i-1)-mean1)^3)*peak1(i);
end

    variance = variance/sum;
    skewness = skewness/(sqrt(variance)^3)/sum;
    


mean2 = 0 
sum = 0;
for i = 1:peak_width_2
    mean2 = mean2 + peak2(i)*(peak_info(5)+i-1);
    sum = sum + peak2(i);
end

mean2 = mean2/sum;

variance = 0;
skewness2 = 0;

for i = 1:peak_width_2
    variance = variance + (mean2 - (peak_info(5) + i-1))^2*peak2(i);
    skewness2 = skewness2 + (((peak_info(5) + i-1)-mean2)^3)*peak2(i);
end

    variance = variance/sum;
    skewness2 = skewness2/(sqrt(variance)^3)/sum;
    


%% Delete COM port object

delete(s);


%% Function definitions

function request_plot(~, ~)
    disp('requested plot');
end

function select_signal(src, ~)
    val = src.Value;
    str = src.String;
    disp(['chosen signal : ' str{val}]);
end

function adjust_trigger_level(~,~, extra)
    val = extra.String;
    disp(['Changed trigger level to : ' val]);
end

function adjust_trigger_delay(src,event, extra)
    val = extra.String;
    disp(['Changed trigger phase offset to : ' val]);
end