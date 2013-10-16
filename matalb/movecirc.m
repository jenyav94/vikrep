function movecirc
  %создание окна
  figure('Color','w')
  
  %создание осей
  axes('XLim',[-1 1],'YLim',[-1 1],'Box','on');
  
  %создание трех маркеров различных цветов
  hM(1)=line(0.5, 0.5, 'Marker','o', 'MarkerSize', 20,... 
     'MarkerFaceColor', 'r', 'MarkerEdgeColor', 'g'); 
  hM(2)=line(-0.5, -0.5, 'Marker','o', 'MarkerSize', 20,... 
    'MarkerFaceColor', 'y', 'MarkerEdgeColor', 'b'); 
  hM(3)=line(0, 0, 'Marker','o', 'MarkerSize', 20,... 
    'MarkerFaceColor', 'c', 'MarkerEdgeColor', 'm');

 %определяем обработку события для каждого маркера
   for k=1:3
       set(hM(k),'ButtonDownFcn',{@MarkerButtonDownFcn,hM});
   end
     
    function MarkerButtonDownFcn(scr,eventdata,hM)
        %подфункция обработки события нажатия кнопокой мыши на маркере
        
        %определяем тип нажатия кнопки
               key = get(gcf, 'SelectionType');  
               
              if isequal(key, 'alt')
                    % если щелчок с Ctrl, то дублируем маркер
                    hM = copyobj(hM, gca);
                    
                   % назначем подфункцию обработки его события ButtonDownFcn
                   set(hM, 'ButtonDownFcn', {@MarkerButtonDownFcn, hM})
              end
              
                 if isequal(key, 'open')
                  % если двойной щелчок, то удаляем маркер
                    delete(hM)
                     % и завершаем работу подфункции
                     return
                 end
        
        %определяем подфункции для обработки событий WindowButtonMotionFcn
        %и WindowButtonUpFcn
        %gcbo - возвращает ссылку на графический объект чей callback обрабатывается 
        %так как в MouseMoving будет дополнительный аргумент, в котором
        %будет ссылка на двигаемый маркер
        set(gcf,'WindowButtonMotionFcn',{@MouseMoving,gcbo});
        set(gcf,'WindowButtonUpFcn',@ButtonUp);
        
        function MouseMoving(scr,eventdata,hM)
            %hM указатель на текущий двигуемый маркер
            
            % получаем координаты текущей точки осей
                  C = get(gca, 'CurrentPoint'); 
                  x = C(1,1);
                  y = C(1,2);
                  
                  % получаем пределы осей
                   xlim = get(gca, 'XLim');
                   ylim = get(gca, 'YLim');
                   
                   % в inaxes 1, если не вышли за оси, иначе - 0
                   inaxes = xlim(1)< x  & xlim(2) > x & ...
                            ylim(1)< y  & ylim(2) > y;
                        
                  if inaxes
                     % если находимся в пределах осей,
                     % то изменяем координаты маркера 
                  set(hM,'XData', x, 'YData', y);
                  end
          
        
        end
        
            function ButtonUp(src, eventdata) 
              % Подфункция для события WindowButtonUpFcn 
              % Когда отпустили кнопку мыши, графическое окно должно перестать реагировать на движение мыши 
                 set(gcf,'WindowButtonMotionFcn', '') 
                 set(gcf,'WindowButtonUpFcn', '')
            end
  
    end
end

