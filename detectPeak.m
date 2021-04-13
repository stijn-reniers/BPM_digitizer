function [peakStart,peakEnd] = detectPeak(values,maxIndex)
%DETECTPEAK finds the edges of a peak through its maximum
%   Detailed explanation goes here
peakStart=-1;
peakEnd=-1;
i=maxIndex;
    while peakStart <0
        if i<2 
            peakStart=1;
        else
            if (values(i)<= 20)
                peakStart= i;
            end
            i=i-1;
        end
    end
    i=maxIndex;
    while peakEnd <0
        if i>size(values,2)-1 
            peakEnd=size(values,2);
        else
            if (values(i)<= 20)
                peakEnd= i;
            end
            i=i+1;
        end
    end
end

