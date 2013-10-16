function Res= ApastoriorCheck(CurrVal,PrevVal,H)
Res=norm(H,inf)*norm(abs(CurrVal-PrevVal),inf)/(1-norm(H,inf));

end

