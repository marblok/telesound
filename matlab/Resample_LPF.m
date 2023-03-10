function Resample_LPF

% 48000 ==> 8000
Fp1 = 48000; Fp2 = 8000;
L = 1; M = 6; order = 14;
[b,a] = ellip(order, 0.1, 100, 0.94*L/M);
sos = tf2sos(b,a);

% b = round(b*1000000)/1000000;
% a = round(a*1000000)/1000000;
% 
figure(1)
freqz(b,a, 8*2048, Fp1)
subplot(2,1,2)
delta = 1; delta(1000) = 0;
plot(filter(b,a, delta));

figure(2)
zplane(b,a)

% save_lpf(Fp1, Fp2, L, M, order, b, a);
save_lpf_sos(Fp1, Fp2, L, M, order, sos);
pause

% 44100 ==> 11025
Fp1 = 44100; Fp2 = 11025;
L = 1; M = 4; order = 14;
[b,a] = ellip(order, 0.1, 100, 0.94*L/M);
sos = tf2sos(b,a);

figure(1)
freqz(b,a, 8*2048, Fp1)
subplot(2,1,2)
delta = 1; delta(1000) = 0;
plot(filter(b,a, delta));

figure(2)
zplane(b,a)

% save_lpf(Fp1, Fp2, L, M, order, b, a);
save_lpf_sos(Fp1, Fp2, L, M, order, sos);
pause

% 48000 ==> 16000
Fp1 = 48000; Fp2 = 16000;
L = 1; M = 3; order = 14;
[b,a] = ellip(order, 0.1, 100, 0.945*L/M);
sos = tf2sos(b,a);

figure(1)
freqz(b,a, 8*2048, Fp1)
subplot(2,1,2)
delta = 1; delta(1000) = 0;
plot(filter(b,a, delta));

figure(2)
zplane(b,a)

% save_lpf(Fp1, Fp2, L, M, order, b, a);
save_lpf_sos(Fp1, Fp2, L, M, order, sos);
pause

% 44100 ==> 22050
Fp1 = 44100; Fp2 = 22050;
L = 1; M = 2; order = 14;
[b,a] = ellip(order, 0.1, 100, 0.95*L/M);
sos = tf2sos(b,a);

figure(1)
freqz(b,a, 8*2048, Fp1)
subplot(2,1,2)
delta = 1; delta(1000) = 0;
plot(filter(b,a, delta));

figure(2)
zplane(b,a)

save_lpf(Fp1, Fp2, L, M, order, b, a);
pause

% 48000 ==> 32000
% 96000 ==> 32000
Fp1 = 48000; Fp2 = 32000;
L = 1; M = 3; order = 14;
[b,a] = ellip(order, 0.1, 100, 0.945*L/M);
sos = tf2sos(b,a);

figure(1)
freqz(b,a, 8*2048, L*Fp1)
subplot(2,1,2)
delta = 1; delta(1000) = 0;
plot(filter(b,a, delta));

figure(2)
zplane(b,a)

% save_lpf(Fp1, Fp2, L, M, order, b, a);
save_lpf_sos(Fp1, Fp2, L, M, order, sos);


% +++++++++++++++++++++++++++++++++++++++++++ 
function save_lpf(Fp1, Fp2, L, M, order, b, a);

filename = sprintf('Fp1 = %i, Fp2 = %i.txt', Fp1, Fp2);

plik = fopen(filename, 'wb');
fprintf(plik, 'Fp1 = %i, Fp2 = %i\n', Fp1, Fp2);
fprintf(plik, 'L = %i, M = %i\n', L, M);
fprintf(plik, 'order = %i\n', order);
fprintf(plik, 'b = {%.15f, ', b(1));
fprintf(plik, '%.15f, ', b(2:end-1));
fprintf(plik, '%.15f};\n', b(end));
fprintf(plik, 'a = {%.15f, ', a(1));
fprintf(plik, '%.15f, ', a(2:end-1));
fprintf(plik, '%.15f};\n', a(end));
fclose(plik);


% +++++++++++++++++++++++++++++++++++++++++++ 
function save_lpf_sos(Fp1, Fp2, L, M, order, sos);

filename = sprintf('Fp1 = %i, Fp2 = %i.txt', Fp1, Fp2);

plik = fopen(filename, 'wb');
fprintf(plik, 'Fp1 = %i, Fp2 = %i\n', Fp1, Fp2);
fprintf(plik, 'L = %i, M = %i\n', L, M);
fprintf(plik, 'order = %i, no_of_sos_segments = %i\n', order, size(sos,1));

for ind=1:size(sos,1)
  b = sos(ind,1:3);
  fprintf(plik, 'wave_in_resample_LPF_b_%i_%i[%i] = {%.15f, ', Fp1, Fp2, ind-1, b(1));
  fprintf(plik, '%.15f, ', b(2:end-1));
  fprintf(plik, '%.15f};\n', b(end));
end
for ind=1:size(sos,1)
  a = sos(ind,4:6);
  fprintf(plik, 'wave_in_resample_LPF_a_%i_%i[%i] = {%.15f, ', Fp1, Fp2, ind-1, a(1));
  fprintf(plik, '%.15f, ', a(2:end-1));
  fprintf(plik, '%.15f};\n', a(end));
end
fclose(plik);


