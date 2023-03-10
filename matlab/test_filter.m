function test_filter

LPF_new_Nb = 8
  LPF_new_coefs_b = [0.000065 -0.000324 0.000581 -0.000322 -0.000322 0.000581 -0.000324 0.000065];
LPF_new_Na = 8
  LPF_new_coefs_a = [1.000000 -6.941425 20.656483 -34.160709 33.906540 -20.198803 6.686966 -0.949050];
  
  
figure(1)
subplot(2,1,1)
freqz(LPF_new_coefs_b, LPF_new_coefs_a, 8*2048, 11025)
subplot(2,1,2)
zplane(LPF_new_coefs_b, LPF_new_coefs_a);

