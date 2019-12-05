%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% This function takes in mouse name as the primary argument and will save the file with the current time and date.
% The function asks the user to confirm the data was just collected otherwise asks for the time when the data was collected.
% The data will be saved to all paths provided within a MouseName folder. If there is no folder, it will be created. 
% A variable argument of “mm/dd/yy hh:mm:ss” can be added to use the given time and date to name the file. 
% The file is deleted following copying.
%
% MAN
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [] = SavemicroSDData(MouseName,varargin)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UPDATE BELOW
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%set directory paths where you want the data to be saved
DirNames = {'Refurb', 'NDrive'};
Directory = 'C:\Users\Yttri Lab\Documents\MATLAB\DATA'; %local
NDirectory = 'N:\yttri-lab\Data Transfer\241_SDData'; %server
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UPDATE ABOVE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

locs = {Directory NDirectory};

SDdata = 'E:\'; %set this to match your drive for the SD card reader

d = dir(SDdata); if isempty(d); fprintf('\nThere is no SD card inserted.\nThe function has been terminated...\n\n'); beep; return; end

if nargin == 2
    s = varargin;
    if ~isempty(strfind(s,'-'))
        DATE = datestr(s);
        DATE(strfind(DATE,':')) = '_';
    else
        DATE = datestr(now);
        DATE(strfind(DATE,':')) = '_';
    end
else
    fprintf('\n**************************************')
    fprintf('\n**************************************')
    fprintf('\nIf you did not just collect this data type the date and time in the following format:\n\nmm/dd/yy hh:mm:ss\nbest guess on time...\n\n...else hit enter\n')
    fprintf('**************************************\n')
    fprintf('**************************************\n\n')
    s = input('','s');
    if contains(s,'/')
        fprintf('\nRunning fx with %s.\n\n',s)
        SavemicroSDData(MouseName,s);
        return;
    end
    switch s
        case 'Quit'
            return
        case ''
        otherwise
            fprintf('\nI dont know what you said.\n\nTry again, the program has been restarted\n\n')
            SavemicroSDData(MouseName)
            return
    end
    DATE = datestr(now);
    DATE(strfind(DATE,':')) = '_';
end
file = dir([SDdata filesep '*.csv']);

try
    file = file.name;
catch
    fprintf('\nThere are no CSV files on this card\nThe function has been terminated...\n\n')
    return
end

for i = 1:size(locs,2)
    DestinationDir = locs{i};
    destination  = [DestinationDir filesep MouseName];
    
    File = [[MouseName '_' DATE] '.csv'];
    a = dir(destination);
    if isempty(a)
        try
            mkdir(destination) 
        catch, 'Error using mkdir';
            fprintf('\nYou are not connected to the server.\nConnect to the server.\n\n')
            beep
            return
        end
        copyfile([SDdata file],[destination filesep File])
    else
        copyfile([SDdata file],[destination filesep File])
    end
    fprintf('\nSaved %s to %s as %s.\n\n', file, DirNames{i}, File)
end
%%
delete([SDdata file])
fprintf('\n%s has been deleted from the SD card.\n\n', file)
