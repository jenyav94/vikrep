function [X1,X2] = IterOptParam(A,b,k)
sz=size(A);
n=sz(1);
m=sz(2);
[H g norma]=CanonType2(A,b);
H
if(m==n)
    xk=zeros(n,1); 
    for i=1:k
        xk1=H*xk+g';
        xk2=xk;
        xk=xk1;
    end
    X1=xk;
    X2=xk2;
else str='IterOptParam Error'
end

end

