function [X1,X2] = SimpleIter(A,b,k)
sz=size(A);
n=sz(1);
m=sz(2);
[H g]=CanonType(A,b);
if(m==n)
    t=1;
    xk=zeros(n,1); %вектор x k-й итерации
    %xk1 вектор k+1-й итерации
    while(t<=k)
        for i=1:n
            xk1(i)=0;
            for j=1:n
                xk1(i)=xk1(i)+H(i,j)*xk(j);
            end
            xk1(i)=xk1(i)+g(i);
        end
        xks=xk; %сохраняет x k-й итерации для люстерника
        xk=xk1;
        t=t+1;
    end
    X1=xk;
    X2=xks;
   
else
    str='SimpleIter Error'
end


end

