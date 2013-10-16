function plotcurve

%создание основного графического окна
figure('Color','w',...
       'Name','DrowingGraph',...
       'NumberTitle','off')

%создание осей
axes('XLim',[-1 1],'YLim',[-1 1],'Box','on','ButtonDownFcn',@BtnDown)
% Box - ограничение поля сверху и справа + привязка обработки нажатия
% кнопики мыши  в подфункции BtnDown

%включение режима сохраниения графика, те при добавлении новых компанентов
%оси не обнавляются
hold on

%создаем структуру Line
Line.X=[]; %абсцисы точек
Line.Y=[]; %ординаты точек
Line.h=[]; %указатели на линию
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
               %selectiontype выдает 'normal' если обычный левый щелчок и 'alt'
               %если левый клик + ctrl
               if isequal(key,'normal')
                   %те обычный щелчок
                   %всем полям структуры присваеваем путые массивы
                   Line=guidata(gcf);
                   Line.X=[];
                   Line.Y=[];
                   Line.h=[];
               else
                   %добавляем координаты новой точки
                   Line.X=[Line.X x]; %так добавляется значение в конец массива
                   Line.Y=[Line.Y y];
                   
                   %рисуем маркер
                   line(x, y, 'Marker','o', 'MarkerSize', 10,... 
                    'MarkerFaceColor', 'c', 'MarkerEdgeColor', 'm');
                %4 аргумент-форма,
                
                % если точек больше одной, то строим сплайн
        if length(Line.X) > 1
            t = 1 : length(Line.X); % узлы сплайна
            tt = 1: 0.1: t(end);  % промежуточные точки для вычисления в них значения сплайна
            sp = spline(t, [Line.X; Line.Y], tt);
            % если есть линия, то удаляем ее
            if ~isempty(Line.h)
                delete(Line.h)
            end
            % рисуем новую линию, 1й аргумент plot-первый столбец sp(те
            % x-ы), второй аргумент - y (2 столбец sp)
            Line.h = plot(sp(1, :), sp(2, :), 'black');
            set(Line.h, 'LineWidth', 2)
            %при нажатии на график также используется функция BtnDown
            set(Line.h, 'ButtonDownFcn', @BtnDown)
        end
               end
    % сохраняем структуру данных
    guidata(gcf, Line)      
                   
                   
           
         
           end



    end
end

