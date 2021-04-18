x = [-3:.1:3];
y = normpdf(x,0,1);
plot(y);
axis([0,65,-0.1,0.6]);
a = normpdf(x,0,2);
plot(a);
axis([0,65,-0.1,0.3]);

for i = 1:61
    for j = 1:61
        z(i,j) = y(i)*a(j);
    end
end

%surf(z, 'edgecolor', 'none');