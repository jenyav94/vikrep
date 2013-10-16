function manyplots

%�������� ��������� ����
figure('Color','b',...
       'Menubar','none',...
       'Name', 'ManyPlots',...
       'NumberTitle','off')
   
   for i = 1 : 4
    for j = 1 : 5
        h(i, j) = subplot(4, 5, 5*(i-1)+j); 
        %subplot ��������� ���� �� ��������� �� 4 ����, �� � ����������� ��
        %5, ��������� �������� ���������� ����� ���� � ������� ���� plot
        % h ������ ������������ ������ �� �������� ��� � ��������
       plot(rand(5,1), rand(5,1), 'Color', rand(1,3));
    end
   end
   
   %��������� ButtonDownFcn � �������� mouse_click, ������ � ������
   %��������� ������ ���������� �� ���, �.� ������� �� ���� �� ��� �����
   %�������������� ����� �������� mouse_click
   set(h,'ButtonDownFcn', @mouse_click);
   
    function mouse_click(scr,evt)
        %scr - ��������� �� ������ �������������� � ������ ������ ��������
        %ButtonDownFcn, evt ���������, ����������� �����, �� ���� ����� ���
        %����, � ������ ������ �������� ������ � ���������� ��������� ~ 
        
        %������� ����� ����
        hNewF = figure('Color', 'w',...
    'MenuBar', 'none',...
    'Position', [200   200   400   320],...
    'NumberTitle', 'off');
        
       %�������� ������ � ���������� scr, �.� �� ������� ������ � �����
       %���� hNewF
         hNewA = copyobj(scr, hNewF);
         
         % ��������� �������� � ��������� ������������� ����
           set(hNewA, 'Position', [0.13 0.11 0.775 0.815])
    end

   
end   

