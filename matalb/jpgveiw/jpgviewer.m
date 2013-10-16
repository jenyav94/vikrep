function jpgviewer
%получаем список файлов из текущего католога с помощью функции dir
files=dir;
%узнаем длинну списка
file_num=length(dir);
jpg_num=0;

%проходим по списку файлов  в каталоге
for k=1:file_num
    %определяем файл или подкаталог
    if (~files(k).isdir)
        %files(k) - файл, значит записываем его имя и формат
        [pathstr, name, ext] = fileparts(files(k).name);
        %проверяем является ли файл jpg
        if strcmpi(ext,'.jpg')
            %считаем солько файлов формата jpg
            jpg_num=jpg_num+1;
            %добавляем имя файла в массив имен файлов
            jpg_names{jpg_num}=[name ext];
        end
    end
end
  %проверяем есть ли файлы jpg
  if (jpg_num>0)
      %разбиение графического окна на оси
       n = ceil(sqrt(jpg_num));
       
    % создаем нулевой массив для хранения указателей на рисунки
    hIMG = zeros(1, jpg_num);
    
    % создаем графическое окно
    figure('Color', 'y',...
        'MenuBar', 'none',...
        'Name', 'jpgshow',...
        'NumberTitle', 'off') 
    
    % задаем расстояние между осями (в нормализованных единицах)
    Delta = 0.005; 
    % вычисляем ширину и высоту каждой пары осей
    Width = (1 - (n+1)*Delta)/n; 
    Height = Width; 
    
    % в циклах создаем оси в графическом окне
    for i = 1 : n
        for j = 1 : n
            
            % вычисляем координаты левого нижнего угла осей
            y = 1 - (Height+Delta)*i; 
            x = Delta + (Width+Delta)*(j-1); 
            % находим номер текущего рисунка
            im_num = (i-1)*n+j;
            
            % проверяем, не превысили ли число рисунков
            if im_num <= jpg_num
                % создаем оси
                axes('Position', [x, y, Width, Height]) 
                % читаем файл jpg
                IMG = imread(jpg_names{im_num});
                % выводим рисунок на оси и сохраняем указатель 
                % на него в массиве hIMG
                hIMG(im_num) = imshow(IMG);
                % обновляем оси, данного оператора сначала бы считались все
                % файлы, а только потом вывелись на экран, что не есть
                % хорошо
                drawnow
            end
        end
    end
    % связываем с событием ButtonDownFcn рисунка  функцию mouse_click
    set(hIMG, 'ButtonDownFcn', @mouse_click)
  end

  function mouse_click(src, evt)
       % функция обработки события ButtonDownFcn изображений 

        % создаем графическое окно
           hNewF = figure('Color', 'w',...
                          'MenuBar', 'none',...
                          'Position', [200   200   400   320],...
                          'NumberTitle', 'off');
         % получаем указатель на оси, т.е. на предка изображения
            hAxes = get(src, 'Parent');
         % копируем оси в новое графическое окно
            hNewA = copyobj(hAxes, hNewF);
          % задаем размеры и положение новых осей
               set(hNewA, 'Position', [0.13 0.11 0.775 0.815])
        % получаем указатели на потомков осей
           hC = get(hNewA, 'Children');
         % задаем их свойству ButtonDownFcn значение пустой массив
            set(hC, 'ButtonDownFcn', [])
  end
  

end

