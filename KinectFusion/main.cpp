#include"KinectFusionProcessing.h"

int main(){
	
	KinectFusionProcessing *test=new KinectFusionProcessing;
	test->CreateFirstConnected();
	test->InitializeKinectFusion();
	test->Run();
	
	delete test;
	
	
return 0;}