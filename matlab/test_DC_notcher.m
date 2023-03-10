function test_DC_notcher
% DC - blocker / DC-notch filter
a = [1, -0.99];
b = [1, -1];
K = (a(1)-a(2))/(b(1)-b(2));

figure(1)
freqz(K*b,a, 2048, 22050);