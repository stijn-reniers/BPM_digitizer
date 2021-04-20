% Off-line measurements analysis

%load('Measurements2.mat');
load('test_signal_analysis.mat');
load('test_signal_analysis_2.mat');
%Plotting the time spectra

% first_spectrum = fwhm_data(1:8334);
% area(1835:2089,first_spectrum(1835:2089), 'FaceColor', 'Red', 'Edgecolor', 'none');
% text(2090, 2000, 'IntX', 'Color', 'Red');
% hold on
% area(5937:6161,first_spectrum(5937:6161), 'FaceColor', 'Blue');
% text(6200, 2000, 'IntY', 'Color', 'Blue')
% hold on
% plot(first_spectrum, 'Color', 'Black');
% hold off
% 
% figure
% second_spectrum = fwhm_data(8335:16668);            % 9607-10755 and 14407-14787
% %plot(second_spectrum);
% % figure;
% third_spectrum = fwhm_data(16669:25002);
% %plot(third_spectrum);
% %figure;
% fourth_spectrum = (fwhm_data(25003:33336));
% %plot(fourth_spectrum);
% %figure;
% fifth_spectrum = (fwhm_data(33337:41670));
% %plot(fifth_spectrum);
% % figure;
% sixth_spectrum = (fwhm_data(41671:50004));
% %plot(sixth_spectrum);
% % figure;
% seventh_spectrum =(fwhm_data(50005:58338));
% %plot(seventh_spectrum);
% % figure;
% eighth_spectrum = (fwhm_data(58339:66672));
% %plot(eighth_spectrum);
% % figure;
% ninth_spectrum = (fwhm_data(66672:75006));
% %plot(ninth_spectrum);

% First peak starts a 1835, ends at 2089

peak = fwhm_data_2;
plot(peak);
figure;

peak = fwhm_data_2(1016:1098);
plot(peak);
figure;

 data = [];
 for i = 1016:1098
    data = vertcat(data, i*ones(fwhm_data_2(i),1)); 
 end
 
skewnessX = skewness(data)
% 
peak = fwhm_data_2(5301:5383);
plot(peak);
 
data = [];
for i = 5301:5383
data = vertcat(data, i*ones(fwhm_data_2(i),1)); 
end
 
skewnessY = skewness(data)


% %% Second spectrum
% 
% peak = fwhm_data(5950:6160);
% %plot(peak);
% 
% data = [];
% for i = 5950:6160
%    data = vertcat(data, i*ones(fwhm_data(i),1)); 
% end
% 
% skewness(data)
% 
% % peak = second_spectrum(1273:2421);
% % 
% % data = [];
% % for i = 1273:2421
% %    data = vertcat(data, i*ones(second_spectrum(i),1)); 
% % end
% % 
% % skewness(data)
% 
% %% Third spectrum
% 
% % peak = third_spectrum(1573:2270);
% % plot(peak);
% % 
% % data = [];
% % for i = 1573:2270
% %    data = vertcat(data, i*ones(third_spectrum(i),1)); 
% % end
% % 
% % skewness(data)
% % 
% % 
% % peak = third_spectrum(6013:6286);
% % plot(peak);
% % 
% % data = [];
% % for i = 6013:6286
% %    data = vertcat(data, i*ones(third_spectrum(i),1)); 
% % end
% % 
% % skewness(data)
% 
% %% Fourth spectrum
% 
% % peak = fourth_spectrum(1574:2280);
% % plot(peak);
% % 
% % data = [];
% % for i = 1574:2280
% %    data = vertcat(data, i*ones(third_spectrum(i),1)); 
% % end
% % 
% % skewness(data)
% % 
% % 
% % peak = third_spectrum(6024:6286);
% % plot(peak);
% % 
% % data = [];
% % for i = 6013:6286
% %    data = vertcat(data, i*ones(third_spectrum(i),1)); 
% % end
% % 
% % skewness(data)
% 
% %% Fifth spectrum
% 
% % peak = fifth_spectrum(1881:2161);
% % plot(peak);
% % 
% % data = [];
% % for i = 1881:2161
% %    data = vertcat(data, i*ones(fifth_spectrum(i),1)); 
% % end
% % 
% % skewness(data)
% % 
% % 
% % peak = fifth_spectrum(5957:6140);
% % %plot(peak);
% % 
% % data = [];
% % for i = 5957:6140
% %    data = vertcat(data, i*ones(fifth_spectrum(i),1)); 
% % end
% % 
% % skewness(data)
% 
% %% Sixth spectrum
% 
% 
% % peak = sixth_spectrum(1892:2135);
% % plot(peak);
% % 
% % data = [];
% % for i = 1892:2135
% %    data = vertcat(data, i*ones(sixth_spectrum(i),1)); 
% % end
% % 
% % skewness(data)
% % 
% % 
% % peak = fifth_spectrum(5969:6137);
% % %plot(peak);
% % 
% % data = [];
% % for i = 5969:6137
% %    data = vertcat(data, i*ones(sixth_spectrum(i),1)); 
% % end
% % 
% % skewness(data)
% 
% %% Seventh spectrum
% % 
% % 
% % peak = seventh_spectrum(1856:2173);
% % plot(peak);
% % 
% % data = [];
% % for i = 1856:2173
% %    data = vertcat(data, i*ones(seventh_spectrum(i),1)); 
% % end
% % 
% % skewness(data)
% % 
% % 
% % peak = seventh_spectrum(5963:6148);
% % plot(peak);
% % 
% % data = [];
% % for i = 5963:6148
% %    data = vertcat(data, i*ones(seventh_spectrum(i),1)); 
% % end
% % 
% % skewness(data)
% 
% 
% %% Eighth spectrum
% 
% 
% % peak = eighth_spectrum(1750:2080);
% % plot(peak);
% % 
% % data = [];
% % for i = 1750:2080
% %    data = vertcat(data, i*ones(eighth_spectrum(i),1)); 
% % end
% % 
% % skewness(data)
% % 
% % 
% % peak = eighth_spectrum(6009:6197);
% % %plot(peak);
% % 
% % data = [];
% % for i = 6009:6197
% %    data = vertcat(data, i*ones(eighth_spectrum(i),1)); 
% % end
% % 
% % skewness(data)
% 
% 
% %% Ninth spectrum
% 
% % 
% % peak = ninth_spectrum(1932:2122);
% % plot(peak);
% % 
% % data = [];
% % for i = 1932:2122
% %    data = vertcat(data, i*ones(ninth_spectrum(i),1)); 
% % end
% % 
% % skewness(data)
% % 
% % 
% % peak = ninth_spectrum(5932:6088);
% % plot(peak);
% % 
% % data = [];
% % for i = 5932:6088
% %    data = vertcat(data, i*ones(ninth_spectrum(i),1)); 
% % end
% % 
% % skewness(data)