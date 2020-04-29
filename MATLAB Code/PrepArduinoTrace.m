function [Trace, badStarts, ind] = PrepArduinoTrace(UndecimatedEMTrace,per,windTime)
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
Trace = UndecimatedEMTrace(:,3);

% add buffer after if ending with a reach
if UndecimatedEMTrace(end,2)~=0
    UndecimatedEMTrace = [UndecimatedEMTrace;transpose(UndecimatedEMTrace(end,1):UndecimatedEMTrace(end,1)+199) zeros(200,1), repmat(nanmedian(Trace),200,1)];
    Trace = UndecimatedEMTrace(:,3);
end

% add buffer before if starting with a reach
first = find(~isnan(UndecimatedEMTrace(:,2)),1);
if UndecimatedEMTrace(first,2)~=0 || UndecimatedEMTrace(first,3)> nanmedian(Trace)*5
    UndecimatedEMTrace(first-200:first-1,2:3) = [zeros(200,1) repmat(nanmedian(Trace),200,1)];
    Trace = UndecimatedEMTrace(:,3);
end
%%
ind = zeros(size(Trace)); badStarts = [];
med = nanmedian(Trace)*5; %scaled to be greater than noise
medTH = nanmedian(Trace)*20; %scaled to be greater than noise but less than reach peaks
if med == 0 % if median value is 0 need to find median of non zero trace
    med = nanmedian(Trace(Trace~=0))*5;
    medTH = nanmedian(Trace(Trace~=0))*20;
end
    

winds = find(mod(1:size(Trace,1),windTime)==0); nwinds = size(winds,2); 

for i = 1:nwinds
    if i == 1
        wind = Trace(1:winds(i)); %first window
    elseif i == nwinds %last window
        wind = Trace(winds(i):end);
    else
        wind = Trace(winds(i):winds(i+1));
    end
    
    if sum(wind>medTH)>size(wind,1)*per % is per% of this window greater than medTH
        % find begining of bad start
        ct = 0; j = winds(i);
        while ct<100 %is JS position below 5 times the median JS position for at lease 100 ms
            if Trace(j)<med
                ct = ct+1;
            end
            j = j-1;
        end
        S = j;
        % Find end of bad start
        ct = 0; j = winds(i+1);
        while ct<100
            if Trace(j)<med
                ct = ct+1;
            end
            j = j+1;
        end
        E = j;
        
        bad = Trace(S:E);
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
        Trace(S:E) = bad; ind(S:E) = ones(size(bad));
        badStarts = [badStarts; S+s];
        if extraplott == 1; plot(bad,'g'); legend('original','fixed'); title(sprintf('BL issue %d',size(badStarts,1))); xlabel('relative time (ms)'); ylabel('amplitude (au)');end
    end
end

if plott
    figure; plot(UndecimatedEMTrace(:,3),'r'); hold on; plot(UndecimatedEMTrace(:,2)*25,'color',[.5 .5 .5]); plot(Trace,'g')
    xlabel('time (ms)'); ylabel('amplitude (au)');
    legend('original','EM', 'fixed')
    txt = sprintf('Found %d bl issues',size(badStarts,1)); title(txt)
    if ~isempty(badStarts); vline(badStarts); end
end
Trace = [UndecimatedEMTrace(:,1:2) Trace];
