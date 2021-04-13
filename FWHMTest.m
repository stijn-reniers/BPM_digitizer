load('Measurements2.mat')
a=1;
b=4167;
results=[]
for i=1:18
    results= [results, fwhm(a:b,fwhm_data(a:b))];
    a=a+4167;
    b=b+4167;
end