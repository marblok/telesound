function test_IIR
%ELLIPORD, BUTTORD, CHEB1ORD, CHEB2ORD
% ELLIP, CHEBY1, CHEBY2, BUTTER, BESSELF, 
SamplingRate = 22050;

% Fp = [300, 3400]; Fs = [100, 3600];
% Rp = 0.2; Rs = 70;

Fp = [300, 3400]; Fs = [200, 3500];
Rp = 1; Rs = 70;

[N, Wn] = ellipord(2*Fp(1)/SamplingRate, 2*Fs(1)/SamplingRate, Rp, Rs)
% [N, Wn] = cheb1ord(2*Fp/SamplingRate, 2*Fs/SamplingRate, Rp, Rs)
% [N, Wn] = cheb2ord(2*Fp/SamplingRate, 2*Fs/SamplingRate, Rp, Rs)
% [N, Wn] = buttord(2*Fp/SamplingRate, 2*Fs/SamplingRate, Rp, Rs)

[B,A] = ELLIP(N,Rp,Rs,Wn, 'high') 
% [B,A] = cheby1(N,Rp,Wn) 
% [B,A] = cheby2(N,Rs,Wn) 
% [B,A] = butter(N,Wn) 
figure(1)
freqz(B, A, 2048, SamplingRate);


[N, Wn] = ellipord(2*Fp(2)/SamplingRate, 2*Fs(2)/SamplingRate, Rp, Rs)
[B,A] = ELLIP(N,Rp,Rs,Wn) 

figure(2)
freqz(B, A, 2048, SamplingRate);