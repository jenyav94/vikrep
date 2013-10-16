function alfa=OptValue(A)
sz=size(A);
n=sz(1);
ms=sz(2);
if(n==ms)
     m=0;
    for j=1:n
        m=m+A(1,j);
    end
      minm=A(1,1)-m;
    
      for i=2:n
        for j=1:n
            if(j~=i)
                m=m+A(i,j);
            end
        end
        m=A(i,i)-m;
        if(m<minm)
            minm=m;
        end
      end
M=0;
    for j=1:n
        M=M+A(1,j);
    end
    maxM=A(1,1)+M;
    
      for i=2:n
        for j=1:n
            if(j~=i)
                M=M+A(i,j);
            end
        end
        M=A(i,i)+m;
        if(M>maxM)
            maxM=M;
        end
      end
    
    alfa=2/(m+M);
else str='OptVal Error'
end

end

