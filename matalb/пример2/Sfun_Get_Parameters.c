//===================================================================

double  A00, A01, A10, B00, B11, C00, C11;

void Sfun_Get_Parameters(SimStruct *S)
{
    const real_T        *param0  = mxGetPr(PARAM_DEF0(S));
    const real_T        *param1  = mxGetPr(PARAM_DEF1(S));
    const real_T        *param2  = mxGetPr(PARAM_DEF2(S));
    const real_T        *param3  = mxGetPr(PARAM_DEF3(S));
    const real_T        *param4  = mxGetPr(PARAM_DEF4(S));
    const real_T        *param5  = mxGetPr(PARAM_DEF5(S));

    const int_T         p_width0  = mxGetNumberOfElements(PARAM_DEF0(S));
    const int_T         p_width1  = mxGetNumberOfElements(PARAM_DEF1(S));
    const int_T         p_width2  = mxGetNumberOfElements(PARAM_DEF2(S));
    const int_T         p_width3  = mxGetNumberOfElements(PARAM_DEF3(S));
    const int_T         p_width4  = mxGetNumberOfElements(PARAM_DEF4(S));
    const int_T         p_width5  = mxGetNumberOfElements(PARAM_DEF5(S));


    double L,R,J,Cm,Cw,Fi;

    L=*param0;  R=*param1;  J=*param2; Cm=*param3; Cw=*param4;  Fi=*param5;

    A00 = -R/L;   A01 = -Cw*Fi/L;  A10 = Cm*Fi/J; 
    B00 = 1/L;    B11 = -1/J;      C00 = Cm*Fi;   C11 = 1;
}
//===================================================================
