%    Matlab skewness routine
%-----------------------------------------------------
function compute_skewness(start, ending, buffer)

mean1 = 0;
sums = 0;
for i = start:ending
    mean1 = mean1 + buffer(i)*i;
    sums = sums + buffer(i);
end

mean1 = mean1/sums;

variance = 0;
skewness = 0;

for i = start:ending
    variance = variance + (mean1 - i)^2*buffer(i);
    skewness = skewness + ((i-mean1)^3)*buffer(i);
end

    variance = variance/sums;
    skewness = skewness/(sqrt(variance)^3)/sums;
    disp(skewness);
end