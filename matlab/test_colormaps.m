function test_colormaps

% HSV, GRAY, HOT, COOL, BONE, COPPER, PINK, FLAG, PRISM, JET,
%     COLORMAP, RGBPLOT, HSV2RGB, RGB2HSV

figure(1)

colormap(jet)
% colormap(cool)
x = colormap;
N = size(x,1);
a = 8*linspace(0,1,N);

subplot(2,1,1)
plot(a,x(:,1), 'r');
hold on
plot(a,x(:,2), 'g');
plot(a,x(:,3), 'b');
hold off

colorbar

[r, g, b] = test(a/8);
subplot(2,1,2)
plot(a,r, 'r');
hold on
plot(a,g, 'g');
plot(a,b, 'b');
hold off

function [r, g, b] = test(val_in),

%       // red
for ind = 1:length(val_in),
  val = val_in(ind);
  
  if (val < 0.375)
    r(ind) = 0;
  else
    if (val < 0.625)
      r(ind) = 4*val-1.5;
    else
      if (val < 0.875)
        r(ind) = 1.0;
      else
        r(ind) = -4*val+4.5;
      end
    end
  end

%     // green
  if (val < 0.125)
    g(ind) = 0;
  else
    if (val < 0.375)
      g(ind) = 4*val-0.5;
    else
      if (val < 0.625)
        g(ind) = 1;
      else
        if (val < 0.875)
          g(ind) = -4*val+3.5;
        else
          g(ind) = 0;
        end
      end
    end
  end

%       // blue
  if (val < 0.125)
    b(ind) = 4*val+0.5;
  else
    if (val < 0.375)
      b(ind) = 1.0;
    else
      if (val < 0.625)
        b(ind) = -4*val+2.5;
      else
        b(ind) = 0;
      end
    end
  end
end