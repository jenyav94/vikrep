function [eigenval,X] = JacobiMethod(A,eps)
sz=size(A);
n=sz(1);
m=sz(2);
  if(m==n)
     X=eye(n);
     
     %нахождение максимального недиагонального элемента матрицы и
     %запоминание его индексов
     aMax=abs(A(1,2)); ik=1; jk=2;
       for i=1:n
           for j=2:n
               if( (i<j) && (abs(A(i,j))>aMax) )
                   aMax=abs(A(i,j));
                   ik=i;
                   jk=j;
               end
           end
       end
       
       %выполнение итераций пока максимальный недиагональный элемент ни
       %станет меньше eps
       while(aMax>=eps)
           d=sqrt(((A(ik,ik)-A(jk,jk))^2)+4*(A(ik,jk)^2));
           c=sqrt(  (1+(abs(A(ik,ik)-A(jk,jk))/d))/2 );
           if(A(ik,ik)==A(jk,jk))
               s=sqrt(  (1-(abs(A(ik,ik)-A(jk,jk))/d))/2 );
           else
           s=sign(A(ik,jk)*(A(ik,ik)-A(jk,jk)))*sqrt(  (1-(abs(A(ik,ik)-A(jk,jk))/d))/2 );
           end
           
           for i=1:n
               for j=1:n
                   
                   if(i~=ik && i~=jk && j~=ik && j~=jk)
                       Ak1(i,j)=A(i,j);
                      
                   elseif(i~=ik && j~=jk)
                          Ak1(i,jk)=-s*A(i,ik)+c*A(i,jk); 
                          Ak1(jk,i)=-s*A(i,ik)+c*A(i,jk);
                  
                   else
                       Ak1(i,ik)=c*A(i,ik)+s*A(i,jk);
                       Ak1(ik,i)=c*A(i,ik)+s*A(i,jk);
                       Ak1(ik,ik)=c*c*A(ik,ik)+2*c*s*A(ik,jk)+s*s*A(jk,jk);
                       Ak1(jk,jk)=s*s*A(ik,ik)-2*c*s*A(ik,jk)+c*c*A(jk,jk);
                       Ak1(ik,jk)=(c*c-s*s)*A(ik,jk)+c*s*(A(jk,jk)-A(ik,ik));
                       Ak1(jk,ik)=(c*c-s*s)*A(ik,jk)+c*s*(A(jk,jk)-A(ik,ik));
                       
                       
                   end
               end
           end
           
             for i=1:n
                 
                     temp1=X(i,ik);
                     temp2=X(i,jk);
                     X(i,ik)=c*X(i,ik)+s*X(i,jk);
                     X(i,jk)=-s*temp1+c*temp2; 
                     
                 
             end
 
           Aks=A;
           A=Ak1;
           
           
            aMax=abs(A(1,2)); ik=1;jk=2;
       for i=1:n
           for j=1:n
               if( (i<j) && (abs(A(i,j))>aMax) )
                   aMax=abs(A(i,j));
                   ik=i;
                   jk=j;
               end
           end
       end
           
       end
       eigenval=[];
       for i=1:n
       eigenval=[eigenval A(i,i)];
       end
     
  else str='JacobiMethod ERROR'
  end

end

