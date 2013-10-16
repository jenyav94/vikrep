import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

import java.util.Scanner;





public class InverseMat {
	 Matrix mat1,mated;
	   Vector res;
	   int n;
	   Scanner scanner=null;
	   double eps;
	   boolean er=true;
		double[] nev;
		int[] ordx;
		
		
		class Matrix{
	   	 double[][] mat;
	   	   Matrix(int n, int m){mat=new double[n][m];}
	    }
	    
	    class Vector{
	   	 double [] vect;
	   	 Vector(int n){vect=new double[n];}
	    }
	    
	    Matrix MultiMat(Matrix fir, Matrix sec,int t){
	    	Matrix matres=new Matrix(t,t);
	    	for (int i = 0; i < t; i++) {
	            for (int j = 0; j < t; j++) {
	                for (int k = 0; k < t; k++) {
	                   matres.mat[i][j] += fir.mat[k][i] * sec.mat[j][k]; 
	                }
	            }
	        }
	    	return matres;
	    }
	    
	    void Enter() throws NumberFormatException, IOException{
	   	
	   	 
	   	 
	   	 
	   	scanner=new Scanner(new BufferedReader(new FileReader("C:\\Users\\--\\workspace\\SystemGause\\src\\mat.txt")));
	    	n=scanner.nextInt();
	   	 
	   	 mat1=new Matrix(n,n+1) ;
	   	 mated=new Matrix(n,n);
	   	 
	   	
	   	 for(int i=0; i<n;i++)
	   		 for(int j=0;j<n+1;j++){ 
	   			 mat1.mat[i][j]=scanner.nextDouble();
	   			 if(i==j)mated.mat[i][j]=1;
	   			 else if(j!=n) mated.mat[i][j]=0;}
	   	 
	   	 
	  
	   	 scanner.close();
	   	 
	   }
	    
	    void DiagGeord(){
	    	int k=0;
	    	double tmp;
	    	
	    	  
	    	 while(k<=n-1){
	    		tmp=mat1.mat[k][k];
	    		
	    		if(Math.abs(tmp)<=eps){er=false;}
	    		for(int j=k;j<n;j++)
	    		   mat1.mat[k][j]=mat1.mat[k][j]/tmp;
	    		   for(int s=0; s<n;s++)
	    		   mated.mat[k][s]=mated.mat[k][s]/tmp;
	    		
	    			for(int i=0; i<n;i++){
	    				tmp=mat1.mat[i][k];
	    				if(i!=k){
	    				  for(int j=k;j<n;j++)
	    				mat1.mat[i][j]=mat1.mat[i][j]-mat1.mat[k][j]*tmp;
	    				  for(int s=0; s<n;s++)
	    				mated.mat[i][s]=mated.mat[i][s]-mated.mat[k][s]*tmp;
	    				}
	    			}
	    	
	    		k++;
	    		
	        	
	    }
	   }
	    
	    void SwapGeord(int str, int stl, int t){
	    	double aux,aux1;
	    	
	    	for(int i=0; i<n;i++){
	    		aux=mat1.mat[i][stl];
				mat1.mat[i][stl]=mat1.mat[i][t];
				mat1.mat[i][t]=aux;
				
				aux1=mated.mat[i][stl];
				mated.mat[i][stl]=mated.mat[i][t];
				mated.mat[i][t]=aux1;
	    	}
	    		for(int j=0; j<n;j++){
	    			aux=mat1.mat[str][j];
	    			mat1.mat[str][j]=mat1.mat[t][j];
	    			mat1.mat[t][j]=aux;
	    			
	    			aux1=mated.mat[str][j];
	    			mated.mat[str][j]=mated.mat[t][j];
	    			mated.mat[t][j]=aux1;
	    		}
	    			
	    }

	    void Print() throws NumberFormatException, IOException{
	    	Matrix check,Nat;
	        
	    	System.out.println();
	    	Enter();
	    	Nat=new Matrix(n,n);
	    	for(int i=0;i<n;i++)
	    		for(int j=0;j<n;j++) Nat.mat[i][j]=mat1.mat[i][j];
	    	DiagGeord();
	    	
	    	System.out.println();
	    	System.out.println("Обратная матрица:");
	    	for(int i=0;i<n;i++){
	    		System.out.println();
	    		for(int j=0;j<n;j++)System.out.print(mated.mat[i][j]+" ");
	    	}
	    	System.out.println();
	    	
	    	System.out.println("Исходная матрица:");
	    	for(int i=0;i<n;i++){
	    		System.out.println();
	    		for(int j=0;j<n;j++)System.out.print(mat1.mat[i][j]+" ");
	    	}
	    	System.out.println();
	    	System.out.println("Проверка:");
	    	check=MultiMat(Nat,mated,n);
	    	for(int i=0;i<n;i++){
	    		System.out.println();
	    		for(int j=0;j<n;j++)System.out.print(check.mat[i][j]+" ");
	    	}
	    	
	    }
}
