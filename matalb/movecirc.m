function movecirc
  %�������� ����
  figure('Color','w')
  
  %�������� ����
  axes('XLim',[-1 1],'YLim',[-1 1],'Box','on');
  
  %�������� ���� �������� ��������� ������
  hM(1)=line(0.5, 0.5, 'Marker','o', 'MarkerSize', 20,... 
     'MarkerFaceColor', 'r', 'MarkerEdgeColor', 'g'); 
  hM(2)=line(-0.5, -0.5, 'Marker','o', 'MarkerSize', 20,... 
    'MarkerFaceColor', 'y', 'MarkerEdgeColor', 'b'); 
  hM(3)=line(0, 0, 'Marker','o', 'MarkerSize', 20,... 
    'MarkerFaceColor', 'c', 'MarkerEdgeColor', 'm');

 %���������� ��������� ������� ��� ������� �������
   for k=1:3
       set(hM(k),'ButtonDownFcn',{@MarkerButtonDownFcn,hM});
   end
     
    function MarkerButtonDownFcn(scr,eventdata,hM)
        %���������� ��������� ������� ������� �������� ���� �� �������
        
        %���������� ��� ������� ������
               key = get(gcf, 'SelectionType');  
               
              if isequal(key, 'alt')
                    % ���� ������ � Ctrl, �� ��������� ������
                    hM = copyobj(hM, gca);
                    
                   % �������� ���������� ��������� ��� ������� ButtonDownFcn
                   set(hM, 'ButtonDownFcn', {@MarkerButtonDownFcn, hM})
              end
              
                 if isequal(key, 'open')
                  % ���� ������� ������, �� ������� ������
                    delete(hM)
                     % � ��������� ������ ����������
                     return
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
                   
                   % � inaxes 1, ���� �� ����� �� ���, ����� - 0
                   inaxes = xlim(1)< x  & xlim(2) > x & ...
                            ylim(1)< y  & ylim(2) > y;
                        
                  if inaxes
                     % ���� ��������� � �������� ����,
                     % �� �������� ���������� ������� 
                  set(hM,'XData', x, 'YData', y);
                  end
          
        
        end
        
            function ButtonUp(src, eventdata) 
              % ���������� ��� ������� WindowButtonUpFcn 
              % ����� ��������� ������ ����, ����������� ���� ������ ��������� ����������� �� �������� ���� 
                 set(gcf,'WindowButtonMotionFcn', '') 
                 set(gcf,'WindowButtonUpFcn', '')
            end
  
    end
end

