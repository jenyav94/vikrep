function [X1,X2,spectrad] = ZeidelMatrix(A,b,k)
sz=size(A);
n=sz(1);
m=sz(2);
[H g]=CanonType(A,b);
if(m==n)
    
for i=1:n
    for j=1:m
        if(i>j)
           Hl(i,j)=H(i,j);
           Hr(i,j)=0;
        else
            Hl(i,j)=0;
            Hr(i,j)=H(i,j);
        end
    end   
end

E=eye(n);
   Inv=E/(E-Hl);
    Hseid=Inv*Hr;
   gseid=Inv*g';

    xk=zeros(n,1);
    
    for i=1:k
        xk1=Hseid*xk+gseid;
        xk2=xk;
        xk=xk1;
    end
    X1=xk;
    X2=xk2;
    
    spectrad=SpectrRad(Hseid);
else str='ZeidelMatrix Error'
end
end

