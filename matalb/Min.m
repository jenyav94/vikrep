function ocenka = Min(mas,vect1,vect2)

vect1=[2 2];
vect2=[1.99 1.97];

x=vect2/mas;
y=vect1/mas;

nvec=norm(x-y,1);
nmat=norm(mas,1);

ocenka= cond(mas)*(norm(vect2-vect1,1)/norm(vect2,1));
otnpogr=nvec/norm(x,1)
abspogr=nvec
end

