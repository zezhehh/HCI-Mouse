clear all;
M = csvread('data_single_813.csv');
time = M(:,1);
value = M(:,2);
sample_time = [time(1):1/(1200):time(end)];

len = 1000;
sample_value = interp1(time,value,sample_time,'nearest');


sample_value_smooth=smooth(sample_value,19)';

% figure;
% hold on;
% % plot(sample_time, sample_value);
% % plot(sample_time, sample_value_smooth,'r');
% plot(sample_time, sample_value - sample_value_smooth,'black');
% hold off;

sample_value_DCremove = smooth(sample_value - sample_value_smooth, 5);
% sample_value_DCremove = sample_value;

figure;
for i=3:10:len
    plot(sample_value_DCremove (i:i+9),'.-');
    hold on;
end



value = zeros(1,5);
for i=1:5
    temp_sample = sample_value_DCremove(i:5:len);
    value(i) = std(temp_sample(temp_sample > (max(temp_sample)+min(temp_sample))/2));
end

[std_min,shift_index] = min(value);
shift_index
sample_wave = sample_value_DCremove(shift_index:5:len);


% thresholding
bit_stream = sample_wave <= (max(temp_sample)+min(temp_sample))/2;
bit_stream = int16(bit_stream);

output=zeros(1,10);
correct_n = 0;

for i=1:length(bit_stream) - 50 
    if (bit_stream(i) == 1 && bit_stream(i+1) == 0 && bit_stream(i+2) == 1 && bit_stream(i+3) == 0)
        f = 0;
%         bit_stream(i:i+38)'
        for j = 1:3:30
            if (bit_stream(i+3+j:i+3+j+2) == [1,0,0]')
                output(int16((j-1)/3)+1) = 1;
            elseif (bit_stream(i+3+j:i+3+j+2) == [1,1,0]')
                output(int16((j-1)/3)+1) = 0;
            else
                f = 1;
%                 disp('false');
                break;
                
            end
        end
        if(f == 0)
            output
%             i
            correct_n = correct_n+1;
            i = i+34;
        else
            i = i+1;
        end
%         output
%         i
    end
end
correct_n

% value 1 0 1 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0