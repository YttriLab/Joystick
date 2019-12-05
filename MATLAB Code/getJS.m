function [out] = getJS(filepth)
% takes in path of csv file and returns cell of {filename Date starttime data}
% data =  [time EM TrialCt X Y pos baseX baseY SolOpenDuration DelaytoRew ITI Threshold]
% this fuction converts the csv to a matrix as data
%


% This function was build because we originally had issues with serial
% communiation and saving data using processing resutling in uneven columns
% and rows

% csvread can be used as well if you are confident of the file writing
% depending on your sample rate, the time between data points are accounted for with the undecimation code.


% MAN

[~, file, ~] = fileparts(filepth);
fid = fopen(filepth, 'r');
X = fread(fid); fclose(fid);
X = char(X');
X = textscan(X, '%s%s%s%s%s%s%s%s%s%s%s%s', 'Delimiter', ','); %need one "s%" per column in CSV file
ms = str2double(X{1});
start = find(ms==1077);
if isempty(start)
    [~, start] = min(ms);
end

mn = find(diff(ms)<0) < 20;
if isempty(mn)
    nm = 0;
else 
    nm = 1;
end

if start ~= 1 || nm
    if size(find(diff(ms)<0),1) ~=1 && start ~= 1
        mn = find(diff(ms)<0)+1;
        if mn(end) < 20
            start = mn(end) ;
        end
%         [~, start] = min(ms);
        minscutout = (ms(start)/1000)/60;
    else 
        if find(diff(ms)<0,1,'last') < 20
            start = find(diff(ms)<0,1,'last')+1;
        end
    end
           
    try
        [~,l]= findpeaks(ms,'MinPeakWidth',mean(diff(ms))*3);
        if ~isempty(l) & start ~= 1
            start = l(end)+1;
        end
    catch
        if start+1 == 1099
            start = start + 1;
        end
%         
%         d = diff(ms);
%         m = nanmean(d);
%         find(d>m-5& d<m+5,10,'first')
%         nanstd(diff(ms))
%         find(isnan(ms)) < 10
    end
    
end
minscutout = (ms(start)/1000)/60;
if start > 100
    fprintf('\n\nCUTOUT ABOUT FIRST %.2f MINS \nCHECK DATA\n\n',minscutout);
end


try
    X = [X{1}(start:end) X{2}(start:end) X{3}(start:end) X{4}(start:end) X{5}(start:end) X{6}(start:end) X{7}(start:end) X{8}(start:end) X{9}(start:end) X{10}(start:end) X{11}(start:end) X{12}(start:end)]; %need one "X{x}(start:end)" per column of CSV file
    data  = str2double(X); % [time EV TrialCt LR UD SolOpen SolOpenTime baseX baseY met SolOpenDuration DelaytoRew ITI Threshold adaptivect clockstart]
catch
    %found newline at end of coulmn 1 (ms) so dimentions inconsistent and
    %find mode of length and select each column
    fprintf('\n\nissue with number of rows\ncheck data\n\n');
    x = cell(1,size(X,2));
    for i = 1:size(X,2)
        x{i} = str2double(X{i}(start:end));
    end

    modelen = mode(cellfun(@length,x));
    ind=cellfun(@length,x) ~= modelen;
    data = zeros(modelen,size(X,2));
    for i = 1:size(X,2)
        if ind(i)
            data(:,i) = str2double(X{i}(start:end-1));
        else
            data(:,i) = str2double(X{i}(start:end));
        end
    end
    
end

nanrows = find(sum(isnan(data),2));
if size(nanrows,1)>0
    
    data(nanrows,:) = [];
    fprintf('\n\nCut out %d nanrows.\n\n', size(nanrows,1));
    if data(1,1) > 2000
        [~,start] = min(data(:,1));
        if start < 20
            data = data(start:end,:);
        end
    end
    if data(1,1) > 2000
        fprintf('\n\nCUT OUT OVER 2000 MS\nCHECK DATA\n\n');
    end
end
[n,bin] = histc(data(:,1),unique(data(:,1)));
multiples = find(n>1);
if size(multiples,1)~=0
    
    ind = find(ismember(bin, multiples));
    vals = unique(data(ind,1));
    
    for i =1:size(multiples,1)
        repeats = sum(data(:,1) == vals(i));
        for j = 1:repeats-1
            data(find(data(:,1) == vals(i),1),:) = [];
        end
    end
    fprintf('\n\nremoved multiple nonunique time values\n\n')
    beep
end


msdiff = diff(data(:,1));
m = mean(msdiff);
ind = [msdiff>m*3;0];
if sum(ind)>0
    data(logical(ind),:)=[];
    fprintf('\n\ntiming issues\ncheck data\n\ncut out %d rows\n\n', sum(ind))
end

tlt = file;
if contains(tlt,'-')
    ind = strfind(tlt, '_');
    endtime =  tlt(ind(2)-2: end); endtime(endtime == '_') = ':';
    tlt(ind(1)) = ' ';  tlt(ind(2)) = ":"; tlt(ind(3)) = ":";
    Date = datestr(tlt(4: end));
else
    ind = strfind(tlt, '_');
    tlt(ind(1)) = ' '; tlt(ind(4)) = ' '; tlt(ind(5)) = ':'; tlt(ind(end)) = ':';
    Date = datestr(tlt(4: end));
    endtime =  tlt(ind(4)+1:end);
end


out  = {file Date endtime data};
