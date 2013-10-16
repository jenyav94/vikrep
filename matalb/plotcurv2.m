function plotcurv2
%создание основного графического окна
figure('Color','w',...
       'Name','DrowingGraph',...
       'NumberTitle','off','KeyReleaseFcn',@KeyboardBtnDown)

%создание осей
axes('XLim',[-1 1],'YLim',[-1 1],'Box','on','ButtonDownFcn',@BtnDown)
% Box - ограничение поля сверху и справа + привязка обработки нажатия
% кнопики мыши  в подфункции BtnDown

%включение режима сохраниения графика, те при добавлении новых компанентов
%оси не обнавляются
hold on

%создаем структуру Line
Line.h=[]; %указатели на линию
Line.hM=[]; %указатели на действующие маркеры
Line.hS=[]; %указатели на удаленный маркеры
guidata(gcf,Line); %сохраняем структуру Line в основное графическое окно

    function BtnDown(scr,eventdata)
        %функция обработки щелчка мышки по осям
        
        %получаем координаты текущей точки
        C=get(gca,'CurrentPoint');
        % gca-current axes handle(ссылка на текущие оси в текущем окне)
        x=C(1,1);
        y=C(1,2);
        
        %узнаем пределы осей
        xlim=get(gca,'XLim');
        ylim=get(gca,'YLim');
        
        %проверяем был ли щелчек в пределах осей
        inaxes=xlim(1)<x && xlim(2)>x &&...
               ylim(1)<y && ylim(2)>y;
           
           if inaxes
               %проверяем тип щелчка
               key=get(gcf,'SelectionType');
               %gca-current figure handle(ссылка на текущее окно)
               %selectiontype выдает 'open' если двойной левый щелчок и 'alt'
               %если левый клик + ctrl
              
               if isequal(key,'alt')
                   %добавляем координаты новой точки
                   
                   %добавляем укзатель на маркер в конец массива и выводим
                   %на экран
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
                
                             %связываем событие нажатия на маркер с функцией MarkerButtonDownFcn 
                              set(Line.hM(i-1),'ButtonDownFcn',@MarkerButtonDownFcn);
                         elseif(xM==Line.hM(end) || xM<=x)
                             Line.hM=[Line.hM line(x, y, 'Marker','o', 'MarkerSize', 10,... 
                             'MarkerFaceColor', 'c', 'MarkerEdgeColor', 'm')];
                              %4 аргумент функции line-форма
                              %связываем событие нажатия на маркер с функцией MarkerButtonDownFcn 
                               set(Line.hM(end),'ButtonDownFcn',@MarkerButtonDownFcn);
                         end
                         
                       
                       else
                        Line.hM=[Line.hM line(x, y, 'Marker','o', 'MarkerSize', 10,... 
                        'MarkerFaceColor', 'c', 'MarkerEdgeColor', 'm')];
                    
                         %связываем событие нажатия на маркер с функцией MarkerButtonDownFcn
                         set(Line.hM(end),'ButtonDownFcn',@MarkerButtonDownFcn);
                   end
                
                 %перерисовываем график если в массиве есть маркеры 
                   replot;
               end
           end
           
            % сохраняем структуру данных
             guidata(gcf, Line)
    end             
        
           
         
                   
     function MarkerButtonDownFcn(scr,eventdata)
        %подфункция обработки события нажатия кнопокой мыши на маркере
         key=get(gcf,'SelectionType');
         
         if isequal(key,'normal')
             
             %получаем номер текущего маркерв в массиве
             for  j=1:length(Line.hM)
                 if isequal(scr,Line.hM(j))
                     curMarker=j;
                 end
             end
         end
         
          if isequal(key,'open')
              %удаление маркеров
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
              % если точек больше одной, то строим сплайн
             replot;
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
                   
                   if curMarker==1
                       %получаем x координаты следующего маркера, если
                       %текущий маркер первый
                       xNext=get(Line.hM(curMarker+1),'XData');
                   elseif curMarker==length(Line.hM)
                        %получаем x координаты следующего маркера, если
                        %текущий маркер последний
                       xPrev=get(Line.hM(curMarker-1),'XData');
                   else
                   %получаем x координаты преыдущего и слудующего маркера
                   xPrev=get(Line.hM(curMarker-1),'XData');
                   xNext=get(Line.hM(curMarker+1),'XData');
                   end
                   
                   % в inaxes 1, если не вышли за оси, иначе - 0
                   inaxes = xlim(1)< x  & xlim(2) > x & ...
                            ylim(1)< y  & ylim(2) > y;
                        
                        if curMarker==1
                            %1 если первый маркер не заходит за следующий
                            inSelfArea=x<xNext;
                        elseif curMarker==length(Line.hM)
                            %1 если последний маркер не заходит за
                            %пердыдущий
                            inSelfArea=x>xPrev;
                        else
                        % в inSelfArea 1, если между пердыдущим маркером и слкдующим
                        inSelfArea= xPrev<x & xNext>x ;
                        end
                        
                  if inaxes && inSelfArea
                     % если находимся в пределах осей, и не пересекаем
                     % соседнии маркеры
                     % то изменяем координаты маркера 
                  set(hM,'XData', x, 'YData', y);
                  
                  end
           %перерисовываем график       
           replot;
        
        end
        
            function ButtonUp(src, eventdata) 
              % Подфункция для события WindowButtonUpFcn 
              % Когда отпустили кнопку мыши, графическое окно должно перестать реагировать на движение мыши 
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
                
                             %связываем событие нажатия на маркер с функцией MarkerButtonDownFcn 
                              set(Line.hM(i-1),'ButtonDownFcn',@MarkerButtonDownFcn);
                          elseif(xM==Line.hM(end) || xM<=x)
                             Line.hM=[Line.hM line(x, y, 'Marker','o', 'MarkerSize', 10,... 
                             'MarkerFaceColor', 'c', 'MarkerEdgeColor', 'm')];
                              %4 аргумент функции line-форма
                              %связываем событие нажатия на маркер с функцией MarkerButtonDownFcn 
                               set(Line.hM(end),'ButtonDownFcn',@MarkerButtonDownFcn);
                         end
                         
                         replot;
             end
        end
        
    end

    
      function replot
          %перерисовывает график по координатам заданным в маркерах, если
          %массив не пуст
         if length(Line.hM) > 1
            t = 1 : length(Line.hM); % узлы сплайна
            tt = 1: 0.1: t(end);  % промежуточные точки для вычисления в них значения сплайна
            for i=1:length(Line.hM)
             %получаем координаты маркеров
            X(i)=get(Line.hM(i),'XData');
            Y(i)=get(Line.hM(i),'YData');
            end
            sp = spline(t, [X;Y], tt);
            % если есть линия, то удаляем ее
            if ~isempty(Line.h)
                delete(Line.h)
            end
            % рисуем новую линию, 1й аргумент plot-первый столбец sp(те
            % x-ы), второй аргумент - y (2 столбец sp)
            Line.h = plot(sp(1, :), sp(2, :), 'black');
            set(Line.h, 'LineWidth', 2)
           set(Line.h,'ButtonDownFcn',@BtnDown);
          
            %переставим маркеры на певый план
             hA=get(Line.h,'Parent');
             set(hA,'Children',[Line.hM,Line.h]);
         end
    end
end


   

     
                  


