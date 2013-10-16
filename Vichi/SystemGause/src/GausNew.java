import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

import java.util.Scanner;








public class GausNew {
	Matrix mat1,matnev;
	   Vector res;
	   int n;
	   Scanner scanner=null;
	   
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
	   	 matnev=new Matrix(n,n+1);
	   	 
	   	
	   	 for(int i=0; i<n;i++)
	   		 for(int j=0;j<n+1;j++){ 
	   			 mat1.mat[i][j]=scanner.nextDouble();
	   			matnev.mat[i][j]=mat1.mat[i][j];}
	   	 
	   	 
	  
	   	 scanner.close();
	   	 
	   }


void DiagNew(){
	int k=0;
	double tmp;
	int[] tmpNew;
	ordx=new int[n];
	int aux;
	  for(int i=0;i<n;i++) ordx[i]=i;
	
	while(k<=n-1){
		tmp=mat1.mat[k][k];
		
		if(k!=(n-1)){ //на последнем шаге остается один элемент// 
		tmpNew=Max(k);
		tmp=mat1.mat[tmpNew[0]][tmpNew[1]];
		Swap(tmpNew[0],tmpNew[1],k);
		
		aux=ordx[k];
	     ordx[k]=tmpNew[1];
	     ordx[tmpNew[1]]=aux;
		   
		}
		
		
		
		  
		for(int j=k;j<=n;j++)
		   mat1.mat[k][j]=mat1.mat[k][j]/tmp;
		
			for(int i=k+1; i<=n-1;i++){
				tmp=mat1.mat[i][k];
				    for(int j=k;j<=n;j++)
				mat1.mat[i][j]=mat1.mat[i][j]-mat1.mat[k][j]*tmp;
				
			}
	
		k++;
		
}

}

void Swap(int str, int stl, int t){
	double aux;
	
	for(int i=0; i<n;i++){
		aux=mat1.mat[i][stl];
		mat1.mat[i][stl]=mat1.mat[i][t];
		mat1.mat[i][t]=aux;
	}
		for(int j=0; j<n+1;j++){
			aux=mat1.mat[str][j];
			mat1.mat[str][j]=mat1.mat[t][j];
			mat1.mat[t][j]=aux;
		}
			
}

int[] Max(int t){
	double max;
	int[] res=new int[2];
	
	max=mat1.mat[t][t];
	res[0]=t; res[1]=t;
	
	for(int i=t; i<n;i++)
	       for(int j=t; j<n; j++)
		if(max<mat1.mat[i][j]){max=mat1.mat[i][j]; res[0]=i; res[1]=j;}
		
		return res;
	
	
}

void ReverseNew(){
	res=new Vector(n);
	double sum=0;
	int i=n-1;
	
	while(i>=0){
		for(int j=i+1;j<=n-1;j++) sum+=mat1.mat[i][j]*res.vect[ordx[j]];
		res.vect[ordx[i]]=mat1.mat[i][n]-sum;
		sum=0;
		i--;
	}
}

void NevNew(){
	double[] Ax;
	Ax=new double[n];
	nev=new double[n];
	
	for(int i=0; i<n;i++){
		Ax[i]=0;
	    for(int j=0; j<n; j++)
		   Ax[i]+=matnev.mat[i][j]*res.vect[j];
	}
	for(int i=0;i<n;i++)
		 nev[i]=matnev.mat[i][n]-Ax[i];
}

void Print() throws NumberFormatException, IOException{
        Enter();
        
    System.out.println();
	System.out.println("Исходная матрица:");
	for(int i=0;i<n;i++){
		System.out.println();
		for(int j=0;j<=n;j++)System.out.print(mat1.mat[i][j]+" ");
	}
	
	System.out.println();
    DiagNew();
	ReverseNew();
	NevNew();
	
	System.out.println();
	System.out.println("Ответ:");
	for(int i=0;i<n;i++)System.out.println(res.vect[i]);
	if(!er) System.out.println("Ведущий элемент меньше эпсилон");
	
	
	System.out.println();
	System.out.println("Невязка:");
	
	for(int i=0;i<n;i++)System.out.println(nev[i]);
}
}

