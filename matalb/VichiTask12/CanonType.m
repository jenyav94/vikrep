function [H,g] = CanonType(A,b)
sz=size(A);
n=sz(1);
m=sz(2);
if(n==m)
for i=1:n
    for j=1:m
        if(i==j)
           H(i,j)=0;
        else H(i,j)=-A(i,j)/A(i,i);
        end
    end
end
  for i=1:n
      g(i)=b(i)/A(i,i);
  end
  

else str='CanonType Error'
end

end

