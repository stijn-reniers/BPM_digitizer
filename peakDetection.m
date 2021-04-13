function [FWHM1,FWHM2] = peakDetection(fwhm_data,a,b)

[maximum,maxIndex]= max(fwhm_data(a:b))
[p_start,p_end]=detectPeak(fwhm_data,maxIndex)
peak1Data= fwhm_data(a+p_start-1:a+p_end-1);
a=a+4167;
b=b+4167;
[maximum,maxIndex]= max(fwhm_data(a:b))
[p_start,p_end]=detectPeak(fwhm_data,maxIndex)
peak2Data= fwhm_data(a+p_start-1:a+p_end-1);
[FWHM1,FWHM2] = fwhm2(peak1Data,peak2Data)
end

