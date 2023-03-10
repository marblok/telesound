function test_firls2(min_PE, W)
% todo: check relation in function of min_PE
if nargin == 0,
  min_PE = -110; W = 10; % W >= 10000 this does not make sense
  W = 1;
end
do_calculation = 1
if do_calculation == 1,
	% N_FIR_zakres = 2:100;
% 	N_FIR_zakres = 4:4:1000; % best results for filter of the length which is the multiple of 4
	N_FIR_zakres = 3:4:1000; % best results for filter of the length which is the multiple of 4 - 1
% 	N_FIR_zakres = 3:2:1000; % best results for filter of the length which is the multiple of 4 - 1
                           % for W = 1;
	N_FIR= N_FIR_zakres(1);
	
	fp = 1/8; 
	% fp_zakres = [1/32, 1/8, 3/16, 7/32];
	fp_zakres = linspace(1/128/4, 1/4-1/128, 41);
	K = 2048; v = linspace(0, 1, K+1); v(end) = [];
	for ind_0=1:length(fp_zakres),
    fp = fp_zakres(ind_0);
%   clear PE
      
    fs = 1/2 - fp;
    figure(1)
    subplot(2,1,2)
    plot(fp*[1,1], [-180, 5], 'k:');
    hold on
    plot(fs*[1,1], [-180, 5], 'k:');
    set(gca, 'Ylim', [-180,5]);
    
    best_PE = 0;
    start_ind = find(N_FIR_zakres == N_FIR(end));
    for ind = start_ind(1):length(N_FIR_zakres),
      N_FIR(ind_0) = N_FIR_zakres(ind)
        
      [h,a]=get_firls(N_FIR(ind_0), fp, fs, W);
    
      PE(ind_0) = 20*log10(abs(polyval(h, exp(j*2*pi*fs))));

      if PE(ind_0) < best_PE
        best_PE =  PE(ind_0);
      else
        [best_PE, PE(ind_0)]
        break;
      end
      
      PE(ind_0)
      pause(0)
      if (PE(ind_0) <= min_PE)
        H = fft(h, K);
%         H_dB = 20*log10(abs(H));
        [Hid, v] = Hideal((N_FIR(ind_0)-1)/2, 1/4, K)
        H_dB = 20*log10(abs(H - Hid));
        
        figure(1)
        subplot(2,1,2)
        plot(v, H_dB);
        hold on
        
        break;
      end
    end

    figure(1)
    subplot(4,1,1)
    plot(fp_zakres(1:length(N_FIR)),log2(N_FIR));
    subplot(4,1,2)
    plot(fp_zakres(1:length(PE)), PE);
    
    pause(0)
	%   pause
	end
	subplot(4,1,1)
	hold off
	subplot(4,1,2)
	hold off
	subplot(2,1,2)
	hold off

  if W >= 1,
    if W > 1,
      tekst = sprintf('test_firls2_%i_%.0f.mat', min_PE, W);
    else
      tekst = sprintf('test_firls2_%i.mat', min_PE);
    end
  else
    tekst = sprintf('test_firls2_%i_%.2f.mat', min_PE, W);
  end
  save(tekst, 'fp_zakres', 'N_FIR', 'min_PE');
else
  if W >= 1,
    if W > 1,
      tekst = sprintf('test_firls2_%i_%.0f.mat', min_PE, W);
    else
      tekst = sprintf('test_firls2_%i.mat', min_PE);
    end
  else
    tekst = sprintf('test_firls2_%i_%.2f.mat', min_PE, W);
  end
  load(tekst);
end
a = polyfit(fp_zakres(1:length(N_FIR)),N_FIR, 6)
a2 = polyfit(fp_zakres(1:length(N_FIR)),log2(N_FIR), 6)
figure(2)
subplot(2,1,1)
plot(fp_zakres(1:length(N_FIR)),N_FIR);
hold on
plot(fp_zakres(1:length(N_FIR)),polyval(a, fp_zakres(1:length(N_FIR))), 'r');
plot(fp_zakres(1:length(N_FIR)),2.^polyval(a2, fp_zakres(1:length(N_FIR))), 'm');
hold off
subplot(2,1,2)
plot(fp_zakres(1:length(N_FIR)),log2(N_FIR));
hold on

fp_min = fp_zakres(1);
a2_coeff = [fp_min, a2];
save( sprintf('N_FIR_a2_%i.txt', -min_PE), 'a2_coeff', '-ASCII', '-DOUBLE');

fp_ = linspace(0, fp_zakres(end), 200);
estym_N_FIR = polyval(a2, fp_);
% od 3 co 4
estym_N_FIR = 3 + 4*round((2.^estym_N_FIR - 3) / 4);
% % od 3 co 4
% estym_N_FIR = 3 + 2*round((2.^estym_N_FIR - 3) / 2);

set(plot(fp_,log2(estym_N_FIR), 'g'), 'LineWidth', 3);
plot(fp_zakres(1:length(N_FIR)),polyval(a2, fp_zakres(1:length(N_FIR))), 'r');
hold off

