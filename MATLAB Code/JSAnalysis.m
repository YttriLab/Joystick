function [] = JSAnalysis(MouseName)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% This function takes in mouse name as the primary argument and will find data saved using SavemicroSDData.m
% This funtion will generate a MouseName_DATA.mat file (cell) with each row being a session and each column being a type of data for that session.
% After calling this function the first time, it will find any new data that has been saved using SavemicroSDData.m and append it to the MouseName_DATA.mat file.
% 
% This funtion requires the following funtions
% getJS.m
% unDecimate.m
% getReachTimes.m
%     moveavg.m -  M. in S. Carlos Adrián Vargas Aguilera, nubeobscura@hotmail.com

% hline - https://www.mathworks.com/matlabcentral/fileexchange/1039-hline-and-vline
% vline - https://www.mathworks.com/matlabcentral/fileexchange/1039-hline-and-vline
% numSubplots - https://www.mathworks.com/matlabcentral/fileexchange/26310-numsubplots-neatly-arrange-subplots
% 


% Datebase should be changed to local directory where data will be saved change base directory to where the data can be found

%JSAnalysis(MouseName)
%
% MouseName - char array of mouse name (ex, 'A1')
% JSAnalysis('A1')
%
%
% MAN, mnichola@andrew.cmu.edu
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%

%find files
Database = 'C:\Users\yttri-lab\Documents\MATLAB\JS2'; %change to location where your data will be saved

%change below basen variables to any location that your data is saved:
base1 = 'N:\yttri-lab\Data Transfer\241_SDData'; 
base2 = 'N:\yttri-lab\Data Transfer\239a_SDData'; % if only one location this variable can be removed or use "base2 = ''";

Bases = {base1 base2}; 
%to add more locations make new "basen" variables (location of data) and append to the above line as the following Bases = {base1 base2 basen}; 

FF={}; nbases = size(Bases,2);
for i =1:nbases
    files = dir([Bases{i} filesep MouseName filesep '*csv']);
    Acell = struct2cell(files)';
    mn = Acell(contains(Acell(:,1), MouseName),:);
    FF = [FF; mn  repmat({i},size(mn,1),1)]; %append location handle 
end
files = FF;

%%
%rename files
%
% This renaming section is based our naming convention of [Letter Number]
% may need to change according to your naming convention
%
filenames = cell(size(files,1),1);
for i = 1:size(files,1)
    if files{i,4} ~= 0
        tlt = files{i};
        if contains(tlt,'-')
            ind = strfind(tlt, '_');
            tlt(ind(1)) = ' '; tlt(ind(2)) = ':'; tlt(ind(end)) = ':';
            filenames{i} = tlt(1:end-4);
        else
            ind = strfind(tlt, '_');
            tlt(ind(1)) = ' '; tlt(ind(4)) = ' '; tlt(ind(5)) = ':'; tlt(ind(end)) = ':';
            
            filenames{i} = [MouseName ' ' datestr(tlt(4:11)) tlt(ind(4):end-4)];
        end
    end
end
%%
%check for/make DATA file
destination = [Database filesep MouseName];
a = dir(destination);
if isempty(a)
    mkdir(destination)
    fprintf('\n\nMade new Directory:\n%s\n\n',destination);
end
D = [destination filesep [MouseName '_DATA.mat']];

if exist(D,'file')
    load(D)
    sz = size(DATA,1); %how many current sessions are saved in DATA
    FILES = files(~contains(files(:,1),DATA(:,1)),:);
    nfiles = size(FILES,1);
else
    nfiles = size(filenames,1);
    DATA = cell(nfiles,17);
    FILES = files;
    sz = 0;
end
% DATA will be a nSessions row by 17 columns of data as below:
% DATA = [1tlt Date endtime data 5undecimatedPos trialspermin reachStart reachStop reach0 10ReachAlighnedtoStart ReachAlighedtoStop maxdisplacement peakamplitude reachesaligned 15trialsper5min undecimatedXY 17rewardedReachTime]

%%
%move files
for i = 1:nfiles
    base = FILES{i,2};
    f = [base filesep MouseName filesep FILES{i}];
    if FILES{i,4} ~= 0
        try
            copyfile(f,destination)
        catch
            f = [base filesep FILES{i}];
            copyfile(f,destination)
        end
        fprintf('\nSaved %s to %s.\n\n',FILES{i,1}, getenv('COMPUTERNAME'));  %getenv('COMPUTERNAME') may not work for linux/MAC...
    else
        fprintf('\n\n%s is %d KB and was deleted.\n\n', FILES{i,1}, FILES{i,4})
        delete(f)
        beep
    end
end
%%
%run analysis for each new session that was not in MouseName_DATA.mat file
hands = zeros(nfiles,1);
for i = 1:nfiles
    F = figure; %session summary figure
    filepth = [destination filesep FILES{i,1}];
    DATA(sz+i,1:4) = getJS(filepth); %[tlt Date endtime data]; - - - can use csvread if you have no issues with your csv files
    %data =  [time EM TrialCt X Y pos baseX baseY SolOpenDuration DelaytoRew ITI Threshold]
    data = DATA{i+sz,4}; ct = 2; threshold = data(end,12); nTrials = data(end,3);
    
    %undecimate JS position
%         Convert js sampling rate to 1 kHz
    for j = [2 6] %column 2(eventmarker) and 6 (JS Position)
        nd = unDecimate(data,j);
        if j == 2
            D = zeros(size(nd,1),3);
            D(:,1:2) = nd;
        else
            D(:,ct) = nd(:,2);
        end
        ct = ct+1;
    end
    DATA{sz+i,5} = D;
    TrialCt = data(end,3);
    SessionTime = ceil((data(end,1)/1000)/60);
    DATA{sz+i,6} = TrialCt/SessionTime;
    
    %plot JS trace
    f = figure; plot(D(:,3))
    hold on
    plot(D(:,2)*25) %eventmarker (EM)
    plot(repmat(threshold, size(D,1),1))
    tlt = sprintf('%s\n%d trials in %d mins',filenames{i+sz}, TrialCt,SessionTime);
    legend({'pos', 'EM', 'threshold'})
    ylabel('Amplitude'); xlabel('Time (ms)');
    title(tlt)
    pth = [Database filesep MouseName filesep FILES{i,1}(1:end-4) '_trace.fig']; %location/name of figure
    savefig(f,pth)
    
    
    ct = 2;
    for j = [4 5] %undecimate X and Y
        nd = unDecimate(data,j);
        if j == 4
            D = zeros(size(nd,1),3);
            D(:,1:2) = nd;
        else
            D(:,ct) = nd(:,2);
        end
        ct = ct+1;
    end
    DATA{sz+i,16} = D;
    
    %%
    %get reach times
    
    data = DATA{i+sz,5};
    Date = DATA{i+sz,2};
    [reachStart, reachStop, reach0, pos1, pos2] = getReachTimes(data, MouseName, Date); %[reachstarttime reachstoptimes fullreachtrace ReachesAlignedtoStart ReachesAlignedtoStop]
    hold on;plot(data(~isnan(data(:,2)),2)*25)%add in event marker (EM)
    legend({'trace','start', 'stop','EM'})
    g = gcf; g = g.Number;
    savefig(g, [Database filesep MouseName filesep DATA{i+sz,1} '_reachStartStops.fig']); %save trace with start stop
    savefig(g-1, [Database filesep MouseName filesep DATA{i+sz,1} '_AlignedHeatMap.fig']); %save aligned reaches
    DATA(i+sz,7:11) = {reachStart, reachStop, reach0, pos1, pos2};
    
    MD = zeros(size(reachStart,2),1); PA = zeros(size(reachStart,2),1);
    for k = 1:size(reachStart,2)
        mn = reach0(reachStart(k):reachStart(k)+400);
        MD(k) = max(mn)-mn(1); %max dispalcement
        PA(k) = max(mn);%peak amplitude        
        %figure; plot((reach0(reachStart(k):reachStart(k)+400)))
    end
    DATA{sz+i,12} = MD; DATA{i+sz,13} = PA;
    
    %align reaches to reach start
    pre = 249; post = 1500;
    reaches_A = zeros(size(reachStart,2),1750);
    d = reach0;
    figure(F); subplot(2,2,2); %plot all reaches and average on summary figure    
    for r = 1:size(reachStart,2)
        if reachStart(r)<pre
            reaches_A(r,:) = [nan(1,pre ) d(reachStart(r):reachStart(r) + post)'];
        elseif reachStart(r)+post > size(d,1)
            reaches_A(r,:) = [d(reachStart(r)-pre:size(d,1))' nan(1,(post+1+pre)-size(d(reachStart(r)-pre:size(d,1)),1))];
        else
            reaches_A(r,:) = d(reachStart(r)-pre:reachStart(r)+post)';
        end
        plot(reaches_A(r,:)); hold on
    end
    
    xlabel('time (ms)'); ylabel('amplitude');
    title('all reaches yellow == mean')
    DATA{i+sz,14} = reaches_A;
    m = nanmean(reaches_A);
    plot(repmat(threshold, size(m,1),1), 'color','k')
    plot(m,'linewidth', 3, 'linestyle', ':','color', [1 1 0])
    hline(threshold) %https://www.mathworks.com/matlabcentral/fileexchange/1039-hline-and-vline
    vline(pre) %https://www.mathworks.com/matlabcentral/fileexchange/1039-hline-and-vline
    
    sem = std(reaches_A)/sqrt(size(reaches_A,1));
    ub = m + sem; lb = m-sem;
    x = 1:length(ub);
    X = [x, fliplr(x)]; Y = [ub, fliplr(lb)];
    
    figure; %plot mean and sem reach
    plot(m); hold on
    fill(X, Y, 'b', 'facealpha', .25, 'edgealpha', .25, 'edgecolor', 'b'); hold on
    hline(threshold) 
    vline(pre)
    
    %PVA - position velocity acceleration
    R = reaches_A(:,250:750);
    %this below conversion factor (1cm/100JS au) was made by measuring change in JS position realtive to a ruler such that an amplitude of 100 was equal to 1 cm
    %this conversion should be done per joystick rig.
    % 1cm/100JS au * 1000ms/1sec * 1 JS au/1ms
    
    vel = diff(R,1,2)./diff(1:size(R,2))*10;%cm/s
    acc = diff(vel,1,2)./diff(1:size(vel,2))*1000;%cm/s^2
    PVA = [{R} {mean(R)} {std(R)/sqrt(size(R,1))} {vel} {mean(vel)} {std(vel)/sqrt(size(vel,1))} {acc} {mean(acc)} {std(acc)/sqrt(size(acc,1))} ];
    
    %pos
    m = PVA{2};sem = PVA{3};
    ub = m + sem; lb = m-sem;
    x = 1:length(ub);
    X = [x, fliplr(x)]; Y = [ub, fliplr(lb)];
    figure;
    plot(m); hold on
    fill(X, Y, 'b', 'facealpha', .25, 'edgealpha', .25, 'edgecolor', 'b'); hold on
    
    %vel
    m = PVA{5};sem = PVA{6};
    ub = m + sem; lb = m-sem;
    x = 1:length(ub);
    X = [x, fliplr(x)]; Y = [ub, fliplr(lb)];
    figure;
    plot(m); hold on
    fill(X, Y, 'b', 'facealpha', .25, 'edgealpha', .25, 'edgecolor', 'b'); hold on
    
    %acc
    m = PVA{8};sem = PVA{9};
    ub = m + sem; lb = m-sem;
    x = 1:length(ub);
    X = [x, fliplr(x)]; Y = [ub, fliplr(lb)];
    figure;
    plot(m); hold on
    fill(X, Y, 'b', 'facealpha', .25, 'edgealpha', .25, 'edgecolor', 'b'); hold on
    
    
    V = PVA{1,4}';
    figure(F); subplot(2,2,3); %plot histogram of reach velocity on summary figure
    try
        hist(max(V),1:2:max(max(V)));
        mMaxVel = mean(max(V));
        vline(mMaxVel)
        ttt = sprintf('Max Velocity per Reach Histogram\nmean = %.2f', mMaxVel);
        title(ttt)
        xlabel('cm/sec');
    catch
        ttt = sprintf('No reaches');
        title(ttt)
    end
    
    %%
    %get rewarded reach times
    data = DATA{i+sz,4};
    RewReachStartTimes = zeros(nTrials,1);
    ct = 1;
    for j = 2:size(data,1)
        if data(j,2) == 1 && data(j-1,2) == 0
            RewReachStartTimes(ct) = data(j,1); ct = ct +1;
        end
    end
    RewReachStartTimes = RewReachStartTimes - data(1); %accounting for leading nans from undecimated data
    
    DATA{sz+i,17} = RewReachStartTimes;
    
    %%
    
    X = DATA{i+sz,4}(:,4)- DATA{i+sz,4}(:,7); Y = DATA{i+sz,4}(:,5) - DATA{i+sz,4}(:,8);
    figure(F); subplot(2,2,1);  plot(X,Y); %plot X by Y on summary figure
    xlabel('x position'); ylabel('y position');
    title('X and Y Baselined JS Position')
    %%
    nReaches = size(R,1);
    B = [TrialCt nReaches];
    figure(F); subplot(2,2,4); %plot all nreach vs ntrials on summary figure
    bar(B);
    text(1,B(1)*1.1, sprintf('%d',TrialCt), 'horizontalalignment' ,'center')
    text(2,ceil(B(2)*1.1), sprintf('%d',nReaches), 'horizontalalignment' ,'center')
    xticklabels({'nTrials', 'nReaches'});
    title('Reach and Trial Number')
    
    
    txt = sprintf('%s %s', MouseName, Date);
    ha = axes('Position',[0 0 1 1],'Xlim',[0 1],'Ylim',[0 1],'Box','off','Visible','off','Units','normalized', 'clipping' , 'off');
    text(0.5, 0.98, txt,'fontsize',13,'HorizontalAlignment','center','VerticalAlignment', 'bottom');
    hold on
    
    savefig(F, [Database filesep MouseName filesep DATA{i+sz,1} '_DATA.fig']) %save summary figure
    %%
    %ITI
    ITIs = zeros(TrialCt,1);ct = 1;
    for j = 2:size(data,1)
        if data(j,2) == 1 && data(j-1,2) < 1
            ITIs(ct,1) = data(j,1);
            ct = ct +1;
        end
    end
    ITIs = diff(ITIs);
    if ~isempty(ITIs)
        figure; bar(ITIs)
        tlt = sprintf('%s\nInter Trial Interval',filenames{i+sz});
        ylabel('time (ms)'); xlabel('Trial');
        title(tlt);
        
        figure; hist(ITIs,2000:50:max(ITIs))
        tlt = sprintf('%s\nInter Trial Interval Histogram',filenames{i+sz});
        xlabel('time (ms)');
        title(tlt);
        
    end
    
    %%
    %save data
    ind = ~cellfun(@isempty, DATA(:,2));
    [~,ii] = sort(datenum(strcat(DATA(ind,2)))); %sort by time
    DATA = DATA(ii,:);
    
    mn = sprintf('%s_DATA.mat', MouseName ); %file name = MouseName_DATA.mat
    pth = [Database filesep MouseName filesep mn];
    
    save(pth , 'DATA','-v7.3');
    hands(i) = F; %add summary figure to hands variable 
end
%%
%extra plotting

%trials per min in 5 min windows
figure; r = numSubplots(size(DATA,1)); %https://www.mathworks.com/matlabcentral/fileexchange/26310-numsubplots-neatly-arrange-subplots
for j = 1:size(DATA,1)
    data = DATA{j,5};
    mn = [0; find(mod(data(:,1),300000)==0)];
    sesh = zeros(size(mn,1)-1,1);
    for k = 2:size(mn,1)
        ems = data(mn(k-1)+1:mn(k),2)==3;
        [~,trials] = bwlabel(ems); 
        sesh(k-1) = trials/5;
    end
    DATA{j,15} = sesh;
    subplot(r(1),r(2),j)
    bar(sesh);
    xticklabels((mn(2:end)/1000)/60)
    if j>25
        xlabel('mins')
    end
    if mod(j,5) == 1
        ylabel('trials')
    end
    title(DATA{j,2});
end
txt = sprintf('%s\nTrials per 5 min windows', MouseName);
ha = axes('Position',[0 0 1 1],'Xlim',[0 1],'Ylim',[0 1],'Box','off','Visible','off','Units','normalized', 'clipping' , 'off');
text(0.5, 0.95, txt,'fontsize',13,'HorizontalAlignment','center','VerticalAlignment', 'bottom');
hold on

%%
%trials per min by session
figure; bar([DATA{:,6}])
xticks(1:size(DATA,1))
xticklabels(DATA(:,2));
xtickangle(45)
ylabel('Trials per min')
xlabel('Session')
tlt = sprintf('%s\ntrials per min', MouseName);
title(tlt)

%%
for i = 1:nfiles; figure(hands(i)); end %pull up summary figures