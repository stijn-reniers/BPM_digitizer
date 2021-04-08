function [FWHM1,FWHM2] = fwhm2(peak1Data,peak2Data)
%FWHM2 Summary of this function goes here
%   Detailed explanation goes here
FWHM1=0;
FWHM2=0;
[max1,I1]=max(peak1Data);
[max2,I2]=max(peak2Data);

%peak 1 FWHM
value1=max1;
Value1Index=I1;
value2=max1;
Value2Index=I1;

for i= 1:I1
    if(abs(peak1Data(i)-(max1/2))<abs(value1-(max1/2)))
        value1= peak1Data(i);
        Value1Index= i;
    end
end
for i= I1:size(peak1Data,1)
    if(abs(peak1Data(i)-(max1/2))<abs(value2-(max1/2)))
        value2= peak1Data(i);
        Value2Index= i;
    end
end
FWHM1= Value2Index- Value1Index;

%peak 2 FWHM
value1=max2;
Value1Index=I2;
value2=max2;
Value2Index=I2;
for i= 1:I1
    if(abs(peak2Data(i)-(max2/2))<abs(value1-(max2/2)))
        value1= peak1Data(i);
        Value1Index= i;
    end
end
for i= I1:size(peak2Data,1)
    if(abs(peak2Data(i)-(max2/2))<abs(value2-(max2/2)))
        value2= peak1Data(i);
        Value2Index= i;
    end
end
FWHM2= Value2Index- Value1Index;


end

