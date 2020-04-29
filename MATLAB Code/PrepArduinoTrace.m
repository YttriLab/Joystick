function [trace, badStarts, ind] = PrepArduinoTrace(UndecimatedEMTrace,per,windTime)
% This function identifies artificial changes in baseline and rectifies them according to median
%%
% Function needed
% vline.m
%
% Arguments
% UndecimatedEMTrace - undecimated Arduino position with time and eventmarkers [time EM Position]
% per - %what percent of the window much be greater than threshold? .75 good starting place but .995 works well with long reaches
    %my dataset works well with .9
% windTime - size of time (ms) window to look at, increase time decreases specificity
    %my dataset works well with 8500
%
% Output
% trace - corrected JS position with BL issues corrected with undecimated eventmarker and time
% BadStarts - time at which baseline issue was identified and fixed
% ind - index of time when trace was altered
%
% MAN
%%
extraplott = 0; %plots all baseline issues
plott = 0; %plots summary figure
%%
trace = UndecimatedEMTrace(:,3);

% add buffer after if ending with a reach
if UndecimatedEMTrace(end,2)~=0
    UndecimatedEMTrace = [UndecimatedEMTrace;transpose(UndecimatedEMTrace(end,1):UndecimatedEMTrace(end,1)+199) zeros(200,1), repmat(nanmedian(trace),200,1)];
    trace = UndecimatedEMTrace(:,3);
end

% add buffer before if starting with a reach
first = find(~isnan(UndecimatedEMTrace(:,2)),1);
if UndecimatedEMTrace(first,2)~=0 || UndecimatedEMTrace(first,3)> nanmedian(trace)*5
    UndecimatedEMTrace(first-200:first-1,2:3) = [zeros(200,1) repmat(nanmedian(trace),200,1)];
    trace = UndecimatedEMTrace(:,3);
end
%%
ind = zeros(size(trace)); badStarts = [];
med = nanmedian(trace)*5; %scaled to be greater than noise
medTH = nanmedian(trace)*20; %scaled to be greater than noise but less than reach peaks
winds = find(mod(1:size(trace,1),windTime)==0); nwinds = size(winds,2); 

for i = 1:nwinds
    if i == 1
        wind = trace(1:winds(i)); %first window
    elseif i == nwinds %last window
        wind = trace(winds(i):end);
    else
        wind = trace(winds(i):winds(i+1));
    end
    
    if sum(wind>medTH)>size(wind,1)*per % is per% of this window greater than medTH
        % find begining of bad start
        ct = 0; j = winds(i);
        while ct<100 %is JS position below 5 times the median JS position for at lease 100 ms
            if trace(j)<med
                ct = ct+1;
            end
            j = j-1;
        end
        S = j;
        % Find end of bad start
        ct = 0; j = winds(i+1);
        while ct<100
            if trace(j)<med
                ct = ct+1;
            end
            j = j+1;
        end
        E = j;
        
        bad = trace(S:E);
        if extraplott == 1; figure; plot(bad,'r'); hold on; end
        [L,n] = bwlabel(bad<medTH);
        s = find(L==1,1,'last');
        e = find(L==n,1);
        bad(s:e) = abs(bad(s:e)-median(bad(s:e)));
        
        [L,n]= bwlabel(bad>median(bad));
        bad(L==n)= median(bad);
        % replace return to normal with median value of the bad
        % bl changes because of reach at end of iti so leave start of bl shift as reach
        % isssue is that the kinematics (vel amp duration) are inaccurate...)
        % ind notates where patches have been made
        trace(S:E) = bad; ind(S:E) = ones(size(bad));
        badStarts = [badStarts; S+s];
        if extraplott == 1; plot(bad,'g'); legend('original','fixed'); title(sprintf('BL issue %d',size(badStarts,1))); xlabel('relative time (ms)'); ylabel('amplitude (au)');end
    end
end

if plott
    figure; plot(UndecimatedEMTrace(:,3),'r'); hold on; plot(UndecimatedEMTrace(:,2)*25,'color',[.5 .5 .5]); plot(trace,'g')
    xlabel('time (ms)'); ylabel('amplitude (au)');
    legend('original','EM', 'fixed')
    txt = sprintf('Found %d bl issues',size(badStarts,1)); title(txt)
    if ~isempty(badStarts); vline(badStarts); end
end
trace = [UndecimatedEMTrace(:,1:2) trace];
