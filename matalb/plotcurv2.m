function plotcurv2
%�������� ��������� ������������ ����
figure('Color','w',...
       'Name','DrowingGraph',...
       'NumberTitle','off','KeyReleaseFcn',@KeyboardBtnDown)

%�������� ����
axes('XLim',[-1 1],'YLim',[-1 1],'Box','on','ButtonDownFcn',@BtnDown)
% Box - ����������� ���� ������ � ������ + �������� ��������� �������
% ������� ����  � ���������� BtnDown

%��������� ������ ����������� �������, �� ��� ���������� ����� �����������
%��� �� �����������
hold on

%������� ��������� Line
Line.h=[]; %��������� �� �����
Line.hM=[]; %��������� �� ����������� �������
Line.hS=[]; %��������� �� ��������� �������
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
               %selectiontype ������ 'open' ���� ������� ����� ������ � 'alt'
               %���� ����� ���� + ctrl
              
               if isequal(key,'alt')
                   %��������� ���������� ����� �����
                   
                   %��������� �������� �� ������ � ����� ������� � �������
                   %�� �����
                   if ~isempty(Line.hM)
                     
                        xM=get(Line.hM(1),'XData');
                        i=2;
                        while((i-1)<length(Line.hM) && x>xM)
                           xM=get(Line.hM(i),'XData');
                           i=i+1;
                        end
                         if(x<=xM)
                             Line.hM=[Line.hM(1:(i-2)) line(x, y, 'Marker','o', 'MarkerSize', 10,... 
                             'MarkerFaceColor', 'c', 'MarkerEdgeColor', 'm') Line.hM((i-1):(end)) ];
                
                             %��������� ������� ������� �� ������ � �������� MarkerButtonDownFcn 
                              set(Line.hM(i-1),'ButtonDownFcn',@MarkerButtonDownFcn);
                         elseif(xM==Line.hM(end) || xM<=x)
                             Line.hM=[Line.hM line(x, y, 'Marker','o', 'MarkerSize', 10,... 
                             'MarkerFaceColor', 'c', 'MarkerEdgeColor', 'm')];
                              %4 �������� ������� line-�����
                              %��������� ������� ������� �� ������ � �������� MarkerButtonDownFcn 
                               set(Line.hM(end),'ButtonDownFcn',@MarkerButtonDownFcn);
                         end
                         
                       
                       else
                        Line.hM=[Line.hM line(x, y, 'Marker','o', 'MarkerSize', 10,... 
                        'MarkerFaceColor', 'c', 'MarkerEdgeColor', 'm')];
                    
                         %��������� ������� ������� �� ������ � �������� MarkerButtonDownFcn
                         set(Line.hM(end),'ButtonDownFcn',@MarkerButtonDownFcn);
                   end
                
                 %�������������� ������ ���� � ������� ���� ������� 
                   replot;
               end
           end
           
            % ��������� ��������� ������
             guidata(gcf, Line)
    end             
        
           
         
                   
     function MarkerButtonDownFcn(scr,eventdata)
        %���������� ��������� ������� ������� �������� ���� �� �������
         key=get(gcf,'SelectionType');
         
         if isequal(key,'normal')
             
             %�������� ����� �������� ������� � �������
             for  j=1:length(Line.hM)
                 if isequal(scr,Line.hM(j))
                     curMarker=j;
                 end
             end
         end
         
          if isequal(key,'open')
              %�������� ��������
                if length(Line.hM)<=2
                    delete(Line.h);
                     Line.h=[];
                end
                      
                 for j=1:length(Line.hM)
                             if isequal(scr,Line.hM(j))
                                 xMarker=get(Line.hM(j),'XData');
                                 yMarker=get(Line.hM(j),'YData');
                                 Line.hS=[Line.hS xMarker yMarker]
                                  delete(Line.hM(j));
                                  Line.hM(j:j)=[];
                                  break
                              end
                 end
              % ���� ����� ������ �����, �� ������ ������
             replot;
          end

        %���������� ���������� ��� ��������� ������� WindowButtonMotionFcn
        %� WindowButtonUpFcn
        %gcbo - ���������� ������ �� ����������� ������ ��� callback �������������� 
        %��� ��� � MouseMoving ����� �������������� ��������, � �������
        %����� ������ �� ��������� ������
        set(gcf,'WindowButtonMotionFcn',{@MouseMoving,gcbo});
        set(gcf,'WindowButtonUpFcn',@ButtonUp);
        
        function MouseMoving(scr,eventdata,hM)
            %hM ��������� �� ������� ��������� ������
            
            % �������� ���������� ������� ����� ����
                  C = get(gca, 'CurrentPoint'); 
                  x = C(1,1);
                  y = C(1,2);
                  
                  % �������� ������� ����
                   xlim = get(gca, 'XLim');
                   ylim = get(gca, 'YLim');
                   
                   if curMarker==1
                       %�������� x ���������� ���������� �������, ����
                       %������� ������ ������
                       xNext=get(Line.hM(curMarker+1),'XData');
                   elseif curMarker==length(Line.hM)
                        %�������� x ���������� ���������� �������, ����
                        %������� ������ ���������
                       xPrev=get(Line.hM(curMarker-1),'XData');
                   else
                   %�������� x ���������� ���������� � ���������� �������
                   xPrev=get(Line.hM(curMarker-1),'XData');
                   xNext=get(Line.hM(curMarker+1),'XData');
                   end
                   
                   % � inaxes 1, ���� �� ����� �� ���, ����� - 0
                   inaxes = xlim(1)< x  & xlim(2) > x & ...
                            ylim(1)< y  & ylim(2) > y;
                        
                        if curMarker==1
                            %1 ���� ������ ������ �� ������� �� ���������
                            inSelfArea=x<xNext;
                        elseif curMarker==length(Line.hM)
                            %1 ���� ��������� ������ �� ������� ��
                            %����������
                            inSelfArea=x>xPrev;
                        else
                        % � inSelfArea 1, ���� ����� ���������� �������� � ���������
                        inSelfArea= xPrev<x & xNext>x ;
                        end
                        
                  if inaxes && inSelfArea
                     % ���� ��������� � �������� ����, � �� ����������
                     % �������� �������
                     % �� �������� ���������� ������� 
                  set(hM,'XData', x, 'YData', y);
                  
                  end
           %�������������� ������       
           replot;
        
        end
        
            function ButtonUp(src, eventdata) 
              % ���������� ��� ������� WindowButtonUpFcn 
              % ����� ��������� ������ ����, ����������� ���� ������ ��������� ����������� �� �������� ���� 
                 set(gcf,'WindowButtonMotionFcn', '') 
                 set(gcf,'WindowButtonUpFcn', '')
            end
     end
 
    function KeyboardBtnDown(src,eventdata)
      
        if strcmp(eventdata.Modifier,'control') & strcmp(eventdata.Key,'z')
           
             if ~isempty(Line.hS)
                     
                     y=Line.hS(end);
                     Line.hS=Line.hS(1:end-1);
                     x=Line.hS(end);
                     Line.hS=Line.hS(1:end-1);
                     
                        xM=get(Line.hM(1),'XData');
                        i=2;
                        while((i-1)<length(Line.hM) && x>xM)
                           xM=get(Line.hM(i),'XData');
                           i=i+1;
                        end
                         if(x<=xM)
                              Line.hM=[Line.hM(1:(i-2)) line(x, y, 'Marker','o', 'MarkerSize', 10,... 
                             'MarkerFaceColor', 'c', 'MarkerEdgeColor', 'm') Line.hM((i-1):(end)) ];
                
                             %��������� ������� ������� �� ������ � �������� MarkerButtonDownFcn 
                              set(Line.hM(i-1),'ButtonDownFcn',@MarkerButtonDownFcn);
                          elseif(xM==Line.hM(end) || xM<=x)
                             Line.hM=[Line.hM line(x, y, 'Marker','o', 'MarkerSize', 10,... 
                             'MarkerFaceColor', 'c', 'MarkerEdgeColor', 'm')];
                              %4 �������� ������� line-�����
                              %��������� ������� ������� �� ������ � �������� MarkerButtonDownFcn 
                               set(Line.hM(end),'ButtonDownFcn',@MarkerButtonDownFcn);
                         end
                         
                         replot;
             end
        end
        
    end

    
      function replot
          %�������������� ������ �� ����������� �������� � ��������, ����
          %������ �� ����
         if length(Line.hM) > 1
            t = 1 : length(Line.hM); % ���� �������
            tt = 1: 0.1: t(end);  % ������������� ����� ��� ���������� � ��� �������� �������
            for i=1:length(Line.hM)
             %�������� ���������� ��������
            X(i)=get(Line.hM(i),'XData');
            Y(i)=get(Line.hM(i),'YData');
            end
            sp = spline(t, [X;Y], tt);
            % ���� ���� �����, �� ������� ��
            if ~isempty(Line.h)
                delete(Line.h)
            end
            % ������ ����� �����, 1� �������� plot-������ ������� sp(��
            % x-�), ������ �������� - y (2 ������� sp)
            Line.h = plot(sp(1, :), sp(2, :), 'black');
            set(Line.h, 'LineWidth', 2)
           set(Line.h,'ButtonDownFcn',@BtnDown);
          
            %���������� ������� �� ����� ����
             hA=get(Line.h,'Parent');
             set(hA,'Children',[Line.hM,Line.h]);
         end
    end
end


   

     
                  


