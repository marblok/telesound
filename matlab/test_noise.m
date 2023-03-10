function test_noise

N1 = 200;
N2 = 201;
N3 = 202;

% x1 = rand(N1, 1); x2 = rand(N2, 1); x3 = rand(N3, 1);
x1 = rand(N1, 1)-0.5; x2 = rand(N2, 1)-0.5; x3 = rand(N3, 1)-0.5;
% x1 = randn(N1, 1); x2 = randn(N2, 1); x3 = randn(N3, 1);

y = [x1; x1; x1; x1];
y2 = [x2; x2; x2; x2];
y3 = [x3; x3; x3; x3];
% y2 = y .* y2(1:length(y)) .* y3(1:length(y));
y2 = y .* y2(1:length(y));
% y2 = y + y2(1:length(y)) + y3(1:length(y));

figure(1)
subplot(4,1,1)
plot(y, '.')
subplot(4,1,2)
hist(y, 31)
subplot(4,1,3)
plot(xcorr(y, y))
subplot(4,1,4)
plot(abs(fft(y, 1024)))

figure(2)
subplot(4,1,1)
plot(y2, '.')
subplot(4,1,2)
hist(y2, 31)
subplot(4,1,3)
plot(xcorr(y2, y2))
subplot(4,1,4)
plot(abs(fft(y2, 1024)))
