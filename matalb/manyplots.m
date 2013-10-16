function manyplots

%создание основного окна
figure('Color','b',...
       'Menubar','none',...
       'Name', 'ManyPlots',...
       'NumberTitle','off')
   
   for i = 1 : 4
    for j = 1 : 5
        h(i, j) = subplot(4, 5, 5*(i-1)+j); 
        %subplot разбивает окно по вертикали на 4 окна, по в горизонтали на
        %5, последний аргумент определяет номер окна в котором буде plot
        % h массив записывающий ссылки на заданные оси в подокнах
       plot(rand(5,1), rand(5,1), 'Color', rand(1,3));
    end
   end
   
   %связываем ButtonDownFcn с функцией mouse_click, причем в первом
   %аргументе массив указателей на оси, т.е нажатие на пару из них будет
   %обрабатываться одной функцией mouse_click
   set(h,'ButtonDownFcn', @mouse_click);
   
    function mouse_click(scr,evt)
        %scr - указатель на объект обрабатываемый в данный момент событием
        %ButtonDownFcn, evt структура, обязательно нужна, но хрен знает для
        %чего, в данном случае струтура пустая и правильней поставить ~ 
        
        %создаем новое окно
        hNewF = figure('Color', 'w',...
    'MenuBar', 'none',...
    'Position', [200   200   400   320],...
    'NumberTitle', 'off');
        
       %копируем объект с указателем scr, т.е на который нажали в новое
       %окно hNewF
         hNewA = copyobj(scr, hNewF);
         
         % изменение размеров и положения скопированных осей
           set(hNewA, 'Position', [0.13 0.11 0.775 0.815])
    end

   
end   

