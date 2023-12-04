function test_IIR
%ELLIPORD, BUTTORD, CHEB1ORD, CHEB2ORD
% ELLIP, CHEBY1, CHEBY2, BUTTER, BESSELF, 

close all
do_not_save = 0;
design_LPF = 1;

% 1. Za�o�enie dF = 100 Hz; Rp = 1; Rs = 70
% 2. dobra� metod� aproksymacji N w funkji Wn dla r�nych szybko�ci pr�bkowania
% 3. projektowa� filtry
% 3a. bezpo�rednio jak w ellip
% 3b. metody aproksymacji (dla sta�ego minimalnego N)
% ? problem co poni�ej dF ?

% ELLIPAP Elliptic analog lowpass filter prototype.
% !!! dla ustalonego N, Rp i Rs tylko jeden filter
% !!! to samo z reprezentacj� space-state
% !!! spor� cz�� transformacji lp2lp oraz lp2hp te� mo�na zrobi� za wczasu
% pozostaje:
%  - transformacja biliniowa
%  - state-space version of bilinear transformation

% !!!! ew. dla ustalonego N, Rp i Rs wyznaczy� struktur� Farrowa

% \todo okre�li� Fp_min (mamy Fp_max)
% \todo podobnie dla innych rz�d�w filtr�w: okre�li� Fp_min i Fp_max
if design_LPF == 1,
  % Fa_max - g�rna cz�stotliwo�� dziedziny aproksymacji
%   SamplingRate = 8000; N = 5; order = 15; Fa_max = 2000; Fmax = 3400; Fmin = 200;
% 	SamplingRate = 11025; N = 5; order = 15; Fa_max = 2500; Fmax = 4600; Fmin = 200;
% 	SamplingRate = 16000; N = 5; order = 15; Fa_max = 4000; Fmax = 7000; Fmin = 300;
% 	SamplingRate = 22050; N = 4; order = 15; Fa_max = 5500; Fmax = 9600; Fmin = 100;
% 	SamplingRate = 32000; N = 4; order = 15; Fa_max = 8000; Fmax = 14000; Fmin = 200;
% 	SamplingRate = 44100; N = 4; order = 15; Fa_max = 11000; Fmax = 19000; Fmin = 200;
	SamplingRate = 48000; N = 4; order = 15; Fa_max = 12000; Fmax = 21000; Fmin = 300;
%   Fmin = 100;
else
% 	SamplingRate = 8000; N = 5; order = 15; Fa_max = 3400; Fmin = 80; Fmax = 3400;
% 	SamplingRate = 11025; N = 5; order = 15; Fa_max = 4500; Fmin = 80; Fmax = 4600;
% 	SamplingRate = 16000; N = 4; order = 15; Fa_max = 6000; Fmin = 80; Fmax = 6600;
% 	SamplingRate = 22050; N = 4; order = 15; Fa_max = 8000; Fmin = 80; Fmax = 8900;
% 	SamplingRate = 32000; N = 4; order = 15; Fa_max = 12000; Fmin = 100; Fmax = 13000;
% 	SamplingRate = 44100; N = 4; order = 15; Fa_max = 16000; Fmin = 120; Fmax = 18000;
% 	SamplingRate = 48000; N = 4; order = 15; Fa_max = 16000; Fmin = 120; Fmax = 18500;
end

% Fp = [300, 3400]; Fs = [100, 3600];
% Rp = 0.2; Rs = 70;

% Fs = 1:50:SamplingRate/2-101; Fp = Fs+100;
if design_LPF == 1
  Fn = 75:75:Fa_max;
%   plot(Fs);  pause
  Fn_ = [20:20:200, (Fmax-200):75:(Fmax+1000)];
else
%   Fs = 1:75:Fs_max; Fp = Fs+100;
  Fn = 75:75:Fa_max;
  Fn_ = [20:20:200, (Fmax-200):50:(Fmax+1000)];
  
end
Rp = 1; Rs = 70;

figure(1)
set(1, 'DoubleBuffer', 'on');

for ind = 1:length(Fn)
  Wn = 2*Fn(ind)/SamplingRate;
  if (design_LPF == 1)
    % [B,A] = ELLIP(N,Rp,Rs,Wn, 'high') ;
    % B(2:2:end) = -B(2:2:end);  A(2:2:end) = -A(2:2:end);
    [B,A] = ELLIP(N,Rp,Rs,Wn) ;
  else
    [B,A] = ELLIP(N,Rp,Rs,Wn, 'high') ;
  end
  
  f_all(ind) = Fn(ind);
  b_all(ind, 1:N+1) = B;
  a_all(ind, 1:N+1) = A;
  freqz(B, A, 1*2048, SamplingRate);
  set(gca, 'Ylim', [-100, 1]);  
  pause(0)
end

if do_not_save == 1,
  return
end

% Fs = 1:75:SamplingRate/2-101; Fp = Fs+100;
f_all_ = Fn;

% SamplingRate = 32000; N = 6; order = 15; Fs_max = 12000
% Rp = 1; Rs = 70;
if design_LPF == 1,
  filename = sprintf('LPF_iir_coefs_%i_%i.txt', SamplingRate, N);
else
  filename = sprintf('HPF_iir_coefs_%i_%i.txt', SamplingRate, N);
end
plik = fopen(filename, 'wt');
if design_LPF == 1,
  fprintf(plik, 'LPF IIR filter coeficients: sampling rate = %i, filter order = %i\n', SamplingRate, N);
  fprintf(plik, 'polynomial order = %i, approximation limit Fa_max = %iHz\n', order, Fa_max);
  fprintf(plik, 'maximum allowed F_min = %iHz, maximum allowed F_max = %iHz\n', Fmin, Fmax);
else
  fprintf(plik, 'HPF IIR filter coeficients: sampling rate = %i, filter order = %i\n', SamplingRate, N);
  fprintf(plik, 'polynomial order = %i, approximation limit Fa_max = %iHz\n', order, Fa_max);
  fprintf(plik, 'maximum allowed F_min = %iHz, maximum allowed F_max = %iHz\n', Fmin, Fmax);
end
fprintf(plik, 'Rp = %.2f, Rs = %.2f\n\n', Rp, Rs);

f_all__ = Fn_;

figure(2)
subplot(2,1,1)
freq_factor = 3.0;
fprintf(plik, 'Freq_factor = %.2f\n\n', freq_factor);
for ind=1:N+1,
  plot(f_all, b_all(:,ind));
  coef_b{ind} = polyfit((f_all(:)-Fa_max/2)*freq_factor/SamplingRate, b_all(:,ind), order);
  %DSP::Float const_22050_coef_b_5[16] =
  fprintf(plik, 'DSP::Float const_%i_coef_b_%i[%i]\n  = { ', SamplingRate, ind-1, order+1);
  %fprintf(plik, 'coef_b[%i] = { ', ind);
  fprintf(plik, '%.15g, ', coef_b{ind}(1:end-1));
  fprintf(plik, '%.15g ', coef_b{ind}(end));
  fprintf(plik, '};\n');
  b_all_F(:,ind) = polyval(coef_b{ind}, (f_all__(:)-Fa_max/2)*freq_factor/SamplingRate);
  hold on
  plot(f_all__(:), b_all_F(:,ind), 'r');
end
hold off
fprintf(plik, '\n');
subplot(2,1,2)
for ind=1:N+1,
  plot(f_all, a_all(:,ind));
  coef_a{ind} = polyfit((f_all(:)-Fa_max/2)*freq_factor/SamplingRate, a_all(:,ind), order);
  %DSP::Float const_22050_coef_a_5[16] =
  fprintf(plik, 'DSP::Float const_%i_coef_a_%i[%i]\n  = { ', SamplingRate, ind-1, order+1);
  %fprintf(plik, 'coef_a[%i] = { ', ind);
  fprintf(plik, '%.15g, ', coef_a{ind}(1:end-1));
  fprintf(plik, '%.15g ', coef_a{ind}(end));
  fprintf(plik, '};\n');
  a_all_F(:,ind) = polyval(coef_a{ind}, (f_all__(:)-Fa_max/2)*freq_factor/SamplingRate);
  hold on
  plot(f_all__(:), a_all_F(:,ind), 'r');
end
hold off
fclose(plik);

pause

% b = []; a = [];
% for ind=1:N+1,
%   b(ind) = polyval(coef_b{ind}, (100-Fa_max/2)*freq_factor/SamplingRate);
%   a(ind) = polyval(coef_a{ind}, (100-Fa_max/2)*freq_factor/SamplingRate);
% end
% b
% a
% freqz(b,a, 8*2048)
% return

figure(3)
for ind = 1:length(Fn_)
  Fn_(ind)
  B = b_all_F(ind, 1:N+1);
  A = a_all_F(ind, 1:N+1);
  [H,F] = freqz(B, A, 1*2048, SamplingRate);

% subplot(2,1,1)
% freqz(B, A, 8*2048, 11025)
subplot(2,1,2)
zplane(B, A);

% pause

subplot(2,1,1)
  if (Fn_(ind) <= Fmax) & (Fn_(ind) >= Fmin),
    plot(F, 20*log10(abs(H)), 'b');
  else
    plot(F, 20*log10(abs(H)), 'r');
  end
  set(gca, 'Ylim', [-100, 1]);  
  pause(0)
  
  hold off;  pause; hold on
end

return

Fs = 101:50:SamplingRate/2-1; Fp = Fs-100;
Rp = 1; Rs = 70;

figure(3)
set(1, 'DoubleBuffer', 'on');

% N = 7;
for ind = 1:length(Fs)
  Fp(ind)
  Wn = 2*Fp(ind)/SamplingRate;
  [B,A] = ELLIP(N,Rp,Rs,Wn) 
  freqz(B, A, 8*2048, SamplingRate);
  set(gca, 'Ylim', [-100, 1]);  
  pause(0)
end
