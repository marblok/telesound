function test_PSD

x = wavread('test.wav');
n = 0:length(x)-1;
x = 2*cos(2*pi/4*n).';

N = 512;
K = 1024;
figure(1)
w = blackman(N);
% w = boxcar(N);
% skala = 1/sum(w.^2); % PSD : gestosc mocy szumu na odpowiednim poziomie
skala = 2/(sum(w).^2); % sin : poziom prazków skladowych sygnalu na pozionie odpowiadajacym ich amplitudzie
% uwaga skladowa stala bedzie podwojeona

w = w * sqrt(skala); skala = 1;
for ind = 1:10,
  x_ = x((ind-1)*N+[1:N]);
%   x_ = x_ + randn(size(x_));
  
  X = fft(x_.*w, K);
  X = X(1:K/2+1);
  W = fft(w, K);
  W = W(1:K/2+1);
  
  X =skala*(abs(X).^2);
  
  subplot(2,1,1);
  plot(X);
  subplot(2,1,2);
  plot(10*log10(X));
  hold on
  plot(20*log10(abs(W)), 'r');
  hold off
  pause
end