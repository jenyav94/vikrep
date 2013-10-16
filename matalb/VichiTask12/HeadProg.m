function HeadProg(A,b)

RealValue=RealVal(A,b);
disp('Accurate Decision');
disp(RealValue');

OptimalValue=OptValue(A)

[Hl gl normHl]=CanonType2(A,b);
normHl

[Xkopt Xk_1opt]=IterOptParam(A,b,9);
disp('Decision Optimal Param');
disp(Xkopt);

FactCheck=Check(RealValue,Xkopt')
Apastorior=ApastoriorCheck(Xkopt,Xk_1opt,Hl)
Aprior=ApriorCheck(RealValue,Xkopt,Hl,gl,9)
[XoptLuster]=Lusternik(Xk_1opt,Xkopt,A,b);
disp('Lusternic Optimal Param');
disp(XoptLuster);

[XkZ Xk_1Z]=Zeidel(A,b,9);
disp('Decision Zeidel');
disp(XkZ');
FactCheckZeidel=Check(RealValue,XkZ)

[XkSimp Xk_1Simp]=SimpleIter(A,b,9);
disp('Decision Simple Iteration');
disp(XkSimp');

[XkZeidIter Xk_1ZeidIter spectrad]=ZeidelMatrix(A,b,9);
disp('Decision Zeidel like Simple Iteration');
disp(XkZeidIter);

ZeidIterLuster=Lusternik(XkZeidIter, Xk_1ZeidIter,A,b);
disp('Lusternic Zeidel like Simple Iteration');
disp(ZeidIterLuster);

FactCheck=Check(XkZeidIter,ZeidIterLuster)
spectrad

end

