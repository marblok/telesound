function convert_wavs

% normalize_wavs('..\audio_wzor\Logatomy', '..\audio_wzor_2\Zdania\Logatomy');
% normalize_wavs('..\audio_wzor\Zdania\male1', '..\audio_wzor_2\Zdania\male1');
% normalize_wavs('..\audio_wzor\Zdania\male2', '..\audio_wzor_2\Zdania\male2');
% normalize_wavs('..\audio_wzor\Zdania\female1', '..\audio_wzor_2\Zdania\female1');
% normalize_wavs('..\audio_wzor\Zdania\female2', '..\audio_wzor_2\Zdania\female2');

% resample_wavs('..\audio_wzor\Zdania\male1', '..\audio_wzor_3\Zdania\male1');
% resample_wavs('..\audio_wzor\Zdania\male2', '..\audio_wzor_3\Zdania\male2');
% resample_wavs('..\audio_wzor\Zdania\female1', '..\audio_wzor_3\Zdania\female1');
% resample_wavs('..\audio_wzor\Zdania\female2', '..\audio_wzor_3\Zdania\female2');
%  resample_wavs('..\audio_wzor\Logatomy', '..\audio_wzor_3\Logatomy');

% normalize_wavs('..\audio_wzor_3\Zdania\male1', '..\audio_wzor_3b\Zdania\male1');
% normalize_wavs('..\audio_wzor_3\Zdania\male2', '..\audio_wzor_3b\Zdania\male2');
% normalize_wavs('..\audio_wzor_3\Zdania\female1', '..\audio_wzor_3b\Zdania\female1');
% normalize_wavs('..\audio_wzor_3\Zdania\female2', '..\audio_wzor_3b\Zdania\female2');
% normalize_wavs('..\audio_wzor_3\Logatomy', '..\audio_wzor_3b\Logatomy');

function normalize_wavs(dir_in, dir_out)

warning off MATLAB:MKDIR:DirectoryExists
mkdir(dir_out)

x = dir(dir_in);

for ind = 1:length(x)
  if (x(ind).isdir == 0)
    name = x(ind).name
    
    [y, Fp, B] = wavread([dir_in , '\', name]);
    
    y = y/max(abs(y));
    mean(y)
    wavwrite(y, Fp, B, [dir_out , '\', name]);

%     plot(y, '.')
%     psd(y,8192,Fp,blackman(2*2048))
%     
    pause(0)
  end
end
 
% ++++++++++++++++++++++++++++++++++++++++++++++++ %
function resample_wavs(dir_in, dir_out)

warning off MATLAB:MKDIR:DirectoryExists
mkdir(dir_out)

x = dir(dir_in);

for ind = 1:length(x)
  if (x(ind).isdir == 0)
    name = x(ind).name
    
    cmd_str = sprintf('! ..\\resampler\\resampler "%s" "%s" ..\\resampler', [dir_in , '\', name], [dir_out , '\', name]);
    
%     cmd_str
    eval(cmd_str);

%     [y, Fp, B] = wavread([dir_out , '\', name]);
%     plot(y, '.')
%     psd(y,8192,Fp,blackman(2*2048))
    pause(0.1)
  end
end
 