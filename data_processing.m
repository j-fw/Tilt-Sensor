if ~isempty(instrfind)%resets port so that it can be opened
     fclose(instrfind);
      delete(instrfind);
end
s = serialport("COM3",9600);%opens serial port with pre determined bauderate


while(true)
    angle=zeros(10);%creates an empty vector for the values to go in
    for i=1:10
        data = readline(s);%reads data from esduino
        num=str2double(data);%converts data from string to integer
        if num==100 %the default value sent from the esduino if no data is being collected
            disp('Data Collection Disabled')
        else
            angle(i)=num
            stem(angle)
            axis([1,10,0,90]);%fixes axis on graph for readability
            xlabel('time')
            ylabel('angle')
            ytickformat('degrees')
            title('Angle Reading - Jack Wawrychuk - 400145293')
            drawnow;%forces the graph to constantly update  
        end
    end    
end

