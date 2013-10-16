function[H,g,norma]= CanonType2(A,b)
sz=size(A);
n=sz(1);
m=sz(2);
if(n==m)
    optval=OptValue(A);
    for i=1:n
        for j=1:n
            
            if(i==j)
                E(i,j)=1;
            else E(i,j)=0;
            end
        end
    end
    
    H=E-optval*A; g=optval*b;
    norma=norm(H,inf);
        
end

end

