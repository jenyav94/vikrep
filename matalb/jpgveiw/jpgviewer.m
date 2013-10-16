function jpgviewer
%�������� ������ ������ �� �������� �������� � ������� ������� dir
files=dir;
%������ ������ ������
file_num=length(dir);
jpg_num=0;

%�������� �� ������ ������  � ��������
for k=1:file_num
    %���������� ���� ��� ����������
    if (~files(k).isdir)
        %files(k) - ����, ������ ���������� ��� ��� � ������
        [pathstr, name, ext] = fileparts(files(k).name);
        %��������� �������� �� ���� jpg
        if strcmpi(ext,'.jpg')
            %������� ������ ������ ������� jpg
            jpg_num=jpg_num+1;
            %��������� ��� ����� � ������ ���� ������
            jpg_names{jpg_num}=[name ext];
        end
    end
end
  %��������� ���� �� ����� jpg
  if (jpg_num>0)
      %��������� ������������ ���� �� ���
       n = ceil(sqrt(jpg_num));
       
    % ������� ������� ������ ��� �������� ���������� �� �������
    hIMG = zeros(1, jpg_num);
    
    % ������� ����������� ����
    figure('Color', 'y',...
        'MenuBar', 'none',...
        'Name', 'jpgshow',...
        'NumberTitle', 'off') 
    
    % ������ ���������� ����� ����� (� ��������������� ��������)
    Delta = 0.005; 
    % ��������� ������ � ������ ������ ���� ����
    Width = (1 - (n+1)*Delta)/n; 
    Height = Width; 
    
    % � ������ ������� ��� � ����������� ����
    for i = 1 : n
        for j = 1 : n
            
            % ��������� ���������� ������ ������� ���� ����
            y = 1 - (Height+Delta)*i; 
            x = Delta + (Width+Delta)*(j-1); 
            % ������� ����� �������� �������
            im_num = (i-1)*n+j;
            
            % ���������, �� ��������� �� ����� ��������
            if im_num <= jpg_num
                % ������� ���
                axes('Position', [x, y, Width, Height]) 
                % ������ ���� jpg
                IMG = imread(jpg_names{im_num});
                % ������� ������� �� ��� � ��������� ��������� 
                % �� ���� � ������� hIMG
                hIMG(im_num) = imshow(IMG);
                % ��������� ���, ������� ��������� ������� �� ��������� ���
                % �����, � ������ ����� �������� �� �����, ��� �� ����
                % ������
                drawnow
            end
        end
    end
    % ��������� � �������� ButtonDownFcn �������  ������� mouse_click
    set(hIMG, 'ButtonDownFcn', @mouse_click)
  end

  function mouse_click(src, evt)
       % ������� ��������� ������� ButtonDownFcn ����������� 

        % ������� ����������� ����
           hNewF = figure('Color', 'w',...
                          'MenuBar', 'none',...
                          'Position', [200   200   400   320],...
                          'NumberTitle', 'off');
         % �������� ��������� �� ���, �.�. �� ������ �����������
            hAxes = get(src, 'Parent');
         % �������� ��� � ����� ����������� ����
            hNewA = copyobj(hAxes, hNewF);
          % ������ ������� � ��������� ����� ����
               set(hNewA, 'Position', [0.13 0.11 0.775 0.815])
        % �������� ��������� �� �������� ����
           hC = get(hNewA, 'Children');
         % ������ �� �������� ButtonDownFcn �������� ������ ������
            set(hC, 'ButtonDownFcn', [])
  end
  

end

