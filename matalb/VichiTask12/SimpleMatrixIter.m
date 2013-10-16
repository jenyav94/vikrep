function [X1,X2] = SimpleMatrixIter(A,b,k)
sz=size(A);
n=sz(1);
m=sz(2);
[H g]=CanonType(A,b);
if(m==n)
    xk=zeros(n,1);
    t=1;
    while(t<=k)
        xk1=H*xk+g';
        xk2=xk;
        xk=xk1;
        t=t+1;
    end
    X1=xk;
    X2=xk2;
else str='SimpleMatrixIter Error'
end



end

