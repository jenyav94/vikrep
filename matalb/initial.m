clear all;
global Rs Rr M Ls Lr sigma gamma beta delta eta np m h ibet
global epsilon G J Tl ro f AA BB O I er GG Ki Kp lam_ref
global num0 num1 denom hh gg rro
global Rs_ Rr_ M_ Ls_ Lr_ sigma_ gamma_ beta_ delta_ eta_ h ibet_
global epsilon_ tau0 taumin taumax thr H weights iw but ter
global theshold HH l dl nl
global bMLr deltaRs dR dw discont
HH=0.1;
H=0.01;
h=0.001;
hh=h/5;
lam_ref=0.02
%Ki=10;
Ki=20;
%Kp=0.2;
Kp=0.1;
%er=0.05;
er=0
Rs=0.0135;
m=0.0005;
G=100; %0;
np=2;
J=[0 -1;1 0];
Rr=0.012;
M=0.0005;
Ls=M+0.00007;
Lr=M+0.00007;
sigma=1-M^2/Ls/Lr;
gamma=(Rs+M^2*Rr/Lr^2)/sigma/Ls;
beta=M/sigma/Lr/Ls;
eta=Rr/Lr;
delta=1/sigma/Ls;
epsilon=3*np*M/m/Lr/2;
bMLr=beta*M*Lr-delta*(M/Lr)^2;
deltaRs=delta*Rs;
alpha=0.1;