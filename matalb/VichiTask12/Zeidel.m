function [X1,X2] = Zeidel(A,b,k)
sz=size(A);
n=sz(1);
m=sz(2);
[H,g]=CanonType(A,b);
norm(H,inf)
if(m==n)
   t=1;
   xk=zeros(n,1);
   %xk- x k-ой итерации
   %xk1 x k+1-й итерации
   while(t<=k)
       
    for i=1:n
       sum=0;
       for j=1:i-1
             sum=sum+H(i,j)*xk1(j);
       end
        
        
       for j=i:n
           sum=sum+H(i,j)*xk(j);
       end
       
          xk1(i)=sum+g(i);
       
    end
      xks=xk; %сохраняет k-ю итерацию
      xk=xk1;
      t=t+1;
   end
   X1=xk;
   X2=xks;
else
    str='Zeidel Error'
end
end

