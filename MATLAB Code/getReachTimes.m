function [ reachStart, reachStop, reach0, pos1, pos2] = getReachTimes( InData, Name, Date )
%Extracts best reach start times from .ns2 data unless BabData set to 1 for
%processed ContData files.
%   Can be made to extract from other data types.  Assumes 1kHz sampling
%filenamestr should be an .ns2 file or ContData.behavior.sLeverData
%   If .ns2, assumes  X and Y position are channels 2 and 3 (line 13)
%outputs, in order:
%  reachStart and reachStop are vectors of reach times
%  reach0 is the amplitude readout of the whole session
%  pos1 = all reach traces, aligned to start
%  pos2 = all reach traces, aligned to stop
%      Yttri 2016
% data = InData(:,1)
% for i = 1:size(InData,2)
%     ms_0 = InData(i,1);
%     ms_1 = InData(i+1,1);
%     d = zeros(ms_1-ms_0,19);
%
%     InData(1:i,:)
%     InData(i:end,:)
%
%     InData(i,1):InData(i+1,1)

% InData = InData(find(isnan(InData(:,2)),1,'last')+1:end,:);
[a,~] = bwlabel(isnan(InData(:,2)));
InData(logical(a),:) = [];

SR = 1/mean(diff(InData(:,1)))*1000;

TbetweenR = 2000; %minimum time between reaches, in ms.  good for disallowing reaching for reward
maxDur=4500; %maximum reach duration
plotOn=1;  %to plot or not


%% EXTRACT POSITION DATA, EITHER ns2 or ContData files
% if isstr(filenamestr) %assumes ContData has already been loaded for session
%     NsDATA = openNSx('report','read',['/', filenamestr]);
% else
%     NsDATA.Data=[nan(1,length(filenamestr)); filenamestr];
% end


%NsDATA.Data=D;
%NsDATA.Data=double(NsDATA.Data);

%determine euclidean eccentricity
% reach1= sgolayfilt(sqrt((InData(:,4)'-median(InData(:,4))).^2 + (InData(:,5)'-median(InData(:,5))).^2),3,201);
% reach1= sgolayfilt(sqrt((InData(:,4)'-InData(:,8)').^2 + (InData(:,5)'-InData(:,9)').^2),3,201);
reach1= sgolayfilt(InData(:,3),3,201);
%  sgolay injects a negative dip before a reach as an artifact of the filter.
%  it is quite nice for reach detection, but makes for poor position traces
% reach0= moveavg(sqrt((InData(:,4)'-median(InData(:,4))).^2 + (InData(:,5)'-median(InData(:,5))).^2),8);
% reach0= moveavg(sqrt((InData(:,4)'-InData(:,8)').^2 + (InData(:,5)'-InData(:,9)').^2),8);
reach0= moveavg(InData(:,3),8);
vel=[0, diff(reach1)'];


%% determine reach position threshold
scc=sort(reach1);    %rank  joystick positions, most will be  = median (js center)
scd=sort(vel);     %rank  veocities, most will be ~0
scc(scc<median(scc))=median(scc); % no interest in sub-baselines
%the sorted files create a curve in which most of the values lie at or near
%the median, with a sharp upward deflection at the far right, representing
%the minority of the session in which a reach is taking place.  By
%essentially lumping all your reach and non-reach times together,
%respectively, you get a greater signal to noise and a better estimate of
%what the amplitude should be.  Furthmore, rather than using any specific
%amplitude value, this automatically scales to whatever the distribution of
%reach amplitudes is for a given dataset.
cutoff = scc(round(length(scc)*.70)); % can use .85  surprisingly good for finding the inflection point, which is not as easy and max convexity
cutoffd = scd(round(length(scd)*.98)); % .95  these values work beter than max convexity (second derivative)
allValidSamps1=find(vel>cutoffd); %find timestamp  of all js positions  > thesh
allValidSamps1=allValidSamps1(allValidSamps1<(length(reach1)-300)); %eliminates reaches too late in the session to be analyzed

%% determine potential reach starts
allValidSamps=[];
for jj = 1:length(allValidSamps1)
    if length(allValidSamps)==0  %find first reach
        if sum(reach1(allValidSamps1(jj):allValidSamps1(jj)+200)>cutoff)>20  %reach must be above thresh fo 20ms
            allValidSamps=[allValidSamps, allValidSamps1(jj)];
            
        end
    else
        if (allValidSamps1(jj)-allValidSamps1(jj-1))>100 %controls for wobbly reach, where the joystick falls below threshold, but only momentaritly
            %  if (allValidSamps1(jj)-allValidSamps(end)) >TbetweenR  %inter reach interval control
            if sum(reach1(allValidSamps1(jj):allValidSamps1(jj)+200)>cutoff)>20  %reach must be above thresh fo 20ms
                
                allValidSamps=[allValidSamps, allValidSamps1(jj)];  %if all of these are satisfied, it's a likely reach start
                
            end;
        end; %end
    end;
end


reachStart=[];
for i = 1:length(allValidSamps)
    ii=allValidSamps(i); %for each potential reach
    iii=find(reach1(1:ii)<cutoff*.4,1,'last'); %find the first time previous to that timestamp where the amplitude fell before 40% of the amplitude
    if iii>150  %if that timestamp is greater than 150 ms
        if ii-iii>1000  %sometimes the joystick doesn't reset to zero and induces 20 secodn long reaches.  this prevents that
            scc2=sort((reach1(iii:ii))); %perform the sorting of positions on just that subset of of times
            cutoff2 = scc2(round(length(scc2)*.90)); %.85  find the uptick in curve again.  this is the amplitude threshold specific for this reach
            iii=find(reach1(1:ii)<cutoff2,1,'last');
        end
        realStartMinus10=find(vel(1:iii)<0,10,'last'); %a reach must start with an increase in velocity.  find it
    else
        realStartMinus10=ii;
    end
    reachStart=[reachStart, realStartMinus10(1)];
end
%  reachStart=reachStart([1,find(diff(reachStart)>TbetweenR)+1]); %checks and balances
reachStart=unique(reachStart)+10;  %checks and balances
reachStart=reachStart(reachStart<length(reach0)-maxDur);  %checks and balances
% this will eliminate pseudo starts,
%you might not want to do eliminate them

th = max(reach0)*.1;
ind = zeros(size(reachStart));
for i = 1:size(reachStart,2)
    %     r = reach0(reachStart(i)-250:reachStart(i)+500);
    %     figure; plot(r); hold on ;
    mn = reach0(reachStart(i):reachStart(i)+500);
    %     plot(repmat(mn(1)+th,751,1))
    %     vline(251)
    
    if sum(mn>(th+mn(1)))<1
        ind(i) = 1;
        %         set(gca,'Color','g')
        %     else
        %         set(gca,'Color','b')
    end
end
reachStart(logical(ind)) = [];
%     if sum(mn>th)>0 & sum(mn>th) ~= size(mn,1)
%
%         mn(180)>mn(1)
%         sum(mn>th) == size(mn,1)
%         sum(mn>th) == 0
%     end
% end

% figure;
% plot(reach1); hold on
% for i = 1:length(reachStart)
%     plot(reachStart(i),reach1(reachStart(i)),'g*')
% end


%%  find reach stop.  actually more difficult than start
reachStop=[];
for i = 1:length(reachStart)
    ii=reach0(reachStart(i):reachStart(i)+maxDur); %define potential times of the reach
    [~,reachPeakTime]=max(ii(1:700)); %find the peak amplitude, typically in the first 300, but 700 is ample
    % DO NOT make it more than 700 MS.  one thing this accomplishes is serving
    % as a check between 2 increases in amplitude that are near in time (eg, within max
    % duration)
    ii2=ii(reachPeakTime:end); %peak ampltude to end
    sortStop=sort(ii2); %this should look familiar.
    jsBinned=hist(ii2,20);
    %binSize=5;  %10 may be better  OLD!  DOESN'T WORK ON LOWER VOLTAGE
    %OUTPUT JS
    %jsBinned=hist(ii2,min(ii2):binSize:max(ii2));  %this is a new way to
    %find the inflection point on the curve  OLD! DOESN'T WORK ON LOWER
    %VOLTAGE OUTPUT JS
    %it may do beter than the 90% used on scc =sort above, and again, doesn't
    %care how big the amplitudes are
    % DIAG     [~,cutoffStop1]=max(jsBinned(1:round(length(jsBinned)/3))); %apply threshold to useful part of data
    % DIAG     cutoffStop=cutoffStop1*binSize+10+min(ii2);
    
    N=150; % # of ms consecutively below threshold, with some cute math tricks
    sortStop(sortStop<median(sortStop))=median(sortStop);
    cutoffStop= sortStop(round(length(sortStop)*.4));%  NEW !!!!
    t=find(ii2<cutoffStop);
    
    x = diff(t)==1;
    f = find([false x']~=[x' false]);
    g = find(f(2:2:end)-f(1:2:end-1)>=N,1,'first');
    almostEnd=t(f(2*g-1))-1; %this is the first member of a string of subthreshold values
    if length(almostEnd)==0
        almostEnd=find(ii2<cutoffStop,1); %find the actual stop value.
        reachStart(i);
%         figure;
%         plot(reach0(reachStart(i):reachStart(i)+maxDur))
%         vline(reachPeakTime)
        if isempty(almostEnd)
            [~,l] = findpeaks(ii2*-1);
            try
                almostEnd = l(1);
            catch
                if isempty(almostEnd)
                    [~,l] = findpeaks(diff(ii2)*-1);
                    try 
                        almostEnd = l(end);
                    catch
                        if ~sum(diff(ii2)) % you have a straight line....
                            almostEnd = 300;
                        end
                    end
                    
                end
            end
        end
%         vline(reachPeakTime+almostEnd)
    end
    %thisStop= find(moveavg(diff(ii2(almostEnd:end)),10)>=0,1);
    reachStop=[reachStop,  reachPeakTime+almostEnd+reachStart(i)]; %thisStop+reachPeakTime+almostEnd+reachStart(i)];
end
% hold on
% for i = 1:length(reachStart)
%     plot(reachStop(i),reach1(reachStop(i)),'r*')
% end

pos1=[];
% reachStart(end)=[];
% reachStop(end+1) = nan
tooShort=(1+find((reachStart(2:end)-reachStop(1:(end-1)))<TbetweenR));
reachStart(tooShort)=[];
reachStop(tooShort)=[];
for i =reachStart  %this takes a segment of time (currently 200 and 999ms) before and after each reach start
    %to provide a quick look at the amplitude profiles of every reach
    if i>200 & i<(length(reach0)-1000)
        pos1=[pos1; (reach0(i-200:i+999)-reach0(i))'];
    end
end


pos2=[];
for i =reachStop  %generate reach traces aligned to stop
    if i>1499
        pos2=[pos2; (reach0(i-1499:i+200)-reach0(i))'];
    end
end



%%  plotting and diagnostics
if plotOn
    
    figure; subplot(1,2,1); imagesc(pos1);
    xlabel('start times, aligned at 200ms')
    subplot(1,2,2); imagesc(pos2); colorbar
    xlabel('stop times, aligned at 1500ms')
    
    txt = sprintf('%s %s\nReach Heat Map Aligned to Start and Stop', Name, Date);
    ha = axes('Position',[0 0 1 1],'Xlim',[0 1],'Ylim',[0 1],'Box','off','Visible','off','Units','normalized', 'clipping' , 'off');
    text(0.5, 0.95, txt,'fontsize',13,'HorizontalAlignment','center','VerticalAlignment', 'bottom');
    hold on
    
    
    figure; plot(reach0)
    hold on; plot(reachStart, reach0(reachStart),'g*')
    hold on; plot(reachStop, reach0(reachStop),'r*')
    tlt = sprintf('%s %s\nJS position with start and stop times',Name, Date);
    
    title(tlt)
end
%
% figure; for i =1:size(pos1,1); plot(pos1(i,:)); hold on; end
% figure; for i =1:size(pos2,1); plot(pos2(i,:)); hold on; end
