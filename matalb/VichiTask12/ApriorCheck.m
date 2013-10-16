function Res = ApriorCheck(RealVal,IterVal,H,g,k)
Res=(norm(H,inf)^k)*norm(g,inf)/(1-norm(H,inf));
end

