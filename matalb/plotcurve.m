function plotcurve

%�������� ��������� ������������ ����
figure('Color','w',...
       'Name','DrowingGraph',...
       'NumberTitle','off')

%�������� ����
axes('XLim',[-1 1],'YLim',[-1 1],'Box','on','ButtonDownFcn',@BtnDown)
% Box - ����������� ���� ������ � ������ + �������� ��������� �������
% ������� ����  � ���������� BtnDown

%��������� ������ ����������� �������, �� ��� ���������� ����� �����������
%��� �� �����������
hold on

%������� ��������� Line
Line.X=[]; %������� �����
Line.Y=[]; %�������� �����
Line.h=[]; %��������� �� �����
guidata(gcf,Line); %��������� ��������� Line � �������� ����������� ����

    function BtnDown(scr,eventdata)
        %������� ��������� ������ ����� �� ����
        
        %�������� ���������� ������� �����
        C=get(gca,'CurrentPoint');
        % gca-current axes handle(������ �� ������� ��� � ������� ����)
        x=C(1,1);
        y=C(1,2);
        
        %������ ������� ����
        xlim=get(gca,'XLim');
        ylim=get(gca,'YLim');
        
        %��������� ��� �� ������ � �������� ����
        inaxes=xlim(1)<x && xlim(2)>x &&...
               ylim(1)<y && ylim(2)>y;
           
           if inaxes
               %��������� ��� ������
               key=get(gcf,'SelectionType');
               %gca-current figure handle(������ �� ������� ����)
               %selectiontype ������ 'normal' ���� ������� ����� ������ � 'alt'
               %���� ����� ���� + ctrl
               if isequal(key,'normal')
                   %�� ������� ������
                   %���� ����� ��������� ����������� ����� �������
                   Line=guidata(gcf);
                   Line.X=[];
                   Line.Y=[];
                   Line.h=[];
               else
                   %��������� ���������� ����� �����
                   Line.X=[Line.X x]; %��� ����������� �������� � ����� �������
                   Line.Y=[Line.Y y];
                   
                   %������ ������
                   line(x, y, 'Marker','o', 'MarkerSize', 10,... 
                    'MarkerFaceColor', 'c', 'MarkerEdgeColor', 'm');
                %4 ��������-�����,
                
                % ���� ����� ������ �����, �� ������ ������
        if length(Line.X) > 1
            t = 1 : length(Line.X); % ���� �������
            tt = 1: 0.1: t(end);  % ������������� ����� ��� ���������� � ��� �������� �������
            sp = spline(t, [Line.X; Line.Y], tt);
            % ���� ���� �����, �� ������� ��
            if ~isempty(Line.h)
                delete(Line.h)
            end
            % ������ ����� �����, 1� �������� plot-������ ������� sp(��
            % x-�), ������ �������� - y (2 ������� sp)
            Line.h = plot(sp(1, :), sp(2, :), 'black');
            set(Line.h, 'LineWidth', 2)
            %��� ������� �� ������ ����� ������������ ������� BtnDown
            set(Line.h, 'ButtonDownFcn', @BtnDown)
        end
               end
    % ��������� ��������� ������
    guidata(gcf, Line)      
                   
                   
           
         
           end



    end
end

