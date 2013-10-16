function [X1 X2]=RelaxMode(A,b)
sz=size(A);
n=sz(1);
m=sz(2);
H=CanonType(A,b);
q=2/(1+sqrt(1-SpectrRad(H))^2);
if m==n
      ch=10;
   xk1=zeros(1,n);
   %xk- x k-ой итерации
   %xk1 x k-1-й итерации
   while(ch>=10^(-5))
       
      for i=1:n
         sum1=0;
         
        for j=1:n
            if(j<i)
         sum1=sum1+A(i,j)*xk(j);
            else
                sum1=sum1+A(i,j)*xk1(j);
            end  
        end
        sum2=q*(b(i)-sum1)/A(i,i);
        xk(i)=xk1(i)+sum2;
        
        
      end 
   
    ch=norm(xk-xk1,inf);
      xks=xk1; %сохраняет k-ю итерацию
      xk1=xk;
     
   end
   X1=xk;
   X2=xks;
else
    str='RelaxMode Error'
end
    

end

