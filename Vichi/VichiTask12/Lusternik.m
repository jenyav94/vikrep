function X = Lusternik( xk1,xk,A,b)
%xk1 - k-1�
%xk - k�
H=CanonType(A,b);
X=xk1+(xk-xk1)/(1-SpectrRad(H));

end

