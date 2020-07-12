function [nd] = unDecimate(data, columnsWanted)

% deterimntes amount of time between samples and interpolates data
% input of data must have time in the first column

% currently takes one column at a time
% can run a for loop to go through matrix on outside - see below

% [nd] = unDecimate(data, columnsWanted)
% Input: 
%   data............matrix from which data will be undicimated with first column being time
%   columnsWanted...what column you want to undecimate

% Output:
%   nd..............2 column matrix of undecimated time and undecimated data


% example for running unDecimate through columns of matrix
% tic
% data = DATA{9,4};ct = 2;
% for i = [2 6] %columns from matrix data wanted
%     nd = unDecimate(data,i);
%     if i == 2
%         D = zeros(size(nd,1),3); 
%         D(:,1:2) = nd;
%     else
%         D(:,ct) = nd(:,2);
%     end
%     ct = ct+1;
% end
% toc


%071318 - MAN 
%%

% dbstop if error
nd = zeros(data(end,1),2);

if data(1)~=0
    newData=[(1:data(1)-1)', nan(length(1:data(1))-1,1)];% data(1, [1 columnsWanted])];
else
    newData= data(1,columnsWanted);
end

sz = size(newData,1);
nd(1:sz,:) = newData;

for i = 2:size(data,1)
    addOn=data(i,1)-data(i-1,1)+1; %find time diff
    addDataMore=zeros(addOn,2);
    addDataMore(:,1) = data(i-1,1):data(i,1); %fill in time
    j=columnsWanted;
    if data(i-1,j)==data(i,j)
        addDataMore(:,2) = repmat(data(i,j),addOn,1); %reproduce same data point between two same values
    else
        addDataMore(:,2) = linspace(data(i-1,j),data(i,j),addOn); %linear interpolation
    end
    ind = find(nd(:,1) == 0,1); %find index of first zeros to place new data
    nd(ind:ind+addOn-2,:) = addDataMore(1:end-1,:);
end
nd(end,:) = [data(end,1) data(end,j)];

%%
% uncomment to plot

% figure; plot(nd(:,2),'.g'); hold on
% plot(data(:,1),data(:,6),'.m')
% plot(data(:,1),data(:,6),'k')
% legend({'interpolated data', 'old data points', 'trace from old data points'})

